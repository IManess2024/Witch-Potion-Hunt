#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <string>
#include <string_view>

#include "game_types.hpp"
#include "gameplay.hpp"

inline sf::Color blendColor(sf::Color a, sf::Color b, float t)
{
    t = std::clamp(t, 0.0f, 1.0f);
    return sf::Color(
        static_cast<std::uint8_t>(static_cast<float>(a.r) + (static_cast<float>(b.r) - static_cast<float>(a.r)) * t),
        static_cast<std::uint8_t>(static_cast<float>(a.g) + (static_cast<float>(b.g) - static_cast<float>(a.g)) * t),
        static_cast<std::uint8_t>(static_cast<float>(a.b) + (static_cast<float>(b.b) - static_cast<float>(a.b)) * t),
        static_cast<std::uint8_t>(static_cast<float>(a.a) + (static_cast<float>(b.a) - static_cast<float>(a.a)) * t));
}

inline void drawPixelRect(sf::RenderWindow& window, sf::Vector2f position, sf::Vector2f size, sf::Color color)
{
    sf::RectangleShape rect(size);
    rect.setPosition(position);
    rect.setFillColor(color);
    window.draw(rect);
}

inline void drawGradientSky(sf::RenderWindow& window, sf::Color topColor, sf::Color bottomColor)
{
    const sf::Vector2u windowSize = window.getSize();
    constexpr int kBands = 18;
    const float bandHeight = static_cast<float>(windowSize.y) / static_cast<float>(kBands);

    for (int band = 0; band < kBands; ++band)
    {
        const float t = static_cast<float>(band) / static_cast<float>(kBands - 1);
        drawPixelRect(window,
            { 0.0f, static_cast<float>(band) * bandHeight },
            { static_cast<float>(windowSize.x), bandHeight + 1.0f },
            blendColor(topColor, bottomColor, t));
    }
}

inline void drawDitherPixels(sf::RenderWindow& window, sf::Color color, int seed, float yMin, float yMax)
{
    const sf::Vector2u windowSize = window.getSize();
    for (int index = 0; index < 72; ++index)
    {
        const float x = std::fmod(static_cast<float>(index * 79 + seed * 37), static_cast<float>(windowSize.x));
        const float y = yMin + std::fmod(static_cast<float>(index * 53 + seed * 61), yMax - yMin);
        const float size = 2.0f + static_cast<float>((index + seed) % 3) * 2.0f;
        drawPixelRect(window, { x, y }, { size, size }, color);
    }
}

inline void drawMountainLayer(sf::RenderWindow& window, float baseY, sf::Color color, int seed)
{
    const sf::Vector2u windowSize = window.getSize();
    for (int index = -1; index < 7; ++index)
    {
        const float width = 260.0f + static_cast<float>((index + seed + 6) % 3) * 70.0f;
        const float x = static_cast<float>(index) * 210.0f + static_cast<float>(seed % 5) * 18.0f;
        const float peakY = baseY - 115.0f - static_cast<float>((index + seed + 3) % 4) * 32.0f;

        sf::ConvexShape mountain(3);
        mountain.setPoint(0, { x, baseY });
        mountain.setPoint(1, { x + width * 0.5f, peakY });
        mountain.setPoint(2, { x + width, baseY });
        mountain.setFillColor(color);
        window.draw(mountain);
    }

    drawPixelRect(window,
        { 0.0f, baseY - 4.0f },
        { static_cast<float>(windowSize.x), 8.0f },
        sf::Color(color.r, color.g, color.b, 95));
}

inline void drawPineLayer(sf::RenderWindow& window, float groundY, sf::Color color, int seed)
{
    const sf::Vector2u windowSize = window.getSize();
    for (int index = 0; index < 18; ++index)
    {
        const float x = std::fmod(static_cast<float>(index * 91 + seed * 43), static_cast<float>(windowSize.x) + 120.0f) - 60.0f;
        const float height = 68.0f + static_cast<float>((index + seed) % 5) * 18.0f;
        const float width = 46.0f + static_cast<float>((index + seed) % 3) * 10.0f;

        sf::ConvexShape tree(3);
        tree.setPoint(0, { x, groundY });
        tree.setPoint(1, { x + width * 0.5f, groundY - height });
        tree.setPoint(2, { x + width, groundY });
        tree.setFillColor(color);
        window.draw(tree);

        drawPixelRect(window, { x + width * 0.45f, groundY - 18.0f }, { 7.0f, 22.0f }, sf::Color(30, 24, 30, 170));
    }
}

