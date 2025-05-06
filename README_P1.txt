====================================
EE 355 PROJECT - PHASE 1 README
====================================

This document provides an explanation of the implementation of Phase 1 of the EE 355 Project: A contact management system that resembles a simple social network (Trojan-Book).

====================================
IMPLEMENTED CLASSES
====================================

1. Contact Class (contact.h, contact.cpp)
   - Abstract base class with two derived classes: Email and Phone
   - Implementation includes virtual methods for setting contact info, getting contact info, and printing
   - 'type' attribute made protected to allow derived classes to access it
   - Email class stores and manages email address information
   - Phone class handles phone numbers with proper formatting (XXX-XXX-XXXX)
   
2. Date Class (date.h, date.cpp)
   - Handles date information with different format options
   - Implemented parsing of M/D/YYYY format
   - Added print_date method with formatting options: MM/DD/YYYY and Month D, YYYY
   - Added comparison operators (==, !=) for date equality checking
   
3. Person Class (person.h, person.cpp)
   - Stores personal information: first name, last name, birthdate, email, and phone
   - Implements constructors, including one that reads from file
   - Added support for doubly linked list pointers (next, prev)
   - Implemented comparison operators for person equality
   - Proper memory management in destructor for dynamic objects
   
4. Network Class (network.h, network.cpp)
   - Manages a doubly linked list of Person objects
   - Implemented load and save functionality for database files
   - Added search methods to find people by name or by Person object
   - Push_front and push_back methods for adding people to the network
   - Remove method to delete people from the network
   - ShowMenu method to provide an interactive interface

====================================
KEY DESIGN DECISIONS
====================================

1. Memory Management
   - All dynamically allocated objects (Date, Email, Phone) are properly deleted in destructors
   - Network destructor properly cleans up all Person objects in the linked list

2. Data Storage
   - Phone numbers stored as strings with formatting applied
   - Used protected inheritance for the 'type' attribute in Contact class

3. File I/O
   - Implemented robust file reading/writing with proper error handling
   - Supports both single person files and network database files

4. User Interface
   - Added clear prompts and error messages
   - Implemented all menu options with proper validation

====================================
TESTING
====================================

The implementation has been tested with:

1. Creating and manipulating individual Person objects
2. Loading and saving from/to files
3. Adding and removing people from the network
4. Searching for people by name
5. Testing equality between Person objects

====================================
COMPILATION INSTRUCTIONS
====================================

To compile the test_person_eq.cpp:
g++ -o test_person date.cpp contact.cpp person.cpp test_person_eq.cpp

To compile the test_network.cpp:
g++ -o test_network date.cpp contact.cpp person.cpp network.cpp misc.cpp test_network.cpp

====================================
FUTURE ENHANCEMENTS
====================================

Possible improvements for future phases could include:
1. Better input validation
2. More robust error handling
3. Supporting multiple contact information per person
4. Implementing additional search and sort options
5. Graphical user interface 