#pragma once

#include <Core.hpp>

// 地の判定は難しいので、勝敗判定はある程度妥協する
class Simulator final
{
public:

	inline Simulator() = delete;

	// 勝敗判定を行う
	static Shusaku::Stone Judge(const Shusaku::Board& board);

	// 与えられた盤面について、次の一手を考える (stone の手番)
	// 最善の着手を返し、その勝率を outWinRate に返す (nullptr なら行わない)
	// 最善の着手を返すだけなので、それを元にパス・投了を判断するのは、メイン処理部分で行うこと
	// 有効手が見つからなかった場合は、(0, 0) を返し、outWinRate は (nullptr でないなら) MIN_double になる (発生しないはず)
	// 単純なモンテカルロ木探索 (ランダムに最後まで着手し、最も勝率の高い手を選ぶ) に基づく
	// 左上角が (1, 1), 右下角が (size, size) の座標系
	static Shusaku::Pos Think(Shusaku::Stone stone, const Shusaku::Board& board, double* outWinRate = nullptr);

	// 与えられた盤面から終局までランダムに試行を行う (stone の手番)
	// 勝った方の石の種類を返し、終局時の盤面を outResultBoard にコピーする (nullptr なら行わない)
	// 勝敗が付かなかった場合は、Stone::Empty を返す
	// 単純なモンテカルロ木探索 (ランダムに最後まで着手し、最も勝率の高い手を選ぶ) に基づく
	// 投了はせず、盤面の空きマスが一定値以下になった段階で終局とする
	// 内部処理用
	static Shusaku::Stone __Try(Shusaku::Stone stone, const Shusaku::Board& boardTemplate, Shusaku::Board* outResultBoard = nullptr);
};
