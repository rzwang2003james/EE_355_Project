#include "person.h"

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
    
    infile.close();
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


void Person::print_person(){
    // Already implemented for you! Do not change!
	cout << l_name <<", " << f_name << endl;
	birthdate->print_date("Month D, YYYY");
    phone->print();
    email->print();
}
