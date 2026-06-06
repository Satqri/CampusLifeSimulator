#ifndef CLS_STATE_CANTEENGAMESTATE_H
#define CLS_STATE_CANTEENGAMESTATE_H

#include "state/GameState.h"
#include "core/Types.h"
#include <SFML/Graphics.hpp>
#include <random>
#include <string>

class Player;

class CanteenGameState : public GameState {
public:
    CanteenGameState(Game* game, Player* player);

    void handleInput(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;

    bool isRunning() const { return running; }
    bool isFinished() const { return finished; }
    const CanteenMealResult& getResult() const { return result; }
    void setFont(const sf::Font* f) { font = f; }

private:
    void lockNext();
    void calculateBonus();

    void renderDiceSection(sf::RenderWindow& window,
                           float startX, float startY,
                           int value, int score,
                           const char* name,
                           const char* category,
                           bool locked);
    void renderResultPanel(sf::RenderWindow& window);
    void renderRecipePanel(sf::RenderWindow& window);

    static const char* stapleName(int value);
    static int stapleScoreValue(int value);
    static const char* mainDishName(int value);
    static int mainDishScoreValue(int value);
    static const char* drinkName(int value);
    static int drinkScoreValue(int value);

    // ── Safe UTF-8 string helper ───────────────────────────────
    static sf::String u8s(const char* s);

    Player* player;
    const sf::Font* font;

    bool running;
    bool finished;
    bool showRecipe;

    bool lockStaple;
    bool lockMainDish;
    bool lockDrink;
    int lockedCount;

    float animTimer;
    int displayStaple;
    int displayMainDish;
    int displayDrink;

    CanteenMealResult result;

    std::mt19937 rng;
    std::uniform_int_distribution<int> dist;
};

#endif