#include "GameBoard.h"
#include <algorithm>

GameBoard::GameBoard() : currentPlayer(PLAYER1) {
    board.resize(5, std::vector<int>(5, NONE));

    for (int i = 0; i < 5; i++) {
        board[0][i] = PLAYER1;
        board[4][i] = PLAYER2;
    }

    killedUnits[PLAYER1] = 0;
    killedUnits[PLAYER2] = 0;

    initializeAdjacency();
}

GameBoard::GameBoard(const GameBoard& other) {
    board = other.board;
    moveHistory = other.moveHistory;
    killedUnits = other.killedUnits;
    killerPositions = other.killerPositions;
    currentPlayer = other.currentPlayer;
    adjacency = other.adjacency;
}

void GameBoard::initializeAdjacency() {
    adjacency.resize(5);
    for (int i = 0; i < 5; i++) {
        adjacency[i].resize(5);
    }

    // Row 0
    adjacency[0][0] = { {1,0}, {1,1} };
    adjacency[0][1] = { {0,0}, {1,0}, {1,1}, {1,2}, {2,1} };
    adjacency[0][2] = { {0,1}, {1,1}, {1,2}, {1,3}, {2,2} };
    adjacency[0][3] = { {0,2}, {1,2}, {1,3}, {1,4}, {2,3} };
    adjacency[0][4] = { {0,3}, {1,3}, {1,4}, {2,3} };

    // Row 1
    adjacency[1][0] = { {0,0}, {0,1}, {2,0}, {2,1}, {3,0} };
    adjacency[1][1] = { {0,1}, {1,0}, {2,1}, {2,2}, {3,1} };
    adjacency[1][2] = { {0,2}, {1,1}, {2,2}, {2,3}, {3,2} };
    adjacency[1][3] = { {0,3}, {1,2}, {2,3}, {2,4}, {3,3} };
    adjacency[1][4] = { {0,4}, {1,3}, {2,4}, {3,3}, {3,4} };

    // Row 2
    adjacency[2][0] = { {1,0}, {3,0} };
    adjacency[2][1] = { {0,1}, {1,0}, {1,1}, {3,1}, {4,1} };
    adjacency[2][2] = { {0,2}, {1,1}, {1,2}, {3,2}, {4,2} };
    adjacency[2][3] = { {0,3}, {1,3}, {1,4}, {3,3}, {4,3} };
    adjacency[2][4] = { {1,4}, {3,4} };

    // Row 3
    adjacency[3][0] = { {1,0}, {2,0}, {4,0}, {4,1} };
    adjacency[3][1] = { {1,1}, {2,1}, {3,0}, {4,1}, {4,2} };
    adjacency[3][2] = { {1,2}, {2,2}, {3,1}, {4,2}, {4,3} };
    adjacency[3][3] = { {1,3}, {2,3}, {3,2}, {4,3}, {4,4} };
    adjacency[3][4] = { {1,4}, {2,4}, {3,3}, {4,4} };

    // Row 4
    adjacency[4][0] = { {3,0}, {4,1} };
    adjacency[4][1] = { {2,1}, {3,0}, {3,1}, {4,0}, {4,2} };
    adjacency[4][2] = { {2,2}, {3,2}, {3,3}, {4,1}, {4,3} };
    adjacency[4][3] = { {2,3}, {3,3}, {3,4}, {4,2}, {4,4} };
    adjacency[4][4] = { {3,4}, {4,3} };
}

void GameBoard::reset() {
    board.assign(5, std::vector<int>(5, NONE));

    for (int i = 0; i < 5; i++) {
        board[0][i] = PLAYER1;
        board[4][i] = PLAYER2;
    }

    moveHistory.clear();
    killedUnits[PLAYER1] = 0;
    killedUnits[PLAYER2] = 0;
    killerPositions.clear();
    currentPlayer = PLAYER1;
}

int GameBoard::getCell(const Position& pos) const {
    if (!isValidPosition(pos)) return NONE;
    return board[pos.row][pos.col];
}

void GameBoard::setCell(const Position& pos, int value) {
    if (isValidPosition(pos)) {
        board[pos.row][pos.col] = value;
    }
}

void GameBoard::switchPlayer() {
    currentPlayer = (currentPlayer == PLAYER1) ? PLAYER2 : PLAYER1;
}

bool GameBoard::isValidPosition(const Position& pos) const {
    return pos.row >= 0 && pos.row < 5 && pos.col >= 0 && pos.col < 5;
}

bool GameBoard::isAdjacent(const Position& from, const Position& to) const {
    if (!isValidPosition(from) || !isValidPosition(to)) return false;

    const auto& neighbors = adjacency[from.row][from.col];
    return std::find(neighbors.begin(), neighbors.end(), to) != neighbors.end();
}

bool GameBoard::isOnStartLine(const Position& pos, Player player) const {
    // Player 1 starts at row 0, targets row 4 (opponent's start)
    // Player 2 starts at row 4, targets row 0 (opponent's start)
    if (player == PLAYER1) return pos.row == 4; // On opponent's start line
    if (player == PLAYER2) return pos.row == 0; // On opponent's start line
    return false;
}

bool GameBoard::canShoot(const Position& from, const Position& to, Player shooter) const {
    // Can't shoot while on opponent's start line
    if (isOnStartLine(from, shooter)) return false;
    // Can only shoot adjacent enemies
    return isAdjacent(from, to);
}

