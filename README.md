# To-Do List App (C++)

## Description
A command-line interactive To-Do list application that stores tasks in either TXT or JSON format (controlled via `#define USE_JSON`).

## How to Run
1. **Download `json.hpp`** from [nlohmann/json](https://github.com/nlohmann/json)  
   Place it in the same folder as `todo.cpp`.
2. **Compile**
   ```bash
   g++ todo.cpp -o todo
3. **Run**
   ./todo 

## Language and tools
    Language: C++
    Compiler: g++
    Libraries: nlohmann/json (header-only)    

## Extra Featuers
1. **Tag/Category support for each task**
2. **Option to switch between TXT and JSON storage**
3. **Delete all tasks at once**
4. **Synchronzation between database and application allowing for multiple devices running on same database**
5. **Automatic timestamp generation for each task**
6. **Persistent storage between runs**

## File Structure
ToDoApp/
│   todo.cpp       // Main program code
│   json.hpp       // JSON library (header-only)
│   README.md      // Project documentation
│   tasks.json     // Saved tasks if using JSON mode
│   tasks.txt      // Saved tasks if using TXT mode

## Notes
If tasks.json or tasks.txt does not exist, it will be created automatically.

When switching between TXT and JSON modes, you may want to delete the old file to avoid conflicts


