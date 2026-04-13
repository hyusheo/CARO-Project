#pragma once
#include <SFML/Graphics.hpp>
#include "Constants.h"   // cần cho GameMode, Config, AppState

void DrawMenu(sf::RenderWindow& window, const sf::Font& font);

void DrawBoard(sf::RenderWindow& window, int boardSize);
void DrawPieces(sf::RenderWindow& window, int boardSize);
void DrawHoverEffect(sf::RenderWindow& window, int mouseX, int mouseY, int boardSize);

// Overload không boardSize (fallback), và overload đầy đủ
void DrawWinLine(sf::RenderWindow& window, int sX, int sY, int eX, int eY);
void DrawWinLine(sf::RenderWindow& window, int sX, int sY, int eX, int eY, int boardSize);

// FIX: thêm boardSize và gameMode để tính đúng panelX và hiển thị đúng tên
void DrawInGamePanel(sf::RenderWindow& window, const sf::Font& font,
    float timeRemaining, bool isPlayerTurn,
    int gameStatus, int boardSize, GameMode gameMode,
    int undoLeft[2]);   // undoLeft[0]=P1, undoLeft[1]=P2

void DrawSettings(sf::RenderWindow& window, const sf::Font& font,
    int boardSize, bool ruleBlock2, int aiLevel,
    float sfxVolume, bool bgmEnabled);