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

    virtual ~User()
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
            cout << "Disable " << (i == 0 ? "view" : i == 1 ? (role == "Doctor" ? "update" : "register")
                                                            : "delete")
                 << "?(Y/N): ";
            char c;
            cin >> c;
            cin.ignore();
            if (toupper(c) == 'Y' && rights[i])
            {
                newRights[i] = false;
                changed = true;
            }
            else if (toupper(c) == 'N' && !rights[i])
            {
                newRights[i] = true;
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

            cout << "\nEnter patient ID to update (0 to cancel): ";
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
                    cout << "\nCurrent Diagnosis: " << patients[i].getDiagnosis() << "\nEnter new diagnosis: ";
                    string diag;
                    getline(cin, diag);
                    patients[i].setDiagnosis(diag.c_str());
                    fh->updatePatient(patients[i]);
                    cout << "Diagnosis updated!\n";
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

            cout << "\nEnter patient ID to delete (0 to cancel): ";
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
                    cout << "Confirm deletion?(Y/N): ";
                    char c;
                    cin >> c;
                    cin.ignore();
                    if (toupper(c) == 'Y')
                    {
                        fh->deletePatient(id);
                        cout << "Patient deleted!\n";
                    }
                    else
                        cout << "Deletion cancelled.\n";
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

public:
    void displayMenu() override
    {
        cout << "\n---Doctor Menu---\n";
        cout << "1. View records\n";
        cout << "2. Update record\n";
        cout << "3. Delete record\n";
        cout << "4. Back\nEnter your choice: ";
    }

    void handleChoice(int choice) override
    {
        switch (choice)
        {
        case 1:
            viewPatientRecords();
            break;
        case 2:
            updatePatientRecord();
            break;
        case 3:
            deletePatientRecord();
            break;
        }
    }
};

