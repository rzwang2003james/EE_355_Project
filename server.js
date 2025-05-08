const express = require('express');
const fs = require('fs').promises;
const path = require('path');
const { exec } = require('child_process');
const util = require('util');

const app = express();
app.use(express.json()); // Middleware to parse JSON bodies
const PORT = process.env.PORT || 3000;
const DB_FILE = path.join(__dirname, 'networkDB.txt');
const CPP_EXECUTABLE = path.join(__dirname, 'test_network.o');
const PYTHON_SCRIPT = path.join(__dirname, 'recommendations.py');

// Promisify exec for cleaner async/await usage
const execPromise = util.promisify(exec);

// --- C++ Interaction Helper ---
async function runCppTool(args) {
    const command = `"${CPP_EXECUTABLE}" ${args.join(' ')}`;
    console.log('Executing C++ command:', command); // Log the command being run
    try {
        const { stdout, stderr } = await execPromise(command);
        if (stderr) {
            // Treat C++ stderr output as potential operational errors (like 'not found') or warnings
            console.warn(`C++ stderr: ${stderr.trim()}`);
            // Decide if stderr always means failure. For now, check for specific error markers.
            if (stderr.includes('ERROR:')) {
                throw new Error(`C++ Error: ${stderr.trim()}`);
            }
        }
        console.log('C++ stdout:', stdout.trim());
        return { success: true, data: stdout.trim(), stderr: stderr.trim() };
    } catch (error) {
        // This catches errors from exec itself (e.g., command not found) or errors thrown above
        console.error(`Error executing C++ command: ${command}`, error);
        // Extract a cleaner message if possible
        const message = error.stderr || error.stdout || error.message;
        // Ensure we throw a standard error object
        const err = new Error(`Failed to execute C++ tool: ${message.trim()}`);
        err.originalError = error; // Keep original error context if needed
        throw err; 
    }
}

// --- API Endpoints ---

// GET /api/people (List all people summaries)
app.get('/api/people', async (req, res) => {
    try {
        const result = await runCppTool(['--get-all']);
        const peopleList = result.data
            .split(/\r?\n/)
            .filter(line => line.trim() !== '')
            .map(line => {
                const parts = line.split(':');
                return { codeName: parts[0], fname: parts[1], lname: parts[2] };
            });
        res.json(peopleList);
    } catch (error) {
        res.status(500).json({ error: 'Failed to get people list', details: error.message });
    }
});

