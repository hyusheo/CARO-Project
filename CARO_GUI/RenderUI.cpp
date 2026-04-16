/*
 * RenderUI.cpp  ─  Cyberpunk Edition
 * Theme: Dark neon, cyan/magenta neon, grid scanlines, corner brackets,
 *        glitch-style decorations, Orbitron-esque capital text layout.
 *
 * Requires SFML 2.6.x  +  Constants.h / CaroAPI.h unchanged.
 * Drop-in replacement for the original RenderUI.cpp.
 */

#include "RenderUI.h"
#include "Constants.h"
#include "CaroAPI.h"
#include <cmath>
#include <string>
#include <sstream>
#include <iomanip>

// ============================================================
//  CYBERPUNK COLOUR PALETTE  (local to this file)
// ============================================================
namespace Cyber {
    // Primary accent – electric cyan
    const sf::Color Cyan    { 0,  255, 255, 255 };
    const sf::Color CyanDim { 0,  180, 200, 180 };
    const sf::Color CyanGlow{ 0,  255, 255,  40 };

    // Secondary accent – hot magenta
    const sf::Color Magenta { 255,  0, 200, 255 };
    const sf::Color MagDim  { 200,  0, 160, 180 };

    // Warning / win – electric yellow
    const sf::Color Yellow  { 255, 220,  0, 255 };
    const sf::Color YellowD { 200, 170,  0, 200 };

    // Danger / X-piece – neon red-orange
    const sf::Color NeonRed { 255,  50,  80, 255 };

    // Background layers
    const sf::Color BgDeep  {  8,   8,  14, 255 };
    const sf::Color BgPanel {  12,  16,  28, 230 };
    const sf::Color BgBtn   {  18,  22,  38, 255 };
    const sf::Color BgHover {  30,  40,  70, 255 };

    // Grid / border
    const sf::Color Grid    {  40,  55,  80, 255 };
    const sf::Color GridDim {  25,  35,  55, 255 };
    const sf::Color White   { 220, 230, 255, 255 };
    const sf::Color Gray    { 100, 115, 145, 255 };
}

// ============================================================
//  Internal helpers
// ============================================================
static float PanelX(int boardSize)
{
    int   cs = GetDynCellSize(boardSize);
    float boardRight = static_cast<float>(Config::OFFSET_X + boardSize * cs);
    float gapWidth   = static_cast<float>(Config::WIN_WIDTH) - boardRight;
    return boardRight + (gapWidth - static_cast<float>(Config::PANEL_W)) / 2.0f;
}

// ── Corner bracket decoration (top-left & bottom-right) ─────
static void DrawCornerBrackets(sf::RenderWindow& window,
    float x, float y, float w, float h,
    sf::Color col, float arm = 14.f, float thick = 2.f)
{
    auto seg = [&](float ax, float ay, float bx, float by) {
        float dx = bx - ax, dy = by - ay;
        float len = std::sqrt(dx * dx + dy * dy);
        float ang = std::atan2(dy, dx) * 180.f / 3.14159265f;
        sf::RectangleShape s({ len, thick });
        s.setOrigin(0, thick / 2.f);
        s.setPosition(ax, ay);
        s.setRotation(ang);
        s.setFillColor(col);
        window.draw(s);
    };
    // top-left
    seg(x, y + arm, x, y);  seg(x, y, x + arm, y);
    // top-right
    seg(x + w - arm, y, x + w, y);  seg(x + w, y, x + w, y + arm);
    // bottom-left
    seg(x, y + h - arm, x, y + h);  seg(x, y + h, x + arm, y + h);
    // bottom-right
    seg(x + w - arm, y + h, x + w, y + h);  seg(x + w, y + h, x + w, y + h - arm);
}

// ── Neon-bordered rectangle ──────────────────────────────────
static void DrawNeonRect(sf::RenderWindow& window,
    float x, float y, float w, float h,
    sf::Color fill, sf::Color border, float thickness = 1.5f)
{
    sf::RectangleShape r({ w, h });
    r.setPosition(x, y);
    r.setFillColor(fill);
    r.setOutlineThickness(thickness);
    r.setOutlineColor(border);
    window.draw(r);
}

// ── Centred text helper ──────────────────────────────────────
static void DrawCentredText(sf::RenderWindow& window, const sf::Font& font,
    const std::string& str, unsigned size, sf::Color col,
    float cx, float cy)
{
    sf::Text t(str, font, size);
    t.setFillColor(col);
    sf::FloatRect b = t.getLocalBounds();
    t.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
    t.setPosition(cx, cy);
    window.draw(t);
}

