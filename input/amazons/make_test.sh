rm -f test.*
python3 ../../utils/random-grid.py | unique_lines > test.raw
sed -i 's/$/ {B, W}/g' test.raw
echo -e "{version 1.3}\n[amazons]" | cat - test.raw > test.test
rm -f test.raw
