#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <optional>
#include <vector>

#include <SFML/Graphics.hpp>

#include "game_types.hpp"
#include "gameplay.hpp"
#include "levels.hpp"
#include "rendering.hpp"

namespace
{
    constexpr unsigned int kWindowWidth = 1280;
    constexpr unsigned int kWindowHeight = 720;
    constexpr char kWindowTitle[] = "Witch Potion Hunt";
    constexpr float kDeathThresholdY = static_cast<float>(kWindowHeight) + 20.0f;
    constexpr float kFrameSeconds = 1.0f / kFixedDt;
    constexpr float kPi = 3.14159265f;

    enum class ParticleKind
    {
        Spark,
        Puff
    };

    enum class SpellType
    {
        StarBolt,
        MoonFlame,
        CrystalBloom
    };

    struct MagicParticle
    {
        sf::Vector2f position;
        sf::Vector2f velocity;
        sf::Color color;
        float age{ 0.0f };
        float lifetime{ 1.0f };
        float radius{ 3.0f };
        ParticleKind kind{ ParticleKind::Spark };
    };

    struct SpellProjectile
    {
        SpellType type{ SpellType::StarBolt };
        sf::Vector2f position;
        sf::Vector2f velocity;
        float age{ 0.0f };
        float lifetime{ 1.0f };
        float trailTimer{ 0.0f };
        float rotation{ 0.0f };
    };

    sf::FloatRect GetRestartButtonBounds()
    {
        return sf::FloatRect({ 510.0f , 430.0f }, { 260.0f, 64.0f });
    }

    bool IsPointInside(const sf::FloatRect& rect, const sf::Vector2f point)
    {
        return point.x >= rect.position.x && point.x <= rect.position.x + rect.size.x &&
            point.y >= rect.position.y && point.y <= rect.position.y + rect.size.y;

    }

    bool CanRestart(bool gamewon, bool gamelost)
    {
        return gamewon || gamelost;
    }

    float GetVectorLength(sf::Vector2f vector)
    {
        return std::sqrt(vector.x * vector.x + vector.y * vector.y);
    }

    sf::Vector2f NormalizeVector(sf::Vector2f vector)
    {
        const float length = GetVectorLength(vector);
        if (length <= 0.001f)
        {
            return { 1.0f, 0.0f };
        }

        return vector / length;
    }

    sf::Vector2f GetMagicStickTip(const Player& player)
    {
        return player.position + sf::Vector2f(player.facingRight ? player.size.x + 34.0f : -34.0f, 29.0f);
    }

    const char* GetSpellName(SpellType spell)
    {
        switch (spell)
        {
        case SpellType::StarBolt:
            return "STAR BOLT";
        case SpellType::MoonFlame:
            return "MOON FLAME";
        case SpellType::CrystalBloom:
            return "CRYSTAL BLOOM";
        }

        return "STAR BOLT";
    }

    float SeededUnitValue(int seed)
    {
        const float value = std::sin(static_cast<float>(seed) * 12.9898f) * 43758.5453f;
        return value - std::floor(value);
    }

    void AddParticle(std::vector<MagicParticle>& particles,
        sf::Vector2f position,
        sf::Vector2f velocity,
        sf::Color color,
        float lifetime,
        float radius,
        ParticleKind kind)
    {
        constexpr std::size_t kMaxParticles = 620;
        if (particles.size() >= kMaxParticles)
        {
            particles.erase(particles.begin());
        }

        particles.push_back({ position, velocity, color, 0.0f, lifetime, radius, kind });
    }

    void SpawnMoveTrail(std::vector<MagicParticle>& particles, const Player& player, float horizontalInput, int& particleSeed)
    {
        constexpr std::array<sf::Color, 4> colors = {
            sf::Color(168, 118, 255, 190),
            sf::Color(98, 218, 255, 180),
            sf::Color(255, 222, 105, 170),
            sf::Color(123, 236, 154, 175)
        };

        const float trailingX = horizontalInput > 0.0f ? 4.0f : player.size.x - 4.0f;
        const sf::Vector2f basePosition = player.position + sf::Vector2f(trailingX, player.size.y - 10.0f);
        const float sideDrift = horizontalInput > 0.0f ? -1.0f : 1.0f;
        const float jitterX = (SeededUnitValue(particleSeed) - 0.5f) * 10.0f;
        const float jitterY = (SeededUnitValue(particleSeed + 7) - 0.5f) * 8.0f;
        const sf::Vector2f velocity(
            sideDrift * (24.0f + SeededUnitValue(particleSeed + 13) * 34.0f),
            -24.0f - SeededUnitValue(particleSeed + 23) * 32.0f);

        AddParticle(particles,
            basePosition + sf::Vector2f(jitterX, jitterY),
            velocity,
            colors[static_cast<std::size_t>(particleSeed) % colors.size()],
            0.55f,
            3.0f + SeededUnitValue(particleSeed + 31) * 2.0f,
            ParticleKind::Spark);
        particleSeed++;
    }