// ── Scanline overlay ─────────────────────────────────────────
static void DrawScanlines(sf::RenderWindow& window, float x, float y,
    float w, float h, sf::Color lineCol)
{
    for (float ly = y; ly < y + h; ly += 4.f) {
        sf::RectangleShape sl({ w, 1.f });
        sl.setPosition(x, ly);
        sl.setFillColor(lineCol);
        window.draw(sl);
    }
}

// ── Glitch accent stripe (thin diagonal-ish lines) ──────────
static void DrawGlitchStripes(sf::RenderWindow& window,
    float x, float y, float w, float h, sf::Color col)
{
    for (int i = 0; i < 3; ++i) {
        float oy = y + h * 0.25f * (i + 1);
        sf::RectangleShape s({ w * 0.4f, 1.5f });
        s.setPosition(x + w * 0.05f + i * 10.f, oy);
        s.setFillColor(col);
        window.draw(s);
    }
}

// ── Dot-matrix progress dots ─────────────────────────────────
static void DrawDots(sf::RenderWindow& window,
    float startX, float y, int total, int filled,
    sf::Color onCol, float r = 7.f, float gap = 20.f)
{
    for (int k = 0; k < total; ++k) {
        sf::CircleShape dot(r);
        dot.setOrigin(r, r);
        dot.setPosition(startX + k * gap, y);
        if (k < filled) {
            dot.setFillColor(onCol);
            dot.setOutlineThickness(0);
        } else {
            dot.setFillColor(sf::Color(20, 25, 40));
            dot.setOutlineThickness(1.5f);
            dot.setOutlineColor(sf::Color(50, 60, 90));
        }
        window.draw(dot);
    }
}

// ── Timer bar ────────────────────────────────────────────────
static void DrawTimerBar(sf::RenderWindow& window,
    float x, float y, float w, float h,
    float ratio, sf::Color barCol)
{
    // track
    DrawNeonRect(window, x, y, w, h, sf::Color(15, 20, 35), Cyber::Grid, 1.f);
    // fill
    if (ratio > 0.f) {
        DrawNeonRect(window, x, y, w * ratio, h,
            sf::Color(barCol.r, barCol.g, barCol.b, 200), sf::Color::Transparent, 0);
        // bright leading edge
        DrawNeonRect(window, x + w * ratio - 3.f, y, 3.f, h,
            barCol, sf::Color::Transparent, 0);
    }
}

// ── Section header with horizontal rule ─────────────────────
static void DrawSectionHeader(sf::RenderWindow& window, const sf::Font& font,
    const std::string& label, float x, float y, float w, sf::Color col)
{
    sf::Text t(label, font, 13);
    t.setFillColor(col);
    t.setPosition(x, y);
    window.draw(t);
    float tw = t.getLocalBounds().width + 8.f;
    sf::RectangleShape line({ w - tw, 1.f });
    line.setPosition(x + tw, y + 9.f);
    line.setFillColor(sf::Color(col.r, col.g, col.b, 80));
    window.draw(line);
}

