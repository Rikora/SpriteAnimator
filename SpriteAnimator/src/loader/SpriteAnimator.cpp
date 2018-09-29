////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SpriteAnimator.hpp"
#include <SFML/Graphics/Sprite.hpp>
#include <fstream>
#include <json.hpp>

using namespace nlohmann;

namespace px
{
	SpriteAnimator::SpriteAnimator() : 
	m_animator(m_animations)
	{
	}

	void SpriteAnimator::update(sf::Sprite& animated, sf::Time dt)
	{
		m_animator.update(dt);
		m_animator.animate(animated);
	}

	void SpriteAnimator::loadAnimation(const std::string& animation)
	{
		// Parse the animation file
		std::ifstream i(animation);
		json data;
		i >> data;
		i.close();

		// Load all the frames
		thor::FrameAnimation anim;
		for (unsigned i = 0; i < data["frames"].get<unsigned>(); ++i)
		{
			sf::FloatRect rect = sf::FloatRect(data["frame"][i]["floatRect"][0], data["frame"][i]["floatRect"][1],
											   data["frame"][i]["floatRect"][2], data["frame"][i]["floatRect"][3]);
			float duration = data["frame"][i]["duration"];
			addFrameAnimation(anim, rect, duration);
		}

		// Add the animation to the container
		addAnimation(data["id"], anim, data["duration"]);
	}

	/*void SpriteAnimator::removeAnimation(const std::string & id)
	{
		m_animations.removeAnimation(id);
	}*/

	void SpriteAnimator::playAnimation(const std::string& id, bool repeat)
	{
		repeat ? m_animator.play() << id << thor::Playback::loop(id) : m_animator.play();
	}

	void SpriteAnimator::stop()
	{
		m_animator.stop();
	}

	void SpriteAnimator::addAnimation(const std::string& id, const thor::FrameAnimation& anim, float duration)
	{
		m_animations.addAnimation(id, anim, sf::seconds(duration));
	}

	void SpriteAnimator::addFrameAnimation(thor::FrameAnimation& anim, const sf::FloatRect& rect, float duration)
	{
		anim.addFrame(duration, static_cast<sf::IntRect>(rect));
	}
}