#include "RenderUI.h"
#include "Constants.h"
#include "CaroAPI.h"
#include <cmath>
#include <string>

// ============================================================
//  Helper: tính tọa độ X để panel luôn NẰM GIỮA khoảng trống
// ============================================================
static float PanelX(int boardSize)
{
    int cellSz = GetDynCellSize(boardSize);
    float boardRight = static_cast<float>(Config::OFFSET_X + boardSize * cellSz);
    float gapWidth = static_cast<float>(Config::WIN_WIDTH) - boardRight;
    return boardRight + (gapWidth - static_cast<float>(Config::PANEL_W)) / 2.0f;
}

// ============================================================
//  DrawMenu
// ============================================================
void DrawMenu(sf::RenderWindow& window, const sf::Font& font)
{
    sf::Text title("CARO MASTER", font, 70);
    title.setFillColor(COLOR_X);
    sf::FloatRect r = title.getLocalBounds();
    title.setOrigin(r.left + r.width / 2.0f, r.top + r.height / 2.0f);
    title.setPosition(Config::WIN_WIDTH / 2.0f, 150.0f);
    window.draw(title);

    const char* menuItems[] = {
        "PVP - 2 Nguoi Choi",
        "PVE - Choi voi May",
        "Cai Dat",
        "Tai Game (Load)",
        "Thoat"
    };
    const float BTN_W = 350.0f;
    const float BTN_H = 60.0f;
    const float START_Y = 300.0f;
    sf::Vector2i mp = sf::Mouse::getPosition(window);

    for (int i = 0; i < 5; ++i) {
        float bX = Config::WIN_WIDTH / 2.0f - BTN_W / 2.0f;
        float bY = START_Y + i * 80.0f;
        bool hov = (mp.x >= bX && mp.x <= bX + BTN_W && mp.y >= bY && mp.y <= bY + BTN_H);

        sf::RectangleShape btn(sf::Vector2f(BTN_W, BTN_H));
        btn.setPosition(bX, bY);
        btn.setOutlineThickness(hov ? 2.f : 1.f);
        btn.setOutlineColor(hov ? COLOR_O : GRID_COLOR);
        btn.setFillColor(hov ? sf::Color(60, 60, 70) : BTN_COLOR);
        window.draw(btn);

        sf::Text txt(menuItems[i], font, 24);
        txt.setFillColor(sf::Color::White);
        sf::FloatRect tr = txt.getLocalBounds();
        txt.setOrigin(tr.left + tr.width / 2.f, tr.top + tr.height / 2.f);
        txt.setPosition(bX + BTN_W / 2.f, bY + BTN_H / 2.f);
        window.draw(txt);
    }
}

