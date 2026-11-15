// group-96 | AcadERP-Lite | Sukhpreet Singh (MT25151), Deepak Singh (MT26157)
#include "grp_96_erp.h"
#include <cstdio>
#include <fstream>  // Allowed for SAVE/LOAD bonus only.
#include <cstdlib>

Course::Course() : type(REGULAR), credits(0), isCore(false), prereq(-1) {}
void Course::initialize(const std::string& c, CourseType t, int cr, bool core, int pre) {
    code=c; type=t; credits=cr; isCore=core; prereq=pre;
}
const std::string& Course::getCode() const { return code; }
CourseType Course::getType() const { return type; }
int Course::getCredits() const { return credits; }
bool Course::getIsCore() const { return isCore; }
int Course::getPrereq() const { return prereq; }

Registration::Registration() : course(-1), term(0), thesis(NOT_THESIS) {}
void Registration::initialize(int c, int t, CourseType type) {
    course=c; term=t; grade=""; thesis=(type==THESIS ? REGISTERED : NOT_THESIS);
}
int Registration::getCourse() const { return course; }
int Registration::getTerm() const { return term; }
const std::string& Registration::getGrade() const { return grade; }
ThesisState Registration::getThesisState() const { return thesis; }
void Registration::setGrade(const std::string& g) { grade=g; }
void Registration::setThesisState(ThesisState s) { thesis=s; }

Student::Student() : regs(new Registration[MAX_REGS]), numRegs(0) {}
Student::~Student() { delete[] regs; }
void Student::initialize(const std::string& r, const std::string& n) {
    roll=r; name=n; numRegs=0;
}
const std::string& Student::getRoll() const { return roll; }
const std::string& Student::getName() const { return name; }
int Student::getNumRegs() const { return numRegs; }
ErrorCode Student::addRegistration(int c, int t, CourseType type) {
    if (!regs || numRegs>=MAX_REGS) return E_FULL;
    regs[numRegs].initialize(c,t,type); ++numRegs; return OK;
}
Registration* Student::getRegistration(int i) {
    return (regs && i>=0 && i<numRegs) ? &regs[i] : NULL;
}
const Registration* Student::getRegistration(int i) const {
    return (regs && i>=0 && i<numRegs) ? &regs[i] : NULL;
}

ERPSystem::ERPSystem() : numCourses(0), numStudents(0) {
    for (int i=0;i<MAX_STUDENTS;++i) students[i]=NULL;
}
ERPSystem::~ERPSystem() {
    for (int i=0;i<numStudents;++i) delete students[i];
}
int ERPSystem::findCourse(const std::string& code) const {
    for (int i=0;i<numCourses;++i) if (courses[i].getCode()==code) return i;
    return -1;
}
int ERPSystem::findStudent(const std::string& roll) const {
    for (int i=0;i<numStudents;++i) if (students[i]->getRoll()==roll) return i;
    return -1;
}
ErrorCode ERPSystem::addCourse(const std::string& code, CourseType type,
                              int credits, bool core, const std::string& prerequisite) {
    if (code.empty() || code.size()>10) return E_PARSE;
    if (findCourse(code)>=0) return E_DUPLICATE;
    if (numCourses>=MAX_COURSES) return E_FULL;
    if (credits<0 || credits>24 || (credits==0 && type!=SU)) return E_BAD_COURSE;
    int pre=-1;
    if (prerequisite!="-") {
        pre=findCourse(prerequisite);
        if (pre<0 || prerequisite==code) return E_BAD_COURSE;
    }
    courses[numCourses].initialize(code,type,credits,core,pre);
    ++numCourses; return OK;
}
ErrorCode ERPSystem::addStudent(const std::string& roll, const std::string& name) {
    if (roll.empty() || roll.size()>10 || name.empty() || name.size()>30) return E_PARSE;
    if (findStudent(roll)>=0) return E_DUPLICATE;
    if (numStudents>=MAX_STUDENTS) return E_FULL;
    Student* s=new Student;
    if (!s) return E_FULL;
    s->initialize(roll,name);
    students[numStudents++]=s; return OK;
}

