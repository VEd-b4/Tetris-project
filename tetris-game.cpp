#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <windows.h>
#include <conio.h>

enum class TetrominoType { I, O, T, S, Z, J, L };

class Tetromino {
public:
    TetrominoType type;
    std::vector<std::vector<int>> shape;
    int color;

    Tetromino() : type(TetrominoType::I), color(0) {}
    Tetromino(TetrominoType t) : type(t) { initializeShape(); }

private:
    void initializeShape() {
        switch(type) {
            case TetrominoType::I: shape = {{1,1,1,1}}; color = 1; break;
            case TetrominoType::O: shape = {{1,1},{1,1}}; color = 2; break;
            case TetrominoType::T: shape = {{0,1,0},{1,1,1}}; color = 3; break;
            case TetrominoType::S: shape = {{0,1,1},{1,1,0}}; color = 4; break;
            case TetrominoType::Z: shape = {{1,1,0},{0,1,1}}; color = 5; break;
            case TetrominoType::J: shape = {{1,0,0},{1,1,1}}; color = 6; break;
            case TetrominoType::L: shape = {{0,0,1},{1,1,1}}; color = 7; break;
        }
    }
};

class TetrisGame {
private:
    static const int WIDTH = 10;
    static const int HEIGHT = 20;
    std::vector<std::vector<int>> grid;
    Tetromino currentPiece;
    int currentX, currentY, score, level, dropTimer;
    bool isPaused, isGameFinished, shouldRestart;
    int pauseMenuSelection;
    HANDLE hConsole;
    COORD consoleBufferSize;
    SMALL_RECT windowSize;
    CHAR_INFO* consoleBuffer;

    TetrominoType getRandomTetromino() { return static_cast<TetrominoType>(rand() % 7); }

    bool canMove(const Tetromino& piece, int newX, int newY) {
        for (size_t y = 0; y < piece.shape.size(); ++y) {
            for (size_t x = 0; x < piece.shape[y].size(); ++x) {
                if (piece.shape[y][x]) {
                    int gridX = newX + x;
                    int gridY = newY + y;

                    // Check if the piece is outside the grid horizontally
                    if (gridX < 0 || gridX >= WIDTH) return false;

                    // Check if the piece is outside the grid vertically
                    if (gridY >= HEIGHT) return false;

                    // Check for collision with existing locked pieces
                    if (gridY >= 0 && grid[gridY][gridX] != 0) return false;
                }
            }
        }
        return true;
    }


public:
    TetrisGame() : grid(HEIGHT, std::vector<int>(WIDTH, 0)), 
        currentPiece(getRandomTetromino()), currentX(WIDTH / 2), currentY(0),
        score(0), level(1), dropTimer(0), isPaused(false), 
        isGameFinished(false), shouldRestart(false), pauseMenuSelection(0) {
        
        srand(static_cast<unsigned>(time(nullptr)));
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        consoleBufferSize = { (SHORT)((WIDTH * 2) + 20), (SHORT)(HEIGHT + 10) };
        windowSize = { 0, 0, consoleBufferSize.X - 1, consoleBufferSize.Y - 1 };
        SetConsoleScreenBufferSize(hConsole, consoleBufferSize);
        SetConsoleWindowInfo(hConsole, TRUE, &windowSize);
        consoleBuffer = new CHAR_INFO[consoleBufferSize.X * consoleBufferSize.Y];

        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hConsole, &cursorInfo);
        cursorInfo.bVisible = false;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
    }

    ~TetrisGame() { delete[] consoleBuffer; }

    bool run() {
        grid = std::vector<std::vector<int>>(HEIGHT, std::vector<int>(WIDTH, 0));
        currentPiece = Tetromino(getRandomTetromino());
        currentX = WIDTH / 2; currentY = 0;
        score = 0; level = 1; dropTimer = 0;
        isPaused = false; isGameFinished = false;
        shouldRestart = false; pauseMenuSelection = 0;

        while(!isGameOver()) {
            handleInput();
            if (!isPaused) update();
            render();
            if (shouldRestart) return true;
            Sleep(50);
        }
        return gameOver();
    }

    void handleInput() {
        if (_kbhit()) {
            int key = _getch();
            if (isPaused) {
                if (key == 72) pauseMenuSelection = (pauseMenuSelection - 1 + 3) % 3;
                if (key == 80) pauseMenuSelection = (pauseMenuSelection + 1) % 3;
                if (key == 13) handlePauseMenuSelection();
                render();
            } else {
                switch(key) {
                    case 75: case 'a': moveLeft(); break;
                    case 77: case 'd': moveRight(); break;
                    case 72: case 'w': rotate(); break;
                    case 80: case 's': softDrop(); break;
                    case 32: hardDrop(); break;
                    case 27: togglePause(); break;
                }
            }
        }
    }


