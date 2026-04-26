#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <SFML/Audio.hpp>
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

    enum class GameScreen
    {
        MainMenu,
        Settings,
        Playing
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

    struct EnemyProjectile
    {
        sf::Vector2f position;
        sf::Vector2f velocity;
        MobType sourceType{ MobType::Bat };
        int damage{ 10 };
        float age{ 0.0f };
        float lifetime{ 2.4f };
    };

    struct FloatingDamage
    {
        sf::Vector2f position;
        sf::Vector2f velocity;
        int value{ 0 };
        float age{ 0.0f };
        float lifetime{ 0.8f };
        sf::Color color{ sf::Color::White };
    };

    enum class SoundEffect
    {
        StarBolt,
        MoonFlame,
        CrystalCharge,
        CrystalExplosion,
        Hit,
        BatDefeat,
        ImpDefeat,
        GolemDefeat,
        Collect,
        Portal,
        Hurt,
        Jump,
        EnemyShoot,
        ManaEmpty,
        Music
    };

    struct GameAudio
    {
        std::array<sf::SoundBuffer, static_cast<std::size_t>(SoundEffect::Music) + 1> buffers;
        std::vector<sf::Sound> activeSounds;
        std::optional<sf::Sound> music;
    };

    sf::FloatRect GetRestartButtonBounds()
    {
        return sf::FloatRect({ 510.0f , 430.0f }, { 260.0f, 64.0f });
    }

    sf::FloatRect GetEndMenuButtonBounds()
    {
        return sf::FloatRect({ 510.0f , 508.0f }, { 260.0f, 56.0f });
    }

    sf::FloatRect GetMainMenuPlayBounds()
    {
        return sf::FloatRect({ 490.0f, 304.0f }, { 300.0f, 58.0f });
    }

    sf::FloatRect GetMainMenuSettingsBounds()
    {
        return sf::FloatRect({ 490.0f, 382.0f }, { 300.0f, 58.0f });
    }

    sf::FloatRect GetMainMenuQuitBounds()
    {
        return sf::FloatRect({ 490.0f, 460.0f }, { 300.0f, 58.0f });
    }

    sf::FloatRect GetSettingsBackBounds()
    {
        return sf::FloatRect({ 510.0f, 520.0f }, { 260.0f, 58.0f });
    }

    sf::FloatRect GetAbilityWarningOkBounds()
    {
        return sf::FloatRect({ 535.0f, 456.0f }, { 210.0f, 58.0f });
    }

    sf::FloatRect GetLevelIntroOkBounds()
    {
        return sf::FloatRect({ 535.0f, 512.0f }, { 210.0f, 58.0f });
    }

    bool IsPointInside(const sf::FloatRect& rect, const sf::Vector2f point)
    {
        return point.x >= rect.position.x && point.x <= rect.position.x + rect.size.x &&
            point.y >= rect.position.y && point.y <= rect.position.y + rect.size.y;

    }

sf::Vector2f GetMouseWorldPosition(const sf::RenderWindow& window)
{
    return window.mapPixelToCoords(sf::Mouse::getPosition(window));
}

sf::Vector2f GetEventWorldPosition(const sf::RenderWindow& window, sf::Vector2i PixelPosition)
{
    return window.mapPixelToCoords(PixelPosition);
}

    bool CanRestart(bool gamewon, bool gamelost)
    {
        return gamewon || gamelost;
    }

    std::array<std::string_view, 3> GetFairyHint(std::size_t levelindex, bool portalready)
    {
        if(portalready == true)
        {
            return{"The portal is ready!", "Go to the cauldron!", "Jump into the glow!"};

        }
        if(levelindex == 0)
        {
            return {"collect all four coins!", "Left click casts your spells.", "Mana regenerates with time."};
        }
        if(levelindex == 1)
        {
            return {"You can now double jump!", "Jump twice in the air!", "Time it carefully- you don't want to waste it..."};
        }
        return{"Climb the green vines!", "Hold \"W\" or \"S\" to climb the walls!", "Then you can reach the portal."};
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

    std::size_t GetSoundIndex(SoundEffect effect)
    {
        return static_cast<std::size_t>(effect);
    }

    int GetSpellDamage(SpellType spell)
    {
        switch (spell)
        {
        case SpellType::StarBolt:
            return 30;
        case SpellType::MoonFlame:
            return 7;
        case SpellType::CrystalBloom:
            return 42;
        }

        return 30;
    }

    float GetSpellManaCost(SpellType spell)
    {
        switch (spell)
        {
        case SpellType::StarBolt:
            return 12.0f;
        case SpellType::MoonFlame:
            return 3.5f;
        case SpellType::CrystalBloom:
            return 30.0f;
        }

        return 12.0f;
    }

    SoundEffect GetMobDefeatSound(MobType type)
    {
        switch (type)
        {
        case MobType::Bat:
            return SoundEffect::BatDefeat;
        case MobType::Imp:
            return SoundEffect::ImpDefeat;
        case MobType::Golem:
            return SoundEffect::GolemDefeat;
        }

        return SoundEffect::BatDefeat;
    }

    sf::Vector2f GetPlayerCenter(const Player& player)
    {
        return player.position + player.size * 0.5f;
    }

    sf::Vector2f GetMobCenter(const Mob& mob)
    {
        return mob.position + mob.size * 0.5f;
    }

    float SeededUnitValue(int seed)
    {
        const float value = std::sin(static_cast<float>(seed) * 12.9898f) * 43758.5453f;
        return value - std::floor(value);
    }

    std::vector<std::int16_t> MakeTone(float startFrequency, float endFrequency, float duration, float volume, bool noise)
    {
        constexpr unsigned int kSampleRate = 44100;
        const std::size_t sampleCount = static_cast<std::size_t>(duration * static_cast<float>(kSampleRate));
        std::vector<std::int16_t> samples(sampleCount);
        float phase = 0.0f;

        for (std::size_t index = 0; index < sampleCount; ++index)
        {
            const float t = static_cast<float>(index) / static_cast<float>(sampleCount);
            const float frequency = startFrequency + (endFrequency - startFrequency) * t;
            phase += frequency / static_cast<float>(kSampleRate);
            const float envelope = std::sin(t * kPi) * (1.0f - t * 0.35f);
            const float noiseValue = noise ? (SeededUnitValue(static_cast<int>(index) + 17) * 2.0f - 1.0f) * 0.35f : 0.0f;
            const float wave = std::sin(phase * kPi * 2.0f) * 0.75f + std::sin(phase * kPi * 4.0f) * 0.25f + noiseValue;
            samples[index] = static_cast<std::int16_t>(std::clamp(wave * envelope * volume, -1.0f, 1.0f) * 32767.0f);
        }

        return samples;
    }

    std::vector<std::int16_t> MakeBackgroundMusic()
    {
        constexpr unsigned int kSampleRate = 44100;
        constexpr float kDuration = 10.0f;
        const std::array<float, 8> notes = { 220.0f, 261.63f, 329.63f, 392.0f, 329.63f, 293.66f, 246.94f, 196.0f };
        const std::size_t sampleCount = static_cast<std::size_t>(kDuration * static_cast<float>(kSampleRate));
        std::vector<std::int16_t> samples(sampleCount);
        float melodyPhase = 0.0f;
        float bassPhase = 0.0f;

        for (std::size_t index = 0; index < sampleCount; ++index)
        {
            const float time = static_cast<float>(index) / static_cast<float>(kSampleRate);
            const std::size_t noteIndex = static_cast<std::size_t>(time * 2.0f) % notes.size();
            const float localBeat = std::fmod(time * 2.0f, 1.0f);
            const float melodyFrequency = notes[noteIndex];
            const float bassFrequency = notes[(noteIndex + 5) % notes.size()] * 0.5f;
            melodyPhase += melodyFrequency / static_cast<float>(kSampleRate);
            bassPhase += bassFrequency / static_cast<float>(kSampleRate);

            const float pluckEnvelope = std::exp(-localBeat * 3.4f) * 0.75f + 0.18f;
            const float fadeIn = std::min(time / 1.4f, 1.0f);
            const float fadeOut = std::min((kDuration - time) / 1.4f, 1.0f);
            const float loopEnvelope = std::min(fadeIn, fadeOut);
            const float melody = std::sin(melodyPhase * kPi * 2.0f) * 0.55f +
                std::sin(melodyPhase * kPi * 4.0f) * 0.18f;
            const float bass = std::sin(bassPhase * kPi * 2.0f) * 0.22f;
            const float shimmer = std::sin((melodyPhase * 3.0f + bassPhase) * kPi * 2.0f) * 0.08f;
            const float wave = (melody * pluckEnvelope + bass + shimmer) * loopEnvelope * 0.16f;
            samples[index] = static_cast<std::int16_t>(std::clamp(wave, -1.0f, 1.0f) * 32767.0f);
        }

        return samples;
    }

    void LoadGeneratedSound(sf::SoundBuffer& buffer, float startFrequency, float endFrequency, float duration, float volume, bool noise)
    {
        const std::vector<std::int16_t> samples = MakeTone(startFrequency, endFrequency, duration, volume, noise);
        const bool loaded = buffer.loadFromSamples(samples.data(), samples.size(), 1, 44100, { sf::SoundChannel::Mono });
        (void)loaded;
    }

    void LoadGeneratedMusic(sf::SoundBuffer& buffer)
    {
        const std::vector<std::int16_t> samples = MakeBackgroundMusic();
        const bool loaded = buffer.loadFromSamples(samples.data(), samples.size(), 1, 44100, { sf::SoundChannel::Mono });
        (void)loaded;
    }

    GameAudio CreateGameAudio()
    {
        GameAudio audio;
        LoadGeneratedSound(audio.buffers[GetSoundIndex(SoundEffect::StarBolt)], 880.0f, 1560.0f, 0.16f, 0.33f, false);
        LoadGeneratedSound(audio.buffers[GetSoundIndex(SoundEffect::MoonFlame)], 420.0f, 760.0f, 0.08f, 0.24f, true);
        LoadGeneratedSound(audio.buffers[GetSoundIndex(SoundEffect::CrystalCharge)], 260.0f, 540.0f, 0.24f, 0.25f, false);
        LoadGeneratedSound(audio.buffers[GetSoundIndex(SoundEffect::CrystalExplosion)], 150.0f, 80.0f, 0.34f, 0.36f, true);
        LoadGeneratedSound(audio.buffers[GetSoundIndex(SoundEffect::Hit)], 320.0f, 190.0f, 0.08f, 0.24f, true);
        LoadGeneratedSound(audio.buffers[GetSoundIndex(SoundEffect::BatDefeat)], 980.0f, 250.0f, 0.22f, 0.27f, true);
        LoadGeneratedSound(audio.buffers[GetSoundIndex(SoundEffect::ImpDefeat)], 520.0f, 120.0f, 0.3f, 0.34f, true);
        LoadGeneratedSound(audio.buffers[GetSoundIndex(SoundEffect::GolemDefeat)], 180.0f, 52.0f, 0.46f, 0.42f, true);
        LoadGeneratedSound(audio.buffers[GetSoundIndex(SoundEffect::Collect)], 720.0f, 1320.0f, 0.16f, 0.28f, false);
        LoadGeneratedSound(audio.buffers[GetSoundIndex(SoundEffect::Portal)], 260.0f, 980.0f, 0.5f, 0.28f, false);
        LoadGeneratedSound(audio.buffers[GetSoundIndex(SoundEffect::Hurt)], 180.0f, 70.0f, 0.24f, 0.34f, true);
        LoadGeneratedSound(audio.buffers[GetSoundIndex(SoundEffect::Jump)], 360.0f, 760.0f, 0.12f, 0.22f, false);
        LoadGeneratedSound(audio.buffers[GetSoundIndex(SoundEffect::EnemyShoot)], 320.0f, 680.0f, 0.14f, 0.22f, true);
        LoadGeneratedSound(audio.buffers[GetSoundIndex(SoundEffect::ManaEmpty)], 130.0f, 95.0f, 0.18f, 0.2f, false);
        LoadGeneratedMusic(audio.buffers[GetSoundIndex(SoundEffect::Music)]);
        return audio;
    }

    void StartBackgroundMusic(GameAudio& audio)
    {
        audio.music.emplace(audio.buffers[GetSoundIndex(SoundEffect::Music)]);
        audio.music->setLooping(true);
        audio.music->setVolume(18.0f);
        audio.music->play();
    }

    void PlaySound(GameAudio& audio, SoundEffect effect, float volume = 45.0f)
    {
        constexpr std::size_t kMaxActiveSounds = 24;
        if (audio.activeSounds.size() >= kMaxActiveSounds)
        {
            audio.activeSounds.erase(audio.activeSounds.begin());
        }

        audio.activeSounds.emplace_back(audio.buffers[GetSoundIndex(effect)]);
        audio.activeSounds.back().setVolume(volume);
        audio.activeSounds.back().play();
    }

    void UpdateAudio(GameAudio& audio)
    {
        audio.activeSounds.erase(
            std::remove_if(audio.activeSounds.begin(),
                audio.activeSounds.end(),
                [](const sf::Sound& sound)
                {
                    return sound.getStatus() == sf::SoundSource::Status::Stopped;
                }),
            audio.activeSounds.end());
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
            ? sf::Color(255, 250, 164, 245)
            : (spell == SpellType::MoonFlame ? sf::Color(70, 255, 209, 225) : sf::Color(255, 82, 231, 235));
        const sf::Color secondaryColor = spell == SpellType::StarBolt
            ? sf::Color(80, 185, 255, 225)
            : (spell == SpellType::MoonFlame ? sf::Color(45, 158, 255, 200) : sf::Color(255, 218, 112, 220));
        const int particleCount = spell == SpellType::MoonFlame ? 9 : (spell == SpellType::CrystalBloom ? 26 : 22);

        for (int index = 0; index < particleCount; ++index)
        {
            const float spread = (SeededUnitValue(particleSeed + index) - 0.5f) * (spell == SpellType::MoonFlame ? 86.0f : 150.0f);
            const float speed = 45.0f + SeededUnitValue(particleSeed + index + 9) * (spell == SpellType::StarBolt ? 145.0f : 90.0f);
            const sf::Vector2f velocity = direction * speed + perpendicular * spread;

            AddParticle(particles,
                position,
                velocity,
                index % 2 == 0 ? primaryColor : secondaryColor,
                spell == SpellType::CrystalBloom ? 0.7f : 0.45f,
                (spell == SpellType::MoonFlame ? 2.2f : 3.5f) + SeededUnitValue(particleSeed + index + 17) * 2.5f,
                ParticleKind::Spark);
        }

        particleSeed += particleCount;
    }

    void SpawnSpellImpactBurst(std::vector<MagicParticle>& particles, SpellType spell, sf::Vector2f position, int& particleSeed)
    {
        const int particleCount = spell == SpellType::CrystalBloom ? 96 : (spell == SpellType::StarBolt ? 48 : 24);
        for (int index = 0; index < particleCount; ++index)
        {
            const float t = static_cast<float>(index) / static_cast<float>(particleCount);
            const float angle = t * kPi * 2.0f;
            const float speed = 90.0f + SeededUnitValue(particleSeed + index) * (spell == SpellType::CrystalBloom ? 230.0f : 115.0f);
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
                spell == SpellType::CrystalBloom ? 1.05f : (spell == SpellType::MoonFlame ? 0.52f : 0.65f),
                spell == SpellType::CrystalBloom ? 6.5f : 4.0f,
                spell == SpellType::CrystalBloom && index % 4 == 0 ? ParticleKind::Puff : ParticleKind::Spark);
        }

        particleSeed += particleCount;
    }

    void SpawnMobDefeatBurst(std::vector<MagicParticle>& particles, const Mob& mob, int& particleSeed)
    {
        const sf::Vector2f center = mob.position + mob.size * 0.5f;
        const int particleCount = mob.type == MobType::Golem ? 72 : 44;
        for (int index = 0; index < particleCount; ++index)
        {
            const float t = static_cast<float>(index) / static_cast<float>(particleCount);
            const float angle = t * kPi * 2.0f;
            const sf::Vector2f direction(std::cos(angle), std::sin(angle));
            const float speed = 70.0f + SeededUnitValue(particleSeed + index) * 155.0f;
            const sf::Color color = mob.type == MobType::Bat
                ? (index % 2 == 0 ? sf::Color(166, 91, 224, 220) : sf::Color(255, 80, 110, 210))
                : (mob.type == MobType::Imp
                    ? (index % 2 == 0 ? sf::Color(255, 88, 80, 225) : sf::Color(255, 203, 82, 210))
                    : (index % 2 == 0 ? sf::Color(120, 255, 130, 220) : sf::Color(112, 135, 116, 210)));

            AddParticle(particles,
                center + direction * 8.0f,
                direction * speed + sf::Vector2f(0.0f, -40.0f),
                color,
                mob.type == MobType::Golem ? 0.95f : 0.72f,
                mob.type == MobType::Golem ? 6.0f : 4.5f,
                mob.type == MobType::Golem && index % 3 == 0 ? ParticleKind::Puff : ParticleKind::Spark);
        }

        particleSeed += particleCount;
    }

    void SpawnSpellTrail(std::vector<MagicParticle>& particles, const SpellProjectile& spell, int& particleSeed)
    {
        if (spell.type == SpellType::StarBolt)
        {
            constexpr std::array<sf::Color, 6> colors = {
                sf::Color(255, 249, 135, 230),
                sf::Color(255, 126, 217, 220),
                sf::Color(77, 204, 255, 220),
                sf::Color(116, 255, 167, 215),
                sf::Color(191, 135, 255, 220),
                sf::Color(255, 255, 255, 235)
            };

            for (int index = 0; index < 6; ++index)
            {
                const sf::Vector2f jitter(
                    (SeededUnitValue(particleSeed + index) - 0.5f) * 22.0f,
                    (SeededUnitValue(particleSeed + index + 7) - 0.5f) * 22.0f);
                AddParticle(particles,
                    spell.position + jitter,
                    -NormalizeVector(spell.velocity) * (55.0f + SeededUnitValue(particleSeed + index + 13) * 85.0f),
                    colors[static_cast<std::size_t>(particleSeed + index) % colors.size()],
                    0.62f,
                    3.0f + static_cast<float>(index % 3),
                    ParticleKind::Spark);
            }
            particleSeed += 6;
            return;
        }

        if (spell.type == SpellType::MoonFlame)
        {
            for (int index = 0; index < 3; ++index)
            {
                const sf::Color color = index % 2 == 0 ? sf::Color(71, 255, 203, 185) : sf::Color(63, 142, 255, 165);
                const sf::Vector2f jitter(
                    (SeededUnitValue(particleSeed + index) - 0.5f) * 16.0f,
                    (SeededUnitValue(particleSeed + index + 11) - 0.5f) * 16.0f);
                AddParticle(particles,
                    spell.position + jitter,
                    sf::Vector2f((SeededUnitValue(particleSeed + index + 19) - 0.5f) * 62.0f, -18.0f),
                    color,
                    0.38f,
                    3.5f + SeededUnitValue(particleSeed + index + 23) * 3.0f,
                    ParticleKind::Puff);
            }
            particleSeed += 3;
            return;
        }

        for (int index = 0; index < 6; ++index)
        {
            const sf::Color color = index % 3 == 0
                ? sf::Color(255, 94, 232, 220)
                : (index % 3 == 1 ? sf::Color(128, 228, 255, 205) : sf::Color(255, 224, 102, 210));
            const sf::Vector2f jitter(
                (SeededUnitValue(particleSeed + index) - 0.5f) * 30.0f,
                (SeededUnitValue(particleSeed + index + 7) - 0.5f) * 30.0f);
            AddParticle(particles,
                spell.position + jitter,
                -NormalizeVector(spell.velocity) * (20.0f + SeededUnitValue(particleSeed + index + 17) * 36.0f),
                color,
                0.85f,
                4.0f + SeededUnitValue(particleSeed + index + 25) * 3.5f,
                ParticleKind::Spark);
        }
        particleSeed += 6;
    }

    bool CastSpell(std::vector<SpellProjectile>& spells,
        std::vector<MagicParticle>& particles,
        Player& player,
        SpellType activeSpell,
        sf::Vector2f target,
        int& particleSeed,
        GameAudio& audio)
    {
        const float manaCost = GetSpellManaCost(activeSpell);
        if (player.mana < manaCost)
        {
            PlaySound(audio, SoundEffect::ManaEmpty, 26.0f);
            return false;
        }

        player.mana -= manaCost;

        constexpr std::size_t kMaxSpells = 36;
        if (spells.size() >= kMaxSpells)
        {
            spells.erase(spells.begin());
        }

        const sf::Vector2f startPosition = GetMagicStickTip(player);
        const sf::Vector2f direction = NormalizeVector(target - startPosition);
        const float speed = activeSpell == SpellType::StarBolt
            ? 980.0f
            : (activeSpell == SpellType::MoonFlame ? 740.0f : 250.0f);
        const float lifetime = activeSpell == SpellType::StarBolt
            ? 1.7f
            : (activeSpell == SpellType::MoonFlame ? 0.48f : 1.05f);

        spells.push_back({ activeSpell, startPosition, direction * speed, 0.0f, lifetime, 0.0f, 0.0f });
        SpawnSpellCastBurst(particles, activeSpell, startPosition, direction, particleSeed);
        PlaySound(audio,
            activeSpell == SpellType::StarBolt
                ? SoundEffect::StarBolt
                : (activeSpell == SpellType::MoonFlame ? SoundEffect::MoonFlame : SoundEffect::CrystalCharge),
            activeSpell == SpellType::MoonFlame ? 24.0f : 42.0f);
        return true;
    }

    void UpdateSpells(std::vector<SpellProjectile>& spells, std::vector<MagicParticle>& particles, float dt, int& particleSeed)
    {
        for (SpellProjectile& spell : spells)
        {
            spell.age += dt;
            spell.trailTimer += dt;
            spell.rotation += dt * (spell.type == SpellType::MoonFlame ? 520.0f : (spell.type == SpellType::CrystalBloom ? 120.0f : 360.0f));

            sf::Vector2f velocity = spell.velocity;
            if (spell.type == SpellType::MoonFlame)
            {
                const sf::Vector2f perpendicular(-NormalizeVector(spell.velocity).y, NormalizeVector(spell.velocity).x);
                velocity += perpendicular * std::sin(spell.age * 30.0f) * 75.0f;
            }
            else if (spell.type == SpellType::CrystalBloom)
            {
                velocity *= std::max(0.12f, 1.0f - spell.age * 0.8f);
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
            if (spell.age >= spell.lifetime && spell.type != SpellType::CrystalBloom)
            {
                SpawnSpellImpactBurst(particles, spell.type, spell.position, particleSeed);
            }
        }

        spells.erase(
            std::remove_if(spells.begin(),
                spells.end(),
                [](const SpellProjectile& spell)
                {
                    return spell.age >= spell.lifetime && spell.type != SpellType::CrystalBloom;
                }),
            spells.end());
    }

    void AddFloatingDamage(std::vector<FloatingDamage>& damageNumbers, sf::Vector2f position, int value, SpellType spell)
    {
        constexpr std::size_t kMaxDamageNumbers = 36;
        if (damageNumbers.size() >= kMaxDamageNumbers)
        {
            damageNumbers.erase(damageNumbers.begin());
        }

        const sf::Color color = spell == SpellType::StarBolt
            ? sf::Color(255, 236, 104)
            : (spell == SpellType::MoonFlame ? sf::Color(107, 239, 225) : sf::Color(255, 126, 231));
        damageNumbers.push_back({ position, { 0.0f, -52.0f }, value, 0.0f, 0.95f, color });
    }

    void UpdateFloatingDamage(std::vector<FloatingDamage>& damageNumbers, float dt)
    {
        for (FloatingDamage& damage : damageNumbers)
        {
            damage.age += dt;
            damage.position += damage.velocity * dt;
            damage.velocity.y += 20.0f * dt;
        }

        damageNumbers.erase(
            std::remove_if(damageNumbers.begin(),
                damageNumbers.end(),
                [](const FloatingDamage& damage)
                {
                    return damage.age >= damage.lifetime;
                }),
            damageNumbers.end());
    }

    void DrawFloatingDamage(sf::RenderWindow& window, const std::vector<FloatingDamage>& damageNumbers)
    {
        for (const FloatingDamage& damage : damageNumbers)
        {
            const float progress = std::clamp(damage.age / damage.lifetime, 0.0f, 1.0f);
            sf::Color shadow(22, 15, 24, static_cast<std::uint8_t>(210.0f * (1.0f - progress)));
            sf::Color color = damage.color;
            color.a = static_cast<std::uint8_t>(255.0f * (1.0f - progress));
            const std::string text = std::to_string(damage.value);

            drawCenteredBlockLabel(window, text, damage.position.x + 2.0f, damage.position.y + 2.0f, 3.0f, 4.0f, shadow);
            drawCenteredBlockLabel(window, text, damage.position.x, damage.position.y, 3.0f, 4.0f, color);
        }
    }

    void SpawnEnemyProjectile(std::vector<EnemyProjectile>& projectiles,
        const Mob& mob,
        const Player& player,
        GameAudio& audio)
    {
        constexpr std::size_t kMaxEnemyProjectiles = 28;
        if (projectiles.size() >= kMaxEnemyProjectiles)
        {
            projectiles.erase(projectiles.begin());
        }

        const sf::Vector2f start = GetMobCenter(mob);
        const sf::Vector2f direction = NormalizeVector(GetPlayerCenter(player) - start);
        const float speed = mob.type == MobType::Bat ? 260.0f : (mob.type == MobType::Imp ? 335.0f : 230.0f);
        projectiles.push_back({ start, direction * speed, mob.type, mob.damage, 0.0f, 2.6f });
        PlaySound(audio, SoundEffect::EnemyShoot, 24.0f);
    }

    void UpdateMobs(Level& level,
        const Player& player,
        std::vector<EnemyProjectile>& enemyProjectiles,
        GameAudio& audio,
        float elapsedSeconds)
    {
        for (Mob& mob : level.mobs)
        {
            if (!mob.alive)
            {
                continue;
            }

            mob.attackTimer = std::max(0.0f, mob.attackTimer - kFrameSeconds);

            if (mob.type == MobType::Bat)
            {
                const float speed = mob.behavior == MobBehavior::Fast ? 180.0f : (mob.behavior == MobBehavior::Chase ? 145.0f : 92.0f);
                if (mob.behavior == MobBehavior::Chase && GetVectorLength(GetPlayerCenter(player) - GetMobCenter(mob)) < 340.0f)
                {
                    mob.facingRight = GetPlayerCenter(player).x > GetMobCenter(mob).x;
                }
                mob.position.x += (mob.facingRight ? speed : -speed) * kFrameSeconds;
                if (mob.position.x < mob.patrolMinX)
                {
                    mob.position.x = mob.patrolMinX;
                    mob.facingRight = true;
                }
                else if (mob.position.x + mob.size.x > mob.patrolMaxX)
                {
                    mob.position.x = mob.patrolMaxX - mob.size.x;
                    mob.facingRight = false;
                }

                mob.position.y = mob.baseY + std::sin(elapsedSeconds * 3.2f + mob.phase) * 26.0f;
            }
            else if (mob.type == MobType::Imp)
            {
                const float playerCenterX = GetPlayerCenter(player).x;
                const float mobCenterX = GetMobCenter(mob).x;
                const bool chasing = mob.behavior == MobBehavior::Chase && std::abs(playerCenterX - mobCenterX) < 330.0f;
                const float speed = mob.behavior == MobBehavior::Fast ? 235.0f : (chasing ? 175.0f : 155.0f);
                if (chasing)
                {
                    mob.facingRight = playerCenterX > mobCenterX;
                }
                mob.position.x += (mob.facingRight ? speed : -speed) * kFrameSeconds;
                if (mob.position.x < mob.patrolMinX)
                {
                    mob.position.x = mob.patrolMinX;
                    mob.facingRight = true;
                }
                else if (mob.position.x + mob.size.x > mob.patrolMaxX)
                {
                    mob.position.x = mob.patrolMaxX - mob.size.x;
                    mob.facingRight = false;
                }
                mob.position.y = mob.baseY + std::abs(std::sin(elapsedSeconds * 8.0f + mob.phase)) * 4.0f;
            }
            else
            {
                const float playerCenterX = GetPlayerCenter(player).x;
                const float mobCenterX = GetMobCenter(mob).x;
                const bool chasing = mob.behavior == MobBehavior::Chase && std::abs(playerCenterX - mobCenterX) < 300.0f;
                const float speed = mob.behavior == MobBehavior::Fast ? 72.0f : (chasing ? 86.0f : 38.0f);
                if (chasing)
                {
                    mob.facingRight = playerCenterX > mobCenterX;
                }

                mob.position.x += (mob.facingRight ? speed : -speed) * kFrameSeconds;
                if (mob.position.x < mob.patrolMinX)
                {
                    mob.position.x = mob.patrolMinX;
                    mob.facingRight = true;
                }
                else if (mob.position.x + mob.size.x > mob.patrolMaxX)
                {
                    mob.position.x = mob.patrolMaxX - mob.size.x;
                    mob.facingRight = false;
                }
            }

            if (mob.behavior == MobBehavior::Shooter)
            {
                const float distanceToPlayer = GetVectorLength(GetPlayerCenter(player) - GetMobCenter(mob));
                if (distanceToPlayer < 520.0f && mob.attackTimer <= 0.0f)
                {
                    mob.facingRight = GetPlayerCenter(player).x > GetMobCenter(mob).x;
                    SpawnEnemyProjectile(enemyProjectiles, mob, player, audio);
                    mob.attackTimer = mob.attackCooldown;
                }
            }
        }
    }

    void UpdateEnemyProjectiles(std::vector<EnemyProjectile>& projectiles, float dt)
    {
        for (EnemyProjectile& projectile : projectiles)
        {
            projectile.age += dt;
            projectile.position += projectile.velocity * dt;
        }

        projectiles.erase(
            std::remove_if(projectiles.begin(),
                projectiles.end(),
                [](const EnemyProjectile& projectile)
                {
                    return projectile.age >= projectile.lifetime ||
                        projectile.position.x < -40.0f ||
                        projectile.position.x > static_cast<float>(kWindowWidth) + 40.0f ||
                        projectile.position.y < -40.0f ||
                        projectile.position.y > static_cast<float>(kWindowHeight) + 40.0f;
                }),
            projectiles.end());
    }

    void ResolveSpellMobHits(std::vector<SpellProjectile>& spells,
        Level& level,
        std::vector<MagicParticle>& particles,
        std::vector<FloatingDamage>& damageNumbers,
        int& particleSeed,
        GameAudio& audio)
    {
        for (auto spell = spells.begin(); spell != spells.end();)
        {
            bool consumed = false;

            if (spell->type == SpellType::CrystalBloom && spell->age >= spell->lifetime)
            {
                constexpr float kExplosionRadius = 112.0f;
                SpawnSpellImpactBurst(particles, spell->type, spell->position, particleSeed);
                PlaySound(audio, SoundEffect::CrystalExplosion, 52.0f);

                for (Mob& mob : level.mobs)
                {
                    if (!mob.alive)
                    {
                        continue;
                    }

                    const sf::Vector2f mobCenter = mob.position + mob.size * 0.5f;
                    const float distance = GetVectorLength(mobCenter - spell->position);
                    if (distance > kExplosionRadius)
                    {
                        continue;
                    }

                    const int damage = GetSpellDamage(spell->type);
                    mob.health -= damage;
                    AddFloatingDamage(damageNumbers, mob.position + sf::Vector2f(mob.size.x * 0.5f, -12.0f), damage, spell->type);
                    PlaySound(audio, SoundEffect::Hit, 32.0f);

                    if (mob.health <= 0)
                    {
                        mob.alive = false;
                        SpawnMobDefeatBurst(particles, mob, particleSeed);
                        PlaySound(audio, GetMobDefeatSound(mob.type), 46.0f);
                    }
                }

                spell = spells.erase(spell);
                continue;
            }

            if (spell->type == SpellType::CrystalBloom)
            {
                ++spell;
                continue;
            }

            const sf::FloatRect spellBounds(spell->position - sf::Vector2f(12.0f, 12.0f), { 24.0f, 24.0f });

            for (Mob& mob : level.mobs)
            {
                if (!mob.alive || !spellBounds.findIntersection(getMobBounds(mob)))
                {
                    continue;
                }

                const int damage = GetSpellDamage(spell->type);
                mob.health -= damage;
                AddFloatingDamage(damageNumbers,
                    mob.position + sf::Vector2f(mob.size.x * 0.5f, -12.0f),
                    damage,
                    spell->type);
                SpawnSpellImpactBurst(particles, spell->type, spell->position, particleSeed);
                PlaySound(audio, SoundEffect::Hit, 32.0f);

                if (mob.health <= 0)
                {
                    mob.alive = false;
                    SpawnMobDefeatBurst(particles, mob, particleSeed);
                    PlaySound(audio, GetMobDefeatSound(mob.type), 46.0f);
                }

                consumed = true;
                break;
            }

            if (consumed)
            {
                spell = spells.erase(spell);
            }
            else
            {
                ++spell;
            }
        }
    }

    void DrawEnemyProjectiles(sf::RenderWindow& window, const std::vector<EnemyProjectile>& projectiles)
    {
        for (const EnemyProjectile& projectile : projectiles)
        {
            const float pulse = (std::sin(projectile.age * 18.0f) + 1.0f) * 0.5f;
            const sf::Color outerColor = projectile.sourceType == MobType::Bat
                ? sf::Color(170, 93, 244, 165)
                : (projectile.sourceType == MobType::Imp ? sf::Color(255, 96, 78, 175) : sf::Color(126, 255, 136, 170));
            const sf::Color coreColor = projectile.sourceType == MobType::Bat
                ? sf::Color(250, 190, 255, 230)
                : (projectile.sourceType == MobType::Imp ? sf::Color(255, 219, 88, 235) : sf::Color(210, 255, 196, 235));

            sf::CircleShape glow(8.0f + pulse * 2.0f, 12);
            glow.setOrigin({ glow.getRadius(), glow.getRadius() });
            glow.setPosition(projectile.position);
            glow.setFillColor(outerColor);
            window.draw(glow);

            sf::CircleShape core(4.0f, 8);
            core.setOrigin({ 4.0f, 4.0f });
            core.setPosition(projectile.position);
            core.setFillColor(coreColor);
            window.draw(core);
        }
    }

    bool DamagePlayer(Player& player,
        int damage,
        sf::Vector2f sourcePosition,
        std::vector<MagicParticle>& particles,
        int& particleSeed,
        GameAudio& audio)
    {
        if (player.hurtCooldown > 0.0f)
        {
            return false;
        }

        player.health = std::max(0, player.health - damage);
        player.hurtCooldown = 0.85f;
        const float knockDirection = GetPlayerCenter(player).x < sourcePosition.x ? -1.0f : 1.0f;
        player.velocity.x = knockDirection * 210.0f;
        player.velocity.y = std::min(player.velocity.y, -250.0f);
        SpawnLandingPuff(particles, player, particleSeed);
        PlaySound(audio, SoundEffect::Hurt, 48.0f);
        return true;
    }

    void ResolveEnemyProjectileHits(std::vector<EnemyProjectile>& projectiles,
        Player& player,
        std::vector<MagicParticle>& particles,
        int& particleSeed,
        GameAudio& audio)
    {
        const sf::FloatRect playerBounds = getPlayerBounds(player);
        for (auto projectile = projectiles.begin(); projectile != projectiles.end();)
        {
            const sf::FloatRect projectileBounds(projectile->position - sf::Vector2f(7.0f, 7.0f), { 14.0f, 14.0f });
            if (playerBounds.findIntersection(projectileBounds))
            {
                DamagePlayer(player, projectile->damage, projectile->position, particles, particleSeed, audio);
                projectile = projectiles.erase(projectile);
            }
            else
            {
                ++projectile;
            }
        }
    }

    void DrawSpellProjectiles(sf::RenderWindow& window, const std::vector<SpellProjectile>& spells)
    {
        for (const SpellProjectile& spell : spells)
        {
            if (spell.type == SpellType::StarBolt)
            {
                const sf::Vector2f direction = NormalizeVector(spell.velocity);
                const sf::Vector2f perpendicular(-direction.y, direction.x);
                const sf::Vector2f tail = spell.position - direction * 46.0f;

                sf::ConvexShape beam(4);
                beam.setPoint(0, spell.position + direction * 18.0f);
                beam.setPoint(1, tail + perpendicular * 9.0f);
                beam.setPoint(2, tail - direction * 18.0f);
                beam.setPoint(3, tail - perpendicular * 9.0f);
                beam.setFillColor(sf::Color(255, 239, 92, 155));
                window.draw(beam);

                sf::ConvexShape coreBeam(4);
                coreBeam.setPoint(0, spell.position + direction * 13.0f);
                coreBeam.setPoint(1, tail + perpendicular * 3.0f);
                coreBeam.setPoint(2, tail - direction * 8.0f);
                coreBeam.setPoint(3, tail - perpendicular * 3.0f);
                coreBeam.setFillColor(sf::Color(255, 255, 235, 245));
                window.draw(coreBeam);

                sf::CircleShape core(7.0f, 5);
                core.setOrigin({ 7.0f, 7.0f });
                core.setPosition(spell.position + direction * 12.0f);
                core.setRotation(sf::degrees(-spell.rotation * 1.4f));
                core.setFillColor(sf::Color(255, 250, 205, 240));
                window.draw(core);
            }
            else if (spell.type == SpellType::MoonFlame)
            {
                const float pulse = (std::sin(spell.age * 14.0f) + 1.0f) * 0.5f;
                sf::CircleShape flame(8.0f + pulse * 3.0f, 14);
                flame.setOrigin({ flame.getRadius(), flame.getRadius() });
                flame.setPosition(spell.position);
                flame.setFillColor(sf::Color(53, 255, 197, 170));
                window.draw(flame);

                sf::CircleShape core(4.0f, 8);
                core.setOrigin({ 4.0f, 4.0f });
                core.setPosition(spell.position + sf::Vector2f(std::sin(spell.age * 18.0f) * 4.0f, 0.0f));
                core.setFillColor(sf::Color(217, 255, 242, 230));
                window.draw(core);
            }
            else
            {
                const float charge = std::clamp(spell.age / spell.lifetime, 0.0f, 1.0f);
                const float pulse = (std::sin(spell.age * 18.0f) + 1.0f) * 0.5f;

                sf::CircleShape warning(28.0f + charge * 34.0f, 32);
                warning.setOrigin({ warning.getRadius(), warning.getRadius() });
                warning.setPosition(spell.position);
                warning.setFillColor(sf::Color(255, 84, 222, static_cast<std::uint8_t>(30.0f + charge * 70.0f)));
                warning.setOutlineThickness(2.0f);
                warning.setOutlineColor(sf::Color(255, 226, 112, static_cast<std::uint8_t>(90.0f + pulse * 80.0f)));
                window.draw(warning);

                sf::ConvexShape crystal(4);
                crystal.setPoint(0, { 0.0f, -20.0f - pulse * 5.0f });
                crystal.setPoint(1, { 16.0f + charge * 5.0f, 0.0f });
                crystal.setPoint(2, { 0.0f, 20.0f + pulse * 5.0f });
                crystal.setPoint(3, { -16.0f - charge * 5.0f, 0.0f });
                crystal.setPosition(spell.position);
                crystal.setRotation(sf::degrees(spell.rotation));
                crystal.setFillColor(sf::Color(255, 96, 230, 220));
                window.draw(crystal);

                sf::CircleShape core(6.0f, 8);
                core.setOrigin({ 6.0f, 6.0f });
                core.setPosition(spell.position);
                core.setFillColor(sf::Color(142, 232, 255, 230));
                window.draw(core);
            }
        }
    }

    void DrawResourceBar(sf::RenderWindow& window,
        std::string_view label,
        sf::Vector2f position,
        sf::Vector2f size,
        float current,
        float maximum,
        sf::Color fillColor)
    {
        const float percent = maximum <= 0.0f ? 0.0f : std::clamp(current / maximum, 0.0f, 1.0f);
        drawBlockLabel(window, label, position, 1.55f, 1.8f, sf::Color(235, 232, 255));

        sf::RectangleShape back(size);
        back.setPosition(position + sf::Vector2f(62.0f, 0.0f));
        back.setFillColor(sf::Color(26, 23, 34, 235));
        back.setOutlineThickness(2.0f);
        back.setOutlineColor(sf::Color(91, 80, 116, 210));
        window.draw(back);

        drawPixelRect(window,
            position + sf::Vector2f(65.0f, 3.0f),
            { (size.x - 6.0f) * percent, size.y - 6.0f },
            fillColor);
        drawPixelRect(window,
            position + sf::Vector2f(65.0f, 3.0f),
            { (size.x - 6.0f) * percent, 3.0f },
            sf::Color(255, 255, 255, 55));
    }

    void drawHUDfairy(sf::RenderWindow& window, std::size_t levelindex, bool portalready, float elapsedSeconds)
    {
        const sf::Vector2f panelPosition(832.0f, 228.0f);
        sf::RectangleShape panel({ 188.0f, 122.0f });
        panel.setPosition({ 18.0f, 228.0f });
        panel.setFillColor(sf::Color(27, 23, 40, 205));
        panel.setOutlineThickness(2.0f);
        panel.setOutlineColor(sf::Color(138, 117, 194, 160));
        window.draw(panel);

        drawPixelRect(window, panel.getPosition() + sf::Vector2f(2.0f, 2.0f), { 426.0f, 4.0f }, sf::Color(255, 255, 255, 22));
        drawBlockLabel(window, "FAIRY SAYS", panel.getPosition() + sf::Vector2f(16.0f, 14.0f), 1.75f, 2.0f, sf::Color(242, 232, 186));

        const std::array<std::string_view, 3> hintlines = GetFairyHint(levelindex, portalready);

        for (std::size_t i = 0; i < hintlines.size(); i++)
        {
            drawBlockLabel(window, hintlines[i], 
                panelPosition + sf::Vector2f(18.0f, 42.0f + static_cast<float>(i)*24.0f), 
                1.45f, 1.6f, i == 0? sf::Color(235, 232, 255):sf::Color(196, 208, 232));
        }
        

        const sf::Vector2f fairyCenter = panel.getPosition() +
            sf::Vector2f(94.0f, 76.0f + std::sin(elapsedSeconds * 2.1f) * 4.5f);
        const float wingLift = std::sin(elapsedSeconds * 8.0f) * 5.0f;
        const float shimmer = (std::sin(elapsedSeconds * 3.6f) + 1.0f) * 0.5f;

        sf::CircleShape glow(30.0f, 22);
        glow.setOrigin({ 30.0f, 30.0f });
        glow.setPosition(fairyCenter + sf::Vector2f(0.0f, 2.0f));
        glow.setFillColor(sf::Color(132, 228, 255, static_cast<std::uint8_t>(42.0f + shimmer * 28.0f)));
        window.draw(glow);

        sf::CircleShape shadow(1.0f, 20);
        shadow.setScale({ 25.0f, 6.5f });
        shadow.setOrigin({ 1.0f, 1.0f });
        shadow.setPosition(panel.getPosition() + sf::Vector2f(94.0f, 101.0f));
        shadow.setFillColor(sf::Color(0, 0, 0, 90));
        window.draw(shadow);

        sf::CircleShape wing(17.0f, 18);
        wing.setOrigin({ 17.0f, 17.0f });
        wing.setFillColor(sf::Color(160, 244, 255, 118));

        wing.setPosition(fairyCenter + sf::Vector2f(-15.0f, -5.0f - wingLift * 0.35f));
        wing.setScale({ 0.9f, 1.45f });
        wing.setRotation(sf::degrees(-26.0f - wingLift));
        window.draw(wing);

        wing.setPosition(fairyCenter + sf::Vector2f(15.0f, -5.0f + wingLift * 0.35f));
        wing.setScale({ 0.9f, 1.45f });
        wing.setRotation(sf::degrees(26.0f + wingLift));
        window.draw(wing);

        sf::CircleShape head(8.0f, 18);
        head.setOrigin({ 8.0f, 8.0f });
        head.setPosition(fairyCenter + sf::Vector2f(0.0f, -18.0f));
        head.setFillColor(sf::Color(255, 224, 198));
        window.draw(head);

        sf::ConvexShape dress(4);
        dress.setPoint(0, { 0.0f, -16.0f });
        dress.setPoint(1, { 13.0f, 8.0f });
        dress.setPoint(2, { 0.0f, 24.0f });
        dress.setPoint(3, { -13.0f, 8.0f });
        dress.setPosition(fairyCenter);
        dress.setFillColor(sf::Color(112, 96, 255));
        window.draw(dress);

        drawPixelRect(window, fairyCenter + sf::Vector2f(-6.0f, -10.0f), { 12.0f, 14.0f }, sf::Color(147, 124, 255));
        drawPixelRect(window, fairyCenter + sf::Vector2f(-3.0f, 14.0f), { 2.0f, 12.0f }, sf::Color(255, 224, 198));
        drawPixelRect(window, fairyCenter + sf::Vector2f(1.0f, 14.0f), { 2.0f, 12.0f }, sf::Color(255, 224, 198));
        drawPixelRect(window, fairyCenter + sf::Vector2f(-14.0f, -1.0f), { 7.0f, 3.0f }, sf::Color(255, 224, 198));
        drawPixelRect(window, fairyCenter + sf::Vector2f(7.0f, -1.0f), { 7.0f, 3.0f }, sf::Color(255, 224, 198));
        drawPixelRect(window, fairyCenter + sf::Vector2f(-5.0f, -21.0f), { 10.0f, 4.0f }, sf::Color(115, 238, 173));
        drawPixelRect(window, fairyCenter + sf::Vector2f(-2.0f, -17.0f), { 4.0f, 3.0f }, sf::Color(82, 176, 121));
        drawPixelRect(window, fairyCenter + sf::Vector2f(-3.0f, -20.0f), { 2.0f, 2.0f }, sf::Color(45, 34, 58));
        drawPixelRect(window, fairyCenter + sf::Vector2f(1.0f, -20.0f), { 2.0f, 2.0f }, sf::Color(45, 34, 58));
        drawPixelRect(window, fairyCenter + sf::Vector2f(-1.0f, -15.0f), { 3.0f, 2.0f }, sf::Color(238, 120, 154));

        constexpr std::array<sf::Vector2f, 5> sparkleOffsets = {
            sf::Vector2f(-46.0f, -14.0f),
            sf::Vector2f(-30.0f, 18.0f),
            sf::Vector2f(34.0f, -22.0f),
            sf::Vector2f(46.0f, 12.0f),
            sf::Vector2f(20.0f, 30.0f)
        };

        for (std::size_t index = 0; index < sparkleOffsets.size(); ++index)
        {
            const float twinkle = (std::sin(elapsedSeconds * 4.4f + static_cast<float>(index) * 1.3f) + 1.0f) * 0.5f;
            const sf::Vector2f sparkleCenter = fairyCenter + sparkleOffsets[index] + sf::Vector2f(0.0f, std::sin(elapsedSeconds * 2.5f + static_cast<float>(index)) * 2.0f);
            const sf::Color sparkleColor = index % 2 == 0
                ? sf::Color(255, 239, 153, static_cast<std::uint8_t>(110.0f + twinkle * 110.0f))
                : sf::Color(127, 232, 255, static_cast<std::uint8_t>(110.0f + twinkle * 110.0f));

            drawPixelRect(window, sparkleCenter + sf::Vector2f(-1.0f, -5.0f), { 3.0f, 10.0f }, sparkleColor);
            drawPixelRect(window, sparkleCenter + sf::Vector2f(-5.0f, -1.0f), { 10.0f, 3.0f }, sparkleColor);
        }
    
    }

    void DrawSpellAndAbilityHud(sf::RenderWindow& window, SpellType activeSpell, const Player& player, 
        std::size_t levelindex, bool portalready, float elapsedSeconds)
    {
        sf::RectangleShape panel({ 430.0f, 198.0f });
        panel.setPosition({ 18.0f, 16.0f });
        panel.setFillColor(sf::Color(28, 24, 42, 210));
        panel.setOutlineThickness(2.0f);
        panel.setOutlineColor(sf::Color(142, 113, 198, 180));
        window.draw(panel);

        drawPixelRect(window, { 20.0f, 18.0f }, { 426.0f, 5.0f }, sf::Color(255, 255, 255, 24));
        DrawResourceBar(window, "HP", { 34.0f, 32.0f }, { 160.0f, 16.0f }, static_cast<float>(player.health), static_cast<float>(player.maxHealth), sf::Color(231, 73, 83, 235));
        DrawResourceBar(window, "MANA", { 34.0f, 56.0f }, { 160.0f, 16.0f }, player.mana, player.maxMana, sf::Color(83, 180, 255, 235));
        drawBlockLabel(window, "SPELLS", { 34.0f, 84.0f }, 1.8f, 2.0f, sf::Color(191, 204, 255));

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
            const sf::Vector2f origin(34.0f + static_cast<float>(index) * 44.0f, 108.0f);
            sf::RectangleShape slot({ 34.0f, 34.0f });
            slot.setPosition(origin);
            slot.setFillColor(active ? sf::Color(72, 57, 96, 235) : sf::Color(43, 36, 58, 220));
            slot.setOutlineThickness(active ? 3.0f : 1.0f);
            slot.setOutlineColor(active ? sf::Color(245, 232, 154) : sf::Color(108, 93, 132));
            window.draw(slot);

            drawBlockLabel(window, std::string_view(index == 0 ? "1" : (index == 1 ? "2" : "3")),
                origin + sf::Vector2f(3.0f, 3.0f),
                1.1f,
                1.0f,
                active ? sf::Color(250, 238, 165) : sf::Color(139, 128, 154));

            const sf::Vector2f center = origin + sf::Vector2f(18.0f, 18.0f);
            if (spells[index] == SpellType::StarBolt)
            {
                sf::CircleShape glow(9.0f, 8);
                glow.setOrigin({ 9.0f, 9.0f });
                glow.setPosition(center);
                glow.setFillColor(sf::Color(255, 226, 88, active ? 130 : 70));
                window.draw(glow);

                drawPixelRect(window, center + sf::Vector2f(-2.0f, -11.0f), { 4.0f, 22.0f }, colors[index]);
                drawPixelRect(window, center + sf::Vector2f(-11.0f, -2.0f), { 22.0f, 4.0f }, colors[index]);
                drawPixelRect(window, center + sf::Vector2f(-6.0f, -6.0f), { 12.0f, 12.0f }, sf::Color(255, 246, 171, 230));
            }
            else if (spells[index] == SpellType::MoonFlame)
            {
                sf::CircleShape flame(11.0f, 14);
                flame.setOrigin({ 11.0f, 11.0f });
                flame.setPosition(center + sf::Vector2f(0.0f, 1.0f));
                flame.setFillColor(sf::Color(52, 157, 255, active ? 205 : 125));
                window.draw(flame);

                sf::CircleShape core(7.0f, 12);
                core.setOrigin({ 7.0f, 7.0f });
                core.setPosition(center + sf::Vector2f(1.0f, 2.0f));
                core.setFillColor(colors[index]);
                window.draw(core);

                drawPixelRect(window, center + sf::Vector2f(-3.0f, -9.0f), { 5.0f, 9.0f }, sf::Color(220, 255, 246, 210));
            }
            else
            {
                sf::ConvexShape crystal(4);
                crystal.setPoint(0, { 0.0f, -12.0f });
                crystal.setPoint(1, { 10.0f, 0.0f });
                crystal.setPoint(2, { 0.0f, 12.0f });
                crystal.setPoint(3, { -10.0f, 0.0f });
                crystal.setPosition(center);
                crystal.setFillColor(colors[index]);
                window.draw(crystal);

                drawPixelRect(window, center + sf::Vector2f(-2.0f, -9.0f), { 4.0f, 18.0f }, sf::Color(255, 220, 249, 170));
                drawPixelRect(window, center + sf::Vector2f(1.0f, -2.0f), { 6.0f, 3.0f }, sf::Color(255, 236, 152, 180));
            }
        }

        drawBlockLabel(window,
            GetSpellName(activeSpell),
            { 178.0f, 113.0f },
            2.0f,
            3.0f,
            sf::Color(235, 232, 255));

        drawPixelRect(window, { 34.0f, 153.0f }, { 384.0f, 2.0f }, sf::Color(142, 113, 198, 90));
        drawBlockLabel(window, "ABILITIES", { 34.0f, 174.0f }, 1.55f, 1.8f, sf::Color(191, 204, 255));
        drawHudAbilityBadge(window,
            { 132.0f, 164.0f },
            { 112.0f, 38.0f },
            player.canDoubleJump && player.extraJumpsRemaining > 0,
            "D JUMP",
            false);
        drawHudAbilityBadge(window,
            { 260.0f, 164.0f },
            { 100.0f, 38.0f },
            player.canClimb,
            "CLIMB",
            true);

            drawHUDfairy(window, levelindex, portalready, elapsedSeconds);
    }

    void DrawAbilityWarningOverlay(sf::RenderWindow& window, const sf::FloatRect& buttonBounds, bool buttonHovered)
    {
        const sf::Vector2u windowSize = window.getSize();
        sf::RectangleShape scrim({ static_cast<float>(windowSize.x), static_cast<float>(windowSize.y) });
        scrim.setFillColor(sf::Color(8, 7, 13, 165));
        window.draw(scrim);

        sf::RectangleShape panel({ 560.0f, 300.0f });
        panel.setPosition({ 360.0f, 210.0f });
        panel.setFillColor(sf::Color(32, 27, 46, 245));
        panel.setOutlineThickness(4.0f);
        panel.setOutlineColor(sf::Color(238, 196, 88, 230));
        window.draw(panel);

        drawCenteredBlockLabel(window, "ABILITY WARNING", 640.0f, 246.0f, 3.0f, 4.0f, sf::Color(255, 237, 168));
        drawCenteredBlockLabel(window, "DOUBLE JUMP IS ONE USE", 640.0f, 316.0f, 2.0f, 3.0f, sf::Color(235, 232, 255));
        drawCenteredBlockLabel(window, "PRESS JUMP IN AIR CAREFULLY", 640.0f, 352.0f, 2.0f, 3.0f, sf::Color(199, 207, 232));

        sf::RectangleShape button({ buttonBounds.size.x, buttonBounds.size.y });
        button.setPosition(buttonBounds.position);
        button.setFillColor(buttonHovered ? sf::Color(255, 206, 99, 245) : sf::Color(217, 151, 74, 235));
        button.setOutlineThickness(3.0f);
        button.setOutlineColor(sf::Color(74, 42, 56, 235));
        window.draw(button);

        drawCenteredBlockLabel(window, "OK", buttonBounds.position.x + buttonBounds.size.x * 0.5f, buttonBounds.position.y + 18.0f, 2.5f, 4.0f, sf::Color(38, 28, 42));
    }

    void DrawMenuButton(sf::RenderWindow& window,
        const sf::FloatRect& bounds,
        std::string_view label,
        bool hovered,
        sf::Color fillColor = sf::Color(73, 146, 94))
    {
        sf::RectangleShape button({ bounds.size.x, bounds.size.y });
        button.setPosition(bounds.position);
        button.setFillColor(hovered ? sf::Color(255, 206, 99, 245) : fillColor);
        button.setOutlineThickness(3.0f);
        button.setOutlineColor(sf::Color(236, 225, 153));
        window.draw(button);

        drawCenteredBlockLabel(window,
            label,
            bounds.position.x + bounds.size.x * 0.5f,
            bounds.position.y + bounds.size.y * 0.5f - 10.0f,
            2.6f,
            4.0f,
            hovered ? sf::Color(38, 28, 42) : sf::Color(246, 250, 225));
    }

    void DrawMainMenu(sf::RenderWindow& window, sf::Vector2f mousePoint, float elapsedSeconds)
    {
        drawLevelBackground(window, 0, elapsedSeconds);

        sf::RectangleShape scrim({ static_cast<float>(kWindowWidth), static_cast<float>(kWindowHeight) });
        scrim.setFillColor(sf::Color(14, 12, 24, 135));
        window.draw(scrim);

        sf::RectangleShape panel({ 560.0f, 420.0f });
        panel.setPosition({ 360.0f, 150.0f });
        panel.setFillColor(sf::Color(32, 27, 46, 240));
        panel.setOutlineThickness(4.0f);
        panel.setOutlineColor(sf::Color(238, 196, 88, 220));
        window.draw(panel);

        drawCenteredBlockLabel(window, "WITCH POTION HUNT", 640.0f, 202.0f, 4.0f, 5.0f, sf::Color(255, 237, 168));
        drawCenteredBlockLabel(window, "COLLECT COINS CAST SPELLS SURVIVE", 640.0f, 266.0f, 1.8f, 2.0f, sf::Color(199, 207, 232));

        DrawMenuButton(window, GetMainMenuPlayBounds(), "PLAY", IsPointInside(GetMainMenuPlayBounds(), mousePoint), sf::Color(73, 146, 94));
        DrawMenuButton(window, GetMainMenuSettingsBounds(), "SETTINGS", IsPointInside(GetMainMenuSettingsBounds(), mousePoint), sf::Color(64, 91, 145));
        DrawMenuButton(window, GetMainMenuQuitBounds(), "QUIT", IsPointInside(GetMainMenuQuitBounds(), mousePoint), sf::Color(131, 65, 74));
    }

    void DrawSettingsScreen(sf::RenderWindow& window, sf::Vector2f mousePoint, float elapsedSeconds)
    {
        drawLevelBackground(window, 1, elapsedSeconds);

        sf::RectangleShape scrim({ static_cast<float>(kWindowWidth), static_cast<float>(kWindowHeight) });
        scrim.setFillColor(sf::Color(8, 11, 18, 150));
        window.draw(scrim);

        sf::RectangleShape panel({ 680.0f, 430.0f });
        panel.setPosition({ 300.0f, 130.0f });
        panel.setFillColor(sf::Color(28, 34, 48, 242));
        panel.setOutlineThickness(4.0f);
        panel.setOutlineColor(sf::Color(142, 190, 214, 220));
        window.draw(panel);

        drawCenteredBlockLabel(window, "SETTINGS", 640.0f, 180.0f, 4.0f, 5.0f, sf::Color(213, 238, 255));
        drawBlockLabel(window, "MOVE A D OR ARROWS", { 390.0f, 260.0f }, 2.0f, 3.0f, sf::Color(235, 232, 255));
        drawBlockLabel(window, "JUMP SPACE OR W", { 390.0f, 304.0f }, 2.0f, 3.0f, sf::Color(235, 232, 255));
        drawBlockLabel(window, "SPELLS 1 2 3", { 390.0f, 348.0f }, 2.0f, 3.0f, sf::Color(235, 232, 255));
        drawBlockLabel(window, "CAST WITH MOUSE", { 390.0f, 392.0f }, 2.0f, 3.0f, sf::Color(235, 232, 255));
        drawBlockLabel(window, "MANA RESTORES OVER TIME", { 390.0f, 436.0f }, 1.8f, 2.0f, sf::Color(199, 207, 232));

        DrawMenuButton(window, GetSettingsBackBounds(), "BACK", IsPointInside(GetSettingsBackBounds(), mousePoint), sf::Color(73, 146, 94));
    }

    void DrawMobPreview(sf::RenderWindow& window,
        MobType type,
        MobBehavior behavior,
        int damage,
        sf::Vector2f center,
        std::string_view label)
    {
        Mob mob;
        mob.type = type;
        mob.behavior = behavior;
        mob.size = type == MobType::Golem
            ? sf::Vector2f(54.0f, 66.0f)
            : (type == MobType::Imp ? sf::Vector2f(34.0f, 42.0f) : sf::Vector2f(40.0f, 25.0f));
        mob.position = center - mob.size * 0.5f + sf::Vector2f(0.0f, type == MobType::Bat ? 12.0f : 0.0f);
        mob.health = 10;
        mob.maxHealth = 10;
        mob.damage = damage;
        mob.alive = true;
        mob.facingRight = true;

        sf::RectangleShape pedestal({ 104.0f, 4.0f });
        pedestal.setOrigin({ 52.0f, 2.0f });
        pedestal.setPosition(center + sf::Vector2f(0.0f, 47.0f));
        pedestal.setFillColor(sf::Color(142, 113, 198, 115));
        window.draw(pedestal);

        drawMob(window, mob, 0, 0.0f);
        drawCenteredBlockLabel(window, label, center.x, center.y + 64.0f, 1.5f, 1.8f, sf::Color(235, 232, 255));
        drawCenteredBlockLabel(window, std::to_string(damage) + " DMG", center.x, center.y + 92.0f, 1.5f, 1.8f, sf::Color(255, 210, 126));
    }

    void DrawLevelIntroOverlay(sf::RenderWindow& window,
        std::size_t levelIndex,
        const sf::FloatRect& buttonBounds,
        bool buttonHovered)
    {
        sf::RectangleShape scrim({ static_cast<float>(kWindowWidth), static_cast<float>(kWindowHeight) });
        scrim.setFillColor(sf::Color(8, 7, 13, 175));
        window.draw(scrim);

        sf::RectangleShape panel({ 820.0f, 440.0f });
        panel.setPosition({ 230.0f, 120.0f });
        panel.setFillColor(sf::Color(32, 27, 46, 246));
        panel.setOutlineThickness(4.0f);
        panel.setOutlineColor(sf::Color(238, 196, 88, 230));
        window.draw(panel);

        const std::string title = "LEVEL " + std::to_string(levelIndex + 1);
        drawCenteredBlockLabel(window, title, 640.0f, 164.0f, 4.0f, 5.0f, sf::Color(255, 237, 168));
        drawCenteredBlockLabel(window, "GOAL COLLECT 4 COINS THEN ENTER PORTAL", 640.0f, 226.0f, 1.7f, 2.0f, sf::Color(235, 232, 255));
        drawCenteredBlockLabel(window, "MOBS HURT HP SPELLS USE MANA", 640.0f, 262.0f, 1.7f, 2.0f, sf::Color(199, 207, 232));

        if (levelIndex == 0)
        {
            DrawMobPreview(window, MobType::Bat, MobBehavior::Patrol, 14, { 340.0f, 330.0f }, "PATROL BAT");
            DrawMobPreview(window, MobType::Bat, MobBehavior::Fast, 12, { 540.0f, 330.0f }, "FAST BAT");
            DrawMobPreview(window, MobType::Imp, MobBehavior::Chase, 20, { 740.0f, 330.0f }, "CHASE IMP");
            DrawMobPreview(window, MobType::Bat, MobBehavior::Shooter, 10, { 940.0f, 330.0f }, "SHOOT BAT");
        }
        else if (levelIndex == 1)
        {
            DrawMobPreview(window, MobType::Imp, MobBehavior::Fast, 18, { 340.0f, 330.0f }, "FAST IMP");
            DrawMobPreview(window, MobType::Imp, MobBehavior::Chase, 22, { 540.0f, 330.0f }, "CHASE IMP");
            DrawMobPreview(window, MobType::Bat, MobBehavior::Shooter, 12, { 740.0f, 330.0f }, "SHOOT BAT");
            DrawMobPreview(window, MobType::Golem, MobBehavior::Patrol, 28, { 940.0f, 330.0f }, "GOLEM");
        }
        else
        {
            DrawMobPreview(window, MobType::Golem, MobBehavior::Chase, 34, { 340.0f, 330.0f }, "CHASE GOLEM");
            DrawMobPreview(window, MobType::Imp, MobBehavior::Fast, 20, { 540.0f, 330.0f }, "FAST IMP");
            DrawMobPreview(window, MobType::Bat, MobBehavior::Shooter, 14, { 740.0f, 330.0f }, "SHOOT BAT");
            DrawMobPreview(window, MobType::Imp, MobBehavior::Shooter, 18, { 940.0f, 330.0f }, "SHOOT IMP");
        }

        DrawMenuButton(window, buttonBounds, "OK", buttonHovered, sf::Color(217, 151, 74, 235));
    }

    void DrawEndOverlay(sf::RenderWindow& window,
        bool won,
        sf::Vector2f mousePoint,
        float elapsedSeconds)
    {
        const sf::Vector2u windowSize = window.getSize();

        sf::RectangleShape scrim({ static_cast<float>(windowSize.x), static_cast<float>(windowSize.y) });
        scrim.setFillColor(won ? sf::Color(10, 18, 18, 185) : sf::Color(12, 12, 18, 205));
        window.draw(scrim);

        if (won)
        {
            drawConfetti(window, elapsedSeconds);
        }

        sf::RectangleShape panel({ 500.0f, 390.0f });
        panel.setOrigin({ 250.0f, 195.0f });
        panel.setPosition({ static_cast<float>(windowSize.x) * 0.5f, static_cast<float>(windowSize.y) * 0.5f });
        panel.setFillColor(won ? sf::Color(34, 49, 40, 245) : sf::Color(42, 31, 30, 245));
        panel.setOutlineThickness(4.0f);
        panel.setOutlineColor(won ? sf::Color(247, 200, 94) : sf::Color(172, 120, 78));
        window.draw(panel);

        drawCenteredBlockLabel(window,
            won ? "YOU WIN" : "GAME OVER",
            panel.getPosition().x,
            panel.getPosition().y - 135.0f,
            won ? 8.0f : 7.0f,
            won ? 8.0f : 7.0f,
            won ? sf::Color(255, 220, 95) : sf::Color(240, 94, 86));

        sf::RectangleShape divider({ 320.0f, 4.0f });
        divider.setOrigin({ 160.0f, 2.0f });
        divider.setPosition(panel.getPosition() + sf::Vector2f(0.0f, -50.0f));
        divider.setFillColor(won ? sf::Color(112, 199, 131) : sf::Color(172, 120, 78));
        window.draw(divider);

        DrawMenuButton(window, GetRestartButtonBounds(), "RESTART", IsPointInside(GetRestartButtonBounds(), mousePoint), sf::Color(73, 146, 94));
        DrawMenuButton(window, GetEndMenuButtonBounds(), "MAIN MENU", IsPointInside(GetEndMenuButtonBounds(), mousePoint), sf::Color(64, 91, 145));
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
        ResetPlayerResources(player);
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
    GameAudio          audio = CreateGameAudio();
    std::vector<MagicParticle> magicParticles;
    std::vector<SpellProjectile> spellProjectiles;
    std::vector<EnemyProjectile> enemyProjectiles;
    std::vector<FloatingDamage> damageNumbers;
    SpellType          activeSpell = SpellType::StarBolt;
    int                particleSeed = 1;
    float              trailSpawnTimer = 0.0f;
    float              spellCooldown = 0.0f;
    bool               castingHeld = false;
    std::vector<bool>  abilityWarningDismissed(levels.size(), false);
    std::vector<bool>  levelIntroDismissed(levels.size(), false);
    GameScreen         screen = GameScreen::MainMenu;
    bool               abilityWarningVisible = false;
    bool               levelIntroVisible = false;

    StartBackgroundMusic(audio);

    ResetPlayerResources(player);
    PlacePlayerAtLevelSpawn(levels[currentLevelIndex], player);

    refreshabilitesforlevel(player, currentLevelIndex);

    const auto keyMatches = [](const sf::Event::KeyPressed& keyPressed,
                               sf::Keyboard::Key key,
                               sf::Keyboard::Scancode scancode)
    {
        return keyPressed.code == key || keyPressed.scancode == scancode;
    };

    const auto isKeyHeld = [](sf::Keyboard::Key key, sf::Keyboard::Scancode scancode)
    {
        return sf::Keyboard::isKeyPressed(key) || sf::Keyboard::isKeyPressed(scancode);
    };

    auto dismissAbilityWarning = [&]()
    {
        if (currentLevelIndex < abilityWarningDismissed.size())
        {
            abilityWarningDismissed[currentLevelIndex] = true;
        }
        abilityWarningVisible = false;
        castingHeld = false;
        jumpwasheld = isKeyHeld(sf::Keyboard::Key::Space, sf::Keyboard::Scan::Space) ||
            isKeyHeld(sf::Keyboard::Key::W, sf::Keyboard::Scan::W) ||
            isKeyHeld(sf::Keyboard::Key::Up, sf::Keyboard::Scan::Up);
    };

    auto clearRunEffects = [&]()
    {
        magicParticles.clear();
        spellProjectiles.clear();
        enemyProjectiles.clear();
        damageNumbers.clear();
        trailSpawnTimer = 0.0f;
        spellCooldown = 0.0f;
        castingHeld = false;
    };

    auto startNewRun = [&]()
    {
        ResetRun(levels, player, currentLevelIndex, gameWon, gameLost, jumpwasheld, window);
        abilityWarningDismissed.assign(levels.size(), false);
        levelIntroDismissed.assign(levels.size(), false);
        abilityWarningVisible = false;
        levelIntroVisible = true;
        screen = GameScreen::Playing;
        effectClock.restart();
        clearRunEffects();
    };

    auto goToMainMenu = [&]()
    {
        screen = GameScreen::MainMenu;
        abilityWarningVisible = false;
        levelIntroVisible = false;
        gameWon = false;
        gameLost = false;
        jumpwasheld = false;
        player.velocity = { 0.0f, 0.0f };
        clearRunEffects();
        window.setTitle(kWindowTitle);
    };


    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }

            if (screen == GameScreen::MainMenu)
            {
                if (const auto* KeyPressed = event->getIf<sf::Event::KeyPressed>())
                {
                    if (keyMatches(*KeyPressed, sf::Keyboard::Key::Escape, sf::Keyboard::Scan::Escape))
                    {
                        window.close();
                    }
                    else if (keyMatches(*KeyPressed, sf::Keyboard::Key::Enter, sf::Keyboard::Scan::Enter))
                    {
                        startNewRun();
                    }
                }
                else if (const auto* mousepressed = event->getIf<sf::Event::MouseButtonPressed>())
                {
                    const sf::Vector2f clickposition = GetEventWorldPosition(window, mousepressed->position);
                       
                        
                    if (mousepressed->button == sf::Mouse::Button::Left &&
                        IsPointInside(GetMainMenuPlayBounds(), clickposition))
                    {
                        startNewRun();
                    }
                    else if (mousepressed->button == sf::Mouse::Button::Left &&
                        IsPointInside(GetMainMenuSettingsBounds(), clickposition))
                    {
                        screen = GameScreen::Settings;
                    }
                    else if (mousepressed->button == sf::Mouse::Button::Left &&
                        IsPointInside(GetMainMenuQuitBounds(), clickposition))
                    {
                        window.close();
                    }
                }
                continue;
            }

            if (screen == GameScreen::Settings)
            {
                if (const auto* KeyPressed = event->getIf<sf::Event::KeyPressed>())
                {
                    if (keyMatches(*KeyPressed, sf::Keyboard::Key::Escape, sf::Keyboard::Scan::Escape) ||
                        keyMatches(*KeyPressed, sf::Keyboard::Key::Enter, sf::Keyboard::Scan::Enter))
                    {
                        screen = GameScreen::MainMenu;
                    }
                }
                else if (const auto* mousepressed = event->getIf<sf::Event::MouseButtonPressed>())
                {
                 const sf::Vector2f clickposition = GetEventWorldPosition(window, mousepressed->position);

                    if (mousepressed->button == sf::Mouse::Button::Left &&
                        IsPointInside(GetSettingsBackBounds(), clickposition))
                    {
                        screen = GameScreen::MainMenu;
                    }
                }
                continue;
            }

            if (const auto* KeyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if (keyMatches(*KeyPressed, sf::Keyboard::Key::Escape, sf::Keyboard::Scan::Escape))
                {
                    window.close();
                }
                if (levelIntroVisible)
                {
                    if (keyMatches(*KeyPressed, sf::Keyboard::Key::Enter, sf::Keyboard::Scan::Enter))
                    {
                        if (currentLevelIndex < levelIntroDismissed.size())
                        {
                            levelIntroDismissed[currentLevelIndex] = true;
                        }
                        levelIntroVisible = false;
                        castingHeld = false;
                        jumpwasheld = false;
                    }
                    continue;
                }
                if (abilityWarningVisible)
                {
                    if (keyMatches(*KeyPressed, sf::Keyboard::Key::Enter, sf::Keyboard::Scan::Enter))
                    {
                        dismissAbilityWarning();
                    }
                    continue;
                }
                if (!CanRestart(gameWon, gameLost))
                {
                    if (keyMatches(*KeyPressed, sf::Keyboard::Key::Num1, sf::Keyboard::Scan::Num1) ||
                        keyMatches(*KeyPressed, sf::Keyboard::Key::Numpad1, sf::Keyboard::Scan::Numpad1))
                    {
                        activeSpell = SpellType::StarBolt;
                    }
                    else if (keyMatches(*KeyPressed, sf::Keyboard::Key::Num2, sf::Keyboard::Scan::Num2) ||
                        keyMatches(*KeyPressed, sf::Keyboard::Key::Numpad2, sf::Keyboard::Scan::Numpad2))
                    {
                        activeSpell = SpellType::MoonFlame;
                    }
                    else if (keyMatches(*KeyPressed, sf::Keyboard::Key::Num3, sf::Keyboard::Scan::Num3) ||
                        keyMatches(*KeyPressed, sf::Keyboard::Key::Numpad3, sf::Keyboard::Scan::Numpad3))
                    {
                        activeSpell = SpellType::CrystalBloom;
                    }
                }
                if (CanRestart(gameWon, gameLost) &&
                    (keyMatches(*KeyPressed, sf::Keyboard::Key::R, sf::Keyboard::Scan::R) ||
                        keyMatches(*KeyPressed, sf::Keyboard::Key::Enter, sf::Keyboard::Scan::Enter)))
                {
                    startNewRun();
                }
            }

            if (levelIntroVisible)
            {
                if (const auto* mousepressed = event->getIf<sf::Event::MouseButtonPressed>())
                {
                const sf::Vector2f clickposition = GetEventWorldPosition(window, mousepressed->position);

                    if (mousepressed->button == sf::Mouse::Button::Left &&
                        IsPointInside(GetLevelIntroOkBounds(), clickposition))
                    {
                        if (currentLevelIndex < levelIntroDismissed.size())
                        {
                            levelIntroDismissed[currentLevelIndex] = true;
                        }
                        levelIntroVisible = false;
                        castingHeld = false;
                        jumpwasheld = false;
                    }
                }
                continue;
            }

            if (abilityWarningVisible)
            {
                if (const auto* mousepressed = event->getIf<sf::Event::MouseButtonPressed>())
                {
                const sf::Vector2f clickposition = GetEventWorldPosition(window, mousepressed->position);
                    if (mousepressed->button == sf::Mouse::Button::Left &&
                        IsPointInside(GetAbilityWarningOkBounds(), clickposition))
                    {
                        dismissAbilityWarning();
                    }
                }
                else if (const auto* mousereleased = event->getIf<sf::Event::MouseButtonReleased>())
                {
                    if (mousereleased->button == sf::Mouse::Button::Left)
                    {
                        castingHeld = false;
                    }
                }
                continue;
            }

            if (CanRestart(gameWon, gameLost))
            {
                if (const auto* mousepressed = event->getIf<sf::Event::MouseButtonPressed>())
                {
                const sf::Vector2f clickposition = GetEventWorldPosition(window, mousepressed->position);

                    if (mousepressed->button == sf::Mouse::Button::Left &&
                        IsPointInside(GetRestartButtonBounds(), clickposition))
                    {
                        startNewRun();
                    }
                    else if (mousepressed->button == sf::Mouse::Button::Left &&
                        IsPointInside(GetEndMenuButtonBounds(), clickposition))
                    {
                        goToMainMenu();
                    }
                }
            }
            else if (const auto* mousepressed = event->getIf<sf::Event::MouseButtonPressed>())
            {
                if (mousepressed->button == sf::Mouse::Button::Left)
                {
                    castingHeld = true;
                }
            }
            else if (const auto* mousereleased = event->getIf<sf::Event::MouseButtonReleased>())
            {
                if (mousereleased->button == sf::Mouse::Button::Left)
                {
                    castingHeld = false;
                }
            }
            
        }

        if (screen == GameScreen::MainMenu)
        {
        const sf::Vector2f mousePoint = GetMouseWorldPosition(window);
            window.clear(sf::Color(24, 28, 36));
            DrawMainMenu(window, mousePoint, effectClock.getElapsedTime().asSeconds());
            window.display();
            continue;
        }

        if (screen == GameScreen::Settings)
        {
        const sf::Vector2f mousePoint = GetMouseWorldPosition(window);

            window.clear(sf::Color(24, 28, 36));
            DrawSettingsScreen(window, mousePoint, effectClock.getElapsedTime().asSeconds());
            window.display();
            continue;
        }

        window.clear(sf::Color(24, 28, 36));
        UpdateAudio(audio);
        const bool gameplayPaused = abilityWarningVisible || levelIntroVisible || gameWon || gameLost;
        if (!gameplayPaused)
        {
            UpdateParticles(magicParticles, kFrameSeconds);
            UpdateSpells(spellProjectiles, magicParticles, kFrameSeconds, particleSeed);
            UpdateEnemyProjectiles(enemyProjectiles, kFrameSeconds);
            UpdateFloatingDamage(damageNumbers, kFrameSeconds);
        }
        if (!gameWon && !gameLost && !abilityWarningVisible && !levelIntroVisible)
        {
            player.mana = std::min(player.maxMana, player.mana + 13.0f * kFrameSeconds);
        }
        if (!gameplayPaused && player.hurtCooldown > 0.0f)
        {
            player.hurtCooldown = std::max(0.0f, player.hurtCooldown - kFrameSeconds);
        }
        if (!gameplayPaused && spellCooldown > 0.0f)
        {
            spellCooldown = std::max(0.0f, spellCooldown - kFrameSeconds);
        }

         Level& currentLevel = levels[currentLevelIndex];

        if (!gameWon && !gameLost && !abilityWarningVisible && !levelIntroVisible)
        {
            if (castingHeld && spellCooldown <= 0.0f)
            {
                
                const bool castSucceeded = CastSpell(spellProjectiles,
                    magicParticles,
                    player,
                    activeSpell,
                    GetMouseWorldPosition(window),
                    particleSeed,
                    audio);
                spellCooldown = castSucceeded
                    ? (activeSpell == SpellType::StarBolt
                        ? 0.25f
                        : (activeSpell == SpellType::MoonFlame ? 0.028f : 0.42f))
                    : 0.18f;
            }

            const bool wasOnGround = player.onGround;
            UpdateMobs(currentLevel, player, enemyProjectiles, audio, effectClock.getElapsedTime().asSeconds());
            ResolveSpellMobHits(spellProjectiles, currentLevel, magicParticles, damageNumbers, particleSeed, audio);
            ResolveEnemyProjectileHits(enemyProjectiles, player, magicParticles, particleSeed, audio);
            if (player.health <= 0)
            {
                gameLost = true;
                jumpwasheld = false;
                player.velocity = { 0.0f, 0.0f };
                window.setTitle("Witch Potion Hunt - Your magic faded! Click the Restart Button or Press 'R'");
            }

            updateclimbwallcontact(player, currentLevel);
            const bool moveLeft = isKeyHeld(sf::Keyboard::Key::A, sf::Keyboard::Scan::A) ||
                isKeyHeld(sf::Keyboard::Key::Left, sf::Keyboard::Scan::Left);
            const bool moveRight = isKeyHeld(sf::Keyboard::Key::D, sf::Keyboard::Scan::D) ||
                isKeyHeld(sf::Keyboard::Key::Right, sf::Keyboard::Scan::Right);
            const bool moveUp = isKeyHeld(sf::Keyboard::Key::W, sf::Keyboard::Scan::W) ||
                isKeyHeld(sf::Keyboard::Key::Up, sf::Keyboard::Scan::Up);
            const bool moveDown = isKeyHeld(sf::Keyboard::Key::S, sf::Keyboard::Scan::S) ||
                isKeyHeld(sf::Keyboard::Key::Down, sf::Keyboard::Scan::Down);
            const bool jumpheld = isKeyHeld(sf::Keyboard::Key::Space, sf::Keyboard::Scan::Space) || moveUp;
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
                    PlaySound(audio, SoundEffect::Jump, usedDoubleJump ? 34.0f : 28.0f);
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
                PlaySound(audio, SoundEffect::Collect, 34.0f);
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
            else if (!gameLost)
            {
                for (const Mob& mob : currentLevel.mobs)
                {
                    if (mob.alive && getPlayerBounds(player).findIntersection(getMobBounds(mob)))
                    {
                        DamagePlayer(player, mob.damage, GetMobCenter(mob), magicParticles, particleSeed, audio);
                        if (player.health <= 0)
                        {
                            gameLost = true;
                            jumpwasheld = false;
                            player.velocity = { 0.0f, 0.0f };
                            window.setTitle("Witch Potion Hunt - A monster got you! Click the Restart Button or Press 'R'");
                        }
                        break;
                    }
                }
            }

            if (!gameLost)
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
                        PlaySound(audio, SoundEffect::Portal, 48.0f);
                        magicParticles.clear();
                        spellProjectiles.clear();
                        enemyProjectiles.clear();
                        damageNumbers.clear();
                        trailSpawnTimer = 0.0f;
                        spellCooldown = 0.0f;
                        castingHeld = false;
                        window.setTitle("Witch Potion Hunt - You Win! Click the Restart Button or Press 'R'");

                    }
                    else
                    {
                        const bool hadDoubleJump = player.canDoubleJump;
                        const bool hadClimb = player.canClimb;

                        PlacePlayerAtLevelSpawn(levels[currentLevelIndex], player);
                        refreshabilitesforlevel(player, currentLevelIndex);
                        trailSpawnTimer = 0.0f;
                        castingHeld = false;
                        enemyProjectiles.clear();
                        SpawnLevelTransitionBurst(magicParticles, player, particleSeed);
                        PlaySound(audio, SoundEffect::Portal, 44.0f);

                        if (!hadDoubleJump && player.canDoubleJump)
                        {
                            SpawnAbilityUnlockBurst(magicParticles, player, false, particleSeed);
                        }

                        if (!hadClimb && player.canClimb)
                        {
                            SpawnAbilityUnlockBurst(magicParticles, player, true, particleSeed);
                        }

                        if (currentLevelIndex < levelIntroDismissed.size() &&
                            !levelIntroDismissed[currentLevelIndex])
                        {
                            levelIntroVisible = true;
                            jumpwasheld = false;
                        }

                        if (player.canDoubleJump &&
                            currentLevelIndex < abilityWarningDismissed.size() &&
                            !abilityWarningDismissed[currentLevelIndex])
                        {
                            abilityWarningVisible = true;
                            jumpwasheld = false;
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

        drawPortal(window, levelToDraw, PortalReady, currentLevelIndex, effectClock.getElapsedTime().asSeconds());

        for (const Mob& mob : levelToDraw.mobs)
        {
            drawMob(window, mob, currentLevelIndex, effectClock.getElapsedTime().asSeconds());
        }
        DrawParticles(window, magicParticles);
        DrawSpellProjectiles(window, spellProjectiles);
        DrawEnemyProjectiles(window, enemyProjectiles);
        DrawFloatingDamage(window, damageNumbers);
        drawPlayer(window, player, effectClock.getElapsedTime().asSeconds());
        DrawSpellAndAbilityHud(window, activeSpell, player, currentLevelIndex, PortalReady, effectClock.getElapsedTime().asSeconds());
        drawCoinsHud(window, levelToDraw);
        if (gameLost)
        {
            const sf::Vector2f mousePoint = GetMouseWorldPosition(window);

            DrawEndOverlay(window, false, mousePoint, effectClock.getElapsedTime().asSeconds());
        }
        else if (gameWon)
        {
            const sf::Vector2f mousePoint = GetMouseWorldPosition(window);

            DrawEndOverlay(window, true, mousePoint, effectClock.getElapsedTime().asSeconds());
        }
        else if (levelIntroVisible)
        {
            const sf::Vector2f mousePoint = GetMouseWorldPosition(window);

            const sf::FloatRect okButtonBounds = GetLevelIntroOkBounds();
            DrawLevelIntroOverlay(window,
                currentLevelIndex,
                okButtonBounds,
                IsPointInside(okButtonBounds, mousePoint));
        }
        else if (abilityWarningVisible)
        {
            const sf::Vector2f mousePoint = GetMouseWorldPosition(window);

            const sf::FloatRect okButtonBounds = GetAbilityWarningOkBounds();
            DrawAbilityWarningOverlay(window,
                okButtonBounds,
                IsPointInside(okButtonBounds, mousePoint));
        }

        window.display();
    }
}
