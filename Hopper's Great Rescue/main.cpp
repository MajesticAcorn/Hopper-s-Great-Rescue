#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <math.h>
#include <vector>
#include <cstdlib>

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
sf::Text youText;
sf::Text winText;
sf::Text deathText;
sf::Text playText;
sf::Text againText;
sf::Text playGOText;
sf::Text againGOText;

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
    GAME_OVER,
    LEVEL_ONE,
    LEVEL_TWO,
    WIN_SCREEN,
    DEATH_SCREEN,
};

// Class to manage player behavior and attributes
class Player {
public:
    vector<float> angles; // Stores angles for player rotation
    vector<RectangleShape> bullets; // Stores bullets fired by the player
    sf::RectangleShape rectangle; // Player's visual representation
    const float movementSpeed = 175.f; // Player's movement speed
    Vector2f velocity; // Player's current velocity
    bool moveUp, moveDown, moveLeft, moveRight; // Movement flags for directional movement
    FloatRect playerbounds; // Player's bounding box for collision detection
    Vector2f playerCenter; // Player's center point

    // Constructor to initialize the player's attributes
    Player(float x, float y, sf::Texture& playerTexture)
    {
        rectangle.setSize(sf::Vector2f(x, y));
        rectangle.setTexture(&playerTexture); // Set the player's texture
        rectangle.setTextureRect(sf::IntRect(400, 400, 1000, 1000)); // Define the texture rectangle
        moveUp = moveDown = moveLeft = moveRight = false; // Initialize movement flags
    }

    // Draw the player to the window
    void drawTo(sf::RenderWindow& window)
    {
        window.draw(rectangle);
    }

    // Process key events for player movement
    void processEvetns(sf::Keyboard::Key key, bool checkPressed)
    {
        if (checkPressed == true)
        {
            // Set the movement flags based on key presses
            if (key == sf::Keyboard::W)
                moveUp = true;
            if (key == sf::Keyboard::A)
                moveLeft = true;
            if (key == sf::Keyboard::S)
                moveDown = true;
            if (key == sf::Keyboard::D)
                moveRight = true;
        }
        // If no keys are pressed, reset movement flags
        if (checkPressed == false)
        {
            moveUp = false;
            moveDown = false;
            moveLeft = false;
            moveRight = false;
        }
    }

    // Update the player's position based on movement flags
    void update()
    {
        velocity.y = 0.f;
        velocity.x = 0.f;

        // Adjust velocity based on movement flags
        if (moveUp)
            velocity.y += -movementSpeed * dt;
        if (moveDown)
            velocity.y += movementSpeed * dt;
        if (moveLeft)
            velocity.x += -movementSpeed * dt;
        if (moveRight)
            velocity.x += movementSpeed * dt;

        // Move the player based on the calculated velocity
        rectangle.move(velocity);
        playerbounds = rectangle.getGlobalBounds();
        playerCenter = Vector2f(rectangle.getPosition().x + 25, rectangle.getPosition().y + 25);
    }

    // Handle collisions with the screen edges
    void Collide()
    {
        // Prevent the player from moving out of the window bounds

        // Left wall collision
        if (rectangle.getPosition().x < 0.f)
            rectangle.setPosition(0.f, rectangle.getPosition().y);

        // Top wall collision
        if (rectangle.getPosition().y < 0.f)
            rectangle.setPosition(rectangle.getPosition().x, 0.f);

        // Right wall collision
        if (rectangle.getPosition().x + rectangle.getGlobalBounds().width > 720)
            rectangle.setPosition(720 - rectangle.getGlobalBounds().width, rectangle.getPosition().y);

        // Bottom wall collision
        if (rectangle.getPosition().y + rectangle.getGlobalBounds().height > 720)
            rectangle.setPosition(rectangle.getPosition().x, 720 - rectangle.getGlobalBounds().height);
    }
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
    startButton.setPosition(40, 275);
    startButton.setFillColor(sf::Color{ 19, 99, 3, 255 });

