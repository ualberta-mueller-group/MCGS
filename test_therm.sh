cmake -B build
cmake --build build -j11
./MCGS --run-tests --test-timeout 0 --test-dir input/autotests/thermographs
python3 create-table.py out.csv -o out.html