inline void drawLevelBackground(sf::RenderWindow& window, std::size_t levelIndex, float elapsedSeconds)
{
    const sf::Vector2u windowSize = window.getSize();

    if (levelIndex == 0)
    {
        drawGradientSky(window, sf::Color(34, 24, 58), sf::Color(82, 52, 84));
        drawDitherPixels(window, sf::Color(236, 221, 170, 145), 1, 24.0f, 250.0f);

        sf::CircleShape moon(34.0f, 24);
        moon.setOrigin({ 34.0f, 34.0f });
        moon.setPosition({ 1030.0f, 94.0f });
        moon.setFillColor(sf::Color(239, 224, 169, 215));
        window.draw(moon);

        for (int cloud = 0; cloud < 5; ++cloud)
        {
            const float x = 120.0f + static_cast<float>(cloud) * 225.0f + std::sin(elapsedSeconds * 0.25f + static_cast<float>(cloud)) * 10.0f;
            const float y = 120.0f + static_cast<float>(cloud % 2) * 62.0f;
            drawPixelRect(window, { x, y }, { 86.0f, 14.0f }, sf::Color(114, 78, 121, 86));
            drawPixelRect(window, { x + 26.0f, y - 12.0f }, { 74.0f, 14.0f }, sf::Color(124, 88, 132, 75));
        }

        drawPineLayer(window, 580.0f, sf::Color(25, 31, 39, 220), 2);
        drawPineLayer(window, 620.0f, sf::Color(18, 24, 31, 235), 6);
        return;
    }

    if (levelIndex == 1)
    {
        drawGradientSky(window, sf::Color(18, 38, 64), sf::Color(54, 77, 102));
        drawDitherPixels(window, sf::Color(151, 210, 226, 85), 7, 40.0f, 270.0f);
        drawMountainLayer(window, 465.0f, sf::Color(32, 58, 82, 210), 3);
        drawMountainLayer(window, 535.0f, sf::Color(25, 45, 67, 230), 8);

        for (int mist = 0; mist < 7; ++mist)
        {
            const float y = 225.0f + static_cast<float>(mist) * 46.0f;
            const float x = std::fmod(elapsedSeconds * 9.0f + static_cast<float>(mist) * 190.0f, static_cast<float>(windowSize.x) + 180.0f) - 120.0f;
            drawPixelRect(window, { x, y }, { 150.0f, 7.0f }, sf::Color(151, 190, 203, 48));
        }
        return;
    }

    drawGradientSky(window, sf::Color(11, 38, 36), sf::Color(31, 70, 48));
    drawDitherPixels(window, sf::Color(141, 235, 126, 120), 11, 65.0f, 500.0f);

    for (int trunk = 0; trunk < 11; ++trunk)
    {
        const float x = std::fmod(static_cast<float>(trunk * 137 + 19), static_cast<float>(windowSize.x) + 80.0f) - 40.0f;
        const float width = 18.0f + static_cast<float>(trunk % 3) * 8.0f;
        drawPixelRect(window, { x, 108.0f }, { width, static_cast<float>(windowSize.y) }, sf::Color(21, 39, 31, 180));
        drawPixelRect(window, { x + width - 5.0f, 108.0f }, { 5.0f, static_cast<float>(windowSize.y) }, sf::Color(38, 70, 45, 135));
    }

    for (int vine = 0; vine < 17; ++vine)
    {
        const float x = std::fmod(static_cast<float>(vine * 83 + 31), static_cast<float>(windowSize.x));
        const float height = 70.0f + static_cast<float>(vine % 5) * 28.0f;
        drawPixelRect(window, { x, 0.0f }, { 5.0f, height }, sf::Color(54, 118, 63, 150));
        drawPixelRect(window, { x - 6.0f, height - 12.0f }, { 12.0f, 5.0f }, sf::Color(78, 154, 84, 130));
    }
}

inline void drawSolidPlatform(sf::RenderWindow& window, const sf::FloatRect& solid, std::size_t levelIndex)
{
    const sf::Color baseColor = levelIndex == 0
        ? sf::Color(87, 65, 52)
        : (levelIndex == 1 ? sf::Color(62, 78, 91) : sf::Color(54, 83, 58));
    const sf::Color darkColor = levelIndex == 0
        ? sf::Color(48, 37, 35)
        : (levelIndex == 1 ? sf::Color(35, 49, 61) : sf::Color(31, 52, 38));
    const sf::Color topColor = levelIndex == 0
        ? sf::Color(96, 120, 62)
        : (levelIndex == 1 ? sf::Color(98, 123, 137) : sf::Color(65, 143, 80));
    const sf::Color highlightColor = levelIndex == 0
        ? sf::Color(128, 95, 70)
        : (levelIndex == 1 ? sf::Color(111, 139, 155) : sf::Color(81, 164, 91));

    drawPixelRect(window, solid.position, solid.size, baseColor);
    drawPixelRect(window, solid.position, { solid.size.x, 7.0f }, topColor);
    drawPixelRect(window, solid.position + sf::Vector2f(0.0f, solid.size.y - 8.0f), { solid.size.x, 8.0f }, darkColor);

    for (int x = 8; x < static_cast<int>(solid.size.x) - 8; x += 24)
    {
        const float chipY = 15.0f + static_cast<float>((x + static_cast<int>(solid.position.y)) % 3) * 11.0f;
        if (chipY + 5.0f < solid.size.y)
        {
            drawPixelRect(window,
                solid.position + sf::Vector2f(static_cast<float>(x), chipY),
                { 10.0f, 4.0f },
                highlightColor);
        }
    }

    for (int x = 14; x < static_cast<int>(solid.size.x) - 8; x += 38)
    {
        drawPixelRect(window,
            solid.position + sf::Vector2f(static_cast<float>(x), 4.0f),
            { 14.0f, 3.0f },
            sf::Color(178, 174, 117, 100));
    }
}

