#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics.hpp>
#include <Thor/Input/ActionMap.hpp>
#include <Thor/Animations/Animator.hpp>
#include <Thor/Animations/FrameAnimation.hpp>

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
		void drawGrid();

	private:
		// Test functionality for animations
		thor::FrameAnimation addFrames(thor::FrameAnimation & anim, int row, int frames, int pxSize = 64, float duration = 1.f);
		void addAnimation(const std::string & id, int row, int frames, sf::Time duration = sf::seconds(1.f));
		void playAnimation(const std::string & id, bool repeat = false);
		void updateAnimation(sf::Time dt);

	private:
		using AnimationsHolder = thor::AnimationMap<sf::Sprite, std::string>;
		using Animator = thor::Animator<sf::Sprite, std::string>;

	private:
		sf::RenderWindow m_window;
		thor::ActionMap<std::string> m_actions;
		AnimationsHolder m_animations;
		Animator m_animator;
		sf::Sprite m_sprite;
		sf::Texture m_spriteTexture;
		sf::Vector2f m_selectedTile;
	};
}