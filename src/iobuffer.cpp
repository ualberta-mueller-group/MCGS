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

void i_ibuffer::_on_bad_read()
{
    std::cerr << "Bad read from ibuffer!" << std::endl;
    std::abort();
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

void i_obuffer::_on_bad_write()
{
    std::cerr << "Bad write to obuffer!" << std::endl;
    std::abort();
}

////////////////////////////////////////////////// file_ibuffer methods
file_ibuffer::file_ibuffer(const std::string& file_name) : _fs(file_name, OPEN_MODE)
{
    THROW_ASSERT(std::filesystem::exists(file_name),
                 "Input file \"" + file_name + "\" not found!");

    THROW_ASSERT(_fs.is_open(),
                 "Failed to open input file \"" + file_name + "\"!");
}

