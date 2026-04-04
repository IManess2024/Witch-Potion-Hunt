#include <optional>

#include <vector>

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>


struct Player {
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Vector2f size{ 34.0f, 52.0f };
    bool OnGround{ false };
    bool TouchingClimbWall{ false };
    bool CanDoubleJump{ false };
    bool CanClimb{ false };
    int ExtraJumpsRemaining{ 0 };
};

struct Ingredient {
    sf::Vector2f position;
    bool collected{ false };
};

struct Level {
    sf::Color backgroundColor;
    sf::Vector2f spawnPosition;
    sf::FloatRect cauldronArea;
    std::vector<sf::FloatRect> solids;
    std::vector<sf::FloatRect> climbWalls;
    std::vector<Ingredient> ingredients;

};

sf::FloatRect makeRect(float x, float y, float width, float height)
{
    return sf::FloatRect({ x, y }, { width, height });
}

std::vector<Level> CreateLevels()
{
    std::vector<Level> levels;

    // Level 1: a gentle platforming tutorial. Just jump around and gather herbs.
    {
        Level level;
        level.backgroundColor = sf::Color(35, 28, 56);
        level.spawnPosition = { 55.f, 500.f };
        level.cauldronArea = makeRect(800.f, 240.f, 70.f, 70.f);

        level.solids.push_back(makeRect(0.f, 580.f, 960.f, 60.f));
        level.solids.push_back(makeRect(140.f, 470.f, 180.f, 20.f));
        level.solids.push_back(makeRect(410.f, 390.f, 180.f, 20.f));
        level.solids.push_back(makeRect(690.f, 310.f, 180.f, 20.f));

        level.ingredients.push_back({ {190.f, 440.f}, false });
        level.ingredients.push_back({ {100.f, 545.f}, false });
        level.ingredients.push_back({ {470.f, 360.f}, false });
        level.ingredients.push_back({ {760.f, 280.f}, false });

        levels.push_back(level);
    }

    // Level 2: the new double-jump lets the witch reach a much higher platform.
    {
        Level level;
        level.backgroundColor = sf::Color(23, 41, 68);
        level.spawnPosition = { 40.f, 500.f };
        level.cauldronArea = makeRect(815.f, 115.f, 70.f, 70.f);

        level.solids.push_back(makeRect(0.f, 580.f, 260.f, 60.f));
        level.solids.push_back(makeRect(350.f, 580.f, 220.f, 60.f));
        level.solids.push_back(makeRect(700.f, 580.f, 260.f, 60.f));
        level.solids.push_back(makeRect(260.f, 470.f, 130.f, 20.f));
        level.solids.push_back(makeRect(480.f, 380.f, 150.f, 20.f));
        level.solids.push_back(makeRect(760.f, 190.f, 150.f, 20.f));

        level.ingredients.push_back({ {120.f, 545.f}, false });
        level.ingredients.push_back({ {320.f, 440.f}, false });
        level.ingredients.push_back({ {550.f, 350.f}, false });
        level.ingredients.push_back({ {835.f, 160.f}, false });

        levels.push_back(level);
    }

    // Level 3: ivy-covered walls can now be climbed, which is enough to reach
    // the highest shelves of the level.
    {
        Level level;
        level.backgroundColor = sf::Color(22, 52, 46);
        level.spawnPosition = { 45.f, 500.f };
        level.cauldronArea = makeRect(820.f, 95.f, 70.f, 70.f);

        level.climbWalls.push_back(makeRect(260.f, 330.f, 40.f, 250.f));
        level.climbWalls.push_back(makeRect(620.f, 180.f, 40.f, 260.f));

        level.solids.push_back(makeRect(0.f, 580.f, 250.f, 60.f));
        level.solids.push_back(makeRect(330.f, 580.f, 250.f, 60.f));
        level.solids.push_back(makeRect(700.f, 580.f, 260.f, 60.f));
        level.solids.push_back(makeRect(190.f, 430.f, 180.f, 20.f));
        level.solids.push_back(makeRect(470.f, 300.f, 180.f, 20.f));
        level.solids.push_back(makeRect(760.f, 170.f, 160.f, 20.f));

        // The climbable walls are also solid walls.
        for (const sf::FloatRect& wall : level.climbWalls)
        {
            level.solids.push_back(wall);
        }

        level.ingredients.push_back({ {110.f, 545.f}, false });
        level.ingredients.push_back({ {300.f, 400.f}, false });
        level.ingredients.push_back({ {560.f, 270.f}, false });
        level.ingredients.push_back({ {840.f, 140.f}, false });

        levels.push_back(level);
    }

    return levels;
}