int gradePoints(const std::string& s) {
    if (s=="A") return 10;
    if (s=="A-") return 9;
    if (s=="B") return 8;
    if (s=="B-") return 7;
    if (s=="C") return 6;
    if (s=="C-") return 5;
    if (s=="D") return 4;
    if (s=="F") return 0;
    return -1;
}
bool isLetterGrade(const std::string& s) { return gradePoints(s)>=0; }
bool isValidGrade(const std::string& s) {
    return isLetterGrade(s) || s=="S" || s=="X";
}

// A counting attempt is the latest attempt with a grade. An in-progress
// thesis has a progress grade S but does not yet earn credits or GPA points.
static const Registration* countingAttempt(const Student* s, int course) {
    const Registration* latest=NULL;
    for (int i=0;i<s->getNumRegs();++i) {
        const Registration* r=s->getRegistration(i);
        if (r && r->getCourse()==course && !r->getGrade().empty()) latest=r;
    }
    return latest;
}
static bool passed(const Registration* r, CourseType type) {
    if (!r) return false;
    if (type==SU) return r->getGrade()=="S";
    if (type==THESIS) return r->getThesisState()==FINAL && gradePoints(r->getGrade())>0;
    return gradePoints(r->getGrade())>0;
}
static int earnedCredits(const Student* s, const Course* courses, int count) {
    int sum=0;
    for (int c=0;c<count;++c) {
        if (passed(countingAttempt(s,c),courses[c].getType())) sum+=courses[c].getCredits();
    }
    return sum;
}
static Registration* registrationFor(Student* s, int c, int term) {
    Registration* found=NULL;
    for (int i=0;i<s->getNumRegs();++i) {
        Registration* r=s->getRegistration(i);
        if (r && r->getCourse()==c && r->getTerm()==term) found=r;
    }
    return found;
}
static Registration* latestThesis(Student* s, int c) {
    Registration* latest=NULL;
    for (int i=0;i<s->getNumRegs();++i) {
        Registration* r=s->getRegistration(i);
        if (r && r->getCourse()==c) latest=r;
    }
    return latest;
}

