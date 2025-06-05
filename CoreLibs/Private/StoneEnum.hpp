#pragma once

#include <functional>
#include "TypeAlias.hpp"

namespace Shusaku
{
	enum class Stone : uint8
	{
		Empty = 0,
		Black = 1,
		White = 2,
	};

	inline constexpr Stone ReverseStone(Stone stone)
	{
		switch (stone)
		{
		case Stone::Black:
			return Stone::White;
		case Stone::White:
			return Stone::Black;
		default:
			return Stone::Empty;
		}
	}
}

namespace std
{
	template<>
	struct hash<Shusaku::Stone>
	{
		size_t operator()(const Shusaku::Stone& stone) const noexcept
		{
			return static_cast<size_t>(stone);
		}
	};
}
