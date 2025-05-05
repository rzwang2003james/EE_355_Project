#include "date.h"
#include <sstream>
#include <vector>

Date::Date() {
    day = 1;
    month = 1;
    year = 2000;
}

Date::Date(string date_string) {
    set_date(date_string);
}

Date::~Date() {
    // Nothing to deallocate
}

void Date::set_date() {
    string date_string;
    cout << "Enter date (M/D/YYYY): ";
    getline(cin, date_string);
    set_date(date_string);
}

void Date::set_date(string date_string) {
    // Parse date in format M/D/YYYY or MM/DD/YYYY
    stringstream ss(date_string);
    char delimiter;
    ss >> month >> delimiter >> day >> delimiter >> year;
}

void Date::print_date(string format) {
    if (format == "MM/DD/YYYY") {
        cout << month << "/" << day << "/" << year << endl;
    } 
    else if (format == "Month D, YYYY") {
        string month_names[] = {"January", "February", "March", "April", "May", "June", 
                               "July", "August", "September", "October", "November", "December"};
        cout << month_names[month-1] << " " << day << ", " << year << endl;
    }
}

string Date::get_date(string format) const {
    stringstream ss;
    if (format == "MM/DD/YYYY") {
        ss << month << "/" << day << "/" << year;
    } 
    else if (format == "Month D, YYYY") {
        string month_names[] = {"January", "February", "March", "April", "May", "June", 
                               "July", "August", "September", "October", "November", "December"};
        ss << month_names[month-1] << " " << day << ", " << year;
    }
    return ss.str();
}

bool Date::operator==(const Date& rhs) {
    return (day == rhs.day && month == rhs.month && year == rhs.year);
}

bool Date::operator!=(const Date& rhs) {
    return !(*this == rhs);
} 