#include "test_framework_stream.h"

#include <cstring>
#include <fstream>
#include <ios>
#include <cassert>
#include <streambuf>
#include <iostream>
#include <cstdio>
#include <vector>

/*
   Prototyping for new test framework output format ".mcgsout" (better name pending)

    Idea:
    - The .mcgsout file is a simple text file containing other text files
    - It starts with an index indicating where each sub-file is located in
        the file, how long the sub-file is, and its hash
    - A custom stream is used to write to the file. It writes to sub-files, and
        handles the index.

    - Messy solution: make a custom std::streambuf and custom std::ostream.
        The streambuf forwards data to a file and does most of the record keeping.
        The ostream initializes its base type with an instance of the streambuf
        type (via a static constructor function which first constructs the
        streambuf), and provides an interface to close a sub-file etc

        This is messy, as space for the index has to be reserved before any
        sub-file writes happen, and the index has to be written upon finishing,
        by seeking to the correct locations, AND each sub-file has to be closed
        before the next one can be written

    - Better solution: derive from std::fstream, open for both reading and
        writing, write each sub-file into one temp file, do some record keeping,
        then write the temp file into the proper file
        
        This isn't perfect as you still have to close a sub-file before writing
        to the next one

    - Best (?) solution: each sub-file is a separate std::fstream
        in read + write mode (and ".temp" extension), write each sub-file
        without any superfluous restrictions, then stitch them together in the end?

        This is very nice in contrast to other options, but there should be
        some way to manage the separate streams

        Maybe have some "mcgs_out" helper class which creates each of the
        sub-file streams. Give it a function to flush all the streams and write
        them into the main file.

*/

class mcgs_out
{
public:
    mcgs_out(const std::string& filename):
        _filename(filename)
    {
    }

    ~mcgs_out(); // call finalize

    std::fstream& get_subfile_stream(const std::string& filename);
    void finalize();

private:
    typedef std::pair<std::fstream, std::string> subfile_pair;

    const std::string _filename;
    std::vector<subfile_pair> _streams;
};


void test_streams()
{
}
