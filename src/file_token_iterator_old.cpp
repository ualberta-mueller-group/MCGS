#include "file_token_iterator_old.h"

#if 0
//////////////////////////////////////////////////////////// file_token_iterator
file_token_iterator::file_token_iterator(istream* stream, bool delete_stream)
    : _main_stream_ptr(stream),
      _delete_stream(delete_stream),
      _line_number(-1),
      _token_buffer(),
      _token_idx(0)
{
    _token_buffer.reserve(8);
}

file_token_iterator::~file_token_iterator()
{
    _cleanup();
}

int file_token_iterator::line_number() const
{
    // Don't call without getting a valid token first
    assert(_line_number >= 0);

    return _line_number + 1;
}

bool file_token_iterator::get_token(string& token)
{
    token.clear();

    // Check if token buffer has data
    if (_token_idx < _token_buffer.size())
    {
        const token_info& ti = _token_buffer[_token_idx];
        _token_idx++;

        _line_number = ti.line_number;
        token = ti.token_string;

        return true;
    }

    // Check if stream has more tokens
    if (_get_token_from_stream(token))
    {
        _token_buffer.emplace_back(token, _line_number);
        _token_idx++;

        assert(_token_idx == _token_buffer.size());

        return true;
    }

    return false;
}

bool file_token_iterator::_get_token_from_stream(string& token)
{
    assert(_main_stream_ptr != nullptr);
    token.clear();

    istream& main_stream = *_main_stream_ptr;

    // Check if current line has more tokens
    if (_line_stream && _line_stream >> token)
    {
        return true;
    }

    if (_line_stream.fail() && !_line_stream.eof())
    {
        throw ios_base::failure("file_token_iterator operator++ line IO error");
    }

    // Scroll through the file's lines until we get a token
    string next_line;
    while (main_stream && getline(main_stream, next_line) &&
           !main_stream.fail())
    {
        _line_number++;
        _line_stream = stringstream(next_line);

        if (_line_stream && _line_stream >> token && !_line_stream.fail())
        {
            return true;
        }

        if (_line_stream.fail() && !_line_stream.eof())
        {
            throw ios_base::failure(
                "file_token_iterator operator++ line IO error");
        }
    }

    if (main_stream.fail() && !main_stream.eof())
    {
        throw ios_base::failure("file_token_iterator operator++ file IO error");
    }

    // no remaining tokens
    return false;
}

void file_token_iterator::consume()
{
    vector<token_info> new_buffer;
    new_buffer.reserve(_token_buffer.size() - _token_idx);

    for (size_t i = _token_idx; i < _token_buffer.size(); i++)
    {
        new_buffer.push_back(_token_buffer[i]);
    }

    _token_buffer.clear();
    _token_buffer = std::move(new_buffer);
    _token_idx = 0;
}

void file_token_iterator::rewind()
{
    _token_idx = 0;
}

void file_token_iterator::_cleanup()
{
    if (_delete_stream && _main_stream_ptr != nullptr)
    {
        // close if it's a file...
        ifstream* file = dynamic_cast<ifstream*>(_main_stream_ptr);

        if (file != nullptr && file->is_open())
        {
            file->close();
        }

        delete _main_stream_ptr;
    }

    _main_stream_ptr = nullptr;
}
#endif
