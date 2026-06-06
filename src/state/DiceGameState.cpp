#include "state/DiceGameState.h"
#include "entity/Player.h"
#include "entity/Student.h"
#include <cmath>
#include <algorithm>
#include <sstream>

DiceGameState::DiceGameState(Game* game, Player* p, Student* s)
    : GameState(game)
    , player(p)
    , student(s)
    , font(nullptr)
    , playerTotalScore(0)
    , studentTotalScore(0)
    , playerLockCount(0)
    , studentLockCount(0)
    , playerAllLocked(false)
    , studentAllLocked(false)
    , gameStarted(false)
    , finished(false)
    , running(true)
    , showRules(false)
    , showFormula(false)
    , socialDelta(0)
    , resultTimer(0.0f)
    , rng(std::random_device{}())
    , dist(1, 6)
    , animTimer(0.0f)
{
    // 鍒濆鍖栭瀛?
    for (int i = 0; i < 3; ++i) {
        playerDice[i].value = dist(rng);
        playerDice[i].locked = false;
        studentDice[i].value = dist(rng);
        studentDice[i].locked = false;
        displayValues[i] = dist(rng);
    }
}

void DiceGameState::handleInput(const sf::Event& event) {
    if (!running) return;

    if (const auto* keyEv = event.getIf<sf::Event::KeyPressed>()) {
        auto code = keyEv->code;

        // 鎸?Escape 閫€鍑烘父鎴忥紙浠讳綍鏃跺€欓兘鍙敤锛?
        if (code == sf::Keyboard::Key::Escape) {
            running = false;
            return;
        }

        // 娓告垙宸茬粨鏉燂紝涓嶅啀鎺ュ彈鍏朵粬鎸夐敭
        if (finished) return;

        // 鎸?G 鍒囨崲瑙勫垯/鍏紡鏄剧ず锛堝鐢熺涓撶敤锛?
        if (code == sf::Keyboard::Key::G) {
            if (showFormula) {
                showFormula = false;
                showRules = false;
            } else if (showRules) {
                showFormula = true;
                showRules = false;
            } else {
                showRules = true;
            }
            return;
        }

        // 鎸?L 閿佸畾楠板瓙
        if (code == sf::Keyboard::Key::L) {
            if (!gameStarted) {
                gameStarted = true;
            }

            if (!playerAllLocked) {
                // 閿佸畾鐜╁褰撳墠鏈攣瀹氱殑绗竴涓瀛?
                for (int i = 0; i < 3; ++i) {
                    if (!playerDice[i].locked) {
                        playerDice[i].value = displayValues[i];
                        playerDice[i].locked = true;
                        playerLockCount++;
                        break;
                    }
                }

                if (playerLockCount >= 3) {
                    playerAllLocked = true;
                    playerTotalScore = calculateTotalScore(playerDice);
                }
            }

            // 鐜╁鍏ㄩ儴閿佸畾鍚庯紝鑷姩涓哄鐢熺‘瀹氶瀛?
            if (playerAllLocked && !studentAllLocked) {
                // 瀛︾敓楠板瓙涓€娆℃€у叏閮ㄧ‘瀹?
                for (int i = 0; i < 3; ++i) {
                    if (!studentDice[i].locked) {
                        studentDice[i] = {dist(rng), true};
                    }
                }
                studentAllLocked = true;
                studentTotalScore = calculateTotalScore(studentDice);

                // 璁＄畻宸€煎苟鏇存柊绀句氦鍊?
                int diff = playerTotalScore - studentTotalScore;
                socialDelta = diff;

                if (socialDelta > 0) {
                    player->modifyAttributes(Attributes(0, 0, 0, socialDelta, 0));
                } else if (socialDelta < 0) {
                    player->modifyAttributes(Attributes(0, 0, 0, socialDelta, 0));
                }
                // diff == 0 鏃朵笉鍙?

                finished = true;
            }
        }

    }
}

