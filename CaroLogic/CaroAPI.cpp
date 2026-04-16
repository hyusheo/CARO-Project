#include "CaroAPI.h"
#include "LogicEngine.h"
#include "AIEngine.h"
#include "DataIO.h"
#include <atomic>
#include <future>
#include <cstring>

int  g_board[30][30] = { 0 };
int  g_boardSize = 15;
bool g_ruleBlock2 = true;
int  g_aiLevel = 3;
int g_winStartX = -1, g_winStartY = -1, g_winEndX = -1, g_winEndY = -1;

struct MoveRecord { int x, y, player; };
MoveRecord g_history[900];
int        g_historyCount = 0;

static void PushHistory(int x, int y, int player) {
    if (g_historyCount < 900) {
        g_history[g_historyCount].x = x; g_history[g_historyCount].y = y;
        g_history[g_historyCount].player = player; ++g_historyCount;
    }
}

std::atomic<bool> g_isAiThinking = false;
std::future<void> g_aiTask;
int g_aiMoveX = -1, g_aiMoveY = -1, g_aiResultState = 0;

extern "C" CARO_API void InitGame(int size, bool ruleBlock2, int level) {
    g_boardSize = size; g_ruleBlock2 = ruleBlock2; g_aiLevel = level;
    for (int i = 0; i < 30; ++i) for (int j = 0; j < 30; ++j) g_board[i][j] = 0;
    g_historyCount = 0; g_winStartX = g_winStartY = g_winEndX = g_winEndY = -1;
}

extern "C" CARO_API int GetCell(int x, int y) {
    if (x < 0 || x >= g_boardSize || y < 0 || y >= g_boardSize) return -1;
    return g_board[x][y];
}

extern "C" CARO_API void GetWinLine(int* sx, int* sy, int* ex, int* ey) {
    *sx = g_winStartX; *sy = g_winStartY; *ex = g_winEndX; *ey = g_winEndY;
}

extern "C" CARO_API int ProcessPlayerMove(int x, int y) { return ProcessMove(x, y, 1); }

extern "C" CARO_API int ProcessMove(int x, int y, int player) {
    if (x < 0 || x >= g_boardSize || y < 0 || y >= g_boardSize || g_board[x][y] != 0 || (player != 1 && player != 2)) return -1;
    g_board[x][y] = player; PushHistory(x, y, player);
    return CheckWinCondition(x, y, player);
}

extern "C" CARO_API int UndoMove() {
    if (g_historyCount == 0) return 0;
    g_winStartX = g_winStartY = g_winEndX = g_winEndY = -1;
    int undone = 0;
    { --g_historyCount; int x = g_history[g_historyCount].x; int y = g_history[g_historyCount].y; g_board[x][y] = 0; ++undone; }
    if (g_historyCount > 0) { --g_historyCount; int x = g_history[g_historyCount].x; int y = g_history[g_historyCount].y; g_board[x][y] = 0; ++undone; }
    return undone;
}

extern "C" CARO_API int UndoOneMove() {
    if (g_historyCount == 0) return 0;
    g_winStartX = g_winStartY = g_winEndX = g_winEndY = -1;
    --g_historyCount; int x = g_history[g_historyCount].x; int y = g_history[g_historyCount].y; g_board[x][y] = 0;
    return 1;
}

extern "C" CARO_API void StartAIThinking() {
    g_isAiThinking = true;
    static int boardCopy[30][30];
    for (int i = 0; i < g_boardSize; ++i)
        for (int j = 0; j < g_boardSize; ++j)
            boardCopy[i][j] = g_board[i][j];

    g_aiTask = std::async(std::launch::async, []() {
        // Chỉ tính toán tọa độ tốt nhất, KHÔNG thay đổi g_board hay PushHistory ở đây
        CalculateBestMove(boardCopy, g_boardSize, g_aiLevel, &g_aiMoveX, &g_aiMoveY);

        // Tính xong thì báo hiệu
        g_isAiThinking = false;
        });
}

extern "C" CARO_API bool IsAIThinking() { return g_isAiThinking; }
extern "C" CARO_API int GetAIResult(int* outX, int* outY) { *outX = g_aiMoveX; *outY = g_aiMoveY; return g_aiResultState; }

