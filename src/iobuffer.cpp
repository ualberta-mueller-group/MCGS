#include "iobuffer.h"
#include <cstdint>

////////////////////////////////////////////////// ibuffer
uint8_t ibuffer::read_u8()
{
    return __fmt_read<uint8_t>(_fs);
}

uint16_t ibuffer::read_u16()
{
    return __fmt_read<uint16_t>(_fs);
}

uint32_t ibuffer::read_u32()
{
    return __fmt_read<uint32_t>(_fs);
}

uint64_t ibuffer::read_u64()
{
    return __fmt_read<uint64_t>(_fs);
}

int8_t ibuffer::read_i8()
{
    return __fmt_read<int8_t>(_fs);
}

int16_t ibuffer::read_i16()
{
    return __fmt_read<int16_t>(_fs);
}

int32_t ibuffer::read_i32()
{
    return __fmt_read<int32_t>(_fs);
}

int64_t ibuffer::read_i64()
{
    return __fmt_read<int64_t>(_fs);
}

////////////////////////////////////////////////// obuffer
void obuffer::write_u8(const uint8_t& val)
{
    __fmt_write<uint8_t>(_fs, val);
}

void obuffer::write_u16(const uint16_t& val)
{
    __fmt_write<uint16_t>(_fs, val);
}

void obuffer::write_u32(const uint32_t& val)
{
    __fmt_write<uint32_t>(_fs, val);
}

void obuffer::write_u64(const uint64_t& val)
{
    __fmt_write<uint64_t>(_fs, val);
}

void obuffer::write_i8(const int8_t& val)
{
    __fmt_write<int8_t>(_fs, val);
}

void obuffer::write_i16(const int16_t& val)
{
    __fmt_write<int16_t>(_fs, val);
}

void obuffer::write_i32(const int32_t& val)
{
    __fmt_write<int32_t>(_fs, val);
}

void obuffer::write_i64(const int64_t& val)
{
    __fmt_write<int64_t>(_fs, val);
}
