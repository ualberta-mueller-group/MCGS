#include "init_serialization.h"

#include "dynamic_serializable.h"
#include "all_game_headers.h"

namespace mcgs_init {

void init_serialization()
{
    register_dyn_serializable<clobber_1xn>();
    register_dyn_serializable<nogo_1xn>();
    register_dyn_serializable<elephants>();
    register_dyn_serializable<toppling_dominoes>();

    register_dyn_serializable<amazons>();
    register_dyn_serializable<nogo>();
    register_dyn_serializable<clobber>();
    register_dyn_serializable<cannibal_clobber>();
    register_dyn_serializable<domineering>();
    register_dyn_serializable<fission>();
    register_dyn_serializable<sheep>();
}

} // namespace mcgs_init

    //// Serialization
    //void save_impl(i_obuffer& os, serializer_ctx* ctx) const override;
    //static dyn_serializable* load_impl(i_ibuffer& is, serializer_ctx* ctx);

//void amazons::save_impl(i_obuffer& os, serializer_ctx* ctx) const
//{
//    save_board(os, board_const(), shape(), ctx);
//}
//
//dyn_serializable* amazons::load_impl(i_ibuffer& is, serializer_ctx* ctx)
//{
//    pair<vector<int>, int_pair> board_pair = load_board(is, ctx);
//    return new amazons(board_pair.first, board_pair.second);
//}