// ============================================================
//  DrawMenu  ─  Cyberpunk Main Menu
// ============================================================
void DrawMenu(sf::RenderWindow& window, const sf::Font& font)
{
    float W = static_cast<float>(Config::WIN_WIDTH);
    float H = static_cast<float>(Config::WIN_HEIGHT);

    // ── Background grid ──────────────────────────────────────
    for (float gx = 0; gx < W; gx += 60.f) {
        sf::RectangleShape vl({ 1.f, H });
        vl.setPosition(gx, 0);
        vl.setFillColor(sf::Color(0, 255, 255, 8));
        window.draw(vl);
    }
    for (float gy = 0; gy < H; gy += 60.f) {
        sf::RectangleShape hl({ W, 1.f });
        hl.setPosition(0, gy);
        hl.setFillColor(sf::Color(0, 255, 255, 8));
        window.draw(hl);
    }

    // ── Scanline overlay full screen ─────────────────────────
    DrawScanlines(window, 0, 0, W, H, sf::Color(0, 0, 0, 18));

    // ── Decorative horizontal lines ──────────────────────────
    for (float lx = 0; lx < W; lx += 1.f) {
        // top accent
        sf::RectangleShape t1({ W, 2.f }); t1.setPosition(0, 0);
        t1.setFillColor(Cyber::Cyan); window.draw(t1);
        // bottom accent
        sf::RectangleShape t2({ W, 2.f }); t2.setPosition(0, H - 2.f);
        t2.setFillColor(Cyber::Magenta); window.draw(t2);
        break;
    }

    // ── Title block ──────────────────────────────────────────
    {
        // shadow / glow pass
        sf::Text shadow("CARO.EXE", font, 78);
        shadow.setFillColor(sf::Color(0, 255, 255, 30));
        sf::FloatRect sr = shadow.getLocalBounds();
        shadow.setOrigin(sr.left + sr.width / 2.f, sr.top + sr.height / 2.f);
        shadow.setPosition(W / 2.f + 3.f, 120.f + 3.f);
        window.draw(shadow);

        sf::Text title("CARO.EXE", font, 78);
        title.setFillColor(Cyber::Cyan);
        title.setStyle(sf::Text::Bold);
        sf::FloatRect r = title.getLocalBounds();
        title.setOrigin(r.left + r.width / 2.f, r.top + r.height / 2.f);
        title.setPosition(W / 2.f, 120.f);
        window.draw(title);

        // subtitle
        sf::Text sub("[ CYBERPUNK NEURAL BATTLE v2.6 ]", font, 16);
        sub.setFillColor(Cyber::MagDim);
        sf::FloatRect sb = sub.getLocalBounds();
        sub.setOrigin(sb.left + sb.width / 2.f, sb.top + sb.height / 2.f);
        sub.setPosition(W / 2.f, 180.f);
        window.draw(sub);

        // decorative separator
        sf::RectangleShape sep({ 400.f, 1.5f });
        sep.setPosition(W / 2.f - 200.f, 202.f);
        sep.setFillColor(Cyber::Cyan);
        window.draw(sep);
    }

    // ── Menu buttons ─────────────────────────────────────────
    const char* items[] = {
        "PVP  >>  2 NGUOI CHOI",
        "PVE  >>  CHOI VOI MAY",
        "CAI DAT  //  SETTINGS",
        "TAI GAME  //  LOAD SAVE",
        "THOAT  //  EXIT.EXE"
    };
    const sf::Color btnBorder[] = {
        Cyber::Cyan, Cyber::Magenta, Cyber::CyanDim,
        Cyber::CyanDim, sf::Color(180, 30, 60, 200)
    };

    const float BTN_W = 380.f, BTN_H = 58.f, START_Y = 240.f;
    sf::Vector2i mp = sf::Mouse::getPosition(window);

    for (int i = 0; i < 5; ++i) {
        float bX = W / 2.f - BTN_W / 2.f;
        float bY = START_Y + i * 76.f;
        bool  hov = (mp.x >= bX && mp.x <= bX + BTN_W &&
                     mp.y >= bY && mp.y <= bY + BTN_H);

        // panel fill
        DrawNeonRect(window, bX, bY, BTN_W, BTN_H,
            hov ? sf::Color(20, 40, 70, 230) : Cyber::BgBtn,
            hov ? btnBorder[i] : sf::Color(btnBorder[i].r, btnBorder[i].g, btnBorder[i].b, 80),
            hov ? 2.f : 1.f);

        // corner brackets on hover
        if (hov)
            DrawCornerBrackets(window, bX, bY, BTN_W, BTN_H,
                btnBorder[i], 10.f, 2.f);

        // index prefix
        std::string idx = "0" + std::to_string(i + 1) + "  ";
        sf::Text prefix(idx, font, 14);
        prefix.setFillColor(sf::Color(btnBorder[i].r, btnBorder[i].g,
            btnBorder[i].b, hov ? 255 : 120));
        prefix.setPosition(bX + 14.f, bY + BTN_H / 2.f - 9.f);
        window.draw(prefix);

        // main label
        sf::Text txt(items[i], font, 20);
        txt.setFillColor(hov ? Cyber::White : sf::Color(160, 175, 200));
        txt.setPosition(bX + 50.f, bY + BTN_H / 2.f - 11.f);
        window.draw(txt);

        // right arrow on hover
        if (hov) {
            sf::Text arr(">>", font, 16);
            arr.setFillColor(btnBorder[i]);
            arr.setPosition(bX + BTN_W - 32.f, bY + BTN_H / 2.f - 9.f);
            window.draw(arr);
        }
    }

    // ── Footer ───────────────────────────────────────────────
    sf::Text footer("SFML 2.6.2  |  CARO ENGINE  |  CYBERPUNK UI MOD", font, 13);
    footer.setFillColor(sf::Color(60, 80, 110, 200));
    sf::FloatRect fr = footer.getLocalBounds();
    footer.setOrigin(fr.left + fr.width / 2.f, fr.top + fr.height / 2.f);
    footer.setPosition(W / 2.f, H - 20.f);
    window.draw(footer);
}

