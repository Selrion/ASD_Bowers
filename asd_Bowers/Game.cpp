#include "Game.h"
#include <cmath>

#define BOARD_OFFSET_X 250
#define BOARD_OFFSET_Y 100
#define CELL_SIZE 100
#define NODE_RADIUS 25

Game::Game() : window(nullptr), renderer(nullptr), font(nullptr),
smallFont(nullptr), ai(nullptr), running(false),
vsAI(true), pieceSelected(false), messageTimer(0) {
    selectedPos = Position(-1, -1);
}

Game::~Game() {
    cleanup();
}

bool Game::init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return false;
    }

    if (TTF_Init() < 0) {
        return false;
    }

    window = SDL_CreateWindow("Nomad Archers",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

    if (!window) {
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        return false;
    }

    font = TTF_OpenFont("arial.ttf", 24);
    smallFont = TTF_OpenFont("arial.ttf", 16);

    if (!font || !smallFont) {
        return false;
    }

    ai = new AIPlayer(PLAYER2, 3);
    running = true;

    return true;
}

void Game::run() {
    while (running) {
        handleEvents();
        update();
        render();
        SDL_Delay(16);
    }
}

void Game::cleanup() {
    if (ai) {
        delete ai;
        ai = nullptr;
    }

    if (font) {
        TTF_CloseFont(font);
        font = nullptr;
    }

    if (smallFont) {
        TTF_CloseFont(smallFont);
        smallFont = nullptr;
    }

    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }

    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    TTF_Quit();
    SDL_Quit();
}

void Game::handleEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            running = false;
        }
        else if (e.type == SDL_MOUSEBUTTONDOWN) {
            if (e.button.button == SDL_BUTTON_LEFT) {
                handleMouseClick(e.button.x, e.button.y);
            }
        }
        else if (e.type == SDL_KEYDOWN) {
            handleKeyPress(e.key.keysym.sym);
        }
    }
}

void Game::update() {
    if (messageTimer > 0) {
        messageTimer--;
    }

    if (board.isGameOver()) {
        Player winner = board.getWinner();
        if (winner == PLAYER1) {
            showMessage("Player 1 Wins!", 300);
        }
        else if (winner == PLAYER2) {
            showMessage("Player 2 Wins!", 300);
        }
    }
    else if (vsAI && board.getCurrentPlayer() == PLAYER2 && !pieceSelected) {
        SDL_Delay(500);
        aiMove();
    }
}

void Game::render() {
    SDL_SetRenderDrawColor(renderer, 240, 230, 210, 255);
    SDL_RenderClear(renderer);

    drawBoard();
    drawPieces();
    drawUI();

    SDL_RenderPresent(renderer);
}

Position Game::screenToBoard(int x, int y) const {
    int boardX = x - BOARD_OFFSET_X;
    int boardY = y - BOARD_OFFSET_Y;

    for (int r = 0; r < 5; r++) {
        for (int c = 0; c < 5; c++) {
            int px, py;
            getBoardPosition(r, c, px, py);

            int dx = boardX - px;
            int dy = boardY - py;
            float dist = sqrt(dx * dx + dy * dy);

            if (dist < 30) {
                return Position(r, c);
            }
        }
    }

    return Position(-1, -1);
}

void Game::getBoardPosition(int row, int col, int& x, int& y) const {
    x = col * CELL_SIZE + CELL_SIZE / 2;
    y = row * CELL_SIZE + CELL_SIZE / 2;
}

