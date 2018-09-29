//////////////////////////////////////////////////////////////
//// Headers
//////////////////////////////////////////////////////////////
#include <editor/Application.hpp>

int main()
{
	px::Application app;
	app.run();
	return 0;
}

/// Usage example of integration
//#include <SFML/Graphics.hpp>
//#include <SFML/Window/Event.hpp>
//#include <SFML/System/Clock.hpp>
//#include <loader/SpriteAnimator.hpp>
//
//int main()
//{
//	sf::RenderWindow window(sf::VideoMode(800, 600), "Animation");
//	sf::Clock clock;
//	sf::Sprite sprite;
//	sf::Texture spriteSheet;
//	px::SpriteAnimator animator;
//
//	// Prepare sprite
//	spriteSheet.loadFromFile("src/res/sprites/orc.png");
//	sprite.setTexture(spriteSheet);
//	sprite.setPosition(sf::Vector2f(400.f, 300.f));
//
//	// Load animations
//	animator.loadAnimation("src/res/data/up_orc.anim");
//	animator.loadAnimation("src/res/data/left_orc.anim");
//	animator.playAnimation("left_orc", true);
//
//	while (window.isOpen())
//	{
//		sf::Event event;
//
//		while (window.pollEvent(event))
//		{
//			if (event.type == sf::Event::Closed)
//				window.close();
//		}
//		
//		// Update animation system
//		animator.update(sprite, clock.restart());
//
//		window.clear();
//		window.draw(sprite);
//		window.display();
//	}
//
//	return 0;
//}