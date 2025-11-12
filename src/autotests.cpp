#include "autotests.h"

#include <string>
#include <sstream>
#include <cstdio>
#include <ios>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <cassert>
#include <memory>
#include <algorithm>

#include "cgt_basics.h"
#include "solver_stats.h"
#include "file_parser.h"
#include "global_options.h"
#include "search_utils.h"
#include "throw_assert.h"
#include "utilities.h"
#include "sumgame.h"
#include "hashing.h"
#include "game.h"

// TODO put this in its own file? remove interface type? unit tests?
////////////////////////////////////////////////// directory iterators
class i_file_iterator
{
public:
    virtual ~i_file_iterator() {}

    virtual operator bool() const = 0;
    virtual const std::filesystem::directory_entry& gen_entry() const = 0;
    virtual void operator++() = 0;
};

class file_iterator_alphabetical: public i_file_iterator
{
public:
    file_iterator_alphabetical(const std::string& start_dir);
    file_iterator_alphabetical(const std::filesystem::directory_entry& start_dir);

    operator bool() const override;
    const std::filesystem::directory_entry& gen_entry() const override;
    void operator++() override;

private:
    void _expand(const std::filesystem::directory_entry& dir);
    void _increment(bool init);

    static bool _compare(const std::filesystem::directory_entry& entry1,
                         const std::filesystem::directory_entry& entry2);

    std::vector<std::filesystem::directory_entry> _file_entries;
    std::vector<std::filesystem::directory_entry> _dir_entries;
};


file_iterator_alphabetical::file_iterator_alphabetical(const std::string& start_dir)
{
    _expand(std::filesystem::directory_entry(start_dir));
    _increment(true);
}

file_iterator_alphabetical::file_iterator_alphabetical(const std::filesystem::directory_entry& start_dir)
{
    _expand(start_dir);
    _increment(true);
}

file_iterator_alphabetical::operator bool() const
{
    // No files --implies--> No directories
    assert(LOGICAL_IMPLIES(_file_entries.empty(), _dir_entries.empty()));
    return !_file_entries.empty();
}

const std::filesystem::directory_entry& file_iterator_alphabetical::gen_entry() const
{
    assert(*this);
    assert(!_file_entries.empty());

    const std::filesystem::directory_entry& entry = _file_entries.back();
    assert(!entry.is_directory());

    return entry;
}

void file_iterator_alphabetical::operator++()
{
    assert(*this);
    _increment(false);
}

void file_iterator_alphabetical::_expand(const std::filesystem::directory_entry& dir)
{
    assert(dir.is_directory());
    assert(_file_entries.empty());

    std::vector<std::filesystem::directory_entry> new_dirs;

    std::filesystem::directory_iterator it(dir);

    for (const std::filesystem::directory_entry& entry : it)
    {
        if (entry.is_directory())
            new_dirs.push_back(entry);
        else
            _file_entries.push_back(entry);
    }

    std::sort(_file_entries.begin(), _file_entries.end(), _compare);
    std::sort(new_dirs.begin(), new_dirs.end(), _compare);

    for (const std::filesystem::directory_entry& entry : new_dirs)
        _dir_entries.push_back(entry);
}

void file_iterator_alphabetical::_increment(bool init)
{
    assert(init || !_file_entries.empty());

    if (!init)
        _file_entries.pop_back();

    if (!_file_entries.empty())
        return;

    // Keep expanding directories
    while (!_dir_entries.empty() && _file_entries.empty())
    {
        std::filesystem::directory_entry dir = _dir_entries.back();
        _dir_entries.pop_back();

        _expand(dir);
    }
}

bool file_iterator_alphabetical::_compare(
    const std::filesystem::directory_entry& entry1,
    const std::filesystem::directory_entry& entry2)
{
    return entry1.path().string() > entry2.path().string();
}

//////////////////////////////////////////////////

/*
   TODO clean up this file after the papers. There should be a test_generator
   interface class, with an implementation for getting tests from the
   filesystem and from stdin

   For now I duplicate some code to save time...
*/

