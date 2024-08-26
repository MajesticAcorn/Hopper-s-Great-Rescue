#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include "Walls.h"
#include <math.h>

using namespace std;
using namespace sf;


class Player
{
public:
	//Defines the player, and the default attributes of the class
	Player(float x, float y)
	{
		rectangle.setSize(sf::Vector2f(x, y));
		rectangle.setFillColor(sf::Color(100, 250, 50));
		moveUp = false;
		moveDown = false;
		moveLeft = false;
		moveRight = false;
	}
	//Draws the player (a rectangle) to the window
	void drawTo(sf::RenderWindow& window)
	{
		window.draw(rectangle);

	}
	//Event to check if key's are pressed. If they are WASD set accordingclass attribute to true
	void processEvetns(sf::Keyboard::Key key, bool checkPressed)
	{
		if (checkPressed == true)
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
		//If there are no key's pressed, set all according class attributes to false
		if (checkPressed == false)
		{
			moveUp = false;
			moveDown = false;
			moveLeft = false;
			moveRight = false;
		}
	}
	//This is what moves the player, and defines the players speed
	void update()
	{
		scale = 1;
		sf::Vector2f movement;
		if (moveUp)
			movement.y -= 0.35f;
		if (moveDown)
			movement.y += 0.35f;
		if (moveLeft)
			movement.x -= 0.35f;
		if (moveRight)
			movement.x += 0.35f;
		rectangle.move(movement);
		
		//WIP collision
		/*leftSide = rectangle.getPosition().x;
		rightSide = rectangle.getPosition().x + (rectangle.getLocalBounds().width * scale);
		topSide = rectangle.getPosition().y;
		bottomSide = rectangle.getPosition().y + (rectangle.getLocalBounds().height * scale);
		if(leftSide > Walls:wallleftSide)
		*/
	}
	void Collide()
	{
		//left wall collision
		if (rectangle.getPosition().x < 0.f)
			rectangle.setPosition(0.f, rectangle.getPosition().y);
		//top wall collision
		if (rectangle.getPosition().y < 0.f)
			rectangle.setPosition(rectangle.getPosition().x, 0.f);
		//right wall collision
		if (rectangle.getPosition().x + rectangle.getGlobalBounds().width > 1280)
			rectangle.setPosition(1280 - rectangle.getGlobalBounds().width, rectangle.getPosition().y);
		//bottom wall collision
		if (rectangle.getPosition().y + rectangle.getGlobalBounds().height > 720)
			rectangle.setPosition(rectangle.getPosition().x, 720 - rectangle.getGlobalBounds().height);

	}
	void Shoot(sf::RenderWindow& window)
	{
		bullets.push_back(CircleShape());
		bullets.back().setRadius(8);
		bullets.back().setOrigin(8, 8);
		if (bullets.back().getGlobalBounds().intersects(rectangle.getGlobalBounds()))
		{
			bullets.back().setRadius(15);
		}
		bullets.back().setPosition(rectangle.getPosition());
		angles.push_back(atan2(Mouse::getPosition(window).y - rectangle.getPosition().y, Mouse::getPosition(window).x - rectangle.getPosition().x));
	}

	void drawBullets(sf::RenderWindow& window)
	{
		for (int i = 0; i < bullets.size(); i++)
		{
			window.draw(bullets[i]);
			bullets[i].move(0.3 * cos(angles[i]), 0.3 * sin(angles[i]));
		}
	}

private:
	vector<float> angles;
	vector<CircleShape> bullets;
	sf::RectangleShape rectangle;
	float leftSide;
	float rightSide;
	float topSide;
	float bottomSide;
	bool moveUp;
	bool moveDown;
	bool moveLeft;
	bool moveRight;
	int scale;
};