void Game::drawBoard() {
    SDL_SetRenderDrawColor(renderer, 100, 80, 60, 255);

    for (int r = 0; r < 5; r++) {
        for (int c = 0; c < 5; c++) {
            const auto& neighbors = board.adjacency[r][c];

            int x1, y1;
            getBoardPosition(r, c, x1, y1);

            for (const auto& neighbor : neighbors) {
                int x2, y2;
                getBoardPosition(neighbor.row, neighbor.col, x2, y2);

                SDL_RenderDrawLine(renderer,
                    BOARD_OFFSET_X + x1, BOARD_OFFSET_Y + y1,
                    BOARD_OFFSET_X + x2, BOARD_OFFSET_Y + y2);
            }
        }
    }

    for (int r = 0; r < 5; r++) {
        for (int c = 0; c < 5; c++) {
            int x, y;
            getBoardPosition(r, c, x, y);

            SDL_Color color = { 200, 200, 200, 255 };
            if (r == 0) color = { 100, 150, 255, 255 };
            if (r == 4) color = { 255, 100, 100, 255 };

            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

            for (int i = 0; i < 25; i++) {
                int radius = 25 - i;
                for (int angle = 0; angle < 360; angle += 10) {
                    int px1 = x + radius * cos(angle * 3.14159 / 180);
                    int py1 = y + radius * sin(angle * 3.14159 / 180);
                    int px2 = x + radius * cos((angle + 10) * 3.14159 / 180);
                    int py2 = y + radius * sin((angle + 10) * 3.14159 / 180);

                    SDL_RenderDrawLine(renderer,
                        BOARD_OFFSET_X + px1, BOARD_OFFSET_Y + py1,
                        BOARD_OFFSET_X + px2, BOARD_OFFSET_Y + py2);
                }
            }
        }
    }

    for (const auto& pos : highlightedMoves) {
        int x, y;
        getBoardPosition(pos.row, pos.col, x, y);

        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 128);
        for (int r = 20; r < 30; r++) {
            for (int angle = 0; angle < 360; angle += 5) {
                int px = x + r * cos(angle * 3.14159 / 180);
                int py = y + r * sin(angle * 3.14159 / 180);
                SDL_RenderDrawPoint(renderer, BOARD_OFFSET_X + px, BOARD_OFFSET_Y + py);
            }
        }
    }
}

void Game::drawPieces() {
    for (int r = 0; r < 5; r++) {
        for (int c = 0; c < 5; c++) {
            Position pos(r, c);
            int cell = board.getCell(pos);

            if (cell == NONE) continue;

            int x, y;
            getBoardPosition(r, c, x, y);

            SDL_Color color = (cell == PLAYER1) ?
                SDL_Color{ 50, 100, 255, 255 } : SDL_Color{ 255, 50, 50, 255 };

            bool isSelected = (pieceSelected && selectedPos == pos);
            if (isSelected) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
                for (int r = 18; r < 22; r++) {
                    for (int angle = 0; angle < 360; angle += 5) {
                        int px = x + r * cos(angle * 3.14159 / 180);
                        int py = y + r * sin(angle * 3.14159 / 180);
                        SDL_RenderDrawPoint(renderer, BOARD_OFFSET_X + px, BOARD_OFFSET_Y + py);
                    }
                }
            }

            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
            for (int i = 0; i < 18; i++) {
                int radius = 18 - i;
                for (int angle = 0; angle < 360; angle += 10) {
                    int px1 = x + radius * cos(angle * 3.14159 / 180);
                    int py1 = y + radius * sin(angle * 3.14159 / 180);
                    int px2 = x + radius * cos((angle + 10) * 3.14159 / 180);
                    int py2 = y + radius * sin((angle + 10) * 3.14159 / 180);

                    SDL_RenderDrawLine(renderer,
                        BOARD_OFFSET_X + px1, BOARD_OFFSET_Y + py1,
                        BOARD_OFFSET_X + px2, BOARD_OFFSET_Y + py2);
                }
            }

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            for (int angle = 0; angle < 360; angle += 10) {
                int px1 = x + 18 * cos(angle * 3.14159 / 180);
                int py1 = y + 18 * sin(angle * 3.14159 / 180);
                int px2 = x + 18 * cos((angle + 10) * 3.14159 / 180);
                int py2 = y + 18 * sin((angle + 10) * 3.14159 / 180);

                SDL_RenderDrawLine(renderer,
                    BOARD_OFFSET_X + px1, BOARD_OFFSET_Y + py1,
                    BOARD_OFFSET_X + px2, BOARD_OFFSET_Y + py2);
            }
        }
    }
}

