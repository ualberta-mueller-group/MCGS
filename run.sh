set -e
sudo renice -n -19 -p $$

################################################## Functions
build_mcgs() {
    cmake -B build
    cmake --build build -j11
}

clear_screen() {
    echo -ne "\e[3J" && clear
}

################################################## Main logic

build_mcgs
clear_screen
./MCGS ""