// ============================================================
//  DrawInGamePanel  ─  Cyberpunk HUD
// ============================================================
void DrawInGamePanel(sf::RenderWindow& window, const sf::Font& font,
    float timeRemaining, bool isPlayerTurn,
    int gameStatus, int boardSize, GameMode gameMode,
    int undoLeft[2])
{
    const float pX   = PanelX(boardSize);
    const float pW   = static_cast<float>(Config::PANEL_W);
    const float pH   = static_cast<float>(Config::WIN_HEIGHT
                       - 2 * Config::OFFSET_Y);
    const float pY   = static_cast<float>(Config::OFFSET_Y);

    // ── Panel background + border ────────────────────────────
    DrawNeonRect(window, pX - 12.f, pY, pW + 24.f, pH,
        Cyber::BgPanel, Cyber::Grid, 1.f);
    DrawCornerBrackets(window, pX - 12.f, pY, pW + 24.f, pH,
        Cyber::CyanDim, 16.f, 1.5f);
    DrawScanlines(window, pX - 12.f, pY, pW + 24.f, pH,
        sf::Color(0, 0, 0, 12));

    // ── Vertical divider ─────────────────────────────────────
    {
        int   cs = GetDynCellSize(boardSize);
        float bR = static_cast<float>(Config::OFFSET_X + boardSize * cs);
        sf::RectangleShape div({ 1.5f, pH });
        div.setPosition(bR + Config::PANEL_GAP / 2.f, pY);
        div.setFillColor(Cyber::Grid);
        window.draw(div);
    }

    // ── SYSTEM STATUS header ─────────────────────────────────
    {
        sf::Text hdr("[ SYSTEM STATUS ]", font, 13);
        hdr.setFillColor(Cyber::CyanDim);
        sf::FloatRect hr = hdr.getLocalBounds();
        hdr.setOrigin(hr.left + hr.width / 2.f, 0);
        hdr.setPosition(pX + pW / 2.f, pY + 8.f);
        window.draw(hdr);

        sf::RectangleShape hl({ pW, 1.f });
        hl.setPosition(pX, pY + 28.f);
        hl.setFillColor(sf::Color(0, 200, 220, 60));
        window.draw(hl);
    }

    float cursor = pY + 40.f;

    // ── Current turn block ───────────────────────────────────
    DrawSectionHeader(window, font, "// LUOT DI", pX, cursor, pW, Cyber::Gray);
    cursor += 22.f;

    std::string turnStr;
    sf::Color   turnCol;
    if (gameMode == PVP) {
        if (isPlayerTurn) { turnStr = "PLAYER 01  [ X ]"; turnCol = Cyber::Cyan; }
        else              { turnStr = "PLAYER 02  [ O ]"; turnCol = Cyber::Magenta; }
    } else {
        if (isPlayerTurn) { turnStr = "HUMAN  [ X ]";    turnCol = Cyber::Cyan; }
        else              { turnStr = "A.I. NEXUS  [ O ]"; turnCol = Cyber::Magenta; }
    }

    // player card
    DrawNeonRect(window, pX, cursor, pW, 44.f,
        sf::Color(turnCol.r, turnCol.g, turnCol.b, 18),
        sf::Color(turnCol.r, turnCol.g, turnCol.b, 120), 1.5f);
    DrawGlitchStripes(window, pX, cursor, pW, 44.f,
        sf::Color(turnCol.r, turnCol.g, turnCol.b, 35));

    sf::Text tTxt(turnStr, font, 22);
    tTxt.setFillColor(turnCol);
    tTxt.setStyle(sf::Text::Bold);
    sf::FloatRect tb = tTxt.getLocalBounds();
    tTxt.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
    tTxt.setPosition(pX + pW / 2.f, cursor + 22.f);
    window.draw(tTxt);

    cursor += 58.f;

    // ── Timer ────────────────────────────────────────────────
    DrawSectionHeader(window, font, "// THOI GIAN CON LAI", pX, cursor, pW, Cyber::Gray);
    cursor += 22.f;

    sf::Color timerCol =
        (timeRemaining > 20.f) ? Cyber::Cyan :
        (timeRemaining > 10.f) ? Cyber::Yellow :
                                  Cyber::NeonRed;

    // large timer digit
    {
        std::ostringstream ss;
        ss << std::setw(2) << std::setfill('0') << (int)timeRemaining << "s";
        DrawCentredText(window, font, ss.str(), 38, timerCol,
            pX + pW / 2.f, cursor + 18.f);
    }
    cursor += 40.f;

    // bar
    DrawTimerBar(window, pX, cursor, pW, 10.f,
        std::max(0.f, std::min(1.f, timeRemaining / 60.f)), timerCol);
    cursor += 22.f;

    // ── Result / Hint block ──────────────────────────────────
    if (gameStatus != 0) {
        DrawNeonRect(window, pX, cursor, pW, 52.f,
            sf::Color(40, 40, 10, 200), Cyber::Yellow, 2.f);
        DrawCornerBrackets(window, pX, cursor, pW, 52.f,
            Cyber::Yellow, 10.f, 2.f);

        std::string s;
        if (gameMode == PVP) {
            s = (gameStatus == 1) ? "PLAYER 01  WINS!" :
                (gameStatus == 2) ? "PLAYER 02  WINS!" : "DRAW  //  HOA!";
        } else {
            s = (gameStatus == 1) ? "HUMAN  WINS!" :
                (gameStatus == 2) ? "A.I. NEXUS  WINS!" : "DRAW  //  HOA!";
        }
        DrawCentredText(window, font, s, 21, Cyber::Yellow,
            pX + pW / 2.f, cursor + 26.f);
        cursor += 64.f;
    } else {
        std::string hint;
        if (gameMode == PVP)
            hint = isPlayerTurn ? ">> P01: CHON O TREN BAN CO"
                                : ">> P02: CHON O TREN BAN CO";
        else
            hint = isPlayerTurn ? ">> CLICK DE DAT QUAN"
                                : ">> AI DANG TINH TOAN...";

        sf::Text ht(hint, font, 13);
        ht.setFillColor(Cyber::Gray);
        ht.setPosition(pX, cursor);
        window.draw(ht);
        cursor += 22.f;
    }

    // ── Undo block (PVP only) ────────────────────────────────
    if (gameMode == PVP) {
        DrawSectionHeader(window, font, "// UNDO TOKENS", pX, cursor, pW, Cyber::Gray);
        cursor += 20.f;

        struct PInfo { std::string lbl; int left; sf::Color col; };
        PInfo info[2] = {
            { "P01 [X]", undoLeft[0], Cyber::Cyan    },
            { "P02 [O]", undoLeft[1], Cyber::Magenta }
        };

        for (int p = 0; p < 2; ++p) {
            sf::Text lt(info[p].lbl, font, 15);
            lt.setFillColor(info[p].col);
            lt.setPosition(pX, cursor);
            window.draw(lt);

            DrawDots(window, pX + 80.f, cursor + 8.f,
                Config::UNDO_MAX, info[p].left, info[p].col, 7.f, 20.f);
            cursor += 28.f;
        }

        // divider
        sf::RectangleShape sep2({ pW, 1.f });
        sep2.setPosition(pX, cursor); cursor += 10.f;
        sep2.setFillColor(sf::Color(50, 60, 90));
        window.draw(sep2);
    }

    // ── Action buttons ───────────────────────────────────────
    const char*  gameBtns[] = { "UNDO", "SAVE GAME", "MAIN MENU" };
    const sf::Color btnAccent[] = { Cyber::Cyan, Cyber::CyanDim, Cyber::MagDim };
    const float BTN_H = 50.f;
    float btnY = (gameMode == PVP) ? cursor : cursor + 20.f;

    sf::Vector2i mp = sf::Mouse::getPosition(window);

    for (int i = 0; i < 3; ++i) {
        float bX = pX;
        float bY = btnY + i * 66.f;
        bool  hov = (mp.x >= bX && mp.x <= bX + pW &&
                     mp.y >= bY && mp.y <= bY + BTN_H);

        DrawNeonRect(window, bX, bY, pW, BTN_H,
            hov ? sf::Color(20, 35, 65, 240) : Cyber::BgBtn,
            hov ? btnAccent[i] : sf::Color(btnAccent[i].r, btnAccent[i].g,
                                           btnAccent[i].b, 70),
            hov ? 2.f : 1.f);

        if (hov)
            DrawCornerBrackets(window, bX, bY, pW, BTN_H,
                btnAccent[i], 8.f, 1.5f);

        // index
        sf::Text idx("0" + std::to_string(i + 1), font, 12);
        idx.setFillColor(sf::Color(btnAccent[i].r, btnAccent[i].g,
            btnAccent[i].b, hov ? 220 : 100));
        idx.setPosition(bX + 8.f, bY + BTN_H / 2.f - 8.f);
        window.draw(idx);

        DrawCentredText(window, font, gameBtns[i], 20,
            hov ? Cyber::White : sf::Color(160, 175, 200),
            bX + pW / 2.f, bY + BTN_H / 2.f);
    }
}

