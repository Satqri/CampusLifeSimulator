#include "state/CanteenGameState.h"
#include "entity/Player.h"
#include <algorithm>
#include <sstream>

// ── UTF-8 → sf::String ─────────────────────────────────────────────
sf::String CanteenGameState::u8s(const char* s) {
    return sf::String::fromUtf8(s, s + std::char_traits<char>::length(s));
}

// ── Lookup tables ──────────────────────────────────────────────────

const char* CanteenGameState::stapleName(int v) {
    switch (v) {
        case 1: return "Rice (20)";
        case 2: return "Noodles (20)";
        case 3: return "Burger (30)";
        case 4: return "Pizza (35)";
        case 5: return "Japanese (40)";
        case 6: return "Buffet (50)";
    }
    return "???";
}

int CanteenGameState::stapleScoreValue(int v) {
    switch (v) { case 1: return 20; case 2: return 20; case 3: return 30; case 4: return 35; case 5: return 40; case 6: return 50; }
    return 0;
}

const char* CanteenGameState::mainDishName(int v) {
    switch (v) {
        case 1: return "Drumstick (10)";
        case 2: return "Steak (20)";
        case 3: return "Fish (15)";
        case 4: return "Shrimp (25)";
        case 5: return "Crab (30)";
        case 6: return "Lobster (40)";
    }
    return "???";
}

int CanteenGameState::mainDishScoreValue(int v) {
    switch (v) { case 1: return 10; case 2: return 20; case 3: return 15; case 4: return 25; case 5: return 30; case 6: return 40; }
    return 0;
}

const char* CanteenGameState::drinkName(int v) {
    switch (v) {
        case 1: return "Water (0)";
        case 2: return "Soup (5)";
        case 3: return "Juice (8)";
        case 4: return "Milkshake (12)";
        case 5: return "Beer (15, -2 mood)";
        case 6: return "Champagne (20)";
    }
    return "???";
}

int CanteenGameState::drinkScoreValue(int v) {
    switch (v) { case 1: return 0; case 2: return 5; case 3: return 8; case 4: return 12; case 5: return 15; case 6: return 20; }
    return 0;
}

// ── Constructor ────────────────────────────────────────────────────

CanteenGameState::CanteenGameState(Game* game, Player* p)
    : GameState(game), player(p), font(nullptr)
    , running(true), finished(false), showRecipe(false)
    , lockStaple(false), lockMainDish(false), lockDrink(false), lockedCount(0)
    , animTimer(0.0f)
    , rng(std::random_device{}()), dist(1, 6)
{
    displayStaple = dist(rng);
    displayMainDish = dist(rng);
    displayDrink = dist(rng);
    result = CanteenMealResult();
}

// ── Lock next dice ─────────────────────────────────────────────────

void CanteenGameState::lockNext() {
    if (lockedCount >= 3) return;

    if (lockedCount == 0) {
        int v = dist(rng);
        result.stapleValue = v;
        result.stapleScore = stapleScoreValue(v);
        result.stapleName = stapleName(v);
        lockStaple = true;
    } else if (lockedCount == 1) {
        int v = dist(rng);
        result.mainDishValue = v;
        result.mainDishScore = mainDishScoreValue(v);
        result.mainDishName = mainDishName(v);
        lockMainDish = true;
    } else if (lockedCount == 2) {
        int v = dist(rng);
        result.drinkValue = v;
        result.drinkScore = drinkScoreValue(v);
        result.drinkName = drinkName(v);
        lockDrink = true;
    }

    lockedCount++;

    if (lockedCount >= 3) {
        calculateBonus();
    }
}

// ── Combo bonus ────────────────────────────────────────────────────

