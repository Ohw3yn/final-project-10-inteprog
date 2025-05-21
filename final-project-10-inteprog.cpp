#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <sstream>
#include <climits>
#include <cctype>
#include <algorithm>

using namespace std;

class HospitalException : public exception {
    char* message;
public:
    HospitalException(const char* msg) : message(new char[strlen(msg) + 1]) { strcpy(message, msg); }
    ~HospitalException() { delete[] message; }
    const char* what() const throw() { return message; }
};

class InvalidCredentialsException : public HospitalException {
public: InvalidCredentialsException() : HospitalException("Invalid Credentials, Please Try Again.") {}
};

class InvalidInputException : public HospitalException {
public: InvalidInputException() : HospitalException("Invalid input") {}
};

class FileOperationException : public HospitalException {
public: FileOperationException(const char* msg) : HospitalException(msg) {}
};

class PatientNotFoundException : public HospitalException {
public: PatientNotFoundException() : HospitalException("Patient not found") {}
};

class PermissionDeniedException : public HospitalException {
public: PermissionDeniedException() : HospitalException("Permission denied: The administrator has restricted your access to this function") {}
};

class MenuStrategy {
public:
    virtual void displayMenu() = 0;
    virtual void handleChoice(int) = 0;
    virtual ~MenuStrategy() {}
};

class Patient {
    int id;
    char *name, *address, *contactNumber, *diagnosis;
    int age;
    char gender;
public:
    Patient(int id=0, const char* name="", int age=0, char gender='\0', const char* address="", const char* contactNumber="", const char* diagnosis="") 
        : id(id), age(age), gender(gender) {
        this->name = new char[strlen(name)+1]; strcpy(this->name, name);
        this->address = new char[strlen(address)+1]; strcpy(this->address, address);
        this->contactNumber = new char[strlen(contactNumber)+1]; strcpy(this->contactNumber, contactNumber);
        this->diagnosis = new char[strlen(diagnosis)+1]; strcpy(this->diagnosis, diagnosis);
    }
    
    Patient(const Patient& other) : Patient(other.id, other.name, other.age, other.gender, other.address, other.contactNumber, other.diagnosis) {}
    
    // Assignment operator to fix memory management issues
    Patient& operator=(const Patient& other) {
        if (this != &other) {
            delete[] name;
            delete[] address;
            delete[] contactNumber;
            delete[] diagnosis;
            
            id = other.id;
            age = other.age;
            gender = other.gender;
            name = new char[strlen(other.name)+1]; strcpy(name, other.name);
            address = new char[strlen(other.address)+1]; strcpy(address, other.address);
            contactNumber = new char[strlen(other.contactNumber)+1]; strcpy(contactNumber, other.contactNumber);
            diagnosis = new char[strlen(other.diagnosis)+1]; strcpy(diagnosis, other.diagnosis);
        }
        return *this;
    }
    
    ~Patient() { delete[] name; delete[] address; delete[] contactNumber; delete[] diagnosis; }
    
    int getId() const { return id; }
    const char* getName() const { return name; }
    int getAge() const { return age; }
    char getGender() const { return gender; }
    const char* getAddress() const { return address; }
    const char* getContactNumber() const { return contactNumber; }
    const char* getDiagnosis() const { return diagnosis; }
    
    void setId(int id) { this->id = id; }
    void setAge(int age) { this->age = age; }
    void setGender(char gender) { this->gender = gender; }
    
    void setName(const char* name) { delete[] this->name; this->name = new char[strlen(name)+1]; strcpy(this->name, name); }
    void setAddress(const char* address) { delete[] this->address; this->address = new char[strlen(address)+1]; strcpy(this->address, address); }
    void setContactNumber(const char* contactNumber) { delete[] this->contactNumber; this->contactNumber = new char[strlen(contactNumber)+1]; strcpy(this->contactNumber, contactNumber); }
    void setDiagnosis(const char* diagnosis) { delete[] this->diagnosis; this->diagnosis = new char[strlen(diagnosis)+1]; strcpy(this->diagnosis, diagnosis); }
    
