<!-- group-96 | OOPD 2026 | AcadERP-Lite -->
# AcadERP-Lite — group-96

**Course:** OOPD (ECE, 2-credit project), IIIT-Delhi  
**Team:** Sukhpreet Singh (MT25151, sukhpreet25151@iiitd.ac.in) and Deepak Singh (MT26157, deepak26157@iiitd.ac.in)

A C++11 command-line Academic Records and Graduation Audit System implementing the professor's *Mini-ERP (Simplified)* specification. It supports regular, satisfactory/unsatisfactory, and thesis courses, exact integer GPA, graduation audits, course prerequisites, eligibility and error recovery.

## Build and run

From the project root on a clean Linux machine:

```sh
make
./acaderp < tests/T08.in
./run_tests.sh
make clean
```

Compilation uses `g++ -std=c++11 -Wall -Wextra -fno-exceptions`. The binary `acaderp` is created **locally by make** and is deliberately excluded from the submission ZIP and GitHub. Do not commit binaries, object files or executables. `run_tests.sh` reports each comparison and the total pass count.

## Layout

- `src/grp_96_erp.h` — constants, error codes, data types, interfaces.
- `src/grp_96_erp.cpp` — course/student/registration storage, GPA, graduation and eligibility logic.
- `src/grp_96_main.cpp` — command tokenization, validation, line numbering, output formatting.
- `Makefile` — build, test and clean targets.
- `run_tests.sh` — regression test runner.
- `tests/T01.in` through `T40.in`, paired with `.out` — specified cases T01–T23, edge-case tests T24–T30, and bonus-feature tests T31–T40. The five `tests/bonus_*.dat` files are malformed-snapshot fixtures used by the LOAD tests.
- `tests/sample.in`, `tests/sample.out` — official worked example from the assignment.

The naming prefix `grp_96_` is used for every C++ header and source file, and each applicable file starts with a `group-96` comment. Makefile and `.in`/`.out` files are exempt from the comment requirement.

## Input and output

Read commands from standard input, one command per line. Supported commands: `PROGRAM`, `COURSE`, `STUDENT`, `REGISTER`, `GRADE`, `CONVERT`, `GPA`, `AVG`, `GRAD`, `ELIGIBLE`, `TRANSCRIPT`, `SAVE`, `LOAD`. Successful changes are silent. Core queries output a single line; bonus TRANSCRIPT can output multiple lines; invalid commands print `ERROR <line> <CODE>` and do not modify the stored state. Blank and comment lines still count toward error line numbers.

## Step-by-step demonstration (Ubuntu / Linux)

This section is written for a first-time user or evaluator. No previous project setup is required. **Run all shell commands from the folder containing this README and `Makefile`.** The program uses standard input, not a graphical interface.

### Step 1 — Requirements and extracting the ZIP

Install a C++ compiler and Make (Ubuntu):

```sh
sudo apt update
sudo apt install -y build-essential unzip
```

Extract the submitted archive and enter its project directory:

```sh
unzip group-96.zip -d group-96
cd group-96
```

If you have cloned the GitHub repository instead, open the cloned directory in the terminal. To confirm you are in the correct folder:

```sh
ls
# You should see: README.md  Makefile  run_tests.sh  src/  tests/
```

### Step 2 — Compile

```sh
make
```

This builds `./acaderp` locally using `g++ -std=c++11 -Wall -Wextra -fno-exceptions`. The executable is intentionally **not included** in the submitted ZIP or GitHub repository.

### Step 3 — Run the official assignment example

```sh
./acaderp < tests/sample.in
```

Expected output:

```text
ERROR 14 E_GRADE_TYPE
GPA 2023CS01 8.71
ELIGIBLE 2023CS01 CS499 NO E4
GRAD 2023CS01 NO R1,R2,R4
GPA 2023CS01 8.05
AVG 2023CS01 B
GRAD 2023CS01 NO R1
ELIGIBLE 2023CS01 CS101 NO E2
```

To check it automatically against the professor's expected output:

```sh
./acaderp < tests/sample.in > sample_actual.out
diff -u tests/sample.out sample_actual.out
rm sample_actual.out
```

