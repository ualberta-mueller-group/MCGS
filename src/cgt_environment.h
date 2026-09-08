#pragma once
#include <string>

#include "ast2_node.h"

class game;

class cgt_environment
{
public:
    cgt_environment(std::shared_ptr<const ast2_sum> sum_node);
    game* make_game() const;

private:
    const std::shared_ptr<const ast2_sum> _sum_node;
};

void test_cgt_environment(const std::string& env_string, size_t line_start, size_t column_start);
cgt_environment parse_cgt_environment(const std::string& env_string, size_t line_start, size_t column_start);

