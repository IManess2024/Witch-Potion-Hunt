#pragma once

#include <vector>

#include <SFML/Graphics.hpp>

struct Player
{
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Vector2f size{34.f, 52.f};

    bool onGround{false};
    bool touchingClimbWall{false};
    bool canDoubleJump{false};
    bool canClimb{false};
    bool facingRight{true};
    int  extraJumpsRemaining{0};
};

struct Ingredient
{
    sf::Vector2f position;
    bool         collected{false};
};

enum class MobType
{
    Bat,
    Imp,
    Golem
};

struct Mob
{
    MobType type{MobType::Bat};
    sf::Vector2f position;
    sf::Vector2f spawnPosition;
    sf::Vector2f velocity;
    sf::Vector2f size{36.f, 28.f};
    float patrolMinX{0.f};
    float patrolMaxX{0.f};
    float baseY{0.f};
    float phase{0.f};
    int health{30};
    int maxHealth{30};
    bool alive{true};
    bool facingRight{true};
};

struct Level
{
    sf::Color                  backgroundColor;
    sf::Vector2f               spawnPosition;
    sf::FloatRect              cauldronArea;
    std::vector<sf::FloatRect> solids;
    std::vector<sf::FloatRect> climbWalls;
    std::vector<Ingredient>    ingredients;
    std::vector<Mob>           mobs;
};
