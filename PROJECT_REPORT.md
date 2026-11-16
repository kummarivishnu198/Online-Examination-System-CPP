# PROJECT REPORT
## ONLINE EXAMINATION SYSTEM USING C++ AND SQLITE

### 1. Abstract
The Online Examination System is a console-based application developed using C++17 and SQLite. It supports student registration, role-based login, question-bank management, randomized multiple-choice examinations, automatic evaluation, and result history. SQLite stores users, questions, examination attempts, and individual responses so that data remains available after the program closes. The project demonstrates object-oriented programming, database connectivity, validation, STL containers, randomization, exception handling, and modular design.

### 2. Problem Statement
Manual examination workflows require significant effort to prepare question papers, evaluate answers, maintain student records, and generate results. This project provides a small computerized system to organize objective examinations and maintain their records consistently.

### 3. Objectives
- Provide separate admin and student workflows.
- Store records persistently using a relational database.
- Allow an administrator to add questions.
- Randomize the questions presented to each student.
- Evaluate objective answers automatically.
- Maintain individual and consolidated result history.

### 4. Scope
The current version supports one local database and console-based use. It is suitable for a demonstration or lab mini-project. It does not provide networked multi-user access, remote proctoring, encrypted credentials, or a tamper-proof exam environment.

### 5. Technologies
- Language: C++17
- Database: SQLite 3
- Libraries: STL, chrono, random, iostream, sqlite3 C API
- Tools: Visual Studio Code, GCC/MinGW, Git and GitHub

### 6. System Modules
1. Authentication and registration
2. Admin dashboard
3. Question-bank management
4. Student dashboard
5. Examination engine
6. Automatic evaluation
7. Result history and reports
8. SQLite persistence layer

### 7. Database Design
**users**: id (PK), username (unique), password, role, full_name.

**questions**: id (PK), topic, question, options A-D, correct option, marks.

**attempts**: id (PK), user_id (FK), started_at, score, total, status.

**responses**: id (PK), attempt_id (FK), question_id (FK), chosen option, correctness; one response per question per attempt.

### 8. OOP and C++ Concepts
The `Database`, `Statement`, and `ExaminationSystem` classes encapsulate database access, prepared statements, and application workflows. Structures represent users and questions. STL vectors store question collections; `shuffle` randomizes question order. Exception handling reports database errors. Prepared statements bind user-provided values instead of concatenating them into SQL statements.

### 9. Algorithms
- Login: query user by username and password.
- Random exam: load questions, shuffle the vector, and select the requested count.
- Evaluation: compare each selected option with the stored answer and add the question's marks when correct.
- Result percentage: (score / total marks) × 100.

### 10. Testing
Test cases should include successful and failed login, duplicate username, empty registration fields, adding a question, invalid menu input, randomized exam selection, correct and incorrect answers, unanswered questions, result persistence after restart, and admin report generation.

### 11. Limitations and Future Enhancements
- Replace demonstration password storage with salted password hashing.
- Add exam creation, categories, difficulty levels, negative marking and configurable duration.
- Add CSV/PDF result export and database backup.
- Add stricter timed input, audit logs and role permissions.
- Add a GUI or client-server architecture for remote examinations.
- Add unit tests and continuous integration.

### 12. Conclusion
The project demonstrates how C++ can be integrated with a relational database to create a persistent, menu-driven examination application. It combines OOP, STL, SQL, validation and automated evaluation in a practical team project.

### 13. Team Contributions
Member 1: Authentication, registration and admin question management.
Member 2: Student interface, examination workflow and randomization.
Member 3: Database schema, result evaluation, reporting, integration and testing.
(Replace these labels with the actual names and describe the real contributions.)
