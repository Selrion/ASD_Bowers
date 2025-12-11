#pragma once
#include "Position.h"

enum Player { NONE = 0, PLAYER1 = 1, PLAYER2 = 2 };

struct Move {
    Position from;
    Position to;
    bool isRevival;
    Position revivePos;

    Move() : isRevival(false) {}
    Move(Position f, Position t) : from(f), to(t), isRevival(false) {}
};