#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <vector>
#include <sqlite3.h>

using namespace std;

class Database {
    sqlite3* db = nullptr;
public:
    explicit Database(const string& file) {
        if (sqlite3_open(file.c_str(), &db) != SQLITE_OK)
            throw runtime_error(string("Cannot open database: ") + sqlite3_errmsg(db));
        initialize();
    }
    ~Database() { if (db) sqlite3_close(db); }
    sqlite3* get() { return db; }

    void execute(const string& sql) {
        char* error = nullptr;
        if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &error) != SQLITE_OK) {
            string message = error ? error : "SQL execution failed";
            sqlite3_free(error);
            throw runtime_error(message);
        }
    }

    void initialize() {
        execute("PRAGMA foreign_keys = ON;");
        execute("CREATE TABLE IF NOT EXISTS users("
                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "username TEXT UNIQUE NOT NULL,"
                "password TEXT NOT NULL,"
                "role TEXT NOT NULL CHECK(role IN ('admin','student')),"
                "full_name TEXT NOT NULL);");
        execute("CREATE TABLE IF NOT EXISTS questions("
                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "topic TEXT NOT NULL, question TEXT NOT NULL,"
                "a TEXT NOT NULL, b TEXT NOT NULL, c TEXT NOT NULL, d TEXT NOT NULL,"
                "correct TEXT NOT NULL CHECK(correct IN ('A','B','C','D')),"
                "marks INTEGER NOT NULL DEFAULT 1);");
        execute("CREATE TABLE IF NOT EXISTS attempts("
                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "user_id INTEGER NOT NULL REFERENCES users(id),"
                "started_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,"
                "score INTEGER NOT NULL DEFAULT 0,"
                "total INTEGER NOT NULL DEFAULT 0,"
                "status TEXT NOT NULL DEFAULT 'SUBMITTED');");
        execute("CREATE TABLE IF NOT EXISTS responses("
                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "attempt_id INTEGER NOT NULL REFERENCES attempts(id) ON DELETE CASCADE,"
                "question_id INTEGER NOT NULL REFERENCES questions(id),"
                "chosen TEXT NOT NULL, is_correct INTEGER NOT NULL,"
                "UNIQUE(attempt_id, question_id));");

        sqlite3_stmt* s = nullptr;
        sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM users WHERE username='admin';", -1, &s, nullptr);
        int count = 0;
        if (sqlite3_step(s) == SQLITE_ROW) count = sqlite3_column_int(s, 0);
        sqlite3_finalize(s);
        if (!count) {
            execute("INSERT INTO users(username,password,role,full_name)"
                    " VALUES('admin','admin123','admin','System Administrator');");
        }

        sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM questions;", -1, &s, nullptr);
        count = 0;
        if (sqlite3_step(s) == SQLITE_ROW) count = sqlite3_column_int(s, 0);
        sqlite3_finalize(s);
        if (!count) {
            execute("INSERT INTO questions(topic,question,a,b,c,d,correct,marks) VALUES"
                    "('C++','Which feature binds data and functions together?','Inheritance','Encapsulation','Overloading','Recursion','B',2),"
                    "('DSA','Which data structure follows FIFO?','Stack','Tree','Queue','Graph','C',1),"
                    "('C++','Which keyword supports runtime polymorphism?','virtual','static','inline','friend','A',2),"
                    "('Database','Which SQL command retrieves rows?','INSERT','UPDATE','DELETE','SELECT','D',1),"
                    "('DSA','Binary search requires the data to be?','Random','Sorted','Hashed','Duplicated','B',2);");
        }
    }
};

class Statement {
    sqlite3_stmt* stmt = nullptr;
public:
    Statement(sqlite3* db, const string& sql) {
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
            throw runtime_error(sqlite3_errmsg(db));
    }
    ~Statement() { if (stmt) sqlite3_finalize(stmt); }
    sqlite3_stmt* get() { return stmt; }
    void text(int index, const string& value) {
        sqlite3_bind_text(stmt, index, value.c_str(), -1, SQLITE_TRANSIENT);
    }
    void integer(int index, int value) { sqlite3_bind_int(stmt, index, value); }
    bool row() { return sqlite3_step(stmt) == SQLITE_ROW; }
    bool done() { return sqlite3_step(stmt) == SQLITE_DONE; }
    string str(int col) {
        const unsigned char* p = sqlite3_column_text(stmt, col);
        return p ? reinterpret_cast<const char*>(p) : "";
    }
    int num(int col) { return sqlite3_column_int(stmt, col); }
};

struct User {
    int id;
    string username, fullName, role;
};

struct Question {
    int id, marks;
    string topic, text, options[4], correct;
};

class ExaminationSystem {
    Database db;
    mt19937 rng{random_device{}()};

