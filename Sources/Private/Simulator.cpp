#include <Simulator.hpp>

using namespace Shusaku;

Stone Simulator::Judge(const Board& board)
{
	constexpr uint8 Comi = 7;  // コミ (黒が出す)

	// アゲハマの数を取得
	UNUSED const uint64 hamaBlack = board.GetHamaBlack();
	UNUSED const uint64 hamaWhite = board.GetHamaWhite();

	// 黒石と白石の数、空き点の数をカウントする
	autosize blackStoneCount = 0, whiteStoneCount = 0, emptyCount = 0;
	for (const Stone& stone : board.GetBoard())
	{
		if (stone == Stone::Black)
			++blackStoneCount;
		else if (stone == Stone::White)
			++whiteStoneCount;
		else if (stone == Stone::Empty)
			++emptyCount;
	}

	// 簡易的に、盤面上の石の数が多かった方を勝ちとする
	// (中国ルールから着想を得た)

	autosize blackScore = blackStoneCount;
	autosize whiteScore = whiteStoneCount + Comi;  // コミを白に加算する

	if (blackScore > whiteScore) return Stone::Black;
	if (blackScore < whiteScore) return Stone::White;
	return Stone::Empty;
}

Pos Simulator::Think(Stone stone, const Board& board, double* outWinRate)
{
	// ハードウェアのスレッド数
	static const uint32 HardwareThreads = std::thread::hardware_concurrency();

	// 着手を考えるとき、それぞれの空き点で、何回終局まで試行するか
	// ハードウェアのスレッド数を元に動的に設定
	// 数値は何となく
	static const uint64 ThinkCount = std::max<uint32>(HardwareThreads << 2, 8);

	const uint8 Size = board.GetSize();
	const autosize PositionsCount = board.GetPositionsCount();

	// 勝率を格納 (0.0が最低、1.0が最高)
	// index = (x-1)+(y-1)*Size で計算する
	vec<double> winRates(PositionsCount, MIN_double);

	// 空き点の場所を順に見ていき、勝率を求める
	for (uint8 x = 1; x <= Size; ++x)
		for (uint8 y = 1; y <= Size; ++y)
		{
			if (board.GetStone(x, y) != Stone::Empty) continue;

			// ここに着手した場合の勝率を、シミュレーションする

			// 盤面をコピーして、着手してみる
			Board tempBoard = board;
			if (!tempBoard.PutStone(x, y, stone)) continue;

			// 非同期で並列シミュレーション
			vec<std::future<Stone>> futures;
			futures.reserve(ThinkCount);
			for (UNUSED autosize i = 0; i < ThinkCount; ++i)
			{
				futures.emplace_back(std::async(std::launch::async,
					[stone, tempBoardCopy = tempBoard]() -> Stone { return Simulator::__Try(ReverseStone(stone), tempBoardCopy); }
				));
			}

			// シミュレーションの結果を取得し、勝率を算出する
			autosize winCount = 0;
			for (auto& future : futures)
				if (future.get() == stone)
					++winCount;
			double winRate = std::clamp(1.0 * winCount / ThinkCount, 0.0, 1.0);  // 最終数値

			// 勝率を格納
			winRates[(x - 1) + (y - 1) * Size] = winRate;
		}

	// 勝率が最大の場所を探す

	Pos bestPos = { 0, 0 };
	double bestWinRate = MIN_double;

	for (uint8 x = 1; x <= Size; ++x)
		for (uint8 y = 1; y <= Size; ++y)
		{
			if (board.GetStone(x, y) != Stone::Empty) continue;

			double winRate = winRates[(x - 1) + (y - 1) * Size];
			if (winRate > bestWinRate)
			{
				bestWinRate = winRate;
				bestPos = { x, y };
			}
		}

	// 値を返す
	if (outWinRate)
		*outWinRate = bestWinRate;
	return bestPos;
}

