#include "InputUI.h"
#include "CaroAPI.h"
#include <algorithm>

static float PanelX(int boardSize) {
    int cellSz = GetDynCellSize(boardSize);
    float boardRight = static_cast<float>(Config::OFFSET_X + boardSize * cellSz);
    float gapWidth = static_cast<float>(Config::WIN_WIDTH) - boardRight;
    return boardRight + (gapWidth - static_cast<float>(Config::PANEL_W)) / 2.0f;
}

void HandleMenuInput(sf::RenderWindow& window, int mouseX, int mouseY, AppState& currentState, GameMode& gameMode, int boardSize, bool ruleBlock2, int aiLevel, float& timeRemaining, bool& isPlayerTurn, int& gameStatus, sf::Sound& errSound, int& currentLoadedSlot, std::string& currentLoadedName) {
    const float BTN_W = 350.f, BTN_H = 60.f, BTN_X = Config::WIN_WIDTH / 2.f - BTN_W / 2.f;
    for (int i = 0; i < 5; ++i) {
        float bY = 300.f + i * 80.f;
        if (mouseX >= BTN_X && mouseX <= BTN_X + BTN_W && mouseY >= bY && mouseY <= bY + BTN_H) {
            if (i == 0 || i == 1) {
                gameMode = (i == 0) ? PVP : PVE;
                InitGame(boardSize, ruleBlock2, aiLevel);
                currentState = IN_GAME_SCREEN;
                isPlayerTurn = true;
                timeRemaining = 60.f;
                gameStatus = 0;

                // RESET: Game mới nên không nhớ slot nào hết
                currentLoadedSlot = -1;
                currentLoadedName = "";
            }
            else if (i == 2) { currentState = SETTINGS_SCREEN; }
            else if (i == 3) { currentState = LOAD_SCREEN; }
            else if (i == 4) { window.close(); }
        }
    }
}

void HandleInGameInput(int mouseX, int mouseY, AppState& currentState, int boardSize, GameMode gameMode, bool& isPlayerTurn, int& gameStatus, float& timeRemaining, int undoLeft[2], int& lastUndoPlayer, float& saveNotifTimer, sf::Sound& errSound, int& currentLoadedSlot, std::string& currentLoadedName) {
    int cellSz = GetDynCellSize(boardSize);
    const int BLEFT = Config::OFFSET_X, BTOP = Config::OFFSET_Y;
    const int BRIGHT = Config::OFFSET_X + boardSize * cellSz, BBOTTOM = Config::OFFSET_Y + boardSize * cellSz;

    if (mouseX >= BLEFT && mouseX <= BRIGHT && mouseY >= BTOP && mouseY <= BBOTTOM) {
        if (gameMode == PVE && !isPlayerTurn) return;
        if (gameStatus == 0 && !IsAIThinking()) {
            int gX = (mouseX - Config::OFFSET_X) / cellSz, gY = (mouseY - Config::OFFSET_Y) / cellSz;
            int currentPlayer = (gameMode == PVP && !isPlayerTurn) ? 2 : 1;
            int res = ProcessMove(gX, gY, currentPlayer);
            if (res == -1) errSound.play();
            else {
                gameStatus = res; timeRemaining = 60.f; lastUndoPlayer = -1;
                if (gameMode == PVP) isPlayerTurn = !isPlayerTurn;
                else { isPlayerTurn = false; if (gameStatus == 0) StartAIThinking(); }
            }
        }
        return;
    }

    const float pX = PanelX(boardSize), BTN_W = static_cast<float>(Config::PANEL_W), BTN_H = 52.f;
    for (int i = 0; i < 3; ++i) {
        float bX = pX, bY = 420.f + i * 80.f;
        if (!(mouseX >= bX && mouseX <= bX + BTN_W && mouseY >= bY && mouseY <= bY + BTN_H)) continue;

        if (i == 0) { // UNDO
            if (gameMode == PVP) {
                int playerIdx = isPlayerTurn ? 0 : 1;
                if (lastUndoPlayer == playerIdx || undoLeft[playerIdx] <= 0) { errSound.play(); return; }
                int undone = UndoMove();
                if (undone == 0) { errSound.play(); return; }
                undoLeft[playerIdx]--; lastUndoPlayer = playerIdx; gameStatus = 0; timeRemaining = 60.f;
                if (undone == 1) isPlayerTurn = true;
            }
            else {
                if (IsAIThinking()) { errSound.play(); return; }
                if (UndoMove() == 0) { errSound.play(); return; }
                gameStatus = 0; isPlayerTurn = true; timeRemaining = 60.f;
            }
        }
        else if (i == 1) { // SAVE GAME
            if (currentLoadedSlot != -1) {
                // Đã có Slot lưu từ trước -> Quick Save luôn, không hỏi nhiều!
                if (SaveGameSlot(currentLoadedSlot, timeRemaining, isPlayerTurn ? 1 : 0, currentLoadedName.c_str())) {
                    saveNotifTimer = 2.0f; // Bật thông báo màu xanh
                }
                else {
                    errSound.play();
                }
            }
            else {
                // Lần đầu Save -> Mở bảng chọn Slot
                currentState = SAVE_SCREEN;
            }
        }
        else if (i == 2) { currentState = MENU_SCREEN; }
    }
}