// ============================================================
//  DrawInGamePanel
// ============================================================
void DrawInGamePanel(sf::RenderWindow& window, const sf::Font& font, float timeRemaining, bool isPlayerTurn, int gameStatus, int boardSize, GameMode gameMode, int undoLeft[2], float saveNotifTimer)
{
    const float pX = PanelX(boardSize);
    const float BTN_W = static_cast<float>(Config::PANEL_W);
    const float BTN_H = 52.0f;

    int cellSz = GetDynCellSize(boardSize);
    float boardRight = static_cast<float>(Config::OFFSET_X + boardSize * cellSz);

    sf::RectangleShape divider(sf::Vector2f(2.f, static_cast<float>(Config::WIN_HEIGHT - 2 * Config::OFFSET_Y)));
    divider.setPosition(boardRight + Config::PANEL_GAP / 2.f, static_cast<float>(Config::OFFSET_Y));
    divider.setFillColor(GRID_COLOR);
    window.draw(divider);

    sf::Text turnLabel("LUOT DI:", font, 18);
    turnLabel.setFillColor(sf::Color(180, 180, 180));
    turnLabel.setPosition(pX, 60.f);
    window.draw(turnLabel);

    std::string turnStr;
    sf::Color turnColor;
    if (gameMode == PVP) {
        if (isPlayerTurn) {
            turnStr = "NGUOI 1 (X)";
            turnColor = COLOR_X;
        }
        else {
            turnStr = "NGUOI 2 (O)";
            turnColor = COLOR_O;
        }
    }
    else {
        if (isPlayerTurn) {
            turnStr = "NGUOI (X)";
            turnColor = COLOR_X;
        }
        else {
            turnStr = "MAY (O)";
            turnColor = COLOR_O;
        }
    }

    sf::Text turnText(turnStr, font, 26);
    turnText.setFillColor(turnColor);
    turnText.setStyle(sf::Text::Bold);
    turnText.setPosition(pX, 85.f);
    window.draw(turnText);

    sf::Text timerLabel("THOI GIAN CON:", font, 18);
    timerLabel.setFillColor(sf::Color(180, 180, 180));
    timerLabel.setPosition(pX, 135.f);
    window.draw(timerLabel);

    sf::Color timerColor = (timeRemaining > 20.f) ? sf::Color(80, 255, 160) : (timeRemaining > 10.f) ? sf::Color(255, 200, 50) : sf::Color(255, 60, 60);

    sf::Text timerText(std::to_string((int)timeRemaining) + " giay", font, 34);
    timerText.setFillColor(timerColor);
    timerText.setStyle(sf::Text::Bold);
    timerText.setPosition(pX, 158.f);
    window.draw(timerText);

    sf::RectangleShape barBg(sf::Vector2f(BTN_W, 8.f));
    barBg.setPosition(pX, 200.f);
    barBg.setFillColor(sf::Color(50, 50, 60));
    window.draw(barBg);

    float ratio = std::max(0.f, std::min(1.f, timeRemaining / 60.f));
    sf::RectangleShape barFg(sf::Vector2f(BTN_W * ratio, 8.f));
    barFg.setPosition(pX, 200.f);
    barFg.setFillColor(timerColor);
    window.draw(barFg);

    if (gameStatus != 0) {
        sf::RectangleShape rBox(sf::Vector2f(BTN_W, 60.f));
        rBox.setPosition(pX, 220.f);
        rBox.setFillColor(sf::Color(50, 50, 20));
        rBox.setOutlineThickness(2);
        rBox.setOutlineColor(WIN_LINE_COLOR);
        window.draw(rBox);

        std::string s;
        if (gameMode == PVP) {
            s = (gameStatus == 1) ? "NGUOI 1 THANG!" : ((gameStatus == 2) ? "NGUOI 2 THANG!" : "HOA!");
        }
        else {
            s = (gameStatus == 1) ? "NGUOI THANG!" : ((gameStatus == 2) ? "MAY THANG!" : "HOA!");
        }

        sf::Text st(s, font, 26);
        st.setFillColor(WIN_LINE_COLOR);
        st.setStyle(sf::Text::Bold);
        sf::FloatRect sr = st.getLocalBounds();
        st.setOrigin(sr.left + sr.width / 2.f, sr.top + sr.height / 2.f);
        st.setPosition(pX + BTN_W / 2.f, 250.f);
        window.draw(st);
    }
    else {
        std::string hintStr;
        if (gameMode == PVP) {
            hintStr = isPlayerTurn ? "Luot Nguoi 1 - bam ban co" : "Luot Nguoi 2 - bam ban co";
        }
        else {
            hintStr = isPlayerTurn ? "Click vao ban co de di" : "Dang doi AI suy nghi...";
        }
        sf::Text hint(hintStr, font, 15);
        hint.setFillColor(sf::Color(120, 120, 140));
        hint.setPosition(pX, 220.f);
        window.draw(hint);
    }

    if (gameMode == PVP) {
        sf::Text undoTitle("LUOT UNDO CON LAI:", font, 17);
        undoTitle.setFillColor(sf::Color(180, 180, 180));
        undoTitle.setPosition(pX, 295.f);
        window.draw(undoTitle);

        struct PUI { std::string label; int left; sf::Color color; float posY; } info[2] = {
            { "Nguoi 1 (X):", undoLeft[0], COLOR_X, 320.f },
            { "Nguoi 2 (O):", undoLeft[1], COLOR_O, 350.f }
        };

        for (int p = 0; p < 2; ++p) {
            sf::Text lblTxt(info[p].label, font, 18);
            lblTxt.setFillColor(info[p].color);
            lblTxt.setPosition(pX, info[p].posY);
            window.draw(lblTxt);

            float startX = pX + 130.f;
            for (int k = 0; k < Config::UNDO_MAX; ++k) {
                sf::CircleShape dot(8.f);
                dot.setPosition(startX + k * 22.f, info[p].posY + 1.f);
                if (k < info[p].left) {
                    dot.setFillColor(info[p].color);
                    dot.setOutlineThickness(0);
                }
                else {
                    dot.setFillColor(sf::Color(30, 30, 40));
                    dot.setOutlineThickness(1.5f);
                    dot.setOutlineColor(sf::Color(80, 80, 90));
                }
                window.draw(dot);
            }
        }
        sf::RectangleShape sep(sf::Vector2f(BTN_W, 1.f));
        sep.setPosition(pX, 382.f);
        sep.setFillColor(sf::Color(60, 60, 70));
        window.draw(sep);
    }

    const float BTN_START_Y = (gameMode == PVP) ? 395.f : 420.f;
    const char* gameBtns[] = { "Undo", "Save Game", "Main Menu" };
    sf::Vector2i mp = sf::Mouse::getPosition(window);

    for (int i = 0; i < 3; ++i) {
        float bX = pX;
        float bY = BTN_START_Y + i * 80.f;
        bool hov = (mp.x >= bX && mp.x <= bX + BTN_W && mp.y >= bY && mp.y <= bY + BTN_H);

        sf::RectangleShape btn(sf::Vector2f(BTN_W, BTN_H));
        btn.setPosition(bX, bY);
        btn.setOutlineThickness(hov ? 2.f : 1.f);
        btn.setOutlineColor(hov ? COLOR_O : GRID_COLOR);
        btn.setFillColor(hov ? sf::Color(60, 60, 80) : BTN_COLOR);
        window.draw(btn);

        sf::Text t(gameBtns[i], font, 22);
        t.setFillColor(sf::Color::White);
        sf::FloatRect tr = t.getLocalBounds();
        t.setOrigin(tr.left + tr.width / 2.f, tr.top + tr.height / 2.f);
        t.setPosition(bX + BTN_W / 2.f, bY + BTN_H / 2.f);
        window.draw(t);
    }

    if (saveNotifTimer > 0.f) {
        int alpha = saveNotifTimer < 0.5f ? static_cast<int>((saveNotifTimer / 0.5f) * 255.f) : 255;
        float bCx = static_cast<float>(Config::OFFSET_X) + (boardSize * cellSz) / 2.0f;
        float bCy = static_cast<float>(Config::OFFSET_Y) + (boardSize * cellSz) / 2.0f;

        sf::RectangleShape notifBox(sf::Vector2f(400.f, 80.f));
        notifBox.setPosition(bCx - 200.f, bCy - 40.f);
        notifBox.setFillColor(sf::Color(10, 10, 10, alpha));
        notifBox.setOutlineThickness(3.f);
        notifBox.setOutlineColor(sf::Color(50, 255, 50, alpha));
        window.draw(notifBox);

        sf::Text notifTxt("DA LUU VAN GAME!", font, 36);
        notifTxt.setFillColor(sf::Color(255, 255, 255, alpha));
        notifTxt.setStyle(sf::Text::Bold);
        sf::FloatRect nr = notifTxt.getLocalBounds();
        notifTxt.setOrigin(nr.left + nr.width / 2.f, nr.top + nr.height / 2.f);
        notifTxt.setPosition(bCx, bCy);
        window.draw(notifTxt);
    }
}