using namespace std;

using filesystem::recursive_directory_iterator;

// CSV separator
inline constexpr const char* SEP = ",";
inline constexpr const char NEWLINE = '\n';

//////////////////////////////////////// helper functions
namespace {

//inline void print_ready_signal()
//{
//    cout << "\nREADY FOR TEST CASE" << endl;
//}

// convert game list to string
string human_readable_game_string(const vector<game*>& games)
{
    stringstream stream;

    const size_t N = games.size();
    for (size_t i = 0; i < N; i++)
    {
        const game* g = games[i];
        g->print(stream);

        if (i + 1 < N)
        {
            stream << " ";
        }
    }

    string game_string = stream.str();

    return game_string;
}

// Print column contents to CSV file
void append_field(ostream& os, const string& field, bool include_separator)
{
    // Remove leading/trailing whitespace, replace double quotes with 2 single
    // quotes
    string sanitized_field;

    bool past_left_whitespace = false;

    const size_t N = field.size();
    for (size_t i = 0; i < N; i++)
    {
        const char& c = field[i];

        if (!past_left_whitespace && c == ' ')
        {
            continue;
        }
        else
        {
            past_left_whitespace = true;
        }

        if (c == '\"')
        {
            sanitized_field += "''";
        }
        else
        {
            sanitized_field.push_back(c);
        }
    }

    while (sanitized_field.size() > 0 && sanitized_field.back() == ' ')
    {
        sanitized_field.pop_back();
    }

    os << "\"" << sanitized_field << "\"";
    if (include_separator)
    {
        os << SEP;
    }
}
} // namespace

//////////////////////////////////////// exported functions
void run_autotests(const string& test_directory, const string& outfile_name,
                   unsigned long long test_timeout)
{
    THROW_ASSERT(test_directory.size() > 0);
    bool first_case = true;

    ofstream outfile(outfile_name); // CSV file

    if (!outfile.is_open())
    {
        throw ios_base::failure("Couldn't open file for writing: \"" +
                                outfile_name + "\"");
    }

    // print format as first row to file
    append_field(outfile, "File", true);
    append_field(outfile, "Case", true);
    append_field(outfile, "Games", true);
    append_field(outfile, "Player", true);
    append_field(outfile, "Expected Result", true);
    append_field(outfile, "Result", true);
    append_field(outfile, "Time (ms)", true);
    append_field(outfile, "Status", true);
    append_field(outfile, "Comments", true);
    append_field(outfile, "Node Count", true);
    append_field(outfile, "Unique Sum Count", true);
    append_field(outfile, "Input hash", false);
    outfile << NEWLINE;


    // iterate over autotests directory
    for (file_iterator_alphabetical iter(test_directory); iter; ++iter)
    {
        const std::filesystem::directory_entry& entry = iter.gen_entry();
        assert(!entry.is_directory());

        // Skip directories and non ".test" files
        if (!entry.is_regular_file())
            continue;

        const filesystem::path& file_path = entry.path();

        if (file_path.extension() != ".test")
            continue;

        const string file_name = file_path.string();

        cout << "New file: " << file_name << endl;

        // Path relative to test directory (this string is printed to CSV file)
        filesystem::path relative_file_path =
            filesystem::relative(file_path, filesystem::path(test_directory));

        // Open test file
        unique_ptr<file_parser> parser =
            unique_ptr<file_parser>(file_parser::from_file(file_name));

        game_case gc;
        int case_number = 0;

        while (parser->parse_chunk(gc))
        {
            cout << file_name << " " << case_number << endl;

            if (global::clear_tt() && !first_case)
                sumgame::reset_ttable();

            stats::reset_stats();
            search_result sr = gc.run(test_timeout);
            first_case = false;

            const solver_stats& st = stats::get_global_stats();

            // If --count-sums AND solve command was not {N}
            std::string sum_count_string =
                (global::count_sums() && is_black_white(sr.player))
                    ? to_string(st.sum_hashes.value().size())
                    : "N/A";

            append_field(outfile, relative_file_path.string(), true);
            append_field(outfile, to_string(case_number), true);
            append_field(outfile, human_readable_game_string(gc.games), true);
            append_field(outfile, sr.player_str(), true);
            append_field(outfile, gc.expected_value.str(), true);
            append_field(outfile, sr.value_str(), true);
            append_field(outfile, sr.duration_str(), true);
            append_field(outfile, sr.status_str(), true);
            append_field(outfile, gc.comments, true);
            append_field(outfile, to_string(st.node_count), true);
            append_field(outfile, sum_count_string, true);
            append_field(outfile, gc.hash.get_string(), false);
            outfile << NEWLINE;

            gc.cleanup_games();
            case_number++;
        }
    }

    if (random_table::did_resize_warning())
        random_table::print_resize_warning();

    outfile.close();
}

