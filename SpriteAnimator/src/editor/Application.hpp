#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics.hpp>
#include <utils/TileInfo.hpp>
#include <Thor/Input/ActionMap.hpp>
#include <Thor/Animations.hpp>

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
		void drawGrid();
		void addAnimationsToGUI();
		void pollEvents();
		void update(sf::Time dt);
		void updateGUI();
		void openTextureFile();
		void saveAnimations();
		void outputAnimationData(const std::string& folderPath);
		const bool hasLoadedTexture() const;
		const bool hasSelectedTileSize() const;

	private:
		// Animations
		void addAnimation(const std::string& id, const thor::FrameAnimation& anim, float duration);
		void addFrameAnimation(thor::FrameAnimation& anim, const sf::FloatRect& rect, float duration);
		void playAnimation(const std::string& id, bool repeat = false);
		void updateAnimation(sf::Time dt);

	private:
		// Types
		using AnimationsHolder = thor::AnimationMap<sf::Sprite, std::string>;
		using Animator = thor::Animator<sf::Sprite, std::string>;

	private:
		struct FrameInfo
		{
			int spriteIndex = 0;
			float duration = 1.f;
		};

		struct AnimationInfo
		{
			std::vector<FrameInfo> framesDetail;
			thor::FrameAnimation frameAnimation;
			float duration = 1.f;
			bool submitted = false;
		};

	private:
		std::string m_playingAnimation;
		sf::Vector2f m_selectedTile;
		sf::Vector2i m_tileSize;
		std::map<std::string, AnimationInfo> m_animations;
		std::vector<TileInfo> m_tiles;
		AnimationsHolder m_spriteAnimations;
		Animator m_animator;

	private:
		sf::RenderWindow m_window;
		thor::ActionMap<std::string> m_actions;
		sf::Sprite m_sprite;
		sf::Sprite m_playButton;
		sf::Sprite m_pauseButton;
		sf::Texture m_playButtonTexture;
		sf::Texture m_pauseButtonTexture;
		sf::Texture m_spritesheet;

	private:
		static bool m_showSpriteSheet;
	};
}