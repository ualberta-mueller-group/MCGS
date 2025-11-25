#pragma once

// IWYU pragma: begin_exports
#include "game.h"
#include "strip.h"
// IWYU pragma: end_exports


#include <vector>
#include <string>
#include <cassert>
#include <ostream>

#include "cgt_basics.h"

////////////////////////////////////////////////// class toppling_dominoes
class toppling_dominoes: public game
{
public:
    toppling_dominoes(const std::vector<int>& board);
    toppling_dominoes(const std::string& game_as_string);

    void play(const move& m, bw to_play) override;
    void undo_move() override;

    move_generator* create_move_generator(bw to_play) const override;

    void print(std::ostream& str) const override;
    void print_move(std::ostream& str, const move& m) const override;

    game* inverse() const override;

    int n_dominoes() const;
    int get_domino_at(int idx_virtual) const;

    const std::vector<int> current_dominoes() const;

protected:
    friend class toppling_dominoes_move_generator;

    void _flip();
    int _idx_virtual_to_real(int idx_virtual) const;

    void _init_hash(local_hash& hash) const override;
    void _normalize_impl() override;
    void _undo_normalize_impl() override;
    relation _order_impl(const game* rhs) const override;

    int _domino_start; // real index of 1st domino
    int _domino_end; // real index of 1 past last domino
    bool _domino_flip_orientation; // IFF true: reverse index

    std::vector<bool> _normalize_did_flip;
    const std::vector<int> _initial_dominoes;

};

////////////////////////////////////////////////// toppling_dominoes methods

// TODO inlining probably not good here
inline int toppling_dominoes::n_dominoes() const
{
    if (_domino_start < _domino_end)
        return _domino_end - _domino_start;

    return 0;
}

inline int toppling_dominoes::get_domino_at(int idx_virtual) const
{
    return _initial_dominoes[_idx_virtual_to_real(idx_virtual)];
}

inline void toppling_dominoes::_flip()
{
    _domino_flip_orientation = !_domino_flip_orientation;
}

inline int toppling_dominoes::_idx_virtual_to_real(int idx_virtual) const
{
    assert(0 <= idx_virtual && idx_virtual < n_dominoes());

    if (_domino_flip_orientation)
        return (_domino_end - 1) - idx_virtual;

    return _domino_start + idx_virtual;
}


