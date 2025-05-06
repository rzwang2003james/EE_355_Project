#include "network.h"
#include <limits>
#include "misc.h"
#include <fstream>
#include <dirent.h>
#include <cstring>
#include <sstream>
#include <map>

Network::Network(){
    head = NULL;
    tail = NULL;
    count = 0;
}


Network::Network(string fileName){
    // TODO: complete this method! - DONE
    // Implement it in one single line!
    // You may need to implement the load method before this!
    head = NULL;
    tail = NULL;
    count = 0;
    loadDB(fileName);
}

Network::~Network(){ 
    // TODO: Complete this method - DONE
    Person* current = head;
    while (current != NULL) {
        Person* next = current->next;
        delete current;
        current = next;
    }
    head = NULL;
    tail = NULL;
    count = 0;
}

Person* Network::search(Person* searchEntry){
    // Searches the Network for searchEntry
    // if found, returns a pointer to it, else returns NULL
    // TODO: Complete this method - DONE
    Person* current = head;
    while (current != NULL) {
        if (*current == *searchEntry)
            return current;
        current = current->next;
    }
    return NULL;
}


Person* Network::search(string fname, string lname){
    // New == for Person, only based on fname and lname
    // if found, returns a pointer to it, else returns NULL
    // TODO: Complete this method - DONE
    // Note: two ways to implement this, 1st making a new Person with fname and lname and and using search(Person*), 2nd using fname and lname directly. 
    Person* current = head;
    while (current != NULL) {
        if (current->f_name == fname && current->l_name == lname)
            return current;
        current = current->next;
    }
    return NULL;
}




void Network::loadDB(string filename){
    // Clear the current network
    Person* current = head;
    while (current != NULL) {
        Person* next = current->next;
        delete current;
        current = next;
    }
    head = NULL;
    tail = NULL;
    count = 0;
    
    ifstream infile(filename.c_str());
    if (!infile) {
        cerr << "Error: Could not open file " << filename << endl;
        return;
    }
    
    // Map to store friend codes read from file, key is the person's code, value is list of friend codes
    map<string, vector<string>> friend_codes_map;
    // Map to store pointers to created persons, key is the person's code
    map<string, Person*> person_map;

    string line;
    while (getline(infile, line)) {
        string f_name = line; // First line is fname
        if (!getline(infile, line)) break; // Check EOF
        string l_name = line;
        if (!getline(infile, line)) break; // Check EOF
        string bdate = line;
        if (!getline(infile, line)) break; // Check EOF
        string phone_str = line;
        if (!getline(infile, line)) break; // Check EOF
        string email_str = line;

        Person* newPerson = new Person(f_name, l_name, bdate, email_str, phone_str);
        string personCode = codeName(f_name, l_name);

        // Read additional info (Phase 3)
        newPerson->additional_info.clear(); // Ensure map is empty
        while (getline(infile, line) && line != "---INFO_END---") {
             if (line == "--------------------" || line == "===================") {
                // Old separator or end of person before info end marker
                infile.seekg(-(line.length() + 1), std::ios_base::cur); // Put line back
                break;
             }
             size_t colon_pos = line.find(':');
             if (colon_pos != string::npos) {
                 string key = line.substr(0, colon_pos);
                 string value = line.substr(colon_pos + 1);
                 // Basic trim
                 key.erase(0, key.find_first_not_of(" \t"));
                 key.erase(key.find_last_not_of(" \t") + 1);
                 value.erase(0, value.find_first_not_of(" \t"));
                 value.erase(value.find_last_not_of(" \t") + 1);
                 if (!key.empty()) {
                     newPerson->add_info(key, value); // Use the method to add info
                 }
             } else {
                 // If it's not key:value and not the end marker, assume it's a friend code (compatibility)
                 friend_codes_map[personCode].push_back(line);
             }
        }

        // Read friend codes (after additional info)
        while (getline(infile, line)) {
            if (line == "--------------------" || line == "===================") {
                break; // End of this person's entry
            }
            friend_codes_map[personCode].push_back(line);
        }

        // Add person to network and map
        push_back(newPerson);
        person_map[personCode] = newPerson;
    }

    infile.close();

    // Now set up friendships using the maps
    for (map<string, vector<string>>::iterator it = friend_codes_map.begin(); it != friend_codes_map.end(); ++it) {
        string personCode = it->first;
        vector<string>& codes_of_friends = it->second;

        if (person_map.count(personCode)) { // Check if person exists in our map
            Person* current_person = person_map[personCode];
            for (size_t i = 0; i < codes_of_friends.size(); ++i) {
                string friendCode = codes_of_friends[i];
                if (person_map.count(friendCode)) { // Check if friend exists
                    Person* friend_person = person_map[friendCode];
                    // Avoid adding self as friend or duplicate friends (optional check)
                    bool already_friend = false;
                    for(size_t j=0; j < current_person->myfriends.size(); ++j) {
                       if (current_person->myfriends[j] == friend_person) {
                          already_friend = true;
                          break;
                       }
                    }
                    if (current_person != friend_person && !already_friend) {
                        current_person->makeFriend(friend_person);
                    }
                } else {
                    cerr << "Warning: Friend code '" << friendCode << "' for person '" << personCode << "' not found in network." << endl;
                }
            }
        }
    }
}