void DiceGameState::update(float deltaTime) {
    if (!running || finished) return;

    animTimer += deltaTime;

    // 涓虹帺瀹舵湭閿佸畾鐨勯瀛愭寔缁殢鏈哄彉鍖栵紙姣?.08绉掑彉鍖栦竴娆★級
    if (animTimer >= 0.08f) {
        animTimer = 0.0f;
        for (int i = 0; i < 3; ++i) {
            if (!playerDice[i].locked) {
                displayValues[i] = dist(rng);
            }
        }
    }

    // 缁撴灉璁℃椂鍣?
    if (finished) {
        resultTimer += deltaTime;
    }
}

// 鈹€鈹€ 鐐规暟璁＄畻鍏紡 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€

int DiceGameState::calculateTotalScore(const std::array<DiceData, 3>& dice) {
    int v[3] = {dice[0].value, dice[1].value, dice[2].value};
    std::sort(v, v + 3, std::greater<int>());

    int a = v[0]; // 鏈€澶?
    int b = v[1]; // 娆″ぇ
    int c = v[2]; // 鏈€灏?

    // 璞瑰瓙锛氫笁涓偣鏁扮浉鍚?
    if (a == b && b == c) {
        // 3涓?鐐规暟涓?00
        if (a == 1) {
            return 100;
        }
        // 鐐规暟 * 10 + 38
        return a * 10 + 38;
    }

    // 瀵瑰瓙锛氫袱涓偣鏁扮浉鍚?
    if (a == b) {
        // 瀵瑰瓙鐐规暟 * 8 + 鍗曞紶鐐规暟
        return a * 8 + c;
    }
    if (b == c) {
        return b * 8 + a;
    }

    // 椤哄瓙锛氫笁涓繛缁殑鐐规暟
    if (a == b + 1 && b == c + 1) {
        // 涓棿鐐规暟 * 10 + 30
        return b * 10 + 30;
    }

    // 鏅€氾紙鍗曞紶锛夛細涓変釜鐐规暟涔嬪拰
    return a + b + c;
}

// 鈹€鈹€ 娓叉煋 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€

