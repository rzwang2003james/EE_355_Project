#include "network.h"
#include "misc.h"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <map> // Include map for --add parsing

using namespace std;

// Removed print_details_parsable helper function - moved into Network class

int main(int argc, char *argv[]) {
    string db_filename = "networkDB.txt"; // Default database filename

    if (argc < 2) {
        // cerr << "Usage: " << argv[0] << " <command> [options]" << endl;
        // cerr << "Commands:" << endl;
        // cerr << "  --get-all" << endl;
        // cerr << "  --get-details <codename>" << endl;
        // cerr << "  --add --fname <fname> --lname <lname> [--bdate <bdate>] [--phone <phone>] [--email <email>] [--info <key:value>] ..." << endl;
        // cerr << "  --remove <codename>" << endl;
        // cerr << "  --connect <codename1> <codename2>" << endl;
        // return 1;

        Network myNet;
        myNet.showMenu();
        return 0;
    }

    Network network; // Create Network object
    
    string command = argv[1];
    bool needs_load = (command != "--help"); 

    if (needs_load) {
        try {
            network.loadDB(db_filename);
        } catch (const std::exception& e) {
            cerr << "ERROR: Loading database '/" << db_filename << "' failed: " << e.what() << endl;
            return 1;
        } catch (...) {
            cerr << "ERROR: Loading database '/" << db_filename << "' failed with unknown error." << endl;
            return 1;
        }
    }


    if (command == "--get-all") {
        network.printAllSummaries(); 

    } else if (command == "--get-details") {
        if (argc < 3) {
            cerr << "Usage: " << argv[0] << " --get-details <codename>" << endl; return 1;
        }
        string code_name = argv[2];
        // Check if person exists before calling print method to control exit code
        Person* p = network.searchByCodeName(code_name);
        if (p) {
             network.printPersonDetailsParsable(code_name); // Call new public method
        } else {
            cerr << "INFO: Person with codename '" << code_name << "' not found." << endl;
            return 1; // Return non-zero for not found
        }
    } else if (command == "--add") {
        string fname, lname, bdate, phone_str, email_str;
        map<string, string> additional_info;
        for (int i = 2; i < argc; ++i) {
            string arg = argv[i];
            if (arg == "--fname" && i + 1 < argc) fname = argv[++i];
            else if (arg == "--lname" && i + 1 < argc) lname = argv[++i];
            else if (arg == "--bdate" && i + 1 < argc) bdate = argv[++i];
            else if (arg == "--phone" && i + 1 < argc) phone_str = argv[++i];
            else if (arg == "--email" && i + 1 < argc) email_str = argv[++i];
            else if (arg == "--info" && i + 1 < argc) {
                string info_pair = argv[++i];
                size_t colon_pos = info_pair.find(':');
                if (colon_pos != string::npos && colon_pos > 0) {
                    string key = info_pair.substr(0, colon_pos);
                    string value = info_pair.substr(colon_pos + 1);
                    additional_info[key] = value;
                } else {
                    cerr << "WARN: Ignoring malformed --info argument: " << info_pair << endl;
                }
            }
        }
        if (fname.empty() || lname.empty()) {
             cerr << "ERROR: --fname and --lname are required for --add." << endl; return 1;
        }
        string new_code_name = codeName(fname, lname);
        if (network.searchByCodeName(new_code_name)) {
             cerr << "ERROR: Person with codename '" << new_code_name << "' already exists." << endl; return 1;
        }
        Person* newPerson = new Person(fname, lname, bdate, email_str, phone_str);
        for(map<string,string>::iterator it = additional_info.begin(); it != additional_info.end(); ++it) {
            newPerson->add_info(it->first, it->second);
        }
        network.push_front(newPerson); 
        network.saveDB(db_filename);   
        cout << "SUCCESS: Added " << new_code_name << endl; 

    } else if (command == "--remove") {
        if (argc < 3) {
             cerr << "Usage: " << argv[0] << " --remove <codename>" << endl; return 1;
        }
        string code_name = argv[2];
        bool removed = network.removeByCodeName(code_name); 
        if (removed) {
            network.saveDB(db_filename);
            cout << "SUCCESS: Removed " << code_name << endl;
        } else {
            cerr << "ERROR: Failed to remove person '" << code_name << "'. Person not found?" << endl; 
            return 1; 
        }

    } else if (command == "--connect") {
        if (argc < 4) {
            cerr << "Usage: " << argv[0] << " --connect <codename1> <codename2>" << endl; return 1;
        }
        string code1 = argv[2];
        string code2 = argv[3];
        Person* p1 = network.searchByCodeName(code1);
        Person* p2 = network.searchByCodeName(code2);

        if (!p1) {
             cerr << "ERROR: Person 1 ('" << code1 << "') not found." << endl; return 1;
        }
         if (!p2) {
             cerr << "ERROR: Person 2 ('" << code2 << "') not found." << endl; return 1;
        }
         if (p1 == p2) {
             cerr << "ERROR: Cannot connect a person ('" << code1 << "') to themselves." << endl; return 1;
        }

        // Removed the "already friends" check here for minimality, 
        // relying on Person::makeFriend to potentially handle it or just adding again.
        p1->makeFriend(p2);
        p2->makeFriend(p1);
        network.saveDB(db_filename);
        cout << "SUCCESS: Connected " << code1 << " and " << code2 << endl;
        
    } else {
        cerr << "ERROR: Unknown command: " << command << endl;
        return 1;
    }

    return 0; // Indicate success
} 