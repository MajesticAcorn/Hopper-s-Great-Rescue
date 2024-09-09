#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <math.h>
#include <vector>
#include <cstdlib>
#include "Players.h"
#include <stdlib.h> //srand
#include <stdio.h>
#include <random>

using namespace std;
using namespace sf;


// Global variables for font, text, and buttons used in the start screen
sf::Font font;
sf::Text startText;
sf::Text gameText;
sf::Text quitText;
sf::Text qGameText;
sf::Text titleText;
sf::RectangleShape startButton;
sf::RectangleShape quitButton;
sf::RectangleShape easy;
sf::RectangleShape medium;
sf::RectangleShape hard;
sf::Text youText;
sf::Text winText;
sf::Text deathText;
sf::Text playText;
sf::Text againText;
sf::Text playGOText;
sf::Text againGOText;
sf::Texture halfHeartTexture;
sf::Texture fullHeartTexture;
sf::Texture emptyHeartTexture;
sf::Texture enemy2Texture;
sf::Texture enemyTexture;
Time shootCooldown = milliseconds(700);
sf::Texture shieldTexture;
sf::Sprite shieldSprite;
sf::Texture shieldPUTexture;
sf::Texture attackPUTexture;
sf::Texture speedPUTexture;
sf::Texture healthPUTexture;
sf::Text easyText;
sf::Text mediumText;
sf::Text hardText;
sf::SoundBuffer explosionBuffer;
sf::Sound explosionSound;

// Global variables for game time management and clocks
float dt; // Delta time for consistent frame rates
Clock dt_clock; // Clock to track delta time
Clock bulletClock; // Clock to manage player shooting cooldown
Clock enemyBulletClock; // Clock to manage enemy shooting cooldown
Clock inputDelayClock; // Clock to prevent immediate input after state changes

// Enum to manage game states
enum GameState
{
    START_SCREEN,
    LEVEL_ONE,
    LEVEL_TWO,
    LEVEL_THREE,
    LEVEL_FOUR,
    WIN_SCREEN,
    DEATH_SCREEN,
};

// Class to manage bullets fired by the player
class Bullet
{
public:
    float speed;
    CircleShape bullet; // Bullet's visual representation
    Vector2f currentVelocity; // Bullet's current velocity

    Bullet(float radius = 8.f)
        : currentVelocity(0.f, 0.f), speed(15.f)
    {
        bullet.setRadius(radius);
        bullet.setFillColor(Color::White); // Set player bullet color
    }
};

// Class to manage bullets fired by enemies
class enemyBullet
{
public:
    float enemyBulletSpeed;
    CircleShape eBullet; // Bullet's visual representation
    Vector2f enemyBulletcurrentVel; // Bullet's current velocity

    enemyBullet(float radius = 8.f)
        : enemyBulletcurrentVel(0.f, 0.f), enemyBulletSpeed(15.f)
    {
        eBullet.setRadius(radius);
        eBullet.setFillColor(Color::Red); // Set enemy bullet color
    }
};

class FollowerEnemy
{
public:
    RectangleShape fEnemy;
    float fSpeed;
    int fPositionx;
    int fPositiony;
    bool hasExploded = false;
    sf::Clock explosionClock;
    sf::Clock lingerClock;
    bool explosionVisible = false;
    bool markedForDeletion = false;

    FollowerEnemy(int x, int y)
    {
        fPositionx = x;
        fPositiony = y;

        fEnemy.setSize(sf::Vector2f(50.f, 50.f));
        fEnemy.setPosition(fPositionx, fPositiony);
        fSpeed = 150.f;
    }

    void followPlayer(const Players& player, float dt)
    {
        //Get direction vector to player
        Vector2f fDirection = player.rectangle.getPosition() - fEnemy.getPosition();

        //Normalize
        float fLength = sqrtf(pow(fDirection.x, 2) + pow(fDirection.y, 2));
        if (fLength != 0)
        {
            fDirection /= fLength;
        }

        //Move enemy towards player
        fEnemy.move(fDirection * fSpeed * dt);
    }

    void draw(sf::RenderWindow& window)
    {
        window.draw(fEnemy);
    }

    void setTexture(const sf::Texture& texture)
    {
        fEnemy.setTexture(&texture);
        fEnemy.setTextureRect(sf::IntRect(450, 650, 800, 820));
    }

    // Function to handle the explosion effect and check if the player is within it
    void explode(std::vector<FollowerEnemy>& fEnemies, Players& player, sf::RenderWindow& window)
    {
        Time explosionCooldown = seconds(5);
        Time lingerDuration = milliseconds(200); // Explosion lingers for 200 milliseconds
        static bool playerDamaged = false; // Flag to check if player was already damaged

        // If the explosion clock reaches the cooldown, start the explosion
        if (!hasExploded && explosionClock.getElapsedTime() > explosionCooldown)
        {
            explosionSound.play();
            hasExploded = true;
            explosionVisible = true;
            lingerClock.restart(); // Start the linger clock when the explosion happens
            fEnemy.setFillColor(sf::Color(0, 0, 0, 0)); // Make the enemy invisible
            playerDamaged = false; // Reset the damage flag for each explosion
        }

        // Draw the explosion when it occurs
        if (hasExploded && explosionVisible)
        {
            CircleShape explosion(100.f);  // Explosion radius of 100 pixels
            explosion.setPosition(fEnemy.getPosition().x - explosion.getRadius(), fEnemy.getPosition().y - explosion.getRadius());
            explosion.setFillColor(sf::Color(255, 0, 0, 100)); // Red, transparent explosion

            // Check if the player is inside the explosion area and hasn't been damaged yet
            if (explosion.getGlobalBounds().intersects(player.rectangle.getGlobalBounds()) && !playerDamaged)
            {
                player.health -= 3;  // Decrease player's health by 3
                playerDamaged = true; // Set the flag so damage is applied only once
            }

            // Draw the explosion effect
            window.draw(explosion);
        }

        // Check if the lingering time has passed
        if (hasExploded && lingerClock.getElapsedTime() > lingerDuration)
        {
            explosionVisible = false; // Stop showing the explosion
            markedForDeletion = true; // Now mark for deletion after lingering
        }
    }
};

void handleFollowers(std::vector<FollowerEnemy>& fEnemies, Players& player, const std::vector<RectangleShape>& walls, float dt, sf::RenderWindow& window)
{
    for (size_t j = 0; j < fEnemies.size(); j++)
    {
        FollowerEnemy& follower = fEnemies[j];

        // If the enemy hasn't exploded yet, allow it to follow the player
        if (!follower.hasExploded)
        {
            Vector2f fOldPosition = follower.fEnemy.getPosition();
            follower.followPlayer(player, dt);

            // Handle collision with walls
            for (auto& wall : walls)
            {
                FloatRect wallBounds = wall.getGlobalBounds();
                FloatRect followerBounds = follower.fEnemy.getGlobalBounds();

                if (followerBounds.intersects(wallBounds))
                {
                    follower.fEnemy.setPosition(fOldPosition);  // Reset to old position on collision
                    break;
                }
            }

            follower.draw(window);  // Draw the follower enemy if it hasn't exploded
        }

        // Check for explosion and manage lingering explosion effect
        follower.explode(fEnemies, player, window);
    }

    // Remove enemies marked for deletion
    fEnemies.erase(std::remove_if(fEnemies.begin(), fEnemies.end(), [](const FollowerEnemy& fEnemy) { return fEnemy.markedForDeletion; }), fEnemies.end());
}

class BaseEnemy {
public:
    RectangleShape enemy;
    Vector2f direction;    // The current movement direction
    float speed;
    Clock directionChangeClock; // Clock to control how often direction changes
    Time directionChangeInterval;
    Clock shootClock;      // Clock to handle enemy shooting
    Time shootCooldown;

    BaseEnemy(int x, int y)
        : speed(50.f), directionChangeInterval(seconds(2.f)) {
        enemy.setSize(Vector2f(50.f, 50.f));
        enemy.setPosition(x, y);
        direction = Vector2f(0.f, 0.f); // Start with no direction
        randomizeShootCooldown(); // Initialize random shoot cooldown
    }