    void SpawnJumpBurst(std::vector<MagicParticle>& particles, const Player& player, bool doubleJump, int& particleSeed)
    {
        const sf::Vector2f footCenter = player.position + sf::Vector2f(player.size.x * 0.5f, player.size.y - 2.0f);
        const int particleCount = doubleJump ? 18 : 12;
        const sf::Color primaryColor = doubleJump ? sf::Color(177, 127, 255, 220) : sf::Color(255, 224, 116, 210);
        const sf::Color secondaryColor = doubleJump ? sf::Color(98, 218, 255, 210) : sf::Color(122, 239, 158, 190);

        for (int index = 0; index < particleCount; ++index)
        {
            const float t = particleCount == 1 ? 0.0f : static_cast<float>(index) / static_cast<float>(particleCount - 1);
            const float angle = kPi + t * kPi;
            const float speed = 90.0f + SeededUnitValue(particleSeed + index) * 95.0f;
            const sf::Vector2f velocity(std::cos(angle) * speed, std::sin(angle) * speed - 35.0f);
            const sf::Color color = index % 2 == 0 ? primaryColor : secondaryColor;

            AddParticle(particles,
                footCenter,
                velocity,
                color,
                doubleJump ? 0.7f : 0.55f,
                doubleJump ? 4.0f : 3.5f,
                ParticleKind::Spark);
        }

        particleSeed += particleCount;
    }

    void SpawnLandingPuff(std::vector<MagicParticle>& particles, const Player& player, int& particleSeed)
    {
        const sf::Vector2f footCenter = player.position + sf::Vector2f(player.size.x * 0.5f, player.size.y - 3.0f);

        for (int index = 0; index < 16; ++index)
        {
            const float t = static_cast<float>(index) / 15.0f;
            const float angle = kPi + t * kPi;
            const float speed = 55.0f + SeededUnitValue(particleSeed + index) * 70.0f;
            const sf::Vector2f velocity(std::cos(angle) * speed, std::sin(angle) * speed * 0.45f - 20.0f);
            const sf::Color color = index % 3 == 0 ? sf::Color(170, 130, 255, 135) : sf::Color(210, 220, 210, 125);

            AddParticle(particles,
                footCenter + sf::Vector2f(SeededUnitValue(particleSeed + index + 19) * 10.0f - 5.0f, 0.0f),
                velocity,
                color,
                0.5f,
                5.0f + SeededUnitValue(particleSeed + index + 29) * 4.0f,
                ParticleKind::Puff);
        }

        particleSeed += 16;
    }

    void SpawnCollectBurst(std::vector<MagicParticle>& particles, sf::Vector2f position, int& particleSeed)
    {
        constexpr std::array<sf::Color, 4> colors = {
            sf::Color(255, 226, 95, 230),
            sf::Color(255, 166, 76, 215),
            sf::Color(132, 239, 158, 210),
            sf::Color(109, 220, 255, 205)
        };

        for (int index = 0; index < 22; ++index)
        {
            const float t = static_cast<float>(index) / 22.0f;
            const float angle = t * kPi * 2.0f;
            const float speed = 70.0f + SeededUnitValue(particleSeed + index) * 105.0f;
            const sf::Vector2f velocity(std::cos(angle) * speed, std::sin(angle) * speed);

            AddParticle(particles,
                position,
                velocity,
                colors[static_cast<std::size_t>(index) % colors.size()],
                0.65f,
                3.0f + SeededUnitValue(particleSeed + index + 17) * 2.5f,
                ParticleKind::Spark);
        }

        particleSeed += 22;
    }

