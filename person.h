#ifndef PERSON_H
#define PERSON_H

#include "date.h"
#include "contact.h"
#include <fstream>
#include <string>
#include <vector>
#include <map>

class Person{
    friend class Network;

private:
	string f_name;
	string l_name;
	Date *birthdate;
    Email *email;
    Phone *phone;
    // the following to attributes are used in the linked list.
    Person* next;
    Person* prev;
    // vector to store friends
    std::vector<Person*> myfriends;
    // map to store additional info
    std::map<std::string, std::string> additional_info;

public: 
    Person();
    ~Person();
    Person(string filename);
    Person(string f_name, string l_name, string b_date, string email, string phone);
	void print_person();
	void set_person();
	void set_person(string filename);
    bool operator==(const Person& rhs);
    bool operator!=(const Person& rhs);
    void makeFriend(Person* newFriend);
    void print_friends();
    // Methods for additional info
    void add_info(const std::string& key, const std::string& value);
    std::string get_info(const std::string& key);
    const std::map<std::string, std::string>& get_all_info() const;
};


#endif