#define PLAYERS_H

#include <SFML/Graphics.hpp>

class Players {
public:
    std::vector<float> angles; // Stores angles for player rotation
    std::vector<sf::RectangleShape> bullets; // Stores bullets fired by the player
    sf::RectangleShape rectangle; // Player's visual representation
    float movementSpeed = 175.f; // Player's movement speed
    sf::Vector2f velocity; // Player's current velocity
    bool moveUp, moveDown, moveLeft, moveRight; // Movement flags
    sf::FloatRect playerbounds; // Player's bounding box for collision detection
    sf::Vector2f playerCenter; // Player's center point
    int health;
    bool shieldActive;

    // Constructor to initialize the player's attributes
    Players(float x, float y, sf::Texture& playerTexture);

    // Draw the player to the window
    void drawTo(sf::RenderWindow& window);

    // Process key events for player movement
    void processEvents(sf::Keyboard::Key key, bool checkPressed);

    // Update the player's position based on movement flags
    void update(float dt);

    // Handle collisions with the screen edges
    void Collide(const sf::RenderWindow& window);

    void takeDamage(int damage);
};