inline void drawClimbWall(sf::RenderWindow& window, const sf::FloatRect& wall)
{
    drawPixelRect(window, wall.position, wall.size, sf::Color(45, 99, 61));
    drawPixelRect(window, wall.position, { wall.size.x, wall.size.y }, sf::Color(31, 69, 46, 80));

    for (int y = 8; y < static_cast<int>(wall.size.y); y += 22)
    {
        const float offset = static_cast<float>((y / 22) % 2) * 13.0f;
        drawPixelRect(window, wall.position + sf::Vector2f(7.0f + offset, static_cast<float>(y)), { 7.0f, 18.0f }, sf::Color(87, 179, 94));
        drawPixelRect(window, wall.position + sf::Vector2f(18.0f - offset * 0.35f, static_cast<float>(y + 7)), { 14.0f, 5.0f }, sf::Color(111, 207, 116));
    }
}

inline void drawIngredientGem(sf::RenderWindow& window, const Ingredient& ingredient)
{
    sf::ConvexShape gem(4);
    gem.setPoint(0, { 0.0f, -13.0f });
    gem.setPoint(1, { 13.0f, 0.0f });
    gem.setPoint(2, { 0.0f, 13.0f });
    gem.setPoint(3, { -13.0f, 0.0f });
    gem.setPosition(ingredient.position);
    gem.setFillColor(sf::Color(255, 178, 72));
    window.draw(gem);

    drawPixelRect(window, ingredient.position + sf::Vector2f(-3.0f, -10.0f), { 5.0f, 20.0f }, sf::Color(255, 224, 108, 170));
    drawPixelRect(window, ingredient.position + sf::Vector2f(2.0f, -4.0f), { 7.0f, 4.0f }, sf::Color(255, 245, 174, 180));
}