    void display() const {
        cout << "Patient ID: " << id << "\nName: " << name << "\nAge: " << age << "\nGender: " << gender 
             << "\nAddress: " << address << "\nContact: " << contactNumber << "\nDiagnosis: " 
             << (strlen(diagnosis) ? diagnosis : "No diagnosis") << endl;
    }
    
    void displayShort() const { cout << "ID: " << id << " - Name: " << name << endl; }
    
    string toString() const {
        return to_string(id) + "|" + name + "|" + to_string(age) + "|" + gender + "|" + address + "|" + contactNumber + "|" + diagnosis;
    }
    
    void fromString(const string& str) {
        stringstream ss(str);
        string token;
        
        getline(ss, token, '|'); id = stoi(token);
        getline(ss, token, '|'); setName(token.c_str());
        getline(ss, token, '|'); age = stoi(token);
        getline(ss, token, '|'); gender = token[0];
        getline(ss, token, '|'); setAddress(token.c_str());
        getline(ss, token, '|'); setContactNumber(token.c_str());
        getline(ss, token); setDiagnosis(token.c_str());
    }
};

class User {
protected:
    char *username, *password;
public:
    User(const char* username, const char* password) {
        this->username = new char[strlen(username)+1]; strcpy(this->username, username);
        this->password = new char[strlen(password)+1]; strcpy(this->password, password);
    }
    
    virtual ~User() { delete[] username; delete[] password; }
    const char* getUsername() const { return username; }
    bool authenticate(const char* pwd) const { return strcmp(password, pwd) == 0; }
    virtual MenuStrategy* createMenuStrategy() = 0;
};

class FileHandler {
    static FileHandler* instance;
    const char *patientFile = "patients.txt", *accessRightsFile = "access_rights.txt";
    
    FileHandler() {
        ifstream file(accessRightsFile);
        if (!file.good()) initializeAccessRights();
    }
    
    void initializeAccessRights() {
        ofstream file(accessRightsFile);
        if (!file) throw FileOperationException("Could not create access rights file");
        file << "Doctor|1|1|1\nReceptionist|1|1\n";
    }
    
public:
    static FileHandler* getInstance() {
        if (!instance) instance = new FileHandler();
        return instance;
    }
    
    void savePatient(const Patient& p) {
        ofstream file(patientFile, ios::app);
        if (!file) throw FileOperationException("Could not open patient file");
        file << p.toString() << endl;
    }
    
    void updatePatient(const Patient& p) {
        Patient* patients; int count;
        loadAllPatients(patients, count);
        
        bool found = false;
        for (int i = 0; i < count; i++) {
            if (patients[i].getId() == p.getId()) {
                patients[i] = p;
                found = true;
                break;
            }
        }
        
        if (!found) { delete[] patients; throw PatientNotFoundException(); }
        
        ofstream file(patientFile);
        if (!file) { delete[] patients; throw FileOperationException("Could not open patient file"); }
        
        for (int i = 0; i < count; i++) file << patients[i].toString() << endl;
        delete[] patients;
    }
    
    void deletePatient(int id) {
        Patient* patients; int count;
        loadAllPatients(patients, count);
        
        bool found = false;
        for (int i = 0; i < count; i++) {
            if (patients[i].getId() == id) {
                found = true;
                break;
            }
        }
        
        if (!found) { delete[] patients; throw PatientNotFoundException(); }
        
        ofstream file(patientFile);
        if (!file) { delete[] patients; throw FileOperationException("Could not open patient file"); }
        
        for (int i = 0; i < count; i++) 
            if (patients[i].getId() != id) file << patients[i].toString() << endl;
        
        delete[] patients;
    }
    
    Patient getPatient(int id) {
        ifstream file(patientFile);
        if (!file) throw FileOperationException("Could not open patient file");
        
        string line;
        while (getline(file, line)) {
            if (line.empty()) continue;
            Patient p; p.fromString(line);
            if (p.getId() == id) return p;
        }
        
        throw PatientNotFoundException();
    }
    
