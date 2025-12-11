#pragma once
#include "GameBoard.h"

class AIPlayer {
private:
    Player aiPlayer;
    int maxDepth;

    int evaluate(const GameBoard& board) const;
    int minimax(GameBoard& board, int depth, int alpha, int beta, bool maximizing);

public:
    AIPlayer(Player player, int depth = 3);
    Move getBestMove(GameBoard& board);
};