    void SpawnAbilityUnlockBurst(std::vector<MagicParticle>& particles, const Player& player, bool climbAbility, int& particleSeed)
    {
        const sf::Vector2f center = player.position + sf::Vector2f(player.size.x * 0.5f, player.size.y * 0.48f);
        const sf::Color primaryColor = climbAbility ? sf::Color(93, 235, 142, 225) : sf::Color(177, 127, 255, 225);
        const sf::Color secondaryColor = climbAbility ? sf::Color(183, 255, 195, 205) : sf::Color(102, 218, 255, 205);

        for (int index = 0; index < 34; ++index)
        {
            const float t = static_cast<float>(index) / 34.0f;
            const float angle = t * kPi * 2.0f;
            const float speed = 50.0f + SeededUnitValue(particleSeed + index) * 80.0f;
            const float startRadius = 18.0f + SeededUnitValue(particleSeed + index + 11) * 14.0f;
            const sf::Vector2f direction(std::cos(angle), std::sin(angle));

            AddParticle(particles,
                center + direction * startRadius,
                direction * speed + sf::Vector2f(0.0f, -20.0f),
                index % 2 == 0 ? primaryColor : secondaryColor,
                0.9f,
                4.0f + SeededUnitValue(particleSeed + index + 23) * 2.5f,
                ParticleKind::Spark);
        }

        particleSeed += 34;
    }

    void SpawnLevelTransitionBurst(std::vector<MagicParticle>& particles, const Player& player, int& particleSeed)
    {
        const sf::Vector2f center = player.position + sf::Vector2f(player.size.x * 0.5f, player.size.y * 0.5f);

        for (int index = 0; index < 46; ++index)
        {
            const float t = static_cast<float>(index) / 46.0f;
            const float angle = t * kPi * 2.0f;
            const float speed = 85.0f + SeededUnitValue(particleSeed + index) * 120.0f;
            const sf::Vector2f direction(std::cos(angle), std::sin(angle));
            const sf::Color color = index % 3 == 0
                ? sf::Color(255, 229, 112, 225)
                : (index % 3 == 1 ? sf::Color(116, 234, 169, 210) : sf::Color(130, 209, 255, 205));

            AddParticle(particles,
                center + direction * 10.0f,
                direction * speed + sf::Vector2f(0.0f, -25.0f),
                color,
                0.85f,
                4.0f + SeededUnitValue(particleSeed + index + 31) * 3.0f,
                ParticleKind::Spark);
        }

        particleSeed += 46;
    }

    void SpawnSpellCastBurst(std::vector<MagicParticle>& particles, SpellType spell, sf::Vector2f position, sf::Vector2f direction, int& particleSeed)
    {
        const sf::Vector2f perpendicular(-direction.y, direction.x);
        const sf::Color primaryColor = spell == SpellType::StarBolt
            ? sf::Color(255, 235, 110, 235)
            : (spell == SpellType::MoonFlame ? sf::Color(104, 234, 223, 225) : sf::Color(255, 128, 230, 230));
        const sf::Color secondaryColor = spell == SpellType::StarBolt
            ? sf::Color(116, 218, 255, 220)
            : (spell == SpellType::MoonFlame ? sf::Color(159, 113, 255, 220) : sf::Color(140, 228, 255, 220));

        for (int index = 0; index < 18; ++index)
        {
            const float spread = (SeededUnitValue(particleSeed + index) - 0.5f) * 130.0f;
            const float speed = 45.0f + SeededUnitValue(particleSeed + index + 9) * 90.0f;
            const sf::Vector2f velocity = direction * speed + perpendicular * spread;

            AddParticle(particles,
                position,
                velocity,
                index % 2 == 0 ? primaryColor : secondaryColor,
                0.45f,
                3.5f + SeededUnitValue(particleSeed + index + 17) * 2.5f,
                ParticleKind::Spark);
        }

        particleSeed += 18;
    }

    void SpawnSpellImpactBurst(std::vector<MagicParticle>& particles, SpellType spell, sf::Vector2f position, int& particleSeed)
    {
        const int particleCount = spell == SpellType::CrystalBloom ? 58 : 38;
        for (int index = 0; index < particleCount; ++index)
        {
            const float t = static_cast<float>(index) / static_cast<float>(particleCount);
            const float angle = t * kPi * 2.0f;
            const float speed = 90.0f + SeededUnitValue(particleSeed + index) * (spell == SpellType::CrystalBloom ? 170.0f : 115.0f);
            const sf::Vector2f direction(std::cos(angle), std::sin(angle));
            const sf::Color color = spell == SpellType::StarBolt
                ? (index % 3 == 0 ? sf::Color(255, 242, 116, 235) : (index % 3 == 1 ? sf::Color(109, 218, 255, 225) : sf::Color(255, 112, 213, 220)))
                : (spell == SpellType::MoonFlame
                    ? (index % 2 == 0 ? sf::Color(117, 239, 226, 225) : sf::Color(157, 115, 255, 210))
                    : (index % 3 == 0 ? sf::Color(255, 124, 236, 230) : (index % 3 == 1 ? sf::Color(127, 232, 255, 220) : sf::Color(255, 225, 115, 220))));

            AddParticle(particles,
                position + direction * 8.0f,
                direction * speed + sf::Vector2f(0.0f, -30.0f),
                color,
                spell == SpellType::MoonFlame ? 0.8f : 0.65f,
                spell == SpellType::CrystalBloom ? 5.0f : 4.0f,
                ParticleKind::Spark);
        }

        particleSeed += particleCount;
    }

