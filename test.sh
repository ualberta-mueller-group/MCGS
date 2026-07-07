# All DB times for `--size-score board_size` and without `--no-pitm`
# clobber_1xn 14: 49s
# nogo_1xn 14: 27s
# elephants 14: 17s
# amazons 3x3: 6s
# clobber 3x3: 1s
# domineering 4x5: 48s

set -e
sudo renice -n -19 -p $$

delete_files() {
    rm -f *.bin *.csv *.html
}

build_mcgs() {
    cmake -B build
    cmake --build build -j11
}

# DB name
# Flags
make_db() {
    local DB_NAME="$1"
    shift 1

    DBSTRING=""
    #DBSTRING="$DBSTRING [clobber_1xn] max_dims=14;"
    #DBSTRING="$DBSTRING [nogo_1xn] max_dims=14;"
    #DBSTRING="$DBSTRING [elephants] max_dims=14;"
    #DBSTRING="$DBSTRING [amazons] max_dims=3,3;"
    #DBSTRING="$DBSTRING [clobber] max_dims=3,3;"
    DBSTRING="$DBSTRING [domineering] max_dims=4,5;"

    ./MCGS --db-file-create "$DB_NAME" "$DBSTRING" $@
}

# Test dir
# DB name
# CSV name
# Flags
run_test() {
    local TEST_DIR="$1"
    local DB_NAME="$2"
    local CSV_NAME="$3"
    shift 3

    ./MCGS --clear-tt --tt-sumgame-idx-bits 29 --db-file-load "$DB_NAME" --run-tests --test-timeout 15000 --test-dir "$TEST_DIR" --out-file "$CSV_NAME" $@
}

# CSV basename
make_html1() {
    python3 create-table.py "$1".csv -o "$1".html
}

# CSV 1 basename
# CSV 2 basename
make_html2() {
    python3 create-table.py "$1".csv --compare-to "$2".csv -o "$1"_VS_"$2".html
}

################################################################################
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
################################################################################
delete_files
build_mcgs

#make_db clobber_1xn_ss_board_size.bin --size-score board_size
#run_test perf_tests/clobber_1xn clobber_1xn_ss_board_size.bin clobber_1xn_seg3.csv --size-score board_size
#run_test perf_tests/clobber_1xn clobber_1xn_ss_board_size.bin clobber_1xn_seg1.csv --size-score board_size --single-seg
#
#make_html2 clobber_1xn_seg3 clobber_1xn_seg1 

#make_db nogo_1xn_ss_max_local_options.bin --size-score max_local_options
#run_test perf_tests/nogo_1xn nogo_1xn_ss_max_local_options.bin nogo_1xn_seg3.csv --size-score max_local_options
#run_test perf_tests/nogo_1xn nogo_1xn_ss_max_local_options.bin nogo_1xn_seg1.csv --size-score max_local_options --single-seg
#make_html2 nogo_1xn_seg3 nogo_1xn_seg1

make_db domineering_ss_max_local_options.bin --size-score max_local_options
run_test perf_tests/domineering domineering_ss_max_local_options.bin domineering_seg3.csv --size-score max_local_options
run_test perf_tests/domineering domineering_ss_max_local_options.bin domineering_seg1.csv --size-score max_local_options --single-seg
make_html2 domineering_seg3 domineering_seg1