void Network::saveDB(string filename){
    ofstream outfile(filename.c_str());
    if (!outfile) {
        cerr << "Error: Could not open file " << filename << " for writing" << endl;
        return;
    }
    
    Person* current = head;
    while (current != NULL) {
        outfile << current->f_name << endl;
        outfile << current->l_name << endl;
        outfile << current->birthdate->get_date("MM/DD/YYYY") << endl;
        outfile << current->phone->get_contact("full") << endl;
        outfile << current->email->get_contact("full") << endl;
        
        // Write Additional Info (Phase 3)
        const map<string, string>& info = current->get_all_info();
        for (map<string, string>::const_iterator it = info.begin(); it != info.end(); ++it) {
            outfile << it->first << ":" << it->second << endl;
        }
        outfile << "---INFO_END---" << endl; // Marker for end of additional info

        // Write friend codes
        for (size_t i = 0; i < current->myfriends.size(); i++) {
            string code = codeName(current->myfriends[i]->f_name, current->myfriends[i]->l_name);
            outfile << code << endl;
        }
        
        // Separator
        if (current->next != NULL) {
            outfile << "--------------------" << endl;
        }
        
        current = current->next;
    }
    
    outfile.close();
}


void Network::printDB(){
    // Leave me alone! I know how to print! 
    // Note: Notice that we don't need to update this even after adding to Personattributes
    // This is a feature of OOP, classes are supposed to take care of themselves!
    cout << "Number of people: " << count << endl;
    cout << "------------------------------" << endl;
    Person* ptr = head;
    while(ptr != NULL){
        ptr->print_person();
        cout << "------------------------------" << endl;
        ptr = ptr->next;
    }
}



void Network::push_front(Person* newEntry){
    newEntry->prev = NULL;
    newEntry->next = head;

    if (head != NULL)
        head->prev = newEntry;
    else
        tail = newEntry;
    
    head = newEntry;
    count++;
}


void Network::push_back(Person* newEntry){
    // Adds a new Person (newEntry) to the back of LL
    // TODO: Complete this method - DONE
    newEntry->next = NULL;
    newEntry->prev = tail;
    
    if (tail != NULL)
        tail->next = newEntry;
    else
        head = newEntry;
    
    tail = newEntry;
    count++;
}


bool Network::remove(string fname, string lname){
    // TODO: Complete this method - DONE
    Person* target = search(fname, lname);
    if (target == NULL)
        return false;
    
    // Update pointers
    if (target->prev != NULL)
        target->prev->next = target->next;
    else
        head = target->next;
    
    if (target->next != NULL)
        target->next->prev = target->prev;
    else
        tail = target->prev;
    
    // Delete the person and decrement count
    delete target;
    count--;
    return true;
}


