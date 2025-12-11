#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
#include "GameBoard.h"
#include "AIPlayer.h"

// Screen constants
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 700
#define BOARD_OFFSET_X 250
#define BOARD_OFFSET_Y 100
#define CELL_SIZE 100
#define NODE_RADIUS 25

class Game {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;
    TTF_Font* smallFont;

    GameBoard board;
    AIPlayer* ai;

    bool running;
    bool vsAI;
    Position selectedPos;
    bool pieceSelected;
    std::vector<Position> highlightedMoves;

    std::string message;
    int messageTimer;

    void handleEvents();
    void update();
    void render();

    void handleMouseClick(int x, int y);
    void handleKeyPress(SDL_Keycode key);

    Position screenToBoard(int x, int y) const;
    void getBoardPosition(int row, int col, int& x, int& y) const;

    void drawBoard();
    void drawPieces();
    void drawUI();
    void drawText(const std::string& text, int x, int y, SDL_Color color, TTF_Font* useFont);
    void drawTextCentered(const std::string& text, int x, int y, SDL_Color color, TTF_Font* useFont);

    void selectPiece(const Position& pos);
    void movePiece(const Position& to);
    void aiMove();

    void showMessage(const std::string& msg, int duration = 120);

public:
    Game();
    ~Game();

    bool init();
    void run();
    void cleanup();
};