inline void drawPlayer(sf::RenderWindow& window, const Player& player, float elapsedSeconds)
{
    const bool moving = std::abs(player.velocity.x) > 1.0f;
    const float direction = player.facingRight ? 1.0f : -1.0f;
    const float walkCycle = moving ? std::sin(elapsedSeconds * 12.0f) : 0.0f;
    const float bob = player.onGround && moving ? std::abs(walkCycle) * 2.0f : 0.0f;
    const float robeSway = moving ? walkCycle * 2.0f : 0.0f;
    const sf::Vector2f origin = player.position + sf::Vector2f(0.0f, -bob);

    auto localX = [&](float x, float width)
    {
        return player.facingRight ? origin.x + x : origin.x + player.size.x - x - width;
    };

    auto localPointX = [&](float x)
    {
        return player.facingRight ? origin.x + x : origin.x + player.size.x - x;
    };

    auto drawLocalRect = [&](float x, float y, float width, float height, sf::Color color)
    {
        drawPixelRect(window, { localX(x, width), origin.y + y }, { width, height }, color);
    };

    sf::CircleShape shadowShape(1.0f, 24);
    shadowShape.setScale({ 19.0f, 4.0f });
    shadowShape.setOrigin({ 1.0f, 1.0f });
    shadowShape.setPosition(player.position + sf::Vector2f(player.size.x * 0.5f, player.size.y + 3.0f));
    shadowShape.setFillColor(sf::Color(0, 0, 0, 75));
    window.draw(shadowShape);

    const float legOffset = player.onGround && moving ? walkCycle * 3.0f : 0.0f;
    drawLocalRect(7.0f + legOffset, 45.0f, 7.0f, 11.0f, sf::Color(42, 30, 58));
    drawLocalRect(20.0f - legOffset, 45.0f, 7.0f, 11.0f, sf::Color(42, 30, 58));

    sf::ConvexShape robe(4);
    robe.setPoint(0, { localPointX(4.0f), origin.y + 18.0f });
    robe.setPoint(1, { localPointX(30.0f), origin.y + 18.0f });
    robe.setPoint(2, { localPointX(32.0f + robeSway), origin.y + 48.0f });
    robe.setPoint(3, { localPointX(2.0f + robeSway), origin.y + 48.0f });
    robe.setFillColor(sf::Color(112, 52, 164));
    window.draw(robe);

    drawLocalRect(8.0f, 20.0f, 8.0f, 27.0f, sf::Color(142, 73, 190));
    drawLocalRect(19.0f, 20.0f, 5.0f, 26.0f, sf::Color(72, 38, 116));
    drawLocalRect(5.0f, 47.0f, 24.0f, 5.0f, sf::Color(58, 34, 97));

    const float armSwing = moving ? walkCycle * 4.0f : 0.0f;
    drawLocalRect(1.0f, 22.0f + armSwing * 0.25f, 7.0f, 17.0f, sf::Color(83, 45, 130));
    drawLocalRect(26.0f, 22.0f - armSwing * 0.25f, 7.0f, 17.0f, sf::Color(83, 45, 130));
    drawLocalRect(0.0f, 36.0f + armSwing * 0.25f, 8.0f, 5.0f, sf::Color(246, 215, 189));

    sf::CircleShape head(11.f);
    head.setPosition(origin + sf::Vector2f(6.f, 1.f));
    head.setFillColor(sf::Color(246, 215, 189));
    window.draw(head);

    drawLocalRect(6.0f, 11.0f, 5.0f, 13.0f, sf::Color(74, 42, 56));
    drawLocalRect(23.0f, 11.0f, 5.0f, 12.0f, sf::Color(74, 42, 56));
    drawLocalRect(player.facingRight ? 22.0f : 8.0f, 10.0f, 3.0f, 3.0f, sf::Color(37, 28, 34));
    drawLocalRect(player.facingRight ? 25.0f : 5.0f, 17.0f, 5.0f, 2.0f, sf::Color(196, 92, 111));

    sf::ConvexShape hat(3);
    hat.setPoint(0, { player.size.x * 0.5f + direction * (moving ? walkCycle * 1.5f : 0.0f), -17.f });
    hat.setPoint(1, { 3.f, 14.f });
    hat.setPoint(2, { player.size.x - 3.f, 14.f });
    hat.setPosition(origin + sf::Vector2f(0.f, -3.f));
    hat.setFillColor(sf::Color(42, 23, 64));
    window.draw(hat);

    drawLocalRect(0.0f, 10.0f, 34.0f, 5.0f, sf::Color(35, 20, 54));
    drawLocalRect(13.0f, -2.0f, 9.0f, 4.0f, sf::Color(116, 66, 171));

    sf::RectangleShape broom({28.f, 5.f});
    broom.setOrigin({ player.facingRight ? 0.0f : 28.0f, 2.5f });
    broom.setPosition(origin +
        sf::Vector2f(player.facingRight ? player.size.x - 2.0f : 2.0f, 32.5f - armSwing * 0.12f));
    broom.setRotation(sf::degrees(direction * (moving ? walkCycle * 4.0f : 0.0f)));
    broom.setFillColor(sf::Color(130, 86, 48));
    window.draw(broom);

    sf::RectangleShape broomBrush({10.f, 10.f});
    broomBrush.setOrigin({ player.facingRight ? 0.0f : 10.0f, 5.0f });
    broomBrush.setPosition(origin +
        sf::Vector2f(player.facingRight ? player.size.x + 22.0f : -22.0f, 32.0f - armSwing * 0.12f));
    broomBrush.setRotation(sf::degrees(direction * (moving ? walkCycle * 4.0f : 0.0f)));
    broomBrush.setFillColor(sf::Color(189, 156, 79));
    window.draw(broomBrush);

    const float glowPulse = (std::sin(elapsedSeconds * 9.0f) + 1.0f) * 0.5f;
    sf::CircleShape wandGlow(4.0f + glowPulse * 1.5f, 8);
    wandGlow.setOrigin({ wandGlow.getRadius(), wandGlow.getRadius() });
    wandGlow.setPosition(player.position + sf::Vector2f(player.facingRight ? player.size.x + 34.0f : -34.0f, 29.0f - bob));
    wandGlow.setFillColor(sf::Color(245, 226, 124, static_cast<std::uint8_t>(180 + glowPulse * 60.0f)));
    window.draw(wandGlow);
}