void CanteenGameState::calculateBonus() {
    int s = result.stapleValue;
    int m = result.mainDishValue;
    int d = result.drinkValue;

    // Leopard: all three same → direct 80
    if (s == m && m == d) {
        result.isLeopard = true;
        result.bonusName = "Leopard Bonus - Direct 80!";
        result.finalRecovery = 80;
        return;
    }

    int bonus = 0;

    // Seafood Set: Main(3-6) + Drink(6) → +15
    if ((m == 3 || m == 4 || m == 5 || m == 6) && d == 6) {
        bonus += 15;
        result.bonusName = "Seafood Set (+15)";
    }

    result.bonusScore = bonus;

    // Beer: -2 social
    if (d == 5) {
        player->modifyAttributes(Attributes(0, 0, 0, -2, 0));
    }

    int total = result.stapleScore + result.mainDishScore + result.drinkScore + bonus;
    result.finalRecovery = std::min(total, 80);
}

// ── Input ──────────────────────────────────────────────────────────

void CanteenGameState::handleInput(const sf::Event& event) {
    if (!running) return;

    if (const auto* keyEv = event.getIf<sf::Event::KeyPressed>()) {
        auto code = keyEv->code;

        if (code == sf::Keyboard::Key::G) {
            showRecipe = !showRecipe;
            return;
        }

        if (code == sf::Keyboard::Key::Escape) {
            running = false;
            finished = true;
            return;
        }

        if (finished) return;

        if (code == sf::Keyboard::Key::L) {
            lockNext();
            if (lockedCount >= 3 && !finished) {
                player->useCanteenVisit();
                player->restoreStamina(result.finalRecovery);
                finished = true;
            }
        }
    }
}

// ── Update animation ───────────────────────────────────────────────

void CanteenGameState::update(float deltaTime) {
    if (!running || finished) return;
    animTimer += deltaTime;
    if (animTimer >= 0.06f) {
        animTimer = 0.0f;
        if (!lockStaple)  displayStaple  = dist(rng);
        if (!lockMainDish) displayMainDish = dist(rng);
        if (!lockDrink)   displayDrink   = dist(rng);
    }
}

// ── Render ─────────────────────────────────────────────────────────

void CanteenGameState::render(sf::RenderWindow& window) {
    if (!font || !running) return;

    sf::RectangleShape bg({960.0f, 540.0f});
    bg.setFillColor(sf::Color(25, 20, 15));
    window.draw(bg);

    sf::Text title(*font, "Cafeteria - Combo Meal", 26);
    title.setFillColor(sf::Color(255, 220, 120));
    title.setPosition({260.0f, 16.0f});
    window.draw(title);

    if (finished) {
        sf::Text sub(*font, "Results below  |  [G] Recipe  |  [Esc] to exit", 13);
        sub.setFillColor(sf::Color(255, 200, 100));
        sub.setPosition({240.0f, 52.0f});
        window.draw(sub);
    } else {
        sf::Text sub(*font, "Press [L] to lock dice  |  [G] Recipe  |  [Esc] cancel", 13);
        sub.setFillColor(sf::Color(180, 160, 120));
        sub.setPosition({220.0f, 52.0f});
        window.draw(sub);
    }

    std::ostringstream prog;
    prog << "Locked: " << lockedCount << " / 3";
    sf::Text pt(*font, prog.str(), 14);
    pt.setFillColor(sf::Color(100, 220, 160));
    pt.setPosition({835.0f, 50.0f});
    window.draw(pt);

    float cw = 280.0f, sx = 60.0f, dy = 120.0f;

    renderDiceSection(window, sx, dy,
        lockStaple ? result.stapleValue : displayStaple,
        lockStaple ? result.stapleScore : stapleScoreValue(displayStaple),
        lockStaple ? result.stapleName : stapleName(displayStaple),
        "Staple (zhu shi)", lockStaple);

    renderDiceSection(window, sx + cw, dy,
        lockMainDish ? result.mainDishValue : displayMainDish,
        lockMainDish ? result.mainDishScore : mainDishScoreValue(displayMainDish),
        lockMainDish ? result.mainDishName : mainDishName(displayMainDish),
        "Main Dish (zhu cai)", lockMainDish);

    renderDiceSection(window, sx + cw * 2, dy,
        lockDrink ? result.drinkValue : displayDrink,
        lockDrink ? result.drinkScore : drinkScoreValue(displayDrink),
        lockDrink ? result.drinkName : drinkName(displayDrink),
        "Drink (yin liao)", lockDrink);

    if (!lockStaple) {
        sf::Text h(*font, "Press [L] to lock Staple", 15);
        h.setFillColor(sf::Color(220, 220, 100, 200));
        h.setPosition({300.0f, 470.0f}); window.draw(h);
    } else if (!lockMainDish) {
        sf::Text h(*font, "Press [L] to lock Main Dish", 15);
        h.setFillColor(sf::Color(220, 220, 100, 200));
        h.setPosition({280.0f, 470.0f}); window.draw(h);
    } else if (!lockDrink) {
        sf::Text h(*font, "Press [L] to lock Drink", 15);
        h.setFillColor(sf::Color(220, 220, 100, 200));
        h.setPosition({310.0f, 470.0f}); window.draw(h);
    }

    if (showRecipe) renderRecipePanel(window);
    if (lockedCount >= 3) renderResultPanel(window);
}

