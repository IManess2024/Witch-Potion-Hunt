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

        const Level& currentLevel = levels[currentLevelIndex];

        if (!gameWon)
        {
            const bool moveLeft = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A);
            const bool moveRight = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);
            const bool moveUp = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W);
            const bool moveDown = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S);

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
            player.velocity.y += kGravity / kFixedDt;

            resolveHorizontalCollisions(player, currentLevel, kFixedDt);
            resolveVerticalCollisions(player, currentLevel, kFixedDt);

            player.position.x =
                std::clamp(player.position.x, 0.f, static_cast<float>(kWindowWidth) - player.size.x);
        }

        for (const sf::FloatRect& solid : currentLevel.solids)
        {
            sf::RectangleShape platform({solid.size.x, solid.size.y});
            platform.setPosition(solid.position);
            platform.setFillColor(sf::Color(92, 74, 60));
            window.draw(platform);
        }

        for (const Ingredient& ingredient : currentLevel.ingredients)
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

        drawCauldron(window, currentLevel, allIngredientsCollected(currentLevel));
        drawPlayer(window, player);
        drawHud(window, currentLevel, player);
        window.display();
    }
}
