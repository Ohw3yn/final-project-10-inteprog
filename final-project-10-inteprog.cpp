#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <sstream>

using namespace std;

class HospitalException : public exception
{
    char *message;

public:
    HospitalException(const char *msg) : message(new char[strlen(msg) + 1]) { strcpy(message, msg); }
    ~HospitalException() { delete[] message; }
    const char *what() const throw() { return message; }
};

class InvalidCredentialsException : public HospitalException
{
public:
    InvalidCredentialsException() : HospitalException("Invalid username or password") {}
};

class InvalidInputException : public HospitalException
{
public:
    InvalidInputException() : HospitalException("Invalid input") {}
};

class FileOperationException : public HospitalException
{
public:
    FileOperationException(const char *msg) : HospitalException(msg) {}
};

class PatientNotFoundException : public HospitalException
{
public:
    PatientNotFoundException() : HospitalException("Patient not found") {}
};

class PermissionDeniedException : public HospitalException
{
public:
    PermissionDeniedException() : HospitalException("Permission denied: The administrator has restricted your access to this function") {}
};

class MenuStrategy
{
public:
    virtual void displayMenu() = 0;
    virtual void handleChoice(int) = 0;
    virtual ~MenuStrategy() {}
};

class Patient
{
    int id;
    char *name, *address, *contactNumber, *diagnosis;
    int age;
    char gender;

public:
    Patient(int id = 0, const char *name = "", int age = 0, char gender = '\0', const char *address = "", const char *contactNumber = "", const char *diagnosis = "")
        : id(id), age(age), gender(gender)
    {
        this->name = new char[strlen(name) + 1];
        strcpy(this->name, name);
        this->address = new char[strlen(address) + 1];
        strcpy(this->address, address);
        this->contactNumber = new char[strlen(contactNumber) + 1];
        strcpy(this->contactNumber, contactNumber);
        this->diagnosis = new char[strlen(diagnosis) + 1];
        strcpy(this->diagnosis, diagnosis);
    }

    Patient(const Patient &other) : Patient(other.id, other.name, other.age, other.gender, other.address, other.contactNumber, other.diagnosis) {}

    ~Patient()
    {
        delete[] name;
        delete[] address;
        delete[] contactNumber;
        delete[] diagnosis;
    }

    int getId() const { return id; }
    const char *getName() const { return name; }
    int getAge() const { return age; }
    char getGender() const { return gender; }
    const char *getAddress() const { return address; }
    const char *getContactNumber() const { return contactNumber; }
    const char *getDiagnosis() const { return diagnosis; }

    void setId(int id) { this->id = id; }
    void setAge(int age) { this->age = age; }
    void setGender(char gender) { this->gender = gender; }

    void setName(const char *name)
    {
        delete[] this->name;
        this->name = new char[strlen(name) + 1];
        strcpy(this->name, name);
    }
    void setAddress(const char *address)
    {
        delete[] this->address;
        this->address = new char[strlen(address) + 1];
        strcpy(this->address, address);
    }
    void setContactNumber(const char *contactNumber)
    {
        delete[] this->contactNumber;
        this->contactNumber = new char[strlen(contactNumber) + 1];
        strcpy(this->contactNumber, contactNumber);
    }
    void setDiagnosis(const char *diagnosis)
    {
        delete[] this->diagnosis;
        this->diagnosis = new char[strlen(diagnosis) + 1];
        strcpy(this->diagnosis, diagnosis);
    }

    void display() const
    {
        cout << "Patient ID: " << id << "\nName: " << name << "\nAge: " << age << "\nGender: " << gender
             << "\nAddress: " << address << "\nContact: " << contactNumber << "\nDiagnosis: "
             << (strlen(diagnosis) ? diagnosis : "No diagnosis") << endl;
    }

    void displayShort() const { cout << "ID: " << id << " - Name: " << name << endl; }

    string toString() const
    {
        return to_string(id) + "|" + name + "|" + to_string(age) + "|" + gender + "|" + address + "|" + contactNumber + "|" + diagnosis;
    }

    void fromString(const string &str)
    {
        stringstream ss(str);
        string token;

        getline(ss, token, '|');
        id = stoi(token);
        getline(ss, token, '|');
        setName(token.c_str());
        getline(ss, token, '|');
        age = stoi(token);
        getline(ss, token, '|');
        gender = token[0];
        getline(ss, token, '|');
        setAddress(token.c_str());
        getline(ss, token, '|');
        setContactNumber(token.c_str());
        getline(ss, token);
        setDiagnosis(token.c_str());
    }
};

