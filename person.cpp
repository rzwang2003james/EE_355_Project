#include "person.h"
#include <algorithm>
#include <iostream>
#include <map>

Person::Person(){
    // I'm already done! 
    set_person();
}


Person::~Person(){
    delete birthdate;
    // TODO: complete the method! - DONE
    delete email;
    delete phone;
}


Person::Person(string f_name, string l_name, string b_date, string email_str, string phone_str){
    // TODO: Complete this method! - DONE
    // phone and email strings are in full version
    this->f_name = f_name;
    this->l_name = l_name;
    birthdate = new Date(b_date);
    
    // Parse email string to extract type and address
    // Format is like: "(Gmail) something@email.com"
    string email_type = email_str.substr(1, email_str.find(")") - 1);
    string email_addr = email_str.substr(email_str.find(") ") + 2);
    email = new Email(email_type, email_addr);
    
    // Parse phone string to extract type and number
    // Format is like: "(Office) 310-291-7482"
    string phone_type = phone_str.substr(1, phone_str.find(")") - 1);
    string phone_num = phone_str.substr(phone_str.find(") ") + 2);
    phone = new Phone(phone_type, phone_num);
    
    // Initialize linked list pointers
    next = NULL;
    prev = NULL;
}


Person::Person(string filename){
    set_person(filename);
}


void Person::set_person(){
    // prompts for the information of the user from the terminal
    // first/last name can have spaces!
    // date format must be "M/D/YYYY"
    // We are sure user enters info in correct format.
    // TODO: complete this method! - DONE
    
    string temp;
    string type;

    cout << "First Name: ";
    // pay attention to how we read first name, as it can have spaces!
    std::getline(std::cin,f_name);

	cout << "Last Name: ";
    std::getline(std::cin,l_name);

    cout << "Birthdate (M/D/YYYY): ";
    std::getline(std::cin,temp);
    // pay attention to how we passed argument to the constructor of a new object created dynamically using new command
    birthdate = new Date(temp); 

    // Create a new Email object and set its contact info
    email = new Email("", "");
    email->set_contact();

    // Create a new Phone object and set its contact info
    phone = new Phone("", "");
    phone->set_contact();
    
    // Prompt for additional information (optional)
    cout << "Add additional info (key:value)? (y/n): ";
    getline(std::cin, temp);
    while (temp == "y" || temp == "Y") {
        string key, value;
        cout << "  Enter key: ";
        getline(std::cin, key);
        cout << "  Enter value: ";
        getline(std::cin, value);
        if (!key.empty()) { // Avoid empty keys
            additional_info[key] = value;
        }
        cout << "Add more info? (y/n): ";
        getline(std::cin, temp);
    }
    
    // Initialize linked list pointers
    next = NULL;
    prev = NULL;
}


void Person::set_person(string filename){
    // reads a Person from a file
    // Look at person_template files as examples.     
    // Phone number in files can have '-' or not.
    // TODO: Complete this method! - DONE
    
    ifstream infile(filename.c_str());
    if (!infile) {
        cerr << "Error: Could not open file " << filename << endl;
        return;
    }
    
    // Read first name, last name, and birthdate
    getline(infile, f_name);
    getline(infile, l_name);
    
    string bdate;
    getline(infile, bdate);
    birthdate = new Date(bdate);
    
    // Read phone and email
    string phone_str, email_str;
    getline(infile, phone_str);
    getline(infile, email_str);
    
    // Parse phone string to get type and number
    string phone_type = phone_str.substr(1, phone_str.find(")") - 1);
    string phone_num = phone_str.substr(phone_str.find(") ") + 2);
    phone = new Phone(phone_type, phone_num);
    
    // Parse email string to get type and address
    string email_type = email_str.substr(1, email_str.find(")") - 1);
    string email_addr = email_str.substr(email_str.find(") ") + 2);
    email = new Email(email_type, email_addr);
    
    // Initialize linked list pointers
    next = NULL;
    prev = NULL;
    
    // Clear existing info before loading (in case object is reused)
    additional_info.clear(); 

    // Read additional info (Phase 3)
    string line;
    while (getline(infile, line) && line != "---INFO_END---") {
        size_t colon_pos = line.find(':');
        if (colon_pos != string::npos) {
            string key = line.substr(0, colon_pos);
            string value = line.substr(colon_pos + 1);
            // Trim potential leading/trailing whitespace (optional but good practice)
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            if (!key.empty()) {
                 additional_info[key] = value;
            }
        } else if (line == "--------------------" || line == "===================") {
            // This handles the case where the old format separator is encountered
            // before any additional info or the end marker. We should put the line back
            // so the main loop in Network::loadDB can read it.
            // Seek back to the beginning of the line we just read.
            infile.seekg(-(line.length() + 1), std::ios_base::cur); // +1 for newline
            break; // Stop reading info for this person
        }
    }
    
    infile.close(); // Close file here if loading single person, otherwise Network::loadDB handles it
}