inline void drawPortal(sf::RenderWindow& window,
    const Level& level,
    bool readyForExit,
    std::size_t levelIndex,
    float elapsedSeconds)
{
    const sf::Vector2f center = level.cauldronArea.position + sf::Vector2f(level.cauldronArea.size.x * 0.5f, level.cauldronArea.size.y * 0.5f);
    const float pulse = (std::sin(elapsedSeconds * 4.0f) + 1.0f) * 0.5f;

    if (levelIndex == 0)
    {
        sf::CircleShape outer(42.0f, 7);
        outer.setOrigin({ 42.0f, 42.0f });
        outer.setPosition(center);
        outer.setRotation(sf::degrees(elapsedSeconds * 20.0f));
        outer.setFillColor(sf::Color(62, 38, 83, 230));
        outer.setOutlineThickness(4.0f);
        outer.setOutlineColor(readyForExit ? sf::Color(238, 196, 88) : sf::Color(106, 83, 115));
        window.draw(outer);

        sf::CircleShape inner(24.0f + pulse * 4.0f, 20);
        inner.setOrigin({ inner.getRadius(), inner.getRadius() });
        inner.setPosition(center);
        inner.setFillColor(readyForExit ? sf::Color(151, 81, 220, 185) : sf::Color(63, 59, 68, 170));
        window.draw(inner);

        drawPixelRect(window, center + sf::Vector2f(-18.0f, 34.0f), { 36.0f, 9.0f }, sf::Color(58, 36, 51));
        return;
    }

    if (levelIndex == 1)
    {
        sf::ConvexShape crystal(6);
        crystal.setPoint(0, { 0.0f, -48.0f });
        crystal.setPoint(1, { 30.0f, -20.0f });
        crystal.setPoint(2, { 24.0f, 30.0f });
        crystal.setPoint(3, { 0.0f, 48.0f });
        crystal.setPoint(4, { -24.0f, 30.0f });
        crystal.setPoint(5, { -30.0f, -20.0f });
        crystal.setPosition(center);
        crystal.setFillColor(readyForExit ? sf::Color(87, 219, 238, 180) : sf::Color(76, 89, 101, 185));
        crystal.setOutlineThickness(4.0f);
        crystal.setOutlineColor(readyForExit ? sf::Color(213, 246, 255) : sf::Color(117, 134, 147));
        window.draw(crystal);

        drawPixelRect(window, center + sf::Vector2f(-7.0f, -35.0f), { 14.0f, 68.0f }, readyForExit ? sf::Color(179, 249, 255, 110) : sf::Color(120, 136, 147, 80));
        drawPixelRect(window, center + sf::Vector2f(-34.0f, 42.0f), { 68.0f, 8.0f }, sf::Color(39, 56, 68));
        return;
    }

    sf::CircleShape ring(46.0f, 28);
    ring.setOrigin({ 46.0f, 46.0f });
    ring.setPosition(center);
    ring.setFillColor(sf::Color(21, 46, 33, 230));
    ring.setOutlineThickness(7.0f);
    ring.setOutlineColor(readyForExit ? sf::Color(82, 228, 112) : sf::Color(58, 100, 65));
    window.draw(ring);

    sf::CircleShape core(26.0f + pulse * 5.0f, 24);
    core.setOrigin({ core.getRadius(), core.getRadius() });
    core.setPosition(center);
    core.setFillColor(readyForExit ? sf::Color(72, 210, 101, 165) : sf::Color(44, 69, 50, 150));
    window.draw(core);

    for (int root = 0; root < 5; ++root)
    {
        const float x = -38.0f + static_cast<float>(root) * 18.0f;
        drawPixelRect(window, center + sf::Vector2f(x, 38.0f + static_cast<float>(root % 2) * 5.0f), { 24.0f, 6.0f }, sf::Color(42, 74, 43));
    }
}

