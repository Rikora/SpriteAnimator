#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <algorithm>
#include <SFML/System/Vector2.hpp>

namespace px
{
	namespace utils
	{
		inline void constrainNegativesFloat(float& value)
		{
			value = std::clamp(value, 0.f, std::numeric_limits<float>::max());
		}

		inline void constrainNegativesVec(sf::Vector2i& value)
		{
			value.x = std::clamp(value.x, 0, std::numeric_limits<int>::max());
			value.y = std::clamp(value.y, 0, std::numeric_limits<int>::max());
		};
	}
}