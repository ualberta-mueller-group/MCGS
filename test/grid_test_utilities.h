#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cassert>
#include <cstddef>
#include "grid.h"
#include "game.h"

template <class T>
void assert_grid_game_move_sequence(const std::string& board,
    const std::vector<std::string>& black_options,
    const std::vector<std::string>& white_options);

template <class T>
void assert_grid_game_inverse(const std::vector<std::string>& boards);

////////////////////////////////////////////////// template implementations

template <class T>
void assert_grid_game_move_sequence(const std::string& board,
    const std::vector<std::string>& black_options,
    const std::vector<std::string>& white_options)
{
    static_assert(std::is_base_of_v<grid, T>);
    T g(board);
    grid& g_grid = g;

    std::vector<bw> colors = {BLACK, WHITE};
    for (const bw color : colors)
    {
        const std::vector<std::string>& options = color == BLACK ?
            black_options : white_options;

        std::unique_ptr<move_generator> mg(g_grid.create_move_generator(color));

        const size_t N = options.size();
        for (size_t i = 0; i < N; i++)
        {
            assert(*mg);
            move m = mg->gen_move();
            ++(*mg);
            g_grid.play(m, color);
            assert(g_grid.board_as_string() == options[i]);
            g_grid.undo_move();
            assert(g_grid.board_as_string() == board);
        }
        assert(!(*mg));
    }
}

template <class T>
void assert_grid_game_inverse(const std::string& board)
{
    static_assert(std::is_base_of_v<grid, T>);

    std::string board_inv = board;
    for (char& c : board_inv)
    {
        if (c == 'O')
            c = 'X';
        else if (c == 'X')
            c = 'O';
    }

    T g(board);

    game* g_inv = g.inverse();
    grid* g_inv_grid = dynamic_cast<grid*>(g_inv);
    assert(g_inv_grid != nullptr);

    assert(g_inv_grid->board_as_string() == board_inv);
    delete g_inv;
}