// ============================================================
//  DrawBoard
// ============================================================
void DrawBoard(sf::RenderWindow& window, int boardSize)
{
    int cellSz = GetDynCellSize(boardSize);
    for (int i = 0; i <= boardSize; ++i) {
        sf::RectangleShape h(sf::Vector2f(static_cast<float>(boardSize * cellSz), 1.f));
        h.setPosition(static_cast<float>(Config::OFFSET_X), static_cast<float>(Config::OFFSET_Y + i * cellSz));
        h.setFillColor(GRID_COLOR);
        window.draw(h);

        sf::RectangleShape v(sf::Vector2f(1.f, static_cast<float>(boardSize * cellSz)));
        v.setPosition(static_cast<float>(Config::OFFSET_X + i * cellSz), static_cast<float>(Config::OFFSET_Y));
        v.setFillColor(GRID_COLOR);
        window.draw(v);
    }
}

// ============================================================
//  DrawPieces
// ============================================================
void DrawPieces(sf::RenderWindow& window, int boardSize)
{
    int cellSz = GetDynCellSize(boardSize);
    for (int x = 0; x < boardSize; ++x) {
        for (int y = 0; y < boardSize; ++y) {
            int cell = GetCell(x, y);
            float px = static_cast<float>(Config::OFFSET_X + x * cellSz);
            float py = static_cast<float>(Config::OFFSET_Y + y * cellSz);

            if (cell == 1) {
                sf::RectangleShape ln(sf::Vector2f(cellSz - 6.f, 2.5f));
                ln.setFillColor(COLOR_X);
                ln.setOrigin((cellSz - 6.f) / 2.f, 1.25f);
                ln.setPosition(px + cellSz / 2.f, py + cellSz / 2.f);
                ln.rotate(45);
                window.draw(ln);

                sf::RectangleShape ln2 = ln;
                ln2.rotate(90);
                window.draw(ln2);
            }
            else if (cell == 2) {
                sf::CircleShape c(cellSz / 2.f - 3.f);
                c.setFillColor(sf::Color::Transparent);
                c.setOutlineThickness(2.5f);
                c.setOutlineColor(COLOR_O);
                c.setPosition(px + 3.f, py + 3.f);
                window.draw(c);
            }
        }
    }
}

// ============================================================
//  DrawHoverEffect
// ============================================================
void DrawHoverEffect(sf::RenderWindow& window, int mouseX, int mouseY, int boardSize)
{
    int cellSz = GetDynCellSize(boardSize);
    int gX = (mouseX - Config::OFFSET_X) / cellSz;
    int gY = (mouseY - Config::OFFSET_Y) / cellSz;

    if (gX >= 0 && gX < boardSize && gY >= 0 && gY < boardSize && GetCell(gX, gY) == 0) {
        sf::RectangleShape hr(sf::Vector2f(cellSz - 2.f, cellSz - 2.f));
        hr.setPosition(static_cast<float>(Config::OFFSET_X + gX * cellSz + 1), static_cast<float>(Config::OFFSET_Y + gY * cellSz + 1));
        hr.setFillColor(HOVER_COLOR);
        window.draw(hr);
    }
}

// ============================================================
//  DrawWinLine
// ============================================================
void DrawWinLine(sf::RenderWindow& window, int sX, int sY, int eX, int eY)
{
    DrawWinLine(window, sX, sY, eX, eY, 15);
}

void DrawWinLine(sf::RenderWindow& window, int sX, int sY, int eX, int eY, int boardSize)
{
    if (sX == -1) return;

    int cs = GetDynCellSize(boardSize);
    float x1 = Config::OFFSET_X + sX * cs + cs / 2.f;
    float y1 = Config::OFFSET_Y + sY * cs + cs / 2.f;
    float x2 = Config::OFFSET_X + eX * cs + cs / 2.f;
    float y2 = Config::OFFSET_Y + eY * cs + cs / 2.f;

    float len = std::sqrt(std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2));
    float angle = std::atan2(y2 - y1, x2 - x1) * 180.f / 3.14159265f;

    sf::RectangleShape line(sf::Vector2f(len, 4.f));
    line.setFillColor(WIN_LINE_COLOR);
    line.setOrigin(0, 2.f);
    line.setPosition(x1, y1);
    line.setRotation(angle);
    window.draw(line);
}