void DiceGameState::render(sf::RenderWindow& window) {
    if (!font || !running) return;

    // 鑳屾櫙
    sf::RectangleShape bg({960.0f, 540.0f});
    bg.setFillColor(sf::Color(15, 18, 28));
    window.draw(bg);

    // 鍒嗛殧绾?
    sf::RectangleShape divider({960.0f, 2.0f});
    divider.setPosition({0.0f, 270.0f});
    divider.setFillColor(sf::Color(120, 140, 180, 180));
    window.draw(divider);

    // 鈹€鈹€ 涓婂崐閮ㄥ垎锛氬鐢熺晫闈?鈹€鈹€
    {
        sf::Text label(*font, "Student's Dice", 18);
        label.setFillColor(sf::Color(255, 180, 100));
        label.setPosition({380.0f, 16.0f});
        window.draw(label);

        // 瀛︾敓楠板瓙锛氱帺瀹舵湭閿佸畾鏃朵笉鏄剧ず锛堜繚鎸侀殣钘忥級
        if (!studentAllLocked && !playerAllLocked) {
            sf::Text hidden(*font, "???", 28);
            hidden.setFillColor(sf::Color(150, 150, 150));
            hidden.setPosition({400.0f, 110.0f});
            window.draw(hidden);

            sf::Text hint(*font, "Student's dice will be revealed\nafter you lock all your dice", 12);
            hint.setFillColor(sf::Color(120, 120, 140));
            hint.setPosition({340.0f, 170.0f});
            window.draw(hint);
        } else {
            renderDiceRow(window, studentDice, 80.0f,
                          studentTotalScore, studentAllLocked, "Student");

            // 瀛︾敓绔彁绀猴細鏄剧ず鍙煡鐪嬭鍒?
            if (!finished) {
                sf::Text studentHint(*font, "Press [G] to view rules/formula", 10);
                studentHint.setFillColor(sf::Color(120, 130, 160, 180));
                studentHint.setPosition({10.0f, 16.0f});
                window.draw(studentHint);
            }
        }

        // 涓婂尯鍩熸爣绛?
        sf::Text topZone(*font, "--- Opponent Area ---", 10);
        topZone.setFillColor(sf::Color(80, 90, 110, 150));
        topZone.setPosition({400.0f, 255.0f});
        window.draw(topZone);
    }

    // 鈹€鈹€ 涓嬪崐閮ㄥ垎锛氱帺瀹剁晫闈?鈹€鈹€
    {
        sf::Text label(*font, "Your Dice", 18);
        label.setFillColor(sf::Color(100, 200, 255));
        label.setPosition({400.0f, 290.0f});
        window.draw(label);

        renderDiceRow(window, playerDice, 352.0f,
                      playerTotalScore, playerAllLocked, "You");

        // 涓嬪尯鍩熸爣绛?
        sf::Text bottomZone(*font, "--- Your Area ---", 10);
        bottomZone.setFillColor(sf::Color(80, 90, 110, 150));
        bottomZone.setPosition({410.0f, 525.0f});
        window.draw(bottomZone);

        // 鎿嶄綔鎻愮ず
        if (!gameStarted) {
            sf::Text startHint(*font, "Press [L] to start the dice game!", 16);
            startHint.setFillColor(sf::Color(220, 220, 100));
            startHint.setPosition({300.0f, 458.0f});
            window.draw(startHint);
        } else if (!playerAllLocked) {
            std::ostringstream oss;
            oss << "Press [L] to lock dice (" << (3 - playerLockCount) << " remaining)";
            sf::Text lockHint(*font, oss.str(), 14);
            lockHint.setFillColor(sf::Color(180, 220, 255));
            lockHint.setPosition({280.0f, 480.0f});
            window.draw(lockHint);
        }
    }

    // 鈹€鈹€ 瑙勫垯/鍏紡闈㈡澘 鈹€鈹€
    if (showRules || showFormula) {
        renderRulesPanel(window);
    }

    // 鈹€鈹€ 缁撴灉瑕嗙洊灞?鈹€鈹€
    if (finished) {
        renderResultOverlay(window);
    }
}

void DiceGameState::renderDiceRow(sf::RenderWindow& window,
                                   const std::array<DiceData, 3>& dice,
                                   float startY,
                                   int totalScore,
                                   bool showScore,
                                   const std::string& label) {
    float diceSize = 56.0f;
    float spacing = 90.0f;
    float startX = 460.0f - spacing; // 灞呬腑涓変釜楠板瓙

    for (int i = 0; i < 3; ++i) {
        float cx = startX + i * spacing;
        float cy = startY;

        int displayVal;
        if (dice[i].locked) {
            displayVal = dice[i].value;
        } else {
            displayVal = displayValues[i];
        }

        renderDiceFace(window, displayVal, cx, cy, diceSize, dice[i].locked);
    }

    // 鏄剧ず鎬荤偣鏁?
    if (showScore) {
        std::ostringstream oss;
        oss << label << " Total: " << totalScore;
        sf::Text scoreText(*font, oss.str(), 16);
        scoreText.setFillColor(label == "You" ? sf::Color(100, 255, 100) : sf::Color(255, 180, 100));
        scoreText.setPosition({380.0f, startY + 72.0f});
        window.draw(scoreText);
    }
}

