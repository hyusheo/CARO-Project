#ifndef LOGICENGINE_H
#define LOGICENGINE_H

// Minimal public interface required by main.cpp.
// Implementations should exist in LogicEngine.cpp.

#ifdef __cplusplus
extern "C" {
#endif



void MakeMove(int x, int y, int player);


int CheckWinCondition(int x, int y, int player);


#ifdef __cplusplus
}
#endif

#endif // LOGICENGINE_H