// GET /api/people/:codename (Get person details)
app.get('/api/people/:codename', async (req, res) => {
    const codename = req.params.codename;
    // Basic validation for codename format if needed
    if (!codename || codename.includes(' ') || codename.includes('..')) { 
        return res.status(400).json({ error: 'Invalid codename format' });
    }
    try {
        const result = await runCppTool(['--get-details', codename]);
        const lines = result.data.split(/\r?\n/);
        const person = { codeName: codename, friends: [], additional_info: {} };
        let infoKey = '';

        for (const line of lines) {
            if (line === '---DETAILS_END---') break;
            const separatorPos = line.indexOf(':');
            if (separatorPos === -1) continue; // Skip lines without a colon

            const key = line.substring(0, separatorPos);
            const value = line.substring(separatorPos + 1);

            if (key === 'fname') person.fname = value;
            else if (key === 'lname') person.lname = value;
            else if (key === 'bdate') person.bdate = value;
            else if (key === 'phone') person.phone = value;
            else if (key === 'email') person.email = value;
            else if (key === 'friend') person.friends.push(value);
            else if (key.startsWith('info_')) {
                infoKey = key.substring(5); // Remove 'info_' prefix
                person.additional_info[infoKey] = value;
            }
        }
        res.json(person);

    } catch (error) {
        // C++ tool might return error/stderr if not found, which runCppTool should catch
        if (error.message && error.message.includes('not found')) {
             res.status(404).json({ error: `Person with codeName '${codename}' not found`, details: error.message });
        } else {
             res.status(500).json({ error: 'Failed to get person details', details: error.message });
        }
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

    const args = ['--add', '--fname', fname, '--lname', lname];
    if (bdate) args.push('--bdate', bdate);
    if (phone) args.push('--phone', phone);
    if (email) args.push('--email', email);
    if (additional_info) {
        for (const [key, value] of Object.entries(additional_info)) {
            if (key && value) { // Ensure key/value are not empty
                 // Ensure key doesn't contain problematic characters for shell
                 const safeKey = key.replace(/[:\s]/g, '_'); // Basic sanitization
                 args.push('--info', `${safeKey}:${value}`);
            }
        }
    }

    try {
        const result = await runCppTool(args);
        // Assuming C++ outputs "SUCCESS: Added <codename>" on stdout
        const match = result.data.match(/SUCCESS: Added (\S+)/);
        if (match && match[1]) {
             // Optionally, call --get-details to return the full new person object
             // For now, just return success and the codename
             res.status(201).json({ message: result.data, codeName: match[1] });
        } else {
             throw new Error('C++ tool did not confirm addition.' + (result.stderr ? ` Stderr: ${result.stderr}`: ''));
        }
    } catch (error) {
         if (error.message && error.message.includes('already exists')) {
             res.status(409).json({ error: 'Person already exists', details: error.message });
         } else {
             res.status(500).json({ error: 'Failed to add person', details: error.message });
         }
    }
});

// DELETE /api/people/:codename (Remove Person)
app.delete('/api/people/:codename', async (req, res) => {
    const codenameToRemove = req.params.codename;
     if (!codenameToRemove || codenameToRemove.includes(' ') || codenameToRemove.includes('..')) { 
        return res.status(400).json({ error: 'Invalid codename format' });
    }

    try {
        const result = await runCppTool(['--remove', codenameToRemove]);
         if (result.data.includes('SUCCESS: Removed')) {
             res.status(200).json({ message: `Person '${codenameToRemove}' removed successfully.` });
         } else {
             throw new Error('C++ tool did not confirm removal.' + (result.stderr ? ` Stderr: ${result.stderr}`: ''));
         }
    } catch (error) {
         if (error.message && error.message.includes('not found')) {
             res.status(404).json({ error: `Person '${codenameToRemove}' not found`, details: error.message });
         } else {
            res.status(500).json({ error: 'Failed to remove person', details: error.message });
         }
    }
});

// POST /api/connect (Connect Friends)
app.post('/api/connect', async (req, res) => {
    const { codeName1, codeName2 } = req.body;
    console.log(`Connect request received: codeName1='${codeName1}', codeName2='${codeName2}'`); 
    if (!codeName1 || !codeName2 || codeName1 === codeName2) {
        return res.status(400).json({ error: 'Two different valid codeNames are required.' });
    }
    // Add validation? Ensure they are not empty strings etc.
    if ( [codeName1, codeName2].some(c => !c || c.includes(' ') || c.includes('..')) ) {
         return res.status(400).json({ error: 'Invalid codename format provided' });
    }

    try {
        const result = await runCppTool(['--connect', codeName1, codeName2]);
        if (result.data.includes('SUCCESS: Connected') || result.data.includes('already connected')) {
             res.status(200).json({ message: `Connection update for '${codeName1}' and '${codeName2}' processed.` , details: result.data});
        } else {
              throw new Error('C++ tool did not confirm connection.' + (result.stderr ? ` Stderr: ${result.stderr}`: ''));
        }
    } catch (error) {
        // Specific check for 'not found' errors from C++ stderr
        if (error.message && error.message.includes('not found')) {
             res.status(404).json({ error: 'One or both persons not found for connection.', details: error.message });
        } else if (error.message && error.message.includes('to themselves')) {
             res.status(400).json({ error: 'Cannot connect a person to themselves.', details: error.message });
        } else {
            res.status(500).json({ error: 'Failed to connect persons', details: error.message });
        }
    }
});

// POST /api/generate (Generate Demo Data)
// Keep JS implementation for this, as it avoids defining complex data in C++
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

    // Need a temporary JS function to format this demo data into the C++ file format
    // OR just write the file directly from JS, avoiding C++ for this specific action.
    // Let's keep the direct write approach:
    let fileContent = '';
    const personCodes = Object.keys(demoPeople);
    personCodes.forEach((code, index) => {
        const person = demoPeople[code];
        fileContent += `${person.fname || ''}\n`;
        fileContent += `${person.lname || ''}\n`;
        fileContent += `${person.bdate || ''}\n`;
        fileContent += `${person.phone || ''}\n`;
        fileContent += `${person.email || ''}\n`;
        if (person.additional_info) {
            for (const [key, value] of Object.entries(person.additional_info)) {
                fileContent += `${key}:${value}\n`;
            }
        }
        fileContent += `---INFO_END---\n`;
        if (person.friends) {
            person.friends.forEach(friendCode => {
                fileContent += `${friendCode}\n`;
            });
        }
        if (index < personCodes.length - 1) {
            fileContent += `--------------------\n`;
        }
    });

    try {
        await fs.writeFile(DB_FILE, fileContent, 'utf8');
        res.status(200).json({ message: 'Demo data generated and saved successfully (using JS).' });
    } catch (err) {
        console.error(`Error writing demo data file '${DB_FILE}':`, err);
         res.status(500).json({ error: 'Failed to save generated demo data.', details: err.message });
    }
});

// --- Static File Server ---
app.use(express.static(path.join(__dirname, 'public')));

// --- Server Start ---
app.listen(PORT, () => {
    console.log(`TrojanBook Phase 3 server running at http://localhost:${PORT}`);
    console.log(`Using C++ executable: ${CPP_EXECUTABLE}`);
}); 