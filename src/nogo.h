//---------------------------------------------------------------------------
// Implementation of NoGo on a 2-dimensional rectangular board
//---------------------------------------------------------------------------

#pragma once

// IWYU pragma: begin_exports
#include "game.h"
#include "grid.h"
// IWYU pragma: end_exports

#include <vector>
#include <string>
#include <ostream>
#include <cstddef>

class nogo : public grid
{
public:
    nogo(std::string game_as_string);
    nogo(const std::vector<int>& board, int_pair shape);
    nogo(const std::vector<int>& board, const std::vector<int>& immortal, int_pair shape);
    void play(const move& m, bw to_play) override;
    void undo_move() override;

    bool is_legal() const;

    std::vector<int> immortal() const { return _immortal; }


protected:
    void _init_hash(local_hash& hash) const override;
    split_result _split_impl() const override;

public:
    game* inverse() const override;
    move_generator* create_move_generator(bw to_play) const override;

    void print(std::ostream& str) const override
    {
        str << "nogo:" << board_as_string();
    }

private:
    std::vector<int> _immortal; // BORDER for immortal points (stones),
                                // BLACK for B-Go due to board partitioning,
                                // WHITE for W-Go due to board partitioning,
                                // EMPTY for others.
    std::vector<int> _immortal_copy;    // A copy for undoing move
};

// Compact nogo board for fast legality checking and board partitioning.
class nogo_board
{
public:
    std::vector<int> board;
    std::vector<int> immortal;
    int size;
    int_pair shape;

    nogo_board(const std::vector<int> board, int_pair shape)
        : board(board), size(shape.first * shape.second), shape(shape)
    { immortal = std::vector<int>(size, EMPTY); };
    nogo_board(std::vector<int> board, std::vector<int> immortal, int_pair shape)
        : board(board), immortal(immortal), size(shape.first * shape.second), shape(shape)
    { };

    int& operator[](size_t pos) { return board[pos]; }
    int operator[](size_t pos) const { return board[pos]; }

    std::vector<int>::iterator begin() { return board.begin(); }
    std::vector<int>::iterator end() { return board.end(); }
    std::vector<int>::const_iterator begin() const { return board.begin(); }
    std::vector<int>::const_iterator end() const { return board.end(); }
};

// Define nogo rules.
class nogo_rule
{
public:
    // Check if it is legal to play at p.
    static bool is_legal(nogo_board board, int p, int toplay);
    // Check if a block at p has liberty.
    static bool has_liberty(const nogo_board& board, int p);
    // Return in-board neighbors of p.
    static std::vector<int> neighbors(const nogo_board& board, int p);
};

class split_by_nogo
{
public:
    static const int OCC = -1;      // occupied
    static const int B_GO = BLACK;  // B-Go
    static const int W_GO = WHITE;  // W-Go
    static const int T_GO = EMPTY;  // 2-Go
    static const int N_GO = BORDER; // No-Go / immortal

    static void classify_empty_points(const nogo_board& board, std::vector<int>& point_markers);

    // Mark all walls.
    static void identify_walls(const nogo_board& board, const std::vector<int>& point_markers, std::vector<bool>& wall_markers);

    // Mark the wall of color at the No-Go point.
    static void mark_wall_at_nogo(const nogo_board& board, int p, int color, const std::vector<int>& point_markers, std::vector<bool>& wall_markers);
    
    static nogo_board mark_region_at_point(const nogo_board& board, int p, const std::vector<int>& point_markers, const std::vector<bool>& wall_markers, std::vector<bool>& region_markers);
    
    static std::vector<nogo_board> split(const nogo_board& board);
};

std::ostream& operator<<(std::ostream& out, const nogo& g);
std::ostream& operator<<(std::ostream& os, const nogo_board& board);