    void SpawnSpellTrail(std::vector<MagicParticle>& particles, const SpellProjectile& spell, int& particleSeed)
    {
        if (spell.type == SpellType::StarBolt)
        {
            constexpr std::array<sf::Color, 5> colors = {
                sf::Color(255, 239, 101, 215),
                sf::Color(255, 108, 211, 210),
                sf::Color(98, 218, 255, 210),
                sf::Color(138, 245, 155, 205),
                sf::Color(188, 126, 255, 210)
            };

            for (int index = 0; index < 3; ++index)
            {
                const sf::Vector2f jitter(
                    (SeededUnitValue(particleSeed + index) - 0.5f) * 16.0f,
                    (SeededUnitValue(particleSeed + index + 7) - 0.5f) * 16.0f);
                AddParticle(particles,
                    spell.position + jitter,
                    -NormalizeVector(spell.velocity) * (35.0f + SeededUnitValue(particleSeed + index + 13) * 35.0f),
                    colors[static_cast<std::size_t>(particleSeed + index) % colors.size()],
                    0.5f,
                    3.0f,
                    ParticleKind::Spark);
            }
            particleSeed += 3;
            return;
        }

        if (spell.type == SpellType::MoonFlame)
        {
            for (int index = 0; index < 4; ++index)
            {
                const sf::Color color = index % 2 == 0 ? sf::Color(99, 235, 218, 170) : sf::Color(151, 112, 255, 165);
                const sf::Vector2f jitter(
                    (SeededUnitValue(particleSeed + index) - 0.5f) * 24.0f,
                    (SeededUnitValue(particleSeed + index + 11) - 0.5f) * 24.0f);
                AddParticle(particles,
                    spell.position + jitter,
                    sf::Vector2f((SeededUnitValue(particleSeed + index + 19) - 0.5f) * 38.0f, -28.0f),
                    color,
                    0.72f,
                    6.0f + SeededUnitValue(particleSeed + index + 23) * 4.0f,
                    ParticleKind::Puff);
            }
            particleSeed += 4;
            return;
        }

        for (int index = 0; index < 3; ++index)
        {
            const sf::Color color = index % 2 == 0 ? sf::Color(255, 125, 233, 210) : sf::Color(128, 228, 255, 205);
            const sf::Vector2f jitter(
                (SeededUnitValue(particleSeed + index) - 0.5f) * 18.0f,
                (SeededUnitValue(particleSeed + index + 7) - 0.5f) * 18.0f);
            AddParticle(particles,
                spell.position + jitter,
                -NormalizeVector(spell.velocity) * (28.0f + SeededUnitValue(particleSeed + index + 17) * 45.0f),
                color,
                0.58f,
                3.5f,
                ParticleKind::Spark);
        }
        particleSeed += 3;
    }

    void CastSpell(std::vector<SpellProjectile>& spells,
        std::vector<MagicParticle>& particles,
        const Player& player,
        SpellType activeSpell,
        sf::Vector2f target,
        int& particleSeed)
    {
        constexpr std::size_t kMaxSpells = 18;
        if (spells.size() >= kMaxSpells)
        {
            spells.erase(spells.begin());
        }

        const sf::Vector2f startPosition = GetMagicStickTip(player);
        const sf::Vector2f direction = NormalizeVector(target - startPosition);
        const float speed = activeSpell == SpellType::StarBolt
            ? 620.0f
            : (activeSpell == SpellType::MoonFlame ? 360.0f : 470.0f);
        const float lifetime = activeSpell == SpellType::StarBolt
            ? 0.82f
            : (activeSpell == SpellType::MoonFlame ? 1.25f : 0.95f);

        spells.push_back({ activeSpell, startPosition, direction * speed, 0.0f, lifetime, 0.0f, 0.0f });
        SpawnSpellCastBurst(particles, activeSpell, startPosition, direction, particleSeed);
    }