No output from `diff` means the files match exactly.

### Step 4 — Run the complete test suite

```sh
./run_tests.sh
```

The script checks `tests/T01.in` through `tests/T40.in` against their matching `.out` files. The expected final summary is:

```text
Passed: 40 | Failed: 0
```

You can also run a particular test, for example the integer GPA rounding case:

```sh
./acaderp < tests/T08.in
# Expected: GPA S1 8.13 (see tests/T08.out for the exact line)
```

### Step 5 — A complete custom demo (copy and paste)

Create a small input script directly in the terminal:

```sh
cat > my_demo.in <<'EOF'
PROGRAM 8 2 5.00 12 4
COURSE CS101 REGULAR 4 CORE -
COURSE HS201 REGULAR 2 ELECTIVE -
COURSE CS499 THESIS 2 CORE CS101
STUDENT S01 Asha
ELIGIBLE S01 CS499 1
REGISTER S01 CS101 1
GRADE S01 CS101 1 A
GPA S01
AVG S01
ELIGIBLE S01 CS499 2
REGISTER S01 HS201 1
GRADE S01 HS201 1 B
REGISTER S01 CS499 2
GRADE S01 CS499 2 S
GRAD S01
CONVERT S01 CS499 A-
GPA S01
GRAD S01
EOF
./acaderp < my_demo.in
```

Expected output (verified by running the program):

```text
ELIGIBLE S01 CS499 NO E4
GPA S01 10.00
AVG S01 A
ELIGIBLE S01 CS499 YES
GRAD S01 NO R1,R2,R4
GPA S01 9.25
GRAD S01 YES
```

**What this demonstrates:** The thesis is initially blocked by its CS101 prerequisite (E4). After CS101 receives an A, thesis eligibility becomes YES. Before conversion, thesis credits do not count and the thesis is not finalized, so graduation fails. Converting thesis progress `S` to letter `A-` adds its GPA/earned credits and makes graduation succeed. The example also demonstrates a CORE course, an ELECTIVE course, registration, grading, exact GPA, average grade and eligibility.

### Step 6 — Type commands interactively (optional)

```sh
./acaderp
```

Then type the commands below, **one per line**, pressing Enter after each:

```text
COURSE CS101 REGULAR 4 CORE -
STUDENT S02 Ravi
REGISTER S02 CS101 1
GRADE S02 CS101 1 A
GPA S02
AVG S02
GRAD S02
```

The last three commands print `GPA S02 10.00`, `AVG S02 A`, and a graduation audit listing unmet rules. Successful data-entry commands do not print anything. Press **Ctrl+D** on an empty line to finish input (EOF). Data is stored in memory for the current run unless you explicitly use the bonus `SAVE` command and later restore with `LOAD`.


### Step 7 — Bonus demo: transcript and save/load

The bonus commands are optional extensions. From the project root, after `make`, run:

```bash
./acaderp
```

Type these commands one per line:

```text
PROGRAM 4 0 5.00 24 0
COURSE CS101 REGULAR 4 CORE -
STUDENT S01 Asha
REGISTER S01 CS101 1
GRADE S01 CS101 1 A
TRANSCRIPT S01
SAVE my_records.dat
COURSE CS102 SU 0 ELECTIVE -
LOAD my_records.dat
GPA S01
ELIGIBLE S01 CS102 2
```

Expected output (the data-entry and persistence commands are silent on success):

```text
TRANSCRIPT S01
TERM 1 CS101 A 4
GPA S01 10.00
ELIGIBLE S01 CS102 NO E1
```

The last line proves `LOAD` restored the earlier snapshot, removing the course added after `SAVE`. Press **Ctrl+D** to exit. `my_records.dat` is a plain-text data file generated locally; it is not required in the submission. To remove it, run `rm -f my_records.dat`. To run the dedicated bonus tests, use `./run_tests.sh` (T31–T40). Do not include generated snapshots or compiled binaries in the submission ZIP.

### Step 8 — Common problems and cleanup

