#include "elephants_rhinos.h"
#include "cgt_basics.h"
#include "cgt_move.h"
#include "game.h"


using std::cout, std::endl;

class elephants_rhinos_move_generator : public move_generator
{
public:
    elephants_rhinos_move_generator(const elephants_rhinos& game, bw to_play);

    void operator++() override;
    operator bool() const override;
    move gen_move() const override;

private:
    bool is_move(int from, int to, bw to_play) const;

    const elephants_rhinos& _game;
    int _idx;
    int _dir;
};


int player_dir(bw to_play)
{
    assert(is_black_white(to_play));
    return (to_play == BLACK) ? 1 : -1;
}


elephants_rhinos::elephants_rhinos(const std::string& game_as_string)
    : strip(game_as_string)
{ }


elephants_rhinos::elephants_rhinos(const std::vector<int>& board)
    : strip(board)
{
}



void elephants_rhinos::play(const move& m, bw to_play)
{
    assert_black_white(to_play);

    game::play(m, to_play);


    int from = cgt_move::from(m);
    int to = cgt_move::to(m);

    assert(checked_is_color(from, to_play));
    assert(checked_is_color(to, EMPTY));
    assert((to - from) == (player_dir(to_play)) ); // correct direction

    play_stone(to, to_play);
    remove_stone(from);
}

void elephants_rhinos::undo_move()
{
    move mc = game::last_move();
    game::undo_move();
    
    int to;
    bw to_play;
    int from = cgt_move::decode3(mc, &to, &to_play);

    assert(is_black_white(to_play));
    assert(checked_is_color(to, to_play));
    assert(checked_is_color(from, EMPTY));

    play_stone(from, to_play);
    remove_stone(to);
}

move_generator* elephants_rhinos::create_move_generator(bw to_play) const
{ 
    return new elephants_rhinos_move_generator(*this, to_play);
}

game* elephants_rhinos::inverse() const
{
    return new elephants_rhinos(inverse_board());
}
 // caller takes ownership



elephants_rhinos_move_generator::
    elephants_rhinos_move_generator(const elephants_rhinos& game, bw to_play)
    : move_generator(to_play), _game(game)
{
    _idx = move_generator::to_play() == BLACK ? 0 : _game.size() - 1;
    _dir = player_dir(to_play);

    if (!is_move(_idx, _idx + _dir, to_play) && _game.size() >= 2)
    {
        ++(*this);
    }
    

}

void elephants_rhinos_move_generator::operator++()
{
    _idx += _dir;

    for (; in_range(_idx, 0, _game.size()); _idx += _dir)
    {
        if (is_move(_idx, _idx + _dir, to_play()))
        {
            break;
        }
    }
}

elephants_rhinos_move_generator::operator bool() const
{
    return in_range(_idx, 0, _game.size());
}

move elephants_rhinos_move_generator::gen_move() const
{
    assert(is_move(_idx, _idx + _dir, to_play()));
    return cgt_move::two_part_move(_idx, _idx + _dir);
}




bool elephants_rhinos_move_generator::is_move(int from, int to, bw to_play) const
{
    if (!in_range(from, 0, _game.size()) || !in_range(to, 0, _game.size()))
    {
        return false;
    }

    int fromColor = _game.at(from);
    int toColor = _game.at(to);

    if (
        (fromColor == to_play)
        && is_black_white(to_play)
        && (toColor == EMPTY)
        && ((to - from) == player_dir(to_play))
    )
    {
        return true;
    }

    return false;
}