//void run_autotests_stdin(const string& outfile_name,
//                         unsigned long long test_timeout)
//{
//    assert(global::clear_tt());
//
//    bool first_case = true;
//
//    ofstream outfile(outfile_name); // CSV file
//
//    if (!outfile.is_open())
//    {
//        throw ios_base::failure("Couldn't open file for writing: \"" +
//                                outfile_name + "\"");
//    }
//
//    // print format as first row to file
//    append_field(outfile, "File", true);
//    append_field(outfile, "Case", true);
//    append_field(outfile, "Games", true);
//    append_field(outfile, "Player", true);
//    append_field(outfile, "Expected Result", true);
//    append_field(outfile, "Result", true);
//    append_field(outfile, "Time (ms)", true);
//    append_field(outfile, "Status", true);
//    append_field(outfile, "Comments", true);
//
//    append_field(outfile, "Node Count", true);
//    append_field(outfile, "TT Hits", true);
//    append_field(outfile, "TT Misses", true);
//    append_field(outfile, "DB Hits", true);
//    append_field(outfile, "DB Misses", true);
//    append_field(outfile, "Max Depth", true);
//    append_field(outfile, "# Subgames", true);
//
//    append_field(outfile, "Input hash", false);
//    outfile << NEWLINE;
//
//    print_ready_signal(); // READY
//
//    unique_ptr<file_parser> parser(file_parser::from_stdin());
//
//    game_case gc;
//    uint64_t case_number = 0;
//
//    while (parser->parse_chunk(gc))
//    {
//        if (case_number % 20 == 0)
//            outfile.flush();
//
//        if (global::clear_tt() && !first_case)
//            sumgame::reset_ttable();
//
//        stats::reset_stats();
//        search_result sr = gc.run(test_timeout);
//        first_case = false;
//
//        const solver_stats& st = stats::get_global_stats();
//
//        append_field(outfile, "stdin", true);
//        append_field(outfile, to_string(case_number), true);
//        append_field(outfile, human_readable_game_string(gc.games), true);
//        append_field(outfile, sr.player_str(), true);
//        append_field(outfile, gc.expected_value.str(), true);
//        append_field(outfile, sr.value_str(), true);
//        append_field(outfile, sr.duration_str(), true);
//        append_field(outfile, sr.status_str(), true);
//        append_field(outfile, gc.comments, true);
//
//        append_field(outfile, to_string(st.node_count), true);   //
//        append_field(outfile, to_string(st.tt_hits), true);      //
//        append_field(outfile, to_string(st.tt_misses), true);    //
//        append_field(outfile, to_string(st.db_hits), true);      //
//        append_field(outfile, to_string(st.db_misses), true);    //
//        append_field(outfile, to_string(st.search_depth), true); //
//        append_field(outfile, to_string(st.n_subgames), true);   //
//
//        append_field(outfile, gc.hash.get_string(), false);
//        outfile << NEWLINE;
//
//        gc.cleanup_games();
//        case_number++;
//
//        print_ready_signal(); // READY
//    }
//
//    if (random_table::did_resize_warning())
//        cerr << "TABLE RESIZE" << endl;
//
//    outfile.close();
//}
