////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <editor/Application.hpp>
#include <SFML/Window/Event.hpp>
#include <imgui.h>
#include <imgui-SFML.h>

namespace px
{
	Application::Application() :
	m_window(
	sf::VideoMode(1200U, 800U), 
	"Sprite Animator", 
	sf::Style::Close, 
	sf::ContextSettings(0U, 0U, 8U)),
	m_animator(m_animations)
	{
		m_window.setVerticalSyncEnabled(true);
		ImGui::SFML::Init(m_window);

		// Supply actions to the action map
		thor::Action eventClosed(sf::Event::Closed);
		thor::Action close(sf::Keyboard::Escape, thor::Action::PressOnce);

		m_actions["close"] = eventClosed || close;

		// Load textures (should not be here later...)
		m_spriteTexture.loadFromFile("src/res/sprites/orc.png");
		m_sprite.setTexture(m_spriteTexture);

		// Test animations
		addAnimation("walk", 11, 9);
		playAnimation("walk", true);
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
		m_sprite.setPosition(sf::Vector2f(400.f, 400.f));

		ImGui::SFML::Update(m_window, dt);
		updateAnimation(dt);
	}

	void Application::updateGUI()
	{	
	}

	void Application::render()
	{
		m_window.clear();
		m_window.draw(m_sprite);
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

	thor::FrameAnimation Application::addFrames(thor::FrameAnimation & anim, int row, int frames, int pxSize, float duration)
	{
		// Remove loop for editor purpose?
		if (frames > 1)
			for (int i = 0; i < frames * pxSize; i += pxSize)
				anim.addFrame(duration, sf::IntRect(i, row * pxSize, pxSize, pxSize));
		else
			anim.addFrame(duration, sf::IntRect(0, row * pxSize, pxSize, pxSize));

		return anim;
	}

	void Application::addAnimation(const std::string & id, int row, int frames, sf::Time duration)
	{
		thor::FrameAnimation frameAnim;
		m_animations.addAnimation(id, addFrames(frameAnim, row, frames), duration);
	}

	void Application::playAnimation(const std::string & id, bool repeat)
	{
		repeat ? m_animator.play() << id << thor::Playback::loop(id) : m_animator.play();
	}

	void Application::updateAnimation(sf::Time dt)
	{
		m_animator.update(dt);
		m_animator.animate(m_sprite);
	}
}