class User
{
protected:
    char *username, *password;

public:
    User(const char *username, const char *password)
    {
        this->username = new char[strlen(username) + 1];
        strcpy(this->username, username);
        this->password = new char[strlen(password) + 1];
        strcpy(this->password, password);
    }

    virtual ~User ()
    {
        delete[] username;
        delete[] password;
    }
    const char *getUsername() const { return username; }
    bool authenticate(const char *pwd) const { return strcmp(password, pwd) == 0; }
    virtual MenuStrategy *createMenuStrategy() = 0;
};

class FileHandler
{
    static FileHandler *instance;
    const char *patientFile = "patients.txt", *accessRightsFile = "access_rights.txt";

    FileHandler()
    {
        ifstream file(accessRightsFile);
        if (!file.good())
            initializeAccessRights();
    }

    void initializeAccessRights()
    {
        ofstream file(accessRightsFile);
        if (!file)
            throw FileOperationException("Could not create access rights file");
        file << "Doctor|1|1|1\nReceptionist|1|1\n";
    }

public:
    static FileHandler *getInstance()
    {
        if (!instance)
            instance = new FileHandler();
        return instance;
    }

    void savePatient(const Patient &p)
    {
        ofstream file(patientFile, ios::app);
        if (!file)
            throw FileOperationException("Could not open patient file");
        file << p.toString() << endl;
    }

    void updatePatient(const Patient &p)
    {
        Patient *patients;
        int count;
        loadAllPatients(patients, count);

        bool found = false;
        for (int i = 0; i < count; i++)
        {
            if (patients[i].getId() == p.getId())
            {
                patients[i] = p;
                found = true;
                break;
            }
        }

        if (!found)
        {
            delete[] patients;
            throw PatientNotFoundException();
        }

        ofstream file(patientFile);
        if (!file)
        {
            delete[] patients;
            throw FileOperationException("Could not open patient file");
        }

        for (int i = 0; i < count; i++)
            file << patients[i].toString() << endl;
        delete[] patients;
    }

    void deletePatient(int id)
    {
        Patient *patients;
        int count;
        loadAllPatients(patients, count);

        bool found = false;
        for (int i = 0; i < count; i++)
        {
            if (patients[i].getId() == id)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            delete[] patients;
            throw PatientNotFoundException();
        }

        ofstream file(patientFile);
        if (!file)
        {
            delete[] patients;
            throw FileOperationException("Could not open patient file");
        }

        for (int i = 0; i < count; i++)
            if (patients[i].getId() != id)
                file << patients[i].toString() << endl;

        delete[] patients;
    }

    Patient getPatient(int id)
    {
        ifstream file(patientFile);
        if (!file)
            throw FileOperationException("Could not open patient file");

        string line;
        while (getline(file, line))
        {
            if (line.empty())
                continue;
            Patient p;
            p.fromString(line);
            if (p.getId() == id)
                return p;
        }

        throw PatientNotFoundException();
    }

    void loadAllPatients(Patient *&patients, int &count)
    {
        ifstream file(patientFile);
        count = 0;

        if (!file)
        {
            patients = new Patient[0];
            return;
        }

        string line;
        while (getline(file, line))
            if (!line.empty())
                count++;
        file.clear();
        file.seekg(0);

        patients = new Patient[count];
        for (int i = 0; i < count && getline(file, line); i++)
            if (!line.empty())
                patients[i].fromString(line);
    }

    int getNextPatientId()
    {
        Patient *patients;
        int count, maxId = 0;
        loadAllPatients(patients, count);

        for (int i = 0; i < count; i++)
            if (patients[i].getId() > maxId)
                maxId = patients[i].getId();

        delete[] patients;
        return maxId + 1;
    }

    bool *getAccessRights(const char *role, int &count)
    {
        ifstream file(accessRightsFile);
        if (!file)
            throw FileOperationException("Could not open access rights file");

        string line;
        while (getline(file, line))
        {
            if (line.find(role) == 0)
            {
                stringstream ss(line);
                string token;
                count = 0;

                getline(ss, token, '|'); // Skip role
                while (getline(ss, token, '|'))
                    count++;

                bool *rights = new bool[count];
                ss.clear();
                ss.seekg(0);
                getline(ss, token, '|'); // Skip role

                for (int i = 0; i < count; i++)
                {
                    getline(ss, token, '|');
                    rights[i] = (token == "1");
                }

                return rights;
            }
        }

        throw FileOperationException("Role not found");
    }

