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

