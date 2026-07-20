#include "test_serialization_stuff.h"
#include "byte_order.h"
#include "iobuffer.h"
#include "random.h"
#include "serializer.h"
#include "stopwatch.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <endian.h>
#include <iostream>
#include <limits>
#include <string>
#include <cstdint>
#include <cassert>
#include <cstddef>
#include <type_traits>
#include <vector>

using namespace std;
struct thing32
{
    thing32() {}
    thing32(int32_t val): val(val) {}

    bool operator==(const thing32& rhs) const
    {
        return val == rhs.val;
    }

    int32_t val;
};

template<>
struct serializer<thing32>
{
    static void save(i_obuffer& os, const thing32& val, serializer_ctx* ctx)
    {
        os.write_i32(val.val);
    }

    static thing32 load(i_ibuffer& is, serializer_ctx* ctx)
    {
        return thing32(is.read_i32());
    }
};

static_assert(sizeof(thing32) == sizeof(int32_t));
static_assert(alignof(thing32) == alignof(int32_t));
static_assert(is_trivially_copyable_v<thing32>);

////////////////////////////////////////////////// Helpers
namespace {


vector<thing32> gen_data_thing32(uint64_t size)
{
    vector<thing32> data;
    data.reserve(size);

    random_generator gen(8641);

    for (uint64_t i = 0; i < size; i++)
        data.emplace_back(gen.get_i32());

    return data;
}

// fwrite without handling endianness
void save_trivial_thing32(const vector<thing32>& data, const string& file_name)
{
    FILE* f = fopen(file_name.c_str(), "w");
    assert(f != nullptr);

    const size_t size = data.size();

    fwrite(&size, sizeof(size_t), 1, f);
    fwrite(data.data(), sizeof(thing32), data.size(), f);

    fclose(f);
}

vector<thing32> load_trivial_thing32(const string& file_name)
{
    FILE* f = fopen(file_name.c_str(), "r");
    assert(f != nullptr);

    size_t size;
    fread(&size, sizeof(size_t), 1, f);

    vector<thing32> vec(size);
    fread(vec.data(), sizeof(thing32), size, f);

    fclose(f);

    return vec;
}

// fwrite after handling endianness
void save_best_case_thing32(const vector<thing32>& data, const string& file_name)
{
    const size_t n_elements = data.size();
    const size_t n_bytes = n_elements * sizeof(int32_t);

    int32_t* out_buffer = new int32_t[n_elements];
    memcpy(out_buffer, data.data(), n_bytes);

    for (size_t i = 0; i < n_elements; i++)
        out_buffer[i] = htobe32(out_buffer[i]);

    FILE* f = fopen(file_name.c_str(), "w");
    assert(f != nullptr);

    fwrite(&n_elements, sizeof(size_t), 1, f);

    //fwrite(out_buffer, sizeof(int32_t), n_elements, f);
    fwrite(out_buffer, 1, n_bytes, f);
    fclose(f);

    delete[] out_buffer;
}

vector<thing32> load_best_case_thing32(const string& file_name)
{
    FILE* f = fopen(file_name.c_str(), "r");
    assert(f != nullptr);

    size_t n_elements;
    fread(&n_elements, sizeof(size_t), 1, f);

    vector<thing32> data(n_elements);
    fread(data.data(), sizeof(thing32), n_elements, f);

    for (size_t i = 0; i < n_elements; i++)
        data[i].val = be32toh(data[i].val);

    fclose(f);

    return data;
}

// production implementation
void save_production_thing32(const vector<thing32>& data, const string& file_name)
{
    file_obuffer os(file_name);
    serializer<vector<thing32>>::save(os, data, nullptr);
}

vector<thing32> load_production_thing32(const string& file_name)
{
    file_ibuffer is(file_name);
    return serializer<vector<thing32>>::load(is, nullptr);
}

void test_save_load(const string& file_name)
{
    random_generator gen(1234);

    const uint64_t val = gen.get_u64();

    cout << "Generated number: " << val << endl;

    {
        file_obuffer os(file_name);
        serializer<uint64_t>::save(os, val, nullptr);
    }

    {
        file_ibuffer is(file_name);
        const uint64_t val_loaded = serializer<uint64_t>::load(is, nullptr);
        cout << "Loaded number: " << val_loaded << endl;
    }

}

vector<uint8_t> gen_data_u8(size_t n_bytes)
{
    random_generator gen(42536);
    vector<uint8_t> data(n_bytes);

    for (size_t i = 0; i < n_bytes; i++)
        data[i] = gen.get_u8();

    return data;
}

void save_trivial_u8(const vector<uint8_t>& data, const string& file_name)
{
    FILE* f = fopen(file_name.c_str(), "w");
    assert(f != nullptr);

    const size_t size = data.size();

    fwrite(&size, sizeof(size_t), 1, f);
    fwrite(data.data(), sizeof(uint8_t), size, f);

    fclose(f);
}

vector<uint8_t> load_trivial_u8(const string& file_name)
{
    FILE* f = fopen(file_name.c_str(), "r");
    assert(f != nullptr);

    size_t size;
    fread(&size, sizeof(size_t), 1, f);

    vector<uint8_t> data(size);
    fread(data.data(), sizeof(uint8_t), 1, f);

    fclose(f);

    return data;
}

void save_production_u8(const vector<uint8_t>& data, const string& file_name)
{
    file_obuffer os(file_name);
    serializer<vector<uint8_t>>::save(os, data, nullptr);
}

vector<uint8_t> load_production_u8(const string& file_name)
{
    file_ibuffer is(file_name);
    return serializer<vector<uint8_t>>::load(is, nullptr);
}

void test_serialization_stuff_thing32()
{
    test_save_load("temp/0.bin");

    stopwatch sw;
    const uint64_t n_elements = uint64_t(128) * 1024 * 1024; // 512 MiB
    const vector<thing32> data = gen_data_thing32(n_elements);

#define N_ITERS 1

    cout << "\t(Writing times):" << endl;

    // Trivial case
    sw.reset();
    sw.start();
    for (int i = 0; i < N_ITERS; i++)
        save_trivial_thing32(data, "temp/1.bin");
    sw.stop();
    cout << "Trivial case: " << (sw.get_duration_ms() / 1000.0) << " seconds"
         << endl;

    // Best case
    sw.reset();
    sw.start();
    for (int i = 0; i < N_ITERS; i++)
        save_best_case_thing32(data, "temp/2.bin");
    sw.stop();
    cout << "Best case: " << (sw.get_duration_ms() / 1000.0) << " seconds"
         << endl;

    // Production case
    sw.reset();
    sw.start();
    for (int i = 0; i < N_ITERS; i++)
        save_production_thing32(data, "temp/3.bin");
    sw.stop();
    cout << "Production case: " << (sw.get_duration_ms() / 1000.0) << " seconds"
         << endl;

    cout << endl;
    cout << "\t(Reading times):" << endl;


    // Trivial case
    sw.reset();
    sw.start();
    for (int i = 0; i < N_ITERS; i++)
        const vector<thing32> loaded = load_trivial_thing32("temp/1.bin");
    sw.stop();
    cout << "Trivial case: " << (sw.get_duration_ms() / 1000.0) << " seconds"
         << endl;

    // Best case
    sw.reset();
    sw.start();

    for (int i = 0; i < N_ITERS; i++)
        const vector<thing32> loaded = load_best_case_thing32("temp/2.bin");
    sw.stop();
    cout << "Best case: " << (sw.get_duration_ms() / 1000.0) << " seconds"
         << endl;

    // Production case
    sw.reset();
    sw.start();
    for (int i = 0; i < N_ITERS; i++)
        const vector<thing32> loaded = load_production_thing32("temp/3.bin");
    sw.stop();
    cout << "Production case: " << (sw.get_duration_ms() / 1000.0) << " seconds"
         << endl;
}

void test_serialization_stuff_u8()
{
    test_save_load("temp/0.bin");
    stopwatch sw;

    // 512 MiB
    const vector<uint8_t> data = gen_data_u8(size_t(512) * 1024 * 1024);

    #define N_ITERS 1

    cout << "\t(Writing times):" << endl;

    // Trivial case
    sw.reset();
    sw.start();
    for (int i = 0; i < N_ITERS; i++)
        save_trivial_u8(data, "temp/1.bin");
    sw.stop();
    cout << "Trivial case: " << (sw.get_duration_ms() / 1000.0) << " seconds"
         << endl;

    // Production case
    sw.reset();
    sw.start();
    for (int i = 0; i < N_ITERS; i++)
        save_production_u8(data, "temp/3.bin");
    sw.stop();
    cout << "Production case: " << (sw.get_duration_ms() / 1000.0) << " seconds"
         << endl;

    cout << endl;
    cout << "\t(Reading times):" << endl;

    // Trivial case
    sw.reset();
    sw.start();
    for (int i = 0; i < N_ITERS; i++)
        const vector<uint8_t> loaded = load_trivial_u8("temp/1.bin");
    sw.stop();
    cout << "Trivial case: " << (sw.get_duration_ms() / 1000.0) << " seconds"
         << endl;

    // Production case
    sw.reset();
    sw.start();
    for (int i = 0; i < N_ITERS; i++)
        const vector<uint8_t> loaded = load_production_u8("temp/3.bin");
    sw.stop();
    cout << "Production case: " << (sw.get_duration_ms() / 1000.0) << " seconds"
         << endl;

}

} // namespace

////////////////////////////////////////////////// Exported functions

void test_serialization_stuff()
{
    test_serialization_stuff_thing32();
    //test_serialization_stuff_u8();
}