    // Setup Quit button
    quitButton.setSize(sf::Vector2f(100, 100));
    quitButton.setPosition(600, 275);
    quitButton.setFillColor(sf::Color{ 255, 114, 118, 255 });
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
void setupLevelOne(std::vector<RectangleShape>& enemies, std::vector<RectangleShape>& walls, Player& player,
    std::vector<Bullet>& bullets, std::vector<enemyBullet>& ebullets, sf::Texture& wallTexture, sf::Texture& enemyTexture)
{
    // Clear existing enemies, bullets, and walls
    enemies.clear();
    bullets.clear();
    ebullets.clear();
    walls.clear();

    // Initialize enemies
    RectangleShape enemy;
    enemy.setTexture(&enemyTexture);
    enemy.setTextureRect(sf::IntRect(500, 500, 1000, 1000));
    enemy.setSize(Vector2f(50.f, 50.f));
    enemy.setPosition(310, 310);
    enemies.push_back(RectangleShape(enemy));

    // Initialize walls
    RectangleShape wall;
    wall.setTexture(&wallTexture);
    wall.setTextureRect(sf::IntRect(250, 250, 1500, 1500));
    wall.setSize(Vector2f(50, 50));

    // Add walls to the level
    wall.setPosition(100, 100);
    walls.push_back(wall);
    wall.setPosition(310, 100);
    walls.push_back(wall);
    wall.setPosition(550, 100);
    walls.push_back(wall);
    wall.setPosition(100, 580);
    walls.push_back(wall);
    wall.setPosition(310, 580);
    walls.push_back(wall);
    wall.setPosition(550, 580);
    walls.push_back(wall);
    wall.setPosition(100, 310);
    walls.push_back(wall);
    wall.setPosition(550, 310);
    walls.push_back(wall);

    // Set player starting position
    player.rectangle.setPosition(50, 50);
}

// Current game state
GameState currentState = START_SCREEN;

// Main function to open the window and run the game
int main()
{
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

    sf::Texture enemyTexture;
    enemyTexture.loadFromFile("C:/Users/whatu/Desktop/C++ Project/Hopper's Great Rescue/Hopper's Great Rescue/Media/Enemy1.PNG");

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

    // Set up enemy attributes and position
    std::vector<RectangleShape> enemies;

    // Create a player object
    Player player(50, 50, playerTexture);

    // Variables to manage player and enemy bullets
    std::vector<Bullet> bullets;
    std::vector<enemyBullet> ebullets;

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
                player.processEvetns(event.key.code, true);
            if (event.type == sf::Event::KeyReleased)
                player.processEvetns(event.key.code, false);
            if (event.type == sf::Event::MouseButtonPressed)
            {
                // Only allow input if enough time has passed since the last state change
                if (inputDelayClock.getElapsedTime().asSeconds() > 0.5f)
                {
                    // Check if start button is clicked
                    if (startButton.getGlobalBounds().contains(mousePosWindow.x, mousePosWindow.y))
                    {
                        setupLevelOne(enemies, walls, player, bullets, ebullets, wallTexture, enemyTexture);
                        currentState = LEVEL_ONE;
                        inputDelayClock.restart(); // Restart the delay clock
                    }
                    else if (quitButton.getGlobalBounds().contains(mousePosWindow.x, mousePosWindow.y))
                    {
                        window.close();
                    }
                }
            }
        }