bool GameBoard::wouldViolateThreeMoveRule(const Position& from, const Position& to) const {
    auto it = moveHistory.find(from);
    if (it == moveHistory.end()) return false;

    if (it->second.first == to) {
        if (it->second.second >= 2) return true;
    }

    return false;
}

bool GameBoard::canMove(const Position& from, const Position& to) const {
    if (!isValidPosition(from) || !isValidPosition(to)) return false;
    if (getCell(from) != currentPlayer) return false;
    if (getCell(to) != NONE) return false;
    if (!isAdjacent(from, to)) return false;
    if (wouldViolateThreeMoveRule(from, to)) return false;

    return true;
}

void GameBoard::checkAndRemoveShot(const Position& movedTo) {
    Player shooter = (Player)getCell(movedTo);
    if (shooter == NONE) return;

    // Rule: Can't shoot while standing on opponent's start line
    // Player 1 targets row 4 (opponent's start), Player 2 targets row 0
    int opponentStartRow = (shooter == PLAYER1) ? 4 : 0;
    if (movedTo.row == opponentStartRow) {
        return; // Can't shoot from opponent's start line
    }

    // Check all adjacent positions for enemy units
    const auto& neighbors = adjacency[movedTo.row][movedTo.col];

    for (const auto& targetPos : neighbors) {
        int targetPiece = getCell(targetPos);

        // Is there an enemy on this adjacent position?
        if (targetPiece == NONE || targetPiece == shooter) {
            continue; // No enemy here
        }

        // Enemy found on "line of fire" (adjacent position = direct line in graph)
        // Remove the enemy
        setCell(targetPos, NONE);
        killedUnits[targetPiece]++;

        // Record this position as having made a kill (needed for revival rule)
        killerPositions[shooter].push_back(movedTo);

        // Only one kill per move
        return;
    }
}

std::vector<Move> GameBoard::getLegalMoves() const {
    std::vector<Move> moves;

    for (int r = 0; r < 5; r++) {
        for (int c = 0; c < 5; c++) {
            Position from(r, c);
            if (getCell(from) != currentPlayer) continue;

            const auto& neighbors = adjacency[r][c];
            for (const auto& to : neighbors) {
                if (canMove(from, to)) {
                    moves.push_back(Move(from, to));
                }
            }
        }
    }

    // Revival moves
    if (killedUnits.find(currentPlayer) != killedUnits.end() &&
        killedUnits.at(currentPlayer) > 0) {

        auto revivalPos = getRevivalPositions(currentPlayer);
        for (const auto& pos : revivalPos) {
            Move reviveMove;
            reviveMove.from = pos;
            reviveMove.to = pos;
            reviveMove.isRevival = true;

            int startRow = (currentPlayer == PLAYER1) ? 0 : 4;
            for (int c = 0; c < 5; c++) {
                Position revPos(startRow, c);
                if (getCell(revPos) == NONE) {
                    reviveMove.revivePos = revPos;
                    moves.push_back(reviveMove);
                    break;
                }
            }
        }
    }

    return moves;
}

void GameBoard::makeMove(const Move& move) {
    if (move.isRevival) {
        setCell(move.revivePos, currentPlayer);
        killedUnits[currentPlayer]--;

        auto& killers = killerPositions[currentPlayer];
        auto it = std::find(killers.begin(), killers.end(), move.from);
        if (it != killers.end()) {
            killers.erase(it);
        }
    }
    else {
        int piece = getCell(move.from);
        setCell(move.from, NONE);
        setCell(move.to, piece);

        // Update move history for 3-move rule
        auto it = moveHistory.find(move.to);
        if (it != moveHistory.end() && it->second.first == move.from) {
            moveHistory[move.to].second++;
        }
        else {
            moveHistory[move.to] = { move.from, 1 };
        }

        moveHistory.erase(move.from);

        // Check if this move results in shooting an enemy
        checkAndRemoveShot(move.to);
    }
}

bool GameBoard::isGameOver() const {
    int p1OnP2Start = 0, p2OnP1Start = 0;

    for (int c = 0; c < 5; c++) {
        if (board[0][c] == PLAYER2) p2OnP1Start++;
        if (board[4][c] == PLAYER1) p1OnP2Start++;
    }

    return p1OnP2Start == 5 || p2OnP1Start == 5;
}

Player GameBoard::getWinner() const {
    int p1OnP2Start = 0, p2OnP1Start = 0;

    for (int c = 0; c < 5; c++) {
        if (board[4][c] == PLAYER1) p1OnP2Start++;
        if (board[0][c] == PLAYER2) p2OnP1Start++;
    }

    if (p1OnP2Start == 5) return PLAYER1;
    if (p2OnP1Start == 5) return PLAYER2;
    return NONE;
}

int GameBoard::getKilledUnits(Player player) const {
    auto it = killedUnits.find(player);
    return (it != killedUnits.end()) ? it->second : 0;
}

bool GameBoard::canRevive(Player player, const Position& pos) const {
    if (getCell(pos) != player) return false;

    auto it = killerPositions.find(player);
    if (it == killerPositions.end()) return false;

    return std::find(it->second.begin(), it->second.end(), pos) != it->second.end();
}

std::vector<Position> GameBoard::getRevivalPositions(Player player) const {
    std::vector<Position> positions;

    int targetRow = (player == PLAYER1) ? 4 : 0;

    for (int r = 0; r < 5; r++) {
        for (int c = 0; c < 5; c++) {
            Position pos(r, c);
            if (getCell(pos) == player && pos.row == targetRow) {
                if (canRevive(player, pos)) {
                    positions.push_back(pos);
                }
            }
        }
    }

    return positions;
}