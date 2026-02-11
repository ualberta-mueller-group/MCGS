set -e
rm -rf *.csv *.html


make_db() {
    ./MCGS --db-file-create database.bin "[clobber_1xn] max_dims=12; [nogo_1xn] max_dims=12; [clobber] max_dims=3,3; [nogo] max_dims=3,3; [impartial clobber_1xn] max_dims=12; [impartial nogo_1xn] max_dims=12; [impartial clobber] max_dims=3,3; [impartial nogo] max_dims=3,3;"
}

run_tests() {
    ./MCGS --run-tests --test-timeout 0 --test-dir input/main_tests/clobber_1xn_autotests --out-file clobber-"$1".csv
    ./MCGS --run-tests --test-timeout 0 --test-dir input/main_tests/nogo_1xn_autotests --out-file nogo-"$1".csv
}

make_table() {
    python3 create-table.py "$1"-"$2".csv --compare-to "$1"-"$3".csv -o "$1".html
}

make_table_single() {
    python3 create-table.py "$1".csv -o "$1".html
}


################################################## Bounds win
make clean
make -j 11 USE_THERM=1 USE_BOUNDS=1 USE_BOUNDS_WIN=1

make_db
run_tests bounds


################################################## Default
make clean
make -j 11

make_db
run_tests default

################################################## HTML
make_table clobber bounds default
make_table nogo bounds default
