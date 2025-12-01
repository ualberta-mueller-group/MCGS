#pragma once

// IWYU pragma: begin_exports
#include "game.h"
#include "strip.h"
// IWYU pragma: end_exports

#include <vector>
#include <string>
#include <ostream>

////////////////////////////////////////////////// class gen_toads
class gen_toads: public strip
{
public:
    /*
        params is a vector of 4 elements:
        - min slide > 0
        - max slide >= min slide
        - max jump >= 0
        - friendly jump in [0, 1]

        BLACK moves right, WHITE moves left.

        A stone's moves are only one of the following:
        - The stone has no move.

        - The stone is not immediately blocked and can slide
          [min slide, max slide] spaces (without jumping over stones).

        - The stone is immediately blocked and can jump over (0, max jump]
            opponent stones.

            - If friendly jump is 1, the stone can jump over (possibly a mix of)
              friendly or opponent stones.
    */
    gen_toads(const std::vector<int>& params, const std::vector<int>& board);
    gen_toads(const std::vector<int>& params, const std::string& game_as_string);

    void play(const move& m, bw to_play) override;
    void undo_move() override;
    move_generator* create_move_generator(bw to_play) const override;
    void print(std::ostream& str) const override;
    void print_move(std::ostream& str, const move& m) const override;
    game* inverse() const override;

    int get_min_slide() const;
    int get_max_slide() const;
    int get_max_jump() const;
    bool get_friendly_jump() const;

protected:
    /*
       TODO how to do this?
    */
    //split_result _split_impl() const override;
    void _init_hash(local_hash& hash) const override;
    void _normalize_impl() override;
    void _undo_normalize_impl() override;
    relation _order_impl(const game* rhs) const override;

    void _init_params(const std::vector<int>& params);


    int _min_slide;
    int _max_slide;
    int _max_jump;
    bool _friendly_jump;
};

////////////////////////////////////////////////// gen_toads methods
inline int gen_toads::get_min_slide() const
{
    return _min_slide;
}

inline int gen_toads::get_max_slide() const
{
    return _max_slide;
}

inline int gen_toads::get_max_jump() const
{
    return _max_jump;
}

inline bool gen_toads::get_friendly_jump() const
{
    return _friendly_jump;
}