    void loadAllPatients(Patient*& patients, int& count) {
        ifstream file(patientFile);
        count = 0;
        
        if (!file) { patients = new Patient[0]; return; }
        
        string line;
        while (getline(file, line)) if (!line.empty()) count++;
        file.clear(); file.seekg(0);
        
        patients = new Patient[count];
        int index = 0;
        while (getline(file, line) && index < count) {
            if (!line.empty()) {
                patients[index].fromString(line);
                index++;
            }
        }
    }
    
    int getNextPatientId() {
        Patient* patients; int count, maxId = 0;
        loadAllPatients(patients, count);
        
        for (int i = 0; i < count; i++) 
            if (patients[i].getId() > maxId) maxId = patients[i].getId();
        
        delete[] patients;
        return maxId + 1;
    }
    
    bool* getAccessRights(const char* role, int& count) {
        ifstream file(accessRightsFile);
        if (!file) throw FileOperationException("Could not open access rights file");
        
        string line;
        while (getline(file, line)) {
            if (line.find(role) == 0) {
                stringstream ss(line);
                string token;
                count = 0;
                
                // Create a copy of the line for counting
                stringstream ss_count(line);
                getline(ss_count, token, '|'); // Skip role
                while (getline(ss_count, token, '|')) count++;
                
                bool* rights = new bool[count];
                getline(ss, token, '|'); // Skip role
                
                for (int i = 0; i < count; i++) {
                    getline(ss, token, '|');
                    rights[i] = (token == "1");
                }
                
                return rights;
            }
        }
        
        throw FileOperationException("Role not found");
    }
    
    void updateAccessRights(const char* role, const bool* rights, int count) {
        ifstream inFile(accessRightsFile);
        if (!inFile) throw FileOperationException("Could not open access rights file");
        
        string content, line;
        bool found = false;
        
        while (getline(inFile, line)) {
            if (line.find(role) == 0) {
                found = true;
                line = role;
                for (int i = 0; i < count; i++) line += "|" + string(rights[i] ? "1" : "0");
            }
            content += line + "\n";
        }
        inFile.close();
        
        if (!found) throw FileOperationException("Role not found");
        
        ofstream outFile(accessRightsFile);
        if (!outFile) throw FileOperationException("Could not open access rights file");
        outFile << content;
    }
};

FileHandler* FileHandler::instance = nullptr;

// Utility functions for input validation
bool isValidInteger(const string& input) {
    return !input.empty() && find_if(input.begin(), input.end(), 
        [](unsigned char c) { return !isdigit(c); }) == input.end();
}

bool isValidYesNo(const string& input) {
    return input.length() == 1 && (toupper(input[0]) == 'Y' || toupper(input[0]) == 'N');
}

bool isValidGender(const string& input) {
    return input.length() == 1 && (toupper(input[0]) == 'M' || toupper(input[0]) == 'F' || toupper(input[0]) == 'O');
}

bool isValidContactNumber(const string& input) {
    return !input.empty() && find_if(input.begin(), input.end(), 
        [](unsigned char c) { return !isdigit(c); }) == input.end();
}

int getInteger(const string& prompt, int min = INT_MIN, int max = INT_MAX) {
    string input;
    int value;
    
    while (true) {
        cout << prompt;
        getline(cin, input);
        
        if (!isValidInteger(input)) {
            cout << "Error: Please enter a valid number.\n";
            continue;
        }
        
        value = stoi(input);
        if (value < min || value > max) {
            cout << "Error: Please enter a number between " << min << " and " << max << ".\n";
            continue;
        }
        
        return value;
    }
}

string getNonEmptyString(const string& prompt, bool (*validator)(const string&) = nullptr, const string& errorMsg = "") {
    string input;
    
    while (true) {
        cout << prompt;
        getline(cin, input);
        
        if (input.empty()) {
            cout << "Error: Input cannot be empty.\n";
            continue;
        }
        
        if (validator && !validator(input)) {
            cout << "Error: " << errorMsg << "\n";
            continue;
        }
        
        return input;
    }
}

