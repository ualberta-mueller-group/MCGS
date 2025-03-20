#include "simple_text_hash.h"
#include <cstring>
#include <cassert>
#include <vector>

using namespace std;

simple_text_hash::simple_text_hash()
{
    clear();
}

simple_text_hash::simple_text_hash(simple_text_hash&& other) noexcept
{
    _move_impl(std::forward<simple_text_hash>(other));
}

simple_text_hash& simple_text_hash::operator=(simple_text_hash&& other) noexcept
{
    _move_impl(std::forward<simple_text_hash>(other));
    return *this;
}

void simple_text_hash::clear()
{
    memset(buffer, 0, BUFFER_SIZE);
    pos = 0;
    bytes_seen = 0;
    string_representation.clear();
}

void simple_text_hash::update(const string& data)
{
    // get_string() shouldn't have been called yet
    assert(string_representation.size() == 0);

    const size_t N = data.size();
    for (size_t i = 0; i < N; i++)
    {
        const char& c = data[i];
        bytes_seen++;

        assert(pos >= 0 && pos < BUFFER_SIZE);
        buffer[pos] ^= c;

        pos = (pos + 1) % BUFFER_SIZE;
    }
}

const string& simple_text_hash::get_string()
{
    if (string_representation.size() > 0)
    {
        return string_representation;
    }

    // Deal with common collision by adding bytes_seen into the hash buffer
    // This helps in the case where there's a repeating pattern that "wraps
    // around" the buffer
    assert(BUFFER_SIZE >= sizeof(bytes_seen));
    for (size_t i = 0; i < sizeof(bytes_seen); i++)
    {
        buffer[i] ^= ((uint8_t*) (&bytes_seen))[i];
    }

    // convert each byte to hex characters
    for (size_t i = 0; i < BUFFER_SIZE; i++)
    {
        const uint8_t& c = buffer[i];

        size_t conversion_space = 3;
        // char converted_byte[conversion_space];
        vector<char> converted_byte(conversion_space);

        // snprintf won't overflow the buffer
        size_t used =
            snprintf(converted_byte.data(), conversion_space, "%02X", (int) c);

        assert(used + 1 == conversion_space);

        string_representation += converted_byte.data();
    }

    return string_representation;
}

void simple_text_hash::_move_impl(simple_text_hash&& other) noexcept
{
    memcpy(buffer, other.buffer, BUFFER_SIZE);
    pos = std::move(other.pos);
    bytes_seen = std::move(other.bytes_seen);
    string_representation = std::move(other.string_representation);

    other.clear();
}
