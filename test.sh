# Command to run
CMD="./MCGS --db-file-create database.bin \"[nogo_1xn] max_dims=15;\""

clear
make -j 11
clear

# Run for a few seconds, to warm up caches etc.
sudo nice -n -19 sudo -u "$(whoami)" sh -c "$CMD" &
sleep 4
kill -9 $!

# Time while running to completion
time sudo nice -n -19 sudo -u "$(whoami)" sh -c "$CMD"