- **`make: command not found` or `g++: command not found`:** Install `build-essential` as shown in Step 1.
- **`No rule to make target`:** Run `pwd` and `ls`; move to the directory containing `Makefile`.
- **`./acaderp: No such file or directory`:** Run `make` first.
- **`Permission denied` for the test script:** Run `sh ./run_tests.sh` (or `chmod +x run_tests.sh`).
- **Unexpected `ERROR <line> E_PARSE`:** Check command spelling, number of arguments and term/credit values. Error line numbers count comments and blank lines too.
- **No output after `REGISTER`/`GRADE`:** This is expected on success; only queries and errors print output.
- **Clean locally generated files:** Run `make clean` and remove any personal demo files (`rm -f my_demo.in`). Never submit `acaderp`, `.o` files, or other binaries.

**Quick evaluator checklist:** `make` → `./acaderp < tests/sample.in` → `./run_tests.sh` → `make clean`.

## Data model and ownership

`ERPSystem` owns a fixed-size array of 50 `Course` objects and an array of up to 100 `Student*` pointers, initialized to null. Each accepted student is allocated using `new Student` in `ERPSystem::addStudent`. Each `Student` allocates a `Registration[MAX_REGS]` array (40 entries) with `new[]` in its constructor. `Student::~Student` calls `delete[] regs`; `ERPSystem::~ERPSystem` calls `delete` on each allocated student. No ownership is shared and no manual copy of a `Student` or `ERPSystem` is permitted. All storage is released on program exit. Registration records store course-array indexes rather than copies of courses. Search and aggregation use simple bounded loops. No STL containers, algorithm header, exceptions, or floating-point GPA arithmetic are used.

## Computations

The counting attempt for each course is its latest graded attempt; failed attempts can be repeated. Regular and converted FINAL thesis letter grades contribute to GPA; SU and unconverted theses do not. GPA uses a credit-weighted integer numerator `n`, denominator `d`, and exact half-up hundredths `(200*n + d)/(2*d)` (integer division). The average-grade bands use this rounded value. Graduation checks R1–R5 are evaluated in order and all failures are reported. Eligibility checks E1–E6 stop at the first failure.

## Assumptions / specification ambiguities

1. A `PROGRAM` command may appear only as the first non-comment, nonblank command; it cannot be repeated. Invalid PROGRAM lines do not change defaults.
2. Numeric syntax must be valid and within signed 32-bit integer range. Invalid numeric input, invalid terms outside 1–12, and invalid PROGRAM values produce `E_PARSE`. GPA thresholds must be between 0.00 and 10.00; nonnegative program credit requirements are accepted.
3. Invalid course type/category is treated as `E_BAD_COURSE`. Course codes, student rolls and names cannot be empty; overlength values produce `E_PARSE`.
4. An unknown or self-referential prerequisite produces `E_BAD_COURSE`. A prerequisite must have been declared earlier.
5. The course load in a term is the sum of credits for **all registrations made in that term**, including failed ones, as the PDF says "credits already registered". A 0-credit SU registration adds no load.
6. A thesis with progress `S` is IN_PROGRESS and not yet passed; only conversion makes it FINAL. Conversion to `F` is a failed attempt eligible for repetition. For a thesis progress `X`, registration may be repeated.
7. For `ELIGIBLE`, missing student or course prints `NO E1` rather than an `ERROR` line; `REGISTER` prints the corresponding unknown-entity error. This follows the PDF's E1 eligibility table.
8. `GRADE` validates the symbol and its course type before checking whether a matching registration exists. `CONVERT` validates its symbol/type before checking thesis state.
9. A thesis must be FINAL with a letter of C or better for R4; at least one such thesis suffices. All CORE courses in the catalogue are required for R2, including CORE courses added after a student enrolled.
10. A later registration in the same course is allowed only after a failed graded attempt. All registration attempts count toward the 40-registration limit, including failures.
11. Input may have trailing `#` comments after whitespace; these are ignored. This extends the PDF's explicit full-line comment support without changing normal input behavior.
12. Bonus `TRANSCRIPT <roll>` prints `TRANSCRIPT <roll>` followed by one `TERM <term> <course> <grade> <credits> [thesis_state]` line per registration, ordered by term and then registration order. `-` denotes an ungraded attempt; thesis entries include their state. An empty transcript prints only its header.
13. Bonus `SAVE <file>` writes a versioned plain-text snapshot and `LOAD <file>` replaces all in-memory data on success. Failed or malformed LOAD returns `E_PARSE` and preserves the old state. Invalid file paths and write failures also return `E_PARSE`; successful SAVE/LOAD print nothing. These bonus formats/error conventions are not specified by the assignment PDF and are documented here as project assumptions.