// ============================================================
//  DrawBoard  ─  Cyberpunk grid
// ============================================================
void DrawBoard(sf::RenderWindow& window, int boardSize)
{
    int   cs  = GetDynCellSize(boardSize);
    float ox  = static_cast<float>(Config::OFFSET_X);
    float oy  = static_cast<float>(Config::OFFSET_Y);
    float bW  = static_cast<float>(boardSize * cs);
    float bH  = static_cast<float>(boardSize * cs);

    // Board background
    DrawNeonRect(window, ox - 2.f, oy - 2.f, bW + 4.f, bH + 4.f,
        sf::Color(6, 8, 16), Cyber::Grid, 1.f);
    DrawScanlines(window, ox, oy, bW, bH, sf::Color(0, 0, 0, 10));

    // Grid lines
    for (int i = 0; i <= boardSize; ++i) {
        bool edge = (i == 0 || i == boardSize);
        sf::Color lc = edge ? Cyber::Grid : Cyber::GridDim;
        float     lw = edge ? 1.5f : 0.5f;

        // horizontal
        sf::RectangleShape h({ bW, lw });
        h.setPosition(ox, oy + i * cs - lw / 2.f);
        h.setFillColor(lc);
        window.draw(h);

        // vertical
        sf::RectangleShape v({ lw, bH });
        v.setPosition(ox + i * cs - lw / 2.f, oy);
        v.setFillColor(lc);
        window.draw(v);
    }

    // Centre dot + star-point dots (like Go board)
    auto drawDot = [&](int gx, int gy, float r, sf::Color c) {
        sf::CircleShape dot(r);
        dot.setOrigin(r, r);
        dot.setPosition(ox + gx * cs + cs / 2.f, oy + gy * cs + cs / 2.f);
        dot.setFillColor(c);
        window.draw(dot);
    };

    int mid = boardSize / 2;
    drawDot(mid, mid, 4.f, Cyber::CyanDim);          // centre

    if (boardSize >= 15) {
        int q = boardSize / 4;
        for (int dx : {q, boardSize - 1 - q}) {
            for (int dy : {q, boardSize - 1 - q}) {
                drawDot(dx, dy, 3.f, Cyber::GridDim);
            }
        }
    }

    // Corner brackets on board
    DrawCornerBrackets(window, ox - 2.f, oy - 2.f,
        bW + 4.f, bH + 4.f, Cyber::CyanDim, 18.f, 1.5f);
}