void moveLeft() { if (canMove(currentPiece, currentX - 1, currentY)) currentX--; }
    void moveRight() { if (canMove(currentPiece, currentX + 1, currentY)) currentX++; }

    void rotate() {
        Tetromino rotatedPiece = currentPiece;
        std::vector<std::vector<int>> newShape(currentPiece.shape[0].size(), std::vector<int>(currentPiece.shape.size()));
        for (size_t i = 0; i < currentPiece.shape.size(); ++i)
            for (size_t j = 0; j < currentPiece.shape[i].size(); ++j)
                newShape[j][currentPiece.shape.size() - 1 - i] = currentPiece.shape[i][j];

        rotatedPiece.shape = newShape;
        if (canMove(rotatedPiece, currentX, currentY)) currentPiece = rotatedPiece;
    }

    void update() {
        dropTimer++;
        if (dropTimer >= (20 - level)) {
            if (!canMove(currentPiece, currentX, currentY + 1)) {
                lockPiece();
                clearLines();
                spawnNewPiece();
            } else currentY++;
            dropTimer = 0;
        }
    }

    void softDrop() {
        if (canMove(currentPiece, currentX, currentY + 1)) currentY++;
        else {
            lockPiece();
            clearLines();
            spawnNewPiece();
        }
    }

    void hardDrop() {
        while (canMove(currentPiece, currentX, currentY + 1)) currentY++;
        lockPiece();
        clearLines();
        spawnNewPiece();
    }

    void lockPiece() {
        for (size_t y = 0; y < currentPiece.shape.size(); ++y)
            for (size_t x = 0; x < currentPiece.shape[y].size(); ++x)
                if (currentPiece.shape[y][x]) grid[currentY + y][currentX + x] = currentPiece.color;
    }

    void spawnNewPiece() {
        currentPiece = Tetromino(getRandomTetromino());
        currentX = WIDTH / 2;
        currentY = 0;
        if (!canMove(currentPiece, currentX, currentY)) gameOver();
    }


    void clearLines() {
        int linesCleared = 0;
        for (int y = HEIGHT - 1; y >= 0; --y) {
            bool fullLine = true;
            for (int x = 0; x < WIDTH; ++x)
                if (!grid[y][x]) { fullLine = false; break; }

            if (fullLine) {
                grid.erase(grid.begin() + y);
                grid.insert(grid.begin(), std::vector<int>(WIDTH, 0));
                linesCleared++;
                y++;
            }
        }
        updateScore(linesCleared);
    }

    void updateScore(int linesCleared) {
        score += (linesCleared == 1 ? 100 : linesCleared == 2 ? 300 : linesCleared == 3 ? 500 : linesCleared == 4 ? 800 : 0);
        level = 1 + (score / 1000);
    }

    bool gameOver() {
        isGameFinished = true;
        std::cout << "GAME OVER! Final Score: " << score << "\nPress R to Replay or ESC to Exit\n";
        while (true) {
            if (_kbhit()) {
                int key = _getch();
                if (key == 'r' || key == 'R') return true;
                if (key == 27) return false;
            }
        }
    }
};

int main() {
    TetrisGame game;
    while (game.run());
    return 0;
}


