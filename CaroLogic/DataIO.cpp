#include "DataIO.h"
#include <fstream>

// Gọi các biến toàn cục đang "sống" bên trong file CaroAPI.cpp
extern int g_board[30][30];
extern int g_boardSize;
extern bool g_ruleBlock2;
extern int g_aiLevel;

bool SaveBinary(const char* filepath, float timeLeft, int isPlayerTurn) {
    // Mở file ở chế độ ghi nhị phân (out | binary)
    std::ofstream file(filepath, std::ios::out | std::ios::binary);
    if (!file.is_open())
    {
        return false;
    }

    // 1. Ghi các biến cấu hình (Metadata)
    file.write(reinterpret_cast<const char*>(&g_boardSize), sizeof(g_boardSize));
    file.write(reinterpret_cast<const char*>(&g_ruleBlock2), sizeof(g_ruleBlock2));
    file.write(reinterpret_cast<const char*>(&g_aiLevel), sizeof(g_aiLevel));

    // 2. Ghi trạng thái lượt đi và thời gian còn lại (nhận từ giao diện EXE truyền vào)
    file.write(reinterpret_cast<const char*>(&isPlayerTurn), sizeof(isPlayerTurn));
    file.write(reinterpret_cast<const char*>(&timeLeft), sizeof(timeLeft));

    // 3. Ghi mảng 2 chiều toàn cục (Sức mạnh của C-Style)
    // Thay vì lặp 900 lần (30x30), ta ghi nguyên khối 3600 bytes (900 * 4 bytes/int) trong 1 tíc tắc
    file.write(reinterpret_cast<const char*>(g_board), sizeof(g_board));

    file.close();
    return true;
}

bool LoadBinary(const char* filepath, float* outTimeLeft, int* outIsPlayerTurn) {
    // Mở file ở chế độ đọc nhị phân (in | binary)
    std::ifstream file(filepath, std::ios::in | std::ios::binary);
    if (!file.is_open()) 
    {
        return false;
    }

    // 1. Đọc các biến cấu hình (BẮT BUỘC phải đọc theo đúng thứ tự lúc ghi)
    file.read(reinterpret_cast<char*>(&g_boardSize), sizeof(g_boardSize));
    file.read(reinterpret_cast<char*>(&g_ruleBlock2), sizeof(g_ruleBlock2));
    file.read(reinterpret_cast<char*>(&g_aiLevel), sizeof(g_aiLevel));

    // 2. Đọc trạng thái (Đẩy ngược giá trị ra ngoài cho EXE qua con trỏ)
    file.read(reinterpret_cast<char*>(outIsPlayerTurn), sizeof(*outIsPlayerTurn));
    file.read(reinterpret_cast<char*>(outTimeLeft), sizeof(*outTimeLeft));

    // 3. Đọc mảng 2 chiều khôi phục lại bàn cờ
    file.read(reinterpret_cast<char*>(g_board), sizeof(g_board));

    file.close();
    return true;
}