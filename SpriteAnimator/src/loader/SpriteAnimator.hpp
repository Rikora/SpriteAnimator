#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <Thor/Animations/Animator.hpp>
#include <Thor/Animations/FrameAnimation.hpp>

namespace sf
{
	class Sprite;
}

namespace px
{
	class SpriteAnimator
	{
	public:
		SpriteAnimator();
		~SpriteAnimator() = default;

	public:
		void update(sf::Sprite& animated, sf::Time dt);
		void loadAnimation(const std::string& animation);
		//void removeAnimation(const std::string& id);
		void playAnimation(const std::string& id, bool repeat = false);
		void stop();

	private:
		void addAnimation(const std::string& id, const thor::FrameAnimation& anim, float duration);
		void addFrameAnimation(thor::FrameAnimation& anim, const sf::FloatRect& rect, float duration);

	private:
		// Types
		using AnimationsHolder = thor::AnimationMap<sf::Sprite, std::string>;
		using Animator = thor::Animator<sf::Sprite, std::string>;

	private:
		AnimationsHolder m_animations;
		Animator m_animator;
	};
}