int ERPSystem::checkEligibility(int s, int c, int term) const {
    if (s<0 || s>=numStudents || c<0 || c>=numCourses) return 1;
    const Student* student=students[s];
    const Course& course=courses[c];
    const Registration* latest=countingAttempt(student,c);
    if (passed(latest,course.getType())) return 2;
    int load=0;
    for (int i=0;i<student->getNumRegs();++i) {
        const Registration* r=student->getRegistration(i);
        if (r->getCourse()==c && (r->getGrade().empty() ||
            (course.getType()==THESIS && r->getThesisState()==IN_PROGRESS))) return 3;
        if (r->getTerm()==term) load+=courses[r->getCourse()].getCredits();
    }
    // A graded failed attempt is required before another registration.
    if (!latest) {
        for (int i=0;i<student->getNumRegs();++i)
            if (student->getRegistration(i)->getCourse()==c) return 3;
    }
    int pre=course.getPrereq();
    if (pre>=0 && !passed(countingAttempt(student,pre),courses[pre].getType())) return 4;
    if (load+course.getCredits()>rules.maxLoad) return 5;
    if (course.getType()==THESIS && earnedCredits(student,courses,numCourses)<rules.thesisCredits) return 6;
    return 0;
}
ErrorCode ERPSystem::registerCourse(int s, int c, int term) {
    if (s<0 || s>=numStudents) return E_UNKNOWN_STUDENT;
    if (c<0 || c>=numCourses) return E_UNKNOWN_COURSE;
    if (term<1 || term>12) return E_PARSE;
    int e=checkEligibility(s,c,term);
    const ErrorCode errors[]={OK,E_UNKNOWN_STUDENT,E_ALREADY_PASSED,
        E_ALREADY_REGISTERED,E_PREREQ,E_CREDIT_LIMIT,E_THESIS_NOT_ALLOWED};
    if (e!=0) return errors[e];
    return students[s]->addRegistration(c,term,courses[c].getType());
}
ErrorCode ERPSystem::submitGrade(int s, int c, int term, const std::string& symbol) {
    if (s<0 || s>=numStudents) return E_UNKNOWN_STUDENT;
    if (c<0 || c>=numCourses) return E_UNKNOWN_COURSE;
    if (!isValidGrade(symbol)) return E_BAD_GRADE;
    if ((courses[c].getType()==REGULAR) != isLetterGrade(symbol)) return E_GRADE_TYPE;
    Registration* r=registrationFor(students[s],c,term);
    if (!r) return E_NOT_REGISTERED;
    if (!r->getGrade().empty()) return E_ALREADY_GRADED;
    r->setGrade(symbol);
    if (courses[c].getType()==THESIS)
        r->setThesisState(symbol=="S" ? IN_PROGRESS : FAILED);
    return OK;
}
ErrorCode ERPSystem::convertThesis(int s, int c, const std::string& letter) {
    if (s<0 || s>=numStudents) return E_UNKNOWN_STUDENT;
    if (c<0 || c>=numCourses) return E_UNKNOWN_COURSE;
    if (!isValidGrade(letter)) return E_BAD_GRADE;
    if (courses[c].getType()!=THESIS || !isLetterGrade(letter)) return E_GRADE_TYPE;
    Registration* r=latestThesis(students[s],c);
    if (!r || r->getThesisState()==REGISTERED || r->getThesisState()==FAILED)
        return E_THESIS_STATE;
    if (r->getThesisState()==FINAL) return E_ALREADY_GRADED;
    if (r->getThesisState()!=IN_PROGRESS) return E_THESIS_STATE;
    r->setGrade(letter); r->setThesisState(FINAL); return OK;
}
ErrorCode ERPSystem::gpaHundredths(int s, int* result) const {
    if (s<0 || s>=numStudents) return E_UNKNOWN_STUDENT;
    if (!result) return E_PARSE;
    long long numerator=0, denominator=0;
    for (int c=0;c<numCourses;++c) {
        const Registration* r=countingAttempt(students[s],c);
        if (!r || courses[c].getType()==SU) continue;
        if (courses[c].getType()==THESIS && r->getThesisState()!=FINAL) continue;
        int gp=gradePoints(r->getGrade());
        if (gp<0) continue;
        numerator+=courses[c].getCredits()*gp;
        denominator+=courses[c].getCredits();
    }
    if (denominator==0) return E_NO_GPA;
    *result=static_cast<int>((200*numerator+denominator)/(2*denominator));
    return OK;
}
ErrorCode ERPSystem::averageGrade(int s, std::string* result) const {
    if (!result) return E_PARSE;
    int h=0;
    ErrorCode e=gpaHundredths(s,&h);
    if (e!=OK) return e;
    *result=h>=950?"A":h>=850?"A-":h>=750?"B":h>=650?"B-":
            h>=550?"C":h>=450?"C-":h>=200?"D":"F";
    return OK;
}
ErrorCode ERPSystem::graduationAudit(int s, std::string* result) const {
    if (s<0 || s>=numStudents) return E_UNKNOWN_STUDENT;
    if (!result) return E_PARSE;
    const Student* st=students[s];
    bool r1=earnedCredits(st,courses,numCourses)>=rules.totalCredits;
    bool r2=true, r3=false, r4=false, r5=false;
    int elective=0;
    for (int c=0;c<numCourses;++c) {
        const Registration* r=countingAttempt(st,c);
        bool p=passed(r,courses[c].getType());
        if (courses[c].getIsCore() && !p) r2=false;
        if (!courses[c].getIsCore() && p) elective+=courses[c].getCredits();
        if (courses[c].getType()==THESIS && r && r->getThesisState()==FINAL &&
            gradePoints(r->getGrade())>=6) r4=true;
    }
    r3=elective>=rules.electiveCredits;
    int h=0;
    ErrorCode e=gpaHundredths(s,&h);
    if (e!=OK && e!=E_NO_GPA) return e;
    r5=(e==OK && h>=rules.minGpaHundredths);
    if (r1 && r2 && r3 && r4 && r5) { *result="YES"; return OK; }
    *result="NO";
    const bool rulesPassed[]={r1,r2,r3,r4,r5};
    for (int i=0;i<5;++i) if (!rulesPassed[i]) {
        *result+=(*result=="NO" ? " " : ",");
        *result+="R"; *result+=static_cast<char>('1'+i);
    }
    return OK;
}
ErrorCode ERPSystem::setProgramRules(const ProgramRules& p) {
    if (p.totalCredits<0 || p.electiveCredits<0 || p.minGpaHundredths<0 ||
        p.minGpaHundredths>1000 || p.maxLoad<0 || p.thesisCredits<0) return E_PARSE;
    rules=p; return OK;
}
const ProgramRules& ERPSystem::getProgramRules() const { return rules; }
const char* errorCodeName(ErrorCode e) {
    static const char* names[]={"OK","E_PARSE","E_FULL","E_DUPLICATE",
        "E_UNKNOWN_STUDENT","E_UNKNOWN_COURSE","E_BAD_COURSE","E_BAD_GRADE",
        "E_GRADE_TYPE","E_NOT_REGISTERED","E_ALREADY_GRADED","E_THESIS_STATE",
        "E_ALREADY_PASSED","E_ALREADY_REGISTERED","E_PREREQ","E_CREDIT_LIMIT",
        "E_THESIS_NOT_ALLOWED","E_NO_GPA"};
    if (e<OK || e>E_NO_GPA) return "E_PARSE";
    return names[e];
}