void DrawPlayer(sf::RenderWindow& window, const Player& player)
{
    // The witch is built from a few primitive shapes so the project stays
    // asset-free and easy to understand.
    sf::CircleShape head(11.f);
    head.setPosition(player.position + sf::Vector2f(6.f, 2.f));
    head.setFillColor(sf::Color(246, 215, 189));
    window.draw(head);

    sf::RectangleShape dress({ player.size.x, player.size.y - 14.f });
    dress.setPosition(player.position + sf::Vector2f(0.f, 14.f));
    dress.setFillColor(sf::Color(120, 64, 170));
    window.draw(dress);

    sf::ConvexShape hat(3);
    hat.setPoint(0, { player.size.x * 0.5f, -16.f });
    hat.setPoint(1, { 2.f, 15.f });
    hat.setPoint(2, { player.size.x - 2.f, 15.f });
    hat.setPosition(player.position + sf::Vector2f(0.f, -2.f));
    hat.setFillColor(sf::Color(42, 23, 64));
    window.draw(hat);

    sf::RectangleShape broom({ 28.f, 5.f });
    broom.setPosition(player.position + sf::Vector2f(player.size.x - 2.f, 30.f));
    broom.setFillColor(sf::Color(130, 86, 48));
    window.draw(broom);

    sf::RectangleShape broomBrush({ 10.f, 10.f });
    broomBrush.setPosition(player.position + sf::Vector2f(player.size.x + 22.f, 27.f));
    broomBrush.setFillColor(sf::Color(189, 156, 79));
    window.draw(broomBrush);
}

sf::FloatRect getplayerbounds(const Player& player)
{
    return sf::FloatRect(player.position, player.size);
}

void resolvehorizontalcollisions(Player& player, const Level& level, float dt)
{
    player.position.x += player.velocity.x / dt;
    sf::FloatRect PlayerBounds = getplayerbounds(player);

    for (const sf::FloatRect& solid : level.solids)
    {
        if (!PlayerBounds.findIntersection(solid))
        {
            continue;
        }
        if (player.velocity.x > 0.0f)
        {
            player.position.x = solid.position.x - player.size.x;

        }
        else if (player.velocity.x < 0.0f)
        {
            player.position.x = solid.position.x + solid.size.x;
        }
        player.velocity.x = 0.0f;
        PlayerBounds = getplayerbounds(player);
    }
}

void resolveVerticalCollisions(Player& player, const Level& level, float dt)
{
    player.OnGround = false;
    player.position.y += player.velocity.y / dt;

    sf::FloatRect playerBounds = getplayerbounds(player);

    for (const sf::FloatRect& solid : level.solids)
    {
        if (!playerBounds.findIntersection(solid))
        {
            continue;
        }

        if (player.velocity.y > 0.f)
        {
            // Landing on top of a platform resets the extra jump.
            player.position.y = solid.position.y - player.size.y;
            player.velocity.y = 0.f;
            player.OnGround = true;
            player.ExtraJumpsRemaining = player.CanDoubleJump ? 1 : 0;
        }
        else if (player.velocity.y < 0.f)
        {
            player.position.y = solid.position.y + solid.size.y;
            player.velocity.y = 0.f;
        }

        playerBounds = getplayerbounds(player);
    }
}

