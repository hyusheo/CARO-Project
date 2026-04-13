#include "RenderUI.h"
#include "Constants.h"
#include "CaroAPI.h"
#include <cmath>
#include <string>

// ============================================================
//  Helper: tọa độ X để panel luôn NẰM GIỮA khoảng trống giữa viền phải bàn cờ và viền phải cửa sổ.
// ============================================================
static float PanelX(int boardSize)
{
    int   cellSz = GetDynCellSize(boardSize);
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
    const float BTN_W = 350.0f, BTN_H = 60.0f, START_Y = 300.0f;
    sf::Vector2i mp = sf::Mouse::getPosition(window);

    for (int i = 0; i < 5; ++i) {
        float bX = Config::WIN_WIDTH / 2.0f - BTN_W / 2.0f;
        float bY = START_Y + i * 80.0f;
        bool  hov = (mp.x >= bX && mp.x <= bX + BTN_W && mp.y >= bY && mp.y <= bY + BTN_H);

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
void DrawInGamePanel(sf::RenderWindow& window, const sf::Font& font,
    float timeRemaining, bool isPlayerTurn,
    int gameStatus, int boardSize, GameMode gameMode,
    int undoLeft[2])
{
    const float pX = PanelX(boardSize);
    const float BTN_W = static_cast<float>(Config::PANEL_W);
    const float BTN_H = 52.0f;

    // Đường kẻ dọc phân cách
    int cellSz = GetDynCellSize(boardSize);
    float boardRight = static_cast<float>(Config::OFFSET_X + boardSize * cellSz);
    sf::RectangleShape divider(sf::Vector2f(2.f,
        static_cast<float>(Config::WIN_HEIGHT - 2 * Config::OFFSET_Y)));
    divider.setPosition(boardRight + Config::PANEL_GAP / 2.f,
        static_cast<float>(Config::OFFSET_Y));
    divider.setFillColor(GRID_COLOR);
    window.draw(divider);

    // 1. Lượt đi
    sf::Text turnLabel("LUOT DI:", font, 18);
    turnLabel.setFillColor(sf::Color(180, 180, 180));
    turnLabel.setPosition(pX, 60.f);
    window.draw(turnLabel);

    // Tên người chơi: PVP → "NGUOI 1 (X)" / "NGUOI 2 (O)"
    //                 PVE → "NGUOI (X)"   / "MAY (O)"
    std::string turnStr;
    sf::Color   turnColor;
    if (gameMode == GameMode::PVP) {
        if (isPlayerTurn) 
        { 
            turnStr = "NGUOI 1 (X)"; 
            turnColor = COLOR_X; 
        }
        else 
        { 
            turnStr = "NGUOI 2 (O)"; 
            turnColor = COLOR_O; 
        }
    }
    else {
        if (isPlayerTurn) 
        { 
            turnStr = "NGUOI (X)";  
            turnColor = COLOR_X; 
        }
        else 
        { 
            turnStr = "MAY (O)";     
            turnColor = COLOR_O; 
        }
    }
    sf::Text turnText(turnStr, font, 26);
    turnText.setFillColor(turnColor);
    turnText.setStyle(sf::Text::Bold);
    turnText.setPosition(pX, 85.f);
    window.draw(turnText);

    // 2. Đồng hồ
    sf::Text timerLabel("THOI GIAN CON:", font, 18);
    timerLabel.setFillColor(sf::Color(180, 180, 180));
    timerLabel.setPosition(pX, 135.f);
    window.draw(timerLabel);

    sf::Color timerColor; 
    if (timeRemaining > 20.f)
    {
        timerColor = sf::Color(80, 255, 160); 
    }
    else if (timeRemaining > 10.f)
    {
        timerColor = sf::Color(255, 200, 60); 
    }
    else
    {
        timerColor = sf::Color(255, 60, 60);
    }

    sf::Text timerText(std::to_string((int)timeRemaining) + " giay", font, 34);
    timerText.setFillColor(timerColor);
    timerText.setStyle(sf::Text::Bold);
    timerText.setPosition(pX, 158.f);
    window.draw(timerText);

    // Progress bar
    sf::RectangleShape barBg(sf::Vector2f(BTN_W, 8.f));
    barBg.setPosition(pX, 200.f);
    barBg.setFillColor(sf::Color(50, 50, 60));
    window.draw(barBg);

    float ratio = std::max(0.f, std::min(1.f, timeRemaining / 60.f));
    sf::RectangleShape barFg(sf::Vector2f(BTN_W * ratio, 8.f));
    barFg.setPosition(pX, 200.f);
    barFg.setFillColor(timerColor);
    window.draw(barFg);

    // 3. Kết quả
    if (gameStatus != 0) {
        sf::RectangleShape rBox(sf::Vector2f(BTN_W, 60.f));
        rBox.setPosition(pX, 220.f);
        rBox.setFillColor(sf::Color(50, 50, 20));
        rBox.setOutlineThickness(2); 
        rBox.setOutlineColor(WIN_LINE_COLOR);
        window.draw(rBox);

        std::string s;
        if (gameMode == GameMode::PVP) 
        {
            if (gameStatus == 1) 
            {
                s = "NGUOI 1 THANG!";
            }
            else if (gameStatus == 2) 
            {
                s = "NGUOI 2 THANG!";
            }
            else
            {
                s = "HOA!"; 
            }
        }
        else 
        {
            if (gameStatus == 1)
            {
                s = "NGUOI THANG!";
            }
            else if (gameStatus == 2)
            {
                s = "MAY THANG!";
            }
            else {
                s = "HOA!";
            }
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
        if (gameMode == GameMode::PVP)
        {
            if (isPlayerTurn) 
            {
                hintStr = "Luot Nguoi 1 - bam vao ban co";
            }
            else
            {
                hintStr = "Luot Nguoi 2 - bam vao ban co";
            }
        }
        else
        {
            if (isPlayerTurn) 
            {
                hintStr = "Click vao ban co de di";
            }
            else 
            {
                hintStr = "Dang doi AI suy nghi...";
            }
        } 
        sf::Text hint(hintStr, font, 15);
        hint.setFillColor(sf::Color(120, 120, 140));
        hint.setPosition(pX, 220.f);
        window.draw(hint);
    }

    // 4. Hiển thị số undo còn lại (chỉ trong PVP)
    if (gameMode == GameMode::PVP)
    {
        // Tiêu đề
        sf::Text undoTitle("LUOT UNDO CON LAI:", font, 17);
        undoTitle.setFillColor(sf::Color(180, 180, 180));
        undoTitle.setPosition(pX, 295.f);
        window.draw(undoTitle);

        // Thông tin của từng người — mỗi người 1 dòng
        struct PlayerUndoInfo {
            std::string label;
            int         left;
            sf::Color   color;
            float       posY;
        } info[2] = {
            { "Nguoi 1 (X):", undoLeft[0], COLOR_X, 320.f },
            { "Nguoi 2 (O):", undoLeft[1], COLOR_O, 350.f }
        };

        for (int p = 0; p < 2; ++p)
        {
            // Tên người chơi
            sf::Text lblTxt(info[p].label, font, 18);
            lblTxt.setFillColor(info[p].color);
            lblTxt.setPosition(pX, info[p].posY);
            window.draw(lblTxt);

            // Vẽ ô tròn cho từng lượt undo (đầy = còn, rỗng = đã dùng)
            const float CIRCLE_R = 8.f;
            const float CIRCLE_GAP = 22.f;
            float startX = pX + 130.f;

            for (int k = 0; k < Config::UNDO_MAX; ++k)
            {
                sf::CircleShape dot(CIRCLE_R);
                dot.setPosition(startX + k * CIRCLE_GAP,
                    info[p].posY + 1.f);

                if (k < info[p].left) 
                {
                    // Còn lượt: hình tròn đặc màu người chơi
                    dot.setFillColor(info[p].color);
                    dot.setOutlineThickness(0);
                }
                else 
                {
                    // Đã dùng: chỉ viền, bên trong tối
                    dot.setFillColor(sf::Color(30, 30, 40));
                    dot.setOutlineThickness(1.5f);
                    dot.setOutlineColor(sf::Color(80, 80, 90));
                }
                window.draw(dot);
            }
        }

        // Đường kẻ ngang phân cách nhỏ
        sf::RectangleShape sep(sf::Vector2f(BTN_W, 1.f));
        sep.setPosition(pX, 382.f);
        sep.setFillColor(sf::Color(60, 60, 70));
        window.draw(sep);
    }

    // 5. Nút chức năng (Undo / Save / Main Menu)
    // Với PVP đẩy nút xuống thêm nếu đã vẽ undo info
    const float BTN_START_Y = (gameMode == GameMode::PVP) ? 395.f : 420.f;
    const char* gameBtns[] = { "Undo", "Save Game", "Main Menu" };
    sf::Vector2i mp = sf::Mouse::getPosition(window);

    for (int i = 0; i < 3; ++i) {
        float bX = pX;
        float bY = BTN_START_Y + i * 80.f;
        bool  hov = (mp.x >= bX && mp.x <= bX + BTN_W &&
            mp.y >= bY && mp.y <= bY + BTN_H);

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
}

// ============================================================
//  DrawBoard
// ============================================================
void DrawBoard(sf::RenderWindow& window, int boardSize)
{
    int cellSz = GetDynCellSize(boardSize);
    for (int i = 0; i <= boardSize; ++i) 
    {
        sf::RectangleShape h(sf::Vector2f(static_cast<float>(boardSize * cellSz), 1.f));
        h.setPosition(static_cast<float>(Config::OFFSET_X),
            static_cast<float>(Config::OFFSET_Y + i * cellSz));
        h.setFillColor(GRID_COLOR); 
        window.draw(h);

        sf::RectangleShape v(sf::Vector2f(1.f, static_cast<float>(boardSize * cellSz)));
        v.setPosition(static_cast<float>(Config::OFFSET_X + i * cellSz),
            static_cast<float>(Config::OFFSET_Y));
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
    for (int x = 0; x < boardSize; ++x) 
    {
        for (int y = 0; y < boardSize; ++y) 
        {
            int   cell = GetCell(x, y);
            float px = static_cast<float>(Config::OFFSET_X + x * cellSz);
            float py = static_cast<float>(Config::OFFSET_Y + y * cellSz);

            if (cell == 1) 
            {
                sf::RectangleShape ln(sf::Vector2f(cellSz - 6.f, 2.5f));
                ln.setFillColor(COLOR_X);
                ln.setOrigin((cellSz - 6.f) / 2.f, 1.25f);
                ln.setPosition(px + cellSz / 2.f, py + cellSz / 2.f);
                ln.rotate(45); 
                window.draw(ln);
                sf::RectangleShape ln2 = ln; ln2.rotate(90); 
                window.draw(ln2);
            }
            else if (cell == 2) 
            {
                float r = cellSz / 2.f - 3.f;
                sf::CircleShape c(r);
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
    if (gX >= 0 && gX < boardSize && gY >= 0 && gY < boardSize && GetCell(gX, gY) == 0) 
    {
        sf::RectangleShape hr(sf::Vector2f(cellSz - 2.f, cellSz - 2.f));
        hr.setPosition(static_cast<float>(Config::OFFSET_X + gX * cellSz + 1),
            static_cast<float>(Config::OFFSET_Y + gY * cellSz + 1));
        hr.setFillColor(HOVER_COLOR);
        window.draw(hr);
    }
}

// ============================================================
//  DrawWinLine (2 overload: không boardSize và có boardSize)
// ============================================================
void DrawWinLine(sf::RenderWindow& window,
    int sX, int sY, int eX, int eY)
{
    DrawWinLine(window, sX, sY, eX, eY, 15); // fallback board 15
}

void DrawWinLine(sf::RenderWindow& window,
    int sX, int sY, int eX, int eY, int boardSize)
{
    if (sX == -1)
    {
        return;
    }
    int   cs = GetDynCellSize(boardSize);
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
void DrawSettings(sf::RenderWindow& window, const sf::Font& font,
    int boardSize, bool ruleBlock2, int aiLevel,
    float sfxVolume, bool bgmEnabled)
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

    const float SY = 200.f;
    const float LX = 250.f; 
    const float CX = 650.f;
    const float RG = 80.f;
    sf::Vector2i mp = sf::Mouse::getPosition(window);

    for (int i = 0; i < 5; ++i) {
        float cy = SY + i * RG;
        sf::Text lbl(labels[i], font, 28);
        lbl.setFillColor(sf::Color::White); 
        lbl.setPosition(LX, cy);
        window.draw(lbl);

        if (i == 0 || i == 2 || i == 3) 
        {
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

    const float BW = 300.f;
    const float BH = 60.f;
    const float BX = Config::WIN_WIDTH / 2.f - BW / 2.f, BY = 650.f;

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