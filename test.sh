set -e
sudo renice -19 -p "$$"

# Command to run
CMD1="./build/MCGS --db-file-create 1_1.bin \"[amazons] max_dims=3,3;\""
CMD2="./build/MCGS --db-file-create 2_1.bin \"[nogo_1xn] max_dims=15;\""
CMD3="./build/MCGS --db-file-create 3_1.bin \"[domineering] max_dims=4,5;\""
CMD4="./build/MCGS --db-file-create 4_1.bin \"[clobber_1xn] max_dims=15;\""
CMD5="./build/MCGS --db-file-create 5_1.bin \"[clobber] max_dims=3,4;\""

clear_screen() {
    echo -ne "\e[3J" && clear
}

build_mcgs() {
    cmake -B build
    cmake --build build -- -j 11
}

run_perf() {
    sh -c "$1" &
    sleep 3
    kill -9 $!

    time sh -c "$1"
}

run() {
    time sh -c "$1"
}

################################################################################
clear_screen
build_mcgs
clear_screen

#run_perf "$CMD1"
#run_perf "$CMD2"
#run_perf "$CMD3"
#run_perf "$CMD4"
run_perf "$CMD5"

#run "$CMD1"
#run "$CMD2"
#run "$CMD3"
#run "$CMD4"
#run "$CMD5"