// ============================================================
//  DrawPieces  ─  Cyberpunk pieces
// ============================================================
void DrawPieces(sf::RenderWindow& window, int boardSize)
{
    int   cs = GetDynCellSize(boardSize);
    float ox = static_cast<float>(Config::OFFSET_X);
    float oy = static_cast<float>(Config::OFFSET_Y);

    for (int x = 0; x < boardSize; ++x) {
        for (int y = 0; y < boardSize; ++y) {
            int  cell = GetCell(x, y);
            if (!cell) continue;

            float cx = ox + x * cs + cs / 2.f;
            float cy = oy + y * cs + cs / 2.f;
            float arm = cs / 2.f - 5.f;

            if (cell == 1) {
                // ── X  (cyan) ────────────────────────────────
                // Corner brackets as decoration
                float br = arm + 3.f;
                DrawCornerBrackets(window,
                    cx - br - 1.f, cy - br - 1.f,
                    (br + 1.f) * 2.f, (br + 1.f) * 2.f,
                    sf::Color(0, 200, 220, 80), 5.f, 1.f);

                // Main X strokes (as rotated rectangles)
                for (int rot : {45, -45}) {
                    sf::RectangleShape ln({ arm * 2.f, 2.5f });
                    ln.setFillColor(Cyber::Cyan);
                    ln.setOrigin(arm, 1.25f);
                    ln.setPosition(cx, cy);
                    ln.setRotation(static_cast<float>(rot));
                    window.draw(ln);
                }
            } else {
                // ── O  (magenta) ─────────────────────────────
                // Outer ring (dim)
                sf::CircleShape outer(arm + 2.f);
                outer.setOrigin(arm + 2.f, arm + 2.f);
                outer.setPosition(cx, cy);
                outer.setFillColor(sf::Color::Transparent);
                outer.setOutlineThickness(1.f);
                outer.setOutlineColor(sf::Color(200, 0, 160, 60));
                window.draw(outer);

                // Main ring
                sf::CircleShape ring(arm);
                ring.setOrigin(arm, arm);
                ring.setPosition(cx, cy);
                ring.setFillColor(sf::Color::Transparent);
                ring.setOutlineThickness(2.5f);
                ring.setOutlineColor(Cyber::Magenta);
                window.draw(ring);

                // Centre dot
                sf::CircleShape dot(2.5f);
                dot.setOrigin(2.5f, 2.5f);
                dot.setPosition(cx, cy);
                dot.setFillColor(Cyber::Magenta);
                window.draw(dot);
            }
        }
    }
}

