#pragma once

#include "game_types.hpp"

constexpr float kMoveSpeed = 260.f; 
constexpr float kJumpSpeed = -590.0f;
constexpr float kGravity        = 1200.f;
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
            player.position.y = solid.position.y - player.size.y;
            player.velocity.y = 0.f;
            player.onGround   = true;
        }
        else if (player.velocity.y < 0.f)
        {
            player.position.y = solid.position.y + solid.size.y;
            player.velocity.y = 0.f;
        }

        playerBounds = getPlayerBounds(player);
    }

}

void CollectIngredients(Player& player, Level& level)
{
    for (Ingredient& ingredient : level.ingredients)
    {
        if (ingredient.collected)
        {
            continue;
        }
        sf::FloatRect IngredientBounds(ingredient.position - sf::Vector2f(10.0f, 10.0f), { 20.0f, 20.0f });
        if (getPlayerBounds(player).findIntersection(IngredientBounds))
        {
            ingredient.collected = true;
        }


    }
}

inline void refreshabilitesforlevel(Player& player, std::size_t levelindex)
{
    player.canDoubleJump = levelindex >= 1;
    player.canClimb = levelindex >= 2;
    player.extraJumpsRemaining = player.canDoubleJump ? 1 : 0;
    
}
