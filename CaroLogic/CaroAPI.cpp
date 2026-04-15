#include "CaroAPI.h"
#include "LogicEngine.h"
#include "AIEngine.h"
#include "DataIO.h"
#include <atomic>
#include <future>

// ============================================================
//  TRẠNG THÁI BÀN CỜ
// ============================================================
int  g_board[30][30] = { 0 };
int  g_boardSize = 15;
bool g_ruleBlock2 = true;
int  g_aiLevel = 3;

// Tọa độ đường thắng (cho GUI vẽ)
int g_winStartX = -1, g_winStartY = -1;
int g_winEndX = -1, g_winEndY = -1;

// ============================================================
//  LỊCH SỬ NƯỚC ĐI – Stack để hỗ trợ Undo
//  Mỗi MoveRecord ghi lại (x, y, player) của 1 nước đã đặt.
//  Kích thước tối đa: 30×30 = 900 nước.
// ============================================================
struct MoveRecord {
    int x, y, player; // player: 1 = người (X), 2 = AI (O)
};

static MoveRecord g_history[900];
static int        g_historyCount = 0; // số nước đang có trong stack

// Helper nội bộ: đẩy 1 nước vào stack
static void PushHistory(int x, int y, int player)
{
    if (g_historyCount < 900)
    {
        g_history[g_historyCount].x = x;
        g_history[g_historyCount].y = y;
        g_history[g_historyCount].player = player;
        ++g_historyCount;
    }
}

// ============================================================
//  LUỒNG AI
// ============================================================
std::atomic<bool> g_isAiThinking = false;
std::future<void> g_aiTask;
int g_aiMoveX = -1;
int g_aiMoveY = -1;
int g_aiResultState = 0;

// ============================================================
//  InitGame – Khởi tạo / reset toàn bộ trạng thái
// ============================================================
extern "C" CARO_API void InitGame(int size, bool ruleBlock2, int level)
{
    g_boardSize = size;
    g_ruleBlock2 = ruleBlock2;
    g_aiLevel = level;

    for (int i = 0; i < 30; ++i)
        for (int j = 0; j < 30; ++j)
            g_board[i][j] = 0;

    // Reset lịch sử và đường thắng
    g_historyCount = 0;
    g_winStartX = g_winStartY = g_winEndX = g_winEndY = -1;
}

// ============================================================
//  GetCell
// ============================================================
extern "C" CARO_API int GetCell(int x, int y)
{
    if (x < 0 || x >= g_boardSize || y < 0 || y >= g_boardSize) return -1;
    return g_board[x][y];
}

// ============================================================
//  GetWinLine
// ============================================================
extern "C" CARO_API void GetWinLine(int* sx, int* sy, int* ex, int* ey)
{
    *sx = g_winStartX; *sy = g_winStartY;
    *ex = g_winEndX;   *ey = g_winEndY;
}

// ============================================================
//  ProcessPlayerMove – giữ lại để tương thích cũ (PvE)
// ============================================================
extern "C" CARO_API int ProcessPlayerMove(int x, int y)
{
    return ProcessMove(x, y, 1);
}

// ============================================================
//  ProcessMove – Tổng quát, dùng cho cả PVP và PVE
//  player: 1 = X (người 1), 2 = O (người 2 hoặc AI)
// ============================================================
extern "C" CARO_API int ProcessMove(int x, int y, int player)
{
    if (x < 0 || x >= g_boardSize ||
        y < 0 || y >= g_boardSize ||
        g_board[x][y] != 0 ||
        (player != 1 && player != 2))
        return -1;

    g_board[x][y] = player;
    PushHistory(x, y, player);

    return CheckWinCondition(x, y, player);
}

// ============================================================
//  UndoMove – Lùi lại theo CẶP nước (AI + người)
//
//  Logic:
//   - Nếu stack có >= 2 nước: xóa 2 nước trên cùng (AI rồi người)
//   - Nếu stack chỉ có 1 nước : xóa 1 nước đó
//   - Nếu stack rỗng           : không làm gì, trả về 0
//
//  Sau khi undo:
//   - Ô bàn cờ tương ứng bị xóa về 0
//   - Nếu có người thắng thì không thể undo được nữa, ván cờ kết thúc
//   - Trả về số nước đã undo thực tế (0 / 1 / 2)
// ============================================================
extern "C" CARO_API int UndoMove()
{
    //Nếu đã có đường thẳng thì không được undo nữa
    if (g_winStartX != -1) {
        return 0;
    }
    if (g_historyCount == g_boardSize * g_boardSize) {
        return 0;
    }
    if (g_historyCount == 0) return 0; // không có gì để undo


    int undone = 0;

    // Undo nước 1 (nước trên cùng stack — thường là nước AI)
    {
        --g_historyCount;
        int x = g_history[g_historyCount].x;
        int y = g_history[g_historyCount].y;
        g_board[x][y] = 0;
        ++undone;
    }

    // Undo thêm nước 2 nếu còn (nước người chơi đứng ngay trước AI)
    if (g_historyCount > 0)
    {
        --g_historyCount;
        int x = g_history[g_historyCount].x;
        int y = g_history[g_historyCount].y;
        g_board[x][y] = 0;
        ++undone;
    }

    return undone; // 1 hoặc 2
}

// ============================================================
//  UndoOneMove – Xóa đúng 1 nước trên cùng stack
//  Dùng cho PVP: mỗi người chỉ lùi lại nước của chính mình
// ============================================================
extern "C" CARO_API int UndoOneMove()
{
    if (g_winStartX != -1) {
        return 0;
    }
    if (g_historyCount == g_boardSize * g_boardSize) {
        return 0;
    }
    if (g_historyCount == 0) return 0;

    --g_historyCount;
    int x = g_history[g_historyCount].x;
    int y = g_history[g_historyCount].y;
    g_board[x][y] = 0;

    return 1;
}
extern "C" CARO_API void StartAIThinking()
{
    g_isAiThinking = true;

    // Snapshot bàn cờ để tránh data race
    static int boardCopy[30][30];
    for (int i = 0; i < g_boardSize; ++i)
        for (int j = 0; j < g_boardSize; ++j)
            boardCopy[i][j] = g_board[i][j];

    g_aiTask = std::async(std::launch::async, []()
        {
            CalculateBestMove(boardCopy, g_boardSize, g_aiLevel,
                &g_aiMoveX, &g_aiMoveY);

            // Đặt quân AI lên bàn cờ thật và ghi vào lịch sử
            g_board[g_aiMoveX][g_aiMoveY] = 2;
            PushHistory(g_aiMoveX, g_aiMoveY, 2); // <-- ghi lịch sử AI

            g_aiResultState = CheckWinCondition(g_aiMoveX, g_aiMoveY, 2);
            g_isAiThinking = false;
        });
}

// ============================================================
//  IsAIThinking / GetAIResult
// ============================================================
extern "C" CARO_API bool IsAIThinking()
{
    return g_isAiThinking;
}

extern "C" CARO_API int GetAIResult(int* outX, int* outY)
{
    *outX = g_aiMoveX;
    *outY = g_aiMoveY;
    return g_aiResultState;
}

// ============================================================
//  Save / Load
// ============================================================
extern "C" CARO_API bool SaveGameBinary(const char* filepath,
    float timeLeft, int isPlayerTurn)
{
    return SaveBinary(filepath, timeLeft, isPlayerTurn);
}

extern "C" CARO_API bool LoadGameBinary(const char* filepath,
    float* timeLeft, int* isPlayerTurn)
{
    return LoadBinary(filepath, timeLeft, isPlayerTurn);
}