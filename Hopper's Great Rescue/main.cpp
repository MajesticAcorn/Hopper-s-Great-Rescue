#include <SFML/Graphics.hpp>
#include <iostream>
#include "Walls.h"
#include "Player.h"

using namespace std;
using namespace sf;


Clock cl;

//Opening the window
int main()
{
	//Generates the window using SFML, default style adds Titlebar, ability to resize, and close button
	sf::RenderWindow window(sf::VideoMode(1280, 720), "Hopper's Great Rescue", sf::Style::Default);

	//Creates an object "player" from the class "player", and sets its size
	Player player(50, 40);

	//Creates The walls
	std::vector<RectangleShape> walls;
	RectangleShape wall;
	wall.setFillColor(Color::Black);
	wall.setSize(Vector2f(50, 50));
	wall.setPosition(100, 100);
	walls.push_back(wall);

	

	//main loop of the window, it runs the program as long as the window is not closed
	while (window.isOpen())
	{

		//checks if any events have occured to the window, poll event will return true if there was an event pending, false if not
		sf::Event event;
		while (window.pollEvent(event))
		{
			//Checks for these events, if the event is triggered run the following line
			if (event.type == sf::Event::Closed)
				window.close();
			if (event.type == sf::Event::KeyPressed)
				player.processEvetns(event.key.code, true);
			if (event.type == sf::Event::KeyReleased)
				player.processEvetns(event.key.code, false);
			if (event.mouseButton.button == sf::Mouse::Right && cl.getElapsedTime().asSeconds() > 0.1)
			{
				player.Shoot(window);
				cl.restart();
			}
		}


		//Render

		// clears the window of any previous frames, then Colors the screen green 
		window.clear(sf::Color::Blue);

		//Draws the player to the window
		player.drawTo(window);

		//Draws the bullets to the window
		player.drawBullets(window);

		//Draws walls, is a for loop to support multiple walls
		for (auto& i : walls)
		{
			window.draw(i);
		}

		//Updates the position of the player
		player.update();

		//Takes what was drawn since the last call to display, and displays it on the window
		window.display();

		//Handles the players collision
		player.Collide();

	}

	return 0;

}