class ReceptionistMenuStrategy : public MenuStrategy
{
    void viewRecords()
    {
        try
        {
            // Check permission
            FileHandler *fh = FileHandler::getInstance();
            int count;
            bool *rights = fh->getAccessRights("Receptionist", count);

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
            bool validInput = false;
            while (!validInput)
            {
                try
                {
                    cout << "\nEnter patient ID to view (0 to cancel): ";
                    string idStr;
                    getline(cin, idStr);

                    if (idStr.empty())
                        throw InvalidInputException();

                    for (char c : idStr)
                    {
                        if (!isdigit(c))
                            throw InvalidInputException();
                    }

                    id = stoi(idStr);

                    validInput = true;
                }
                catch (InvalidInputException &e)
                {
                    cout << "Invalid input!";
                }
                catch (std::out_of_range &e)
                {
                    cout << "ID value is too large! Please enter a smaller number.";
                }
                catch (std::exception &e)
                {
                    cout << "An error occurred: " << e.what() << "\n";
                }
            }

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
                    cout << "\nPatient Details:\nID: " << id << "\nName: " << patients[i].getName()
                         << "\nAge: " << patients[i].getAge() << "\nGender: " << patients[i].getGender()
                         << "\nAddress: " << patients[i].getAddress() << "\nContact: " << patients[i].getContactNumber() << endl;
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                cout << "Patient not found with ID: " << id << "\n";
            }

            delete[] patients;
        }
        catch (PermissionDeniedException &e)
        {
            cout << e.what() << endl;
        }
        catch (HospitalException &e)
        {
            cout << "Hospital system error: " << e.what() << endl;
        }
        catch (std::exception &e)
        {
            cout << "Unexpected error: " << e.what() << endl;
        }
    }

    bool isWhiteSpace(const string &input)
    {
        for (char c : input)
        {
            if (!isspace(c))
            {
                return false;
            }
        }
        return true;
    }

    bool isValidReceptionistMenuInput(const string &input)
    {
        if (input.empty() || isWhiteSpace(input))
        {
            return false;
        }

        for (char c : input)
        {
            if (!isdigit(c))
            {
                return false;
            }
        }

        try
        {
            int choice = stoi(input);
            return (choice >= 1 && choice <= 3);
        }
        catch (const std::exception &)
        {
            return false;
        }
    }

    void execute()
    {
        while (true)
        {
            cout << "\n------- Receptionist Menu --------\n";
            cout << "1. View Records\n";
            cout << "2. Register Patient\n";
            cout << "3. Back\n";
            cout << "Enter your choice: ";

            string input;
            getline(cin, input);

            if (!isValidReceptionistMenuInput(input))
            {
                cout << "Invalid input! Please enter only numbers between 1-3.\n";
                continue;
            }

            int choice = stoi(input);

            switch (choice)
            {
            case 1:
                viewRecords();
                break;
            case 2:
                registerPatient();
                break;
            case 3:
                return;
            default:
                // This should never execute due to validation
                cout << "Invalid choice. Please try again.\n";
            }
        }
    }

    void registerPatient()
    {
        try
        {
            // Check permission
            FileHandler *fh = FileHandler::getInstance();
            int count;
            bool *rights = fh->getAccessRights("Receptionist", count);

            if (!rights[0])
            {
                delete[] rights;
                throw PermissionDeniedException();
            }
            delete[] rights;

            int id = fh->getNextPatientId();

            cout << "\nPatient Registration\n--------------------------\n";
            string name, addr, contact;
            int age;
            char gender;

            auto isWhiteSpace = [](const string &str) -> bool
            {
                for (char c : str)
                {
                    if (!isspace(c))
                    {
                        return false;
                    }
                }
                return true;
            };

            bool validInput = false;
            while (!validInput)
            {
                try
                {
                    cout << "Name: ";
                    getline(cin, name);

                    if (name.empty() || isWhiteSpace(name))
                        throw InvalidInputException();

                    for (char c : name)
                    {
                        if (!isalpha(c) && !isspace(c))
                            throw InvalidInputException();
                    }
                    validInput = true;
                }
                catch (InvalidInputException &e)
                {
                    cout << "Invalid input!\n";
                }
            }

            validInput = false;
            while (!validInput)
            {
                try
                {
                    cout << "Age: ";
                    string ageStr;
                    getline(cin, ageStr);

                    if (ageStr.empty())
                        throw InvalidInputException();

                    for (char c : ageStr)
                    {
                        if (!isdigit(c))
                            throw InvalidInputException();
                    }

                    age = stoi(ageStr);
                    if (age <= 0 || age > 150)
                        throw InvalidInputException();

                    validInput = true;
                }
                catch (InvalidInputException &e)
                {
                    cout << "Invalid input!\n";
                }
            }

            validInput = false;
            while (!validInput)
            {
                try
                {
                    cout << "Gender (M/F/O): ";
                    string genderStr;
                    getline(cin, genderStr);

                    if (genderStr.length() != 1)
                        throw InvalidInputException();

                    gender = toupper(genderStr[0]);
                    if (gender != 'M' && gender != 'F' && gender != 'O')
                        throw InvalidInputException();

                    validInput = true;
                }
                catch (InvalidInputException &e)
                {
                    cout << "Invalid input!\n";
                }
            }

            validInput = false;
            while (!validInput)
            {
                try
                {
                    cout << "Address: ";
                    getline(cin, addr);

                    if (addr.empty() || isWhiteSpace(addr))
                        throw InvalidInputException();

                    for (char c : addr)
                    {
                        if (!isalnum(c) && !isspace(c) && c != ',' && c != '.' && c != '-' && c != '/' && c != '#')
                            throw InvalidInputException();
                    }
                    validInput = true;
                }
                catch (InvalidInputException &e)
                {
                    cout << "Invalid input!\n";
                }
            }

            validInput = false;
            while (!validInput)
            {
                try
                {
                    cout << "Contact: ";
                    getline(cin, contact);

                    if (contact.empty())
                        throw InvalidInputException();

                    for (char c : contact)
                    {
                        if (!isdigit(c))
                            throw InvalidInputException();
                    }
                    validInput = true;
                }
                catch (InvalidInputException &e)
                {
                    cout << "Invalid input!\n";
                }
            }

            Patient p(id, name.c_str(), age, toupper(gender), addr.c_str(), contact.c_str());
            fh->savePatient(p);
            cout << "Patient registered with ID: " << id << endl;
        }
        catch (PermissionDeniedException &e)
        {
            cout << e.what() << endl;
        }
    }

public:
    void displayMenu() override
    {
        cout << "\n---Receptionist Menu---\n";
        cout << "1. View records\n";
        cout << "2. Register patient\n";
        cout << "3. Back\nEnter your choice: ";
    }

    void handleChoice(int choice) override
    {
        switch (choice)
        {
        case 1:
            viewRecords();
            break;
        case 2:
            registerPatient();
            break;
        }
    }
};

