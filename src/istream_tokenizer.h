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
    int line_start() const;
    int line_end() const;

    void consume();
    void rewind();

    struct token_t
    {
        token_t(const std::string& token_string, bool is_whitespace, int line_start, int line_end);

        std::string token_string;
        bool is_whitespace;
        int line_start;
        int line_end;
    };

private:
    void _cleanup();
    bool _get_token_from_stream(std::string& token);

    std::istream* _stream_ptr;
    bool _delete_stream;

    bool _is_whitespace;
    int _line_start;
    int _line_end;

    std::vector<token_t> _token_buffer;
    size_t _token_idx;
};

////////////////////////////////////////////////// istream_tokenizer::token_t methods
inline istream_tokenizer::token_t::token_t(const std::string& token_string,
                                   bool is_whitespace, int line_start,
                                   int line_end)
    : token_string(token_string),
      is_whitespace(is_whitespace),
      line_start(line_start),
      line_end(line_end)
{
}

//////////////////////////////////////////////////
void test_istream_tokenizer();
