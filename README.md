# Online Examination System (C++17 + SQLite)

A team mini-project: console-based examination platform with persistent SQLite storage.

## Features
- Admin/student roles and login
- Student self-registration
- Admin question-bank management
- Randomized question selection
- Multiple-choice examinations and automatic marking
- Attempt history and admin result report
- Relational database with foreign keys and prepared statements

## Requirements
- C++17 compiler (GCC/MinGW or Clang)
- SQLite3 development library and headers

## Build (Linux)
```bash
sudo apt install g++ libsqlite3-dev
g++ -std=c++17 -Wall -Wextra -pedantic src/main.cpp -lsqlite3 -o exam_system
./exam_system
```

## Build (Windows / MinGW)
Install SQLite development files and ensure `sqlite3.h` and `libsqlite3.a` are available.
```bash
g++ -std=c++17 -Wall -Wextra src/main.cpp -I"path/to/sqlite/include" -L"path/to/sqlite/lib" -lsqlite3 -o exam_system.exe
exam_system.exe
```
In VS Code, open this folder, use the integrated terminal, and run the command above. Configure include/library paths for your own SQLite installation.

## Default admin (demo only)
- Username: `admin`
- Password: `admin123`

Change this immediately for any real deployment. The sample stores passwords as plain text for teaching simplicity; it is NOT production-grade authentication.

## Database
On first run, `exam_system.db` is created automatically with users, questions, attempts and responses tables. Five sample questions are inserted if the question bank is empty.

## Important implementation notes
- The exam duration is a simple elapsed-time check between questions; console input itself is not forcibly interrupted. For a stricter timer, implement a platform-specific asynchronous input layer or GUI.
- This is an educational prototype, not a secure production examination platform. Add password hashing, audit logs, stronger validation, backup/restore and server-side authorization before deployment.