void HandleSettingsInput(int mouseX, int mouseY, AppState& currentState, int& boardSize, bool& ruleBlock2, int& aiLevel, float& sfxVolume, bool& bgmEnabled, sf::Sound& errSound) {
    const float SY = 200.f, CX = 650.f, PX2 = CX + 200.f, BS = 50.f, RG = 80.f;
    auto hMinus = [&](float ry) { return mouseX >= CX && mouseX <= CX + BS && mouseY >= ry && mouseY <= ry + BS; };
    auto hPlus = [&](float ry) { return mouseX >= PX2 && mouseX <= PX2 + BS && mouseY >= ry && mouseY <= ry + BS; };
    auto hToggle = [&](float ry) { return mouseX >= CX && mouseX <= CX + 250 && mouseY >= ry && mouseY <= ry + BS; };

    if (hMinus(SY)) { boardSize = std::max(10, boardSize - 1); errSound.play(); }
    else if (hPlus(SY)) { boardSize = std::min(30, boardSize + 1); errSound.play(); }
    if (hToggle(SY + RG)) { ruleBlock2 = !ruleBlock2; errSound.play(); }
    if (hMinus(SY + RG * 2)) { aiLevel = std::max(1, aiLevel - 1); errSound.play(); }
    else if (hPlus(SY + RG * 2)) { aiLevel = std::min(6, aiLevel + 1); errSound.play(); }
    if (hMinus(SY + RG * 3)) { sfxVolume = std::max(0.f, sfxVolume - 10.f); errSound.setVolume(sfxVolume); errSound.play(); }
    else if (hPlus(SY + RG * 3)) { sfxVolume = std::min(100.f, sfxVolume + 10.f); errSound.setVolume(sfxVolume); errSound.play(); }
    if (hToggle(SY + RG * 4)) { bgmEnabled = !bgmEnabled; errSound.play(); }

    if (mouseX >= Config::WIN_WIDTH / 2.f - 150.f && mouseX <= Config::WIN_WIDTH / 2.f + 150.f && mouseY >= 650.f && mouseY <= 710.f) currentState = MENU_SCREEN;
}

void HandleLoadInput(sf::RenderWindow& window, int mouseX, int mouseY, AppState& currentState, float& timeRemaining, bool& isPlayerTurn, int& gameStatus, sf::Sound& errSound, int& currentLoadedSlot, std::string& currentLoadedName) {
    const float START_X = 80.f, START_Y = 150.f, BTN_W = 400.f, BTN_H = 75.f, DEL_W = 70.f;
    for (int i = 1; i <= 5; ++i) {
        float bY = START_Y + (i - 1) * 95.f;
        int size = 0, moves = 0, turn = 0; char name[64];
        bool hasData = PeekGameSlot(i, &size, &moves, &turn, name);

        if (hasData && mouseX >= START_X + BTN_W + 10.f && mouseX <= START_X + BTN_W + 10.f + DEL_W && mouseY >= bY && mouseY <= bY + BTN_H) {
            DeleteGameSlot(i); errSound.play(); return;
        }

        if (mouseX >= START_X && mouseX <= START_X + BTN_W && mouseY >= bY && mouseY <= bY + BTN_H) {
            float tRem = 0; int turnData = 0;
            if (LoadGameSlot(i, &tRem, &turnData)) {
                timeRemaining = tRem;
                isPlayerTurn = (turnData == 1);
                gameStatus = EvaluateBoard();

                // Ghi nhớ lại Slot và Tên vừa load để đánh tiếp save khỏi hỏi
                currentLoadedSlot = i;
                currentLoadedName = std::string(name);

                currentState = IN_GAME_SCREEN;
            }
            else { errSound.play(); }
            return;
        }
    }
    if (mouseX >= 80.f && mouseX <= 280.f && mouseY >= 680.f && mouseY <= 740.f) currentState = MENU_SCREEN;
}

void HandleSaveInput(sf::RenderWindow& window, int mouseX, int mouseY, AppState& currentState, float timeRemaining, bool isPlayerTurn, float& saveNotifTimer, sf::Sound& errSound, bool& isNaming, int& selectedSlot, std::string& inputName, int& currentLoadedSlot, std::string& currentLoadedName) {
    const float START_X = 80.f, START_Y = 150.f, BTN_W = 480.f, BTN_H = 75.f;
    if (!isNaming) {
        for (int i = 1; i <= 5; ++i) {
            float bY = START_Y + (i - 1) * 95.f;
            if (mouseX >= START_X && mouseX <= START_X + BTN_W && mouseY >= bY && mouseY <= bY + BTN_H) {
                selectedSlot = i; isNaming = true; inputName = ""; return;
            }
        }
        if (mouseX >= 80.f && mouseX <= 280.f && mouseY >= 680.f && mouseY <= 740.f) currentState = IN_GAME_SCREEN;
    }
    else {
        float w = Config::WIN_WIDTH / 2.f, h = Config::WIN_HEIGHT / 2.f;
        if (mouseX >= w - 100.f && mouseX <= w + 100.f && mouseY >= h + 35.f && mouseY <= h + 85.f) {
            if (inputName.empty()) inputName = "Game khong ten";
            if (SaveGameSlot(selectedSlot, timeRemaining, isPlayerTurn ? 1 : 0, inputName.c_str())) {
                saveNotifTimer = 2.0f;

                // Ghi nhớ lại Slot và Tên lần đầu tiên Save này!
                currentLoadedSlot = selectedSlot;
                currentLoadedName = inputName;

                isNaming = false;
                currentState = IN_GAME_SCREEN;
            }
            else errSound.play();
        }
        else if (mouseX < w - 250.f || mouseX > w + 250.f || mouseY < h - 125.f || mouseY > h + 125.f) {
            isNaming = false;
        }
    }
}