    void UpdateSpells(std::vector<SpellProjectile>& spells, std::vector<MagicParticle>& particles, float dt, int& particleSeed)
    {
        for (SpellProjectile& spell : spells)
        {
            spell.age += dt;
            spell.trailTimer += dt;
            spell.rotation += dt * (spell.type == SpellType::MoonFlame ? 120.0f : 260.0f);

            sf::Vector2f velocity = spell.velocity;
            if (spell.type == SpellType::MoonFlame)
            {
                const sf::Vector2f perpendicular(-NormalizeVector(spell.velocity).y, NormalizeVector(spell.velocity).x);
                velocity += perpendicular * std::sin(spell.age * 12.0f) * 95.0f;
            }
            else if (spell.type == SpellType::CrystalBloom)
            {
                velocity *= 0.92f + std::sin(spell.age * 9.0f) * 0.06f;
            }

            spell.position += velocity * dt;
            while (spell.trailTimer >= 0.018f)
            {
                SpawnSpellTrail(particles, spell, particleSeed);
                spell.trailTimer -= 0.018f;
            }
        }

        for (const SpellProjectile& spell : spells)
        {
            if (spell.age >= spell.lifetime)
            {
                SpawnSpellImpactBurst(particles, spell.type, spell.position, particleSeed);
            }
        }

        spells.erase(
            std::remove_if(spells.begin(),
                spells.end(),
                [](const SpellProjectile& spell)
                {
                    return spell.age >= spell.lifetime;
                }),
            spells.end());
    }

    void DrawSpellProjectiles(sf::RenderWindow& window, const std::vector<SpellProjectile>& spells)
    {
        for (const SpellProjectile& spell : spells)
        {
            if (spell.type == SpellType::StarBolt)
            {
                sf::CircleShape glow(18.0f, 8);
                glow.setOrigin({ 18.0f, 18.0f });
                glow.setPosition(spell.position);
                glow.setRotation(sf::degrees(spell.rotation));
                glow.setFillColor(sf::Color(255, 228, 88, 120));
                window.draw(glow);

                sf::CircleShape core(7.0f, 5);
                core.setOrigin({ 7.0f, 7.0f });
                core.setPosition(spell.position);
                core.setRotation(sf::degrees(-spell.rotation * 1.4f));
                core.setFillColor(sf::Color(255, 250, 205, 240));
                window.draw(core);
            }
            else if (spell.type == SpellType::MoonFlame)
            {
                const float pulse = (std::sin(spell.age * 14.0f) + 1.0f) * 0.5f;
                sf::CircleShape flame(17.0f + pulse * 5.0f, 18);
                flame.setOrigin({ flame.getRadius(), flame.getRadius() });
                flame.setPosition(spell.position);
                flame.setFillColor(sf::Color(75, 228, 211, 170));
                window.draw(flame);

                sf::CircleShape core(10.0f, 18);
                core.setOrigin({ 10.0f, 10.0f });
                core.setPosition(spell.position + sf::Vector2f(std::sin(spell.age * 18.0f) * 4.0f, 0.0f));
                core.setFillColor(sf::Color(161, 119, 255, 220));
                window.draw(core);
            }
            else
            {
                sf::ConvexShape crystal(4);
                crystal.setPoint(0, { 0.0f, -18.0f });
                crystal.setPoint(1, { 14.0f, 0.0f });
                crystal.setPoint(2, { 0.0f, 18.0f });
                crystal.setPoint(3, { -14.0f, 0.0f });
                crystal.setPosition(spell.position);
                crystal.setRotation(sf::degrees(spell.rotation));
                crystal.setFillColor(sf::Color(255, 126, 231, 205));
                window.draw(crystal);

                sf::CircleShape core(6.0f, 8);
                core.setOrigin({ 6.0f, 6.0f });
                core.setPosition(spell.position);
                core.setFillColor(sf::Color(142, 232, 255, 230));
                window.draw(core);
            }
        }
    }