void DiceGameState::renderDiceFace(sf::RenderWindow& window, int value,
                                    float cx, float cy, float size, bool locked) {
    float half = size / 2.0f;

    // 楠板瓙鑳屾櫙
    sf::RectangleShape diceBg({size, size});
    diceBg.setPosition({cx - half, cy - half});
    diceBg.setFillColor(locked ? sf::Color(70, 70, 90) : sf::Color(50, 50, 65));
    diceBg.setOutlineColor(locked ? sf::Color(120, 220, 120) : sf::Color(100, 100, 120));
    diceBg.setOutlineThickness(locked ? 3.0f : 2.0f);
    window.draw(diceBg);

    // 鐐规暟鏄剧ず - 鐢ㄧ偣闃佃〃绀?
    auto drawDot = [&](float dx, float dy) {
        sf::CircleShape dot(4.0f);
        dot.setFillColor(sf::Color(240, 240, 250));
        dot.setPosition({cx + dx - 4.0f, cy + dy - 4.0f});
        window.draw(dot);
    };

    // 鏍规嵁鐐规暟缁樺埗鐐归樀
    //  1     2     3
    //  路     路.    路.
    //         .     .路
    //              路.路
    //
    //  4     5     6
    //  路路   路路   路路
    //       .    路路
    //  路路   路路   路路

    switch (value) {
        case 1:
            drawDot(0, 0);
            break;
        case 2:
            drawDot(-10, -10);
            drawDot(10, 10);
            break;
        case 3:
            drawDot(-10, -10);
            drawDot(0, 0);
            drawDot(10, 10);
            break;
        case 4:
            drawDot(-10, -10);
            drawDot(10, -10);
            drawDot(-10, 10);
            drawDot(10, 10);
            break;
        case 5:
            drawDot(-10, -10);
            drawDot(10, -10);
            drawDot(0, 0);
            drawDot(-10, 10);
            drawDot(10, 10);
            break;
        case 6:
            drawDot(-10, -10);
            drawDot(10, -10);
            drawDot(-10, 0);
            drawDot(10, 0);
            drawDot(-10, 10);
            drawDot(10, 10);
            break;
    }

    // 鐐规暟鏁板瓧鏍囨敞
    sf::Text numText(*font, std::to_string(value), 14);
    numText.setFillColor(sf::Color(220, 220, 240));
    numText.setPosition({cx - half + 4.0f, cy + half - 18.0f});
    window.draw(numText);
}

void DiceGameState::renderRulesPanel(sf::RenderWindow& window) {
    // 鍗婇€忔槑鑳屾櫙闈㈡澘
    sf::RectangleShape panelBg({580.0f, showFormula ? 380.0f : 310.0f});
    panelBg.setPosition({190.0f, 70.0f});
    panelBg.setFillColor(sf::Color(0, 0, 0, 220));
    panelBg.setOutlineColor(sf::Color(180, 160, 100, 200));
    panelBg.setOutlineThickness(2.0f);
    window.draw(panelBg);

    float y = 80.0f;

    if (showRules) {
        sf::Text title(*font, "--- Dice Game Rules ---", 16);
        title.setFillColor(sf::Color(255, 220, 100));
        title.setPosition({210.0f, y});
        window.draw(title);
        y += 28.0f;

        const char* rules[] = {
            "Both sides each have 3 dice, values range from 1 to 6.",
            "Your dice keep rolling until you press [L] to lock one.",
            "Press [L] three times to lock all three dice.",
            "The opponent's dice will be revealed only after you finish.",
            "Total score is calculated based on special rules (see formula).",
            "If your score > opponent's, you gain Social points equal to the difference.",
            "If your score < opponent's, you lose Social points equal to the difference.",
            "Press [G] again to view the formula."
        };
        for (const auto& line : rules) {
            sf::Text rule(*font, line, 12);
            rule.setFillColor(sf::Color(200, 200, 210));
            rule.setPosition({210.0f, y});
            window.draw(rule);
            y += 22.0f;
        }
    } else if (showFormula) {
        sf::Text title(*font, "--- Scoring Formula ---", 16);
        title.setFillColor(sf::Color(255, 220, 100));
        title.setPosition({210.0f, y});
        window.draw(title);
        y += 30.0f;

        const char* formulas[] = {
            "Normal (all different): Sum of all three dice",
            "Pair (two same): Pair*8 + Single",
            "Straight (three consecutive): Mid*10 + 30",
            "Triple (three same, except 1,1,1): Value*10 + 38",
            "Special Triple (1,1,1): 100 points",
            "",
            "Example 1: 6,6,2 -> Pair(6)*8 + 2 = 50",
            "Example 2: 4,5,6 -> Straight: 5*10 + 30 = 80",
            "Example 3: 5,5,5 -> 5*10 + 38 = 88",
            "Example 4: 1,1,1 -> 100",
            "Example 5: 6,3,1 -> 6+3+1 = 10"
        };
        for (const auto& line : formulas) {
            sf::Text fmt(*font, line, 12);
            fmt.setFillColor(sf::Color(180, 200, 190));
            fmt.setPosition({210.0f, y});
            window.draw(fmt);
            y += 22.0f;
        }
    }

    sf::Text closeHint(*font, "Press [G] to close", 10);
    closeHint.setFillColor(sf::Color(140, 140, 160, 180));
    closeHint.setPosition({210.0f, y + 10.0f});
    window.draw(closeHint);
}