    void updateAccessRights(const char *role, const bool *rights, int count)
    {
        ifstream inFile(accessRightsFile);
        if (!inFile)
            throw FileOperationException("Could not open access rights file");

        string content, line;
        bool found = false;

        while (getline(inFile, line))
        {
            if (line.find(role) == 0)
            {
                found = true;
                line = role;
                for (int i = 0; i < count; i++)
                    line += "|" + string(rights[i] ? "1" : "0");
            }
            content += line + "\n";
        }

        if (!found)
            throw FileOperationException("Role not found");

        ofstream outFile(accessRightsFile);
        if (!outFile)
            throw FileOperationException("Could not open access rights file");
        outFile << content;
    }
};

FileHandler *FileHandler::instance = nullptr;

class AdminMenuStrategy : public MenuStrategy
{
    void manageMenu(const char *role, int count)
    {
        FileHandler *fh = FileHandler::getInstance();
        bool *rights = fh->getAccessRights(role, count);

        cout << "\n"
             << role << " Current Status\n--------------------------------\n";
        for (int i = 0; i < count; i++)
            cout << (i == 0 ? "View" : i == 1 ? (role == "Doctor" ? "Update" : "Register")
                                              : "Delete")
                 << " - " << (rights[i] ? "ENABLED" : "DISABLED") << endl;

        bool newRights[count], changed = false;
        copy(rights, rights + count, newRights);

        for (int i = 0; i < count; i++)
        {
            string action = (i == 0 ? "view" : i == 1 ? (role == "Doctor" ? "update" : "register") : "delete");
            
            // Ask whether to enable or disable based on current state
            if (rights[i]) {
                cout << "Disable " << action << "? (Y/N): ";
            } else {
                cout << "Enable " << action << "? (Y/N): ";
            }
            
            char c;
            cin >> c;
            cin.ignore();
            
            if (toupper(c) == 'Y') {
                // If it's currently enabled, disable it, and vice versa
                newRights[i] = !rights[i];
                changed = true;
            }
        }

        if (changed)
        {
            cout << "Apply changes?(Y/N): ";
            char c;
            cin >> c;
            cin.ignore();
            if (toupper(c) == 'Y')
            {
                fh->updateAccessRights(role, newRights, count);
                cout << "Changes applied!\n";
            }
            else
                cout << "Changes discarded.\n";
        }
        else
            cout << "No changes applied!\n";

        delete[] rights;
    }

public:
    void displayMenu() override
    {
        cout << "\n---Admin---\n1. Manage doctor's menu\n2. Manage receptionist's menu\n3. Back\nEnter your choice: ";
    }

    void handleChoice(int choice) override
    {
        if (choice == 1)
            manageMenu("Doctor", 3);
        else if (choice == 2)
            manageMenu("Receptionist", 2);
    }
};

class DoctorMenuStrategy : public MenuStrategy
{
    void viewPatientRecords()
    {
        try
        {
            // Check permission
            FileHandler *fh = FileHandler::getInstance();
            int count;
            bool *rights = fh->getAccessRights("Doctor", count);

            if (!rights[0])
            {
                delete[] rights;
                throw PermissionDeniedException();
            }
            delete[] rights;

            Patient *patients;
            fh->loadAllPatients(patients, count);

            if (!count)
            {
                cout << "No patients registered yet.\n";
                return;
            }

            cout << "\nPatient List:\n";
            for (int i = 0; i < count; i++)
                patients[i].displayShort();

            cout << "\nEnter patient ID to view details (0 to cancel): ";
            int id;
            cin >> id;
            cin.ignore();
            if (!id)
            {
                delete[] patients;
                return;
            }

            for (int i = 0; i < count; i++)
            {
                if (patients[i].getId() == id)
                {
                    cout << "\nPatient Details:\n";
                    patients[i].display();
                    delete[] patients;
                    return;
                }
            }

            cout << "Patient not found.\n";
            delete[] patients;
        }
        catch (PermissionDeniedException &e)
        {
            cout << e.what() << endl;
        }
    }