// ============================================================
//  DrawSettings
// ============================================================
void DrawSettings(sf::RenderWindow& window, const sf::Font& font, int boardSize, bool ruleBlock2, int aiLevel, float sfxVolume, bool bgmEnabled)
{
    sf::Text title("CAI DAT HE THONG", font, 60);
    title.setFillColor(WIN_LINE_COLOR);
    sf::FloatRect r = title.getLocalBounds();
    title.setOrigin(r.left + r.width / 2.f, r.top + r.height / 2.f);
    title.setPosition(Config::WIN_WIDTH / 2.f, 80.f);
    window.draw(title);

    const char* labels[] = {
        "Kich thuoc ban co (10-30):",
        "Luat chan 2 dau:",
        "Do kho AI (1-6):",
        "Am luong SFX (0-100):",
        "Nhac nen BGM:"
    };
    std::string values[] = {
        std::to_string(boardSize),
        ruleBlock2 ? "BAT (ON)" : "TAT (OFF)",
        std::to_string(aiLevel),
        std::to_string((int)sfxVolume),
        bgmEnabled ? "BAT (ON)" : "TAT (OFF)"
    };

    const float SY = 200.f, LX = 250.f, CX = 650.f, RG = 80.f;
    sf::Vector2i mp = sf::Mouse::getPosition(window);

    for (int i = 0; i < 5; ++i) {
        float cy = SY + i * RG;
        sf::Text lbl(labels[i], font, 28);
        lbl.setFillColor(sf::Color::White);
        lbl.setPosition(LX, cy);
        window.draw(lbl);

        if (i == 0 || i == 2 || i == 3) {
            sf::RectangleShape mb(sf::Vector2f(50, 50));
            mb.setPosition(CX, cy);
            mb.setFillColor((mp.x >= CX && mp.x <= CX + 50 && mp.y >= cy && mp.y <= cy + 50) ? sf::Color(100, 50, 50) : BTN_COLOR);
            mb.setOutlineThickness(1);
            mb.setOutlineColor(GRID_COLOR);
            window.draw(mb);

            sf::Text mt("-", font, 40);
            mt.setFillColor(sf::Color::White);
            mt.setPosition(CX + 15, cy - 5);
            window.draw(mt);

            sf::Text vt(values[i], font, 28);
            vt.setFillColor(COLOR_O);
            vt.setPosition(CX + 80, cy + 8);
            window.draw(vt);

            float pX2 = CX + 200.f;
            sf::RectangleShape pb(sf::Vector2f(50, 50));
            pb.setPosition(pX2, cy);
            pb.setFillColor((mp.x >= pX2 && mp.x <= pX2 + 50 && mp.y >= cy && mp.y <= cy + 50) ? sf::Color(50, 100, 50) : BTN_COLOR);
            pb.setOutlineThickness(1);
            pb.setOutlineColor(GRID_COLOR);
            window.draw(pb);

            sf::Text pt("+", font, 40);
            pt.setFillColor(sf::Color::White);
            pt.setPosition(pX2 + 12, cy - 5);
            window.draw(pt);
        }
        else {
            sf::RectangleShape tb(sf::Vector2f(250, 50));
            tb.setPosition(CX, cy);
            tb.setFillColor((mp.x >= CX && mp.x <= CX + 250 && mp.y >= cy && mp.y <= cy + 50) ? sf::Color(60, 60, 70) : BTN_COLOR);
            tb.setOutlineThickness(1);
            tb.setOutlineColor(GRID_COLOR);
            window.draw(tb);

            sf::Text vt(values[i], font, 28);
            vt.setFillColor(values[i] == "BAT (ON)" ? COLOR_O : COLOR_X);
            vt.setPosition(CX + 50, cy + 8);
            window.draw(vt);
        }
    }

    const float BW = 300.f, BH = 60.f;
    const float BX = Config::WIN_WIDTH / 2.f - BW / 2.f;
    const float BY = 650.f;
    sf::RectangleShape bb(sf::Vector2f(BW, BH));
    bb.setPosition(BX, BY);
    bool bh = (mp.x >= BX && mp.x <= BX + BW && mp.y >= BY && mp.y <= BY + BH);
    bb.setOutlineThickness(bh ? 2.f : 1.f);
    bb.setOutlineColor(bh ? COLOR_O : GRID_COLOR);
    bb.setFillColor(bh ? sf::Color(60, 60, 70) : BTN_COLOR);
    window.draw(bb);

    sf::Text bt("QUAY LAI MENU", font, 24);
    bt.setFillColor(sf::Color::White);
    sf::FloatRect br = bt.getLocalBounds();
    bt.setOrigin(br.left + br.width / 2.f, br.top + br.height / 2.f);
    bt.setPosition(BX + BW / 2.f, BY + BH / 2.f);
    window.draw(bt);
}