inline void drawMob(sf::RenderWindow& window, const Mob& mob, std::size_t levelIndex, float elapsedSeconds)
{
    if (!mob.alive)
    {
        return;
    }

    const float direction = mob.facingRight ? 1.0f : -1.0f;
    const sf::Vector2f center = mob.position + mob.size * 0.5f;

    if (mob.type == MobType::Bat)
    {
        const float flap = std::sin(elapsedSeconds * 14.0f + mob.phase) * 7.0f;
        sf::CircleShape body(13.0f, 10);
        body.setOrigin({ 13.0f, 13.0f });
        body.setPosition(center);
        body.setFillColor(levelIndex == 0 ? sf::Color(70, 41, 95) : sf::Color(39, 64, 90));
        window.draw(body);

        sf::ConvexShape wing(3);
        wing.setFillColor(levelIndex == 0 ? sf::Color(104, 58, 139) : sf::Color(67, 104, 132));
        wing.setPoint(0, center + sf::Vector2f(-8.0f, 0.0f));
        wing.setPoint(1, center + sf::Vector2f(-30.0f, -8.0f - flap));
        wing.setPoint(2, center + sf::Vector2f(-22.0f, 13.0f + flap * 0.3f));
        window.draw(wing);
        wing.setPoint(0, center + sf::Vector2f(8.0f, 0.0f));
        wing.setPoint(1, center + sf::Vector2f(30.0f, -8.0f - flap));
        wing.setPoint(2, center + sf::Vector2f(22.0f, 13.0f + flap * 0.3f));
        window.draw(wing);

        drawPixelRect(window, center + sf::Vector2f(direction * 4.0f - 2.0f, -4.0f), { 4.0f, 4.0f }, sf::Color(255, 76, 93));
    }
    else if (mob.type == MobType::Imp)
    {
        drawPixelRect(window, mob.position + sf::Vector2f(5.0f, 10.0f), { mob.size.x - 10.0f, mob.size.y - 8.0f }, sf::Color(166, 54, 67));
        drawPixelRect(window, mob.position + sf::Vector2f(9.0f, 3.0f), { mob.size.x - 18.0f, 18.0f }, sf::Color(206, 74, 74));
        drawPixelRect(window, mob.position + sf::Vector2f(direction > 0.0f ? 22.0f : 8.0f, 9.0f), { 5.0f, 4.0f }, sf::Color(255, 229, 91));
        drawPixelRect(window, mob.position + sf::Vector2f(4.0f, 0.0f), { 8.0f, 8.0f }, sf::Color(92, 34, 49));
        drawPixelRect(window, mob.position + sf::Vector2f(mob.size.x - 12.0f, 0.0f), { 8.0f, 8.0f }, sf::Color(92, 34, 49));
        drawPixelRect(window, mob.position + sf::Vector2f(7.0f, mob.size.y - 3.0f), { 7.0f, 7.0f }, sf::Color(51, 34, 44));
        drawPixelRect(window, mob.position + sf::Vector2f(mob.size.x - 14.0f, mob.size.y - 3.0f), { 7.0f, 7.0f }, sf::Color(51, 34, 44));
    }
    else
    {
        drawPixelRect(window, mob.position + sf::Vector2f(6.0f, 14.0f), { mob.size.x - 12.0f, mob.size.y - 16.0f }, sf::Color(75, 91, 77));
        drawPixelRect(window, mob.position + sf::Vector2f(12.0f, 2.0f), { mob.size.x - 24.0f, 24.0f }, sf::Color(95, 116, 93));
        drawPixelRect(window, mob.position + sf::Vector2f(direction > 0.0f ? 32.0f : 20.0f, 10.0f), { 7.0f, 6.0f }, sf::Color(139, 255, 132));
        drawPixelRect(window, mob.position + sf::Vector2f(0.0f, 28.0f), { 13.0f, 24.0f }, sf::Color(63, 78, 64));
        drawPixelRect(window, mob.position + sf::Vector2f(mob.size.x - 13.0f, 28.0f), { 13.0f, 24.0f }, sf::Color(63, 78, 64));
        drawPixelRect(window, mob.position + sf::Vector2f(10.0f, mob.size.y - 8.0f), { 15.0f, 8.0f }, sf::Color(42, 54, 45));
        drawPixelRect(window, mob.position + sf::Vector2f(mob.size.x - 25.0f, mob.size.y - 8.0f), { 15.0f, 8.0f }, sf::Color(42, 54, 45));
    }

    if (mob.behavior == MobBehavior::Fast)
    {
        drawPixelRect(window, mob.position + sf::Vector2f(-8.0f, mob.size.y * 0.45f), { 10.0f, 3.0f }, sf::Color(255, 231, 95, 190));
        drawPixelRect(window, mob.position + sf::Vector2f(-15.0f, mob.size.y * 0.6f), { 12.0f, 3.0f }, sf::Color(255, 231, 95, 130));
    }
    else if (mob.behavior == MobBehavior::Chase)
    {
        drawPixelRect(window, mob.position + sf::Vector2f(mob.size.x * 0.5f - 8.0f, -17.0f), { 16.0f, 4.0f }, sf::Color(255, 76, 93, 210));
    }
    else if (mob.behavior == MobBehavior::Shooter)
    {
        sf::CircleShape focus(5.0f, 8);
        focus.setOrigin({ 5.0f, 5.0f });
        focus.setPosition(mob.position + sf::Vector2f(mob.size.x * 0.5f, -15.0f));
        focus.setFillColor(sf::Color(98, 229, 216, 210));
        window.draw(focus);
    }

    const float healthPercent = static_cast<float>(mob.health) / static_cast<float>(mob.maxHealth);
    drawPixelRect(window, mob.position + sf::Vector2f(0.0f, -9.0f), { mob.size.x, 4.0f }, sf::Color(36, 24, 30, 170));
    drawPixelRect(window, mob.position + sf::Vector2f(0.0f, -9.0f), { mob.size.x * healthPercent, 4.0f }, sf::Color(234, 70, 82, 220));
}

