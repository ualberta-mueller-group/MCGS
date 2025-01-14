#pragma once
#include <string>
#include <cstdint>

class simple_text_hash
{
public:
    static constexpr size_t buffer_size = 8;

    uint8_t buffer[buffer_size];
    int pos;
    size_t bytes_seen;
    std::string string_representation;

    static_assert(buffer_size > 0, "Buffer size must be > 0");
    static_assert(buffer_size >= sizeof(bytes_seen), "Buffer must be able to hold byte count");


    simple_text_hash();

    simple_text_hash(simple_text_hash&& other) noexcept;
    simple_text_hash& operator=(simple_text_hash&& other) noexcept;

    void clear();
    void update(const std::string& data);
    const std::string& get_string();


private:
    void _move_impl(simple_text_hash&& other) noexcept;

};

