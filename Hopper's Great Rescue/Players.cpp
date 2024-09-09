#include "Players.h"
#include <iostream>

// Constructor to initialize the player's attributes
Players::Players(float x, float y, sf::Texture& playerTexture) 
{
    shieldActive = false;
    health = 6;
    rectangle.setSize(sf::Vector2f(x, y));
    rectangle.setTexture(&playerTexture);
    rectangle.setTextureRect(sf::IntRect(450, 550, 1000, 900)); //Left & Right position, up and down position, left and right stretch, up and down stretch
    //rectangle.setOutlineThickness(1);
    //rectangle.setOutlineColor(sf::Color::Red);
    moveUp = moveDown = moveLeft = moveRight = false;
}

// Draw the player to the window
void Players::drawTo(sf::RenderWindow& window) 
{
    window.draw(rectangle);
}

// Process key events for player movement 
void Players::processEvents(sf::Keyboard::Key key, bool checkPressed)
{
    if (checkPressed) 
    {
        if (key == sf::Keyboard::W)
            moveUp = true;
        if (key == sf::Keyboard::A)
            moveLeft = true;
        if (key == sf::Keyboard::S)
            moveDown = true;
        if (key == sf::Keyboard::D)
            moveRight = true;
    }
    else {
        if (key == sf::Keyboard::W)
            moveUp = false;
        if (key == sf::Keyboard::A)
            moveLeft = false;
        if (key == sf::Keyboard::S)
            moveDown = false;
        if (key == sf::Keyboard::D)
            moveRight = false;
    }
}


// Update the player's position based on movement flags 
void Players::update(float dt)
{
    velocity = sf::Vector2f(0.f, 0.f);  // Reset velocity at the start of each update

    // Update the velocity based on movement inputs
    if (moveUp)
        velocity.y -= movementSpeed * dt;
    if (moveDown)
        velocity.y += movementSpeed * dt;
    if (moveLeft)
        velocity.x -= movementSpeed * dt;
    if (moveRight)
        velocity.x += movementSpeed * dt;

    // Normalize diagonal movement (avoid moving faster diagonally)
    if (velocity.x != 0.f && velocity.y != 0.f) {
        velocity.x /= sqrt(2.f);
        velocity.y /= sqrt(2.f);
    }

    rectangle.move(velocity); // Move the player by the calculated velocity
    playerbounds = rectangle.getGlobalBounds();  // Update bounding box after movement
    playerCenter = sf::Vector2f(rectangle.getPosition().x + 25, rectangle.getPosition().y + 25);
}

// Handle collisions with the screen edges
void Players::Collide(const sf::RenderWindow& window) 
{
    // Left wall collision
    if (rectangle.getPosition().x < 0.f)
        rectangle.setPosition(0.f, rectangle.getPosition().y);

    // Top wall collision
    if (rectangle.getPosition().y < 0.f)
        rectangle.setPosition(rectangle.getPosition().x, 0.f);

    // Right wall collision
    if (rectangle.getPosition().x + rectangle.getGlobalBounds().width > window.getSize().x)
        rectangle.setPosition(window.getSize().x - rectangle.getGlobalBounds().width, rectangle.getPosition().y);

    // Bottom wall collision
    if (rectangle.getPosition().y + rectangle.getGlobalBounds().height > window.getSize().y)
        rectangle.setPosition(rectangle.getPosition().x, window.getSize().y - rectangle.getGlobalBounds().height);
}

void Players::takeDamage(int damage) {
    if (shieldActive) {
        shieldActive = false;  // Deactivate shield after one hit
    }
    else {
        health -= damage;  // Reduce health only if shield is not active
    }
}
