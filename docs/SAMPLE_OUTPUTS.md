# Sample Console Run (illustrative)

## First run: admin
```text
========== MAIN MENU ==========
1. Login
2. Student registration
3. Exit
Select: 1
Username: admin
Password: admin123

Welcome, System Administrator (admin)!

========== ADMIN DASHBOARD ==========
1. Add question
2. View question bank
3. View all results
4. Logout
Select: 2

--- Question Bank (5) ---
#1 [C++] Which feature binds data and functions together? (2 marks)
...
```

## Student registration and exam
```text
Select: 2
--- Student Registration ---
Full name: Demo Student
Choose username: demo
Password: demo123
Registration successful. You can now log in.

Select: 1
Username: demo
Password: demo123
Welcome, Demo Student (student)!

========== STUDENT DASHBOARD ==========
1. Take examination
2. View my results
3. Logout
Select: 1
Number of questions (1-5): 3

========== EXAM STARTED ==========
Questions: 3 | Maximum marks: 5
Q1. [DSA, 1 marks] Which data structure follows FIFO?
A) Stack
B) Tree
C) Queue
D) Graph
Your answer: C

========== EXAM RESULT ==========
Student: Demo Student
Score: 5/5
Percentage: 100.00%
Status: PASS
Attempt ID: 1
```
The exact questions, order, timestamp, score and attempt ID depend on the run because question order is randomized.