void Game::drawUI() {
    SDL_Color white = { 0, 0, 0, 255 };
    SDL_Color p1Color = { 50, 100, 255, 255 };
    SDL_Color p2Color = { 255, 50, 50, 255 };

    drawText("Nomad Archers", 20, 20, white, font);

    std::string currentPlayerText = "Current: Player " +
        std::to_string((int)board.getCurrentPlayer());
    SDL_Color currentColor = (board.getCurrentPlayer() == PLAYER1) ? p1Color : p2Color;
    drawText(currentPlayerText, 20, 60, currentColor, smallFont);

    drawText("Controls:", 20, 120, white, smallFont);
    drawText("Mouse: Select/Move", 20, 150, white, smallFont);
    drawText("R: Reset Game", 20, 180, white, smallFont);
    drawText("A: Toggle AI", 20, 210, white, smallFont);
    drawText("ESC: Quit", 20, 240, white, smallFont);

    std::string aiText = vsAI ? "Mode: vs AI" : "Mode: vs Human";
    drawText(aiText, 20, 280, white, smallFont);

    int p1Killed = board.getKilledUnits(PLAYER1);
    int p2Killed = board.getKilledUnits(PLAYER2);

    drawText("Player 1 Lost: " + std::to_string(p1Killed), 20, 320, p1Color, smallFont);
    drawText("Player 2 Lost: " + std::to_string(p2Killed), 20, 350, p2Color, smallFont);

    if (messageTimer > 0 && !message.empty()) {
        SDL_Color msgColor = { 255, 200, 0, 255 };
        drawTextCentered(message, SCREEN_WIDTH / 2, 30, msgColor, font);
    }

    if (board.isGameOver()) {
        Player winner = board.getWinner();
        std::string winText = "Player " + std::to_string((int)winner) + " Wins!";
        SDL_Color winColor = (winner == PLAYER1) ? p1Color : p2Color;
        drawTextCentered(winText, SCREEN_WIDTH / 2, SCREEN_HEIGHT - 50, winColor, font);
    }
}

void Game::drawText(const std::string& text, int x, int y, SDL_Color color, TTF_Font* useFont) {
    if (!useFont) return;

    SDL_Surface* surface = TTF_RenderText_Blended(useFont, text.c_str(), color);
    if (!surface) return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_FreeSurface(surface);
        return;
    }

    SDL_Rect rect = { x, y, surface->w, surface->h };
    SDL_RenderCopy(renderer, texture, NULL, &rect);

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

void Game::drawTextCentered(const std::string& text, int x, int y, SDL_Color color, TTF_Font* useFont) {
    if (!useFont) return;

    SDL_Surface* surface = TTF_RenderText_Blended(useFont, text.c_str(), color);
    if (!surface) return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_FreeSurface(surface);
        return;
    }

    SDL_Rect rect = { x - surface->w / 2, y - surface->h / 2, surface->w, surface->h };
    SDL_RenderCopy(renderer, texture, NULL, &rect);

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

void Game::handleMouseClick(int x, int y) {
    if (board.isGameOver()) return;
    if (vsAI && board.getCurrentPlayer() == PLAYER2) return;

    Position clickedPos = screenToBoard(x, y);

    if (clickedPos.row == -1) {
        pieceSelected = false;
        highlightedMoves.clear();
        return;
    }

    if (pieceSelected) {
        movePiece(clickedPos);
    }
    else {
        selectPiece(clickedPos);
    }
}

void Game::handleKeyPress(SDL_Keycode key) {
    switch (key) {
    case SDLK_ESCAPE:
        running = false;
        break;

    case SDLK_r:
        board.reset();
        pieceSelected = false;
        highlightedMoves.clear();
        showMessage("Game Reset", 60);
        break;

    case SDLK_a:
        vsAI = !vsAI;
        showMessage(vsAI ? "AI Enabled" : "AI Disabled", 60);
        break;
    }
}

void Game::selectPiece(const Position& pos) {
    if (board.getCell(pos) != board.getCurrentPlayer()) {
        pieceSelected = false;
        highlightedMoves.clear();
        return;
    }

    selectedPos = pos;
    pieceSelected = true;
    highlightedMoves.clear();

    std::vector<Move> allMoves = board.getLegalMoves();
    for (const auto& move : allMoves) {
        if (move.from == pos && !move.isRevival) {
            highlightedMoves.push_back(move.to);
        }
    }
}

void Game::movePiece(const Position& to) {
    std::vector<Move> allMoves = board.getLegalMoves();

    for (const auto& move : allMoves) {
        if (move.from == selectedPos && move.to == to && !move.isRevival) {
            board.makeMove(move);
            board.switchPlayer();

            pieceSelected = false;
            highlightedMoves.clear();
            return;
        }
    }

    if (board.getCell(to) == board.getCurrentPlayer()) {
        selectPiece(to);
    }
    else {
        pieceSelected = false;
        highlightedMoves.clear();
    }
}

void Game::aiMove() {
    if (board.isGameOver()) return;

    Move bestMove = ai->getBestMove(board);
    board.makeMove(bestMove);
    board.switchPlayer();

    showMessage("AI moved", 60);
}

void Game::showMessage(const std::string& msg, int duration) {
    message = msg;
    messageTimer = duration;
}