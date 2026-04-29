set -e
sudo renice -19 -p "$$"

# Command to run
#CMD1="./MCGS --db-file-create database1.bin \"[amazons] max_dims=3,3;\""
#CMD2="./MCGS --db-file-create database2.bin \"[nogo_1xn] max_dims=15;\""
#CMD3="./MCGS --db-file-create database3.bin \"[domineering] max_dims=4,5;\""

CMD1="./MCGS --db-file-create database.bin \"[amazons] max_dims=3,3;\""
CMD2="./MCGS --db-file-create database.bin \"[nogo_1xn] max_dims=15;\""
CMD3="./MCGS --db-file-create database.bin \"[domineering] max_dims=4,5;\""

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

run "$CMD1"
run "$CMD2"
run "$CMD3"
