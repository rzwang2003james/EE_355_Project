#ifndef NETWORK_H
#define NETWORK_H

#include "person.h"
#include "date.h"
// You may need to add more libraries 
#include <stdlib.h>
#include <vector>

class Network{

    private:
        Person* head;
        Person* tail;
        int count; 
        Person* search(Person* searchEntry);
        void print_details_parsable_helper(Person* p);

    public:
        Network();
        Network(string fileName);
        ~Network();
        void printDB();
        void push_front(Person* newEntry);
        void push_back(Person* newEntry);
        bool remove(string fname, string lname);
        void showMenu();
        void loadDB(string filename);
        void saveDB(string filename);
        Person* search(string fname, string lname);
        Person* searchByCodeName(string codeName);
        void printAllSummaries();
        bool removeByCodeName(string codeName);
        void printPersonDetailsParsable(string codeName);
};

#endif