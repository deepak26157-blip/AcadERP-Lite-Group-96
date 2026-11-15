// group-96 | AcadERP-Lite | Sukhpreet Singh (MT25151), Deepak Singh (MT26157)
#include "grp_96_erp.h"
#include <cstdio>
#include <string>
#include <iostream>

// Fixed-size token array: no STL containers or stringstream.
static const int MAX_TOKENS = 8;

static bool split(const std::string& line, std::string tokens[MAX_TOKENS], int& count) {
    count = 0;
    std::size_t i = 0;
    while (i < line.size()) {
        while (i < line.size() && (line[i]==' ' || line[i]=='\t' || line[i]=='\r')) ++i;
        if (i == line.size() || line[i]=='#') break;
        if (count == MAX_TOKENS) return false;
        const std::size_t begin = i;
        while (i < line.size() && line[i]!=' ' && line[i]!='\t' && line[i]!='\r' && line[i]!='#') ++i;
        tokens[count++] = line.substr(begin, i-begin);
        if (i < line.size() && line[i]=='#') break;
    }
    return true;
}

static bool parseInt(const std::string& s, int& value) {
    if (s.empty()) return false;
    std::size_t i = 0;
    bool negative = false;
    if (s[i]=='+' || s[i]=='-') { negative = s[i]=='-'; ++i; }
    if (i == s.size()) return false;
    long long n = 0;
    for (; i < s.size(); ++i) {
        if (s[i]<'0' || s[i]>'9') return false;
        // Check before multiplication so even extremely long numbers cannot overflow.
        const int digit = s[i]-'0';
        if (n > (2147483648LL-digit)/10) return false;
        n = n*10 + digit;
    }
    if ((!negative && n>2147483647) || (negative && n>2147483648LL)) return false;
    value = static_cast<int>(negative ? -n : n);
    return true;
}

// Accept 5, 5.0 or 5.00 and store hundredths exactly.
static bool parseGpa(const std::string& s, int& hundredths) {
    if (s.empty()) return false;
    std::size_t dot = s.find('.');
    if (dot != std::string::npos && s.find('.',dot+1)!=std::string::npos) return false;
    std::string whole = dot==std::string::npos ? s : s.substr(0,dot);
    int n=0;
    if (!parseInt(whole,n) || n<0 || n>10) return false;
    int frac=0;
    if (dot!=std::string::npos) {
        const std::string digits=s.substr(dot+1);
        if (digits.empty() || digits.size()>2) return false;
        for (std::size_t i=0;i<digits.size();++i) {
            if (digits[i]<'0' || digits[i]>'9') return false;
            frac=frac*10+(digits[i]-'0');
        }
        if (digits.size()==1) frac*=10;
    }
    hundredths=n*100+frac;
    return hundredths<=1000;
}

static void printError(int line, ErrorCode e) {
    std::cout << "ERROR " << line << " " << errorCodeName(e) << '\n';
}

static void printGpa(int h) {
    std::cout << (h/100) << '.';
    if (h%100<10) std::cout << '0';
    std::cout << h%100;
}

// Tracks whether PROGRAM is still allowed (first command only, only once).
struct ParseState {
    bool seenCommand;
    bool seenProgram;
    ParseState() : seenCommand(false), seenProgram(false) {}
};

static ErrorCode cmdProgram(ERPSystem& erp, const std::string t[MAX_TOKENS], int n,
                            ParseState& st) {
    if (st.seenCommand || st.seenProgram || n != 6) return E_PARSE;
    ProgramRules p;
    if (!parseInt(t[1], p.totalCredits) || !parseInt(t[2], p.electiveCredits) ||
        !parseGpa(t[3], p.minGpaHundredths) || !parseInt(t[4], p.maxLoad) ||
        !parseInt(t[5], p.thesisCredits)) return E_PARSE;
    ErrorCode e = erp.setProgramRules(p);
    if (e == OK) st.seenProgram = true;
    return e;
}

static ErrorCode cmdCourse(ERPSystem& erp, const std::string t[MAX_TOKENS], int n) {
    if (n != 6) return E_PARSE;
    CourseType type;
    if (t[2] == "REGULAR") type = REGULAR;
    else if (t[2] == "SU") type = SU;
    else if (t[2] == "THESIS") type = THESIS;
    else return E_BAD_COURSE;
    bool core;
    if (t[4] == "CORE") core = true;
    else if (t[4] == "ELECTIVE") core = false;
    else return E_BAD_COURSE;
    int credits = 0;
    if (!parseInt(t[3], credits)) return E_PARSE;
    return erp.addCourse(t[1], type, credits, core, t[5]);
}

static ErrorCode cmdStudent(ERPSystem& erp, const std::string t[MAX_TOKENS], int n) {
    if (n != 3) return E_PARSE;
    return erp.addStudent(t[1], t[2]);
}

// REGISTER <roll> <course> <term>
static ErrorCode cmdRegister(ERPSystem& erp, const std::string t[MAX_TOKENS], int n) {
    if (n != 4) return E_PARSE;
    int term = 0;
    if (!parseInt(t[3], term) || term < 1 || term > 12) return E_PARSE;
    int s = erp.findStudent(t[1]);
    if (s < 0) return E_UNKNOWN_STUDENT;
    int c = erp.findCourse(t[2]);
    if (c < 0) return E_UNKNOWN_COURSE;
    return erp.registerCourse(s, c, term);
}