bool Person::operator==(const Person& rhs){
    // TODO: Complete this method! - DONE
    // Note: you should check first name, last name and birthday between two persons
    // refer to bool Date::operator==(const Date& rhs)
    return (f_name == rhs.f_name && 
            l_name == rhs.l_name && 
            *birthdate == *(rhs.birthdate));
}

bool Person::operator!=(const Person& rhs){ 
    // TODO: Complete this method! - DONE
    return !(*this == rhs);
}


void Person::makeFriend(Person* newFriend) {
    // Add the new friend to the myfriends vector
    myfriends.push_back(newFriend);
}

void Person::print_person(){
    // Print basic person information
	cout << l_name <<", " << f_name << endl;
	birthdate->print_date("Month D, YYYY");
    phone->print();
    email->print();
    
    // Print Additional Info (Phase 3)
    if (!additional_info.empty()) {
        cout << "Additional Info:" << endl;
        for (std::map<string, string>::const_iterator it = additional_info.begin(); it != additional_info.end(); ++it) {
            cout << "  " << it->first << ": " << it->second << endl;
        }
    }

    // Print friends information if available
    if (!myfriends.empty()) {
        cout << "Friends:" << endl;
        // Keep using codeName logic from misc.h/cpp if it exists, otherwise use placeholder
        // Assuming codeName function exists globally or via misc.h
        // #include "misc.h" // Make sure this is included if needed
        for (size_t i = 0; i < myfriends.size(); i++) {
            string friend_code = myfriends[i]->f_name + myfriends[i]->l_name; // Basic code if misc::codeName unavailable
            // TODO: Replace above line with call to actual codeName function if available
            // string friend_code = codeName(myfriends[i]->f_name, myfriends[i]->l_name);
            cout << "  " << friend_code << " (" << myfriends[i]->f_name << " " << myfriends[i]->l_name << ")" << endl;
        }
    }
}

void Person::print_friends() {
    // Sort friends based on their code names in alphabetical order
    vector<pair<string, Person*>> sorted_friends;
    
    // Create pairs of code and friend pointer for sorting
    for (size_t i = 0; i < myfriends.size(); i++) {
        string code = myfriends[i]->f_name + myfriends[i]->l_name;
        transform(code.begin(), code.end(), code.begin(), ::tolower);
        code.erase(remove(code.begin(), code.end(), ' '), code.end());
        sorted_friends.push_back(make_pair(code, myfriends[i]));
    }
    
    // Sort based on the first letter of the code, and if tied, by the second letter
    sort(sorted_friends.begin(), sorted_friends.end(), 
        [](const pair<string, Person*>& a, const pair<string, Person*>& b) -> bool {
            if (a.first.empty() || b.first.empty() || a.first[0] != b.first[0]) {
                return a.first < b.first;
            }
            // Sort by second letter if first letters are the same
            if (a.first.length() > 1 && b.first.length() > 1) {
                return a.first[1] < b.first[1];
            }
            return a.first < b.first;
        });
    
    // Print the sorted friends
    cout << f_name << ", " << l_name << endl;
    cout << "--------------------------------" << endl;
    for (size_t i = 0; i < sorted_friends.size(); i++) {
        cout << sorted_friends[i].second->f_name << ", " << sorted_friends[i].second->l_name << endl;
    }
}

// Implementation of new methods for additional info
void Person::add_info(const std::string& key, const std::string& value) {
    if (!key.empty()) { // Don't allow empty keys
        additional_info[key] = value;
    }
}

std::string Person::get_info(const std::string& key) {
    std::map<std::string, std::string>::iterator it = additional_info.find(key);
    if (it != additional_info.end()) {
        return it->second;
    } else {
        return ""; // Return empty string if key not found
    }
}

const std::map<std::string, std::string>& Person::get_all_info() const {
    return additional_info;
}
