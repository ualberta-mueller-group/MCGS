#pragma once

#if 0
//////////////////////////////////////// token_iterator

/*
   token_iterators generate string tokens from some input stream, and
    remember line numbers. For use by file_parser
*/
class token_iterator
{
public:
    virtual ~token_iterator() {}

    // get next token, writing it into "token". Returns true iff result is valid
    virtual bool get_token(std::string& token) = 0;

    // line number of previous token returned by get_token()
    virtual int line_number() const = 0;

    // caller consumes all previously returned tokens
    virtual void consume() = 0;

    // rewind to first previously unconsumed token
    virtual void rewind() = 0;
};

class file_token_iterator : public token_iterator
{
public:
    /*
        if delete_stream is true, the stream is owned by the
       file_token_iterator; i.e. stream might be std::cin and delete_stream will
       be false, or stream might be some std::ifstream and delete_stream will be
       true
    */
    file_token_iterator(std::istream* stream, bool delete_stream);
    ~file_token_iterator();

    bool get_token(std::string& token) override;
    int line_number() const override;

    void consume() override;
    void rewind() override;

private:
    struct token_info
    {
        token_info(const std::string& token_string, int line_number)
            : token_string(token_string), line_number(line_number)
        {
        }

        std::string token_string;
        int line_number;
    };

    void _cleanup();
    bool _get_token_from_stream(std::string& token);

    std::istream* _main_stream_ptr;
    bool _delete_stream; // do we own this stream?

    std::stringstream _line_stream;

    int _line_number;

    std::vector<token_info> _token_buffer;
    size_t _token_idx;
};

#endif

