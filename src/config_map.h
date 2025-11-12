#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <optional>

#include "int_pair.h"

class config_map
{
public:
    /*
        String of key/value pairs. Key and value are separated by a '='
        character, and each pair ends with a mandatory ';' character.
    */
    config_map(const std::string& config_string);

    void check_unused_keys() const;

    /*
       GETTER FUNCTIONS

       These functions look up the value string for a given key, and try to
       perform the conversion. The returned optional has a value IFF the
       key exists and conversion of its value was successful.
    */

    // Vector of 0 or more ints
    std::optional<std::vector<int>> get_int_vec(const std::string& key) const;

    // Calls get_int_vec(). Successful IFF an int vector with exactly 1 or 2
    // non-negative elements is found. In the case of 1 int, int_pair(1,
    // found_int) is returned.
    std::optional<int_pair> get_dims(const std::string& key) const;

    std::optional<int> get_int(const std::string& key) const;

private:
    struct value_t
    {
        std::string value_string;
        mutable bool used;
    };

    const std::string* _get_value_string(const std::string& key) const;

    std::unordered_map<std::string, value_t> _key_value_map;

    friend std::ostream& operator<<(std::ostream&, const config_map&);
};

std::ostream& operator<<(std::ostream& os, const config_map& config);
