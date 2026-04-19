#pragma once

#include <array>
#include <cmath>
#include <string_view>

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
    case 'G':
        return " XXXX"
            "X    "
            "X    "
            "X XXX"
            "X   X"
            "X   X"
            " XXXX";
    case 'M':
        return "X   X"
            "XX XX"
            "X X X"
            "X   X"
            "X   X"
            "X   X"
            "X   X";
    case 'I':
        return "XXXXX"
            "  X  "
            "  X  "
            "  X  "
            "  X  "
            "  X  "
            "XXXXX";
    case 'N':
        return "X   X"
            "XX  X"
            "X X X"
            "X  XX"
            "X   X"
            "X   X"
            "X   X";
    case 'O':
        return " XXX "
            "X   X"
            "X   X"
            "X   X"
            "X   X"
            "X   X"
            " XXX ";
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
    case 'V':
        return "X   X"
            "X   X"
            "X   X"
            "X   X"
            "X   X"
            " X X "
            "  X  ";
    case 'U':
        return "X   X"
            "X   X"
            "X   X"
            "X   X"
            "X   X"
            "X   X"
            " XXX ";
    case 'W':
        return "X   X"
            "X   X"
            "X   X"
            "X   X"
            "X X X"
            "XX XX"
            "X   X";
    case 'Y':
        return "X   X"
            "X   X"
            " X X "
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

