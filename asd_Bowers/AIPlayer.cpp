#include "AIPlayer.h"
#include <algorithm>
#include <climits>

AIPlayer::AIPlayer(Player player, int depth) : aiPlayer(player), maxDepth(depth) {}

int AIPlayer::evaluate(const GameBoard& board) const {
    if (board.isGameOver()) {
        Player winner = board.getWinner();
        if (winner == aiPlayer) return 10000;
        if (winner != NONE) return -10000;
    }

    int score = 0;
    Player opponent = (aiPlayer == PLAYER1) ? PLAYER2 : PLAYER1;

    int aiTargetRow = (aiPlayer == PLAYER1) ? 4 : 0;
    int oppTargetRow = (opponent == PLAYER1) ? 4 : 0;

    int aiOnTarget = 0, oppOnTarget = 0;
    int aiProgress = 0, oppProgress = 0;

    for (int r = 0; r < 5; r++) {
        for (int c = 0; c < 5; c++) {
            int cell = board.getCell(Position(r, c));

            if (cell == aiPlayer) {
                if (r == aiTargetRow) {
                    aiOnTarget++;
                    score += 500;
                }
                else {
                    int dist = abs(r - aiTargetRow);
                    aiProgress += (4 - dist);
                    score += (4 - dist) * 20;
                }
            }
            else if (cell == opponent) {
                if (r == oppTargetRow) {
                    oppOnTarget++;
                    score -= 500;
                }
                else {
                    int dist = abs(r - oppTargetRow);
                    oppProgress += (4 - dist);
                    score -= (4 - dist) * 20;
                }
            }
        }
    }

    score += aiOnTarget * 100;
    score -= oppOnTarget * 100;

    int aiKilled = board.getKilledUnits(aiPlayer);
    int oppKilled = board.getKilledUnits(opponent);
    score -= aiKilled * 150;
    score += oppKilled * 150;

    return score;
}

int AIPlayer::minimax(GameBoard& board, int depth, int alpha, int beta, bool maximizing) {
    if (depth == 0 || board.isGameOver()) {
        return evaluate(board);
    }

    std::vector<Move> moves = board.getLegalMoves();

    if (moves.empty()) {
        return evaluate(board);
    }

    if (maximizing) {
        int maxEval = INT_MIN;
        for (const auto& move : moves) {
            GameBoard newBoard = board;
            newBoard.makeMove(move);
            newBoard.switchPlayer();

            int eval = minimax(newBoard, depth - 1, alpha, beta, false);
            maxEval = std::max(maxEval, eval);
            alpha = std::max(alpha, eval);

            if (beta <= alpha) break;
        }
        return maxEval;
    }
    else {
        int minEval = INT_MAX;
        for (const auto& move : moves) {
            GameBoard newBoard = board;
            newBoard.makeMove(move);
            newBoard.switchPlayer();

            int eval = minimax(newBoard, depth - 1, alpha, beta, true);
            minEval = std::min(minEval, eval);
            beta = std::min(beta, eval);

            if (beta <= alpha) break;
        }
        return minEval;
    }
}

Move AIPlayer::getBestMove(GameBoard& board) {
    std::vector<Move> moves = board.getLegalMoves();

    if (moves.empty()) {
        return Move();
    }

    Move bestMove = moves[0];
    int bestScore = INT_MIN;

    for (const auto& move : moves) {
        GameBoard newBoard = board;
        newBoard.makeMove(move);
        newBoard.switchPlayer();

        int score = minimax(newBoard, maxDepth - 1, INT_MIN, INT_MAX, false);

        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
        }
    }

    return bestMove;
}