    void updatePatientRecord()
    {
        try
        {
            // Check permission
            FileHandler *fh = FileHandler::getInstance();
            int count;
            bool *rights = fh->getAccessRights("Doctor", count);

            if (!rights[1])
            {
                delete[] rights;
                throw PermissionDeniedException();
            }
            delete[] rights;

            Patient *patients;
            fh->loadAllPatients(patients, count);

            if (!count)
            {
                cout << "No patients registered yet.\n";
                return;
            }

            cout << "\nPatient List:\n";
            for (int i = 0; i < count; i++)
                patients[i].displayShort();

            int id = 0;
            bool validPatient = false;
            
            while (!validPatient)
            {
                cout << "\nEnter patient ID to update (0 to cancel): ";
                cin >> id;
                cin.ignore();
                
                if (id == 0)
                {
                    delete[] patients;
                    return;
                }

                bool found = false;
                for (int i = 0; i < count; i++)
                {
                    if (patients[i].getId() == id)
                    {
                        cout << "\nCurrent Diagnosis: " << patients[i].getDiagnosis() << "\nEnter new diagnosis: ";
                        string diag;
                        getline(cin, diag);
                        patients[i].setDiagnosis(diag.c_str());
                        fh->updatePatient(patients[i]);
                        cout << "Diagnosis updated!\n";
                        found = true;
                        validPatient = true;
                        break;
                    }
                }

                if (!found)
                {
                    cout << "Patient not found. Please try again.\n";
                }
            }
            
            delete[] patients;
        }
        catch (PermissionDeniedException &e)
        {
            cout << e.what() << endl;
        }
    }

    void deletePatientRecord()
    {
        try
        {
            // Check permission
            FileHandler *fh = FileHandler::getInstance();
            int count;
            bool *rights = fh->getAccessRights("Doctor", count);

            if (!rights[2])
            {
                delete[] rights;
                throw PermissionDeniedException();
            }
            delete[] rights;

            Patient *patients;
            fh->loadAllPatients(patients, count);

            if (!count)
            {
                cout << "No patients registered yet.\n";
                return;
            }

            cout << "\nPatient List:\n";
            for (int i = 0; i < count; i++)
                patients[i].displayShort();

            int id = 0;
            bool validPatient = false;
            
            while (!validPatient)
            {
                cout << "\nEnter patient ID to delete (0 to cancel): ";
                cin >> id;
                cin.ignore();
                
                if (id == 0)
                {
                    delete[] patients;
                    return;
                }

                bool found = false;
                for (int i = 0; i < count; i++)
                {
                    if (patients[i].getId() == id)
                    {
                        found = true;
                        cout << "Confirm deletion?(Y/N): ";
                        char c;
                        cin >> c;
                        cin.ignore();
                        if (toupper(c) == 'Y')
                        {
                        fh->deletePatient(id);
                            cout << "Patient record deleted.\n";
                        }
                        validPatient = true;
                        break;
                    }
                }

                if (!found)
                {
                    cout << "Patient not found. Please try again.\n";
                }
            }
            
            delete[] patients;
        }
        catch (PermissionDeniedException &e)
        {
            cout << e.what() << endl;
        }
    }

public:
    void displayMenu() override
    {
        cout << "\n---Doctor---\n1. View Patient Records\n2. Update Patient Record\n3. Delete Patient Record\n4. Back\nEnter your choice: ";
    }

    void handleChoice(int choice) override
    {
        if (choice == 1)
            viewPatientRecords();
        else if (choice == 2)
            updatePatientRecord();
        else if (choice == 3)
            deletePatientRecord();
    }
};

class ReceptionistMenuStrategy : public MenuStrategy
{
    void registerPatient()
    {
        try
        {
            FileHandler *fh = FileHandler::getInstance();
            Patient p;
            p.setId(fh->getNextPatientId());

            cout << "Enter patient name: ";
            string name;
            getline(cin, name);
            p.setName(name.c_str());

            cout << "Enter patient age: ";
            int age;
            cin >> age;
            cin.ignore();
            p.setAge(age);

            cout << "Enter patient gender (M/F): ";
            char gender;
            cin >> gender;
            cin.ignore();
            p.setGender(gender);

            cout << "Enter patient address: ";
            string address;
            getline(cin, address);
            p.setAddress(address.c_str());

            cout << "Enter patient contact number: ";
            string contact;
            getline(cin, contact);
            p.setContactNumber(contact.c_str());

            cout << "Enter patient diagnosis: ";
            string diagnosis;
            getline(cin, diagnosis);
            p.setDiagnosis(diagnosis.c_str());

            fh->savePatient(p);
            cout << "Patient registered successfully with ID: " << p.getId() << endl;
        }
        catch (const HospitalException &e)
        {
            cout << e.what() << endl;
        }
    }

public:
    void displayMenu() override
    {
        cout << "\n---Receptionist---\n1. Register Patient\n2. Back\nEnter your choice: ";
    }

    void handleChoice(int choice) override
    {
        if (choice == 1)
            registerPatient();
    }
};

class HospitalManagementSystem
{
    User *currentUser ;

public:
    void login()
    {
        // Implement login logic here
    }

    void run()
    {
        while (true)
        {
            // Display main menu and handle user choice
        }
    }
};

int main()
{
    HospitalManagementSystem hms;
    hms.login();
    hms.run();
    return 0;
}
