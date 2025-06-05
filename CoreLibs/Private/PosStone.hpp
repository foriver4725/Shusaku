#pragma once

#include <functional>
#include "TypeAlias.hpp"
#include "StoneEnum.hpp"
#include "Pos.hpp"

namespace Shusaku
{
	struct PosStone final
	{
		Pos pos;
		Stone stone;

		inline bool operator==(const PosStone& other) const noexcept { return pos == other.pos && stone == other.stone; }
		inline bool operator!=(const PosStone& other) const noexcept { return !(*this == other); }
	};
}

namespace std
{
	template <>
	struct hash<Shusaku::PosStone>
	{
		std::size_t operator()(const Shusaku::PosStone& ps) const noexcept
		{
			return (std::hash<Shusaku::Pos>()(ps.pos) ^ (std::hash<Shusaku::Stone>()(ps.stone) << 1));
		}
	};
}