    static string readLine(const string& prompt) {
        cout << prompt;
        string value;
        getline(cin, value);
        return value;
    }

    static int readInt(const string& prompt, int low, int high) {
        while (true) {
            string input = readLine(prompt);
            try {
                size_t pos = 0;
                int value = stoi(input, &pos);
                if (pos == input.size() && value >= low && value <= high) return value;
            } catch (...) {}
            cout << "Invalid input. Enter a number from " << low << " to " << high << ".\n";
        }
    }

    static string readPassword() {
        return readLine("Password: ");
    }

    bool authenticate(User& user) {
        cout << "\n========== ONLINE EXAMINATION SYSTEM ==========\n";
        string username = readLine("Username: ");
        string password = readPassword();
        Statement q(db.get(), "SELECT id,username,full_name,role FROM users WHERE username=? AND password=?;");
        q.text(1, username);
        q.text(2, password);
        if (!q.row()) {
            cout << "Login failed. Check your username and password.\n";
            return false;
        }
        user = {q.num(0), q.str(1), q.str(2), q.str(3)};
        cout << "\nWelcome, " << user.fullName << " (" << user.role << ")!\n";
        return true;
    }

    void registerStudent() {
        cout << "\n--- Student Registration ---\n";
        string name = readLine("Full name: ");
        string username = readLine("Choose username: ");
        string password = readPassword();
        if (name.empty() || username.empty() || password.empty()) {
            cout << "All fields are required.\n"; return;
        }
        try {
            Statement q(db.get(), "INSERT INTO users(username,password,role,full_name) VALUES(?,?,'student',?);");
            q.text(1, username); q.text(2, password); q.text(3, name);
            if (!q.done()) throw runtime_error("Registration failed");
            cout << "Registration successful. You can now log in.\n";
        } catch (const exception& e) {
            cout << "Could not register: " << e.what() << "\n";
        }
    }

    vector<Question> loadQuestions() {
        vector<Question> questions;
        Statement q(db.get(), "SELECT id,topic,question,a,b,c,d,correct,marks FROM questions;");
        while (q.row()) {
            Question item;
            item.id=q.num(0); item.topic=q.str(1); item.text=q.str(2);
            for (int i=0;i<4;i++) item.options[i]=q.str(3+i);
            item.correct=q.str(7); item.marks=q.num(8);
            questions.push_back(item);
        }
        return questions;
    }

    void addQuestion() {
        cout << "\n--- Add Question ---\n";
        string topic=readLine("Topic: ");
        string question=readLine("Question: ");
        string options[4];
        for (int i=0;i<4;i++) options[i]=readLine(string(1,'A'+i)+") ");
        string correct=readLine("Correct option (A/B/C/D): ");
        transform(correct.begin(),correct.end(),correct.begin(),::toupper);
        if (topic.empty() || question.empty() || correct.size()!=1 ||
            correct[0]<'A' || correct[0]>'D') {
            cout << "Invalid question details.\n"; return;
        }
        int marks=readInt("Marks (1-10): ",1,10);
        Statement q(db.get(), "INSERT INTO questions(topic,question,a,b,c,d,correct,marks) VALUES(?,?,?,?,?,?,?,?);");
        q.text(1,topic); q.text(2,question);
        for(int i=0;i<4;i++) q.text(3+i,options[i]);
        q.text(7,correct); q.integer(8,marks);
        if(q.done()) cout << "Question added successfully.\n";
        else cout << "Could not add question.\n";
    }

    void listQuestions() {
        auto questions=loadQuestions();
        cout << "\n--- Question Bank (" << questions.size() << ") ---\n";
        for(const auto& q:questions)
            cout << "#" << q.id << " [" << q.topic << "] " << q.text << " (" << q.marks << " marks)\n";
    }