char getYesNo(const string& prompt) {
    string input;
    
    while (true) {
        cout << prompt;
        getline(cin, input);
        
        if (!isValidYesNo(input)) {
            cout << "Error: Please enter 'Y' or 'N'.\n";
            continue;
        }
        
        return toupper(input[0]);
    }
}

char getGender(const string& prompt) {
    string input;
    
    while (true) {
        cout << prompt;
        getline(cin, input);
        
        if (!isValidGender(input)) {
            cout << "Error: Please enter 'M', 'F', or 'O'.\n";
            continue;
        }
        
        return toupper(input[0]);
    }
}

class AdminMenuStrategy : public MenuStrategy {
    void manageMenu(const char* role, int count) {
        FileHandler* fh = FileHandler::getInstance();
        bool* rights = fh->getAccessRights(role, count);
        
        cout << "\n" << role << " Current Status\n--------------------------------\n";
        for (int i = 0; i < count; i++) 
            cout << (i==0?"View":i==1?(strcmp(role,"Doctor")==0?"Update":"Register"):"Delete") 
                 << " - " << (rights[i]?"ENABLED":"DISABLED") << endl;
        
        bool* newRights = new bool[count];
        bool changed = false;
        copy(rights, rights+count, newRights);
        
        for (int i = 0; i < count; i++) {
            string action = (i==0?"view":i==1?(strcmp(role,"Doctor")==0?"update":"register"):"delete");
            
            // Fixed logic: Ask to enable if currently disabled, ask to disable if currently enabled
            char c;
            if (rights[i]) {
                // Currently enabled, ask if they want to disable it
                c = getYesNo("Disable " + action + "? (Y/N): ");
                if (c == 'Y') { 
                    newRights[i] = false; 
                    changed = true; 
                }
            } else {
                // Currently disabled, ask if they want to enable it
                c = getYesNo("Enable " + action + "? (Y/N): ");
                if (c == 'Y') { 
                    newRights[i] = true; 
                    changed = true; 
                }
            }
        }
        
        if (changed) {
            char c = getYesNo("Apply changes? (Y/N): ");
            if (c == 'Y') { 
                fh->updateAccessRights(role, newRights, count); 
                cout << "Changes applied successfully!\n"; 
            }
            else cout << "Changes discarded.\n";
        } else cout << "No changes made.\n";
        
        delete[] rights;
        delete[] newRights;
    }
public:
    void displayMenu() override {
        cout << "\n---Admin Menu---\n1. Manage doctor's menu\n2. Manage receptionist's menu\n3. Back\nEnter your choice: ";
    }
    
    void handleChoice(int choice) override {
        switch(choice) {
            case 1: {
                int count;
                manageMenu("Doctor", count = 3);
                break;
            }
            case 2: {
                int count;
                manageMenu("Receptionist", count = 2);
                break;
            }
            case 3: break; // Back option
        }
    }
};

class DoctorMenuStrategy : public MenuStrategy {
    void viewPatientRecords() {
        try {
            FileHandler* fh = FileHandler::getInstance();
            int count; 
            bool* rights = fh->getAccessRights("Doctor", count);
            
            if (!rights[0]) {
                delete[] rights;
                throw PermissionDeniedException();
            }
            delete[] rights;
            
            Patient* patients; 
            fh->loadAllPatients(patients, count);
            
            if (count == 0) { 
                cout << "No patients registered yet.\n"; 
                delete[] patients;
                return; 
            }
            
            cout << "\nPatient List:\n";
            for (int i = 0; i < count; i++) patients[i].displayShort();
            
            int id = getInteger("\nEnter patient ID to view details (0 to cancel): ", 0);
            if (id == 0) { 
                delete[] patients; 
                cout << "Operation cancelled.\n";
                return; 
            }
            
            bool found = false;
            for (int i = 0; i < count; i++) {
                if (patients[i].getId() == id) {
                    cout << "\nPatient Details:\n"; 
                    patients[i].display();
                    found = true;
                    break;
                }
            }
            
            if (!found) cout << "Patient with ID " << id << " not found.\n";
            delete[] patients;
        } catch (const PermissionDeniedException& e) {
            cout << e.what() << endl;
        } catch (const exception& e) {
            cout << "Error: " << e.what() << endl;
        }
    }
    
