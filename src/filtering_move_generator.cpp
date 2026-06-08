#include "filtering_move_generator.h"

#include <cassert>

#include "game.h"

void filtering_move_generator::_increment(bool init)
{
    assert(init || *this);

    assert(_gen.get() != nullptr);
    move_generator& gen = *_gen;

    if (!init)
    {
        assert(gen);
        ++gen;
    }

    while (1)
    {
        if (!gen)
        {
            _gen.reset();
            _has_move = false;
            return;
        }

        _m = gen.gen_move();

        const move m_enc = _g.encode_grid_move_to_db(_m);
        if (_db_encoded_dom_moves.find(m_enc) != _db_encoded_dom_moves.end())
        {
            // m is dominated
            ++gen;
            continue;
        }

        _has_move = true;
        return;
    }
}
