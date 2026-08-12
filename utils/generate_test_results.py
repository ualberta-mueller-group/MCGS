"""
    MCGS Utility - generate test results
    
    Convert raw results for periodic impartial games 
    into a .test file for MCGS.
    Reads a file with the expected results = nimbers, one per line
    Other settings can be configured directly in this file
    
    Usage: python3 generate_test_results.py > mytest.test
"""
import re

version_number = "1.7"
game = "impartial cannibal_clobber"
results_file = "results.txt"
pattern = "X"

def write_entry(nimber, index):
    print(f'\n/* {index} */')
    print(pattern * index, end = "")
    print(f' {{N {nimber}}}')

print(f'{{version {version_number}}}')
print(f'[{game}]')

with open(results_file, 'r') as file:
    content = file.read()
    numbers = re.findall(r'\d+', content)
    index = 1
    for nimber in numbers:
        write_entry(nimber, index)
        index += 1
