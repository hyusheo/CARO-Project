#include "InputUI.h"
#include "CaroAPI.h"
#include <algorithm>

// ============================================================
//  Helper n?i b?
// ============================================================
static float PanelX(int boardSize)
{
    int   cellSz = GetDynCellSize(boardSize);
    float boardRight = static_cast<float>(Config::OFFSET_X + boardSize * cellSz);
    float gapWidth = static_cast<float>(Config::WIN_WIDTH) - boardRight;
    return boardRight + (gapWidth - static_cast<float>(Config::PANEL_W)) / 2.0f;
}

// ============================================================
//  HandleMenuInput
// ============================================================
void HandleMenuInput(
    sf::RenderWindow& window,
    int mouseX, int mouseY,
    AppState& currentState,
    GameMode& gameMode,
    int boardSize, bool ruleBlock2, int aiLevel,
    float& timeRemaining, bool& isPlayerTurn, int& gameStatus,
    sf::Sound& errSound)
{
    const float BTN_W = 350.f, BTN_H = 60.f;
    const float BTN_X = Config::WIN_WIDTH / 2.f - BTN_W / 2.f;

    for (int i = 0; i < 5; ++i)
    {
        float bY = 300.f + i * 80.f;
        if (mouseX >= BTN_X && mouseX <= BTN_X + BTN_W &&
            mouseY >= bY && mouseY <= bY + BTN_H)
        {
            if (i == 0) { gameMode = PVP; goto START_GAME; }
            if (i == 1) { gameMode = PVE; goto START_GAME; }
            if (false) {
            START_GAME:
                InitGame(boardSize, ruleBlock2, aiLevel);
                currentState = IN_GAME_SCREEN;
                isPlayerTurn = true;
                timeRemaining = 60.f;
                gameStatus = 0;
            }
            else if (i == 2) { currentState = SETTINGS_SCREEN; }
            else if (i == 3) {
                float st = 0.f; int stn = 0;
                if (LoadGameBinary("savegame.bin", &st, &stn)) {
                    timeRemaining = st;
                    isPlayerTurn = (stn == 1);
                    currentState = IN_GAME_SCREEN;
                    gameStatus = 0;
                }
                else errSound.play();
            }
            else if (i == 4) { window.close(); }
        }
    }
}

// ============================================================
//  HandleInGameInput
//
//  Undo PVP:
//    - M?i ng??i có Config::UNDO_MAX l??t (m?c ??nh 3)
//    - Không ???c dùng liên ti?p: sau khi dùng undo ph?i ?i 1 n??c
//      th?c s? tr??c khi ???c undo ti?p
//    - Undo ch? lùi 1 n??c c?a chính mình (UndoOneMove)
//    - Sau undo v?n là l??t c?a ng??i ?ó (???c ?i l?i)
//
//  Undo PVE:
//    - Gi? nguyên hành vi c?: undo theo c?p (AI + ng??i)
//    - Không gi?i h?n l??t (undoLeft không áp d?ng)
// ============================================================
void HandleInGameInput(
    int mouseX, int mouseY,
    AppState& currentState,
    int boardSize,
    GameMode gameMode,
    bool& isPlayerTurn, int& gameStatus, float& timeRemaining,
    int undoLeft[2], int& lastUndoPlayer,
    sf::Sound& errSound)
{
    // --- A. Khu v?c bàn c? ---
    int cellSz = GetDynCellSize(boardSize);
    const int BLEFT = Config::OFFSET_X;
    const int BTOP = Config::OFFSET_Y;
    const int BRIGHT = Config::OFFSET_X + boardSize * cellSz;
    const int BBOTTOM = Config::OFFSET_Y + boardSize * cellSz;

    if (mouseX >= BLEFT && mouseX <= BRIGHT &&
        mouseY >= BTOP && mouseY <= BBOTTOM)
    {
        if (gameMode == PVE && !isPlayerTurn) return;

        if (gameStatus == 0 && !IsAIThinking())
        {
            int gX = (mouseX - Config::OFFSET_X) / cellSz;
            int gY = (mouseY - Config::OFFSET_Y) / cellSz;

            int currentPlayer = (gameMode == PVP && !isPlayerTurn) ? 2 : 1;
            int res = ProcessMove(gX, gY, currentPlayer);

            if (res == -1)
            {
                errSound.play();
            }
            else
            {
                gameStatus = res;
                timeRemaining = 60.f;

                // Ng??i v?a ?i n??c th?c ? reset quy?n undo c?a h?
                // (???c phép undo l?i ? l??t sau n?u còn l??t)
                int playerIdx = (currentPlayer == 1) ? 0 : 1;
                lastUndoPlayer = -1; // ai c?ng có th? undo, không b? ch?n liên ti?p

                if (gameMode == PVP)
                    isPlayerTurn = !isPlayerTurn;
                else {
                    isPlayerTurn = false;
                    if (gameStatus == 0) StartAIThinking();
                }
            }
        }
        return;
    }

    // --- B. Nút Undo / Save / Main Menu ---
    const float pX = PanelX(boardSize);
    const float BTN_W = static_cast<float>(Config::PANEL_W);
    const float BTN_H = 52.f;

    for (int i = 0; i < 3; ++i)
    {
        float bX = pX;
        float bY = 420.f + i * 80.f;
        if (!(mouseX >= bX && mouseX <= bX + BTN_W &&
            mouseY >= bY && mouseY <= bY + BTN_H)) continue;

        if (i == 0) // ?? UNDO ??????????????????????????????
        {
            //Nếu thắng, hòa, thua thì sẽ không undo được nữa
            if (gameStatus != 0) {
                errSound.play();
                return;
            }
            if (gameMode == PVP)
            {
                // playerIdx = ng??i ?ang ??n l??t (ng??i b?m Undo)
                // isPlayerTurn true = l??t X (P1, idx 0)
                // isPlayerTurn false = l??t O (P2, idx 1)
                int playerIdx = isPlayerTurn ? 0 : 1;

                // Ch?n undo liên ti?p: ph?i ?i 1 n??c th?c tr??c
                if (lastUndoPlayer == playerIdx) {
                    errSound.play();
                    return;
                }
                // Ch?n khi h?t l??t undo
                if (undoLeft[playerIdx] <= 0) {
                    errSound.play();
                    return;
                }

                // ?? FIX BUG 1 & 2: dùng UndoMove() pop C?P 2 n??c ??
                // Lý do:
                //   Khi ??n l??t P1, stack top là n??c c?a P2 (v?a ?ánh).
                //   UndoOneMove() ch? xóa n??c P2 ? P1 không l?y l?i
                //   ???c n??c c?a mình, và l??t b? l?ch.
                //   UndoMove() xóa c? 2 (P2 + P1) ? P1 v? ?úng tr?ng
                //   thái tr??c khi P1 b?m n??c ?ó, isPlayerTurn không
                //   ??i ? P1 ???c ?i l?i ?úng.
                int undone = UndoMove();
                if (undone == 0) {
                    errSound.play(); // stack r?ng
                    return;
                }

                // Undo thành công
                undoLeft[playerIdx]--;
                lastUndoPlayer = playerIdx; // ?ánh d?u ng??i này v?a undo

                gameStatus = 0;
                timeRemaining = 60.f;

                // isPlayerTurn KHÔNG ??i ? ?úng ng??i v?a undo
                // ???c ?i l?i n??c c?a mình.
                //
                // Tr??ng h?p ??c bi?t: n?u stack ch? có 1 n??c
                // (P1 m?i ?ánh 1 n??c, ch?a ai ?ánh thêm) và P1 undo
                // ? undone == 1, board s?ch, tr? l??t v? P1 (isPlayerTurn = true)
                if (undone == 1) isPlayerTurn = true;
            }
            else // PVE
            {
                if (IsAIThinking()) { errSound.play(); return; }
                int undone = UndoMove(); // undo c?p AI + ng??i
                if (undone == 0) { errSound.play(); return; }
                gameStatus = 0;
                isPlayerTurn = true;
                timeRemaining = 60.f;
            }
        }
        else if (i == 1) // Save
        {
            SaveGameBinary("savegame.bin", timeRemaining, isPlayerTurn ? 1 : 0);
        }
        else if (i == 2) // Main Menu
        {
            currentState = MENU_SCREEN;
        }
    }
}

