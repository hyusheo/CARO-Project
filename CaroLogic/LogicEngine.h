#ifndef LOGICENGINE_H
#define LOGICENGINE_H

// Minimal public interface required by main.cpp.
// Implementations should exist in LogicEngine.cpp.

#ifdef __cplusplus
extern "C" {
#endif

// Returns true if the AI is currently thinking (prevents requesting a result)
bool IsAIThinking();

// Make a move for the given player at (x,y). Player convention used in project (e.g., 1 or 2).
void MakeMove(int x, int y, int player);

// Check win condition after a move at (x,y) by player.
// Return value convention used by the project (e.g., 0 = ongoing, 1 = player1 wins, 2 = player2 wins, etc.).
int CheckWinCondition(int x, int y, int player);

// Retrieve the winning line coordinates; used when a win is detected.
// Each pointer may be nullptr if the caller does not need that coordinate.
void GetWinLine(int* x1, int* y1, int* x2, int* y2);

#ifdef __cplusplus
}
#endif

#endif // LOGICENGINE_H