    // Set the texture for the enemy
    void setTexture(const sf::Texture& texture) {
        enemy.setTexture(&texture);
        enemy.setTextureRect(sf::IntRect(500, 500, 1000, 1000)); // Set texture rectangle if needed
    }

    // Handle random movement and collision with walls
    void moveRandomly(float dt, const std::vector<RectangleShape>& walls, const sf::RenderWindow& window) {
        if (directionChangeClock.getElapsedTime() > directionChangeInterval) {
            direction = Vector2f(static_cast<float>(rand() % 3 - 1), static_cast<float>(rand() % 3 - 1)); // Randomize direction (-1, 0, or 1)
            float length = sqrtf(pow(direction.x, 2) + pow(direction.y, 2));
            if (length != 0) direction /= length; // Normalize the direction vector
            directionChangeClock.restart(); // Reset the clock after changing direction
        }

        Vector2f oldPosition = enemy.getPosition(); // Store old position in case of collision
        enemy.move(direction * speed * dt); // Move the enemy based on the current direction and speed

        // Handle wall collisions
        for (auto& wall : walls) {
            FloatRect wallBounds = wall.getGlobalBounds();
            FloatRect enemyBounds = enemy.getGlobalBounds();
            if (enemyBounds.intersects(wallBounds)) {
                enemy.setPosition(oldPosition); // Reset to old position if there's a collision
                break;
            }
        }

        // Prevent the enemy from going off the screen
        preventOffScreen(window);
    }

    // Prevent enemy from going off screen
    void preventOffScreen(const sf::RenderWindow& window) {
        Vector2f pos = enemy.getPosition();
        FloatRect bounds = enemy.getGlobalBounds();
        float windowWidth = window.getSize().x;
        float windowHeight = window.getSize().y;

        // Check left boundary
        if (bounds.left < 0)
            enemy.setPosition(0, pos.y);

        // Check right boundary (subtract only the bounds.width to keep the enemy within screen)
        if (bounds.left + bounds.width > windowWidth)
            enemy.setPosition(windowWidth - bounds.width, pos.y);

        // Check top boundary
        if (bounds.top < 0)
            enemy.setPosition(pos.x, 0);

        // Check bottom boundary (subtract windowHeight correctly, not windowWidth)
        if (bounds.top + bounds.height > windowHeight)
            enemy.setPosition(pos.x, windowHeight - bounds.height);
    }


    // Handle enemy shooting bullets with random angles near the player
    void shoot(std::vector<enemyBullet>& ebullets, const Players& player, Sound& eshootSound, std::string difficulty) 
    {
        if (shootClock.getElapsedTime() >= shootCooldown) 
        {
            int burstCount = 1; // Default single bullet shot for "easy" difficulty
            Vector2f basePosition = enemy.getPosition() + Vector2f(enemy.getSize().x / 2, enemy.getSize().y / 2);

            if (difficulty == "medium") 
            {
                burstCount = 2; // Shoot two bullets for medium difficulty
            }
            else if (difficulty == "hard") 
            {
                burstCount = 3; // Shoot three bullets for hard difficulty
            }

            for (int i = 0; i < burstCount; i++) 
            {
                enemyBullet newEnemyBullet(8.f);
                newEnemyBullet.eBullet.setPosition(basePosition);

                // Calculate the shooting direction with slight offset for each bullet in burst
                Vector2f enemyShootDir = player.playerCenter - newEnemyBullet.eBullet.getPosition();
                float length = sqrtf(pow(enemyShootDir.x, 2) + pow(enemyShootDir.y, 2));

                if (i == 1) 
                {
                    enemyShootDir.x += 50.f; // Offset right for second bullet
                }
                if (i == 2) 
                {
                    enemyShootDir.x -= 50.f; // Offset left for third bullet
                }

                if (length != 0) 
                {
                    newEnemyBullet.enemyBulletcurrentVel = (enemyShootDir / length) * newEnemyBullet.enemyBulletSpeed;
                    ebullets.push_back(newEnemyBullet);
                }
            }

            eshootSound.play(); // Play enemy shoot sound
            randomizeShootCooldown(); // Randomize the next cooldown after shooting
            shootClock.restart(); // Restart the shoot clock after shooting
        }
    }

    // Randomize the shooting cooldown for each enemy
    void randomizeShootCooldown() 
    {
        int minCooldown = 1000; // Minimum cooldown of 1 second
        int maxCooldown = 2000; // Maximum cooldown of 2 seconds
        int randomCooldown = rand() % (maxCooldown - minCooldown + 1) + minCooldown;
        shootCooldown = milliseconds(randomCooldown);
    }

    // Draw the base enemy
    void draw(sf::RenderWindow& window) 
    {
        window.draw(enemy);
    }
};


class PowerUp {
public:
    enum Type {
        HealthRestore,
        SpeedBoost,
        AttackSpeedBoost,
        Shield
    };

    sf::RectangleShape shape;
    Type type;
    bool collected = false;
    bool active = false;  // Tracks if the power-up is actively modifying player stats
    sf::Clock timer;      // For time-based power-ups
    float duration = 5.0f;  // Duration in seconds for timed power-ups (set to 5 seconds here)
    sf::Text descriptionText;  // Text for describing the power-up
    sf::Clock descriptionClock;  // Clock for how long to display the text
    float descriptionDuration = 3.0f;  // Duration to display the description

    PowerUp(Type type, float x, float y, float size)
        : type(type)
    {
        shape.setSize(sf::Vector2f(size, size));
        shape.setPosition(x, y);

        switch (type) {
        case HealthRestore:
            shape.setTexture(&healthPUTexture);
            shape.setTextureRect(sf::IntRect(625, 550, 500, 500));
            break;
        case SpeedBoost:
            shape.setTexture(&speedPUTexture);
            shape.setTextureRect(sf::IntRect(700, 620, 500, 500));
            break;
        case AttackSpeedBoost:
            shape.setTexture(&attackPUTexture);
            shape.setTextureRect(sf::IntRect(700, 475, 550, 550));
            break;
        case Shield:
            shape.setTexture(&shieldPUTexture);
            shape.setTextureRect(sf::IntRect(650, 600, 600, 600));
            break;
        }

        // Set up description text
        descriptionText.setFont(font);  
        descriptionText.setCharacterSize(36);
        descriptionText.setFillColor(sf::Color::White);
        /*descriptionText.setPosition(50.f, 650.f);  */
    }

    // Apply the power-up effect to the player
    void apply(Players& player) {
        if (!active) {  // Ensure the power-up is applied only once
            switch (type) {
            case HealthRestore:
                if (player.health < 6)
                {
                    player.health += 2;
                }
                descriptionText.setString("Health Restored!");
                break;
            case SpeedBoost:
                player.movementSpeed += 50;  
                timer.restart();       
                descriptionText.setString("Speed Boost Activated!");
                break;
            case AttackSpeedBoost:
                shootCooldown = sf::milliseconds(shootCooldown.asMilliseconds() / 2);  
                timer.restart();
                descriptionText.setString("Attack Speed Boost!");
                break;
            case Shield:
                player.shieldActive = true;  
                descriptionText.setString("Shield Activated!");
                break;
            }
            collected = true;  
            active = true;     

            //Start description clock
            descriptionClock.restart();

            // Position the description text above the power-up
            float textX = shape.getPosition().x;
            float textY = shape.getPosition().y - 30;  // 30 pixels above the power-up

            // Prevent text from going off the left side of the screen
            if (textX < 0) {
                textX = 10;  // Add a small padding from the edge
            }

            // Prevent text from going off the right side of the screen
            if (textX + descriptionText.getLocalBounds().width > 720) {  
                textX = 720 - descriptionText.getLocalBounds().width - 10;  // Add a small padding from the edge
            }

            // Prevent text from going off the top of the screen
            if (textY < 0) {
                textY = 10;  // Add a small padding from the top edge
            }

            descriptionText.setPosition(textX, textY);
        }
    }

