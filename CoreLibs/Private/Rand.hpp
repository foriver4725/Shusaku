#pragma once

#include <random>
#include "TypeAlias.hpp"

namespace Shusaku
{
	class Rand final
	{
		template <typename T>
		using randint = std::uniform_int_distribution<T>;
		template <typename T>
		using randreal = std::uniform_real_distribution<T>;

	public:

		// シード値を変更 (呼び出されたスレッドでのみ有効)
		inline static void ChangeSeed(uint32 seed) { engine.seed(seed); }

		// [min, max] の範囲の整数を生成
		inline static int32 Range(int32 min, int32 max) { return randint<int32>{min, max}(engine); }

		// [min, max) の範囲の実数を生成
		inline static float Range(float min, float max) { return randreal<float>{min, max}(engine); }
		// [min, max) の範囲の実数を生成
		inline static double Range(double min, double max) { return randreal<double>{min, max}(engine); }

	private:

		// 乱数生成器
		inline static thread_local std::mt19937 engine = std::mt19937(std::random_device{}());
	};
}