        // Handle wall collisions
        for (auto& wall : walls) {
            FloatRect wallBounds = wall.getGlobalBounds();
            sf::FloatRect nextPosition = player.playerbounds;
            nextPosition.left += player.velocity.x;
            nextPosition.top += player.velocity.y;

            if (wallBounds.intersects(nextPosition)) {
                float horizontalOverlap;
                if (player.velocity.x > 0)
                    horizontalOverlap = (player.playerbounds.left + player.playerbounds.width - wallBounds.left);
                else
                    horizontalOverlap = (wallBounds.left + wallBounds.width - player.playerbounds.left);

                float verticalOverlap;
                if (player.velocity.y > 0)
                    verticalOverlap = (player.playerbounds.top + player.playerbounds.height - wallBounds.top);
                else
                    verticalOverlap = (wallBounds.top + wallBounds.height - player.playerbounds.top);

                bool resolveHorizontal = horizontalOverlap < verticalOverlap;

                if (resolveHorizontal) {
                    if (player.velocity.x > 0) { // Moving right
                        player.velocity.x = 0.f;
                        player.rectangle.setPosition(wallBounds.left - player.playerbounds.width, player.playerbounds.top);
                    }
                    else if (player.velocity.x < 0) { // Moving left
                        player.velocity.x = 0.f;
                        player.rectangle.setPosition(wallBounds.left + wallBounds.width, player.playerbounds.top);
                    }
                }
                else {
                    if (player.velocity.y > 0) { // Moving down
                        player.velocity.y = 0.f;
                        player.rectangle.setPosition(player.playerbounds.left, wallBounds.top - player.playerbounds.height);
                    }
                    else if (player.velocity.y < 0) { // Moving up
                        player.velocity.y = 0.f;
                        player.rectangle.setPosition(player.playerbounds.left, wallBounds.top + wallBounds.height);
                    }
                }
            }
        }

        // Handle player shooting bullets
        Time shootCooldown = milliseconds(700);
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

        // Handle enemy shooting bullets
        Time enemyShootCooldown = milliseconds(1800);
        if (enemyBulletClock.getElapsedTime() >= enemyShootCooldown)
        {
            for (auto& enemy : enemies)
            {
                enemyBullet newEnemyBullet(8.f);
                newEnemyBullet.eBullet.setPosition(enemy.getPosition() + Vector2f(enemy.getSize().x / 2, enemy.getSize().y / 2));
                Vector2f enemyShootDir = player.playerCenter - newEnemyBullet.eBullet.getPosition();
                float length = sqrtf(pow(enemyShootDir.x, 2) + pow(enemyShootDir.y, 2));
                newEnemyBullet.enemyBulletcurrentVel = (enemyShootDir / length) * newEnemyBullet.enemyBulletSpeed;
                ebullets.push_back(newEnemyBullet);
                eshootSound.play(); // Play enemy shoot sound
            }
            enemyBulletClock.restart();
        }

        // Move enemy bullets
        for (int i = 0; i < ebullets.size(); ++i)
        {
            ebullets[i].eBullet.move(ebullets[i].enemyBulletcurrentVel);
            // Check if player is hit by an enemy bullet
            if (ebullets[i].eBullet.getGlobalBounds().intersects(player.playerbounds))
            {
                loseSound.play(); // Play lose sound
                setupDeathScreen();
                currentState = DEATH_SCREEN;
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
            for (size_t k = 0; k < enemies.size(); k++)
            {
                if (bullets[i].bullet.getGlobalBounds().intersects(enemies[k].getGlobalBounds()))
                {
                    bullets.erase(bullets.begin() + i);
                    enemies.erase(enemies.begin() + k);
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
        }

        // Draw the game for level one
        else if (currentState == LEVEL_ONE)
        {
            window.draw(floorSprite);

            // Draw enemies
            for (size_t i = 0; i < enemies.size(); i++)
            {
                window.draw(enemies[i]);
            }

            // Draw the player
            player.drawTo(window);

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
            player.update();

            // Handle player collision with screen edges
            player.Collide();

            // If all enemies are defeated, go to the win screen
            if (enemies.empty())
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
                        setupLevelOne(enemies, walls, player, bullets, ebullets, wallTexture, enemyTexture);
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
            enemies.clear();
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
                        setupLevelOne(enemies, walls, player, bullets, ebullets, wallTexture, enemyTexture);
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
