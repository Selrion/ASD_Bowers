#pragma once
#include <vector>
#include <map>
#include "GameTypes.h"

class GameBoard {
private:
    std::vector<std::vector<int>> board;
    std::map<Position, std::pair<Position, int>> moveHistory;
    std::map<int, int> killedUnits;
    std::map<int, std::vector<Position>> killerPositions;
    Player currentPlayer;

    void initializeAdjacency();
    bool isOnStartLine(const Position& pos, Player player) const;
    bool canShoot(const Position& from, const Position& to, Player shooter) const;
    void checkAndRemoveShot(const Position& movedTo);

public:
    std::vector<std::vector<std::vector<Position>>> adjacency;

    GameBoard();
    GameBoard(const GameBoard& other);

    void reset();
    int getCell(const Position& pos) const;
    void setCell(const Position& pos, int value);
    Player getCurrentPlayer() const { return currentPlayer; }
    void switchPlayer();

    bool isValidPosition(const Position& pos) const;
    bool isAdjacent(const Position& from, const Position& to) const;
    bool canMove(const Position& from, const Position& to) const;
    bool wouldViolateThreeMoveRule(const Position& from, const Position& to) const;

    std::vector<Move> getLegalMoves() const;
    void makeMove(const Move& move);

    bool isGameOver() const;
    Player getWinner() const;

    int getKilledUnits(Player player) const;
    bool canRevive(Player player, const Position& pos) const;
    std::vector<Position> getRevivalPositions(Player player) const;
};
