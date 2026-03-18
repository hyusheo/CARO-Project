#include "AIEngine.h"
#include <thread>
#include <chrono>

void CalculateBestMove(int boardCopy[30][30], int boardSize, int level, int* outX, int* outY) {
    // 1. Giả lập AI đang "vắt óc suy nghĩ" trong 1 giây
    // Lúc này giao diện đồ họa bên EXE vẫn phải rê chuột mượt mà, đồng hồ vẫn chạy
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // 2. Thuật toán AI "Ngốc nghếch": Tìm thấy ô trống nào đầu tiên là quất luôn
    for (int x = 0; x < boardSize; ++x) {
        for (int y = 0; y < boardSize; ++y) {
            if (boardCopy[x][y] == 0) {
                *outX = x;
                *outY = y;
                return; // Tìm thấy thì dừng ngay và trả về tọa độ qua con trỏ
            }
        }
    }
}