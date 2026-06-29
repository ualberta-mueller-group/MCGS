sudo renice -n -19 -p $$

build_mcgs() {
    cmake -B build
    cmake --build build -j11
}

run_test() {
    ./MCGS --clear-tt --tt-sumgame-idx-bits 29 --db-file-load "$1" --run-tests --test-timeout 10000 --test-dir perf_tests --out-file "$2".csv
}

compare() {
    python3 create-table.py "$1".csv --compare-to "$2".csv -o "$1"_VS_"$2".html
}

make_db() {
    ./MCGS --db-file-create "$2".bin "[clobber_1xn] max_dims=$1;"
}

##################################################

build_mcgs

#make_db 14 db14
make_db 15 db15

#run_test db15.bin 15_replacer12
#run_test db15.bin 15_replacer123

#rm -f *.html
#compare 15_replacer123 15_replacer12 
