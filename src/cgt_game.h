#pragma once

#include "game.h"

#include <vector>
#include <memory>

////////////////////////////////////////////////// class i_cgt_game_base
class i_cgt_game_base: public game
{
public:
    virtual void get_flat_hash_elements(std::vector<hash_t>& elements) const = 0;

    static void get_flat_hash_elements_single(std::vector<hash_t>& elements,
                                              const game* g);
    hash_t get_flat_hash_value() const;

protected:
private:
};

////////////////////////////////////////////////// class game_sum
class game_sum: public i_cgt_game_base
{
public:
    game_sum(std::vector<std::shared_ptr<const game>> games);

    void play(const move& m, bw to_play) override;
    void undo_move() override;

    move encode_grid_move_to_db(const move& m) const override;
    move decode_grid_move_from_db(const move& m) const override;

    void print(std::ostream& str) const override;
    void print_move(std::ostream& str, const move& m, ebw to_play) const override;
    game* inverse() const override; // caller takes ownership
    game* clone() const override; // caller takes ownership

    void get_flattened_operands(std::vector<std::shared_ptr<const game>>& operands) const;

    void get_flat_hash_elements(std::vector<hash_t>& elements) const override;

protected:
    move_generator* _create_move_generator_impl(bw to_play) const override;
    split_result _split_impl() const override;
    void _init_hash(local_hash& hash) const override;

private:
    const std::vector<std::shared_ptr<const game>> _games;
};

////////////////////////////////////////////////// class cgt_game
class cgt_game: public i_cgt_game_base
{
public:
    cgt_game(std::vector<std::shared_ptr<const game>> left_options,
             std::vector<std::shared_ptr<const game>> right_options);

    void play(const move& m, bw to_play) override;
    void undo_move() override;

    void print(std::ostream& str) const override;
    void print_move(std::ostream& str, const move& m, ebw to_play) const override;
    game* inverse() const override; // caller takes ownership
    game* clone() const override; // caller takes ownership

    void get_flat_hash_elements(std::vector<hash_t>& elements) const override;

    const std::shared_ptr<const game>& get_selected_option() const;
    const std::vector<std::shared_ptr<const game>>& get_player_option_set(
        bw player) const;

protected:
    static void _print_option_set(
        std::ostream& str,
        const std::vector<std::shared_ptr<const game>>& options);

    move_generator* _create_move_generator_impl(bw to_play) const override;
    split_result _split_impl() const override;
    void _init_hash(local_hash& hash) const override;

private:
    cgt_game(std::shared_ptr<const game> selected);

    const std::vector<std::shared_ptr<const game>> _left_options;
    const std::vector<std::shared_ptr<const game>> _right_options;

    std::shared_ptr<const game> _selected;
};