// Bonus output is ordered by term; registration insertion order breaks ties.
ErrorCode ERPSystem::transcript(int s, std::string* result) const {
    if (s<0 || s>=numStudents) return E_UNKNOWN_STUDENT;
    if (!result) return E_PARSE;
    *result="TRANSCRIPT "+students[s]->getRoll();
    const Student* student=students[s];
    for (int term=1;term<=12;++term) {
        for (int i=0;i<student->getNumRegs();++i) {
            const Registration* r=student->getRegistration(i);
            if (r->getTerm()!=term) continue;
            const Course& c=courses[r->getCourse()];
            char number[32];
            std::sprintf(number,"%d",term);
            *result+="\nTERM "+std::string(number)+" "+c.getCode();
            *result+=" "+(r->getGrade().empty()?"-":r->getGrade());
            std::sprintf(number,"%d",c.getCredits());
            *result+=" "+std::string(number);
            if (c.getType()==THESIS) {
                const char* states[]={"NOT_THESIS","REGISTERED","IN_PROGRESS","FINAL","FAILED"};
                *result+=" "+std::string(states[r->getThesisState()]);
            }
        }
    }
    return OK;
}

// Plain-text, versioned snapshot. Output file paths are supplied by the user.
ErrorCode ERPSystem::save(const std::string& filename) const {
    std::ofstream out(filename.c_str(),std::ios::out|std::ios::trunc);
    if (!out) return E_PARSE;
    out << "# group-96 | AcadERP-Lite snapshot\n";
    out << "ACADERP96 1\n";
    out << rules.totalCredits << ' ' << rules.electiveCredits << ' '
        << rules.minGpaHundredths << ' ' << rules.maxLoad << ' '
        << rules.thesisCredits << '\n';
    out << numCourses << '\n';
    for (int c=0;c<numCourses;++c) {
        const Course& x=courses[c];
        out << x.getCode() << ' ' << static_cast<int>(x.getType()) << ' '
            << x.getCredits() << ' ' << static_cast<int>(x.getIsCore()) << ' '
            << (x.getPrereq()<0 ? "-" : courses[x.getPrereq()].getCode()) << '\n';
    }
    out << numStudents << '\n';
    for (int i=0;i<numStudents;++i) {
        const Student* st=students[i];
        out << st->getRoll() << ' ' << st->getName() << ' ' << st->getNumRegs() << '\n';
        for (int j=0;j<st->getNumRegs();++j) {
            const Registration* r=st->getRegistration(j);
            out << r->getCourse() << ' ' << r->getTerm() << ' '
                << (r->getGrade().empty()?"-":r->getGrade()) << ' '
                << static_cast<int>(r->getThesisState()) << '\n';
        }
    }
    out.flush();
    return out.good()?OK:E_PARSE;
}

