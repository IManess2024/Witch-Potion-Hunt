#pragma once

#include "game_types.hpp"

inline sf::FloatRect makeRect(float x, float y, float width, float height)
{
    return sf::FloatRect({x, y}, {width, height});
}

inline Mob makeMob(MobType type,
    sf::Vector2f position,
    sf::Vector2f size,
    int health,
    float patrolMinX,
    float patrolMaxX,
    float phase = 0.0f)
{
    Mob mob;
    mob.type = type;
    mob.position = position;
    mob.spawnPosition = position;
    mob.size = size;
    mob.health = health;
    mob.maxHealth = health;
    mob.patrolMinX = patrolMinX;
    mob.patrolMaxX = patrolMaxX;
    mob.baseY = position.y;
    mob.phase = phase;
    return mob;
}

inline void PlacePlayerAtLevelSpawn(const Level& level, Player& player)
{
    player.position = level.spawnPosition;
    player.velocity = { 0.0f, 0.0f };
    player.onGround = false;
    player.touchingClimbWall = false;
    player.facingRight = true;

}

inline std::vector<Level> createLevels()
{
    std::vector<Level> levels;
    // level 1
    {
        Level level;
        level.backgroundColor = sf::Color(35, 28, 56);
        level.spawnPosition   = {55.f, 500.f};
        level.cauldronArea    = makeRect(800.f, 240.f, 70.f, 70.f);

        level.solids.push_back(makeRect(0.f, 580.f, 1280.f, 60.f));
        level.solids.push_back(makeRect(140.f, 470.f, 180.f, 20.f));
        level.solids.push_back(makeRect(410.f, 390.f, 180.f, 20.f));
        level.solids.push_back(makeRect(690.f, 310.f, 180.f, 20.f));

        level.ingredients.push_back({ {100.f, 545.f}, false });
        level.ingredients.push_back({ {190.f, 440.f}, false });
        level.ingredients.push_back({{470.f, 360.f}, false});
        level.ingredients.push_back({{760.f, 280.f}, false});

        level.mobs.push_back(makeMob(MobType::Bat, { 310.f, 385.f }, { 38.f, 24.f }, 36, 180.f, 590.f, 0.3f));
        level.mobs.push_back(makeMob(MobType::Bat, { 670.f, 230.f }, { 42.f, 26.f }, 42, 520.f, 870.f, 2.0f));

        levels.push_back(level);
    }
    // level 2
    {
        Level level;
        level.backgroundColor = sf::Color(23, 41, 68);
        level.spawnPosition   = {40.f, 500.f};
        level.cauldronArea    = makeRect(815.f, 115.f, 70.f, 70.f);

        level.solids.push_back(makeRect(0.f, 580.f, 260.f, 60.f));
        level.solids.push_back(makeRect(350.f, 580.f, 220.f, 60.f));
        level.solids.push_back(makeRect(700.f, 580.f, 260.f, 60.f));
        level.solids.push_back(makeRect(260.f, 470.f, 130.f, 20.f));
        level.solids.push_back(makeRect(480.f, 380.f, 150.f, 20.f));
        level.solids.push_back(makeRect(760.f, 190.f, 150.f, 20.f));

        level.ingredients.push_back({{120.f, 545.f}, false});
        level.ingredients.push_back({{320.f, 440.f}, false});
        level.ingredients.push_back({{550.f, 350.f}, false});
        level.ingredients.push_back({{720.f, 160.f}, false});

        level.mobs.push_back(makeMob(MobType::Imp, { 370.f, 532.f }, { 34.f, 42.f }, 48, 350.f, 560.f, 0.0f));
        level.mobs.push_back(makeMob(MobType::Imp, { 700.f, 532.f }, { 34.f, 42.f }, 52, 700.f, 930.f, 1.2f));
        level.mobs.push_back(makeMob(MobType::Bat, { 585.f, 280.f }, { 38.f, 24.f }, 38, 470.f, 780.f, 1.8f));

        levels.push_back(level);
    }
    // level 3
    {
        Level level;
        level.backgroundColor = sf::Color(22, 52, 46);
        level.spawnPosition   = {45.f, 500.f};
        level.cauldronArea    = makeRect(820.f, 95.f, 70.f, 70.f);

        level.climbWalls.push_back(makeRect(260.f, 330.f, 40.f, 250.f));
        level.climbWalls.push_back(makeRect(620.f, 180.f, 40.f, 260.f));

        level.solids.push_back(makeRect(0.f, 580.f, 250.f, 60.f));
        level.solids.push_back(makeRect(330.f, 580.f, 250.f, 60.f));
        level.solids.push_back(makeRect(700.f, 580.f, 260.f, 60.f));
        level.solids.push_back(makeRect(190.f, 430.f, 180.f, 20.f));
        level.solids.push_back(makeRect(470.f, 300.f, 180.f, 20.f));
        level.solids.push_back(makeRect(760.f, 170.f, 160.f, 20.f));

        for (const sf::FloatRect& wall : level.climbWalls)
        {
            level.solids.push_back(wall);
        }

        level.ingredients.push_back({{110.f, 545.f}, false});
        level.ingredients.push_back({{300.f, 400.f}, false});
        level.ingredients.push_back({{560.f, 270.f}, false});
        level.ingredients.push_back({{780.f, 140.f}, false});

        level.mobs.push_back(makeMob(MobType::Golem, { 720.f, 512.f }, { 58.f, 68.f }, 110, 700.f, 930.f, 0.0f));
        level.mobs.push_back(makeMob(MobType::Imp, { 342.f, 532.f }, { 34.f, 42.f }, 56, 330.f, 570.f, 2.0f));
        level.mobs.push_back(makeMob(MobType::Bat, { 540.f, 210.f }, { 42.f, 26.f }, 48, 470.f, 660.f, 1.5f));

        levels.push_back(level);
    }

    return levels;
}