    //Update for power ups with timers
    void update(Players& player) 
    {
        if (collected && active) 
        {
            if (type == SpeedBoost && timer.getElapsedTime().asSeconds() > 5) 
            {
                player.movementSpeed -= 50;
                active = false;
            }

            if (type == AttackSpeedBoost && timer.getElapsedTime().asSeconds() > 5) 
            {
                shootCooldown = sf::milliseconds(shootCooldown.asMilliseconds() * 2);
                active = false;
            }
        }
    }

    void drawDescription(sf::RenderWindow& window)
    {
        // Only draw the description if the timer hasn't expired
        if (descriptionClock.getElapsedTime().asSeconds() <= descriptionDuration) 
        {
            window.draw(descriptionText);
        }
    }
};

std::vector<PowerUp> powerUps;

void handlePowerUps(Players& player, float dt, sf::RenderWindow& window)
{
    for (auto& powerUp : powerUps)
    {
        // If player collects the power-up, apply its effect
        if (!powerUp.collected && player.rectangle.getGlobalBounds().intersects(powerUp.shape.getGlobalBounds()))
        {
            powerUp.apply(player);  // Apply the power-up effect
        }

        // Update the power-up's state (timed boosts)
        powerUp.update(player);

        // Draw the power-up if not yet collected
        if (!powerUp.collected)
        {
            window.draw(powerUp.shape);
        }

        // Draw the description if the power-up is collected and description is active
        if (powerUp.collected && powerUp.descriptionClock.getElapsedTime().asSeconds() <= powerUp.descriptionDuration)
        {
            window.draw(powerUp.descriptionText);
        }
    }

    // Remove collected power-ups (timed ones except the shield)
    powerUps.erase(std::remove_if(powerUps.begin(), powerUps.end(),
        [](const PowerUp& p) { return p.collected && !p.active && p.type != PowerUp::Shield; }), powerUps.end());
}



void displayHealth(sf::RenderWindow& window, const Players& player)
{
    sf::Sprite heart1, heart2, heart3;
    heart3.scale(.05f, .05f);
    heart2.scale(.05f, .05f);
    heart1.scale(.05f, .05f);

    // First Heart
    if (player.health >= 2) {
        heart1.setTexture(fullHeartTexture);
    }
    else if (player.health == 1) {
        heart1.setTexture(halfHeartTexture);
    }
    else {
        heart1.setTexture(emptyHeartTexture);
    }
    heart1.setPosition(-10.f, -10.f);
    window.draw(heart1);

    // Second Heart
    if (player.health >= 4) {
        heart2.setTexture(fullHeartTexture);
    }
    else if (player.health == 3) {
        heart2.setTexture(halfHeartTexture);
    }
    else {
        heart2.setTexture(emptyHeartTexture);
    }
    heart2.setPosition(30.f, -10.f);
    window.draw(heart2);

    // Third Heart
    if (player.health == 6) {
        heart3.setTexture(fullHeartTexture);
    }
    else if (player.health == 5) {
        heart3.setTexture(halfHeartTexture);
    }
    else {
        heart3.setTexture(emptyHeartTexture);
    }
    heart3.setPosition(70.f, -10.f);
    window.draw(heart3);

    // Display the shield icon if the player's shield is active
    if (player.shieldActive) {
        shieldSprite.setTexture(shieldTexture);
        shieldSprite.setPosition(110.f, -12.f);   // Adjust position to fit next to the hearts
        shieldSprite.setScale(0.055f, 0.055f);     // Match the size of the hearts
        window.draw(shieldSprite);               // Draw the shield icon
    }
}


// Function to set up the start screen (title, buttons, etc.)
void setupStartScreen()
{
    // Load the font
    if (!font.loadFromFile("C:/Users/whatu/Desktop/C++ Project/Hopper's Great Rescue/Hopper's Great Rescue/Media/Silver.ttf"))
    {
        std::cout << "Failed to load font\n";
    }

    // Setup Title text
    titleText.setFont(font);
    titleText.setString("Hopper's Great Rescue");
    titleText.setCharacterSize(100);
    titleText.setFillColor(sf::Color::Black);
    titleText.setPosition(50, 1);

    // Setup Start text
    startText.setFont(font);
    startText.setString("Start");
    startText.setCharacterSize(60);
    startText.setPosition(45, 250);

    // Setup Game text
    gameText.setFont(font);
    gameText.setString("Game");
    gameText.setCharacterSize(60);
    gameText.setPosition(50, 300);

    // Setup Quit text
    quitText.setFont(font);
    quitText.setString("Quit");
    quitText.setCharacterSize(60);
    quitText.setPosition(610, 250);

    // Setup Quit Game text
    qGameText.setFont(font);
    qGameText.setString("Game");
    qGameText.setCharacterSize(60);
    qGameText.setPosition(610, 300);

    // Setup Start button
    startButton.setSize(sf::Vector2f(100, 100));
    startButton.setOutlineColor(sf::Color{ 50, 50, 50, 255 });
    startButton.setOutlineThickness(3);
    startButton.setPosition(40, 275);
    startButton.setFillColor(sf::Color{ 19, 99, 3, 255 });

    // Setup Quit button
    quitButton.setSize(sf::Vector2f(100, 100));
    quitButton.setOutlineColor(sf::Color{ 50, 50, 50, 255 });
    quitButton.setOutlineThickness(3);
    quitButton.setPosition(600, 275);
    quitButton.setFillColor(sf::Color{ 255, 114, 118, 255 });

    // Setup easy button
    easy.setSize(sf::Vector2f(100, 40));
    easy.setOutlineColor(sf::Color{ 50, 50, 50, 255 });
    easy.setOutlineThickness(3);
    easy.setPosition(100, 650);
    easy.setFillColor(sf::Color{ 255, 255, 153, 255 });

    // Setup easy text
    easyText.setFont(font);
    easyText.setFillColor(Color::Black);
    easyText.setString("Easy");
    easyText.setCharacterSize(40);
    easyText.setPosition(130, 640);

    // Setup medium button
    medium.setSize(sf::Vector2f(100, 40));
    medium.setOutlineColor(sf::Color{ 50, 50, 50, 255 });
    medium.setOutlineThickness(3);
    medium.setPosition(300, 650);
    medium.setFillColor(sf::Color{ 255, 179, 71, 255 });

    // Setup medium text
    mediumText.setFont(font);
    mediumText.setFillColor(Color::Black);
    mediumText.setString("Medium");
    mediumText.setCharacterSize(40);
    mediumText.setPosition(320, 640);

    // Setup hard button
    hard.setSize(sf::Vector2f(100, 40));
    hard.setOutlineColor(sf::Color{ 50, 50, 50, 255 });
    hard.setOutlineThickness(3);
    hard.setPosition(500, 650);
    hard.setFillColor(sf::Color{ 204, 85, 0, 255 });

    // Setup hard text
    hardText.setFont(font);
    hardText.setFillColor(Color::Black);
    hardText.setString("Hard");
    hardText.setCharacterSize(40);
    hardText.setPosition(530, 640);
}

// Function to set up the win screen
void setupWinScreen()
{
    //Setup "You" Text
    youText.setFont(font);
    youText.setString("You");
    youText.setCharacterSize(150);
    youText.setFillColor(sf::Color::Black);
    youText.setPosition(20, 100);

    //Setup "Win!" Text
    winText.setFont(font);
    winText.setString("Win!");
    winText.setCharacterSize(150);
    winText.setFillColor(sf::Color::Black);
    winText.setPosition(550, 100);

    // Setup Play Again text
    playText.setFont(font);
    playText.setString("Play");
    playText.setCharacterSize(60);
    playText.setPosition(45, 250);

    // Setup Again text
    againText.setFont(font);
    againText.setString("Again");
    againText.setCharacterSize(60);
    againText.setPosition(45, 300);
}

// Function to set up the death screen
void setupDeathScreen()
{
    //Setup Game Over text
    deathText.setFont(font);
    deathText.setString("Game Over");
    deathText.setCharacterSize(200);
    deathText.setFillColor(sf::Color{ 191, 28, 45, 255 });
    deathText.setPosition(125, -50);

    // Setup Play Again text for game over screen
    playGOText.setFont(font);
    playGOText.setString("Play");
    playGOText.setCharacterSize(60);
    playGOText.setPosition(45, 250);

    againGOText.setFont(font);
    againGOText.setString("Again");
    againGOText.setCharacterSize(60);
    againGOText.setPosition(45, 300);
}