14. If a failed course is repeated in the **same term**, `GRADE` applies to the latest registration for that student, course and term.
15. `CONVERT` for a student who has never registered the thesis gives `E_THESIS_STATE` (there is no term in the command to report `E_NOT_REGISTERED` against).
16. `COURSE` with an unknown type or category keyword gives `E_BAD_COURSE`; a non-numeric credits value gives `E_PARSE`.

17. Saved snapshots start with a `# group-96` comment and a version marker (`ACADERP96 1`). LOAD also accepts older version-1 snapshots without the comment. LOAD validates course references, grades, thesis states, repeated-course history, per-term credit loads, prerequisite completion and thesis credit thresholds before replacing current data. The snapshot does not record the exact timestamps of grading versus registration, so it cannot independently prove every historical operation ordering; this is a documented limitation.

## Code structure

`grp_96_main.cpp` reads one line, splits it into at most 8 words with a fixed array, and `execute()` dispatches to one short handler per command (`cmdCourse`, `cmdRegister`, `cmdGrade`, ...). Every handler returns an `ErrorCode`; `main` checks it and prints `ERROR <line> <CODE>`. All data rules live in `ERPSystem` (`grp_96_erp.cpp`), so the parsing code never touches the arrays directly.

## Testing

Each `tests/Tnn.in` has a hand-computed `Tnn.out`. The tests cover T01–T23 from the assignment, plus E1 handling in `ELIGIBLE`, a thesis converted to F and repeated, an already-passed course, a same-term repeat, huge malformed numeric values, failed-command state preservation, and thesis conversion. T31–T40 cover transcript ordering, save/load round trips, missing files, malformed snapshots, invalid repeated-course histories, invalid saved credit loads, saved registrations whose prerequisite was never passed (T39), saved thesis registrations below the thesis-credit requirement (T40), write errors, and repeat-after-failure round trips. `./run_tests.sh` runs all 40 numbered cases and prints the pass count. The official sample is separately checked against `tests/sample.out`.

## LLM Usage Disclosure

**AI Tools Used:** OpenAI ChatGPT and Anthropic Claude

### Purpose of AI Assistance

AI tools were used during the development of AcadERP-Lite for understanding assignment requirements, implementation assistance, debugging, code review, testing, and documentation.

**ChatGPT (OpenAI)** assisted with:

- Understanding the project specifications and C++11 implementation requirements.
- Developing and reviewing ERP functionality, including GPA, graduation, eligibility, and thesis-related operations.
- Preparing automated tests, build scripts, and demonstration instructions.
- Implementing and validating optional features such as `TRANSCRIPT`, `SAVE`, and `LOAD`.
- Identifying potential implementation issues and suggesting improvements.

**Claude (Anthropic)** assisted with:

- Reviewing the implementation against the assignment requirements.
- Identifying and correcting issues involving repeated registrations and grade handling.
- Improving code organization and readability.
- Reviewing test coverage and validating file-handling functionality.
- Checking the final project structure and documentation.

### Build and Verification

The project was compiled and tested using Ubuntu with the GNU C++ compiler and C++11.

The automated test suite completed successfully, with **40 out of 40 tests passing**.

AI-assisted suggestions, generated code, and corrections were incorporated into the project development and verification workflow.

### Verification, responsibility, and limitations

AI assistance does not replace the group's responsibility to understand, test, and explain the submitted code. The included automated tests and example runs provide evidence of behavior but do not guarantee hidden-test or bonus marks. Both members should review the implementation and confirm that this disclosure accurately describes their own workflow before submitting. No AI-generated commits should be presented as independent work by a member who did not make them.
