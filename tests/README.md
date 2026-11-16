
<!-- group-96 -->

# AcadERP-Lite Test Suite

This directory contains input (`.in`) and expected output (`.out`) files for automated regression testing.

## Running tests

From the repository root:

```bash
make
bash run_tests.sh
```

The test runner executes each input file and compares the actual program output against its matching expected output.

## Test conventions

- Each `Txx.in` file contains ERP commands.
- Each `Txx.out` file contains the expected response.
- Tests cover valid operations, invalid commands, error handling, and bonus functionality.
- Test T42 checks rejection of an oversized integer and continued processing of subsequent commands.

All tests should pass before submitting the project.
