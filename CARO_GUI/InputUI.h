#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "Constants.h"

// ============================================================
//  InputUI.h – Khai báo các hàm x? lý input theo t?ng màn hình
//
//  Nguyên t?c: main.cpp ch? g?i hàm, không ch?a logic click.
//  Toàn b? logic ki?m tra t?a ?? chu?t n?m trong InputUI.cpp.
// ============================================================

// X? lý click chu?t ? màn hình Menu
//   gameMode: ???c SET bên trong hàm khi ng??i ch?i ch?n PVP ho?c PVE
void HandleMenuInput(
    sf::RenderWindow& window,
    int mouseX, int mouseY,
    AppState& currentState,
    GameMode& gameMode,          // <-- THÊM: ?? bi?t ch? ?? khi vào game
    int boardSize, bool ruleBlock2, int aiLevel,
    float& timeRemaining, bool& isPlayerTurn, int& gameStatus,
    sf::Sound& errSound
);

// X? lý click chu?t khi ?ang trong tr?n (bàn c? + nút Undo/Save/Menu)
//   undoLeft[0] = s? undo còn l?i c?a P1, undoLeft[1] = c?a P2
//   lastUndoPlayer: 0/1 = ng??i v?a dùng undo, -1 = ch?a ai dùng
void HandleInGameInput(
    int mouseX, int mouseY,
    AppState& currentState,
    int boardSize,
    GameMode gameMode,
    bool& isPlayerTurn, int& gameStatus, float& timeRemaining,
    int undoLeft[2], int& lastUndoPlayer,
    sf::Sound& errSound
);

// X? lý click chu?t ? màn hình Cài ??t (không ??i)
void HandleSettingsInput(
    int mouseX, int mouseY,
    AppState& currentState,
    int& boardSize, bool& ruleBlock2, int& aiLevel,
    float& sfxVolume, bool& bgmEnabled,
    sf::Sound& errSound
);