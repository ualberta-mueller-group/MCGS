#include "iobuffer.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include "throw_assert.h"

////////////////////////////////////////////////// i_ibuffer methods
uint16_t i_ibuffer::read_u16()
{
    return __read<uint16_t>();
}

uint32_t i_ibuffer::read_u32()
{
    return __read<uint32_t>();
}

uint64_t i_ibuffer::read_u64()
{
    return __read<uint64_t>();
}

int8_t i_ibuffer::read_i8()
{
    return __read<int8_t>();
}

int16_t i_ibuffer::read_i16()
{
    return __read<int16_t>();
}

int32_t i_ibuffer::read_i32()
{
    return __read<int32_t>();
}

int64_t i_ibuffer::read_i64()
{
    return __read<int64_t>();
}

////////////////////////////////////////////////// i_obuffer methods
void i_obuffer::write_u16(const uint16_t& val)
{
    __write<uint16_t>(val);
}

void i_obuffer::write_u32(const uint32_t& val)
{
    __write<uint32_t>(val);
}

void i_obuffer::write_u64(const uint64_t& val)
{
    __write<uint64_t>(val);
}

void i_obuffer::write_i8(const int8_t& val)
{
    __write<int8_t>(val);
}

void i_obuffer::write_i16(const int16_t& val)
{
    __write<int16_t>(val);
}

void i_obuffer::write_i32(const int32_t& val)
{
    __write<int32_t>(val);
}

void i_obuffer::write_i64(const int64_t& val)
{
    __write<int64_t>(val);
}

////////////////////////////////////////////////// file_ibuffer methods
file_ibuffer::file_ibuffer(const std::string& file_name) : _fs(file_name, OPEN_MODE)
{
    THROW_ASSERT(std::filesystem::exists(file_name),
                 "Input file \"" + file_name + "\" not found!");

    THROW_ASSERT(_fs.is_open(),
                 "Failed to open input file \"" + file_name + "\"!");
}

uint8_t file_ibuffer::read_u8()
{
    bool stream_ok = static_cast<bool>(_fs);

    // Don't use extraction operator (control codes mess up the data)
    uint8_t byte;
    _fs.read((char*) &byte, 1);

    stream_ok &= static_cast<bool>(_fs);
    THROW_ASSERT(stream_ok);

    return byte;
}

////////////////////////////////////////////////// file_obuffer methods
void file_obuffer::write_u8(const uint8_t& val)
{
    bool stream_ok = static_cast<bool>(_fs);

    // Don't use insertion operator (control codes mess up the data)
    _fs.write((const char*) &val, 1);

    stream_ok &= static_cast<bool>(_fs);
    THROW_ASSERT(stream_ok);
}