// Function to set up level one
void setupLevelOne(std::vector<RectangleShape>& walls, Players& players, std::vector<Bullet>& bullets, std::vector<enemyBullet>& ebullets, 
    sf::Texture& wallTexture, sf::Texture& enemyTexture, std::vector<FollowerEnemy>& fEnemies, std::vector<BaseEnemy>& baseEnemies)
{
    // Clear existing enemies, bullets, and walls
    baseEnemies.clear();
    bullets.clear();
    ebullets.clear();
    walls.clear();
    fEnemies.clear();
    powerUps.clear();

    int firstEnemyPosX;
    int firstEnemyPosY;
    int secondEnemyPosX;
    int secondEnemyPosY;
    int firstFollowerPosX;
    int firstFollowerPosY;
    int areaSelect;
    int healthX;
    int healthY;
    int speedX;
    int speedY;
    int attackX;
    int attackY;
    int shieldX;
    int shieldY;
    int puSelect;

    std::random_device enemy1Seed;
    std::mt19937 E1(enemy1Seed());
    std::uniform_int_distribution<int> enemy1genX(1, 670);
    std::uniform_int_distribution<int> enemy1genY(570, 670);
    std::uniform_int_distribution<int> enemy2genX(570, 670);
    std::uniform_int_distribution<int> enemy2genY(1, 670);
    std::uniform_int_distribution<int> healthgenX(30, 690);
    std::uniform_int_distribution<int> healthgenY(30, 690);
    std::uniform_int_distribution<int> speedgenX(30, 690);
    std::uniform_int_distribution<int> speedgenY(30, 690);
    std::uniform_int_distribution<int> attackgenX(30, 690);
    std::uniform_int_distribution<int> attackgenY(30, 690);
    std::uniform_int_distribution<int> shieldgenX(30, 690);
    std::uniform_int_distribution<int> shieldgenY(30, 690);
    firstEnemyPosX = enemy1genX(E1);
    firstEnemyPosY = enemy1genY(E1);
    secondEnemyPosX = enemy2genX(E1);
    secondEnemyPosY = enemy2genY(E1);
    healthX = healthgenX(E1);
    healthY = healthgenY(E1);
    speedX = speedgenX(E1);
    speedY = speedgenY(E1);
    attackX = attackgenX(E1);
    attackY = attackgenY(E1);
    shieldX = shieldgenX(E1);
    shieldY = shieldgenY(E1);

    areaSelect = rand() % 2 + 1;
    if (areaSelect == 1)
    {
        std::random_device enemy1Seed;
        std::mt19937 E1(enemy1Seed());
        std::uniform_int_distribution<int> follower1genX(570, 670);
        std::uniform_int_distribution<int> follower1genY(1, 670);
        firstFollowerPosX = follower1genX(E1);
        firstFollowerPosY = follower1genY(E1);
    }
    else if (areaSelect == 2)
    {
        std::random_device enemy1Seed;
        std::mt19937 E1(enemy1Seed());
        std::uniform_int_distribution<int> follower1genX(1, 670);
        std::uniform_int_distribution<int> follower1genY(570, 670);
        firstFollowerPosX = follower1genX(E1);
        firstFollowerPosY = follower1genY(E1);
    }

    BaseEnemy enemy(firstEnemyPosX, firstEnemyPosY);
    baseEnemies.push_back(enemy);
    BaseEnemy enemy2(secondEnemyPosX, secondEnemyPosY);
    baseEnemies.push_back(enemy2);


    FollowerEnemy follower(firstFollowerPosX, firstFollowerPosY);
    fEnemies.push_back(follower);

    puSelect = rand() % 4 + 1;
    if (puSelect == 1)
    {
        powerUps.push_back(PowerUp(PowerUp::HealthRestore, healthX, healthY, 30.f));
    }
    if (puSelect == 2)
    {
        powerUps.push_back(PowerUp(PowerUp::SpeedBoost, speedX, speedY, 30.f));
    }
    if (puSelect == 3)
    {
        powerUps.push_back(PowerUp(PowerUp::Shield, shieldX, shieldY, 30.f));
    }
    if (puSelect == 4)
    {
        powerUps.push_back(PowerUp(PowerUp::AttackSpeedBoost, attackX, attackY, 30.f));
    }

    // Initialize walls
    RectangleShape wall;
    wall.setTexture(&wallTexture);
    wall.setTextureRect(sf::IntRect(200, 350, 1500, 1350));
    wall.setSize(Vector2f(50, 50));

    // Add walls to the level
    wall.setPosition(100, 100);
    walls.push_back(wall);
    wall.setPosition(Vector2f(150, 50));
    walls.push_back(wall);
    wall.setPosition(150, 100);
    walls.push_back(wall);

    // Set player starting position
    players.rectangle.setPosition(50, 50);

}

void setupLevelTwo(std::vector<RectangleShape>& walls, Players& players,std::vector<Bullet>& bullets, std::vector<enemyBullet>& ebullets, 
    sf::Texture& wallTexture, sf::Texture& enemyTexture, std::vector<FollowerEnemy>& fEnemies, std::vector<BaseEnemy>& baseEnemies)
{
    // Clear existing enemies, bullets, and walls
    baseEnemies.clear();
    bullets.clear();
    ebullets.clear();
    walls.clear();
    fEnemies.clear();
    powerUps.clear();

    int firstEnemyPosX;
    int firstEnemyPosY;
    int secondEnemyPosX;
    int secondEnemyPosY;
    int thirdEnemyPosX;
    int thirdEnemyPosY;
    int firstFollowerPosX;
    int firstFollowerPosY;
    int secondFollowerPosX;
    int secondFollowerPosY;
    int areaSelect;
    int healthX;
    int healthY;
    int speedX;
    int speedY;
    int attackX;
    int attackY;
    int shieldX;
    int shieldY;
    int puSelect;

    std::random_device enemy1Seed;
    std::mt19937 E1(enemy1Seed());
    std::uniform_int_distribution<int> enemy1genX(1, 670);
    std::uniform_int_distribution<int> enemy1genY(470, 670);
    std::uniform_int_distribution<int> enemy2genX(470, 670);
    std::uniform_int_distribution<int> enemy2genY(1, 670);
    std::uniform_int_distribution<int> follower1genX(1, 670);
    std::uniform_int_distribution<int> follower1genY(470, 670);
    std::uniform_int_distribution<int> follower2genX(470, 670);
    std::uniform_int_distribution<int> follower2genY(1, 670);
    std::uniform_int_distribution<int> healthgenX(30, 690);
    std::uniform_int_distribution<int> healthgenY(30, 690);
    std::uniform_int_distribution<int> speedgenX(30, 690);
    std::uniform_int_distribution<int> speedgenY(30, 690);
    std::uniform_int_distribution<int> attackgenX(30, 690);
    std::uniform_int_distribution<int> attackgenY(30, 690);
    std::uniform_int_distribution<int> shieldgenX(30, 690);
    std::uniform_int_distribution<int> shieldgenY(30, 690);
    firstEnemyPosX = enemy1genX(E1);
    firstEnemyPosY = enemy1genY(E1);
    secondEnemyPosX = enemy2genX(E1);
    secondEnemyPosY = enemy2genY(E1);
    firstFollowerPosX = follower1genX(E1);
    firstFollowerPosY = follower1genY(E1);
    secondFollowerPosX = follower2genX(E1);
    secondFollowerPosY = follower2genY(E1);
    healthX = healthgenX(E1);
    healthY = healthgenY(E1);
    speedX = speedgenX(E1);
    speedY = speedgenY(E1);
    attackX = attackgenX(E1);
    attackY = attackgenY(E1);
    shieldX = shieldgenX(E1);
    shieldY = shieldgenY(E1);

    areaSelect = rand() % 2 + 1;
    if (areaSelect == 1)
    {
        std::random_device enemy3Seed;
        std::mt19937 E3(enemy3Seed());
        std::uniform_int_distribution<int> enemy3genX(470, 670);
        std::uniform_int_distribution<int> enemy3genY(1, 670);
        thirdEnemyPosX = enemy3genX(E3);
        thirdEnemyPosY = enemy3genY(E3);
    }
    else if (areaSelect == 2)
    {
        std::random_device enemy3Seed;
        std::mt19937 E3(enemy3Seed());
        std::uniform_int_distribution<int> enemy3genX(1, 670);
        std::uniform_int_distribution<int> enemy3genY(470, 670);
        thirdEnemyPosX = enemy3genX(E3);
        thirdEnemyPosY = enemy3genY(E3);
    }


    BaseEnemy enemy(firstEnemyPosX, firstEnemyPosY);
    baseEnemies.push_back(enemy);
    BaseEnemy enemy2(secondEnemyPosX, secondEnemyPosY);
    baseEnemies.push_back(enemy2);
    BaseEnemy enemy3(thirdEnemyPosX, thirdEnemyPosY);
    baseEnemies.push_back(enemy3);

    FollowerEnemy follower(firstFollowerPosX, firstFollowerPosY);
    fEnemies.push_back(follower);
    FollowerEnemy follower2(secondFollowerPosX, secondFollowerPosY);
    fEnemies.push_back(follower2);

    // Initialize walls
    RectangleShape wall;
    wall.setTexture(&wallTexture);
    wall.setTextureRect(sf::IntRect(250, 250, 1500, 1500));
    wall.setSize(Vector2f(50, 50));

    puSelect = rand() % 4 + 1;
    if (puSelect == 1)
    {
        powerUps.push_back(PowerUp(PowerUp::HealthRestore, healthX, healthY, 30.f));
    }
    if (puSelect == 2)
    {
        powerUps.push_back(PowerUp(PowerUp::SpeedBoost, speedX, speedY, 30.f));
    }
    if (puSelect == 3)
    {
        powerUps.push_back(PowerUp(PowerUp::Shield, shieldX, shieldY, 30.f));
    }
    if (puSelect == 4)
    {
        powerUps.push_back(PowerUp(PowerUp::AttackSpeedBoost, attackX, attackY, 30.f));
    }


    // Add walls to the level
    wall.setPosition(100, 100);
    walls.push_back(wall);
    wall.setPosition(Vector2f(150, 50));
    walls.push_back(wall);
    wall.setPosition(150, 100);
    walls.push_back(wall);

    // Set player starting position
    players.rectangle.setPosition(50, 50);

}

