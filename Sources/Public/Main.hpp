#pragma once

#include <Core.hpp>

#include <Simulator.hpp>
#include <ImageWriter.hpp>
#include <PathMaker.hpp>

inline int Main()
{
	using namespace Shusaku;

	// true なら自動、false なら手動で着手する
	constexpr bool BlackAuto = true;
	constexpr bool WhiteAuto = true;

	// パス・終局を判定する際の、勝率の閾値
	constexpr double WinRateThreshold = 0.1;

	Board board = Board::Create9x9();
	const uint8 Size = board.GetSize();

	Stone turn = Stone::Black;
	bool dontWannaPut = false;  // 着手した際の自分の勝率がかなり低いので、パスしたというフラグ
	double winRate = MIN_double;  // 勝率をメモ (この変数のポインタを使いまわす)
	// コンピュータが算出した、各着手における勝率を保存する
	// 1 手目は黒番が、2 手目は白番が、3 手目は黒番が、交互に算出する
	// 終局まで何手かかるか読みにくいので、reserve はしないでおく
	vec<double> winRates{};
	Stone forcibleWin = Stone::Empty;  // 投了したときの勝者を記録しておく

	// 最初の盤面を表示する
	ImageWriter::Show(board, true, false);

	while (true)
	{
		// 着手を考える
		// メインは自動対戦なので、手動入力の値のバリデーションは簡単にしている
		// 数字以外を入力したり、既に石が置かれている場所を指定したり出来てしまうが、今回は気にしない
		Pos nextPos;
		if (turn == Stone::Black)
		{
			if (BlackAuto)
			{
				nextPos = Simulator::Think(turn, board, &winRate);
				winRates.push_back(winRate);  // 勝率を保存
			}
			else
			{
				int x, y;
				do
				{
					std::cout << "Black turn: 'x y' = ";
					std::cin >> x >> y;
				} while (x < 1 || Size < x || y < 1 || Size < y);
				nextPos = { static_cast<uint8>(x), static_cast<uint8>(y) };
			}
		}
		else if (turn == Stone::White)
		{
			if (WhiteAuto)
			{
				nextPos = Simulator::Think(turn, board, &winRate);
				winRates.push_back(winRate);  // 勝率を保存
			}
			else
			{
				int x, y;
				do
				{
					std::cout << "White turn: 'x y' = ";
					std::cin >> x >> y;
				} while (x < 1 || Size < x || y < 1 || Size < y);
				nextPos = { static_cast<uint8>(x), static_cast<uint8>(y) };
			}
		}
		else
			return 1;  // ここに来ることはない

		// 異常処理 : 有効手が見つからなかった場合、強制終局させる
		if (nextPos == Pos(0, 0)) break;

		// 着手した際の、自分の勝率がかなり低かった
		if (winRate < WinRateThreshold)
		{
			// 双方、これ以上打ちたくなくてパスしたので、終局する
			if (dontWannaPut) break;
			else
			{
				// パスする (盤面は表示しない)
				dontWannaPut = true;
				turn = ReverseStone(turn);
				continue;
			}
		}
		// 相手の勝率がかなり低く、自分の勝率がかなり高いので、
		// 相手が投了したものとみなし、終局する
		else if (dontWannaPut && winRate > (1.0 - WinRateThreshold))
		{
			forcibleWin = turn;
			break;
		}
		else
			dontWannaPut = false;

		// 着手できる

		// 着手する
		// 異常処理 : 着手できなかった場合、強制終局させる
		if (!board.PutStone(nextPos, turn)) break;

		turn = ReverseStone(turn);

		// 盤面を表示する
		ImageWriter::Show(board, true, false);
	}

	// 勝敗を判定する
	// 投了の場合は、その勝者の情報をそのまま使う
	const Stone Win = forcibleWin != Stone::Empty ? forcibleWin : Simulator::Judge(board);

	// 出力部
	{
		// 勝敗をコンソールに出力する
		const str WinLog =
			Win == Stone::Black ? "BlackWin" :
			Win == Stone::White ? "WhiteWin" : "Draw";
		std::cout << "Result: " << WinLog << std::endl;

		// 対局の識別子を作成する
		const str Identifier = PathMaker::GetIdentifier({
			std::to_string(Size),
			str("Black") + str(BlackAuto ? "Auto" : "Manual"),
			str("White") + str(WhiteAuto ? "Auto" : "Manual"),
			WinLog,
			});

		// 終局時の盤面を保存する
		ImageWriter::Write(PathMaker::CreateWithDatetime("BoardOnEnd", Identifier), board);

		// 勝率のグラフを保存する
		ImageWriter::WriteGraph(PathMaker::CreateWithDatetime("WinRateGraph", Identifier), winRates);

		// 棋譜を保存する
		ImageWriter::WriteHistory(PathMaker::CreateWithDatetime("Kifu", Identifier), board.GetHistory());

		// 終局時の盤面を表示する
		ImageWriter::Show(board);

		// 勝率のグラフを表示する
		ImageWriter::ShowGraph(winRates);
	}

	return 0;
}
