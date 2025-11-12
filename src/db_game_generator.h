/*
    Interface type for generator used by database creation process
*/
#pragma once
#include "game.h"

////////////////////////////////////////////////// class i_db_game_generator
class i_db_game_generator
{
public:
    virtual ~i_db_game_generator() {}

    virtual operator bool() const = 0;
    virtual void operator++() = 0;
    virtual game* gen_game() const = 0;
};