    void updatePatientRecord() {
        try {
            FileHandler* fh = FileHandler::getInstance();
            int count; 
            bool* rights = fh->getAccessRights("Doctor", count);
            
            if (!rights[1]) {
                delete[] rights;
                throw PermissionDeniedException();
            }
            delete[] rights;
            
            Patient* patients;
            fh->loadAllPatients(patients, count);
            
            if (count == 0) { 
                cout << "No patients registered yet.\n"; 
                delete[] patients;
                return; 
            }
            
            cout << "\nPatient List:\n";
            for (int i = 0; i < count; i++) {
                patients[i].displayShort();
            }
            
            int id = getInteger("\nEnter patient ID to update (0 to cancel): ", 0);
            if (id == 0) { 
                delete[] patients; 
                cout << "Update cancelled.\n";
                return; 
            }
            
            bool patientFound = false;
            for (int i = 0; i < count; i++) {
                if (patients[i].getId() == id) {
                    patientFound = true;
                    cout << "\nCurrent Patient Details:\n";
                    cout << "Name: " << patients[i].getName() << "\n";
                    cout << "Current Diagnosis: " << (strlen(patients[i].getDiagnosis()) > 0 ? patients[i].getDiagnosis() : "No diagnosis") << "\n";
                    
                    string newDiagnosis = getNonEmptyString("Enter new diagnosis: ");
                    patients[i].setDiagnosis(newDiagnosis.c_str());
                    
                    fh->updatePatient(patients[i]);
                    cout << "Diagnosis updated successfully!\n";
                    break;
                }
            }
            
            if (!patientFound) {
                cout << "Patient with ID " << id << " not found.\n";
            }
            
            delete[] patients;
            
        } catch (const PermissionDeniedException& e) {
            cout << e.what() << endl;
        } catch (const exception& e) {
            cout << "Error: " << e.what() << endl;
        }
    }
    
    void deletePatientRecord() {
        try {
            FileHandler* fh = FileHandler::getInstance();
            int count; 
            bool* rights = fh->getAccessRights("Doctor", count);
            
            if (!rights[2]) {
                delete[] rights;
                throw PermissionDeniedException();
            }
            delete[] rights;
            
            Patient* patients;
            fh->loadAllPatients(patients, count);
            
            if (count == 0) { 
                cout << "No patients registered yet.\n"; 
                delete[] patients;
                return; 
            }
            
            cout << "\nPatient List:\n";
            for (int i = 0; i < count; i++) patients[i].displayShort();
            
            int id = getInteger("\nEnter patient ID to delete (0 to cancel): ", 0);
            if (id == 0) { 
                delete[] patients; 
                cout << "Operation cancelled.\n";
                return; 
            }
            
            bool found = false;
            for (int i = 0; i < count; i++) {
                if (patients[i].getId() == id) {
                    found = true;
                    cout << "\nPatient to be deleted:\n";
                    patients[i].display();
                    char c = getYesNo("\nConfirm deletion? (Y/N): ");
                    if (c == 'Y') { 
                        fh->deletePatient(id); 
                        cout << "Patient deleted successfully!\n"; 
                    }
                    else cout << "Deletion cancelled.\n";
                    break;
                }
            }
            
            if (!found) cout << "Patient with ID " << id << " not found.\n";
            delete[] patients;
        } catch (const PermissionDeniedException& e) {
            cout << e.what() << endl;
        } catch (const exception& e) {
            cout << "Error: " << e.what() << endl;
        }
    }
    
public:
    void displayMenu() override {
        cout << "\n---Doctor Menu---\n";
        cout << "1. View patient records\n";
        cout << "2. Update patient record\n";
        cout << "3. Delete patient record\n";
        cout << "4. Back\nEnter your choice: ";
    }
    
    void handleChoice(int choice) override {
        switch(choice) {
            case 1: viewPatientRecords(); break;
            case 2: updatePatientRecord(); break;
            case 3: deletePatientRecord(); break;
            case 4: break; // Back option
        }
    }
};

