#include "iobuffer.h"

#include <iostream>
#include <cstdlib>

namespace {
} // namespace

//void file_ibuffer::read_bytes_raw(void* dst, size_t n_bytes)
//{
//    assert(_file != nullptr);
//
//    // Initial read
//    const size_t bytes_in_buffer = _remaining_unread_bytes();
//    const size_t read1_len = std::min(n_bytes, bytes_in_buffer);
//
//    if (read1_len != 0)
//    {
//        std::memcpy(
//            dst, reinterpret_cast<const char*>(_physical_buffer + _buffer_idx),
//            read1_len);
//
//        _buffer_idx += read1_len;
//    }
//
//    if (read1_len == n_bytes)
//        return;
//
//    assert(_remaining_unread_bytes() == 0);
//
//    const size_t read2_len = n_bytes - read1_len;
//
//    if (n_bytes >= _BUFFER_SIZE)
//    {
//        // Do unbuffered read
//        fread((char*) dst + read1_len, 1, read2_len, _file);
//        _remaining_file_bytes -= read2_len;
//    }
//    else
//    {
//        // Do buffered read
//        _preload_bytes(read2_len);
//        assert(_remaining_unread_bytes() >= read2_len && _buffer_idx == 0);
//
//        std::memcpy((char*) dst + read1_len, _physical_buffer + _buffer_idx,
//                    read2_len);
//
//        _buffer_idx += read2_len;
//    }
//}

//void memory_ibuffer::read_bytes_raw(void* dst, size_t n_bytes)
//{
//    assert(_remaining_unread_bytes() >= n_bytes);
//    std::memcpy((char*) dst, _buffer + _buffer_idx, n_bytes);
//    _buffer_idx += n_bytes;
//}

//void file_obuffer::write_bytes_raw(const void* src, size_t n_bytes)
//{
//    if (n_bytes >= _buffer_size)
//    {
//        // Do unbuffered write
//        if (_buffer_fill > 0)
//            flush();
//        assert(_buffer_fill == 0);
//
//        //_fs.write((const char *) src, n_bytes);
//        fwrite((const char*) src, 1, n_bytes, _file);
//    }
//    else
//    {
//        // Do buffered write
//        const size_t remaining_capacity = _remaining_buffer_capacity();
//        const size_t write1_len = std::min(remaining_capacity, n_bytes);
//        std::memcpy(_buffer + _buffer_fill, src, write1_len);
//
//        if (write1_len == n_bytes)
//        {
//            _buffer_fill += n_bytes;
//            return;
//        }
//
//        flush();
//        assert(_buffer_fill == 0);
//
//        const size_t write2_len = _buffer_size - write1_len;
//        assert(write1_len + write2_len == n_bytes);
//
//        std::memcpy(_buffer, (const char*) src + write1_len, write2_len);
//        _buffer_fill = write2_len;
//    }
//}


//void memory_obuffer::write_bytes_raw(const void* src, size_t n_bytes)
//{
//    size_t remaining_capacity = _remaining_buffer_capacity();
//    bool need_resize = false;
//
//    while (remaining_capacity < n_bytes)
//    {
//        need_resize = true;
//        const size_t new_size = _buffer_size * 2;
//        assert(new_size > _buffer_size);
//        _buffer_size = new_size;
//        remaining_capacity = _remaining_buffer_capacity();
//    }
//
//    if (need_resize)
//    {
//        _data_vec.resize(_buffer_size);
//        _buffer = _data_vec.data();
//    }
//
//    std::memcpy(_buffer + _buffer_fill, src, n_bytes);
//    _buffer_fill += n_bytes;
//}

////////////////////////////////////////////////// i_ibuffer methods
void i_ibuffer::_on_buffer_not_freed()
{
    std::cerr << "ibuffer: derived class didn't clean up!" << std::endl;
    std::abort();
}

void i_ibuffer::_on_bad_read()
{
    std::cerr << "Bad read from ibuffer!" << std::endl;
    std::abort();
}

////////////////////////////////////////////////// i_obuffer methods
//void i_obuffer::_on_buffer_not_flushed()
//{
//
//    std::cerr << "obuffer not flushed by derived class!" << std::endl;
//    std::abort();
//}

void i_obuffer::_on_bad_write()
{
    std::cerr << "Bad write to obuffer!" << std::endl;
    std::abort();
}

////////////////////////////////////////////////// file_ibuffer methods
//file_ibuffer::file_ibuffer(const std::string& file_name) : _fs(file_name, OPEN_MODE)
//{
//    THROW_ASSERT(std::filesystem::exists(file_name),
//                 "Input file \"" + file_name + "\" not found!");
//
//    THROW_ASSERT(_fs.is_open(),
//                 "Failed to open input file \"" + file_name + "\"!");
//}
//