void DiceGameState::renderResultOverlay(sf::RenderWindow& window) {
    // 鈹€鈹€ 搴曢儴缁撴灉妯箙锛堜笉閬尅楠板瓙锛夆攢鈹€
    // 鍗婇€忔槑搴曢儴鏉?
    sf::RectangleShape banner({960.0f, 68.0f});
    banner.setPosition({0.0f, 472.0f});
    banner.setFillColor(sf::Color(10, 12, 22, 235));
    banner.setOutlineColor(socialDelta > 0 ? sf::Color(100, 230, 100, 200) :
                            (socialDelta < 0 ? sf::Color(230, 100, 100, 200) :
                             sf::Color(180, 180, 180, 200)));
    banner.setOutlineThickness(2.0f);
    window.draw(banner);

    // 缁撴灉鏍囬锛堢揣鍑戝崟琛岋級
    std::string resultTitle = (socialDelta > 0) ? "YOU WIN!" :
                               (socialDelta < 0) ? "YOU LOSE!" : "TIE GAME!";
    sf::Color titleColor = (socialDelta > 0) ? sf::Color(100, 255, 100) :
                            (socialDelta < 0) ? sf::Color(255, 100, 100) :
                            sf::Color(200, 200, 200);

    sf::Text title(*font, resultTitle, 22);
    title.setFillColor(titleColor);
    title.setPosition({20.0f, 480.0f});
    window.draw(title);

    // 鍒嗘暟瀵规瘮
    std::ostringstream scores;
    scores << "You: " << playerTotalScore << "  vs  Student: " << studentTotalScore;
    sf::Text scoreLine(*font, scores.str(), 15);
    scoreLine.setFillColor(sf::Color(220, 220, 240));
    scoreLine.setPosition({180.0f, 482.0f});
    window.draw(scoreLine);

    // 绀句氦鍊煎彉鍖?
    std::ostringstream delta;
    if (socialDelta > 0) {
        delta << "Social +" << socialDelta;
    } else if (socialDelta < 0) {
        delta << "Social " << socialDelta;
    } else {
        delta << "Social unchanged";
    }
    sf::Text deltaText(*font, delta.str(), 15);
    deltaText.setFillColor(socialDelta > 0 ? sf::Color(100, 255, 100) :
                            (socialDelta < 0 ? sf::Color(255, 100, 100) :
                             sf::Color(200, 200, 200)));
    deltaText.setPosition({560.0f, 482.0f});
    window.draw(deltaText);

    // 绀句氦褰撳墠鍊?
    std::ostringstream curSocial;
    curSocial << "-> " << player->getAttributes().social;
    sf::Text curText(*font, curSocial.str(), 13);
    curText.setFillColor(sf::Color(205, 140, 255));
    curText.setPosition({740.0f, 484.0f});
    window.draw(curText);

    // 閫€鍑烘彁绀?
    sf::Text cont(*font, "[Esc] to leave", 11);
    cont.setFillColor(sf::Color(160, 160, 180));
    cont.setPosition({860.0f, 520.0f});
    window.draw(cont);
}
