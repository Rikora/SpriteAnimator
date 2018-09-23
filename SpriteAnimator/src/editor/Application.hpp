#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics.hpp>
#include <utils/TileInfo.hpp>
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
		void addAnimationsToGUI();

	private:
		// Test functionality for animations
		thor::FrameAnimation addFrames(thor::FrameAnimation& anim, int row, int frames, int pxSize = 64, float duration = 1.f);
		void addAnimation(const std::string& id, int row, int frames, sf::Time duration = sf::seconds(1.f));
		void playAnimation(const std::string& id, bool repeat = false);
		void updateAnimation(sf::Time dt);

	private:
		using AnimationsHolder = thor::AnimationMap<sf::Sprite, std::string>;
		using Animator = thor::Animator<sf::Sprite, std::string>;

	private:
		struct AnimationInfo
		{
			int spriteIndex = 0;
			float duration = 1.f;
		};

	private:
		sf::RenderWindow m_window;
		sf::Vector2f m_selectedTile;
		std::vector<TileInfo> m_tiles;
		std::vector<AnimationInfo> m_animations;
		sf::Vector2i m_tileSize;

	private:
		thor::ActionMap<std::string> m_actions;
		AnimationsHolder m_spriteAnimations;
		Animator m_animator;
		sf::Sprite m_sprite;
		sf::Texture m_spritesheet;

	private:
		static bool m_showSpriteSheet;
	};
}