void ERPSystem::swapWith(ERPSystem& other) {
    for (int i=0;i<MAX_COURSES;++i) {
        Course tmp=courses[i]; courses[i]=other.courses[i]; other.courses[i]=tmp;
    }
    for (int i=0;i<MAX_STUDENTS;++i) {
        Student* tmp=students[i]; students[i]=other.students[i]; other.students[i]=tmp;
    }
    int nc=numCourses; numCourses=other.numCourses; other.numCourses=nc;
    int ns=numStudents; numStudents=other.numStudents; other.numStudents=ns;
    ProgramRules tmp=rules; rules=other.rules; other.rules=tmp;
}

// Latest graded registration of 'course' among the first 'end' registrations, or NULL.
static const Registration* latestGradedBefore(const Student* st, int end, int course) {
    const Registration* latest=NULL;
    for (int j=0;j<end;++j) {
        const Registration* r=st->getRegistration(j);
        if (r->getCourse()==course && !r->getGrade().empty()) latest=r;
    }
    return latest;
}

// A repeat requires every earlier attempt of the same course to be a finished failure.
bool ERPSystem::repeatLegal(const Student* st, int i) const {
    const int c=st->getRegistration(i)->getCourse();
    for (int j=0;j<i;++j) {
        const Registration* earlier=st->getRegistration(j);
        if (earlier->getCourse()!=c) continue;
        if (earlier->getGrade().empty() ||
            (courses[c].getType()==THESIS && earlier->getThesisState()==IN_PROGRESS) ||
            passed(earlier,courses[c].getType())) return false;
    }
    return true;
}

// The prerequisite, if any, must have a passed counting attempt registered earlier.
bool ERPSystem::prereqPassedBefore(const Student* st, int i) const {
    const int pre=courses[st->getRegistration(i)->getCourse()].getPrereq();
    if (pre<0) return true;
    return passed(latestGradedBefore(st,i,pre),courses[pre].getType());
}

// A thesis requires enough earned credits from earlier attempts.
bool ERPSystem::thesisCreditsBefore(const Student* st, int i) const {
    if (courses[st->getRegistration(i)->getCourse()].getType()!=THESIS) return true;
    int credits=0;
    for (int k=0;k<numCourses;++k) {
        if (passed(latestGradedBefore(st,i,k),courses[k].getType()))
            credits+=courses[k].getCredits();
    }
    return credits>=rules.thesisCredits;
}

// Validate a saved registration history without changing the live ERP.
// A snapshot records registration order, but not the time grades were entered.
// Therefore we check prerequisites and thesis credits against preceding
// registrations, without inventing a grading timestamp.
bool ERPSystem::validateSnapshot() const {
    for (int s=0;s<numStudents;++s) {
        const Student* st=students[s];
        int termCredits[13]={0};
        for (int i=0;i<st->getNumRegs();++i) {
            const Registration* r=st->getRegistration(i);
            const int c=r->getCourse();
            if (c<0 || c>=numCourses || r->getTerm()<1 || r->getTerm()>12)
                return false;
            termCredits[r->getTerm()]+=courses[c].getCredits();
            if (termCredits[r->getTerm()]>rules.maxLoad) return false;
            if (!repeatLegal(st,i) || !prereqPassedBefore(st,i) ||
                !thesisCreditsBefore(st,i)) return false;
        }
    }
    return true;
}

