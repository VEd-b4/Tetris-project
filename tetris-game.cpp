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

