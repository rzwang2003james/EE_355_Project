#ifndef DATE_H
#define DATE_H

#include <iostream>
#include <string>
using namespace std;

class Network; // Forward declaration for friendship

class Date {
    friend class Network; // Allow Network to access private members
    
private:
    int day;
    int month;
    int year;
    
public:
    Date();
    Date(string date_string);
    ~Date();
    void set_date();
    void set_date(string date_string);
    void print_date(string format = "MM/DD/YYYY");
    string get_date(string format = "MM/DD/YYYY") const; // Add method to get date as string
    bool operator==(const Date& rhs);
    bool operator!=(const Date& rhs);
};

#endif 