void Network::showMenu(){
    // TODO: Complete this method!
    // All the prompts are given to you, 
    // You should add code before, between and after prompts!

    int opt;
    while(1){
        cout << "\033[2J\033[1;1H";
        printMe("banner"); // from misc library

        cout << "Select from below: \n";
        cout << "1. Save network database \n";
        cout << "2. Load network database \n";
        cout << "3. Add a new person \n";
        cout << "4. Remove a person \n";
        cout << "5. Print people with last name  \n";
        cout << "6. Connect \n";
        cout << "7. Display sorted friends \n";
        cout << "\nSelect an option ... ";
        
        if (cin >> opt) {
            cin.clear();
            cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        } else {
            cin.clear();
            cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            cout << "Wrong option! " << endl;
            return;
        }
        
        // You may need these variables! Add more if you want!
        string fname, lname, fileName, bdate;
        cout << "\033[2J\033[1;1H";

        if (opt == 1){
            // TODO: Complete me! - DONE
            cout << "Saving network database \n";
            cout << "Enter the name of the save file: ";
            // Save the network database into the file with the given name,
            // with each person saved in the format the save as printing out the person info,
            // and people are delimited similar to "networkDB.txt" format
            getline(cin, fileName);
            saveDB(fileName);
            cout << "Network saved in " << fileName << endl;
        }
        else if (opt == 2){
            // TODO: Complete me! - DONE
            cout << "Loading network database \n";
            // TODO: print all the files in this same directory that have "networkDB.txt" format
            // print format: one filename one line.
            // This step just shows all the available .txt file to load.
            DIR *dir;
            struct dirent *ent;
            if ((dir = opendir(".")) != NULL) {
                while ((ent = readdir(dir)) != NULL) {
                    string file_name = ent->d_name;
                    if (file_name.length() >= 4 && 
                        file_name.substr(file_name.length() - 4) == ".txt") {
                        cout << file_name << endl;
                    }
                }
                closedir(dir);
            }
            
            cout << "Enter the name of the load file: "; 
            getline(cin, fileName);
            
            // Check if file exists
            ifstream test(fileName.c_str());
            if (!test) {
                // If file with name FILENAME does not exist: 
                cout << "File " << fileName << " does not exist!" << endl;
            } else {
                test.close();
                loadDB(fileName);
                // If file is loaded successfully, also print the count of people in it: 
                cout << "Network loaded from " << fileName << " with " << count << " people \n";
            }
        }
        else if (opt == 3){
            // TODO: Complete me! - DONE
            // TODO: use push_front, and not push_back 
            // Add a new Person ONLY if it does not exists!
            cout << "Adding a new person \n";
            Person* newPerson = new Person();
            
            // Check if person already exists
            if (search(newPerson) != NULL) {
                cout << "Person already exists in the network!" << endl;
                delete newPerson;
            } else {
                push_front(newPerson);
                cout << "Person added to the network!" << endl;
            }
        }
        else if (opt == 4){
            // TODO: Complete me! - DONE
            // if found, cout << "Remove Successful! \n";
            // if not found: cout << "Person not found! \n";
            cout << "Removing a person \n";
            cout << "First name: ";
            getline(cin, fname);
            cout << "Last name: ";
            getline(cin, lname);
            
            if (remove(fname, lname))
                cout << "Remove Successful! \n";
            else
                cout << "Person not found! \n";
        }
        else if (opt == 5){
            // TODO: Complete me! - DONE
            // print the people with the given last name
            // if not found: cout << "Person not found! \n";
            cout << "Print people with last name \n";
            cout << "Last name: ";
            getline(cin, lname);
            
            bool found = false;
            Person* current = head;
            while (current != NULL) {
                if (current->l_name == lname) {
                    current->print_person();
                    cout << "------------------------------" << endl;
                    found = true;
                }
                current = current->next;
            }
            
            if (!found)
                cout << "Person not found! \n";
        }
        else if (opt == 6){
            // Connect functionality - Make friends between two people
            cout << "Make friends" << endl;
            
            // Get info for first person
            cout << "Person 1" << endl;
            cout << "First Name: ";
            getline(cin, fname);
            cout << "Last Name: ";
            getline(cin, lname);
            
            // Search for first person
            Person* person1 = search(fname, lname);
            if (person1 == NULL) {
                cout << "Person not found" << endl;
            } else {
                // Get info for second person
                cout << "Person 2" << endl;
                cout << "First Name: ";
                getline(cin, fname);
                cout << "Last Name: ";
                getline(cin, lname);
                
                // Search for second person
                Person* person2 = search(fname, lname);
                if (person2 == NULL) {
                    cout << "Person not found" << endl;
                } else {
                    // Print information for both people before connection
                    cout << endl;
                    person1->print_person();
                    cout << endl;
                    person2->print_person();
                    
                    // Make them friends with each other
                    person1->makeFriend(person2);
                    person2->makeFriend(person1);
                    
                    cout << "\nConnection successful!" << endl;
                    
                    // Print the friends in sorted order
                    cout << "\nShowing sorted friends list for " << person1->f_name << " " << person1->l_name << ":" << endl;
                    person1->print_friends();
                    
                    cout << "\nShowing sorted friends list for " << person2->f_name << " " << person2->l_name << ":" << endl;
                    person2->print_friends();
                }
            }
        }
        else if (opt == 7) {
            // TODO: Complete this method!
            // Implement the display sorted friends functionality
            cout << "Display sorted friends" << endl;
            cout << "Enter the first name of the person: ";
            getline(cin, fname);
            cout << "Enter the last name of the person: ";
            getline(cin, lname);
            
            Person* person = search(fname, lname);
            if (person == NULL) {
                cout << "Person not found!" << endl;
            } else {
                cout << "\nShowing sorted friends list for " << person->f_name << " " << person->l_name << ":" << endl;
                person->print_friends();
            }
        }
        else
            cout << "Nothing matched!\n";
        
        cin.clear();
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        cout << "\n\nPress Enter key to go back to main menu ... ";
        string temp;
        std::getline (std::cin, temp);
        cout << "\033[2J\033[1;1H";
    }
}