class ReceptionistMenuStrategy : public MenuStrategy {
    void registerPatient() {
        try {
            FileHandler* fh = FileHandler::getInstance();
            int count; 
            bool* rights = fh->getAccessRights("Receptionist", count);
            
            if (!rights[0]) {
                delete[] rights;
                throw PermissionDeniedException();
            }
            delete[] rights;
            
            int id = fh->getNextPatientId();
            
            cout << "\nPatient Registration\n--------------------------\n";
            string name = getNonEmptyString("Name: ");
            int age = getInteger("Age: ", 0, 150);
            char gender = getGender("Gender (M/F/O): ");
            string addr = getNonEmptyString("Address: ");
            string contact = getNonEmptyString("Contact: ", isValidContactNumber, "Contact should contain only numeric digits");
            
            Patient p(id, name.c_str(), age, gender, addr.c_str(), contact.c_str());
            fh->savePatient(p);
            cout << "Patient registered successfully with ID: " << id << endl;
        } catch (const PermissionDeniedException& e) {
            cout << e.what() << endl;
        } catch (const exception& e) {
            cout << "Error: " << e.what() << endl;
        }
    }
    
    void viewRecords() {
        try {
            FileHandler* fh = FileHandler::getInstance();
            int count; 
            bool* rights = fh->getAccessRights("Receptionist", count);
            
            if (!rights[1]) {
                delete[] rights;
                throw PermissionDeniedException();
            }
            delete[] rights;
            
            Patient* patients;
            fh->loadAllPatients(patients, count);
            
            if (count == 0) { 
                cout << "No patients registered yet.\n"; 
                delete[] patients;
                return; 
            }
            
            cout << "\nPatient List:\n";
            for (int i = 0; i < count; i++) patients[i].displayShort();
            
            int id = getInteger("\nEnter patient ID to view details (0 to cancel): ", 0);
            if (id == 0) { 
                delete[] patients; 
                cout << "Operation cancelled.\n";
                return; 
            }
            
            bool found = false;
            for (int i = 0; i < count; i++) {
                if (patients[i].getId() == id) {
                    cout << "\nPatient Details (Receptionist View):\n";
                    cout << "ID: " << patients[i].getId() 
                         << "\nName: " << patients[i].getName() 
                         << "\nAge: " << patients[i].getAge() 
                         << "\nGender: " << patients[i].getGender() 
                         << "\nAddress: " << patients[i].getAddress() 
                         << "\nContact: " << patients[i].getContactNumber() << endl;
                    found = true;
                    break;
                }
            }
            
            if (!found) cout << "Patient with ID " << id << " not found.\n";
            delete[] patients;
        } catch (const PermissionDeniedException& e) {
            cout << e.what() << endl;
        } catch (const exception& e) {
            cout << "Error: " << e.what() << endl;
        }
    }
    
public:
    void displayMenu() override {
        cout << "\n---Receptionist Menu---\n";
        cout << "1. Register patient\n";
        cout << "2. View patient records\n";
        cout << "3. Back\nEnter your choice: ";
    }
    
    void handleChoice(int choice) override {
        switch(choice) {
            case 1: registerPatient(); break;
            case 2: viewRecords(); break;
            case 3: break; // Back option
        }
    }
};

class Admin : public User {
public: 
    Admin() : User("Admin", "admin") {}
    MenuStrategy* createMenuStrategy() override { return new AdminMenuStrategy(); }
};

class Doctor : public User {
public: 
    Doctor() : User("Doctor", "doctor") {}
    MenuStrategy* createMenuStrategy() override { return new DoctorMenuStrategy(); }
};

class Receptionist : public User {
public: 
    Receptionist() : User("Receptionist", "receptionist") {}
    MenuStrategy* createMenuStrategy() override { return new ReceptionistMenuStrategy(); }
};

class Hospital {
    User* currentUser = nullptr;
    MenuStrategy* currentMenu = nullptr;
    
