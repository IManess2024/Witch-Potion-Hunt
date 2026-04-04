#pragma once

#include "game_types.hpp"

inline sf::FloatRect makeRect(float x, float y, float width, float height)
{
    return sf::FloatRect({x, y}, {width, height});
}

inline void PlacePlayerAtLevelSpawn(const Level& level, Player& player)
{
    player.position = level.spawnPosition;
    player.velocity = { 0.0f, 0.0f };
    player.onGround = false;
    player.touchingClimbWall = false;

}

inline std::vector<Level> createLevels()
{
    std::vector<Level> levels;

    {
        Level level;
        level.backgroundColor = sf::Color(35, 28, 56);
        level.spawnPosition   = {55.f, 500.f};
        level.cauldronArea    = makeRect(800.f, 240.f, 70.f, 70.f);

        level.solids.push_back(makeRect(0.f, 580.f, 960.f, 60.f));
        level.solids.push_back(makeRect(140.f, 470.f, 180.f, 20.f));
        level.solids.push_back(makeRect(410.f, 390.f, 180.f, 20.f));
        level.solids.push_back(makeRect(690.f, 310.f, 180.f, 20.f));

        level.ingredients.push_back({ {100.f, 545.f}, false });
        level.ingredients.push_back({ {190.f, 440.f}, false });
        level.ingredients.push_back({{470.f, 360.f}, false});
        level.ingredients.push_back({{760.f, 280.f}, false});

        levels.push_back(level);
    }

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
        level.ingredients.push_back({{835.f, 160.f}, false});

        levels.push_back(level);
    }

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
        level.ingredients.push_back({{840.f, 140.f}, false});

        levels.push_back(level);
    }

    return levels;
}