void CanteenGameState::renderDiceSection(sf::RenderWindow& w,
                                          float x, float y,
                                          int val, int score,
                                          const char* name,
                                          const char* cat,
                                          bool locked) {
    float bw = 240.0f, bh = 330.0f;

    sf::RectangleShape box({bw, bh});
    box.setPosition({x, y});
    box.setFillColor(sf::Color(40, 30, 20, 200));
    box.setOutlineColor(locked ? sf::Color(100, 220, 100) : sf::Color(180, 140, 80));
    box.setOutlineThickness(locked ? 3.0f : 2.0f);
    w.draw(box);

    sf::Text cl(*font, u8s(cat), 14);
    cl.setFillColor(locked ? sf::Color(100, 240, 100) : sf::Color(200, 170, 100));
    cl.setPosition({x + 12.0f, y + 8.0f});
    w.draw(cl);

    float ds = 100.0f;
    float dx = x + bw / 2.0f - ds / 2.0f;
    float dy2 = y + 36.0f;

    sf::RectangleShape db({ds, ds});
    db.setPosition({dx, dy2});
    db.setFillColor(sf::Color(50, 35, 20));
    db.setOutlineColor(sf::Color(230, 190, 100));
    db.setOutlineThickness(3.0f);
    w.draw(db);

    sf::Text num(*font, std::to_string(val), 52);
    num.setFillColor(sf::Color(255, 230, 180));
    auto nb = num.getLocalBounds();
    num.setPosition({dx + ds / 2.0f - nb.size.x / 2.0f, dy2 + ds / 2.0f - nb.size.y / 2.0f - 10.0f});
    w.draw(num);

    sf::Text fn(*font, name, 13);
    fn.setFillColor(sf::Color(240, 210, 150));
    fn.setPosition({x + 12.0f, dy2 + ds + 12.0f});
    w.draw(fn);

    std::ostringstream ss;
    ss << "Score: +" << score;
    sf::Text st(*font, ss.str(), 18);
    st.setFillColor(sf::Color(100, 240, 120));
    st.setPosition({x + 12.0f, dy2 + ds + 38.0f});
    w.draw(st);

    if (locked) {
        sf::Text lk(*font, "LOCKED", 13);
        lk.setFillColor(sf::Color(100, 255, 100));
        lk.setPosition({x + bw - 70.0f, y + 10.0f});
        w.draw(lk);
    }
}

