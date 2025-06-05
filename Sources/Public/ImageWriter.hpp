#pragma once

#include <Core.hpp>

class ImageWriter final
{
public:

	inline ImageWriter() = delete;

	static void Write(const str& path, const Shusaku::Board& board, bool bWithHistory = true);
	static void Show(const Shusaku::Board& board, bool bWithHistory = true, bool waitKey = true);

	// 黒にとっての勝率
	static void WriteGraph(const str& path, const vec<double>& winRates);
	// 黒にとっての勝率
	static void ShowGraph(const vec<double>& winRates, bool waitKey = true);

	static void WriteHistory(const str& path, const vec<Shusaku::PosStone>& history);
};