// ============================================================
//  HandleSettingsInput (không ??i)
// ============================================================
void HandleSettingsInput(
    int mouseX, int mouseY,
    AppState& currentState,
    int& boardSize, bool& ruleBlock2, int& aiLevel,
    float& sfxVolume, bool& bgmEnabled,
    sf::Sound& errSound)
{
    const float SY = 200.f, CX = 650.f, PX2 = CX + 200.f, BS = 50.f, RG = 80.f;

    auto hMinus = [&](float ry) { return mouseX >= CX && mouseX <= CX + BS && mouseY >= ry && mouseY <= ry + BS; };
    auto hPlus = [&](float ry) { return mouseX >= PX2 && mouseX <= PX2 + BS && mouseY >= ry && mouseY <= ry + BS; };
    auto hToggle = [&](float ry) { return mouseX >= CX && mouseX <= CX + 250 && mouseY >= ry && mouseY <= ry + BS; };

    float r0 = SY;
    if (hMinus(r0)) { boardSize = std::max(10, boardSize - 1); errSound.play(); }
    else if (hPlus(r0)) { boardSize = std::min(30, boardSize + 1); errSound.play(); }

    if (hToggle(SY + RG)) { ruleBlock2 = !ruleBlock2; errSound.play(); }

    float r2 = SY + RG * 2;
    if (hMinus(r2)) { aiLevel = std::max(1, aiLevel - 1); errSound.play(); }
    else if (hPlus(r2)) { aiLevel = std::min(6, aiLevel + 1); errSound.play(); }

    float r3 = SY + RG * 3;
    if (hMinus(r3)) {
        sfxVolume = std::max(0.f, sfxVolume - 10.f);
        errSound.setVolume(sfxVolume); errSound.play();
    }
    else if (hPlus(r3)) {
        sfxVolume = std::min(100.f, sfxVolume + 10.f);
        errSound.setVolume(sfxVolume); errSound.play();
    }

    if (hToggle(SY + RG * 4)) { bgmEnabled = !bgmEnabled; errSound.play(); }

    const float BW = 300.f, BH = 60.f;
    const float BX = Config::WIN_WIDTH / 2.f - BW / 2.f, BY = 650.f;
    if (mouseX >= BX && mouseX <= BX + BW && mouseY >= BY && mouseY <= BY + BH)
        currentState = MENU_SCREEN;
}