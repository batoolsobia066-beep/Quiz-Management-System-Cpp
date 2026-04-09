#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <thread>
#include <iomanip>
#include <ctime>
#include <cstdlib>
#include <limits>
#include <random>

using namespace std;

// ============================================
// QUESTION CLASS
// ============================================
class Question {
private:
    string questionText;
    vector<string> options;
    char correctAnswer;
    string difficulty;

public:
    Question() : correctAnswer(' '), difficulty("medium") {}

    Question(const string& text, const vector<string>& opts,
             char correct, const string& diff)
        : questionText(text), options(opts), correctAnswer(correct), difficulty(diff) {}

    string getQuestionText() const { return questionText; }
    vector<string> getOptions() const { return options; }
    char getCorrectAnswer() const { return correctAnswer; }
    string getDifficulty() const { return difficulty; }

    void setQuestionText(const string& text) { questionText = text; }
    void setOptions(const vector<string>& opts) { options = opts; }
    void setCorrectAnswer(char answer) { correctAnswer = toupper(answer); }
    void setDifficulty(const string& diff) { difficulty = diff; }

    bool isCorrect(char userAnswer) const {
        return toupper(userAnswer) == correctAnswer;
    }

    string toString() const {
        stringstream ss;
        ss << "Q: " << questionText << "\n";
        for (size_t i = 0; i < options.size(); ++i) {
            ss << (char)('A' + i) << ") " << options[i] << "\n";
        }
        return ss.str();
    }

    string toFileString() const {
        stringstream ss;
        ss << questionText << "|"
           << options[0] << "|" << options[1] << "|" << options[2] << "|" << options[3] << "|"
           << correctAnswer << "|" << difficulty;
        return ss.str();
    }

    static Question fromFileString(const string& line) {
        vector<string> parts;
        stringstream ss(line);
        string part;

        while (getline(ss, part, '|')) {
            parts.push_back(part);
        }

        if (parts.size() >= 7) { // FIXED
            vector<string> options(parts.begin() + 1, parts.begin() + 5);
            return Question(parts[0], options, parts[5][0], parts[6]);
        }

        return Question();
    }
};

// ============================================
// UTILITY FUNCTIONS
// ============================================
bool getValidInt(int& value, int min, int max) {
    string input;
    while (true) {
        getline(cin >> ws, input);
        if (input.empty()) return false;

        try {
            value = stoi(input);
            if (value >= min && value <= max) return true;
            cout << "? Enter number between " << min << " and " << max << ": ";
        } catch (...) {
            cout << "? Invalid input! Try again: ";
        }
    }
}

// Simplified stable input (better for Dev-C++)
char getInput(const string& prompt) {
    cout << prompt;
    char ans;
    cin >> ans;
    return toupper(ans);
}

void pauseScreen() {
    cout << "\nPress Enter to continue...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

// ============================================
// USER CLASS
// ============================================
class User {
private:
    string name;
    int score;
    int totalQuestions;
    double timeTaken;

public:
    User(const string& userName) : name(userName), score(0), totalQuestions(0), timeTaken(0) {}

    void startQuiz(const vector<Question>& quizQuestions) {
        vector<Question> shuffled = quizQuestions;

        shuffle(shuffled.begin(), shuffled.end(), default_random_engine(time(0)));

        int limit = min(10, (int)shuffled.size());
        totalQuestions = limit;

        auto start = chrono::steady_clock::now();

        for (int i = 0; i < limit; ++i) {
            cout << "\nQuestion " << i + 1 << ":\n";
            cout << shuffled[i].toString();

            char ans = getInput("Answer (A/B/C/D): ");

            if (shuffled[i].isCorrect(ans)) {
                cout << "? Correct\n";
                score++;
            } else {
                cout << "? Wrong. Correct: " << shuffled[i].getCorrectAnswer() << "\n";
            }
        }

        auto end = chrono::steady_clock::now();
        timeTaken = chrono::duration<double>(end - start).count();
    }

    void displayResults() const {
        cout << "\n===== RESULT =====\n";
        cout << "Name: " << name << "\n";
        cout << "Score: " << score << "/" << totalQuestions << "\n";
        cout << "Percentage: " << (score * 100.0 / totalQuestions) << "%\n";
        cout << "Time: " << timeTaken << " sec\n";
    }

    string getResultString() const {
        stringstream ss;
        time_t now = time(nullptr);
        ss << name << "|" << score << "|" << totalQuestions << "|"
           << (score * 100.0 / totalQuestions) << "|"
           << timeTaken << "|" << ctime(&now);
        return ss.str();
    }
};

// ============================================
// ADMIN CLASS
// ============================================
class Admin {
private:
    vector<Question>& questions;

public:
    Admin(vector<Question>& q) : questions(q) {}

    void addQuestion() {
        Question q;
        string text;
        cout << "Enter question: ";
        getline(cin >> ws, text);
        q.setQuestionText(text);

        vector<string> opts(4);
        for (int i = 0; i < 4; i++) {
            cout << "Option " << char('A' + i) << ": ";
            getline(cin >> ws, opts[i]);
        }
        q.setOptions(opts);

        char c;
        cout << "Correct (A-D): ";
        cin >> c;
        q.setCorrectAnswer(c);

        string diff;
        cout << "Difficulty: ";
        cin >> diff;
        q.setDifficulty(diff);

        questions.push_back(q);
    }

    void save(const string& file) {
        ofstream f(file);
        for (auto& q : questions)
            f << q.toFileString() << "\n";
    }

    void load(const string& file) {
        ifstream f(file);
        if (!f) return;

        questions.clear();
        string line;
        while (getline(f, line))
            questions.push_back(Question::fromFileString(line));
    }
};

// ============================================
// QUIZ SYSTEM
// ============================================
class Quiz {
private:
    vector<Question> questions;
    Admin admin;

public:
    Quiz() : admin(questions) {}

    void run() {
        admin.load("questions.txt");

        while (true) {
            cout << "\n1. Admin\n2. Start Quiz\n3. Exit\nChoice: ";
            int ch;
            cin >> ch;

            if (ch == 1) {
                admin.addQuestion();
                admin.save("questions.txt");
            }
            else if (ch == 2) {
                string name;
                cout << "Enter name: ";
                cin >> name;

                User u(name);
                u.startQuiz(questions);
                u.displayResults();

                ofstream f("results.txt", ios::app);
                f << u.getResultString();
            }
            else break;
        }
    }
};

// ============================================
// MAIN
// ============================================
int main() {
    Quiz q;
    q.run();
    return 0;
}