void setupLevelThree(std::vector<RectangleShape>& walls, Players& players, std::vector<Bullet>& bullets, std::vector<enemyBullet>& ebullets, 
    sf::Texture& wallTexture, sf::Texture& enemyTexture, std::vector<FollowerEnemy>& fEnemies, std::vector<BaseEnemy>& baseEnemies)
{
    // Clear existing enemies, bullets, and walls
    baseEnemies.clear();
    bullets.clear();
    ebullets.clear();
    walls.clear();
    fEnemies.clear();
    powerUps.clear();

    int firstEnemyPosX;
    int firstEnemyPosY;
    int secondEnemyPosX;
    int secondEnemyPosY;
    int thirdEnemyPosX;
    int thirdEnemyPosY;
    int fourthEnemyPosX;
    int fourthEnemyPosY;
    int firstFollowerPosX;
    int firstFollowerPosY;
    int secondFollowerPosX;
    int secondFollowerPosY;
    int thirdFollowerPosX;
    int thirdFollowerPosY;
    int healthX;
    int healthY;
    int speedX;
    int speedY;
    int attackX;
    int attackY;
    int shieldX;
    int shieldY;
    int puSelect;

    std::random_device enemy1Seed;
    std::mt19937 E1(enemy1Seed());
    std::uniform_int_distribution<int> enemy1genX(1, 670);
    std::uniform_int_distribution<int> enemy1genY(370, 670);
    std::uniform_int_distribution<int> enemy2genX(370, 670);
    std::uniform_int_distribution<int> enemy2genY(1, 670);
    std::uniform_int_distribution<int> enemy3genX(370, 670);
    std::uniform_int_distribution<int> enemy3genY(1, 670);
    std::uniform_int_distribution<int> enemy4genX(1, 670);
    std::uniform_int_distribution<int> enemy4genY(370, 670);
    std::uniform_int_distribution<int> enemy5genX(1, 670);
    std::uniform_int_distribution<int> enemy5genY(370, 670);
    std::uniform_int_distribution<int> enemy6genX(1, 670);
    std::uniform_int_distribution<int> enemy6genY(370, 670);
    std::uniform_int_distribution<int> enemy7genX(1, 670);
    std::uniform_int_distribution<int> enemy7genY(370, 670);
    std::uniform_int_distribution<int> healthgenX(30, 690);
    std::uniform_int_distribution<int> healthgenY(30, 690);
    std::uniform_int_distribution<int> speedgenX(30, 690);
    std::uniform_int_distribution<int> speedgenY(30, 690);
    std::uniform_int_distribution<int> attackgenX(30, 690);
    std::uniform_int_distribution<int> attackgenY(30, 690);
    std::uniform_int_distribution<int> shieldgenX(30, 690);
    std::uniform_int_distribution<int> shieldgenY(30, 690);
    firstEnemyPosX = enemy1genX(E1);
    firstEnemyPosY = enemy1genY(E1);
    secondEnemyPosX = enemy2genX(E1);
    secondEnemyPosY = enemy2genY(E1);
    thirdEnemyPosX = enemy3genX(E1);
    thirdEnemyPosY = enemy3genY(E1);
    fourthEnemyPosX = enemy4genX(E1);
    fourthEnemyPosY = enemy4genY(E1);
    firstFollowerPosX = enemy5genY(E1);
    firstFollowerPosY = enemy5genY(E1);
    secondFollowerPosX = enemy6genY(E1);
    secondFollowerPosY = enemy6genY(E1);
    thirdFollowerPosX = enemy7genY(E1);
    thirdFollowerPosY = enemy7genY(E1);
    healthX = healthgenX(E1);
    healthY = healthgenY(E1);
    speedX = speedgenX(E1);
    speedY = speedgenY(E1);
    attackX = attackgenX(E1);
    attackY = attackgenY(E1);
    shieldX = shieldgenX(E1);
    shieldY = shieldgenY(E1);

    BaseEnemy enemy(firstEnemyPosX, firstEnemyPosY);
    baseEnemies.push_back(enemy);
    BaseEnemy enemy2(secondEnemyPosX, secondEnemyPosY);
    baseEnemies.push_back(enemy2);
    BaseEnemy enemy3(thirdEnemyPosX, thirdEnemyPosY);
    baseEnemies.push_back(enemy3);
    BaseEnemy enemy4(fourthEnemyPosX, fourthEnemyPosY);
    baseEnemies.push_back(enemy4);

    FollowerEnemy follower(firstFollowerPosX, firstFollowerPosY);
    fEnemies.push_back(follower);
    FollowerEnemy follower2(secondFollowerPosX, secondFollowerPosY);
    fEnemies.push_back(follower2);
    FollowerEnemy follower3(thirdFollowerPosX, thirdFollowerPosY);
    fEnemies.push_back(follower3);

    puSelect = rand() % 4 + 1;
    if (puSelect == 1)
    {
        powerUps.push_back(PowerUp(PowerUp::HealthRestore, healthX, healthY, 30.f));
    }
    if (puSelect == 2)
    {
        powerUps.push_back(PowerUp(PowerUp::SpeedBoost, speedX, speedY, 30.f));
    }
    if (puSelect == 3)
    {
        powerUps.push_back(PowerUp(PowerUp::Shield, shieldX, shieldY, 30.f));
    }
    if (puSelect == 4)
    {
        powerUps.push_back(PowerUp(PowerUp::AttackSpeedBoost, attackX, attackY, 30.f));
    }


    // Initialize walls
    RectangleShape wall;
    wall.setTexture(&wallTexture);
    wall.setTextureRect(sf::IntRect(250, 250, 1500, 1500));
    wall.setSize(Vector2f(50, 50));


    // Add walls to the level
    wall.setPosition(Vector2f(100, 100));
    walls.push_back(wall);
    wall.setPosition(Vector2f(150, 50));
    walls.push_back(wall);
    wall.setPosition(150, 100);
    walls.push_back(wall);


    // Set player starting position
    players.rectangle.setPosition(50, 50);
}
// Current game state
GameState currentState = START_SCREEN;

