rm -f test.*
python3 ../../utils/random-grid.py | unique_lines > test.raw

cat test.raw | cgstring > test.temp
echo "" >> test.raw
cat test.temp >> test.raw
rm -f test.temp

#sed -i 's/$/ {B, W}/g' test.raw
echo -e "{version 1.3}\n[toppling_dominoes]" | cat - test.raw > test.test
rm -f test.raw
