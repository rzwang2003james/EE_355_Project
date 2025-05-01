const express = require('express');
const fs = require('fs').promises;
const path = require('path');
const { exec } = require('child_process');

const app = express();
app.use(express.json()); // Middleware to parse JSON bodies
const PORT = process.env.PORT || 3000;
const DB_FILE = path.join(__dirname, 'networkDB.txt');
const PYTHON_SCRIPT = path.join(__dirname, 'recommendations.py');

// --- Data Parsing/Saving Logic ---

function codeNameJS(fName, lName) {
    const fnameLower = fName ? fName.toLowerCase().replace(/\s/g, '') : '';
    const lnameLower = lName ? lName.toLowerCase().replace(/\s/g, '') : '';
    return fnameLower + lnameLower;
}

async function parseNetworkFile(filename) {
    const people = {};
    try {
        const data = await fs.readFile(filename, 'utf8');
        const lines = data.split(/\r?\n/); // Split by newline

        let currentPerson = null;
        let readingState = 'fname'; // fname, lname, bdate, phone, email, info, friends, separator
        let i = 0;

        while (i < lines.length) {
            const line = lines[i].trim();
            i++;

            // Skip empty lines, except potentially within multi-line values if we supported them
            if (!line && readingState !== 'info' && readingState !== 'friends') continue;
            // Handle edge case of completely empty file or just separators
            if (!line && i === lines.length) break;

            if (readingState === 'fname') {
                if (!line || line === '--------------------') continue; // Skip separators at start
                currentPerson = { fname: line, additional_info: {}, friends: [] };
                readingState = 'lname';
            } else if (readingState === 'lname') {
                currentPerson.lname = line;
                currentPerson.codeName = codeNameJS(currentPerson.fname, currentPerson.lname);
                readingState = 'bdate';
            } else if (readingState === 'bdate') {
                currentPerson.bdate = line;
                readingState = 'phone';
            } else if (readingState === 'phone') {
                currentPerson.phone = line;
                readingState = 'email';
            } else if (readingState === 'email') {
                currentPerson.email = line;
                readingState = 'info';
            } else if (readingState === 'info') {
                if (line === '---INFO_END---') {
                    readingState = 'friends';
                } else if (line.includes(':')) {
                    const [key, ...valueParts] = line.split(':');
                    const value = valueParts.join(':');
                    if (key.trim()) {
                       currentPerson.additional_info[key.trim()] = value.trim();
                    }
                } else if (line === '--------------------') {
                    // Separator encountered before INFO_END - error or legacy?
                    // Assuming end of person if separator found here.
                    if (currentPerson && currentPerson.codeName) {
                        people[currentPerson.codeName] = currentPerson;
                    }
                    currentPerson = null;
                    readingState = 'fname';
                } else if (line) { // Treat non-empty, non-keyed line as potential friend code (legacy?)
                     currentPerson.friends.push(line);
                     readingState = 'friends'; // Jump to friend state
                }
                // Allow empty lines within info block?\ Current logic skips them.
            } else if (readingState === 'friends') {
                if (line === '--------------------') {
                    if (currentPerson && currentPerson.codeName) {
                        people[currentPerson.codeName] = currentPerson;
                    }
                    currentPerson = null;
                    readingState = 'fname';
                } else if (line) { // Only add non-empty lines as friend codes
                    currentPerson.friends.push(line);
                }
            }
        }

        // Add the last person if file doesn't end with separator
        if (currentPerson && currentPerson.codeName && !(currentPerson.codeName in people)) {
            people[currentPerson.codeName] = currentPerson;
        }

    } catch (err) {
        if (err.code === 'ENOENT') {
            // File not found is okay, means an empty network
            console.log(`Database file '${filename}' not found. Starting fresh.`);
            return {}; // Return empty object, not null
        } else {
            console.error(`Error parsing file '${filename}':`, err);
            return null; // Indicate a real parsing error
        }
    }
    return people;
}