void CallHUD(sf::RenderWindow& window, const Level& level, const Player& player)
{
    // The HUD uses only shapes because we intentionally avoid external font
    // files. The window title carries the written instructions.
    for (std::size_t i = 0; i < level.ingredients.size(); ++i)
    {
        sf::CircleShape icon(8.f);
        icon.setPosition({ 18.f + static_cast<float>(i) * 24.f, 16.f });
        icon.setFillColor(level.ingredients[i].collected ? sf::Color(80, 80, 80)
            : sf::Color(255, 180, 80));
        window.draw(icon);
    }

    sf::RectangleShape doubleJumpIcon({ 22.f, 10.f });
    doubleJumpIcon.setPosition({ 18.f, 42.f });
    doubleJumpIcon.setFillColor(player.CanDoubleJump ? sf::Color(132, 105, 255)
        : sf::Color(60, 60, 60));
    window.draw(doubleJumpIcon);

    sf::RectangleShape climbIcon({ 22.f, 10.f });
    climbIcon.setPosition({ 48.f, 42.f });
    climbIcon.setFillColor(player.CanClimb ? sf::Color(91, 196, 116)
        : sf::Color(60, 60, 60));
    window.draw(climbIcon);
}

void DrawCauldron(sf::RenderWindow& window, const Level& level, bool ReadyforExit){
    sf::RectangleShape bowl({ 70.f, 38.f });
    bowl.setPosition(level.cauldronArea.position + sf::Vector2f(0.f, 24.f));
    bowl.setFillColor(ReadyforExit ? sf::Color(82, 204, 103) : sf::Color(70, 70, 70));
    window.draw(bowl);

    sf::RectangleShape leg({ 10.f, 18.f });
    leg.setFillColor(sf::Color(45, 30, 20));

    leg.setPosition(level.cauldronArea.position + sf::Vector2f(8.f, 60.f));
    window.draw(leg);
    leg.setPosition(level.cauldronArea.position + sf::Vector2f(52.f, 60.f));
    window.draw(leg);

    // A few bubbles make the exit look active.
    for (int bubbleIndex = 0; bubbleIndex < 3; ++bubbleIndex)
    {
        sf::CircleShape bubble(7.f - static_cast<float>(bubbleIndex));
        bubble.setFillColor(ReadyforExit ? sf::Color(170, 255, 190) : sf::Color(120, 120, 120));
        bubble.setPosition(level.cauldronArea.position +
            sf::Vector2f(12.f + bubbleIndex * 18.f, 10.f - bubbleIndex * 8.f));
        window.draw(bubble);
    }
}

bool AllIngredientsCollected(const Level& level)
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

int main()
{
    sf::RenderWindow window(sf::VideoMode({1280, 720}), "Witch Potion Hunt");
    window.setFramerateLimit(60);

    std::vector<Level> levels = CreateLevels();

    Player player;

    bool GameWon{ false };



   

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
            {
                window.close();
            }
        }

        window.clear(sf::Color(24, 28, 36));

        if (!GameWon)
        {
            const bool MoveLeft = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A);
            const bool MoveRight = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);
            const bool MoveUp = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W);
            const bool MoveDown = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S);

            float HorizontalInput = 0.0f;

            if (MoveLeft)
            {
                HorizontalInput -= 1.0f;
            }

            if (MoveRight)
            {
                HorizontalInput += 1.0f;
            }

            resolvehorizontalcollisions(player, levels[0], 60.0f);

            resolveVerticalCollisions(player, levels[0], 60.0f);

            player.velocity.y += 1200 / 60.0f;

            player.velocity.x = HorizontalInput * 260.0f;

            player.position.x = std::clamp(player.position.x, 0.0f, static_cast<float>(1280) - player.size.x);

        }
        
    

        const Level& LeveltoDraw = levels[0];

        for (const sf::FloatRect& solid : LeveltoDraw.solids)
        {
            sf::RectangleShape Platform({ solid.size.x, solid.size.y });
            Platform.setPosition(solid.position);
            Platform.setFillColor(sf::Color(92, 74, 60));
            window.draw(Platform);
        }

        for (const Ingredient& ingredient : levels[0].ingredients)
        {
            if (ingredient.collected)
            {
                continue;
            }
            sf::CircleShape circle(10.0f);
            circle.setOrigin({ 10.0f, 10.0f });
            circle.setPosition(ingredient.position);
            circle.setFillColor(sf::Color(255, 185, 90));
            window.draw(circle);
        }

        DrawCauldron(window, levels[0], AllIngredientsCollected(levels[0]));
        DrawPlayer(window, player);
        CallHUD(window, levels[0], player);
        window.display();
    }
}
