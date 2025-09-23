#pragma once

#include <unordered_set>
#include <vector>

#include "grid_generator.h"
#include "grid.h"

//////////////////////////////////////////////////
// class grid_generator_masked_cc

class grid_generator_masked_cc: public grid_generator_masked
{
public:
    grid_generator_masked_cc(const int_pair& max_shape, bool only_max);
    grid_generator_masked_cc(int max_rows, int max_cols, bool only_max);
    grid_generator_masked_cc(int max_cols, bool only_max);

    virtual ~grid_generator_masked_cc() {}

    //void operator++() override;

    const std::unordered_set<hash_t>& get_mask_hashes() const
    {
        return _mask_hashes;
    }

protected:
    std::unordered_set<hash_t> _mask_hashes;

    static bool _is_one_cc(const grid_generator_impl::grid_mask& board, int_pair shape);

    void _init_mask() override;
    bool _increment_mask() override;

    void _scroll_mask_to_cc();
    void _init();

    const bool _only_max;
    grid_generator_impl::grid_mask _mask;

};

//////////////////////////////////////////////////
// class grid_generator_clobber_cc
class grid_generator_clobber_cc: public grid_generator_masked_cc
{
public:
    grid_generator_clobber_cc(const int_pair& max_shape);
    grid_generator_clobber_cc(int max_rows, int max_cols);
    grid_generator_clobber_cc(int max_cols);

    virtual ~grid_generator_clobber_cc() {}

protected:
    void _init_board() override;
    bool _increment_board() override;
};



//////////////////////////////////////////////////
void test_gen_components2();