// ============================================================
//  DrawHoverEffect
// ============================================================
void DrawHoverEffect(sf::RenderWindow& window, int mouseX, int mouseY, int boardSize)
{
    int   cs = GetDynCellSize(boardSize);
    int   gX = (mouseX - Config::OFFSET_X) / cs;
    int   gY = (mouseY - Config::OFFSET_Y) / cs;

    if (gX >= 0 && gX < boardSize && gY >= 0 && gY < boardSize
        && GetCell(gX, gY) == 0)
    {
        float rx = static_cast<float>(Config::OFFSET_X + gX * cs);
        float ry = static_cast<float>(Config::OFFSET_Y + gY * cs);

        // subtle fill
        sf::RectangleShape hr({ static_cast<float>(cs), static_cast<float>(cs) });
        hr.setPosition(rx, ry);
        hr.setFillColor(sf::Color(0, 200, 255, 22));
        window.draw(hr);

        // corner brackets as hover indicator
        DrawCornerBrackets(window, rx + 2.f, ry + 2.f,
            static_cast<float>(cs) - 4.f, static_cast<float>(cs) - 4.f,
            sf::Color(0, 255, 255, 180), 6.f, 1.5f);
    }
}

// ============================================================
//  DrawWinLine  (2 overloads)
// ============================================================
void DrawWinLine(sf::RenderWindow& window,
    int sX, int sY, int eX, int eY)
{
    DrawWinLine(window, sX, sY, eX, eY, 15);
}

void DrawWinLine(sf::RenderWindow& window,
    int sX, int sY, int eX, int eY, int boardSize)
{
    if (sX == -1) return;
    int   cs = GetDynCellSize(boardSize);
    float x1 = Config::OFFSET_X + sX * cs + cs / 2.f;
    float y1 = Config::OFFSET_Y + sY * cs + cs / 2.f;
    float x2 = Config::OFFSET_X + eX * cs + cs / 2.f;
    float y2 = Config::OFFSET_Y + eY * cs + cs / 2.f;

    float dx  = x2 - x1, dy = y2 - y1;
    float len = std::sqrt(dx * dx + dy * dy);
    float ang = std::atan2(dy, dx) * 180.f / 3.14159265f;

    // glow layer (thick, dim)
    sf::RectangleShape glow({ len, 10.f });
    glow.setFillColor(sf::Color(255, 220, 0, 40));
    glow.setOrigin(0, 5.f);
    glow.setPosition(x1, y1);
    glow.setRotation(ang);
    window.draw(glow);

    // main line
    sf::RectangleShape line({ len, 3.5f });
    line.setFillColor(Cyber::Yellow);
    line.setOrigin(0, 1.75f);
    line.setPosition(x1, y1);
    line.setRotation(ang);
    window.draw(line);

    // end caps
    for (float ex : { x1, x2 }) {
        float ey = (ex == x1) ? y1 : y2;
        sf::CircleShape cap(5.f);
        cap.setOrigin(5.f, 5.f);
        cap.setPosition(ex, ey);
        cap.setFillColor(Cyber::Yellow);
        window.draw(cap);
    }
}

