set -e
sudo renice -19 -p "$$"

# Command to run
CMD1="./MCGS --db-file-create database.bin \"[amazons] max_dims=3,3;\""
CMD2="./MCGS --db-file-create database.bin \"[nogo_1xn] max_dims=15;\""
CMD3="./MCGS --db-file-create database.bin \"[domineering] max_dims=4,5;\""
CMD4="./MCGS --db-file-create database.bin \"[clobber_1xn] max_dims=15;\""
CMD5="./MCGS --db-file-create database.bin \"[clobber] max_dims=3,4;\""

clear_screen() {
    echo -ne "\e[3J" && clear
}

build_mcgs() {
    make -j 11
}

run_perf() {
    sh -c "$1" &
    sleep 5
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
#run_perf "$CMD5"
