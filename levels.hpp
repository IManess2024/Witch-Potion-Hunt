#pragma once

#include "game_types.hpp"

inline sf::FloatRect makeRect(float x, float y, float width, float height)
{
    return sf::FloatRect({x, y}, {width, height});
}

inline Mob makeMob(MobType type,
    MobBehavior behavior,
    sf::Vector2f position,
    sf::Vector2f size,
    int health,
    int damage,
    float patrolMinX,
    float patrolMaxX,
    float phase = 0.0f)
{
    Mob mob;
    mob.type = type;
    mob.behavior = behavior;
    mob.position = position;
    mob.spawnPosition = position;
    mob.size = size;
    mob.health = health;
    mob.maxHealth = health;
    mob.damage = damage;
    mob.patrolMinX = patrolMinX;
    mob.patrolMaxX = patrolMaxX;
    mob.baseY = position.y;
    mob.phase = phase;
    mob.attackTimer = phase;
    mob.attackCooldown = behavior == MobBehavior::Shooter ? 1.25f : 1.6f;
    return mob;
}

inline void PlacePlayerAtLevelSpawn(const Level& level, Player& player)
{
    player.position = level.spawnPosition;
    player.velocity = { 0.0f, 0.0f };
    player.onGround = false;
    player.touchingClimbWall = false;
    player.facingRight = true;
    player.hurtCooldown = 0.0f;

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

        level.mobs.push_back(makeMob(MobType::Bat, MobBehavior::Patrol, { 310.f, 385.f }, { 38.f, 24.f }, 36, 14, 180.f, 590.f, 0.3f));
        level.mobs.push_back(makeMob(MobType::Bat, MobBehavior::Fast, { 545.f, 285.f }, { 36.f, 22.f }, 30, 12, 420.f, 865.f, 1.0f));
        level.mobs.push_back(makeMob(MobType::Imp, MobBehavior::Chase, { 900.f, 532.f }, { 32.f, 40.f }, 44, 20, 815.f, 1210.f, 0.6f));
        level.mobs.push_back(makeMob(MobType::Bat, MobBehavior::Shooter, { 670.f, 230.f }, { 42.f, 26.f }, 42, 10, 520.f, 870.f, 2.0f));

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

        level.mobs.push_back(makeMob(MobType::Imp, MobBehavior::Fast, { 370.f, 532.f }, { 34.f, 42.f }, 48, 18, 350.f, 560.f, 0.0f));
        level.mobs.push_back(makeMob(MobType::Imp, MobBehavior::Chase, { 700.f, 532.f }, { 34.f, 42.f }, 52, 22, 700.f, 930.f, 1.2f));
        level.mobs.push_back(makeMob(MobType::Bat, MobBehavior::Shooter, { 585.f, 280.f }, { 38.f, 24.f }, 38, 12, 470.f, 780.f, 1.8f));
        level.mobs.push_back(makeMob(MobType::Bat, MobBehavior::Fast, { 220.f, 385.f }, { 36.f, 22.f }, 34, 12, 120.f, 430.f, 2.5f));
        level.mobs.push_back(makeMob(MobType::Golem, MobBehavior::Patrol, { 790.f, 512.f }, { 52.f, 68.f }, 86, 28, 700.f, 940.f, 0.4f));

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

        level.mobs.push_back(makeMob(MobType::Golem, MobBehavior::Chase, { 720.f, 512.f }, { 58.f, 68.f }, 110, 34, 700.f, 930.f, 0.0f));
        level.mobs.push_back(makeMob(MobType::Imp, MobBehavior::Fast, { 342.f, 532.f }, { 34.f, 42.f }, 56, 20, 330.f, 570.f, 2.0f));
        level.mobs.push_back(makeMob(MobType::Bat, MobBehavior::Shooter, { 540.f, 210.f }, { 42.f, 26.f }, 48, 14, 470.f, 660.f, 1.5f));
        level.mobs.push_back(makeMob(MobType::Imp, MobBehavior::Shooter, { 770.f, 128.f }, { 34.f, 42.f }, 60, 18, 735.f, 920.f, 0.8f));
        level.mobs.push_back(makeMob(MobType::Bat, MobBehavior::Fast, { 245.f, 350.f }, { 38.f, 24.f }, 44, 16, 175.f, 390.f, 2.7f));
        level.mobs.push_back(makeMob(MobType::Golem, MobBehavior::Patrol, { 420.f, 512.f }, { 56.f, 68.f }, 96, 30, 330.f, 570.f, 1.0f));

        levels.push_back(level);
    }

    return levels;
}
