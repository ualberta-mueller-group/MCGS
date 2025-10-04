# Delete diagram images, experiment output from previous run, and subgame database
rm -rf diagrams experiment_results database.bin

# Make debug build of MCGS
make clean
make -j 11

# Generate the database
./MCGS "[]"
# OR, comment out above line, and uncomment MCGS invocation below, to
# generate the database AND (compiler/machine-specific) experiment inputs
#./MCGS --gen-experiments --experiment-seed 4612

# Make production build of MCGS and run experiments
make clean
make DEBUG=0 -j
python3 runtests.py

# Make diagram images from experiment output
python3 diagram.py experiment_results --save diagrams
