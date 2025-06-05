#pragma once

#include <cmath>

namespace Shusaku
{
	class Math final
	{
	public:

		// 区間を線形変換する
		// value が fromMin から fromMax の範囲にあるとき、戻り値は toMin から toMax の範囲に変換される
		// ただし、この範囲外についても、延伸して線形変換される
		inline static constexpr float Remap(float value, float fromMin, float fromMax, float toMin, float toMax)
		{
			if (fromMin == fromMax) return toMin;
			return (value - fromMin) * (toMax - toMin) / (fromMax - fromMin) + toMin;
		}

		// 区間を線形変換する
		// value が fromMin から fromMax の範囲にあるとき、戻り値は toMin から toMax の範囲に変換される
		// ただし、この範囲外についても、延伸して線形変換される
		inline static constexpr double Remap(double value, double fromMin, double fromMax, double toMin, double toMax)
		{
			if (fromMin == fromMax) return toMin;
			return (value - fromMin) * (toMax - toMin) / (fromMax - fromMin) + toMin;
		}

		// 区間を線形変換する
		// value が fromMin から fromMax の範囲にあるとき、戻り値は toMin から toMax の範囲に変換される
		// この範囲外についても、戻り値は必ず toMin から toMax の範囲に制限される
		inline static constexpr float RemapClamped(float value, float fromMin, float fromMax, float toMin, float toMax)
		{
			float out = Remap(value, fromMin, fromMax, toMin, toMax);
			const auto [Min, Max] = std::minmax(toMin, toMax);
			return std::clamp(out, Min, Max);
		}

		// 区間を線形変換する
		// value が fromMin から fromMax の範囲にあるとき、戻り値は toMin から toMax の範囲に変換される
		// この範囲外についても、戻り値は必ず toMin から toMax の範囲に制限される
		inline static constexpr double RemapClamped(double value, double fromMin, double fromMax, double toMin, double toMax)
		{
			double out = Remap(value, fromMin, fromMax, toMin, toMax);
			const auto [Min, Max] = std::minmax(toMin, toMax);
			return std::clamp(out, Min, Max);
		}
	};
}