// ELIGIBLE <roll> <course> <term>: prints YES or NO Ek; never an ERROR for E1.
static ErrorCode cmdEligible(ERPSystem& erp, const std::string t[MAX_TOKENS], int n) {
    if (n != 4) return E_PARSE;
    int term = 0;
    if (!parseInt(t[3], term) || term < 1 || term > 12) return E_PARSE;
    int s = erp.findStudent(t[1]);
    int c = erp.findCourse(t[2]);
    int e = erp.checkEligibility(s, c, term);
    std::cout << "ELIGIBLE " << t[1] << ' ' << t[2] << ' ';
    if (e == 0) std::cout << "YES\n";
    else std::cout << "NO E" << e << '\n';
    return OK;
}

static ErrorCode cmdGrade(ERPSystem& erp, const std::string t[MAX_TOKENS], int n) {
    if (n != 5) return E_PARSE;
    int term = 0;
    if (!parseInt(t[3], term) || term < 1 || term > 12) return E_PARSE;
    int s = erp.findStudent(t[1]);
    if (s < 0) return E_UNKNOWN_STUDENT;
    int c = erp.findCourse(t[2]);
    if (c < 0) return E_UNKNOWN_COURSE;
    return erp.submitGrade(s, c, term, t[4]);
}

static ErrorCode cmdConvert(ERPSystem& erp, const std::string t[MAX_TOKENS], int n) {
    if (n != 4) return E_PARSE;
    int s = erp.findStudent(t[1]);
    if (s < 0) return E_UNKNOWN_STUDENT;
    int c = erp.findCourse(t[2]);
    if (c < 0) return E_UNKNOWN_COURSE;
    return erp.convertThesis(s, c, t[3]);
}

static ErrorCode cmdGpa(ERPSystem& erp, const std::string t[MAX_TOKENS], int n) {
    if (n != 2) return E_PARSE;
    int s = erp.findStudent(t[1]);
    if (s < 0) return E_UNKNOWN_STUDENT;
    int h = 0;
    ErrorCode e = erp.gpaHundredths(s, &h);
    if (e != OK) return e;
    std::cout << "GPA " << t[1] << ' ';
    printGpa(h);
    std::cout << '\n';
    return OK;
}

// AVG <roll> and GRAD <roll>: both print "<CMD> <roll> <result>".
static ErrorCode cmdAvgOrGrad(ERPSystem& erp, const std::string t[MAX_TOKENS], int n) {
    if (n != 2) return E_PARSE;
    int s = erp.findStudent(t[1]);
    if (s < 0) return E_UNKNOWN_STUDENT;
    std::string result;
    ErrorCode e = (t[0] == "AVG") ? erp.averageGrade(s, &result)
                                  : erp.graduationAudit(s, &result);
    if (e != OK) return e;
    std::cout << t[0] << ' ' << t[1] << ' ' << result << '\n';
    return OK;
}

static ErrorCode cmdTranscript(ERPSystem& erp, const std::string t[MAX_TOKENS], int n) {
    if (n!=2) return E_PARSE;
    int s=erp.findStudent(t[1]);
    if (s<0) return E_UNKNOWN_STUDENT;
    std::string output;
    ErrorCode e=erp.transcript(s,&output);
    if (e!=OK) return e;
    std::cout << output << '\n';
    return OK;
}

static ErrorCode cmdPersistence(ERPSystem& erp, const std::string t[MAX_TOKENS], int n) {
    if (n!=2 || t[1].empty()) return E_PARSE;
    if (t[0]=="SAVE") return erp.save(t[1]);
    return erp.load(t[1]);
}

static ErrorCode execute(ERPSystem& erp, const std::string t[MAX_TOKENS], int n,
                         ParseState& st) {
    const std::string& cmd = t[0];
    if (cmd == "PROGRAM") return cmdProgram(erp, t, n, st);
    st.seenCommand = true;
    if (cmd == "TRANSCRIPT") return cmdTranscript(erp, t, n);
    if (cmd == "SAVE" || cmd == "LOAD") return cmdPersistence(erp, t, n);
    if (cmd == "COURSE") return cmdCourse(erp, t, n);
    if (cmd == "STUDENT") return cmdStudent(erp, t, n);
    if (cmd == "REGISTER") return cmdRegister(erp, t, n);
    if (cmd == "ELIGIBLE") return cmdEligible(erp, t, n);
    if (cmd == "GRADE") return cmdGrade(erp, t, n);
    if (cmd == "CONVERT") return cmdConvert(erp, t, n);
    if (cmd == "GPA") return cmdGpa(erp, t, n);
    if (cmd == "AVG" || cmd == "GRAD") return cmdAvgOrGrad(erp, t, n);
    return E_PARSE;
}

int main() {
    ERPSystem erp;
    std::string line;
    int lineNumber=0;
    ParseState state;
    while (std::getline(std::cin,line)) {
        ++lineNumber;
        std::string tokens[MAX_TOKENS];
        int count=0;
        if (!split(line,tokens,count)) {
            printError(lineNumber,E_PARSE);
            continue;
        }
        if (count==0) continue;
        ErrorCode e=execute(erp,tokens,count,state);
        if (e!=OK) printError(lineNumber,e);
    }
    return 0;
}
