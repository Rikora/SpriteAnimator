#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics.hpp>
#include <Thor/Input/ActionMap.hpp>

namespace px
{
	class Application
	{
	public:
		Application();
		~Application();

	public:
		void run();

	private:
		void pollEvents();
		void update(sf::Time dt);
		void updateGUI();
		void render();

	private:
		sf::RenderWindow m_window;
		thor::ActionMap<std::string> m_actions;
	};
}