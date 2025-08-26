/*
    Interface type for generator used by database creation process
*/
#pragma once
#include "game.h"


////////////////////////////////////////////////// class db_game_generator
class db_game_generator
{
public:
    virtual ~db_game_generator() {}

    virtual operator bool() const = 0;
    virtual void operator++() = 0;
    virtual game* gen_game() const = 0;
};
