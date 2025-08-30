rm -rf diagrams experiment_results database.bin

make clean
make -j 11
./MCGS --gen-experiments --experiment-seed 4612

make clean
make DEBUG=0 -j 11
python3 runtests.py

python3 diagram.py experiment_results --save diagrams
