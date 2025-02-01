rm errors.txt expected_errors.txt got_errors.txt
python3 compile.py
# Get expected errors
cat style_test.cpp | tr '[:upper:]' '[:lower:]' | grep -o -e 'yeserror_[0-9]\+' | grep -o -e '[0-9]\+' | sort -u -n > expected_errors.txt
make style 2>&1 | tr '[:upper:]' '[:lower:]' | grep -o -e '_[0-9]\+' | tr -d '_' | sort -u -n > got_errors.txt
python3 interpret_errors.py

