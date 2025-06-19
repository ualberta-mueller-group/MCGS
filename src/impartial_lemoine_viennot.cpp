//---------------------------------------------------------------------------
// Implementation of Lemoine - Viennot algorithm for impartial games
//---------------------------------------------------------------------------
#include "impartial_lemoine_viennot.h"

#include "cgt_nimber.h"
#include "hashing.h"
#include "impartial_game.h"
#include "transposition.h"

namespace lemoine_viennot{

std::vector<hash_t> nimber_hashcode::_codes;

void nimber_hashcode::init_codes(int max_nimber)
{
    _codes.reserve(max_nimber + 1);
    for (int i=0; i <= max_nimber; ++i)
    {
        nimber g(i);
        _codes[i] = g.get_local_hash();
    }

}
//---------------------------------------------------------------------------

inline hash_t combined_hash(const impartial_game* g, int nim_value)
{
    return g->get_local_hash() ^ nimber_hashcode::get(nim_value);
}

inline void tt_store(lemoine_viennot_tt& tt,
                     const impartial_game* g,
                     int nim_value)
{
    const hash_t hash = combined_hash(g, nim_value);
    auto tt_result = tt.search(hash);
    tt_result.set_entry(lv_ttable_entry(nim_value));
}

inline bool tt_lookup(lemoine_viennot_tt& tt,
                      const impartial_game* g,
                      int nim_value, 
                      bool& result)
{
    const hash_t hash = combined_hash(g, nim_value);
    auto tt_result = tt.search(hash);
    const bool is_valid = tt_result.entry_valid();
    if (is_valid)
        result = tt_result.get_entry().value;
    return is_valid;
}

int search_with_tt(const impartial_game& g, int tt_size)
{
    lemoine_viennot_tt tt(tt_size, 0);
    return search_impartial_game(g, tt);
}

int search_impartial_game(const impartial_game& g, lemoine_viennot_tt& tt)
{
    int result = 0;
    assert(result >= 0);
    return result;
}

} // namespace lemoine_viennot
