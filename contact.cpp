#include "contact.h"
// TODO: Add needed libraries! - DONE
#include <sstream>
#include <algorithm>

Email::Email(string type, string email_addr){
    // TODO: Complete me! - DONE
    this->type = type;
    this->email_addr = email_addr;
}


void Email::set_contact(){
    // TODO: Do not change the prompts! - DONE
	
    cout << "Enter the type of email address: ";
    // some code here
    getline(cin, type);
    cout << "Enter email address: ";
    // some code here
    getline(cin, email_addr);
}


string Email::get_contact(string style){
    // Note: We have default argument in declaration and not in definition!
    if (style=="full")
	    return "(" + type + ") :" + email_addr;
    else 
        return email_addr;
}


void Email::print(){
    // Note: get_contact is called with default argument
	cout << "Email " << get_contact() << endl;
}


Phone::Phone(string type, string num){
    // TODO: It is possible that num includes "-" or not, manage it! - DONE
    // TODO: Complete this method! - DONE
    // Note: We don't want to use C++11! stol is not valid!
    
    this->type = type;
    
    // Remove any existing hyphens from the phone number
    string clean_num = num;
    clean_num.erase(remove(clean_num.begin(), clean_num.end(), '-'), clean_num.end());
    
    // Format the phone number with hyphens (XXX-XXX-XXXX)
    if (clean_num.length() == 10) {
        phone_num = clean_num.substr(0, 3) + "-" + clean_num.substr(3, 3) + "-" + clean_num.substr(6, 4);
    } else {
        // If the number is not 10 digits, store it as is
        phone_num = num;
    }
}


void Phone::set_contact(){
    // TODO: Complete this method - DONE
    // Use the same prompts as given!
	cout << "Enter the type of phone number: ";
	getline(cin, type);
	cout << "Enter the phone number: ";
	string num;
	getline(cin, num);
	
	// Remove any existing hyphens
    string clean_num = num;
    clean_num.erase(remove(clean_num.begin(), clean_num.end(), '-'), clean_num.end());
    
    // Format the phone number with hyphens (XXX-XXX-XXXX)
    if (clean_num.length() == 10) {
        phone_num = clean_num.substr(0, 3) + "-" + clean_num.substr(3, 3) + "-" + clean_num.substr(6, 4);
    } else {
        // If the number is not 10 digits, store it as is
        phone_num = num;
    }
}


string Phone::get_contact(string style){
    // TODO: Complete this method, get hint from Email - DONE
    if (style=="full")
	    return "(" + type + ") :" + phone_num;
    else 
        return phone_num;
}


void Phone::print(){
    // Note: get_contact is called with default argument
	cout << "Phone " << get_contact() << endl;
}

