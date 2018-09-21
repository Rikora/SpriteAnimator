////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <editor/Application.hpp>
#include <SFML/Window/Event.hpp>
#include <imgui.h>
#include <imgui-SFML.h>

namespace px
{
	Application::Application() : m_window(sf::VideoMode(1200U, 800U), "Sprite Animator", sf::Style::Close,
										  sf::ContextSettings(0U, 0U, 8U))
	{
		m_window.setVerticalSyncEnabled(true);
		ImGui::SFML::Init(m_window);

		// Supply actions to the action map
		thor::Action eventClosed(sf::Event::Closed);
		thor::Action close(sf::Keyboard::Escape, thor::Action::PressOnce);

		m_actions["close"] = eventClosed || close;
	}

	Application::~Application()
	{
		ImGui::SFML::Shutdown();
	}

	void Application::pollEvents()
	{
		m_actions.clearEvents();

		sf::Event event;
		while (m_window.pollEvent(event))
		{
			ImGui::SFML::ProcessEvent(event);
			m_actions.pushEvent(event);
		}

		// React to different action types
		if (m_actions.isActive("close"))
			m_window.close();
	}

	void Application::update(sf::Time dt)
	{
		ImGui::SFML::Update(m_window, dt);
	}

	void Application::updateGUI()
	{	
	}

	void Application::render()
	{
		m_window.clear();
		ImGui::SFML::Render(m_window);
		m_window.display();
	}

	void Application::run()
	{
		sf::Clock clock;

		while (m_window.isOpen())
		{
			pollEvents();
			update(clock.restart());
			updateGUI();
			render();
		}
	}
}