// A snapshot registration must have a grade and thesis state that fit its course type.
static bool snapshotStateValid(CourseType type, int state, const std::string& grade) {
    if (type!=THESIS) {
        if (state!=NOT_THESIS) return false;
        if (grade=="-") return true;
        return type==SU ? (grade=="S" || grade=="X") : isLetterGrade(grade);
    }
    if (state==NOT_THESIS) return false;
    if (state==REGISTERED) return grade=="-";
    if (state==IN_PROGRESS) return grade=="S";
    if (state==FAILED) return grade=="X";
    return isLetterGrade(grade);   // FINAL
}

// Reads one registration line of a snapshot into st.
ErrorCode ERPSystem::loadRegistration(std::istream& in, Student* st) {
    std::string grade;
    int c=0, term=0, state=0;
    if (!(in>>c>>term>>grade>>state) || c<0 || c>=numCourses ||
        term<1 || term>12 || state<0 || state>4) return E_PARSE;
    const CourseType type=courses[c].getType();
    if (!snapshotStateValid(type,state,grade)) return E_PARSE;
    ErrorCode e=st->addRegistration(c,term,type);
    if (e!=OK) return E_PARSE;
    Registration* r=st->getRegistration(st->getNumRegs()-1);
    if (grade!="-") r->setGrade(grade);
    r->setThesisState(static_cast<ThesisState>(state));
    return OK;
}

// Reads the course section of a snapshot.
ErrorCode ERPSystem::loadCourses(std::istream& in) {
    int nc=0;
    if (!(in>>nc) || nc<0 || nc>MAX_COURSES) return E_PARSE;
    for (int i=0;i<nc;++i) {
        std::string code, pre;
        int type=0, credits=0, core=0;
        if (!(in>>code>>type>>credits>>core>>pre) || type<0 || type>2 ||
            (core!=0 && core!=1)) return E_PARSE;
        ErrorCode e=addCourse(code,static_cast<CourseType>(type),credits,core==1,pre);
        if (e!=OK) return E_PARSE;
    }
    return OK;
}

// Reads the student section (and each student's registrations) of a snapshot.
ErrorCode ERPSystem::loadStudents(std::istream& in) {
    int ns=0;
    if (!(in>>ns) || ns<0 || ns>MAX_STUDENTS) return E_PARSE;
    for (int i=0;i<ns;++i) {
        std::string roll, name;
        int nr=0;
        if (!(in>>roll>>name>>nr) || nr<0 || nr>MAX_REGS) return E_PARSE;
        ErrorCode e=addStudent(roll,name);
        if (e!=OK) return E_PARSE;
        for (int j=0;j<nr;++j) {
            e=loadRegistration(in,students[i]);
            if (e!=OK) return E_PARSE;
        }
    }
    return OK;
}

// Parse into a temporary object; an invalid LOAD leaves the current ERP intact.
ErrorCode ERPSystem::load(const std::string& filename) {
    std::ifstream in(filename.c_str());
    if (!in) return E_PARSE;
    std::string magic;
    int version=0;
    if (in.peek()=='#') {
        std::string comment;
        std::getline(in,comment);
    }
    if (!(in>>magic>>version) || magic!="ACADERP96" || version!=1) return E_PARSE;
    ERPSystem temp;
    ProgramRules p;
    if (!(in>>p.totalCredits>>p.electiveCredits>>p.minGpaHundredths
          >>p.maxLoad>>p.thesisCredits)) return E_PARSE;
    ErrorCode e=temp.setProgramRules(p);
    if (e!=OK) return e;
    e=temp.loadCourses(in);
    if (e!=OK) return e;
    e=temp.loadStudents(in);
    if (e!=OK) return e;
    std::string extra;
    if (in>>extra) return E_PARSE;
    if (!in.eof()) return E_PARSE;
    if (!temp.validateSnapshot()) return E_PARSE;
    swapWith(temp);
    return OK;
}