inline float getBlockLabelWidth(std::string_view text, float pixelSize, float spacing)
{
    float cursor = 0.f;
    float visibleWidth = 0.f;
    for (char letter : text)
    {
        if (letter == ' ')
        {
            cursor += 3.f * pixelSize;
            continue;
        }

        visibleWidth = cursor + 5.f * pixelSize;
        cursor += 5.f * pixelSize + spacing;
    }

    return visibleWidth;
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

inline void drawCenteredBlockLabel(sf::RenderWindow& window,
    std::string_view text,
    float centerX,
    float topY,
    float pixelSize,
    float spacing,
    sf::Color color)
{
    const float labelWidth = getBlockLabelWidth(text, pixelSize, spacing);
    drawBlockLabel(window, text, { centerX - labelWidth * 0.5f, topY }, pixelSize, spacing, color);
}

inline void drawConfetti(sf::RenderWindow& window, float elapsedSeconds)
{
    const sf::Vector2u windowSize = window.getSize();
    constexpr std::array<sf::Color, 6> colors = {
        sf::Color(244, 92, 92),
        sf::Color(255, 202, 76),
        sf::Color(86, 205, 132),
        sf::Color(91, 174, 255),
        sf::Color(214, 116, 255),
        sf::Color(255, 139, 83)
    };

    for (int index = 0; index < 90; ++index)
    {
        const float speed = 58.f + static_cast<float>((index % 7) * 15);
        const float drift = std::sin(elapsedSeconds * 1.7f + static_cast<float>(index)) * 24.f;
        const float x = std::fmod(static_cast<float>(index * 73), static_cast<float>(windowSize.x) + 120.f) - 60.f + drift;
        const float y = std::fmod(static_cast<float>(index * 47) + elapsedSeconds * speed,
            static_cast<float>(windowSize.y) + 140.f) - 70.f;

        sf::RectangleShape confetti({ 10.f + static_cast<float>(index % 3) * 3.f, 5.f });
        confetti.setOrigin({ confetti.getSize().x * 0.5f, confetti.getSize().y * 0.5f });
        confetti.setPosition({ x, y });
        confetti.setRotation(sf::degrees(std::fmod(elapsedSeconds * 120.f + static_cast<float>(index * 31), 360.f)));
        confetti.setFillColor(colors[static_cast<std::size_t>(index) % colors.size()]);
        window.draw(confetti);
    }
}

inline void drawRestartOverlay(sf::RenderWindow& window,
    const sf::FloatRect& buttonBounds,
    bool hovered)
{
    const sf::Vector2u windowSize = window.getSize();

    sf::RectangleShape scrim({ static_cast<float>(windowSize.x), static_cast<float>(windowSize.y) });
    scrim.setFillColor(sf::Color(12, 12, 18, 205));
    window.draw(scrim);

    sf::RectangleShape panel({ 460.f, 300.f });
    panel.setOrigin({ 230.f, 150.f });
    panel.setPosition({ static_cast<float>(windowSize.x) * 0.5f, static_cast<float>(windowSize.y) * 0.5f });
    panel.setFillColor(sf::Color(42, 31, 30, 245));
    panel.setOutlineThickness(4.f);
    panel.setOutlineColor(sf::Color(172, 120, 78));
    window.draw(panel);

    drawCenteredBlockLabel(window,
        "GAME OVER",
        panel.getPosition().x,
        panel.getPosition().y - 102.f,
        7.f,
        7.f,
        sf::Color(240, 94, 86));

    sf::RectangleShape divider({ 300.f, 4.f });
    divider.setOrigin({ 150.f, 2.f });
    divider.setPosition(panel.getPosition() + sf::Vector2f(0.f, -24.f));
    divider.setFillColor(sf::Color(172, 120, 78));
    window.draw(divider);

    sf::RectangleShape button({ buttonBounds.size.x, buttonBounds.size.y });
    button.setPosition(buttonBounds.position);
    button.setFillColor(hovered ? sf::Color(96, 169, 112) : sf::Color(70, 129, 86));
    button.setOutlineThickness(4.f);
    button.setOutlineColor(sf::Color(212, 233, 190));
    window.draw(button);

    drawCenteredBlockLabel(window,
        "RESTART",
        buttonBounds.position.x + buttonBounds.size.x * 0.5f,
        buttonBounds.position.y + 20.f,
        4.f,
        6.f,
        sf::Color(236, 248, 230));
}

inline void drawWinOverlay(sf::RenderWindow& window,
    const sf::FloatRect& buttonBounds,
    bool hovered,
    float elapsedSeconds)
{
    const sf::Vector2u windowSize = window.getSize();

    sf::RectangleShape scrim({ static_cast<float>(windowSize.x), static_cast<float>(windowSize.y) });
    scrim.setFillColor(sf::Color(10, 18, 18, 185));
    window.draw(scrim);

    drawConfetti(window, elapsedSeconds);

    sf::RectangleShape panel({ 500.f, 310.f });
    panel.setOrigin({ 250.f, 155.f });
    panel.setPosition({ static_cast<float>(windowSize.x) * 0.5f, static_cast<float>(windowSize.y) * 0.5f });
    panel.setFillColor(sf::Color(34, 49, 40, 245));
    panel.setOutlineThickness(4.f);
    panel.setOutlineColor(sf::Color(247, 200, 94));
    window.draw(panel);

    drawCenteredBlockLabel(window,
        "YOU WIN",
        panel.getPosition().x,
        panel.getPosition().y - 104.f,
        8.f,
        8.f,
        sf::Color(255, 220, 95));

    sf::RectangleShape divider({ 320.f, 4.f });
    divider.setOrigin({ 160.f, 2.f });
    divider.setPosition(panel.getPosition() + sf::Vector2f(0.f, -22.f));
    divider.setFillColor(sf::Color(112, 199, 131));
    window.draw(divider);

    sf::CircleShape sparkle(7.f, 4);
    sparkle.setOrigin({ 7.f, 7.f });
    sparkle.setFillColor(sf::Color(255, 238, 150));

    sparkle.setPosition(panel.getPosition() + sf::Vector2f(-192.f, -95.f));
    sparkle.setRotation(sf::degrees(45.f + elapsedSeconds * 90.f));
    window.draw(sparkle);

    sparkle.setPosition(panel.getPosition() + sf::Vector2f(195.f, -54.f));
    sparkle.setRotation(sf::degrees(18.f + elapsedSeconds * 110.f));
    window.draw(sparkle);

    sparkle.setPosition(panel.getPosition() + sf::Vector2f(-158.f, 42.f));
    sparkle.setRotation(sf::degrees(70.f + elapsedSeconds * 75.f));
    window.draw(sparkle);

    sf::RectangleShape button({ buttonBounds.size.x, buttonBounds.size.y });
    button.setPosition(buttonBounds.position);
    button.setFillColor(hovered ? sf::Color(97, 178, 119) : sf::Color(73, 146, 94));
    button.setOutlineThickness(4.f);
    button.setOutlineColor(sf::Color(236, 225, 153));
    window.draw(button);

    drawCenteredBlockLabel(window,
        "RESTART",
        buttonBounds.position.x + buttonBounds.size.x * 0.5f,
        buttonBounds.position.y + 20.f,
        4.f,
        6.f,
        sf::Color(246, 250, 225));
}
