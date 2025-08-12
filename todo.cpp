#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>

#define USE_JSON // Comment this line to use TXT storage

#ifdef USE_JSON
#include "json.hpp"
using json = nlohmann::json;
#endif

using namespace std;

// ==================== Task Class ====================
class Task {
private:
    string description;
    bool done;
    string created_at;
    string priority;
    string tag;

    string current_timestamp() const {
        // Get the current time as a time_t object ()
        // Stores the current time as seconds since Unix epoch (January 1, 1970)
        time_t now = time(nullptr);
        //the local time structure of type tm which is a struct 
        tm local_tm{};

        // stores the now time in local_tm structure
        localtime_s(&local_tm, &now);

        //part of the ostream library that allows us to format and output data
        //ostringstream is needed as output functionality only 
        ostringstream oss;
        // Format the time as "YYYY-MM-DD HH:MM:SS"
        //put_time is used to format the time into a string
        oss << put_time(&local_tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

public:
    // Constructors
    Task(string desc, string prio = "normal", string tag = "General", bool isDone = false, string created = "")
        : description(desc), priority(prio), done(isDone) ,tag(tag) 
    {
        // either use a given timestamp or generate the current one
        created_at = (created.empty()) ? current_timestamp() : created;
    }

    // Getters
    string getDescription() const { return description; }
    bool isDone() const { return done; }
    string getCreatedAt() const { return created_at; }
    string getPriority() const { return priority; }
    string getTag() const { return tag; }

    // Setters
    void markDone() { done = true; }

    // Convert task to string for saving in file
    // special per 
    string serialize() const {
        return description + "|" + priority + "|" + (done ? "1" : "0") + "|" + created_at + "|" + tag;
    }

    // Create task from a serialized string
    //extracts the task details from a given string 
    // global per class 
    static Task deserialize(const string &line) {
        // Create a stringstream to parse the line
        stringstream ss(line);
        string desc, prio, doneStr, created, TaskTag;
        //getline takes (input stream, string variable, delimiter)
        getline(ss, desc, '|');
        getline(ss, prio, '|');
        getline(ss, doneStr, '|');
        getline(ss, created, '|');
        getline(ss, TaskTag, '|'); 

        if (TaskTag.empty()) TaskTag = "General"; 
        //returns a Task object constructed from the parsed values
        return Task(desc, prio, TaskTag, (doneStr == "1"), created);
    }


    //Json Version 
    #ifdef USE_JSON
    json toJson() const {
        return {
            {"description", description},
            {"priority", priority},
            {"done", done},
            {"created_at", created_at},
            {"tag", tag}
            };
        }

    static Task fromJson(const json& j) {
        return Task(
            j.at("description").get<string>(),
            j.at("priority").get<string>(),
            j.value("tag", "General"),
            j.at("done").get<bool>(),
            j.at("created_at").get<string>()
        );
        }
#endif

};

// ==================== TaskManager Class ====================
class TaskManager {
private:
    vector<Task> tasks;
    // File to store tasks
    #ifdef USE_JSON
    const string TASKS_FILE = "tasks.json";
    #else
    const string TASKS_FILE = "tasks.txt";
    #endif

    void loadFromFile() {
        ifstream file(TASKS_FILE);
        #ifdef USE_JSON
            if (file.peek() == ifstream::traits_type::eof()) {
                // File is empty, nothing to load
                return;
            }
            json jTasks;
            file >> jTasks;
            for (const auto &jTask : jTasks) {
                tasks.push_back(Task::fromJson(jTask));
            }
        #else    
            string line;
            // keeps reading each line in the .txt until the end
            while (getline(file, line)) {
                if (!line.empty()) {
                    //adds returned deserilaized task to the tasks vector 
                    tasks.push_back(Task::deserialize(line));
                }
            }
        #endif
    }

    void saveToFile() const {
        // Open the file in write mode
        ofstream file(TASKS_FILE);

        #ifdef USE_JSON
            json jTasks = json::array();
            for (const auto &task : tasks) {
                jTasks.push_back(task.toJson());
                }
            file << jTasks.dump(4);
        #else       
            //loops on all tasks in vector and writes it into the file
            for (const auto &task : tasks) {
            file << task.serialize() << "\n";
            }
        #endif
    }

    /*Extra feature to allow for manually writing tasks to the file
    or if another application is writing to the same database (several devices)*/ 
    void refreshDatabase() 
    {
        tasks.clear();  // Clear existing tasks
        loadFromFile(); // Reload from file
    }

public:
    TaskManager() {
        //cant use refreshDatabase here as it will clear tasks
        loadFromFile();
    }

    // Interactive methods
    // These methods allow the user to interactively add, list, mark as done, and delete tasks
    void addTaskInteractive() {
        refreshDatabase(); // Ensure we have the latest tasks
        string description, priority, tag;
        cout << "Enter task description: ";
        getline(cin, description); //using getline to allow spaces in description
        cout << "Enter priority (low/normal/high): ";
        getline(cin, priority);
        if (priority.empty()) priority = "normal";
        cout << "Enter tag/category: ";
        getline(cin, tag);
        if (tag.empty()) tag = "General";    

        /*add a task object to the vector with the given description and priority
        and default not done and timestap inside function*/
        tasks.push_back(Task(description, priority,tag));
        // Save the updated task list to file
        saveToFile();
        cout << "Task added successfully!\n";
    }

    void listTasks() {
        refreshDatabase(); // Ensure we have the latest tasks
        if (tasks.empty()) {
            cout << "No tasks found.\n";
            return;
        }
        int index = 1;
        cout << "\n---- To-Do List ----\n";
        for (const auto &task : tasks) {
            cout << index++ << ". "
                 << " [" << task.getPriority() << "] "
                 << "(" << task.getTag() << ") " 
                 << task.getDescription()
                 <<(task.isDone() ? " (done) " : " (pending) ")
                 << " (Created: " << task.getCreatedAt() << ")\n";
        }
        cout << "--------------------\n";
    }

    void markTaskDoneInteractive() {
        refreshDatabase(); // Ensure we have the latest tasks
        listTasks();
        if (tasks.empty()) 
        {
            cout << "No tasks to mark as done.\n";
            return;
        };
        int index;
        cout << "Enter task number to mark as done: ";
        cin >> index;
        cin.ignore();
        // Check if the index is negative or exceeds the number of tasks
        if (index < 1 || index > (int)tasks.size()) {
            cout << "Invalid task number.\n";
            return;
        }
        //index is 1-based, so we need to access tasks[index - 1]
        tasks[index - 1].markDone();
        saveToFile();
        cout << "Task "<< tasks[index - 1].getDescription() << "> marked as done.\n";
    }

    void deleteTaskInteractive() {
        refreshDatabase(); // Ensure we have the latest tasks
        listTasks();
        if (tasks.empty())
         {
            cout << "No tasks to delete.\n";
            return;
         };
        int index;
        cout << "Enter task number to delete: ";
        cin >> index;
        cin.ignore();
        if (index < 1 || index > (int)tasks.size()) {
            cout << "Invalid task number.\n";
            return;
        }
        cout << "Deleted task: " << tasks[index - 1].getDescription() << "\n";
        // Remove the task from the vector
        tasks.erase(tasks.begin() + (index - 1));
        saveToFile();
    }

    void deleteAllTasksInteractive()
    {
        refreshDatabase(); // Ensure we have the latest tasks
        if (tasks.empty()) {
            cout << "No tasks to delete.\n";
            return;
        }
        char confirm;
        cout << "Are you sure you want to delete all tasks? (y/n): ";
        cin >> confirm;
        cin.ignore(); // clear newline from buffer
        if (confirm == 'y' || confirm == 'Y') {
            tasks.clear();
            saveToFile();
            cout << "All tasks deleted successfully!\n";
        } else {
            cout << "Deletion cancelled.\n";
        }
    }
    void interactiveMenu() {
        int choice;
        do {
            cout << "\n====== To-Do App ======\n";
            cout << "1. Add Task\n";
            cout << "2. List Tasks\n";
            cout << "3. Mark Task as Done\n";
            cout << "4. Delete Task\n";
            cout << "5. Delete All Tasks\n";
            cout << "6. Exit\n";
            cout << "Choose an option: ";
            cin >> choice;
            cin.ignore(); // clear newline from buffer

            switch (choice) {
                case 1: addTaskInteractive(); break;
                case 2: listTasks(); break;
                case 3: markTaskDoneInteractive(); break;
                case 4: deleteTaskInteractive(); break;
                case 5: 
                    deleteAllTasksInteractive();break;
                case 6: cout << "Exiting...\n"; break;
                default: cout << "Invalid choice. Try again.\n"; break;
            }
        } while (choice != 6);
    }
};



// ==================== Main ====================
int main() {
    TaskManager manager;
    manager.interactiveMenu();
    return 0;
}