// Main function to open the window and run the game
int main()
{
    srand(time(NULL));

    // Create a window with specific dimensions and settings
    sf::RenderWindow window(sf::VideoMode(720, 720), "Hopper's Great Rescue", sf::Style::None);
    window.setFramerateLimit(60); // Limit the frame rate to 60 FPS

    // Set up the start screen
    setupStartScreen();

    // Load textures for the game
    sf::Texture floorTexture;
    floorTexture.loadFromFile("C:/Users/whatu/Desktop/C++ Project/Hopper's Great Rescue/Hopper's Great Rescue/Media/Floor3.jpg");
    sf::Sprite floorSprite;
    floorSprite.setTexture(floorTexture);
    floorSprite.scale(0.6f, 0.6f);

    sf::Texture wallTexture;
    wallTexture.loadFromFile("C:/Users/whatu/Desktop/C++ Project/Hopper's Great Rescue/Hopper's Great Rescue/Media/wall2.PNG");

    sf::Texture playerTexture;
    playerTexture.loadFromFile("C:/Users/whatu/Desktop/C++ Project/Hopper's Great Rescue/Hopper's Great Rescue/Media/Player.PNG");

    enemyTexture.loadFromFile("C:/Users/whatu/Desktop/C++ Project/Hopper's Great Rescue/Hopper's Great Rescue/Media/Enemy1.PNG");

    shieldPUTexture.loadFromFile("C:/Users/whatu/Desktop/C++ Project/Hopper's Great Rescue/Hopper's Great Rescue/Media/shieldPU.PNG");

    attackPUTexture.loadFromFile("C:/Users/whatu/Desktop/C++ Project/Hopper's Great Rescue/Hopper's Great Rescue/Media/attackPU.PNG");

    healthPUTexture.loadFromFile("C:/Users/whatu/Desktop/C++ Project/Hopper's Great Rescue/Hopper's Great Rescue/Media/healthPU.PNG");

    speedPUTexture.loadFromFile("C:/Users/whatu/Desktop/C++ Project/Hopper's Great Rescue/Hopper's Great Rescue/Media/speedPU.PNG");

    shieldTexture.loadFromFile("C:/Users/whatu/Desktop/C++ Project/Hopper's Great Rescue/Hopper's Great Rescue/Media/shield.PNG");

    enemy2Texture.loadFromFile("C:/Users/whatu/Desktop/C++ Project/Hopper's Great Rescue/Hopper's Great Rescue/Media/Enemy2.PNG");

    halfHeartTexture.loadFromFile("C:/Users/whatu/Desktop/C++ Project/Hopper's Great Rescue/Hopper's Great Rescue/Media/halfHeart.PNG");

    fullHeartTexture.loadFromFile("C:/Users/whatu/Desktop/C++ Project/Hopper's Great Rescue/Hopper's Great Rescue/Media/fullHeart.PNG");

    emptyHeartTexture.loadFromFile("C:/Users/whatu/Desktop/C++ Project/Hopper's Great Rescue/Hopper's Great Rescue/Media/emptyHeart.PNG");

    sf::Texture mmBGTexture;
    mmBGTexture.loadFromFile("C:/Users/whatu/Desktop/C++ Project/Hopper's Great Rescue/Hopper's Great Rescue/Media/Main_BG.PNG");
    sf::Sprite mmBGSprite;
    mmBGSprite.setTexture(mmBGTexture);
    mmBGSprite.scale(.4f, .4f);
    mmBGSprite.setPosition(1, 1);

    sf::Texture goBGTexture;
    goBGTexture.loadFromFile("C:/Users/whatu/Desktop/C++ Project/Hopper's Great Rescue/Hopper's Great Rescue/Media/Game_Over.PNG");
    sf::Sprite goBGSprite;
    goBGSprite.setTexture(goBGTexture);
    goBGSprite.scale(.4f, .4f);
    goBGSprite.setPosition(1, 1);

    sf::Texture wsBGTexture;
    wsBGTexture.loadFromFile("C:/Users/whatu/Desktop/C++ Project/Hopper's Great Rescue/Hopper's Great Rescue/Media/win_screen.PNG");
    sf::Sprite wsBGSprite;
    wsBGSprite.setTexture(wsBGTexture);
    wsBGSprite.scale(.4f, .4f);
    wsBGSprite.setPosition(1, 1);

    // Load and set up sound effects
    sf::SoundBuffer shootSoundBuffer;
    shootSoundBuffer.loadFromFile("C:/Users/whatu/Desktop/C++ Project/Hopper's Great Rescue/Hopper's Great Rescue/Media/Shoot.wav");
    sf::Sound shootSound;
    shootSound.setBuffer(shootSoundBuffer);

    sf::SoundBuffer enemySoundBuffer;
    enemySoundBuffer.loadFromFile("C:/Users/whatu/Desktop/C++ Project/Hopper's Great Rescue/Hopper's Great Rescue/Media/EnemyShoot.wav");
    sf::Sound eshootSound;
    eshootSound.setBuffer(enemySoundBuffer);

    sf::SoundBuffer winSoundBuffer;
    winSoundBuffer.loadFromFile("C:/Users/whatu/Desktop/C++ Project/Hopper's Great Rescue/Hopper's Great Rescue/Media/PlayerWin.wav");
    sf::Sound winSound;
    winSound.setBuffer(winSoundBuffer);

    explosionBuffer.loadFromFile("C:/Users/whatu/Desktop/C++ Project/Hopper's Great Rescue/Hopper's Great Rescue/Media/explosion.mp3");
    explosionSound.setBuffer(explosionBuffer);

    sf::SoundBuffer loseSoundBuffer;
    loseSoundBuffer.loadFromFile("C:/Users/whatu/Desktop/C++ Project/Hopper's Great Rescue/Hopper's Great Rescue/Media/PlayerLose.wav");
    sf::Sound loseSound;
    loseSound.setBuffer(loseSoundBuffer);

    sf::SoundBuffer musicSoundBuffer;
    musicSoundBuffer.loadFromFile("C:/Users/whatu/Desktop/C++ Project/Hopper's Great Rescue/Hopper's Great Rescue/Media/Music.mp3");
    sf::Sound music;
    music.setBuffer(musicSoundBuffer);
    music.setLoop(true);


    music.play(); // Start background music

    // Create the walls for the level
    std::vector<RectangleShape> walls;

    // Create a player object
    Players player(50, 50, playerTexture);

    // Variables to manage player and enemy bullets
    std::vector<Bullet> bullets;
    std::vector<enemyBullet> ebullets;
    std::vector<FollowerEnemy> fEnemies;
    std::vector<BaseEnemy> baseEnemies;

    bool difficultymedium = false;
    bool difficultyeasy = true;
    bool difficultyhard = false;

    // Main game loop
    while (window.isOpen())
    {
        Vector2f mousePosWindow;
        Vector2f aimDir;
        Vector2f aimDirNorm;

        // Update delta time for smooth movement
        dt = dt_clock.restart().asSeconds();

        // Calculate bullet aim direction based on mouse position
        mousePosWindow = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));
        aimDir = mousePosWindow - player.playerCenter;
        aimDirNorm = aimDir / (sqrtf(pow(aimDir.x, 2) + pow(aimDir.y, 2)));

        // Handle window events
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed)
                player.processEvents(event.key.code, true);
            if (event.type == sf::Event::KeyReleased)
                player.processEvents(event.key.code, false);
            if (event.type == sf::Event::MouseButtonPressed)
            {
                // Only allow input if enough time has passed since the last state change
                if (inputDelayClock.getElapsedTime().asSeconds() > 0.5f)
                {
                    // Check if start button is clicked
                    if (startButton.getGlobalBounds().contains(mousePosWindow.x, mousePosWindow.y) && currentState == START_SCREEN)
                    {
                        setupLevelOne(walls, player, bullets, ebullets, wallTexture, enemyTexture, fEnemies, baseEnemies);
                        currentState = LEVEL_ONE;
                        inputDelayClock.restart(); // Restart the delay clock
                    }
                    else if (quitButton.getGlobalBounds().contains(mousePosWindow.x, mousePosWindow.y) && currentState == START_SCREEN)
                    {
                        window.close();
                    }
                    else if (medium.getGlobalBounds().contains(mousePosWindow.x, mousePosWindow.y) && currentState == START_SCREEN)
                    {
                        std::cout << "medium";
                        difficultymedium = true;
                        difficultyeasy = false;
                        difficultyhard = false;
                    }
                    else if (easy.getGlobalBounds().contains(mousePosWindow.x, mousePosWindow.y) && currentState == START_SCREEN)
                    {
                        std::cout << "easy";
                        difficultyeasy = true;
                        difficultymedium = false;
                        difficultyhard = false;
                    }
                    else if (hard.getGlobalBounds().contains(mousePosWindow.x, mousePosWindow.y) && currentState == START_SCREEN)
                    {
                        std::cout << "hard";
                        difficultyhard = true;
                        difficultyeasy = false;
                        difficultymedium = false;
                    }
                }
            }
        }

        for (auto& wall : walls) {
            FloatRect wallBounds = wall.getGlobalBounds();

            // 1. Create the player's next position based on their velocity
            sf::FloatRect nextPosition = player.playerbounds;
            nextPosition.left += player.velocity.x;
            nextPosition.top += player.velocity.y;

            // 2. Check for intersection with the wall
            if (wallBounds.intersects(nextPosition)) {
                // 3. Calculate the overlap on both axes
                float horizontalOverlap;
                if (player.velocity.x > 0) { // Moving right
                    horizontalOverlap = (player.playerbounds.left + player.playerbounds.width - wallBounds.left);
                }
                else { // Moving left
                    horizontalOverlap = (wallBounds.left + wallBounds.width - player.playerbounds.left);
                }

                float verticalOverlap;
                if (player.velocity.y > 0) { // Moving down
                    verticalOverlap = (player.playerbounds.top + player.playerbounds.height - wallBounds.top);
                }
                else { // Moving up
                    verticalOverlap = (wallBounds.top + wallBounds.height - player.playerbounds.top);
                }

                // 4. Resolve the smallest overlap (either horizontal or vertical)
                if (horizontalOverlap < verticalOverlap) {
                    // Resolve horizontal collision
                    if (player.velocity.x > 0) { // Moving right
                        player.velocity.x = 0.f;
                        player.rectangle.setPosition(wallBounds.left - player.playerbounds.width, player.rectangle.getPosition().y);
                    }
                    else if (player.velocity.x < 0) { // Moving left
                        player.velocity.x = 0.f;
                        player.rectangle.setPosition(wallBounds.left + wallBounds.width, player.rectangle.getPosition().y);
                    }
                }
                else {
                    // Resolve vertical collision
                    if (player.velocity.y > 0) { // Moving down
                        player.velocity.y = 0.f;
                        player.rectangle.setPosition(player.rectangle.getPosition().x, wallBounds.top - player.playerbounds.height);
                    }
                    else if (player.velocity.y < 0) { // Moving up
                        player.velocity.y = 0.f;
                        player.rectangle.setPosition(player.rectangle.getPosition().x, wallBounds.top + wallBounds.height);
                    }
                }

                // Update the player's bounding box after resolving collision
                player.playerbounds = player.rectangle.getGlobalBounds();
            }
        }

        // Handle player shooting bullets
        if (Mouse::isButtonPressed(Mouse::Left) && bulletClock.getElapsedTime() >= shootCooldown)
        {

            Bullet b1;
            b1.bullet.setPosition(player.playerCenter);
            b1.currentVelocity = aimDirNorm * b1.speed;
            bullets.push_back(b1);
            shootSound.play(); // Play shoot sound
            bulletClock.restart();
        }

        // Move player bullets
        for (size_t i = 0; i < bullets.size(); i++) {
            bullets[i].bullet.move(bullets[i].currentVelocity);
        }

        // Move enemy bullets
        for (int i = 0; i < ebullets.size(); ++i)
        {
            ebullets[i].eBullet.move(ebullets[i].enemyBulletcurrentVel);

            // Check if player is hit by an enemy bullet
            if (ebullets[i].eBullet.getGlobalBounds().intersects(player.playerbounds))
            {
                player.takeDamage(1);  // Use the takeDamage function that checks the shield

                ebullets.erase(ebullets.begin() + i);  // Remove the bullet after collision
                break;
            }

            // Check if enemy bullet hits a wall
            for (auto& wall : walls)
            {
                if (ebullets[i].eBullet.getGlobalBounds().intersects(wall.getGlobalBounds()))
                {
                    ebullets.erase(ebullets.begin() + i);
                    break;
                }
            }
        }

        // Handle player bullet collisions
        bool bulletRemoved = false;
        for (size_t i = 0; i < bullets.size(); i++)
        {
            // Remove bullets if they are out of window bounds
            if (bullets[i].bullet.getPosition().x < 0 || bullets[i].bullet.getPosition().x > window.getSize().x
                || bullets[i].bullet.getPosition().y < 0 || bullets[i].bullet.getPosition().y > window.getSize().y)
            {
                bullets.erase(bullets.begin() + i);
                bulletRemoved = true;
            }
            else
            {
                // Check if player bullet hits a wall
                for (auto& wall : walls)
                {
                    if (bullets[i].bullet.getGlobalBounds().intersects(wall.getGlobalBounds()))
                    {
                        bullets.erase(bullets.begin() + i);
                        bulletRemoved = true;
                        break;
                    }
                }
            }

            if (bulletRemoved)
                continue;

            // Check if player bullet hits an enemy
            for (size_t k = 0; k < baseEnemies.size(); k++)
            {
                if (bullets[i].bullet.getGlobalBounds().intersects(baseEnemies[k].enemy.getGlobalBounds()))
                {
                    bullets.erase(bullets.begin() + i);
                    baseEnemies.erase(baseEnemies.begin() + k);
                    bulletRemoved = true;
                    break;
                }
            }

            if (bulletRemoved)
                continue;

            // Check if player bullet hits a follower enemy
            for (size_t j = 0; j < fEnemies.size(); j++)
            {
                if (bullets[i].bullet.getGlobalBounds().intersects(fEnemies[j].fEnemy.getGlobalBounds()))
                {
                    bullets.erase(bullets.begin() + i);
                    fEnemies.erase(fEnemies.begin() + j); // You can add health reduction logic here if followers have health.
                    break;
                }
            }
        }

        // Render the game
        window.clear();

        // Draw the start screen
        if (currentState == START_SCREEN) {
            window.draw(mmBGSprite);
            window.draw(titleText);
            window.draw(startButton);
            window.draw(quitButton);
            window.draw(startText);
            window.draw(gameText);
            window.draw(quitText);
            window.draw(qGameText);
            window.draw(easy);
            window.draw(medium);
            window.draw(hard);
            window.draw(easyText);
            window.draw(mediumText);
            window.draw(hardText);

            player.health = 6;
        }

        // Draw the game for level one
        else if (currentState == LEVEL_ONE)
        {

            window.draw(floorSprite);

            // Update each enemy's movement, shooting, and drawing
            for (auto& enemy : baseEnemies)
            {
                if (difficultyeasy) {
                    enemy.shoot(ebullets, player, eshootSound, "easy");
                }
                else if (difficultymedium) {
                    enemy.shoot(ebullets, player, eshootSound, "medium");
                }
                else if (difficultyhard) {
                    enemy.shoot(ebullets, player, eshootSound, "hard");
                }
                enemy.moveRandomly(dt, walls, window);           // Random slow movement
                enemy.draw(window);               // Draw the enemy to the screen
                enemy.setTexture(enemyTexture);
            }

            for (auto& follower : fEnemies)
            {
                follower.setTexture(enemy2Texture);  // Set texture once
                follower.explode(fEnemies, player, window);
                follower.draw(window);  // Draw the follower enemy
            }


            handlePowerUps(player, dt, window);
            // Render power-ups
              // Render power-ups
            for (auto& powerUp : powerUps) {
                if (!powerUp.collected) {
                    window.draw(powerUp.shape);  // Directly draw the shape of each power-up
                }
            }

            handleFollowers(fEnemies, player, walls, dt, window);  // Follow player and handle collisions

            // Draw the player
            player.drawTo(window);

            //Draws HUD
            displayHealth(window, player);

            // Draw player bullets
            for (size_t i = 0; i < bullets.size(); i++)
            {
                window.draw(bullets[i].bullet);
            }

            // Draw enemy bullets
            for (auto& eBullet : ebullets) {
                window.draw(eBullet.eBullet);
            }

            // Draw walls
            for (auto& i : walls)
            {
                window.draw(i);
            }

            // Update player position
            player.update(dt);

            // Handle player collision with screen edges
            player.Collide(window);

            if (player.health <= 0)
            {
                loseSound.play(); // Play lose sound
                setupDeathScreen();
                currentState = DEATH_SCREEN;
            }

            // If all enemies are defeated, go to Level 2
            if (fEnemies.empty() &&baseEnemies.empty())
            {
                ebullets.clear();
                setupLevelTwo(walls, player, bullets, ebullets, wallTexture, enemyTexture, fEnemies, baseEnemies);
                currentState = LEVEL_TWO;
            }
        }
        else if (currentState == LEVEL_TWO)
        {

            window.draw(floorSprite);

            // Update each enemy's movement, shooting, and drawing
            for (auto& enemy : baseEnemies)
            {
                if (difficultyeasy) {
                    enemy.shoot(ebullets, player, eshootSound, "easy");
                }
                else if (difficultymedium) {
                    enemy.shoot(ebullets, player, eshootSound, "medium");
                }
                else if (difficultyhard) {
                    enemy.shoot(ebullets, player, eshootSound, "hard");
                }
                enemy.moveRandomly(dt, walls, window);           // Random slow movement
                enemy.draw(window);               // Draw the enemy to the screen
                enemy.setTexture(enemyTexture);
            }

            // Draw the player
            player.drawTo(window);

            //Draws HUD
            displayHealth(window, player);

            for (auto& follower : fEnemies)
            {
                follower.setTexture(enemy2Texture);  // Set texture once
                follower.explode(fEnemies, player, window);
                follower.draw(window);  // Draw the follower enemy
            }

            handlePowerUps(player, dt, window);
            // Render power-ups
              // Render power-ups
            for (auto& powerUp : powerUps) {
                if (!powerUp.collected) {
                    window.draw(powerUp.shape);  // Directly draw the shape of each power-up
                }
            }

            handleFollowers(fEnemies, player, walls, dt, window);  // Follow player and handle collisions

            // Draw player bullets
            for (size_t i = 0; i < bullets.size(); i++)
            {
                window.draw(bullets[i].bullet);
            }

            // Draw enemy bullets
            for (auto& eBullet : ebullets) {
                window.draw(eBullet.eBullet);
            }

            // Draw walls
            for (auto& i : walls)
            {
                window.draw(i);
            }

            // Update player position
            player.update(dt);

            // Handle player collision with screen edges
            player.Collide(window);

            if (player.health <= 0)
            {
                loseSound.play(); // Play lose sound
                setupDeathScreen();
                currentState = DEATH_SCREEN;
            }

            // If all enemies are defeated, go to the third level
            if (fEnemies.empty() && baseEnemies.empty())
            {
                ebullets.clear();
                setupLevelThree(walls, player, bullets, ebullets, wallTexture, enemyTexture, fEnemies, baseEnemies);
                currentState = LEVEL_THREE;
            }
        }
        else if (currentState == LEVEL_THREE)
        {

            window.draw(floorSprite);

            // Update each enemy's movement, shooting, and drawing
            for (auto& enemy : baseEnemies)
            {
                if (difficultyeasy) {
                    enemy.shoot(ebullets, player, eshootSound, "easy");
                }
                else if (difficultymedium) {
                    enemy.shoot(ebullets, player, eshootSound, "medium");
                }
                else if (difficultyhard) {
                    enemy.shoot(ebullets, player, eshootSound, "hard");
                }
                enemy.moveRandomly(dt, walls, window);           // Random slow movement
                enemy.draw(window);               // Draw the enemy to the screen
                enemy.setTexture(enemyTexture);
            }

            // Draw the player
            player.drawTo(window);

            //Draws HUD
            displayHealth(window, player);


            for (auto& follower : fEnemies)
            {
                follower.setTexture(enemy2Texture);  // Set texture once
                follower.explode(fEnemies, player, window);
                follower.draw(window);  // Draw the follower enemy
            }
 
            handlePowerUps(player, dt, window);
            // Render power-ups
              // Render power-ups
            for (auto& powerUp : powerUps) {
                if (!powerUp.collected) {
                    window.draw(powerUp.shape);  // Directly draw the shape of each power-up
                }
            }

            handleFollowers(fEnemies, player, walls, dt, window);  // Follow player and handle collisions

            // Draw player bullets
            for (size_t i = 0; i < bullets.size(); i++)
            {
                window.draw(bullets[i].bullet);
            }

            // Draw enemy bullets
            for (auto& eBullet : ebullets) {
                window.draw(eBullet.eBullet);
            }

            // Draw walls
            for (auto& i : walls)
            {
                window.draw(i);
            }

            // Update player position
            player.update(dt);

            // Handle player collision with screen edges
            player.Collide(window);

            if (player.health <= 0)
            {
                loseSound.play(); // Play lose sound
                setupDeathScreen();
                currentState = DEATH_SCREEN;
            }

            // If all enemies are defeated, go to the win screen
            if (fEnemies.empty() && baseEnemies.empty())
            {
                winSound.play(); // Play win sound
                ebullets.clear();
                setupWinScreen();
                currentState = WIN_SCREEN;
                inputDelayClock.restart(); // Restart the delay clock for the win screen
            }
        }
        // Draw the win screen
        else if (currentState == WIN_SCREEN)
        {
            baseEnemies.clear();
            bullets.clear();
            ebullets.clear();
            walls.clear();
            fEnemies.clear();
            window.draw(wsBGSprite);
            window.draw(youText);
            window.draw(winText);
            window.draw(startButton);
            window.draw(quitButton);
            window.draw(playText);
            window.draw(againText);
            window.draw(quitText);
            window.draw(qGameText);

            // Handle input for restarting or quitting after winning
            if (Mouse::isButtonPressed(Mouse::Left))
            {
                if (inputDelayClock.getElapsedTime().asSeconds() > 0.5f)
                {
                    if (startButton.getGlobalBounds().contains(mousePosWindow.x, mousePosWindow.y))
                    {
                        setupStartScreen();
                        currentState = START_SCREEN;
                        inputDelayClock.restart();
                    }
                    else if (quitButton.getGlobalBounds().contains(mousePosWindow.x, mousePosWindow.y))
                    {
                        window.close();
                    }
                }
            }
        }
        // Draw the death screen
        else if (currentState == DEATH_SCREEN)
        {
            baseEnemies.clear();
            bullets.clear();
            walls.clear();
            fEnemies.clear();
            bullets.clear();
            ebullets.clear();
            window.draw(goBGSprite);
            window.draw(deathText);
            window.draw(startButton);
            window.draw(quitButton);
            window.draw(playGOText);
            window.draw(againGOText);
            window.draw(quitText);
            window.draw(qGameText);

            // Handle input for restarting or quitting after losing
            if (Mouse::isButtonPressed(Mouse::Left))
            {
                if (inputDelayClock.getElapsedTime().asSeconds() > 0.5f)
                {
                    if (startButton.getGlobalBounds().contains(mousePosWindow.x, mousePosWindow.y))
                    {
                        setupStartScreen();
                        currentState = START_SCREEN;
                        inputDelayClock.restart();
                    }
                    else if (quitButton.getGlobalBounds().contains(mousePosWindow.x, mousePosWindow.y)) {
                        window.close();
                    }
                }
            }
        }

        // Display everything that has been drawn
        window.display();
    }

    return 0;
}