Stone Simulator::__Try(Stone stone, const Board& boardTemplate, Board* outResultBoard)
{
	Stone turn = stone;
	Board board = boardTemplate;

	const uint8 Size = board.GetSize();
	const uint16 PositionsCount = board.GetPositionsCount();
	constexpr double EarlyGameFilledRateLimit = 0.8;  // 序盤の埋まり具合の閾値 (これを越えたら、終盤とみなす)

	// 対局の最大手数 (同形反復の無限ループなどを回避、という理由もある)
	// 処理コストを考慮して、上限値を設定する
	const uint16 MaxTurns = static_cast<uint16>(std::min<uint32>(PositionsCount << 1, 200));

	// 空き点の数がこれ以下になった段階で、シミュレーションを終了する
	// 自分の眼を潰す、などの行為を避けるため
	constexpr uint8 EmptyCountLimit = 6;

	// 序盤では全交点の中からランダムに、終盤では先に空き点を探しその中からランダムに、着手を試行する
	const uint32 MaxPutChecksOnEarlyGame = PositionsCount << 1;  // 着手の最大試行回数 (序盤)
	const uint32 MaxPutChecksOnLateGame = PositionsCount >> 2;  // 着手の最大試行回数 (終盤)

	// 空き点の位置を調べる (終盤のみならず、盤面が埋まって来たら終局にするところでも使うので、最初に調べておく)
	vec<Pos> emptyPositions;  // これに格納
	emptyPositions.reserve(PositionsCount);
	for (uint8 x = 1; x <= Size; ++x)
		for (uint8 y = 1; y <= Size; ++y)
			if (board.GetStone(x, y) == Stone::Empty)
				emptyPositions.emplace_back(x, y);

	// 打つところがなかったら、パスする
	// 双方がパスしたら終局
	bool passed = false;

	for (UNUSED uint64 i = 0; i < MaxTurns; ++i)
	{
		bool couldPut = false;

		const double BoardFilledRate = 1.0 * board.GetHistory().size() / PositionsCount;  // 簡単に、盤面の埋まり具合を計算する
		const bool IsEarlyGame = BoardFilledRate < EarlyGameFilledRateLimit;  // 序盤かどうか判定する
		if (IsEarlyGame)
		{
			for (UNUSED uint32 j = 0; j < MaxPutChecksOnEarlyGame; ++j)
			{
				// ランダムに交点を選ぶ
				uint8 x, y;
				do
				{
					x = static_cast<uint8>(Rand::Range(1, Size + 1));
					y = static_cast<uint8>(Rand::Range(1, Size + 1));
				} while (board.GetStone(x, y) != Stone::Empty);

				// 着手を試行する
				if (!board.PutStone(x, y, turn)) continue;

				// 着手できた
				{
					// 空き点から削除
					auto it = std::find(emptyPositions.begin(), emptyPositions.end(), Pos(x, y));
					if (it != emptyPositions.end())
						emptyPositions.erase(it);
				}
				couldPut = true;
				break;
			}
		}
		else
		{
			// 空き点があるならば、そこからランダムに着手を試行する
			if (!emptyPositions.empty())
			{
				for (UNUSED uint32 j = 0; j < MaxPutChecksOnLateGame; ++j)
				{
					// ランダムに空き点を選ぶ
					const autosize Idx = Rand::Range(0, emptyPositions.size() - 1);
					const Pos& pos = emptyPositions[Idx];

					// 着手を試行する
					if (!board.PutStone(pos, turn)) continue;

					// 着手できた
					emptyPositions.erase(emptyPositions.begin() + Idx);  // 空き点から削除
					couldPut = true;
					break;
				}
			}
		}

		// 着手箇所がなかった
		if (!couldPut)
		{
			if (passed) break;  // 双方がパスしたので、終局
			else
			{
				// パスする
				passed = true;
				turn = ReverseStone(turn);
				continue;
			}
		}
		else
			passed = false;

		// 着手できた

		// 終局判定
		{
			const autosize EmptyCount = emptyPositions.size();
			if (EmptyCount <= EmptyCountLimit)
				break;
		}

		turn = ReverseStone(turn);
	}

	// 終局した

	// 値を返す
	if (outResultBoard)
		*outResultBoard = board;
	return Judge(board);
}
