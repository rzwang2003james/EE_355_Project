document.addEventListener('DOMContentLoaded', () => {
    // --- Element References ---
    const peopleListElement = document.getElementById('people-list');
    const personDetailsElement = document.getElementById('person-details');
    const recommendationsListElement = document.getElementById('recommendations-list');
    const addPersonBtn = document.getElementById('add-person-btn');
    const generateDataBtn = document.getElementById('generate-data-btn');
    const addPersonModal = document.getElementById('add-person-modal');
    const connectPersonModal = document.getElementById('connect-person-modal');
    const addPersonForm = document.getElementById('add-person-form');
    const closeModalButtons = document.querySelectorAll('.close-button');
    const connectPerson1Name = document.getElementById('connect-person1-name');
    const connectPerson1Code = document.getElementById('connect-person1-code');
    const connectPerson2Select = document.getElementById('connect-person2-select');
    const confirmConnectBtn = document.getElementById('confirm-connect-btn');

    // --- State ---
    let currentSelectedCodename = null;
    let peopleCache = {}; // Use object for easier lookup by codeName
    let allPeopleListCache = []; // Cache for populating connect dropdown

    // --- Helper Functions ---
    function displayLoading(element) {
        element.innerHTML = '<li>Loading...</li>';
    }

    function displayStatus(element, message, isError = false) {
        // Simple alert for now, could be improved with dedicated status area
        alert(message);
        console.log(message);
        if (isError) {
            console.error(message);
        }
    }

    function clearSelection() {
        const selected = peopleListElement.querySelector('.selected');
        if (selected) {
            selected.classList.remove('selected');
        }
        currentSelectedCodename = null;
        personDetailsElement.innerHTML = '<h2>Select a person to see details</h2>';
        recommendationsListElement.innerHTML = '';
    }

    function showModal(modalElement) {
        modalElement.classList.remove('hidden');
    }

    function hideModal(modalElement) {
        modalElement.classList.add('hidden');
        // Clear forms inside modal when hiding
        const form = modalElement.querySelector('form');
        if (form) form.reset();
        const errorMsg = modalElement.querySelector('.error-message');
        if (errorMsg) errorMsg.remove();
    }

    // --- API Call Functions ---
    async function apiCall(url, method = 'GET', body = null) {
        try {
            const options = {
                method,
                headers: body ? { 'Content-Type': 'application/json' } : {},
            };
            if (body) {
                options.body = JSON.stringify(body);
            }
            const response = await fetch(url, options);
            const responseData = await response.json(); // Attempt to parse JSON regardless of status

            if (!response.ok) {
                 // Use error message from response body if available
                const errorMsg = responseData.error || responseData.message || `HTTP error! status: ${response.status}`;
                throw new Error(errorMsg);
            }
            return responseData;
        } catch (error) {
            console.error(`API call failed for ${method} ${url}:`, error);
            displayStatus(null, `Operation failed: ${error.message}`, true);
            throw error; // Re-throw for caller to handle if needed
        }
    }

    // --- Fetch and Display Functions ---
    async function fetchPeopleList() {
        displayLoading(peopleListElement);
        peopleCache = {}; // Clear cache on full refresh
        allPeopleListCache = [];
        try {
            const people = await apiCall('/api/people');
            peopleListElement.innerHTML = ''; // Clear loading/error
            if (people.length === 0) {
                 peopleListElement.innerHTML = '<li>No contacts found. Add one or generate demo data.</li>';
                 return;
            }

            allPeopleListCache = people.sort((a, b) => {
                const nameA = `${a.lname}, ${a.fname}`.toLowerCase();
                const nameB = `${b.lname}, ${b.fname}`.toLowerCase();
                return nameA.localeCompare(nameB);
            });

            allPeopleListCache.forEach(person => {
                const li = document.createElement('li');
                li.textContent = `${person.lname}, ${person.fname}`;
                li.dataset.codename = person.codeName;
                li.addEventListener('click', () => {
                    if (currentSelectedCodename !== person.codeName) {
                        fetchPersonDetails(person.codeName);
                        // Recommendations fetched within fetchPersonDetails success
                        clearSelection();
                        li.classList.add('selected');
                        currentSelectedCodename = person.codeName;
                    }
                });
                peopleListElement.appendChild(li);
                peopleCache[person.codeName] = person; // Pre-fill cache with basic info
            });

        } catch (error) {
            peopleListElement.innerHTML = `<li class="error">Failed to load contacts.</li>`;
        }
    }

    async function fetchPersonDetails(codename) {
        personDetailsElement.innerHTML = '<h2>Loading details...</h2>';
        recommendationsListElement.innerHTML = ''; // Clear previous recommendations
        try {
            const person = await apiCall(`/api/people/${codename}`);
            peopleCache[codename] = person; // Update cache with full details
            renderPersonDetails(person);
            fetchRecommendations(codename); // Fetch recommendations after details are loaded

        } catch (error) {
             personDetailsElement.innerHTML = `<h2>Error loading details for ${codename}</h2>`;
        }
    }

    function renderPersonDetails(person) {
        let detailsHtml = `<h2>${person.fname} ${person.lname}</h2>`;
        detailsHtml += `<p><strong>Code Name:</strong> ${person.codeName}</p>`;
        detailsHtml += `<p><strong>Birthdate:</strong> ${person.bdate || 'N/A'}</p>`;
        detailsHtml += `<p><strong>Phone:</strong> ${person.phone || 'N/A'}</p>`;
        detailsHtml += `<p><strong>Email:</strong> ${person.email || 'N/A'}</p>`;

        if (person.additional_info && Object.keys(person.additional_info).length > 0) {
            detailsHtml += '<h3>Additional Info:</h3><ul>';
            for (const [key, value] of Object.entries(person.additional_info)) {
                detailsHtml += `<li><strong>${key}:</strong> ${value}</li>`;
            }
            detailsHtml += '</ul>';
        }

        detailsHtml += '<h3>Friends:</h3>';
        if (person.friends && person.friends.length > 0) {
            detailsHtml += '<ul>';
             person.friends.forEach(friendCode => {
                // Use combined cache (list + detail fetches)
                const friend = peopleCache[friendCode];
                const displayName = friend ? `${friend.lname}, ${friend.fname}` : friendCode;
                detailsHtml += `<li>${displayName} (${friendCode})</li>`;
             });
             detailsHtml += '</ul>';
        } else {
             detailsHtml += '<p>No friends connected yet.</p>';
        }

        // Add Action Buttons
        detailsHtml += `<div class="details-actions">`;
        detailsHtml += `<button class="action-button connect-btn" data-codename="${person.codeName}" data-name="${person.fname} ${person.lname}">Connect</button>`;
        detailsHtml += `<button class="action-button danger remove-btn" data-codename="${person.codeName}">Remove</button>`;
        detailsHtml += `</div>`;

        personDetailsElement.innerHTML = detailsHtml;

        // Add event listeners for the newly created buttons
        personDetailsElement.querySelector('.remove-btn').addEventListener('click', handleRemovePerson);
        personDetailsElement.querySelector('.connect-btn').addEventListener('click', handleConnectPerson);
    }

    async function fetchRecommendations(codename) {
        displayLoading(recommendationsListElement);
        try {
            const recommendations = await apiCall(`/api/recommendations/${codename}`);
            recommendationsListElement.innerHTML = ''; // Clear loading
            if (recommendations.length === 0) {
                 recommendationsListElement.innerHTML = '<li>No recommendations found.</li>';
                 return;
            }
            recommendationsListElement.innerHTML = '<h3>Recommendations</h3><ul>'; // Add header back
            recommendations.forEach(recCode => {
                const li = document.createElement('li');
                const recPerson = peopleCache[recCode];
                const displayName = recPerson ? `${recPerson.lname}, ${recPerson.fname}` : recCode;
                li.innerHTML = `
                    <span>${displayName} (${recCode})</span>
                    <button class="action-button recommend-connect-btn" data-connect-to="${recCode}">Connect</button>
                `;
                recommendationsListElement.appendChild(li);
            });
            recommendationsListElement.innerHTML += '</ul>';

            // Add event listeners after the HTML is in the DOM
            const connectButtons = recommendationsListElement.querySelectorAll('.recommend-connect-btn');
            connectButtons.forEach(button => {
                button.addEventListener('click', handleDirectConnect);
            });

        } catch (error) {
             recommendationsListElement.innerHTML = '<li class="error">Failed to load recommendations.</li>';
        }
    }

    // --- Event Handlers ---
    function handleAddPersonClick() {
        showModal(addPersonModal);
    }

    async function handleAddPersonSubmit(event) {
        event.preventDefault();
        const formData = new FormData(addPersonForm);
        const personData = Object.fromEntries(formData.entries());

        // Parse additional_info textarea into an object
        const additionalInfoText = personData.additional_info || '';
        personData.additional_info = {};
        additionalInfoText.split('\n').forEach(line => {
            const parts = line.split(':');
            if (parts.length >= 2) {
                const key = parts[0].trim();
                const value = parts.slice(1).join(':').trim();
                if (key) {
                    personData.additional_info[key] = value;
                }
            }
        });

        try {
            await apiCall('/api/people', 'POST', personData);
            hideModal(addPersonModal);
            displayStatus(null, 'Person added successfully.');
            fetchPeopleList(); // Refresh list
            clearSelection();
        } catch (error) {
            // Error already displayed by apiCall
        }
    }

    async function handleRemovePerson(event) {
        const codename = event.target.dataset.codename;
        if (!codename) return;

        if (confirm(`Are you sure you want to remove person ${codename}?`)) {
            try {
                await apiCall(`/api/people/${codename}`, 'DELETE');
                displayStatus(null, `Person ${codename} removed.`);
                fetchPeopleList(); // Refresh list
                clearSelection(); // Clear details panel
            } catch (error) {
                 // Error already displayed by apiCall
            }
        }
    }

    function handleConnectPerson(event) {
        const codeName1 = event.target.dataset.codename;
        const name1 = event.target.dataset.name;
        if (!codeName1) return;

        connectPerson1Name.textContent = name1;
        connectPerson1Code.value = codeName1;

        // Populate select dropdown, excluding person1
        connectPerson2Select.innerHTML = '<option value="">-- Select Person 2 --</option>';
        allPeopleListCache.forEach(p => {
            if (p.codeName !== codeName1) {
                const option = document.createElement('option');
                option.value = p.codeName;
                option.textContent = `${p.lname}, ${p.fname} (${p.codeName})`;
                connectPerson2Select.appendChild(option);
            }
        });

        showModal(connectPersonModal);
    }

    async function handleConfirmConnect() {
        const codeName1 = connectPerson1Code.value;
        const codeName2 = connectPerson2Select.value;

        if (!codeName1 || !codeName2) {
            displayStatus(null, 'Please select a person to connect with.', true);
            return;
        }

        try {
            await apiCall('/api/connect', 'POST', { codeName1, codeName2 });
            hideModal(connectPersonModal);
            displayStatus(null, `Connected ${codeName1} and ${codeName2}.`);
            // Refresh details for the currently selected person if they were involved
            if (currentSelectedCodename === codeName1 || currentSelectedCodename === codeName2) {
                 fetchPersonDetails(currentSelectedCodename);
            }
        } catch (error) {
            // Error handled by apiCall
        }
    }

    async function handleDirectConnect(event) {
        const button = event.target;
        const codeName2 = button.dataset.connectTo; // The person recommended
        const codeName1 = currentSelectedCodename; // The person currently being viewed

        // --- Add Logging --- 
        console.log('Direct Connect Clicked:');
        console.log('  - Current Selected (codeName1):', codeName1);
        console.log('  - Recommended Person (codeName2):', codeName2);
        console.log('  - Button element:', button);
        console.log('  - Button dataset:', button.dataset);
        // -----------------

        if (!codeName1 || !codeName2) {
            displayStatus(null, 'Cannot connect: Missing user information.', true);
            console.error('Connection failed - Missing info:', { codeName1, codeName2 });
            return;
        }

        // Disable button temporarily to prevent double clicks
        button.disabled = true;
        button.textContent = 'Connecting...';

        try {
            console.log('Sending API request to connect:', { codeName1, codeName2 });
            const response = await apiCall('/api/connect', 'POST', { codeName1, codeName2 });
            console.log('Connection successful:', response);
            displayStatus(null, `Connected ${codeName1} and ${codeName2}.`);
            // Refresh details for the currently selected person to show the new friend
            if (currentSelectedCodename) {
                 fetchPersonDetails(currentSelectedCodename);
            }
        } catch (error) {
            console.error('Connection failed with error:', error);
            // Error handled by apiCall
            // Re-enable button on failure
            button.disabled = false;
            button.textContent = 'Connect';
        }
    }

    async function handleGenerateData() {
        if (confirm('This will overwrite the current network with demo data. Are you sure?')) {
            try {
                await apiCall('/api/generate', 'POST');
                displayStatus(null, 'Demo data generated successfully.');
                fetchPeopleList();
                clearSelection();
            } catch (error) {
                 // Error handled by apiCall
            }
        }
    }

    // --- Event Listener Setup ---
    addPersonBtn.addEventListener('click', handleAddPersonClick);
    generateDataBtn.addEventListener('click', handleGenerateData);
    addPersonForm.addEventListener('submit', handleAddPersonSubmit);
    confirmConnectBtn.addEventListener('click', handleConfirmConnect);

    closeModalButtons.forEach(button => {
        button.addEventListener('click', () => {
            hideModal(button.closest('.modal'));
        });
    });

    // Close modal if clicking outside the content area
    window.addEventListener('click', (event) => {
        if (event.target.classList.contains('modal')) {
            hideModal(event.target);
        }
    });

    // --- Initial Load ---
    fetchPeopleList();
}); 