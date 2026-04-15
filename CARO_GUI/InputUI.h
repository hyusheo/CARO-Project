#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>
#include "Constants.h"

void HandleMenuInput(sf::RenderWindow& window, int mouseX, int mouseY, AppState& currentState, GameMode& gameMode, int boardSize, bool ruleBlock2, int aiLevel, float& timeRemaining, bool& isPlayerTurn, int& gameStatus, sf::Sound& errSound, int& currentLoadedSlot, std::string& currentLoadedName);

void HandleInGameInput(int mouseX, int mouseY, AppState& currentState, int boardSize, GameMode gameMode, bool& isPlayerTurn, int& gameStatus, float& timeRemaining, int undoLeft[2], int& lastUndoPlayer, float& saveNotifTimer, sf::Sound& errSound, int& currentLoadedSlot, std::string& currentLoadedName);

void HandleSettingsInput(int mouseX, int mouseY, AppState& currentState, int& boardSize, bool& ruleBlock2, int& aiLevel, float& sfxVolume, bool& bgmEnabled, sf::Sound& errSound);

void HandleLoadInput(sf::RenderWindow& window, int mouseX, int mouseY, AppState& currentState, float& timeRemaining, bool& isPlayerTurn, int& gameStatus, sf::Sound& errSound, int& currentLoadedSlot, std::string& currentLoadedName);

void HandleSaveInput(sf::RenderWindow& window, int mouseX, int mouseY, AppState& currentState, float timeRemaining, bool isPlayerTurn, float& saveNotifTimer, sf::Sound& errSound, bool& isNaming, int& selectedSlot, std::string& inputName, int& currentLoadedSlot, std::string& currentLoadedName);