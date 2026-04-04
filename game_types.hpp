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
    int  extraJumpsRemaining{0};
};

struct Ingredient
{
    sf::Vector2f position;
    bool         collected{false};
};

struct Level
{
    sf::Color                  backgroundColor;
    sf::Vector2f               spawnPosition;
    sf::FloatRect              cauldronArea;
    std::vector<sf::FloatRect> solids;
    std::vector<sf::FloatRect> climbWalls;
    std::vector<Ingredient>    ingredients;
};
