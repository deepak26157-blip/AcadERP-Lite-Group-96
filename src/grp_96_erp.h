// group-96 | AcadERP-Lite | Sukhpreet Singh (MT25151), Deepak Singh (MT26157)
#ifndef GRP_96_ERP_H
#define GRP_96_ERP_H

#include <iostream>
#include <string>

const int MAX_STUDENTS = 100;
const int MAX_COURSES = 50;
const int MAX_REGS = 40;

enum ErrorCode {
    OK,
    E_PARSE,
    E_FULL,
    E_DUPLICATE,
    E_UNKNOWN_STUDENT,
    E_UNKNOWN_COURSE,
    E_BAD_COURSE,
    E_BAD_GRADE,
    E_GRADE_TYPE,
    E_NOT_REGISTERED,
    E_ALREADY_GRADED,
    E_THESIS_STATE,
    E_ALREADY_PASSED,
    E_ALREADY_REGISTERED,
    E_PREREQ,
    E_CREDIT_LIMIT,
    E_THESIS_NOT_ALLOWED,
    E_NO_GPA
};

enum CourseType { REGULAR, SU, THESIS };

enum ThesisState { NOT_THESIS, REGISTERED, IN_PROGRESS, FINAL, FAILED };

// Program-wide graduation rules. GPA threshold is kept in hundredths.
struct ProgramRules {
    int totalCredits;
    int electiveCredits;
    int minGpaHundredths;
    int maxLoad;
    int thesisCredits;

    ProgramRules()
        : totalCredits(160), electiveCredits(24), minGpaHundredths(500),
          maxLoad(24), thesisCredits(100) {}
};

class Course {
private:
    std::string code;
    CourseType type;
    int credits;
    bool isCore;   // false means ELECTIVE
    int prereq;    // index into courses[], or -1

public:
    Course();
    void initialize(const std::string& c, CourseType t, int cr, bool core, int pre);
    const std::string& getCode() const;
    CourseType getType() const;
    int getCredits() const;
    bool getIsCore() const;
    int getPrereq() const;
};

class Registration {
private:
    int course;           // index into courses[]
    int term;
    std::string grade;    // "" while ungraded
    ThesisState thesis;

public:
    Registration();
    void initialize(int c, int t, CourseType type);
    int getCourse() const;
    int getTerm() const;
    const std::string& getGrade() const;
    ThesisState getThesisState() const;
    void setGrade(const std::string& g);
    void setThesisState(ThesisState s);
};

class Student {
private:
    std::string roll;
    std::string name;
    Registration* regs;   // allocated with new Registration[MAX_REGS]
    int numRegs;

    Student(const Student&);              // copying is disabled
    Student& operator=(const Student&);

public:
    Student();
    ~Student();
    void initialize(const std::string& r, const std::string& n);
    const std::string& getRoll() const;
    const std::string& getName() const;
    int getNumRegs() const;
    ErrorCode addRegistration(int c, int t, CourseType type);
    Registration* getRegistration(int i);
    const Registration* getRegistration(int i) const;
};

class ERPSystem {
private:
    Course courses[MAX_COURSES];
    Student* students[MAX_STUDENTS];   // each allocated with new Student
    int numCourses;
    int numStudents;
    ProgramRules rules;

    ERPSystem(const ERPSystem&);       // copying is disabled
    ERPSystem& operator=(const ERPSystem&);

    // Helpers for LOAD; they are called on a temporary ERPSystem.
    ErrorCode loadCourses(std::istream& in);
    ErrorCode loadStudents(std::istream& in);
    ErrorCode loadRegistration(std::istream& in, Student* st);
    // Helpers for validateSnapshot; each checks registration i of student st.
    bool repeatLegal(const Student* st, int i) const;
    bool prereqPassedBefore(const Student* st, int i) const;
    bool thesisCreditsBefore(const Student* st, int i) const;

public:
    ERPSystem();
    ~ERPSystem();

    int findCourse(const std::string& code) const;
    int findStudent(const std::string& roll) const;

    ErrorCode addCourse(const std::string& code, CourseType type, int credits,
                        bool core, const std::string& prerequisite);
    ErrorCode addStudent(const std::string& roll, const std::string& name);
    ErrorCode registerCourse(int s, int c, int term);
    ErrorCode submitGrade(int s, int c, int term, const std::string& symbol);
    ErrorCode convertThesis(int s, int c, const std::string& letter);

    ErrorCode gpaHundredths(int s, int* result) const;
    ErrorCode averageGrade(int s, std::string* result) const;
    int checkEligibility(int s, int c, int term) const;   // 0 if eligible, else k for Ek
    ErrorCode graduationAudit(int s, std::string* result) const;

    ErrorCode setProgramRules(const ProgramRules& p);
    const ProgramRules& getProgramRules() const;
    ErrorCode transcript(int s, std::string* result) const;
    ErrorCode save(const std::string& filename) const;
    ErrorCode load(const std::string& filename);
    void swapWith(ERPSystem& other);
    bool validateSnapshot() const;
};

int gradePoints(const std::string& symbol);   // -1 if not a letter grade
bool isLetterGrade(const std::string& symbol);
bool isValidGrade(const std::string& symbol);
const char* errorCodeName(ErrorCode e);

#endif