async function saveNetworkFile(filename, peopleData) {
    let fileContent = '';
    const personCodes = Object.keys(peopleData);

    personCodes.forEach((code, index) => {
        const person = peopleData[code];
        fileContent += `${person.fname || ''}\n`;
        fileContent += `${person.lname || ''}\n`;
        fileContent += `${person.bdate || ''}\n`;
        fileContent += `${person.phone || ''}\n`;
        fileContent += `${person.email || ''}\n`;

        // Write Additional Info
        if (person.additional_info) {
            for (const [key, value] of Object.entries(person.additional_info)) {
                fileContent += `${key}:${value}\n`;
            }
        }
        fileContent += `---INFO_END---\n`;

        // Write friend codes
        if (person.friends) {
            person.friends.forEach(friendCode => {
                fileContent += `${friendCode}\n`;
            });
        }

        // Separator
        if (index < personCodes.length - 1) {
            fileContent += `--------------------\n`;
        }
    });

    try {
        await fs.writeFile(filename, fileContent, 'utf8');
        return true;
    } catch (err) {
        console.error(`Error writing file '${filename}':`, err);
        return false;
    }
}

// --- API Endpoints ---

// GET /api/people (Existing)
app.get('/api/people', async (req, res) => {
    const peopleData = await parseNetworkFile(DB_FILE);
    if (peopleData !== null) {
        const peopleList = Object.values(peopleData).map(p => ({
            fname: p.fname,
            lname: p.lname,
            codeName: p.codeName
        }));
        res.json(peopleList);
    } else {
        res.status(500).json({ error: 'Failed to load network data' });
    }
});

// GET /api/people/:codename (Existing)
app.get('/api/people/:codename', async (req, res) => {
    const peopleData = await parseNetworkFile(DB_FILE);
    const codename = req.params.codename;
    if (peopleData !== null) {
        if (peopleData[codename]) {
            res.json(peopleData[codename]);
        } else {
            res.status(404).json({ error: `Person with codeName '${codename}' not found` });
        }
    } else {
        res.status(500).json({ error: 'Failed to load network data' });
    }
});

// GET /api/recommendations/:codename (Existing)
app.get('/api/recommendations/:codename', (req, res) => {
    const codename = req.params.codename;
    const command = `python "${PYTHON_SCRIPT}" "${codename}" -f "${DB_FILE}"`;
    exec(command, (error, stdout, stderr) => {
        if (error) {
            console.error(`Error executing python script: ${error}`);
            console.error(`Python stderr: ${stderr}`);
            return res.status(500).json({ error: 'Failed to get recommendations', details: stderr });
        }
        if (stderr) {
             console.warn(`Python stderr: ${stderr}`);
        }
        const recommendations = stdout.trim().split(/\r?\n/).filter(line => line.trim() !== '');
        res.json(recommendations);
    });
});

// POST /api/people (Add Person)
app.post('/api/people', async (req, res) => {
    const { fname, lname, bdate, phone, email, additional_info } = req.body;
    if (!fname || !lname) {
        return res.status(400).json({ error: 'First and last name are required.' });
    }

    const peopleData = await parseNetworkFile(DB_FILE);
    if (peopleData === null) {
        return res.status(500).json({ error: 'Failed to load network data before adding.' });
    }

    const newCodeName = codeNameJS(fname, lname);
    if (peopleData[newCodeName]) {
        return res.status(409).json({ error: `Person with codeName '${newCodeName}' already exists.` });
    }

    peopleData[newCodeName] = {
        fname,
        lname,
        codeName: newCodeName,
        bdate: bdate || '',
        phone: phone || '',
        email: email || '',
        additional_info: additional_info || {},
        friends: []
    };

    const saved = await saveNetworkFile(DB_FILE, peopleData);
    if (saved) {
        res.status(201).json(peopleData[newCodeName]); // Respond with the created person
    } else {
        res.status(500).json({ error: 'Failed to save network data after adding.' });
    }
});