// Add the implementation for searchByCodeName
Person* Network::searchByCodeName(string codeName) {
    Person* current = head;
    while (current != NULL) {
        if (::codeName(current->f_name, current->l_name) == codeName) {
            return current;
        }
        current = current->next;
    }
    return NULL; 
}

// Implementation for printAllSummaries
void Network::printAllSummaries() {
    Person* current = head;
    while(current != nullptr) {
        cout << codeName(current->f_name, current->l_name) << ":"
             << current->f_name << ":" 
             << current->l_name << endl;
        current = current->next;
    }
}

// Implementation for removeByCodeName
bool Network::removeByCodeName(string codeName) {
    Person* target = searchByCodeName(codeName);
    if (!target) {
        return false; // Person not found
    }

    // Step 1: Remove target from all other people's friend lists
    Person* current = head;
    while (current != nullptr) {
        if (current != target) { // Don't try to remove from self
             // Iterate through current person's friends
             vector<Person*>::iterator it = current->myfriends.begin();
             while (it != current->myfriends.end()) {
                 if (*it == target) {
                     it = current->myfriends.erase(it); // Erase and update iterator
                 } else {
                     ++it;
                 }
             }
        }
        current = current->next;
    }

    // Step 2: Unlink target from the doubly linked list
    if (target->prev) {
        target->prev->next = target->next;
    } else {
        head = target->next; // Target was head
    }
    if (target->next) {
        target->next->prev = target->prev;
    } else {
        tail = target->prev; // Target was tail
    }

    // Step 3: Delete the target object
    delete target;
    count--;
    return true;
}

// Implementation for the public printPersonDetailsParsable method
void Network::printPersonDetailsParsable(string codeName) {
    Person* p = searchByCodeName(codeName);
    if (p) {
        print_details_parsable_helper(p);
    } else {
        // Error handled by caller in main, maybe print info here?
        // cerr << "INFO: Person with codename '" << codeName << "' not found." << endl;
         // No output needed here, main handles not found.
    }
}

// Implementation for the private print_details_parsable_helper method
// (Moved from test_network.cpp)
void Network::print_details_parsable_helper(Person* p) {
     if (!p) return;
     // Access private members directly because Network is a friend
    cout << "fname:" << p->f_name << endl;
    cout << "lname:" << p->l_name << endl;
    cout << "bdate:" << p->birthdate->get_date("MM/DD/YYYY") << endl;
    if (p->phone) cout << "phone:" << p->phone->get_contact("full") << endl; 
    else cout << "phone:" << endl;
    if (p->email) cout << "email:" << p->email->get_contact("full") << endl;
    else cout << "email:" << endl;

    const map<string, string>& info = p->get_all_info(); // Use public getter for info map
    for (map<string, string>::const_iterator it = info.begin(); it != info.end(); ++it) {
        cout << "info_" << it->first << ":" << it->second << endl;
    }

    // Access private myfriends vector directly
    for (size_t i = 0; i < p->myfriends.size(); ++i) {
        if (p->myfriends[i]) { 
           // Access friend's private names directly
           cout << "friend:" << ::codeName(p->myfriends[i]->f_name, p->myfriends[i]->l_name) << endl;
        }
    }
    cout << "---DETAILS_END---" << endl;
}