    void takeExam(const User& user) {
        auto questions=loadQuestions();
        if(questions.empty()) { cout << "No questions available.\n"; return; }
        int count=min(static_cast<int>(questions.size()), readInt("Number of questions (1-"+to_string(questions.size())+"): ",1,questions.size()));
        shuffle(questions.begin(),questions.end(),rng);
        questions.resize(count);
        int total=0, score=0;
        for(const auto& q:questions) total+=q.marks;

        Statement attempt(db.get(), "INSERT INTO attempts(user_id,score,total,status) VALUES(?,0,?,'IN_PROGRESS');");
        attempt.integer(1,user.id); attempt.integer(2,total);
        if(!attempt.done()) { cout << "Unable to start exam.\n"; return; }
        int attemptId=static_cast<int>(sqlite3_last_insert_rowid(db.get()));

        cout << "\n========== EXAM STARTED ==========\n"
             << "Questions: " << count << " | Maximum marks: " << total << "\n"
             << "Choose A, B, C or D. Enter X to leave a question unanswered.\n\n";
        auto start=chrono::steady_clock::now();
        const int durationMinutes=10;
        for(size_t i=0;i<questions.size();i++) {
            auto elapsed=chrono::duration_cast<chrono::seconds>(chrono::steady_clock::now()-start).count();
            if(elapsed>=durationMinutes*60) {
                cout << "\nTime limit reached. Remaining questions will be unanswered.\n";
                break;
            }
            const auto& q=questions[i];
            cout << "\nQ" << i+1 << ". [" << q.topic << ", " << q.marks << " marks] " << q.text << "\n";
            for(int j=0;j<4;j++) cout << char('A'+j) << ") " << q.options[j] << "\n";
            cout << "Time remaining: " << max(0,durationMinutes*60-static_cast<int>(elapsed))/60
                 << " min approx.\n";
            string answer=readLine("Your answer: ");
            transform(answer.begin(),answer.end(),answer.begin(),::toupper);
            if(answer!="A" && answer!="B" && answer!="C" && answer!="D") answer="X";
            bool correct=(answer==q.correct);
            if(correct) score+=q.marks;
            Statement response(db.get(), "INSERT INTO responses(attempt_id,question_id,chosen,is_correct) VALUES(?,?,?,?);");
            response.integer(1,attemptId); response.integer(2,q.id);
            response.text(3,answer); response.integer(4,correct?1:0);
            response.done();
        }

        Statement finish(db.get(), "UPDATE attempts SET score=?,status='SUBMITTED' WHERE id=?;");
        finish.integer(1,score); finish.integer(2,attemptId); finish.done();
        double percent=total ? 100.0*score/total : 0.0;
        cout << "\n========== EXAM RESULT ==========\n"
             << "Student: " << user.fullName << "\n"
             << "Score: " << score << "/" << total << "\n"
             << fixed << setprecision(2) << "Percentage: " << percent << "%\n"
             << (percent>=40 ? "Status: PASS\n" : "Status: NEEDS IMPROVEMENT\n")
             << "Attempt ID: " << attemptId << "\n";
    }

    void myResults(const User& user) {
        cout << "\n--- My Exam History ---\n";
        Statement q(db.get(), "SELECT id,started_at,score,total,status FROM attempts WHERE user_id=? ORDER BY id DESC;");
        q.integer(1,user.id);
        bool found=false;
        while(q.row()) {
            found=true;
            double pct=q.num(3)?100.0*q.num(2)/q.num(3):0;
            cout << "Attempt #" << q.num(0) << " | " << q.str(1)
                 << " | Score " << q.num(2) << "/" << q.num(3)
                 << " | " << fixed << setprecision(1) << pct << "% | " << q.str(4) << "\n";
        }
        if(!found) cout << "No attempts found.\n";
    }

    void adminReports() {
        cout << "\n--- All Student Results ---\n";
        Statement q(db.get(),
            "SELECT u.full_name,u.username,a.id,a.started_at,a.score,a.total "
            "FROM attempts a JOIN users u ON a.user_id=u.id "
            "WHERE a.status='SUBMITTED' ORDER BY a.id DESC;");
        bool found=false;
        while(q.row()) {
            found=true;
            cout << q.str(0) << " (" << q.str(1) << ") | Attempt #" << q.num(2)
                 << " | " << q.num(4) << "/" << q.num(5) << " | " << q.str(3) << "\n";
        }
        if(!found) cout << "No submitted exams yet.\n";
    }

    void adminMenu() {
        while(true) {
            cout << "\n========== ADMIN DASHBOARD ==========\n"
                 << "1. Add question\n2. View question bank\n3. View all results\n4. Logout\n";
            int choice=readInt("Select: ",1,4);
            if(choice==1) addQuestion();
            else if(choice==2) listQuestions();
            else if(choice==3) adminReports();
            else break;
        }
    }

    void studentMenu(const User& user) {
        while(true) {
            cout << "\n========== STUDENT DASHBOARD ==========\n"
                 << "1. Take examination\n2. View my results\n3. Logout\n";
            int choice=readInt("Select: ",1,3);
            if(choice==1) takeExam(user);
            else if(choice==2) myResults(user);
            else break;
        }
    }

public:
    ExaminationSystem():db("exam_system.db") {}
    void run() {
        while(true) {
            cout << "\n========== MAIN MENU ==========\n"
                 << "1. Login\n2. Student registration\n3. Exit\n";
            int choice=readInt("Select: ",1,3);
            if(choice==3) { cout << "Thank you for using the system.\n"; break; }
            if(choice==2) { registerStudent(); continue; }
            User user{};
            if(authenticate(user)) {
                if(user.role=="admin") adminMenu();
                else studentMenu(user);
            }
        }
    }
};

int main() {
    try {
        ExaminationSystem app;
        app.run();
    } catch(const exception& e) {
        cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