    int getMenuChoice(int min, int max) {
        string input;
        int choice;
        
        while (true) {
            getline(cin, input);
            
            if (!isValidInteger(input)) {
                cout << "Invalid input, please choose " << min << "-" << max << ".\nEnter your choice: ";
                continue;
            }
            
            choice = stoi(input);
            if (choice < min || choice > max) {
                cout << "Invalid input, please choose " << min << "-" << max << ".\nEnter your choice: ";
                continue;
            }
            
            return choice;
        }
    }
    
    void login(int role) {
        string username, password;
        if (role == 1) username = "Admin";
        else if (role == 2) username = "Doctor";
        else if (role == 3) username = "Receptionist";
        
        bool loginSuccessful = false;
        while (!loginSuccessful) {
            cout << "User: " << username << "\n";
            password = getNonEmptyString("Enter password: ");
            
            if ((role == 1 && password == "admin") || 
                (role == 2 && password == "doctor") || 
                (role == 3 && password == "receptionist")) {
                if (role == 1) currentUser = new Admin();
                else if (role == 2) currentUser = new Doctor();
                else if (role == 3) currentUser = new Receptionist();
                cout << "Login successful!\n";
                currentMenu = currentUser->createMenuStrategy();
                loginSuccessful = true;
            } else {
                cout << "Invalid Credentials, Please Try Again.\n";
            }
        }
    }
public:
    ~Hospital() { 
        delete currentUser; 
        delete currentMenu; 
    }
    
    void start() { 
        bool running = true; 
        
        cout << "=== Welcome to Hospital Management System ===\n";
        
        while (running) { 
            try { 
                if (!currentUser) { 
                    // Login menu
                    cout << "\n---Hospital Management System---\n";
                    cout << "1. Admin\n";
                    cout << "2. Doctor\n";
                    cout << "3. Receptionist\n";
                    cout << "4. Exit\n";
                    cout << "Enter your choice: "; 
                    
                    int choice = getMenuChoice(1, 4); 
                    
                    if (choice == 4) { 
                        cout << "Thank you for using our Hospital Management System. Goodbye!\n"; 
                        running = false;
                        break; 
                    } 
                    
                    try { 
                        login(choice); 
                    } catch (const InvalidCredentialsException& e) { 
                        cout << e.what() << endl; 
                    } 
                } else {
                    // User is logged in, display appropriate menu
                    currentMenu->displayMenu(); 
                    
                    // Set the max choice based on the menu type
                    int maxChoice; 
                    if (dynamic_cast<AdminMenuStrategy*>(currentMenu)) { 
                        maxChoice = 3; 
                    } else if (dynamic_cast<DoctorMenuStrategy*>(currentMenu)) { 
                        maxChoice = 4; 
                    } else if (dynamic_cast<ReceptionistMenuStrategy*>(currentMenu)) { 
                        maxChoice = 3; 
                    } else { 
                        maxChoice = 4; // Default fallback
                    } 
                    
                    int choice = getMenuChoice(1, maxChoice); 
                    
                    // Check if user selected logout option
                    if ((dynamic_cast<AdminMenuStrategy*>(currentMenu) && choice == 3) || 
                        (dynamic_cast<DoctorMenuStrategy*>(currentMenu) && choice == 4) || 
                        (dynamic_cast<ReceptionistMenuStrategy*>(currentMenu) && choice == 3)) { 
                        delete currentUser; currentUser = nullptr; 
                        delete currentMenu; currentMenu = nullptr; 
                        cout << "Logged out successfully.\n";
                    } else {
                        // Handle the user's choice
                        currentMenu->handleChoice(choice);
                    }
                }
            } catch (const HospitalException& e) { 
                cout << "Hospital System Error: " << e.what() << endl; 
            } catch (const exception& e) { 
                cout << "System error: " << e.what() << endl; 
            } catch (...) { 
                cout << "An unknown error occurred. Please try again." << endl; 
            } 
        } 
    }
};

int main() {
    try { 
        Hospital hospital;
        hospital.start(); 
    } catch (const exception& e) { 
        cout << "Fatal error: " << e.what() << endl;
        return 1;
    } catch (...) { 
        cout << "Fatal unknown error occurred." << endl; 
        return 1; 
    }
    return 0;
}