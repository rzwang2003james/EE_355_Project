#include "network.h"
#include "misc.h"
#include <iostream>
using namespace std;

int main(){
    cout << "TrojanBook - Phase 2 Testing" << endl;
    cout << "----------------------------" << endl;
    
    // Create a new network
    Network myNet;
    
    // Create some test people
    Person* p1 = new Person("Tommy", "Trojan", "1/1/2000", "(USC) tommy@usc.edu", "(Cell) 213-740-1111");
    Person* p2 = new Person("Amy", "Chen", "2/2/2001", "(Gmail) amy@gmail.com", "(Home) 213-740-2222");
    Person* p3 = new Person("Karan", "Kapoor", "3/3/2002", "(Yahoo) karan@yahoo.com", "(Office) 213-740-3333");
    Person* p4 = new Person("Katy", "Bruin", "4/4/2003", "(Hotmail) katy@hotmail.com", "(Cell) 213-740-4444");
    Person* p5 = new Person("Kurt", "Cobain", "5/5/2004", "(Work) kurt@work.com", "(Work) 213-740-5555");
    
    // Add people to the network
    myNet.push_back(p1);
    myNet.push_back(p2);
    myNet.push_back(p3);
    myNet.push_back(p4);
    myNet.push_back(p5);
    
    cout << "\nInitial network:" << endl;
    myNet.printDB();
    
    // Test Phase 2 feature 1: Create friend connections
    cout << "\nCreating friend connections..." << endl;
    
    // Tommy is friends with everyone
    p1->makeFriend(p2);
    p2->makeFriend(p1);
    
    p1->makeFriend(p3);
    p3->makeFriend(p1);
    
    p1->makeFriend(p4);
    p4->makeFriend(p1);
    
    p1->makeFriend(p5);
    p5->makeFriend(p1);
    
    // Amy and Karan are also friends
    p2->makeFriend(p3);
    p3->makeFriend(p2);
    
    cout << "\nTesting print_person with friends:" << endl;
    cout << "------------------------------" << endl;
    p1->print_person();
    
    cout << "\nTesting sorted friends list:" << endl;
    cout << "------------------------------" << endl;
    p1->print_friends();
    
    // Test Phase 2 feature 3: Save and load with friends
    cout << "\nSaving network to test_phase2.txt..." << endl;
    myNet.saveDB("test_phase2.txt");
    
    cout << "\nClearing and reloading network..." << endl;
    Network newNet;
    newNet.loadDB("test_phase2.txt");
    
    cout << "\nReloaded network:" << endl;
    newNet.printDB();
    
    // Find Tommy in the new network and print his friends
    Person* loadedTommy = newNet.search("Tommy", "Trojan");
    if (loadedTommy != NULL) {
        cout << "\nTommy's friends after reload:" << endl;
        cout << "------------------------------" << endl;
        loadedTommy->print_friends();
    } else {
        cout << "Error: Tommy not found in reloaded network!" << endl;
    }
    
    cout << "\nTest completed successfully!" << endl;
    cout << "\nNow you can run the main program with the menu:" << endl;
    cout << "./test_network.o" << endl;
    
    return 0;
} 