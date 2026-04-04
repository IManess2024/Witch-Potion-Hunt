#pragma once

#include "game_types.hpp"
#include "gameplay.hpp"

inline void drawPlayer(sf::RenderWindow& window, const Player& player)
{
    sf::CircleShape head(11.f);
    head.setPosition(player.position + sf::Vector2f(6.f, 2.f));
    head.setFillColor(sf::Color(246, 215, 189));
    window.draw(head);

    sf::RectangleShape dress({player.size.x, player.size.y - 14.f});
    dress.setPosition(player.position + sf::Vector2f(0.f, 14.f));
    dress.setFillColor(sf::Color(120, 64, 170));
    window.draw(dress);

    sf::ConvexShape hat(3);
    hat.setPoint(0, {player.size.x * 0.5f, -16.f});
    hat.setPoint(1, {2.f, 15.f});
    hat.setPoint(2, {player.size.x - 2.f, 15.f});
    hat.setPosition(player.position + sf::Vector2f(0.f, -2.f));
    hat.setFillColor(sf::Color(42, 23, 64));
    window.draw(hat);

    sf::RectangleShape broom({28.f, 5.f});
    broom.setPosition(player.position + sf::Vector2f(player.size.x - 2.f, 30.f));
    broom.setFillColor(sf::Color(130, 86, 48));
    window.draw(broom);

    sf::RectangleShape broomBrush({10.f, 10.f});
    broomBrush.setPosition(player.position + sf::Vector2f(player.size.x + 22.f, 27.f));
    broomBrush.setFillColor(sf::Color(189, 156, 79));
    window.draw(broomBrush);
}

inline void drawHud(sf::RenderWindow& window, const Level& level, const Player& player)
{
    for (std::size_t i = 0; i < level.ingredients.size(); ++i)
    {
        sf::CircleShape icon(8.f);
        icon.setPosition({18.f + static_cast<float>(i) * 24.f, 16.f});
        icon.setFillColor(level.ingredients[i].collected ? sf::Color(80, 80, 80)
                                                         : sf::Color(255, 180, 80));
        window.draw(icon);
    }

    sf::RectangleShape doubleJumpIcon({22.f, 10.f});
    doubleJumpIcon.setPosition({18.f, 42.f});
    doubleJumpIcon.setFillColor(player.canDoubleJump ? sf::Color(132, 105, 255)
                                                     : sf::Color(60, 60, 60));
    window.draw(doubleJumpIcon);

    sf::RectangleShape climbIcon({22.f, 10.f});
    climbIcon.setPosition({48.f, 42.f});
    climbIcon.setFillColor(player.canClimb ? sf::Color(91, 196, 116)
                                           : sf::Color(60, 60, 60));
    window.draw(climbIcon);
}

inline void drawCauldron(sf::RenderWindow& window, const Level& level, bool readyForExit)
{
    sf::RectangleShape bowl({70.f, 38.f});
    bowl.setPosition(level.cauldronArea.left, level.cauldronArea.top + 24.f);
    bowl.setFillColor(readyForExit ? sf::Color(82, 204, 103) : sf::Color(70, 70, 70));
    window.draw(bowl);

    sf::RectangleShape leg({10.f, 18.f});
    leg.setFillColor(sf::Color(45, 30, 20));

    leg.setPosition(level.cauldronArea.left + 8.f, level.cauldronArea.top + 60.f);
    window.draw(leg);
    leg.setPosition(level.cauldronArea.left + 52.f, level.cauldronArea.top + 60.f);
    window.draw(leg);

    for (int bubbleIndex = 0; bubbleIndex < 3; ++bubbleIndex)
    {
        sf::CircleShape bubble(7.f - static_cast<float>(bubbleIndex));
        bubble.setFillColor(readyForExit ? sf::Color(170, 255, 190) : sf::Color(120, 120, 120));
        bubble.setPosition(level.cauldronArea.left + 12.f + bubbleIndex * 18.f,
                           level.cauldronArea.top + 10.f - bubbleIndex * 8.f);
        window.draw(bubble);
    }
}
