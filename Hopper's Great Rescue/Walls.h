#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>


class Walls
{
public:
	static float wallleftSide;
	static float wallrightSide;
	static float walltopSide;
	static float wallbottomSide;

	Walls(float x, float y, float a, float b)
	{
		scale = 3;
		wall.setSize(sf::Vector2f(x, y));
		wall.setFillColor(sf::Color::Red);
		wall.setPosition(a, b);
		wallleftSide = wall.getPosition().x;
		wallrightSide = wall.getPosition().x + (wall.getLocalBounds().width * scale);
		walltopSide = wall.getPosition().y;
		wallbottomSide = wall.getPosition().y + (wall.getLocalBounds().height * scale);
	}
private:
	sf::RectangleShape wall;
	int scale;
};

