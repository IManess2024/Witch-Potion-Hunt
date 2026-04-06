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
    bowl.setPosition(level.cauldronArea.position + sf::Vector2f(0.f, 24.f));
    bowl.setFillColor(readyForExit ? sf::Color(82, 204, 103) : sf::Color(70, 70, 70));
    window.draw(bowl);

    sf::RectangleShape leg({10.f, 18.f});
    leg.setFillColor(sf::Color(45, 30, 20));

    leg.setPosition(level.cauldronArea.position + sf::Vector2f(8.f, 60.f));
    window.draw(leg);
    leg.setPosition(level.cauldronArea.position + sf::Vector2f(52.f, 60.f));
    window.draw(leg);

    for (int bubbleIndex = 0; bubbleIndex < 3; ++bubbleIndex)
    {
        sf::CircleShape bubble(7.f - static_cast<float>(bubbleIndex));
        bubble.setFillColor(readyForExit ? sf::Color(170, 255, 190) : sf::Color(120, 120, 120));
        bubble.setPosition(level.cauldronArea.position +
                           sf::Vector2f(12.f + bubbleIndex * 18.f, 10.f - bubbleIndex * 8.f));
        window.draw(bubble);
    }
}

inline const char* getBlockLetterPattern(char letter)
{
    switch (letter)
    {
    case 'A':
        return " XXX "
            "X   X"
            "X   X"
            "XXXXX"
            "X   X"
            "X   X"
            "X   X";
    case 'E':
        return "XXXXX"
            "X    "
            "X    "
            "XXXX "
            "X    "
            "X    "
            "XXXXX";
    case 'R':
        return "XXXX "
            "X   X"
            "X   X"
            "XXXX "
            "X X  "
            "X  X "
            "X   X";
    case 'S':
        return " XXXX"
            "X    "
            "X    "
            " XXX "
            "    X"
            "    X"
            "XXXX ";
    case 'T':
        return "XXXXX"
            "  X  "
            "  X  "
            "  X  "
            "  X  "
            "  X  "
            "  X  ";
    default:
        return "     "
            "     "
            "     "
            "     "
            "     "
            "     "
            "     ";
    }
}

inline void drawBlockLetter(sf::RenderWindow& window,
    char letter,
    sf::Vector2f topLeft,
    float pixelSize,
    sf::Color color)
{
    const char* pattern = getBlockLetterPattern(letter);

    for (int row = 0; row < 7; ++row)
    {
        for (int column = 0; column < 5; ++column)
        {
            if (pattern[row * 5 + column] == ' ')
            {
                continue;
            }

            sf::RectangleShape pixel({ pixelSize, pixelSize });
            pixel.setPosition(topLeft + sf::Vector2f(column * pixelSize, row * pixelSize));
            pixel.setFillColor(color);
            window.draw(pixel);
        }
    }
}

inline void drawBlockLabel(sf::RenderWindow& window,
    std::string_view text,
    sf::Vector2f topLeft,
    float pixelSize,
    float spacing,
    sf::Color color)
{
    sf::Vector2f cursor = topLeft;
    for (char letter : text)
    {
        if (letter == ' ')
        {
            cursor.x += 3.f * pixelSize;
            continue;
        }

        drawBlockLetter(window, letter, cursor, pixelSize, color);
        cursor.x += 5.f * pixelSize + spacing;
    }
}

inline void drawRestartOverlay(sf::RenderWindow& window,
    const sf::FloatRect& buttonBounds,
    bool hovered)
{
    const sf::Vector2u windowSize = window.getSize();

    sf::RectangleShape scrim({ static_cast<float>(windowSize.x), static_cast<float>(windowSize.y) });
    scrim.setFillColor(sf::Color(12, 12, 18, 180));
    window.draw(scrim);

    sf::RectangleShape panel({ 360.f, 220.f });
    panel.setOrigin({ 180.f, 110.f });
    panel.setPosition({ static_cast<float>(windowSize.x) * 0.5f, static_cast<float>(windowSize.y) * 0.5f });
    panel.setFillColor(sf::Color(46, 34, 30, 240));
    panel.setOutlineThickness(4.f);
    panel.setOutlineColor(sf::Color(156, 111, 72));
    window.draw(panel);

    sf::CircleShape potion(26.f);
    potion.setOrigin({ 26.f, 26.f });
    potion.setPosition(panel.getPosition() + sf::Vector2f(0.f, -42.f));
    potion.setFillColor(sf::Color(167, 68, 68));
    window.draw(potion);

    sf::RectangleShape cork({ 18.f, 10.f });
    cork.setOrigin({ 9.f, 5.f });
    cork.setPosition(potion.getPosition() + sf::Vector2f(0.f, -28.f));
    cork.setFillColor(sf::Color(130, 86, 48));
    window.draw(cork);

    sf::RectangleShape crack({ 8.f, 46.f });
    crack.setOrigin({ 4.f, 23.f });
    crack.setPosition(potion.getPosition());
    crack.setRotation(sf::degrees(30.f));
    crack.setFillColor(sf::Color(255, 230, 190));
    window.draw(crack);

    sf::RectangleShape button({ buttonBounds.size.x, buttonBounds.size.y });
    button.setPosition(buttonBounds.position);
    button.setFillColor(hovered ? sf::Color(92, 160, 109) : sf::Color(70, 129, 86));
    button.setOutlineThickness(4.f);
    button.setOutlineColor(sf::Color(212, 233, 190));
    window.draw(button);

    drawBlockLabel(window,
        "RESTART",
        buttonBounds.position + sf::Vector2f(31.f, 22.f),
        4.f,
        6.f,
        sf::Color(236, 248, 230));
}

