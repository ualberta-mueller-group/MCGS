# Run this script to test the closest .clang-tidy rules going up the file tree
# Will create ./temp/errors.txt, which lists missing errors, and false positives
# See ./scripts/tidy_test_template.cpp

rm -rf temp
mkdir temp

python3 scripts/replace.py scripts/tidy_test_template.cpp temp/tidy_test.cpp

# Get expected errors
cat temp/tidy_test.cpp | tr '[:upper:]' '[:lower:]' | grep -o -e 'yeserror_[0-9]\+' | grep -o -e '[0-9]\+' | sort -u -n > temp/expected_errors.txt

# Get actual errors
clang-tidy temp/tidy_test.cpp -- | tr '[:upper:]' '[:lower:]' | grep -o -e '_[0-9]\+' | tr -d '_' | sort -u -n > temp/got_errors.txt

# Interpret result
python3 scripts/interpret_errors.py temp/expected_errors.txt temp/got_errors.txt temp/errors.txt

echo "See temp/errors.txt to see 'missing' and 'false positive' errors"