inline const char* getBlockLetterPattern(char letter)
{
    switch (letter)
    {
    case '0':
        return " XXX "
            "X   X"
            "X  XX"
            "X X X"
            "XX  X"
            "X   X"
            " XXX ";
    case '1':
        return "  X  "
            " XX  "
            "  X  "
            "  X  "
            "  X  "
            "  X  "
            " XXX ";
    case '2':
        return " XXX "
            "X   X"
            "    X"
            "   X "
            "  X  "
            " X   "
            "XXXXX";
    case '3':
        return "XXXX "
            "    X"
            "    X"
            " XXX "
            "    X"
            "    X"
            "XXXX ";
    case '4':
        return "X   X"
            "X   X"
            "X   X"
            "XXXXX"
            "    X"
            "    X"
            "    X";
    case '5':
        return "XXXXX"
            "X    "
            "X    "
            "XXXX "
            "    X"
            "    X"
            "XXXX ";
    case '6':
        return " XXX "
            "X    "
            "X    "
            "XXXX "
            "X   X"
            "X   X"
            " XXX ";
    case '7':
        return "XXXXX"
            "    X"
            "   X "
            "  X  "
            " X   "
            " X   "
            " X   ";
    case '8':
        return " XXX "
            "X   X"
            "X   X"
            " XXX "
            "X   X"
            "X   X"
            " XXX ";
    case '9':
        return " XXX "
            "X   X"
            "X   X"
            " XXXX"
            "    X"
            "    X"
            " XXX ";
    case '/':
        return "    X"
            "    X"
            "   X "
            "  X  "
            " X   "
            "X    "
            "X    ";
    case 'A':
        return " XXX "
            "X   X"
            "X   X"
            "XXXXX"
            "X   X"
            "X   X"
            "X   X";
    case 'B':
        return "XXXX "
            "X   X"
            "X   X"
            "XXXX "
            "X   X"
            "X   X"
            "XXXX ";
    case 'C':
        return " XXXX"
            "X    "
            "X    "
            "X    "
            "X    "
            "X    "
            " XXXX";
    case 'D':
        return "XXXX "
            "X   X"
            "X   X"
            "X   X"
            "X   X"
            "X   X"
            "XXXX ";
    case 'E':
        return "XXXXX"
            "X    "
            "X    "
            "XXXX "
            "X    "
            "X    "
            "XXXXX";
    case 'F':
        return "XXXXX"
            "X    "
            "X    "
            "XXXX "
            "X    "
            "X    "
            "X    ";
    case 'G':
        return " XXXX"
            "X    "
            "X    "
            "X XXX"
            "X   X"
            "X   X"
            " XXXX";
    case 'H':
        return "X   X"
            "X   X"
            "X   X"
            "XXXXX"
            "X   X"
            "X   X"
            "X   X";
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
    case 'J':
        return "XXXXX"
            "    X"
            "    X"
            "    X"
            "X   X"
            "X   X"
            " XXX ";
    case 'K':
        return "X   X"
            "X  X "
            "X X  "
            "XX   "
            "X X  "
            "X  X "
            "X   X";
    case 'L':
        return "X    "
            "X    "
            "X    "
            "X    "
            "X    "
            "X    "
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
    case 'P':
        return "XXXX "
            "X   X"
            "X   X"
            "XXXX "
            "X    "
            "X    "
            "X    ";
    case 'Q':
        return " XXX "
            "X   X"
            "X   X"
            "X   X"
            "X X X"
            "X  X "
            " XX X";
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
    case 'X':
        return "X   X"
            "X   X"
            " X X "
            "  X  "
            " X X "
            "X   X"
            "X   X";
    case 'Y':
        return "X   X"
            "X   X"
            " X X "
            "  X  "
            "  X  "
            "  X  "
            "  X  ";
    case 'Z':
        return "XXXXX"
            "    X"
            "   X "
            "  X  "
            " X   "
            "X    "
            "XXXXX";
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

inline void drawHudIngredientToken(sf::RenderWindow& window,
    sf::Vector2f center,
    bool collected,
    std::size_t index)
{
    const sf::Color rimColor = collected ? sf::Color(55, 58, 64, 235) : sf::Color(160, 92, 35, 245);
    const sf::Color faceColor = collected ? sf::Color(82, 86, 94, 235) : sf::Color(255, 179, 65, 245);
    const sf::Color warmShade = collected ? sf::Color(48, 50, 56, 180) : sf::Color(198, 103, 38, 180);
    const sf::Color highlight = collected ? sf::Color(132, 136, 146, 170) : sf::Color(255, 235, 144, 220);

    sf::CircleShape shadow(12.0f, 18);
    shadow.setOrigin({ 12.0f, 12.0f });
    shadow.setPosition(center + sf::Vector2f(1.5f, 2.5f));
    shadow.setFillColor(sf::Color(10, 9, 16, 120));
    window.draw(shadow);

    sf::CircleShape rim(12.0f, 24);
    rim.setOrigin({ 12.0f, 12.0f });
    rim.setPosition(center);
    rim.setFillColor(rimColor);
    rim.setOutlineThickness(2.0f);
    rim.setOutlineColor(collected ? sf::Color(32, 34, 40, 230) : sf::Color(255, 214, 103, 230));
    window.draw(rim);

    sf::CircleShape face(8.5f, 18);
    face.setOrigin({ 8.5f, 8.5f });
    face.setPosition(center + sf::Vector2f(-0.8f, -0.8f));
    face.setFillColor(faceColor);
    window.draw(face);

    drawPixelRect(window, center + sf::Vector2f(-4.0f, -8.0f), { 4.0f, 14.0f }, highlight);
    drawPixelRect(window, center + sf::Vector2f(0.0f, -3.0f), { 7.0f, 3.0f }, highlight);
    drawPixelRect(window, center + sf::Vector2f(3.0f, 4.0f), { 5.0f, 3.0f }, warmShade);

    const float chipOffset = static_cast<float>(index % 2) * 2.0f;
    drawPixelRect(window, center + sf::Vector2f(-8.0f + chipOffset, 5.0f), { 3.0f, 3.0f }, warmShade);
    drawPixelRect(window, center + sf::Vector2f(5.0f - chipOffset, -7.0f), { 2.0f, 2.0f }, highlight);

    if (collected)
    {
        drawPixelRect(window, center + sf::Vector2f(-5.0f, -1.0f), { 3.0f, 5.0f }, sf::Color(151, 233, 163, 230));
        drawPixelRect(window, center + sf::Vector2f(-2.0f, 2.0f), { 3.0f, 3.0f }, sf::Color(151, 233, 163, 230));
        drawPixelRect(window, center + sf::Vector2f(1.0f, -5.0f), { 3.0f, 9.0f }, sf::Color(151, 233, 163, 230));
    }
}

inline void drawHudAbilityBadge(sf::RenderWindow& window,
    sf::Vector2f position,
    sf::Vector2f size,
    bool unlocked,
    std::string_view label,
    bool climbBadge)
{
    const sf::Color accent = climbBadge ? sf::Color(91, 196, 116) : sf::Color(132, 105, 255);
    const sf::Color mutedAccent = climbBadge ? sf::Color(64, 106, 75) : sf::Color(80, 72, 119);
    const sf::Color iconColor = unlocked ? accent : mutedAccent;
    const sf::Color textColor = unlocked ? sf::Color(236, 235, 255) : sf::Color(135, 134, 150);

    sf::RectangleShape badge(size);
    badge.setPosition(position);
    badge.setFillColor(unlocked ? sf::Color(43, 36, 58, 235) : sf::Color(32, 30, 38, 225));
    badge.setOutlineThickness(2.0f);
    badge.setOutlineColor(unlocked ? sf::Color(accent.r, accent.g, accent.b, 210) : sf::Color(75, 72, 86, 210));
    window.draw(badge);

    drawPixelRect(window, position + sf::Vector2f(1.0f, 1.0f), { size.x - 2.0f, 4.0f }, sf::Color(255, 255, 255, unlocked ? 34 : 16));

    if (climbBadge)
    {
        drawPixelRect(window, position + sf::Vector2f(10.0f, 8.0f), { 5.0f, 20.0f }, iconColor);
        drawPixelRect(window, position + sf::Vector2f(22.0f, 8.0f), { 5.0f, 20.0f }, iconColor);
        drawPixelRect(window, position + sf::Vector2f(10.0f, 12.0f), { 17.0f, 4.0f }, iconColor);
        drawPixelRect(window, position + sf::Vector2f(10.0f, 22.0f), { 17.0f, 4.0f }, iconColor);
        drawPixelRect(window, position + sf::Vector2f(17.0f, 6.0f), { 4.0f, 25.0f }, sf::Color(57, 139, 80, unlocked ? 190 : 80));
    }
    else
    {
        drawPixelRect(window, position + sf::Vector2f(15.0f, 7.0f), { 5.0f, 18.0f }, iconColor);
        drawPixelRect(window, position + sf::Vector2f(10.0f, 12.0f), { 15.0f, 5.0f }, iconColor);
        drawPixelRect(window, position + sf::Vector2f(8.0f, 24.0f), { 19.0f, 4.0f }, iconColor);
        drawPixelRect(window, position + sf::Vector2f(12.0f, 4.0f), { 11.0f, 4.0f }, iconColor);
        drawPixelRect(window, position + sf::Vector2f(16.0f, 1.0f), { 3.0f, 3.0f }, iconColor);
    }

    drawBlockLabel(window, label, position + sf::Vector2f(35.0f, 12.0f), 1.6f, 2.0f, textColor);
}

inline void drawCoinsHud(sf::RenderWindow& window, const Level& level)
{
    sf::RectangleShape panel({ 338.0f, 92.0f });
    panel.setPosition({ 924.0f, 16.0f });
    panel.setFillColor(sf::Color(24, 22, 34, 220));
    panel.setOutlineThickness(2.0f);
    panel.setOutlineColor(sf::Color(142, 113, 198, 150));
    window.draw(panel);

    drawPixelRect(window, { 926.0f, 18.0f }, { 334.0f, 5.0f }, sf::Color(255, 255, 255, 28));
    drawBlockLabel(window, "COINS TO COLLECT", { 940.0f, 29.0f }, 2.0f, 2.0f, sf::Color(241, 231, 189));

    const int collectedCount = static_cast<int>(std::count_if(level.ingredients.begin(),
        level.ingredients.end(),
        [](const Ingredient& ingredient)
        {
            return ingredient.collected;
        }));

    for (std::size_t i = 0; i < level.ingredients.size(); ++i)
    {
        drawHudIngredientToken(window,
            { 954.0f + static_cast<float>(i) * 34.0f, 70.0f },
            static_cast<int>(i) < collectedCount,
            i);
    }

    const std::string progress = std::to_string(collectedCount) + "/" + std::to_string(level.ingredients.size());
    drawBlockLabel(window, progress, { 1104.0f, 61.0f }, 2.0f, 2.0f, sf::Color(235, 232, 255));
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
