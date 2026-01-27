#include "istream_tokenizer.h"

#include "utilities.h"

#include <cctype>
#include <iostream>
#include <vector>
#include <cstddef>
#include <optional>
#include <fstream>
#include <string>
#include <cassert>
#include <utility>

#include "throw_assert.h"

using namespace std;


////////////////////////////////////////////////// istream_tokenizer methods
istream_tokenizer::istream_tokenizer(istream* stream_ptr, bool delete_stream)
    : _stream_ptr(stream_ptr),
      _delete_stream(delete_stream),
      _line_start(0),
      _line_end(0),
      _token_idx(0)
{
}

istream_tokenizer::~istream_tokenizer()
{
    _cleanup();
}

bool istream_tokenizer::get_token(string& token)
{
    token.clear();

    // check if token buffer has more data
    if (_token_idx < _token_buffer.size())
    {
        const token_t& t = _token_buffer[_token_idx];
        _token_idx++;

        _line_start = t.line_start;
        _line_end = t.line_end;
        token = t.token_string;

        return true;
    }

    // get token from stream
    if (_get_token_from_stream(token))
    {
        _token_buffer.emplace_back(token, _is_whitespace, _line_start, _line_end);
        _token_idx++;

        assert(_token_idx == _token_buffer.size());
        return true;
    }

    return false;
}

bool istream_tokenizer::is_whitespace() const
{
    assert(_line_start > 0);
    return _is_whitespace;
}

int istream_tokenizer::line_start() const
{
    assert(_line_start > 0);
    return _line_start;
}

int istream_tokenizer::line_end() const
{
    assert(_line_end > 0);
    return _line_end;
}

void istream_tokenizer::consume()
{
    vector<token_t> new_buffer;
    new_buffer.reserve(_token_buffer.size() - _token_idx);

    for (size_t i = _token_idx; i < _token_buffer.size(); i++)
        new_buffer.emplace_back(_token_buffer[i]);

    _token_buffer.clear();
    _token_buffer = std::move(new_buffer);
    _token_idx = 0;
}

void istream_tokenizer::rewind()
{
    _token_idx = 0;
}

void istream_tokenizer::_cleanup()
{
    if (_delete_stream && _stream_ptr != nullptr)
    {
        ifstream* file = dynamic_cast<ifstream*>(_stream_ptr);

        if (file != nullptr && file->is_open())
            file->close();

        delete _stream_ptr;
    }

    _stream_ptr = nullptr;
}

bool istream_tokenizer::_get_token_from_stream(string& token)
{
    assert(_stream_ptr != nullptr);
    token.clear();

    int line_start_new = _line_end;
    if (line_start_new == 0)
        line_start_new = 1;

    int line_end_new = line_start_new;

    // read from stream until isspace flips
    int advance_lines = 0;

    optional<bool> token_is_space;

    while (true)
    {
        const char c = _stream_ptr->peek();

        THROW_ASSERT(!(_stream_ptr->bad() && !_stream_ptr->eof()));
        if (_stream_ptr->eof())
            break;

        const bool char_is_space = isspace(c);

        if (token_is_space.has_value())
        {
            if (token_is_space.value() != char_is_space)
                break;
        }
        else
        {
            token_is_space = char_is_space;
        }

        const char c2 = _stream_ptr->get();
        THROW_ASSERT(!(_stream_ptr->bad() && !_stream_ptr->eof()));
        assert(c == c2);
        
        token.push_back(c);
        if (is_newline(c))
            advance_lines++;
    }

    line_end_new = line_start_new + advance_lines;

    if (!token.empty())
    {
        assert(token_is_space.has_value());
        _is_whitespace = token_is_space.value();

        _line_start = line_start_new;
        _line_end = line_end_new;

        return true;
    }

    return false;
}


//////////////////////////////////////////////////
void test_istream_tokenizer()
{
    ifstream* fs = new ifstream("test.test");
    istream_tokenizer tk(fs, true);

    string token;
    while (tk.get_token(token))
    {
        cout << "|" << tk.line_start() << "-";
        cout << tk.line_end() << " ";
        cout << tk.is_whitespace() << "|";
        cout << "\"" << token << "\"" << endl;
    }
}

