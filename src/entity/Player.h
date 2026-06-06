#ifndef CLS_ENTITY_PLAYER_H
#define CLS_ENTITY_PLAYER_H

#include "entity/Character.h"

struct CombatBuffs {
    bool nextEventPositive;
    int nextRollModifier;

    CombatBuffs() : nextEventPositive(false), nextRollModifier(0) {}
    void clear() { nextEventPositive = false; nextRollModifier = 0; }
};

class Player : public Character {
public:
    Player();
    Player(float x, float y);

    void move(float directionX, float directionY, float deltaTime) override;
    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;

    void modifyAttributes(const Attributes& delta);

    bool isSanCritical() const;
    bool isSanDangerous() const;
    int getSanLevel() const;

    CombatBuffs& getCombatBuffs();
    const CombatBuffs& getCombatBuffs() const;

    void clearBuffs();
    void stopMovement();

    // Stamina & Canteen
    void restoreStamina(int amount);
    void consumeStamina(int amount = 20);
    bool canVisitCanteen() const;
    int getCanteenVisitsToday() const;
    int getCanteenLimit() const;
    void useCanteenVisit();
    void resetCanteenVisits();
    void advanceToNextDay();

private:
    sf::Vector2f velocity;
    float acceleration;
    float deceleration;
    float stopSpeedThreshold;
    CombatBuffs combatBuffs;
    sf::RectangleShape sprite;

    static constexpr int kCanteenLimit = 1;
    int canteenVisitsToday = 0;
};

#endif