void CanteenGameState::renderResultPanel(sf::RenderWindow& w) {
    sf::RectangleShape rb({920.0f, 130.0f});
    rb.setPosition({20.0f, 395.0f});
    rb.setFillColor(sf::Color(10, 10, 18, 230));
    rb.setOutlineColor(sf::Color(230, 190, 100, 180));
    rb.setOutlineThickness(2.0f);
    w.draw(rb);

    float yy = 403.0f;

    std::ostringstream oss;
    oss << "Raw Total: " << result.stapleScore << " + " << result.mainDishScore << " + " << result.drinkScore;
    if (result.bonusScore > 0) oss << " + " << result.bonusScore << " (bonus)";
    oss << " = " << (result.stapleScore + result.mainDishScore + result.drinkScore + result.bonusScore);
    sf::Text rt(*font, oss.str(), 14);
    rt.setFillColor(sf::Color(200, 200, 210));
    rt.setPosition({40.0f, yy}); w.draw(rt);
    yy += 22.0f;

    if (result.isLeopard) {
        sf::Text lt(*font, result.bonusName, 16);
        lt.setFillColor(sf::Color(255, 200, 50));
        lt.setPosition({40.0f, yy}); w.draw(lt);
        yy += 24.0f;
    } else if (result.bonusScore > 0) {
        sf::Text bt(*font, result.bonusName, 14);
        bt.setFillColor(sf::Color(100, 220, 160));
        bt.setPosition({40.0f, yy}); w.draw(bt);
        yy += 22.0f;
    }

    int raw = result.stapleScore + result.mainDishScore + result.drinkScore + result.bonusScore;
    if (raw > 80 && !result.isLeopard) {
        sf::Text ct(*font, "Capped at 80 (max recovery)", 13);
        ct.setFillColor(sf::Color(255, 160, 100));
        ct.setPosition({40.0f, yy}); w.draw(ct);
        yy += 20.0f;
    }

    std::ostringstream fo;
    fo << ">> Final Recovery: +" << result.finalRecovery << " Stamina";
    sf::Text ft(*font, fo.str(), 20);
    ft.setFillColor(sf::Color(100, 255, 100));
    ft.setPosition({40.0f, yy}); w.draw(ft);
}

void CanteenGameState::renderRecipePanel(sf::RenderWindow& w) {
    sf::RectangleShape pb({580.0f, 400.0f});
    pb.setPosition({190.0f, 60.0f});
    pb.setFillColor(sf::Color(0, 0, 0, 230));
    pb.setOutlineColor(sf::Color(230, 190, 100, 200));
    pb.setOutlineThickness(2.0f);
    w.draw(pb);

    sf::Text tl(*font, "--- Combo Recipe ---", 18);
    tl.setFillColor(sf::Color(255, 220, 100));
    tl.setPosition({210.0f, 70.0f}); w.draw(tl);

    float yy = 100.0f;
    auto line = [&](const char* s, sf::Color c = sf::Color(200, 200, 200)) {
        sf::Text t(*font, s, 12);
        t.setFillColor(c);
        t.setPosition({210.0f, yy}); w.draw(t);
        yy += 20.0f;
    };

    line("Staple: 1=Rice(20)  2=Noodles(20)  3=Burger(30)");
    line("        4=Pizza(35)  5=Japanese(40)  6=Buffet(50)");
    yy += 4.0f;
    line("Main:   1=Drumstick(10)  2=Steak(20)  3=Fish(15)");
    line("        4=Shrimp(25)     5=Crab(30)   6=Lobster(40)");
    yy += 4.0f;
    line("Drink:  1=Water(0)   2=Soup(5)   3=Juice(8)");
    line("        4=Milkshake(12)  5=Beer(15,-2mood)  6=Champagne(20)");
    yy += 8.0f;

    line("SPECIAL COMBOS:", sf::Color(255, 200, 50));
    line("  Seafood Set : Main(3/4/5/6) + Drink(6) => +15", sf::Color(100, 220, 160));
    line("  Luxury Set  : Steak(2) + Lobster(6) + Champagne(6) => +25", sf::Color(100, 220, 160));
    line("  All-Meat    : Drumstick(1) + Steak(2) => +10", sf::Color(100, 220, 160));
    line("  Leopard     : All 3 dice same => 80!", sf::Color(255, 200, 50));
    yy += 4.0f;
    line("Max recovery per meal: 80 Stamina", sf::Color(255, 160, 100));

    sf::Text cl(*font, "Press [G] to close", 11);
    cl.setFillColor(sf::Color(140, 140, 160, 180));
    cl.setPosition({210.0f, yy + 10.0f}); w.draw(cl);
}