// ============================================================
//  DrawLoadScreen
// ============================================================
void DrawLoadScreen(sf::RenderWindow& window, const sf::Font& font)
{
    sf::Text title("DANH SACH DIEM LUU", font, 45);
    title.setFillColor(WIN_LINE_COLOR);
    title.setStyle(sf::Text::Bold);
    title.setPosition(80.f, 50.f);
    window.draw(title);

    sf::Vector2i mp = sf::Mouse::getPosition(window);
    int hoveredSlot = -1;
    const float BTN_W = 400.f, BTN_H = 75.f, START_X = 80.f, START_Y = 150.f, DEL_W = 70.f;

    for (int i = 1; i <= 5; ++i) {
        float bY = START_Y + (i - 1) * 95.f;
        bool hov = (mp.x >= START_X && mp.x <= START_X + BTN_W && mp.y >= bY && mp.y <= bY + BTN_H);

        if (hov) hoveredSlot = i;

        int size = 0, moves = 0, turn = 0;
        char gName[64] = "";
        bool hasData = PeekGameSlot(i, &size, &moves, &turn, gName);

        sf::RectangleShape btn(sf::Vector2f(BTN_W, BTN_H));
        btn.setPosition(START_X, bY);
        btn.setOutlineThickness(hov ? 3.f : 1.f);
        btn.setOutlineColor(hov ? COLOR_O : GRID_COLOR);
        btn.setFillColor(hov ? sf::Color(60, 60, 70) : BTN_COLOR);
        window.draw(btn);

        std::string slotText = (hasData && std::string(gName) != "") ? std::string(gName) : "Slot " + std::to_string(i);
        if (!hasData) slotText += " - [ TRONG ]";

        sf::Text txt(slotText, font, 22);
        txt.setFillColor(hasData ? sf::Color::White : sf::Color(130, 130, 130));
        txt.setPosition(START_X + 25.f, bY + BTN_H / 2.f - 15.f);
        window.draw(txt);

        if (hasData) {
            float delX = START_X + BTN_W + 10.f;
            bool delHov = (mp.x >= delX && mp.x <= delX + DEL_W && mp.y >= bY && mp.y <= bY + BTN_H);

            sf::RectangleShape delBtn(sf::Vector2f(DEL_W, BTN_H));
            delBtn.setPosition(delX, bY);
            delBtn.setOutlineThickness(1.f);
            delBtn.setOutlineColor(sf::Color(255, 100, 100));
            delBtn.setFillColor(delHov ? sf::Color(200, 50, 50) : sf::Color(120, 30, 30));
            window.draw(delBtn);

            sf::Text dTxt("X", font, 28);
            dTxt.setFillColor(sf::Color::White);
            dTxt.setStyle(sf::Text::Bold);
            dTxt.setPosition(delX + DEL_W / 2.f - 10.f, bY + BTN_H / 2.f - 20.f);
            window.draw(dTxt);
        }
    }

    float backX = 80.f, backY = 680.f;
    bool backHov = (mp.x >= backX && mp.x <= backX + 200.f && mp.y >= backY && mp.y <= backY + 60.f);
    sf::RectangleShape backBtn(sf::Vector2f(200.f, 60.f));
    backBtn.setPosition(backX, backY);
    backBtn.setOutlineThickness(2.f);
    backBtn.setOutlineColor(GRID_COLOR);
    backBtn.setFillColor(backHov ? sf::Color(100, 50, 50) : BTN_COLOR);
    window.draw(backBtn);

    sf::Text bTxt("<-- QUAY LAI", font, 22);
    bTxt.setFillColor(sf::Color::White);
    bTxt.setStyle(sf::Text::Bold);
    bTxt.setPosition(backX + 30.f, backY + 15.f);
    window.draw(bTxt);

    const float PREV_X = 620.f, PREV_Y = 150.f, PREV_W = 480.f, PREV_H = 550.f;
    sf::RectangleShape prevBox(sf::Vector2f(PREV_W, PREV_H));
    prevBox.setPosition(PREV_X, PREV_Y);
    prevBox.setFillColor(sf::Color(25, 25, 30));
    prevBox.setOutlineThickness(2.f);
    prevBox.setOutlineColor(GRID_COLOR);
    window.draw(prevBox);

    if (hoveredSlot != -1) {
        int tempBoard[30][30];
        int bSize, moves, turn;
        char sDate[32];
        char gName[64];

        if (GetSlotPreview(hoveredSlot, &bSize, &moves, &turn, sDate, gName, tempBoard)) {
            const float BOARD_AREA = 260.f;
            float mX = PREV_X + (PREV_W - BOARD_AREA) / 2.f;
            float mY = PREV_Y + 30.f;
            float mCellSz = BOARD_AREA / bSize;

            sf::RectangleShape mBg(sf::Vector2f(BOARD_AREA, BOARD_AREA));
            mBg.setPosition(mX, mY);
            mBg.setFillColor(BG_COLOR);
            mBg.setOutlineThickness(2.f);
            mBg.setOutlineColor(sf::Color::White);
            window.draw(mBg);

            for (int i = 1; i < bSize; ++i) {
                sf::RectangleShape vLine(sf::Vector2f(1.f, BOARD_AREA));
                vLine.setPosition(mX + i * mCellSz, mY);
                vLine.setFillColor(GRID_COLOR);
                window.draw(vLine);

                sf::RectangleShape hLine(sf::Vector2f(BOARD_AREA, 1.f));
                hLine.setPosition(mX, mY + i * mCellSz);
                hLine.setFillColor(GRID_COLOR);
                window.draw(hLine);
            }

            float thickness = std::max(1.f, mCellSz / 6.f);
            for (int x = 0; x < bSize; ++x) {
                for (int y = 0; y < bSize; ++y) {
                    float cX = mX + x * mCellSz + mCellSz / 2.f;
                    float cY = mY + y * mCellSz + mCellSz / 2.f;
                    float sz = mCellSz / 2.f - mCellSz / 6.f;

                    if (tempBoard[x][y] == 1) {
                        sf::RectangleShape l1(sf::Vector2f(mCellSz * 0.7f, thickness));
                        l1.setOrigin(mCellSz * 0.7f / 2.f, thickness / 2.f);
                        l1.setPosition(cX, cY);
                        l1.setFillColor(COLOR_X);
                        l1.rotate(45.f);
                        window.draw(l1);

                        sf::RectangleShape l2(sf::Vector2f(mCellSz * 0.7f, thickness));
                        l2.setOrigin(mCellSz * 0.7f / 2.f, thickness / 2.f);
                        l2.setPosition(cX, cY);
                        l2.setFillColor(COLOR_X);
                        l2.rotate(-45.f);
                        window.draw(l2);
                    }
                    else if (tempBoard[x][y] == 2) {
                        sf::CircleShape circle(sz);
                        circle.setOrigin(sz, sz);
                        circle.setPosition(cX, cY);
                        circle.setFillColor(sf::Color::Transparent);
                        circle.setOutlineThickness(thickness);
                        circle.setOutlineColor(COLOR_O);
                        window.draw(circle);
                    }
                }
            }

            float tY = mY + BOARD_AREA + 25.f;
            float tPadding = 25.f;

            sf::Text infoTitle("THONG TIN VAN GAME", font, 24);
            infoTitle.setFillColor(WIN_LINE_COLOR);
            infoTitle.setStyle(sf::Text::Bold | sf::Text::Underlined);
            infoTitle.setPosition(PREV_X + tPadding, tY);
            window.draw(infoTitle);

            std::string detailStr = " - Ten luu: " + std::string(gName) +
                "\n - Ngay luu: " + std::string(sDate) +
                "\n - Ban co: " + std::to_string(bSize) + "x" + std::to_string(bSize) +
                "\n - Da danh: " + std::to_string(moves) + " nuoc\n - Luot ke: " +
                (turn == 1 ? "Nguoi 1 (X)" : "Nguoi 2/May (O)");

            sf::Text infoTxt(detailStr, font, 20);
            infoTxt.setFillColor(sf::Color(220, 220, 220));
            infoTxt.setPosition(PREV_X + tPadding, tY + 45.f);
            infoTxt.setLineSpacing(1.4f);
            window.draw(infoTxt);
        }
        else {
            sf::Text empTxt("DIEM LUU TRONG", font, 28);
            empTxt.setFillColor(sf::Color(130, 130, 130));
            empTxt.setStyle(sf::Text::Bold);
            sf::FloatRect tr = empTxt.getLocalBounds();
            empTxt.setOrigin(tr.left + tr.width / 2.f, tr.top + tr.height / 2.f);
            empTxt.setPosition(PREV_X + PREV_W / 2.f, PREV_Y + PREV_H / 2.f);
            window.draw(empTxt);
        }
    }
    else {
        sf::Text hintTxt("<- DI CHUOT VAO SLOT\n   DE XEM TRUOC", font, 24);
        hintTxt.setFillColor(sf::Color(100, 100, 100));
        hintTxt.setLineSpacing(1.3f);
        sf::FloatRect tr = hintTxt.getLocalBounds();
        hintTxt.setOrigin(tr.left + tr.width / 2.f, tr.top + tr.height / 2.f);
        hintTxt.setPosition(PREV_X + PREV_W / 2.f, PREV_Y + PREV_H / 2.f);
        window.draw(hintTxt);
    }
}