    void DrawSpellHud(sf::RenderWindow& window, SpellType activeSpell)
    {
        sf::RectangleShape panel({ 268.0f, 58.0f });
        panel.setPosition({ 18.0f, 76.0f });
        panel.setFillColor(sf::Color(28, 24, 42, 210));
        panel.setOutlineThickness(2.0f);
        panel.setOutlineColor(sf::Color(142, 113, 198, 180));
        window.draw(panel);

        constexpr std::array<SpellType, 3> spells = {
            SpellType::StarBolt,
            SpellType::MoonFlame,
            SpellType::CrystalBloom
        };
        constexpr std::array<sf::Color, 3> colors = {
            sf::Color(255, 226, 88),
            sf::Color(98, 229, 216),
            sf::Color(255, 119, 232)
        };

        for (std::size_t index = 0; index < spells.size(); ++index)
        {
            const bool active = spells[index] == activeSpell;
            const sf::Vector2f origin(34.0f + static_cast<float>(index) * 44.0f, 93.0f);
            sf::RectangleShape slot({ 32.0f, 32.0f });
            slot.setPosition(origin);
            slot.setFillColor(active ? sf::Color(72, 57, 96, 235) : sf::Color(43, 36, 58, 220));
            slot.setOutlineThickness(active ? 3.0f : 1.0f);
            slot.setOutlineColor(active ? sf::Color(245, 232, 154) : sf::Color(108, 93, 132));
            window.draw(slot);

            sf::CircleShape icon(8.0f + static_cast<float>(index) * 1.5f, index == 2 ? 4 : 12);
            icon.setOrigin({ icon.getRadius(), icon.getRadius() });
            icon.setPosition(origin + sf::Vector2f(16.0f, 16.0f));
            icon.setRotation(sf::degrees(index == 2 ? 45.0f : 0.0f));
            icon.setFillColor(colors[index]);
            window.draw(icon);
        }

        drawBlockLabel(window,
            GetSpellName(activeSpell),
            { 170.0f, 96.0f },
            2.0f,
            3.0f,
            sf::Color(235, 232, 255));
    }

    void UpdateParticles(std::vector<MagicParticle>& particles, float dt)
    {
        for (MagicParticle& particle : particles)
        {
            particle.age += dt;
            particle.position += particle.velocity * dt;
            particle.velocity.y += particle.kind == ParticleKind::Puff ? 85.0f * dt : -8.0f * dt;
            particle.velocity.x *= 0.985f;
        }

        particles.erase(
            std::remove_if(particles.begin(),
                particles.end(),
                [](const MagicParticle& particle)
                {
                    return particle.age >= particle.lifetime;
                }),
            particles.end());
    }

    void DrawParticles(sf::RenderWindow& window, const std::vector<MagicParticle>& particles)
    {
        for (const MagicParticle& particle : particles)
        {
            const float lifeProgress = std::clamp(particle.age / particle.lifetime, 0.0f, 1.0f);
            sf::Color color = particle.color;
            color.a = static_cast<std::uint8_t>(static_cast<float>(color.a) * (1.0f - lifeProgress));

            const float radius = particle.kind == ParticleKind::Puff
                ? particle.radius * (1.0f + lifeProgress * 1.25f)
                : particle.radius * (1.0f - lifeProgress * 0.35f);

            sf::CircleShape spark(radius, particle.kind == ParticleKind::Puff ? 12 : 5);
            spark.setOrigin({ radius, radius });
            spark.setPosition(particle.position);
            spark.setFillColor(color);
            window.draw(spark);
        }
    }

    void ResetRun(std::vector <Level>& levels, Player& player, std::size_t& currentlevelindex, bool& gamewon, bool& gamelost, bool& jumpwasheld,
        sf::RenderWindow& window)
    {
        levels = createLevels();
        currentlevelindex = 0;
        gamewon = false;
        gamelost = false;
        jumpwasheld = false;
        PlacePlayerAtLevelSpawn(levels[currentlevelindex], player);
        refreshabilitesforlevel(player, currentlevelindex);
        window.setTitle(kWindowTitle);

    }

}

