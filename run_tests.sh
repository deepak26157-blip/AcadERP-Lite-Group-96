#!/bin/sh
# group-96 | AcadERP-Lite automated regression suite
set -u
cd "$(dirname "$0")" || exit 1
if [ ! -x ./acaderp ]; then echo 'Build first: make'; exit 1; fi
passed=0
failed=0
for input in tests/T*.in; do
    [ -f "$input" ] || { echo 'No tests found'; exit 1; }
    expected="${input%.in}.out"
    actual=$(mktemp) || exit 1
    if [ -f "$expected" ] && ./acaderp < "$input" > "$actual" && diff -u "$expected" "$actual"; then
        echo "PASS $input"; passed=$((passed+1))
    else
        echo "FAIL $input"; failed=$((failed+1))
    fi
    rm -f "$actual" tests/bonus_snapshot.tmp
done
echo "Passed: $passed | Failed: $failed"
[ "$failed" -eq 0 ]
