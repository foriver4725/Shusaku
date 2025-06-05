#pragma once

#include <functional>
#include "TypeAlias.hpp"

namespace Shusaku
{
	struct Pos final
	{
		uint8 x = 0;
		uint8 y = 0;

		inline bool operator==(const Pos& other) const noexcept { return x == other.x && y == other.y; }
		inline bool operator!=(const Pos& other) const noexcept { return !(*this == other); }
		inline operator std::pair<uint8, uint8>() const { return { x, y }; }
	};
}

namespace std
{
	template <>
	struct hash<Shusaku::Pos>
	{
		std::size_t operator()(const Shusaku::Pos& p) const noexcept
		{
			return (std::hash<int>()(p.x) ^ (std::hash<int>()(p.y) << 1));
		}
	};
}
