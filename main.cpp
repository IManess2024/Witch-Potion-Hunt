#include <algorithm>
#include <optional>
#include <vector>

#include <SFML/Graphics.hpp>

#include "game_types.hpp"
#include "gameplay.hpp"
#include "levels.hpp"
#include "rendering.hpp"

namespace
{
constexpr unsigned int kWindowWidth  = 1280;
constexpr unsigned int kWindowHeight = 720;
}

int main()
{
    sf::RenderWindow window(sf::VideoMode({kWindowWidth, kWindowHeight}), "Witch Potion Hunt");
    window.setFramerateLimit(60);

    std::vector<Level> levels = createLevels();
    Player             player;
    std::size_t        currentLevelIndex = 0;
    bool               gameWon           = false;
    bool               jumpWasHeld       = false;

    resetLevel(levels[currentLevelIndex], player, currentLevelIndex);

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
        {
            window.close();
        }

        Level& currentLevel = levels[currentLevelIndex];

        if (!gameWon)
        {
            updateClimbWallContact(player, currentLevel);

            const bool moveLeft = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A);
            const bool moveRight = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);
            const bool moveUp = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W);
            const bool moveDown = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S);

            const bool jumpHeld             = moveUp || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);
            const bool jumpPressedThisFrame = jumpHeld && !jumpWasHeld;
            jumpWasHeld                     = jumpHeld;

            float horizontalInput = 0.f;

            if (moveLeft)
            {
                horizontalInput -= 1.f;
            }

            if (moveRight)
            {
                horizontalInput += 1.f;
            }

            player.velocity.x = horizontalInput * kMoveSpeed;

            if (jumpPressedThisFrame)
            {
                if (player.onGround || (player.touchingClimbWall && player.canClimb))
                {
                    player.velocity.y = kJumpSpeed;
                    player.onGround   = false;
                }
                else if (player.extraJumpsRemaining > 0)
                {
                    player.velocity.y = kJumpSpeed;
                    --player.extraJumpsRemaining;
                }
            }

            const bool climbingNow = player.canClimb && player.touchingClimbWall && (moveUp || moveDown);
            if (climbingNow)
            {
                player.velocity.y = 0.f;

                if (moveUp)
                {
                    player.velocity.y = -kClimbSpeed;
                }
                else if (moveDown)
                {
                    player.velocity.y = kClimbSpeed;
                }
            }
            else
            {
                player.velocity.y += kGravity / kFixedDt;

                if (player.touchingClimbWall && player.canClimb && player.velocity.y > kWallSlideSpeed)
                {
                    player.velocity.y = kWallSlideSpeed;
                }
            }

            resolveHorizontalCollisions(player, currentLevel, kFixedDt);
            resolveVerticalCollisions(player, currentLevel, kFixedDt);
            updateClimbWallContact(player, currentLevel);

            player.position.x =
                std::clamp(player.position.x, 0.f, static_cast<float>(kWindowWidth) - player.size.x);

            if (player.position.y < 0.f)
            {
                player.position.y = 0.f;
                player.velocity.y = std::max(0.f, player.velocity.y);
            }

            if (player.position.y > static_cast<float>(kWindowHeight) + 120.f)
            {
                resetLevel(currentLevel, player, currentLevelIndex);
            }

            for (Ingredient& ingredient : currentLevel.ingredients)
            {
                if (ingredient.collected)
                {
                    continue;
                }

                sf::FloatRect ingredientBounds(ingredient.position - sf::Vector2f(10.f, 10.f), {20.f, 20.f});
                if (getPlayerBounds(player).findIntersection(ingredientBounds))
                {
                    ingredient.collected = true;
                }
            }

            if (allIngredientsCollected(currentLevel) &&
                getPlayerBounds(player).findIntersection(currentLevel.cauldronArea))
            {
                ++currentLevelIndex;

                if (currentLevelIndex >= levels.size())
                {
                    currentLevelIndex = levels.size() - 1;
                    gameWon           = true;
                }
                else
                {
                    resetLevel(levels[currentLevelIndex], player, currentLevelIndex);
                }
            }
        }
        else
        {
            jumpWasHeld = false;
        }

        const Level& levelToDraw = levels[currentLevelIndex];
        window.clear(levelToDraw.backgroundColor);

        for (const sf::FloatRect& solid : levelToDraw.solids)
        {
            sf::RectangleShape platform({solid.size.x, solid.size.y});
            platform.setPosition(solid.position);
            platform.setFillColor(sf::Color(92, 74, 60));
            window.draw(platform);
        }

        for (const sf::FloatRect& wall : levelToDraw.climbWalls)
        {
            sf::RectangleShape ivy({wall.size.x, wall.size.y});
            ivy.setPosition(wall.position);
            ivy.setFillColor(sf::Color(68, 150, 92));
            window.draw(ivy);
        }

        for (const Ingredient& ingredient : levelToDraw.ingredients)
        {
            if (ingredient.collected)
            {
                continue;
            }

            sf::CircleShape circle(10.f);
            circle.setOrigin({10.f, 10.f});
            circle.setPosition(ingredient.position);
            circle.setFillColor(sf::Color(255, 185, 90));
            window.draw(circle);
        }

        drawCauldron(window, levelToDraw, allIngredientsCollected(levelToDraw));
        drawPlayer(window, player);
        drawHud(window, levelToDraw, player);
        window.display();
    }
}