class Admin : public User
{
public:
    Admin() : User("Admin", "admin") {}
    MenuStrategy *createMenuStrategy() override { return new AdminMenuStrategy(); }
};

class Doctor : public User
{
public:
    Doctor() : User("Doctor", "doctor") {}
    MenuStrategy *createMenuStrategy() override { return new DoctorMenuStrategy(); }
};

class Receptionist : public User
{
public:
    Receptionist() : User("Receptionist", "receptionist") {}
    MenuStrategy *createMenuStrategy() override { return new ReceptionistMenuStrategy(); }
};

class Hospital
{
    User *currentUser = nullptr;
    MenuStrategy *currentMenu = nullptr;

    int getChoice(int min, int max)
    {
        string choiceStr;
        int choice = 0;
        bool validInput = false;

        while (!validInput)
        {
            try
            {
                getline(cin, choiceStr);

                if (choiceStr.empty())
                    throw InvalidInputException();

                for (char c : choiceStr)
                {
                    if (!isdigit(c))
                        throw InvalidInputException();
                }

                choice = stoi(choiceStr);

                if (choice < min || choice > max)
                    throw InvalidInputException();

                validInput = true;
            }
            catch (InvalidInputException &e)
            {
                cout << "Invalid input. Enter a number between " << min << " and " << max << ": ";
            }
            catch (std::out_of_range &e)
            {
                cout << "Number too large! Enter a number between " << min << " and " << max << ": ";
            }
            catch (std::exception &e)
            {
                cout << "An error occurred: " << e.what() << "\nEnter a number between " << min << " and " << max << ": ";
            }
        }

        return choice;
    }

    void login(int role)
    {
        string username, password;
        if (role == 1)
            username = "Admin";
        else if (role == 2)
            username = "Doctor";
        else if (role == 3)
            username = "Receptionist";

        cout << "User: " << username << "\nEnter password: ";
        getline(cin, password);

        if ((role == 1 && password == "admin") ||
            (role == 2 && password == "doctor") ||
            (role == 3 && password == "receptionist"))
        {
            if (role == 1)
                currentUser = new Admin();
            else if (role == 2)
                currentUser = new Doctor();
            else if (role == 3)
                currentUser = new Receptionist();
            cout << "Login successful!\n";
            currentMenu = currentUser->createMenuStrategy();
        }
        else
            throw InvalidCredentialsException();
    }

public:
    ~Hospital()
    {
        delete currentUser;
        delete currentMenu;
    }

    void start()
    {
        bool running = true;
        while (running)
        {
            if (!currentUser)
            {
                cout << "\n---Hospital Management System---\n1. Admin\n2. Doctor\n3. Receptionist\n4. Exit\nEnter choice: ";
                int choice = getChoice(1, 4);
                if (choice == 4)
                {
                    cout << "Goodbye!\n";
                    running = false;
                    break;
                }

                try
                {
                    login(choice);
                }
                catch (InvalidCredentialsException &e)
                {
                    cout << e.what() << endl;
                }
                catch (std::exception &e)
                {
                    cout << "Error: " << e.what() << endl;
                }
            }
            else
            {
                try
                {
                    currentMenu->displayMenu();
                    int choice;

                    if (dynamic_cast<Admin *>(currentUser))
                        choice = getChoice(1, 3);
                    else if (dynamic_cast<Doctor *>(currentUser))
                        choice = getChoice(1, 4);
                    else if (dynamic_cast<Receptionist *>(currentUser))
                        choice = getChoice(1, 3);

                    if ((dynamic_cast<Admin *>(currentUser) && choice == 3) ||
                        (dynamic_cast<Doctor *>(currentUser) && choice == 4) ||
                        (dynamic_cast<Receptionist *>(currentUser) && choice == 3))
                    {
                        delete currentUser;
                        currentUser = nullptr;
                        delete currentMenu;
                        currentMenu = nullptr;
                        continue;
                    }
                    currentMenu->handleChoice(choice);
                }
                catch (HospitalException &e)
                {
                    cout << "Hospital system error: " << e.what() << endl;
                }
                catch (std::exception &e)
                {
                    cout << "Error occurred: " << e.what() << endl;
                }
            }
        }
    }
};

int main()
{
    try
    {
        Hospital().start();
    }
    catch (...)
    {
        cout << "Fatal error\n";
        return 1;
    }
    return 0;
}