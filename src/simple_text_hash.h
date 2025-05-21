#pragma once
#include <string>
#include <cstdint>
#include <cstddef>

/*
    Used by file_parser to hash input tokens, creating a hash for a test case.
        The hash is used by the HTML table generator to verify that the test
        case hasn't been changed
*/

class simple_text_hash
{
public:
    simple_text_hash();

    simple_text_hash(simple_text_hash&& other) noexcept;
    simple_text_hash& operator=(simple_text_hash&& other) noexcept;

    // Re-initialize everything. Can reuse this object after calling this
    void clear();

    void update(const std::string& data);

    // finalize the hash. After calling get_string(), update() shouldn't be
    // called again
    const std::string& get_string();

    static constexpr int BUFFER_SIZE = 8;

    uint8_t buffer[BUFFER_SIZE];
    int pos; // current index into buffer
    size_t bytes_seen;
    std::string string_representation;

    static_assert(BUFFER_SIZE > 0, "Buffer size must be > 0");
    static_assert(BUFFER_SIZE >= sizeof(bytes_seen),
                  "Buffer must be able to hold byte count");

private:
    void _move_impl(simple_text_hash&& other) noexcept;
};