extern "C" CARO_API bool SaveGameSlot(int slotId, float timeLeft, int isPlayerTurn, const char* gameName) {
    return SaveSlotBinary(slotId, timeLeft, isPlayerTurn, gameName);
}

extern "C" CARO_API bool LoadGameSlot(int slotId, float* timeLeft, int* isPlayerTurn) {
    return LoadSlotBinary(slotId, timeLeft, isPlayerTurn);
}

extern "C" CARO_API bool PeekGameSlot(int slotId, int* outBoardSize, int* outMoves, int* outTurn, char* outName) {
    SaveMetadata meta;
    if (PeekSlotMetadata(slotId, &meta)) {
        *outBoardSize = meta.boardSize; *outMoves = meta.historyCount; *outTurn = meta.isPlayerTurn;
        strcpy_s(outName, 64, meta.gameName);
        return true;
    }
    return false;
}

extern "C" CARO_API bool DeleteGameSlot(int slotId) { return DeleteSlotBinary(slotId); }

extern "C" CARO_API bool GetSlotPreview(int slotId, int* outBoardSize, int* outMoves, int* outTurn, char* outDate, char* outName, int outBoard[30][30]) {
    SaveMetadata meta;
    if (PeekSlotPreview(slotId, &meta, outBoard)) {
        *outBoardSize = meta.boardSize; *outMoves = meta.historyCount; *outTurn = meta.isPlayerTurn;
        strcpy_s(outDate, 32, meta.saveDate); strcpy_s(outName, 64, meta.gameName);
        return true;
    }
    return false;
}
extern "C" CARO_API int EvaluateBoard() {
    // Nếu chưa có nước cờ nào được đánh thì chắc chắn là đang chơi (0)
    if (g_historyCount == 0) return 0;
    // Lấy tọa độ của nước đi cuối cùng ra
    int lastX = g_history[g_historyCount - 1].x;
    int lastY = g_history[g_historyCount - 1].y;
    int lastPlayer = g_history[g_historyCount - 1].player;
    // Check lại xem nước đi cuối đó có tạo thành 5 ố thẳng hàng không
    return CheckWinCondition(lastX, lastY, lastPlayer);
}


extern "C" CARO_API void MakeMove(int x, int y, int player) {
    // Cập nhật mảng cờ và lịch sử
    g_board[x][y] = player;
    PushHistory(x, y, player);
}

int CheckWinCondition(int x, int y, int player) {
    const int dx[] = { 1, 0, 1, 1 };
    const int dy[] = { 0, 1, 1, -1 };

    for (int dir = 0; dir < 4; ++dir) {
        int count = 1, blocks = 0;
        int sx = x, sy = y, ex = x, ey = y;

        // Duyệt tới
        int i = 1;
        while (x + i * dx[dir] >= 0 && x + i * dx[dir] < g_boardSize && y + i * dy[dir] >= 0 && y + i * dy[dir] < g_boardSize) {
            if (g_board[x + i * dx[dir]][y + i * dy[dir]] == player) {
                count++; ex = x + i * dx[dir]; ey = y + i * dy[dir];
            }
            else {
                if (g_board[x + i * dx[dir]][y + i * dy[dir]] != 0) blocks++;
                break;
            }
            i++;
        }

        // Duyệt lùi
        i = 1;
        while (x - i * dx[dir] >= 0 && x - i * dx[dir] < g_boardSize && y - i * dy[dir] >= 0 && y - i * dy[dir] < g_boardSize) {
            if (g_board[x - i * dx[dir]][y - i * dy[dir]] == player) {
                count++; sx = x - i * dx[dir]; sy = y - i * dy[dir];
            }
            else {
                if (g_board[x - i * dx[dir]][y - i * dy[dir]] != 0) blocks++;
                break;
            }
            i++;
        }

        if (count >= 5) {
            if (g_ruleBlock2 && blocks == 2 && count == 5) continue;
            g_winStartX = sx; g_winStartY = sy;
            g_winEndX = ex; g_winEndY = ey;
            return player; // 1 hoặc 2
        }
    }

    // Check Hòa
    for (int r = 0; r < g_boardSize; ++r)
        for (int c = 0; c < g_boardSize; ++c)
            if (g_board[r][c] == 0) return 0; // Đang chơi

    return 3; // Hòa
}
