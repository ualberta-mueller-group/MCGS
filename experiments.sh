rm -rf diagrams && make -j 11 && ./MCGS --gen-experiments --experiment-seed 0 && python3 runtests.py && python3 diagram.py experiment_results --save diagrams
