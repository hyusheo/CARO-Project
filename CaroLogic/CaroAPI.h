#pragma once

#ifdef CAROLOGIC_EXPORTS
#define CARO_API __declspec(dllexport)
#else
#define CARO_API __declspec(dllimport)
#endif

extern "C" {
    CARO_API void InitGame(int size, bool ruleBlock2, int level);
    CARO_API int  GetCell(int x, int y);
    CARO_API void GetWinLine(int* startX, int* startY, int* endX, int* endY);
    CARO_API int  ProcessPlayerMove(int x, int y);
    CARO_API int  ProcessMove(int x, int y, int player);
    CARO_API int  UndoMove();
    CARO_API int  UndoOneMove();
    CARO_API void StartAIThinking();
    CARO_API bool IsAIThinking();
    CARO_API int  GetAIResult(int* outX, int* outY);
    extern "C" CARO_API void MakeMove(int x, int y, int player);
    extern "C" CARO_API int CheckWinCondition(int x, int y, int player);

    // Multi-slot IO
    CARO_API bool SaveGameSlot(int slotId, float timeLeft, int isPlayerTurn, const char* gameName);
    CARO_API bool LoadGameSlot(int slotId, float* timeLeft, int* isPlayerTurn);
    CARO_API bool PeekGameSlot(int slotId, int* outBoardSize, int* outMoves, int* outTurn, char* outName);
    CARO_API bool DeleteGameSlot(int slotId);
    CARO_API bool GetSlotPreview(int slotId, int* outBoardSize, int* outMoves, int* outTurn, char* outDate, char* outName, int outBoard[30][30]);
    CARO_API int EvaluateBoard();
}
