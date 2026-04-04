#pragma once

#include "game_types.hpp"

constexpr float kMoveSpeed      = 260.f;
constexpr float kJumpSpeed      = -590.f;
constexpr float kGravity        = 1200.f;
constexpr float kClimbSpeed     = 180.f;
constexpr float kWallSlideSpeed = 120.f;
constexpr float kFixedDt        = 60.f;

inline sf::FloatRect getPlayerBounds(const Player& player)
{
    return sf::FloatRect(player.position, player.size);
}

inline bool allIngredientsCollected(const Level& level)
{
    for (const Ingredient& ingredient : level.ingredients)
    {
        if (!ingredient.collected)
        {
            return false;
        }
    }

    return true;
}

inline void updateClimbWallContact(Player& player, const Level& level)
{
    player.touchingClimbWall = false;

    if (!player.canClimb)
    {
        return;
    }

    sf::FloatRect wallCheck = getPlayerBounds(player);
    wallCheck.position.x -= 3.f;
    wallCheck.size.x += 6.f;

    for (const sf::FloatRect& wall : level.climbWalls)
    {
        if (wallCheck.findIntersection(wall))
        {
            player.touchingClimbWall = true;
            return;
        }
    }
}

inline void resolveHorizontalCollisions(Player& player, const Level& level, float dt)
{
    player.position.x += player.velocity.x / dt;

    sf::FloatRect playerBounds = getPlayerBounds(player);

    for (const sf::FloatRect& solid : level.solids)
    {
        if (!playerBounds.findIntersection(solid))
        {
            continue;
        }

        if (player.velocity.x > 0.f)
        {
            player.position.x = solid.position.x - player.size.x;
        }
        else if (player.velocity.x < 0.f)
        {
            player.position.x = solid.position.x + solid.size.x;
        }

        player.velocity.x = 0.f;
        playerBounds      = getPlayerBounds(player);
    }
}

inline void resolveVerticalCollisions(Player& player, const Level& level, float dt)
{
    player.onGround = false;
    player.position.y += player.velocity.y / dt;

    sf::FloatRect playerBounds = getPlayerBounds(player);

    for (const sf::FloatRect& solid : level.solids)
    {
        if (!playerBounds.findIntersection(solid))
        {
            continue;
        }

        if (player.velocity.y > 0.f)
        {
            player.position.y          = solid.position.y - player.size.y;
            player.velocity.y          = 0.f;
            player.onGround            = true;
            player.extraJumpsRemaining = player.canDoubleJump ? 1 : 0;
        }
        else if (player.velocity.y < 0.f)
        {
            player.position.y = solid.position.y + solid.size.y;
            player.velocity.y = 0.f;
        }

        playerBounds = getPlayerBounds(player);
    }
}