// ============================================================
//  DrawSettings  ─  Cyberpunk Settings Screen
// ============================================================
void DrawSettings(sf::RenderWindow& window, const sf::Font& font,
    int boardSize, bool ruleBlock2, int aiLevel,
    float sfxVolume, bool bgmEnabled)
{
    float W = static_cast<float>(Config::WIN_WIDTH);
    float H = static_cast<float>(Config::WIN_HEIGHT);

    // ── Background grid + scanlines ──────────────────────────
    for (float gx = 0; gx < W; gx += 60.f) {
        sf::RectangleShape vl({ 1.f, H });
        vl.setPosition(gx, 0);
        vl.setFillColor(sf::Color(0, 255, 255, 6));
        window.draw(vl);
    }
    for (float gy = 0; gy < H; gy += 60.f) {
        sf::RectangleShape hl({ W, 1.f });
        hl.setPosition(0, gy);
        hl.setFillColor(sf::Color(0, 255, 255, 6));
        window.draw(hl);
    }
    DrawScanlines(window, 0, 0, W, H, sf::Color(0, 0, 0, 14));

    // ── Title ────────────────────────────────────────────────
    {
        sf::Text title("// SETTINGS.CFG", font, 54);
        title.setFillColor(Cyber::Yellow);
        title.setStyle(sf::Text::Bold);
        sf::FloatRect r = title.getLocalBounds();
        title.setOrigin(r.left + r.width / 2.f, r.top + r.height / 2.f);
        title.setPosition(W / 2.f, 70.f);
        window.draw(title);

        sf::RectangleShape sep({ 500.f, 1.5f });
        sep.setPosition(W / 2.f - 250.f, 112.f);
        sep.setFillColor(Cyber::Yellow);
        window.draw(sep);
    }

    // ── Settings rows ────────────────────────────────────────
    const char* labels[] = {
        "BOARD SIZE  (10 – 30)",
        "RULE : BLOCK DOUBLE-2",
        "AI DIFFICULTY  (1 – 6)",
        "SFX VOLUME  (0 – 100)",
        "BGM  //  BACKGROUND MUSIC"
    };
    std::string values[] = {
        std::to_string(boardSize),
        ruleBlock2 ? "ENABLED" : "DISABLED",
        std::to_string(aiLevel),
        std::to_string((int)sfxVolume),
        bgmEnabled ? "ENABLED" : "DISABLED"
    };

    const float LX   = 200.f;
    const float CX   = 700.f;
    const float ROW_H = 80.f;
    const float S_Y  = 160.f;
    const float CTRL_H = 48.f;

    sf::Vector2i mp = sf::Mouse::getPosition(window);

    for (int i = 0; i < 5; ++i) {
        float ry = S_Y + i * ROW_H;

        // label
        sf::Text lbl(labels[i], font, 22);
        lbl.setFillColor(Cyber::White);
        lbl.setPosition(LX, ry + (CTRL_H - 22.f) / 2.f);
        window.draw(lbl);

        bool isToggle = (i == 1 || i == 4);
        bool isOn     = (i == 1) ? ruleBlock2 : bgmEnabled;

        if (!isToggle) {
            // ── [ - ]  value  [ + ] ──────────────────────────
            const float BW = 46.f;

            // minus button
            bool hovM = (mp.x >= CX && mp.x <= CX + BW &&
                         mp.y >= ry && mp.y <= ry + CTRL_H);
            DrawNeonRect(window, CX, ry, BW, CTRL_H,
                hovM ? sf::Color(80, 20, 20) : Cyber::BgBtn,
                Cyber::NeonRed, 1.5f);
            DrawCentredText(window, font, "-", 28, Cyber::NeonRed,
                CX + BW / 2.f, ry + CTRL_H / 2.f);

            // value box
            DrawNeonRect(window, CX + BW + 4.f, ry, 80.f, CTRL_H,
                sf::Color(10, 20, 40), Cyber::CyanDim, 1.f);
            DrawCentredText(window, font, values[i], 24, Cyber::Cyan,
                CX + BW + 4.f + 40.f, ry + CTRL_H / 2.f);

            // plus button
            float px2 = CX + BW + 4.f + 80.f + 4.f;
            bool hovP = (mp.x >= px2 && mp.x <= px2 + BW &&
                         mp.y >= ry  && mp.y <= ry + CTRL_H);
            DrawNeonRect(window, px2, ry, BW, CTRL_H,
                hovP ? sf::Color(20, 60, 20) : Cyber::BgBtn,
                sf::Color(50, 200, 100), 1.5f);
            DrawCentredText(window, font, "+", 28, sf::Color(50, 200, 100),
                px2 + BW / 2.f, ry + CTRL_H / 2.f);
        } else {
            // ── Toggle button ─────────────────────────────────
            const float TW = 200.f;
            bool hovT = (mp.x >= CX && mp.x <= CX + TW &&
                         mp.y >= ry && mp.y <= ry + CTRL_H);
            sf::Color tCol = isOn ? Cyber::Cyan : Cyber::NeonRed;
            DrawNeonRect(window, CX, ry, TW, CTRL_H,
                sf::Color(tCol.r, tCol.g, tCol.b, hovT ? 40u : 15u),
                tCol, 1.5f);
            if (hovT) DrawCornerBrackets(window, CX, ry, TW, CTRL_H, tCol, 8.f, 1.5f);
            DrawCentredText(window, font, values[i], 22, tCol,
                CX + TW / 2.f, ry + CTRL_H / 2.f);
        }
    }

    // ── Back button ──────────────────────────────────────────
    const float BW = 320.f, BH = 56.f;
    const float BX = W / 2.f - BW / 2.f, BY = H - 100.f;
    bool bh = (mp.x >= BX && mp.x <= BX + BW && mp.y >= BY && mp.y <= BY + BH);
    DrawNeonRect(window, BX, BY, BW, BH,
        bh ? sf::Color(20, 40, 70) : Cyber::BgBtn,
        bh ? Cyber::Magenta : sf::Color(180, 0, 150, 100), bh ? 2.f : 1.f);
    if (bh) DrawCornerBrackets(window, BX, BY, BW, BH, Cyber::Magenta, 10.f, 2.f);
    DrawCentredText(window, font, "<< BACK TO MENU", 22,
        bh ? Cyber::White : sf::Color(160, 175, 200),
        BX + BW / 2.f, BY + BH / 2.f);
}