// ============================================================
//  DrawSaveScreen
// ============================================================
void DrawSaveScreen(sf::RenderWindow& window, const sf::Font& font, bool isNaming, const std::string& inputName, sf::Clock& clock)
{
    sf::Text title("CHON O DE LUU VAN GAME", font, 45);
    title.setFillColor(WIN_LINE_COLOR);
    title.setStyle(sf::Text::Bold);
    title.setPosition(80.f, 50.f);
    window.draw(title);

    sf::Vector2i mp = sf::Mouse::getPosition(window);
    int hoveredSlot = -1;
    const float BTN_W = 480.f, BTN_H = 75.f, START_X = 80.f, START_Y = 150.f;

    for (int i = 1; i <= 5; ++i) {
        float bY = START_Y + (i - 1) * 95.f;
        bool hov = (mp.x >= START_X && mp.x <= START_X + BTN_W && mp.y >= bY && mp.y <= bY + BTN_H);

        if (hov && !isNaming) hoveredSlot = i;

        int size = 0, moves = 0, turn = 0;
        char gName[64] = "";
        bool hasData = PeekGameSlot(i, &size, &moves, &turn, gName);

        sf::RectangleShape btn(sf::Vector2f(BTN_W, BTN_H));
        btn.setPosition(START_X, bY);
        btn.setOutlineThickness(hov ? 3.f : 1.f);
        btn.setOutlineColor(hov ? COLOR_O : GRID_COLOR);
        btn.setFillColor(hov ? sf::Color(60, 60, 70) : BTN_COLOR);
        window.draw(btn);

        std::string slotText = (hasData && std::string(gName) != "") ? std::string(gName) : "Slot " + std::to_string(i);
        slotText += hasData ? " - [ GHI DE ]" : " - [ TRONG ]";

        sf::Text txt(slotText, font, 22);
        txt.setFillColor(hasData ? sf::Color(255, 200, 100) : sf::Color(130, 130, 130));
        txt.setPosition(START_X + 25.f, bY + BTN_H / 2.f - 15.f);
        window.draw(txt);
    }

    float backX = 80.f, backY = 680.f;
    bool backHov = (mp.x >= backX && mp.x <= backX + 200.f && mp.y >= backY && mp.y <= backY + 60.f);

    sf::RectangleShape backBtn(sf::Vector2f(200.f, 60.f));
    backBtn.setPosition(backX, backY);
    backBtn.setOutlineThickness(2.f);
    backBtn.setOutlineColor(GRID_COLOR);
    backBtn.setFillColor(backHov ? sf::Color(100, 50, 50) : BTN_COLOR);
    window.draw(backBtn);

    sf::Text bTxt("<-- QUAY LAI", font, 22);
    bTxt.setFillColor(sf::Color::White);
    bTxt.setStyle(sf::Text::Bold);
    bTxt.setPosition(backX + 30.f, backY + 15.f);
    window.draw(bTxt);

    const float PREV_X = 620.f, PREV_Y = 150.f, PREV_W = 480.f, PREV_H = 550.f;

    sf::RectangleShape prevBox(sf::Vector2f(PREV_W, PREV_H));
    prevBox.setPosition(PREV_X, PREV_Y);
    prevBox.setFillColor(sf::Color(25, 25, 30));
    prevBox.setOutlineThickness(2.f);
    prevBox.setOutlineColor(GRID_COLOR);
    window.draw(prevBox);

    if (hoveredSlot != -1 && !isNaming) {
        int tempBoard[30][30];
        int bSize, moves, turn;
        char sDate[32];
        char gName[64];

        if (GetSlotPreview(hoveredSlot, &bSize, &moves, &turn, sDate, gName, tempBoard)) {
            const float BOARD_AREA = 260.f;
            float mX = PREV_X + (PREV_W - BOARD_AREA) / 2.f;
            float mY = PREV_Y + 30.f;
            float mCellSz = BOARD_AREA / bSize;

            sf::RectangleShape mBg(sf::Vector2f(BOARD_AREA, BOARD_AREA));
            mBg.setPosition(mX, mY);
            mBg.setFillColor(BG_COLOR);
            mBg.setOutlineThickness(2.f);
            mBg.setOutlineColor(sf::Color::White);
            window.draw(mBg);

            for (int i = 1; i < bSize; ++i) {
                sf::RectangleShape vLine(sf::Vector2f(1.f, BOARD_AREA));
                vLine.setPosition(mX + i * mCellSz, mY);
                vLine.setFillColor(GRID_COLOR);
                window.draw(vLine);

                sf::RectangleShape hLine(sf::Vector2f(BOARD_AREA, 1.f));
                hLine.setPosition(mX, mY + i * mCellSz);
                hLine.setFillColor(GRID_COLOR);
                window.draw(hLine);
            }

            float thickness = std::max(1.f, mCellSz / 6.f);
            for (int x = 0; x < bSize; ++x) {
                for (int y = 0; y < bSize; ++y) {
                    float cX = mX + x * mCellSz + mCellSz / 2.f;
                    float cY = mY + y * mCellSz + mCellSz / 2.f;
                    float sz = mCellSz / 2.f - mCellSz / 6.f;

                    if (tempBoard[x][y] == 1) {
                        sf::RectangleShape l1(sf::Vector2f(mCellSz * 0.7f, thickness));
                        l1.setOrigin(mCellSz * 0.7f / 2.f, thickness / 2.f);
                        l1.setPosition(cX, cY);
                        l1.setFillColor(COLOR_X);
                        l1.rotate(45.f);
                        window.draw(l1);

                        sf::RectangleShape l2(sf::Vector2f(mCellSz * 0.7f, thickness));
                        l2.setOrigin(mCellSz * 0.7f / 2.f, thickness / 2.f);
                        l2.setPosition(cX, cY);
                        l2.setFillColor(COLOR_X);
                        l2.rotate(-45.f);
                        window.draw(l2);
                    }
                    else if (tempBoard[x][y] == 2) {
                        sf::CircleShape circle(sz);
                        circle.setOrigin(sz, sz);
                        circle.setPosition(cX, cY);
                        circle.setFillColor(sf::Color::Transparent);
                        circle.setOutlineThickness(thickness);
                        circle.setOutlineColor(COLOR_O);
                        window.draw(circle);
                    }
                }
            }

            float tY = mY + BOARD_AREA + 25.f;
            float tPadding = 25.f;

            sf::Text infoTitle("THONG TIN VAN GAME CU:", font, 24);
            infoTitle.setFillColor(sf::Color(255, 100, 100));
            infoTitle.setStyle(sf::Text::Bold | sf::Text::Underlined);
            infoTitle.setPosition(PREV_X + tPadding, tY);
            window.draw(infoTitle);

            std::string detailStr = " - Ngay luu: " + std::string(sDate) +
                "\n - Ban co: " + std::to_string(bSize) + "x" + std::to_string(bSize) +
                "\n - Da danh: " + std::to_string(moves) + " nuoc\n\n CHU Y: LUU VAO DAY SE GHI DE!";

            sf::Text infoTxt(detailStr, font, 20);
            infoTxt.setFillColor(sf::Color(220, 220, 220));
            infoTxt.setPosition(PREV_X + tPadding, tY + 45.f);
            infoTxt.setLineSpacing(1.4f);
            window.draw(infoTxt);
        }
        else {
            sf::Text empTxt("O LUU TRONG\n\nSAN SANG LUU GAME!", font, 26);
            empTxt.setFillColor(COLOR_O);
            empTxt.setStyle(sf::Text::Bold);
            empTxt.setLineSpacing(1.5f);
            sf::FloatRect tr = empTxt.getLocalBounds();
            empTxt.setOrigin(tr.left + tr.width / 2.f, tr.top + tr.height / 2.f);
            empTxt.setPosition(PREV_X + PREV_W / 2.f, PREV_Y + PREV_H / 2.f);
            window.draw(empTxt);
        }
    }
    else if (!isNaming) {
        sf::Text hintTxt("<- CHON 1 O DE LUU VAN GAME", font, 24);
        hintTxt.setFillColor(sf::Color(100, 100, 100));
        sf::FloatRect tr = hintTxt.getLocalBounds();
        hintTxt.setOrigin(tr.left + tr.width / 2.f, tr.top + tr.height / 2.f);
        hintTxt.setPosition(PREV_X + PREV_W / 2.f, PREV_Y + PREV_H / 2.f);
        window.draw(hintTxt);
    }

    if (isNaming) {
        sf::RectangleShape overlay(sf::Vector2f(Config::WIN_WIDTH, Config::WIN_HEIGHT));
        overlay.setFillColor(sf::Color(0, 0, 0, 180));
        window.draw(overlay);

        sf::RectangleShape box(sf::Vector2f(500.f, 250.f));
        box.setOrigin(250.f, 125.f);
        box.setPosition(Config::WIN_WIDTH / 2.f, Config::WIN_HEIGHT / 2.f);
        box.setFillColor(BTN_COLOR);
        box.setOutlineThickness(3.f);
        box.setOutlineColor(COLOR_O);
        window.draw(box);

        sf::Text prompt("NHAP TEN VAN GAME", font, 24);
        prompt.setFillColor(sf::Color::White);
        prompt.setOrigin(prompt.getLocalBounds().width / 2, 0);
        prompt.setPosition(Config::WIN_WIDTH / 2.f, Config::WIN_HEIGHT / 2.f - 80.f);
        window.draw(prompt);

        sf::RectangleShape inputRect(sf::Vector2f(400.f, 50.f));
        inputRect.setOrigin(200.f, 25.f);
        inputRect.setPosition(Config::WIN_WIDTH / 2.f, Config::WIN_HEIGHT / 2.f - 10.f);
        inputRect.setFillColor(sf::Color::Black);
        window.draw(inputRect);

        sf::Text inputText(inputName, font, 22);
        inputText.setFillColor(COLOR_O);
        sf::FloatRect textBounds = inputText.getLocalBounds();
        inputText.setOrigin(textBounds.left + textBounds.width / 2.f, textBounds.top + textBounds.height / 2.f);
        inputText.setPosition(Config::WIN_WIDTH / 2.f, Config::WIN_HEIGHT / 2.f - 10.f);
        window.draw(inputText);

        // 2. Tạo đồng hồ chạy ngầm ĐỘC LẬP (không bị ảnh hưởng bởi main.cpp)
        static sf::Clock blinkClock;
        static size_t lastLength = inputName.length();

        // UX Game xịn: Hễ cứ gõ phím (chuỗi dài ra/ngắn đi) là con trỏ phải HIỆN LÊN ngay lập tức
        if (inputName.length() != lastLength) {
            blinkClock.restart(); // Reset đồng hồ nhấp nháy
            lastLength = inputName.length();
        }

        // Nhấp nháy: 500ms hiện, 500ms ẩn (Tổng chu kỳ 1000ms)
        if (blinkClock.getElapsedTime().asMilliseconds() % 1000 < 500) {
            sf::RectangleShape cursor(sf::Vector2f(2.f, 24.f)); // Thanh dọc |
            cursor.setFillColor(COLOR_O);

            // Tính tọa độ vệt sáng nằm ngay sát đít chữ
            float cursorX = (Config::WIN_WIDTH / 2.f) + (inputName.empty() ? 0 : textBounds.width / 2.f + 4.f);

            cursor.setOrigin(1.f, 12.f);
            cursor.setPosition(cursorX, Config::WIN_HEIGHT / 2.f - 10.f);
            window.draw(cursor);
        }

        sf::RectangleShape okBtn(sf::Vector2f(200.f, 50.f));
        okBtn.setOrigin(100.f, 25.f);
        okBtn.setPosition(Config::WIN_WIDTH / 2.f, Config::WIN_HEIGHT / 2.f + 60.f);
        okBtn.setFillColor(sf::Color(50, 150, 50));
        window.draw(okBtn);

        sf::Text okText("CHAP NHAN", font, 20);
        okText.setFillColor(sf::Color::White);
        okText.setOrigin(okText.getLocalBounds().width / 2, okText.getLocalBounds().height / 2);
        okText.setPosition(Config::WIN_WIDTH / 2.f, Config::WIN_HEIGHT / 2.f + 55.f);
        window.draw(okText);
    }
}