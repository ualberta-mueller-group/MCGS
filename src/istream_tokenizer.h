/*
   Helper class used by file_parser. Breaks input string into tokens.
   Each token either contains no whitespace, or contains only whitespace.

   consume() and rewind() functions allow for speculative logic in parsing:
*/
#pragma once

#include <iostream>
#include <vector>
#include <cstddef>
#include <string>

////////////////////////////////////////////////// class istream_tokenizer
class istream_tokenizer
{
public:
    istream_tokenizer(std::istream* stream, bool delete_stream);
    ~istream_tokenizer();

    bool get_token(std::string& token);

    bool is_whitespace() const;
    size_t line_start() const;
    size_t line_end() const;
    size_t column_start() const;
    size_t column_end() const;

    void consume();
    void rewind();

    struct token_t
    {
        token_t(const std::string& token_string, bool is_whitespace,
                size_t line_start, size_t line_end, size_t column_start,
                size_t column_end);

        std::string token_string;
        bool is_whitespace;
        size_t line_start;
        size_t line_end;
        size_t column_start;
        size_t column_end;
    };

private:
    void _cleanup();
    bool _get_token_from_stream(std::string& token);

    std::istream* _stream_ptr;
    bool _delete_stream;

    bool _is_whitespace;
    size_t _line_start;
    size_t _line_end;
    size_t _column_start;
    size_t _column_end;

    std::vector<token_t> _token_buffer;
    size_t _token_idx;
};

////////////////////////////////////////////////// istream_tokenizer::token_t methods
inline istream_tokenizer::token_t::token_t(const std::string& token_string,
                                           bool is_whitespace, size_t line_start,
                                           size_t line_end, size_t column_start,
                                           size_t column_end)
    : token_string(token_string),
      is_whitespace(is_whitespace),
      line_start(line_start),
      line_end(line_end),
      column_start(column_start),
      column_end(column_end)
{
}


