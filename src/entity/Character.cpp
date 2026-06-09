#include "entity/Character.h"
#include <algorithm>

Character::Character()
    : Entity()
    , attributes()
    , moveSpeed(0.0f)
    , name("Unknown")
{
}

Character::Character(float x, float y, const Attributes& attrs, float speed)
    : Entity(x, y)
    , attributes(attrs)
    , moveSpeed(speed)
    , name("Unknown")
{
}

void Character::takeDamage(int damage) {
    if (damage <= 0) return;
    attributes.san = std::max(0, attributes.san - damage);
}

Attributes& Character::getAttributes() {
    return attributes;
}

const Attributes& Character::getAttributes() const {
    return attributes;
}

void Character::setAttributes(const Attributes& attrs) {
    attributes = attrs;
}

float Character::getMoveSpeed() const {
    return moveSpeed;
}

void Character::setMoveSpeed(float speed) {
    moveSpeed = speed;
}

const std::string& Character::getName() const {
    return name;
}

void Character::setName(const std::string& name) {
    this->name = name;
}

void Character::clampAttributes() {
    attributes.energy   = std::clamp(attributes.energy,   0, 100);
    attributes.health   = std::clamp(attributes.health,   0, 100);
    attributes.gold     = std::clamp(attributes.gold,     0, 9999);
    attributes.san      = std::clamp(attributes.san,      0, 100);
    attributes.academic = std::clamp(attributes.academic, 0, 100);
    attributes.social   = std::clamp(attributes.social,   0, 100);
}