// DELETE /api/people/:codename (Remove Person)
app.delete('/api/people/:codename', async (req, res) => {
    const codenameToRemove = req.params.codename;

    const peopleData = await parseNetworkFile(DB_FILE);
    if (peopleData === null) {
        return res.status(500).json({ error: 'Failed to load network data before removing.' });
    }

    if (!peopleData[codenameToRemove]) {
        return res.status(404).json({ error: `Person with codeName '${codenameToRemove}' not found.` });
    }

    // Remove the person
    delete peopleData[codenameToRemove];

    // Remove the person from others' friend lists
    for (const code in peopleData) {
        const person = peopleData[code];
        if (person.friends) {
            person.friends = person.friends.filter(friendCode => friendCode !== codenameToRemove);
        }
    }

    const saved = await saveNetworkFile(DB_FILE, peopleData);
    if (saved) {
        res.status(200).json({ message: `Person '${codenameToRemove}' removed successfully.` });
    } else {
        res.status(500).json({ error: 'Failed to save network data after removing.' });
    }
});

// POST /api/connect (Connect Friends)
app.post('/api/connect', async (req, res) => {
    const { codeName1, codeName2 } = req.body;
    console.log(`Connect request received: codeName1='${codeName1}', codeName2='${codeName2}'`); // Log received data
    if (!codeName1 || !codeName2 || codeName1 === codeName2) {
        return res.status(400).json({ error: 'Two different valid codeNames are required.' });
    }

    const peopleData = await parseNetworkFile(DB_FILE);
    if (peopleData === null) {
        return res.status(500).json({ error: 'Failed to load network data before connecting.' });
    }

    // Log available keys for debugging
    console.log('Available people keys:', Object.keys(peopleData));

    const person1 = peopleData[codeName1];
    const person2 = peopleData[codeName2];

    if (!person1 || !person2) {
        console.error(`Connect Error: Person1 found? ${!!person1}. Person2 found? ${!!person2}`); // Log lookup result
        return res.status(404).json({ error: 'One or both persons not found.' });
    }

    // Add friends bidirectionally, avoiding duplicates
    if (!person1.friends.includes(codeName2)) {
        person1.friends.push(codeName2);
    }
    if (!person2.friends.includes(codeName1)) {
        person2.friends.push(codeName1);
    }

    const saved = await saveNetworkFile(DB_FILE, peopleData);
    if (saved) {
        res.status(200).json({ message: `Connection between '${codeName1}' and '${codeName2}' successful.` });
    } else {
        res.status(500).json({ error: 'Failed to save network data after connecting.' });
    }
});

// POST /api/generate (Generate Demo Data)
app.post('/api/generate', async (req, res) => {
    const demoPeople = {
        'johndoe': {
            fname: 'John', lname: 'Doe', codeName: 'johndoe',
            bdate: '01/15/1995', phone: '(Work) 123-456-7890', email: '(Personal) john.doe@email.com',
            additional_info: { 'Major': 'CSCI', 'Year': 'Senior', 'City': 'Los Angeles' }, friends: ['janesmith']
        },
        'janesmith': {
            fname: 'Jane', lname: 'Smith', codeName: 'janesmith',
            bdate: '05/20/1996', phone: '(Mobile) 987-654-3210', email: '(Work) jane.smith@company.com',
            additional_info: { 'Major': 'EE', 'Year': 'Senior', 'City': 'Los Angeles' }, friends: ['johndoe', 'peterjones']
        },
        'peterjones': {
            fname: 'Peter', lname: 'Jones', codeName: 'peterjones',
            bdate: '11/01/1994', phone: '(Home) 555-123-4567', email: '(Other) pjones@mail.net',
            additional_info: { 'Major': 'CSCI', 'Year': 'Graduate', 'City': 'Pasadena' }, friends: ['janesmith']
        },
        'marychen': {
            fname: 'Mary', lname: 'Chen', codeName: 'marychen',
            bdate: '03/10/1997', phone: '(Mobile) 111-222-3333', email: '(Personal) mary.chen@email.com',
            additional_info: { 'Major': 'BME', 'Year': 'Junior', 'City': 'Irvine' }, friends: []
        }
    };

    const saved = await saveNetworkFile(DB_FILE, demoPeople);
    if (saved) {
        res.status(200).json({ message: 'Demo data generated and saved successfully.' });
    } else {
        res.status(500).json({ error: 'Failed to save generated demo data.' });
    }
});

// --- Static File Server ---
app.use(express.static(path.join(__dirname, 'public')));

// --- Server Start ---
app.listen(PORT, () => {
    console.log(`TrojanBook Phase 3 server running at http://localhost:${PORT}`);
}); 