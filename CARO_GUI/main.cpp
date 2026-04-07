#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "Constants.h"
#include "CaroAPI.h"
#include "RenderUI.h"
#include "InputUI.h"
#include <iostream>

int main()
{
    sf::RenderWindow window(
        sf::VideoMode(Config::WIN_WIDTH, Config::WIN_HEIGHT),
        "Caro Master - Procedural SFML 2.6.2"
    );
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.loadFromFile("arial.ttf"))
        std::cout << "Loi: Khong tim thay arial.ttf" << std::endl;

    sf::SoundBuffer errBuffer;
    sf::Sound errSound;
    if (errBuffer.loadFromFile("error.wav"))
        errSound.setBuffer(errBuffer);

    sf::Music bgMusic;
    if (bgMusic.openFromFile("bgm.ogg")) {
        bgMusic.setLoop(true);
        bgMusic.setVolume(50.f);
        bgMusic.play();
    }

    // ── Trạng thái game ──────────────────────────────────────
    AppState currentState = MENU_SCREEN;
    GameMode gameMode = PVE;

    int   boardSize = 15;
    bool  isPlayerTurn = true;
    float timeRemaining = 60.f;
    int   gameStatus = 0;

    bool  ruleBlock2 = true;
    int   aiLevel = 2;
    float sfxVolume = 100.f;
    bool  bgmEnabled = true;

    int winX1 = -1, winY1 = -1, winX2 = -1, winY2 = -1;

    // ── Trạng thái Undo (PVP) ────────────────────────────────
    // undoLeft[0] = lượt undo còn lại của P1 (X)
    // undoLeft[1] = lượt undo còn lại của P2 (O)
    // lastUndoPlayer: 0/1 = ai vừa dùng undo, -1 = chưa ai / đã reset
    int undoLeft[2] = { Config::UNDO_MAX, Config::UNDO_MAX };
    int lastUndoPlayer = -1;

    sf::Clock clock;

    while (window.isOpen())
    {
        float dt = clock.restart().asSeconds();
        sf::Event event;

        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed &&
                event.mouseButton.button == sf::Mouse::Left)
            {
                int mx = event.mouseButton.x;
                int my = event.mouseButton.y;

                if (currentState == MENU_SCREEN)
                {
                    HandleMenuInput(
                        window, mx, my, currentState,
                        gameMode,
                        boardSize, ruleBlock2, aiLevel,
                        timeRemaining, isPlayerTurn, gameStatus,
                        errSound
                    );
                    // Reset win line và undo state khi bắt đầu game mới
                    winX1 = winY1 = winX2 = winY2 = -1;
                    undoLeft[0] = Config::UNDO_MAX;
                    undoLeft[1] = Config::UNDO_MAX;
                    lastUndoPlayer = -1;
                }
                else if (currentState == IN_GAME_SCREEN)
                {
                    HandleInGameInput(
                        mx, my, currentState,
                        boardSize, gameMode,
                        isPlayerTurn, gameStatus, timeRemaining,
                        undoLeft, lastUndoPlayer,   
                        aiLevel,
                        errSound
                    );
                }
                else if (currentState == SETTINGS_SCREEN)
                {
                    HandleSettingsInput(
                        mx, my, currentState,
                        boardSize, ruleBlock2, aiLevel,
                        sfxVolume, bgmEnabled, errSound
                    );
                    if (bgmEnabled) bgMusic.play();
                    else            bgMusic.pause();
                }
            }
        }

        // ── Cập nhật logic ───────────────────────────────────
        if (currentState == IN_GAME_SCREEN && gameStatus == 0)
        {
            timeRemaining -= dt;
            if (timeRemaining <= 0.f)
                gameStatus = isPlayerTurn ? 2 : 1;

            if (gameMode == PVE && !isPlayerTurn && !IsAIThinking())
            {
                int aiX, aiY;
                gameStatus = GetAIResult(&aiX, &aiY);
                isPlayerTurn = true;
                timeRemaining = 60.f;
                if (gameStatus != 0)
                    GetWinLine(&winX1, &winY1, &winX2, &winY2);
            }
        }

        if (gameStatus != 0 && winX1 == -1)
            GetWinLine(&winX1, &winY1, &winX2, &winY2);

        // ── Vẽ ──────────────────────────────────────────────
        window.clear(BG_COLOR);

        if (currentState == MENU_SCREEN)
        {
            DrawMenu(window, font);
        }
        else if (currentState == SETTINGS_SCREEN)
        {
            DrawSettings(window, font, boardSize, ruleBlock2,
                aiLevel, sfxVolume, bgmEnabled);
        }
        else if (currentState == IN_GAME_SCREEN)
        {
            DrawBoard(window, boardSize);
            DrawPieces(window, boardSize);

            bool showHover = (gameStatus == 0) &&
                (gameMode == PVP || isPlayerTurn);
            if (showHover) {
                sf::Vector2i mp = sf::Mouse::getPosition(window);
                DrawHoverEffect(window, mp.x, mp.y, boardSize);
            }

            if (gameStatus != 0)
                DrawWinLine(window, winX1, winY1, winX2, winY2, boardSize);

            DrawInGamePanel(window, font, timeRemaining,
                isPlayerTurn, gameStatus, boardSize,
                gameMode, undoLeft);   // <-- truyền undoLeft
        }

        window.display();
    }

    return 0;
}