int main()
{
    sf::RenderWindow window(sf::VideoMode({kWindowWidth, kWindowHeight}), kWindowTitle);
    window.setFramerateLimit(60);

    std::vector<Level> levels = createLevels();
    Player             player;
    std::size_t        currentLevelIndex = 0;
    bool               gameWon = false;
    bool               gameLost = false;
    bool               jumpwasheld = false;
    sf::Clock          effectClock;
    std::vector<MagicParticle> magicParticles;
    std::vector<SpellProjectile> spellProjectiles;
    SpellType          activeSpell = SpellType::StarBolt;
    int                particleSeed = 1;
    float              trailSpawnTimer = 0.0f;
    float              spellCooldown = 0.0f;


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

            if (const auto* KeyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if (KeyPressed->code == sf::Keyboard::Key::Escape)
                {
                    window.close();
                }
                if (!CanRestart(gameWon, gameLost))
                {
                    if (KeyPressed->code == sf::Keyboard::Key::Num1 ||
                        KeyPressed->code == sf::Keyboard::Key::Numpad1)
                    {
                        activeSpell = SpellType::StarBolt;
                    }
                    else if (KeyPressed->code == sf::Keyboard::Key::Num2 ||
                        KeyPressed->code == sf::Keyboard::Key::Numpad2)
                    {
                        activeSpell = SpellType::MoonFlame;
                    }
                    else if (KeyPressed->code == sf::Keyboard::Key::Num3 ||
                        KeyPressed->code == sf::Keyboard::Key::Numpad3)
                    {
                        activeSpell = SpellType::CrystalBloom;
                    }
                }
                if (CanRestart(gameWon, gameLost) &&
                    (KeyPressed->code == sf::Keyboard::Key::R ||
                        KeyPressed->code == sf::Keyboard::Key::Enter))
                {
                    ResetRun(levels, player, currentLevelIndex, gameWon, gameLost, jumpwasheld, window);
                    effectClock.restart();
                    magicParticles.clear();
                    spellProjectiles.clear();
                    trailSpawnTimer = 0.0f;
                    spellCooldown = 0.0f;
                }
            }

            if (CanRestart(gameWon, gameLost))
            {
                if (const auto* mousepressed = event->getIf<sf::Event::MouseButtonPressed>())
                {
                    const sf::Vector2f clickposition(
                        static_cast<float>(mousepressed->position.x),
                        static_cast<float>(mousepressed->position.y));
                    if (mousepressed->button == sf::Mouse::Button::Left &&
                        IsPointInside(GetRestartButtonBounds(), clickposition))
                    {
                        ResetRun(levels, player, currentLevelIndex, gameWon, gameLost, jumpwasheld, window);
                        effectClock.restart();
                        magicParticles.clear();
                        spellProjectiles.clear();
                        trailSpawnTimer = 0.0f;
                        spellCooldown = 0.0f;
                    }
                }
            }
            else if (const auto* mousepressed = event->getIf<sf::Event::MouseButtonPressed>())
            {
                if (mousepressed->button == sf::Mouse::Button::Left && spellCooldown <= 0.0f)
                {
                    CastSpell(spellProjectiles,
                        magicParticles,
                        player,
                        activeSpell,
                        sf::Vector2f(static_cast<float>(mousepressed->position.x), static_cast<float>(mousepressed->position.y)),
                        particleSeed);
                    spellCooldown = activeSpell == SpellType::StarBolt ? 0.16f : 0.24f;
                }
            }
            
        }

        window.clear(sf::Color(24, 28, 36));
        UpdateParticles(magicParticles, kFrameSeconds);
        UpdateSpells(spellProjectiles, magicParticles, kFrameSeconds, particleSeed);
        if (spellCooldown > 0.0f)
        {
            spellCooldown = std::max(0.0f, spellCooldown - kFrameSeconds);
        }

         Level& currentLevel = levels[currentLevelIndex];

        if (!gameWon && !gameLost)
        {
            const bool wasOnGround = player.onGround;
            updateclimbwallcontact(player, currentLevel);
            const bool moveLeft = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A);
            const bool moveRight = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);
            const bool moveUp = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W);
            const bool moveDown = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S);
            const bool jumpheld = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) || moveUp;
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

            if (horizontalInput < 0.0f)
            {
                player.facingRight = false;
            }
            else if (horizontalInput > 0.0f)
            {
                player.facingRight = true;
            }

            player.velocity.x = horizontalInput * kMoveSpeed;
            if (jumppressedthisframe)
            {
                bool jumpedThisFrame = false;
                bool usedDoubleJump = false;

                if (player.onGround || (player.touchingClimbWall && player.canClimb))
                {
                    player.velocity.y = kJumpSpeed;
                    player.onGround = false;
                    jumpedThisFrame = true;
                }
                else if (player.extraJumpsRemaining > 0)
                {
                    player.velocity.y = kJumpSpeed;
                    player.extraJumpsRemaining--;
                    jumpedThisFrame = true;
                    usedDoubleJump = true;

                }

                if (jumpedThisFrame)
                {
                    SpawnJumpBurst(magicParticles, player, usedDoubleJump, particleSeed);
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

            const float verticalVelocityBeforeCollision = player.velocity.y;
            resolveHorizontalCollisions(player, currentLevel, kFixedDt);
            resolveVerticalCollisions(player, currentLevel, kFixedDt);
            updateclimbwallcontact(player, currentLevel);
            std::vector<sf::Vector2f> collectedIngredientPositions;
            for (const Ingredient& ingredient : currentLevel.ingredients)
            {
                if (ingredient.collected)
                {
                    continue;
                }

                const sf::FloatRect ingredientBounds(
                    ingredient.position - sf::Vector2f(10.0f, 10.0f),
                    { 20.0f, 20.0f });
                if (getPlayerBounds(player).findIntersection(ingredientBounds))
                {
                    collectedIngredientPositions.push_back(ingredient.position);
                }
            }

            CollectIngredients(player, currentLevel);
            for (const sf::Vector2f position : collectedIngredientPositions)
            {
                SpawnCollectBurst(magicParticles, position, particleSeed);
            }

            if (std::abs(player.velocity.x) > 1.0f && std::abs(horizontalInput) > 0.0f)
            {
                trailSpawnTimer += kFrameSeconds;
                while (trailSpawnTimer >= 0.035f)
                {
                    SpawnMoveTrail(magicParticles, player, horizontalInput, particleSeed);
                    trailSpawnTimer -= 0.035f;
                }
            }
            else
            {
                trailSpawnTimer = 0.035f;
            }

            if (!wasOnGround && player.onGround && verticalVelocityBeforeCollision > 150.0f)
            {
                SpawnLandingPuff(magicParticles, player, particleSeed);
            }

            player.position.x =
                std::clamp(player.position.x, 0.f, static_cast<float>(kWindowWidth) - player.size.x);

            
            if (player.position.y > kDeathThresholdY)
            {
                gameLost = true;
                jumpwasheld = false;
                player.velocity = { 0.0f, 0.0f };
                window.setTitle("Witch Potion Hunt - You Died! Click the Restart Button or Press 'R'");
            }
            else
            {
                const bool PortalReady = allIngredientsCollected(currentLevel);
                if (PortalReady && getPlayerBounds(player).findIntersection(currentLevel.cauldronArea))
                {
                    currentLevelIndex++;
                    if (currentLevelIndex >= levels.size())
                    {
                        currentLevelIndex = levels.size() - 1;
                        gameWon = true;
                        effectClock.restart();
                        magicParticles.clear();
                        spellProjectiles.clear();
                        trailSpawnTimer = 0.0f;
                        spellCooldown = 0.0f;
                        window.setTitle("Witch Potion Hunt - You Win! Click the Restart Button or Press 'R'");

                    }
                    else
                    {
                        const bool hadDoubleJump = player.canDoubleJump;
                        const bool hadClimb = player.canClimb;

                        PlacePlayerAtLevelSpawn(levels[currentLevelIndex], player);
                        refreshabilitesforlevel(player, currentLevelIndex);
                        trailSpawnTimer = 0.0f;
                        SpawnLevelTransitionBurst(magicParticles, player, particleSeed);

                        if (!hadDoubleJump && player.canDoubleJump)
                        {
                            SpawnAbilityUnlockBurst(magicParticles, player, false, particleSeed);
                        }

                        if (!hadClimb && player.canClimb)
                        {
                            SpawnAbilityUnlockBurst(magicParticles, player, true, particleSeed);
                        }

                    }
                }
            }

        }
        else
        {
            jumpwasheld = false;

        }

        const Level& levelToDraw = levels[currentLevelIndex];
        const bool PortalReady = allIngredientsCollected(levelToDraw);

        drawLevelBackground(window, currentLevelIndex, effectClock.getElapsedTime().asSeconds());

        for (const sf::FloatRect& solid : levelToDraw.solids)
        {
            drawSolidPlatform(window, solid, currentLevelIndex);
        }

        for (const sf::FloatRect& wall : levelToDraw.climbWalls)
        {
            drawClimbWall(window, wall);
        }

        for (const Ingredient& ingredient : levelToDraw.ingredients)
        {
            if (ingredient.collected)
            {
                continue;
            }

            drawIngredientGem(window, ingredient);
        }

        drawCauldron(window, levelToDraw,PortalReady);
        DrawParticles(window, magicParticles);
        DrawSpellProjectiles(window, spellProjectiles);
        drawPlayer(window, player, effectClock.getElapsedTime().asSeconds());
        drawHud(window, levelToDraw, player);
        DrawSpellHud(window, activeSpell);
        if (gameLost)
        {
            const sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
            const sf::Vector2f mousePoint(static_cast<float>(mousePosition.x),
                static_cast<float>(mousePosition.y));
            const sf::FloatRect restartButtonBounds = GetRestartButtonBounds();
            drawRestartOverlay(window,
                restartButtonBounds,
                IsPointInside(restartButtonBounds, mousePoint));
        }
        else if (gameWon)
        {
            const sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
            const sf::Vector2f mousePoint(static_cast<float>(mousePosition.x),
                static_cast<float>(mousePosition.y));
            const sf::FloatRect restartButtonBounds = GetRestartButtonBounds();
            drawWinOverlay(window,
                restartButtonBounds,
                IsPointInside(restartButtonBounds, mousePoint),
                effectClock.getElapsedTime().asSeconds());
        }

        window.display();
    }
}
