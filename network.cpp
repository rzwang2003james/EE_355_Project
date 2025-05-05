#include "network.h"
#include <limits>
#include "misc.h"
#include <fstream>
#include <dirent.h>
#include <cstring>
#include <sstream>

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
    
    string line;
    string f_name, l_name, bdate, phone_str, email_str;
    int line_count = 0;
    vector<vector<string>> all_friend_codes;
    vector<string> friend_codes;
    
    while (getline(infile, line)) {
        // Skip separator lines
        if (line.find("----") != string::npos) {
            // Add friend codes for this person
            all_friend_codes.push_back(friend_codes);
            friend_codes.clear();
            line_count = 0;
            continue;
        }
        
        if (line_count == 0)
            f_name = line;
        else if (line_count == 1)
            l_name = line;
        else if (line_count == 2)
            bdate = line;
        else if (line_count == 3)
            phone_str = line;
        else if (line_count == 4) {
            email_str = line;
            
            // Create new person and add to network
            Person* newPerson = new Person(f_name, l_name, bdate, email_str, phone_str);
            push_back(newPerson);
            
            // Reset for reading friend codes
            line_count = 5;
        }
        else if (line_count >= 5) {
            // This line contains a friend code
            friend_codes.push_back(line);
        }
        
        line_count++;
    }
    
    // Add the last person's friend codes
    if (!friend_codes.empty()) {
        all_friend_codes.push_back(friend_codes);
    }
    
    // Now set up friendships
    Person* p = head;
    int person_idx = 0;
    while (p != NULL && person_idx < all_friend_codes.size()) {
        for (string code : all_friend_codes[person_idx]) {
            // Find the friend with this code
            Person* temp = head;
            while (temp != NULL) {
                if (codeName(temp->f_name, temp->l_name) == code) {
                    p->makeFriend(temp);
                    break;
                }
                temp = temp->next;
            }
        }
        
        p = p->next;
        person_idx++;
    }
    
    infile.close();
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
        
        // Use the get_date method to get the formatted date
        outfile << current->birthdate->get_date("MM/DD/YYYY") << endl;
        
        // Output phone and email
        outfile << current->phone->get_contact("full") << endl;
        outfile << current->email->get_contact("full") << endl;
        
        // Write friend codes
        for (size_t i = 0; i < current->myfriends.size(); i++) {
            string code = codeName(current->myfriends[i]->f_name, current->myfriends[i]->l_name);
            outfile << code << endl;
        }
        
        if (current->next != NULL)
            outfile << "--------------------" << endl;
        
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

