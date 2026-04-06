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
    bool               gameWon = false;
    bool               jumpwasheld = false;


    PlacePlayerAtLevelSpawn(levels[currentLevelIndex], player);

    refreshabilitesforlevel(player, currentLevelIndex);




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

         Level& currentLevel = levels[currentLevelIndex];

        if (!gameWon)
        {
            updateclimbwallcontact(player, currentLevel);
            const bool moveLeft = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A);
            const bool moveRight = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);
            const bool moveUp = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W);
            const bool moveDown = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S);
            const bool jumpheld = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);
            const bool jumppressedthisframe = jumpheld && !jumpwasheld;
            jumpwasheld = jumpheld;


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
            if (jumppressedthisframe)
            {
                if (player.onGround || (player.touchingClimbWall && player.canClimb))
                {
                    player.velocity.y = kJumpSpeed;
                    player.onGround = false;
                }
                else if (player.extraJumpsRemaining > 0)
                {
                    player.velocity.y = kJumpSpeed;
                    player.extraJumpsRemaining--;

                }
            }
            const bool climbnow = player.canClimb && player.touchingClimbWall && (moveUp || moveDown);
            if (climbnow)
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
            updateclimbwallcontact(player, currentLevel);
            CollectIngredients(player, currentLevel);

            player.position.x =
                std::clamp(player.position.x, 0.f, static_cast<float>(kWindowWidth) - player.size.x);

            const bool PortalReady = allIngredientsCollected(currentLevel);
            if (PortalReady && getPlayerBounds(player).findIntersection(currentLevel.cauldronArea))
            {
                currentLevelIndex++;
                if (currentLevelIndex >= levels.size())
                {
                    currentLevelIndex = levels.size() - 1;
                    gameWon = true;

                }
                else
                {
                    PlacePlayerAtLevelSpawn(levels[currentLevelIndex], player);
                    refreshabilitesforlevel(player, currentLevelIndex);

                }
                   
            }

        }
        else
        {
            jumpwasheld = false;

        }

        const Level& levelToDraw = levels[currentLevelIndex];
        const bool PortalReady = allIngredientsCollected(levelToDraw);


        for (const sf::FloatRect& solid : levelToDraw.solids)
        {
            sf::RectangleShape platform({solid.size.x, solid.size.y});
            platform.setPosition(solid.position);
            platform.setFillColor(sf::Color(92, 74, 60));
            window.draw(platform);
        }

        for (const sf::FloatRect& wall : levelToDraw.climbWalls)
        {
            sf::RectangleShape ivy({ wall.size.x, wall.size.y });
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

        drawCauldron(window, levelToDraw,PortalReady);
        drawPlayer(window, player);
        drawHud(window, levelToDraw, player);
        window.display();
    }
}
