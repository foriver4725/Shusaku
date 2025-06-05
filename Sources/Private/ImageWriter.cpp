#include <ImageWriter.hpp>

using namespace Shusaku;

static vec<Pos> GetStarPositions(BoardSize boardSize);
static cv::Mat ConvertToPngImage(const Board& board, bool bWithHistory = false);
static cv::Mat ConvertGraphToPngImage(const vec<double>& winRates);

void ImageWriter::Write(const str& path, const Board& board, bool bWithHistory)
{
	const str OutputPath = "../Outputs/" + path + ".png";
	cv::Mat image = ConvertToPngImage(board, bWithHistory);
	cv::imwrite(OutputPath, image);
}

void ImageWriter::Show(const Board& board, bool bWithHistory, bool waitKey)
{
	const cv::Mat image = ConvertToPngImage(board, bWithHistory);
	cv::imshow("Board", image);

	// キー入力待ち (画像を閉じるため)
	// waitKey が true の場合は無限に待つ (0)、false の場合は 1秒 (1000ms) 待つ
	cv::waitKey(waitKey ? 0 : 1000);
}

void ImageWriter::WriteGraph(const str& path, const vec<double>& winRates)
{
	const str OutputPath = "../Outputs/" + path + ".png";
	cv::Mat image = ConvertGraphToPngImage(winRates);
	cv::imwrite(OutputPath, image);
}

void ImageWriter::ShowGraph(const vec<double>& winRates, bool waitKey)
{
	const cv::Mat image = ConvertGraphToPngImage(winRates);
	cv::imshow("Win Rate Graph", image);

	// キー入力待ち (画像を閉じるため)
	// waitKey が true の場合は無限に待つ (0)、false の場合は 1秒 (1000ms) 待つ
	cv::waitKey(waitKey ? 0 : 1000);
}

void ImageWriter::WriteHistory(const str& path, const vec<PosStone>& history)
{
	const str OutputPath = "../Outputs/" + path + ".txt";

	std::ofstream ofs(OutputPath, std::ios::out | std::ios::trunc);
	if (!ofs) return;

	for (autosize i = 0; i < history.size(); ++i)
	{
		const PosStone& Put = history[i];
		const str Turn = Put.stone == Stone::Black ? "B" : Put.stone == Stone::White ? "W" : "E";
		const str PosX = std::to_string(Put.pos.x);
		const str PosY = std::to_string(Put.pos.y);

		ofs << Turn << " " << PosX << "," << PosY << "\n";
	}

	ofs.close();
}

// 左上角が (1, 1), 右下角が (size, size) の座標系
vec<Pos> GetStarPositions(BoardSize boardSize)
{
	if (boardSize == BoardSize::_9x9)
	{
		return
		{
			{ 3, 3 }, { 7, 3 },
			{ 3, 7 }, { 7, 7 },
		};
	}
	else if (boardSize == BoardSize::_13x13)
	{
		return
		{
			{ 4, 4 }, { 7, 4 }, { 10, 4 },
			{ 4, 7 }, { 7, 7 }, { 10, 7 },
			{ 4, 10 }, { 7, 10 }, { 10, 10 },
		};
	}
	else if (boardSize == BoardSize::_19x19)
	{
		return
		{
			{ 4, 4 }, { 10, 4 }, { 16, 4 },
			{ 4, 10 }, { 10, 10 }, { 16, 10 },
			{ 4, 16 }, { 10, 16 }, { 16, 16 },
		};
	}
	else
		return {};
}

cv::Mat ConvertToPngImage(const Board& board, bool bWithHistory)
{
	constexpr uint8 LineWidth = 1;  // 線の太さ (px)
	constexpr uint16 ImageSize = 720;  // 画像のサイズ (px)
	constexpr uint8 MaxMargin = 50;  // 盤面の外側の余白の大きさ 最大値 (px)
	constexpr uint8 StoneMargin = 4;  // 石の余白 (px)
	constexpr uint8 StarRadius = 4;  // 星の半径 (px)

	const uint8 LineCount = board.GetSize();  // 盤面のサイズ (9x9, 19x19など)
	const uint8 CellSize = static_cast<uint8>(std::ceil(1.0 * (ImageSize - (MaxMargin << 1)) / (LineCount - 1)));  // 1マスのサイズ 切り上げ (px)
	const uint16 ImageBoardSize = CellSize * (LineCount - 1);  // 盤面のサイズ (px)
	const uint8 Margin = (ImageSize - ImageBoardSize) >> 1;  // 盤面の外側の余白の大きさ (px)

	const uint16 StoneRadius = (CellSize - StoneMargin) >> 1;  // 石の半径 (px)
	const uint8 StoneOutlineWidth = StoneRadius * 0.15;  // 石のアウトラインの太さ (px)

	static const cv::Scalar BoardColor = { 63, 194, 253 };  // 盤面の背景色 (BGR)
	static const cv::Scalar FrameColor = { 43, 56, 59 };  // 盤面の枠線の色 (BGR)
	static const cv::Scalar StarColor = FrameColor;  // 星の色 (BGR)
	static const cv::Scalar BlackStoneColor = { 0, 0, 0 };  // 黒石の色 (BGR)
	static const cv::Scalar WhiteStoneColor = { 255, 255, 255 };  // 白石の色 (BGR)
	static const cv::Scalar StoneOutlineColor = { 0, 0, 255 };  // 石のアウトラインの色 (赤) (BGR)
	static const cv::Scalar BlackStoneTextColor = WhiteStoneColor;  // 黒石の上に描画する手番の数字の色 (BGR)
	static const cv::Scalar WhiteStoneTextColor = BlackStoneColor;  // 白石の上に描画する手番の数字の色 (BGR)

	const vec<PosStone>& History = board.GetHistory();  // 棋譜のデータ
	const autosize HistoryLength = History.size();  // 棋譜の長さ

	// 画像の初期化、背景を設定
	cv::Mat image(ImageSize, ImageSize, CV_8UC3, BoardColor);

	// 盤面の枠線を描画
	for (uint8 i = 0; i < LineCount; ++i)
	{
		// 画像端からの位置
		const uint16 pos = Margin + i * CellSize;

		// 横線の描画
		cv::line(image,
			cv::Point(Margin, pos),
			cv::Point(Margin + ImageBoardSize, pos),
			FrameColor, LineWidth);
		// 縦線の描画
		cv::line(image,
			cv::Point(pos, Margin),
			cv::Point(pos, Margin + ImageBoardSize),
			FrameColor, LineWidth);
	}

	// 星を描画
	const vec<Pos> StarPositions = GetStarPositions(board.GetBoardSize());
	for (const Pos& pos : StarPositions)
	{
		// 画像端からの位置
		const uint16 PosX = Margin + (pos.x - 1) * CellSize;
		const uint16 PosY = Margin + (pos.y - 1) * CellSize;
		cv::circle(image, cv::Point(PosX, PosY), StarRadius, StarColor, cv::FILLED, cv::LINE_AA);
	}

	// 最新の着手は、石を赤いアウトラインで囲む
	if (HistoryLength >= 1)
	{
		const PosStone& LatestPut = History.back();  // 最新の着手を取得

		// 空き点でない場合のみ
		if (LatestPut.stone != Stone::Empty)
		{
			// 画像端からの位置
			const uint16 PosX = Margin + (LatestPut.pos.x - 1) * CellSize;
			const uint16 PosY = Margin + (LatestPut.pos.y - 1) * CellSize;

			// 石のアウトラインを描画
			cv::circle(image, cv::Point(PosX, PosY), StoneRadius + (StoneMargin >> 1), StoneOutlineColor, cv::FILLED, cv::LINE_AA);
		}
	}

	// 石を描画
	// LineWidthの分、若干小さくする
	for (uint8 y = 1; y <= LineCount; ++y)
		for (uint8 x = 1; x <= LineCount; ++x)
		{
			const Stone stone = board.GetStone(x, y);

			// 空きマスは描画しない
			if (stone == Stone::Empty) continue;

			// 画像端からの位置
			const uint16 posX = Margin + (x - 1) * CellSize;
			const uint16 posY = Margin + (y - 1) * CellSize;

			// 最新の着手の石は、アウトラインの分、半径を小さくする
			// 棋譜から読み取るのではなく、石の座標で判断している点に注意!
			uint16 radius = StoneRadius;
			if (HistoryLength >= 1 && Pos(x, y) == History.back().pos)
				radius -= (StoneOutlineWidth - (StoneMargin >> 1));

			// 黒石
			if (stone == Stone::Black)
				cv::circle(image, cv::Point(posX, posY), radius, BlackStoneColor, cv::FILLED, cv::LINE_AA);
			// 白石
			else if (stone == Stone::White)
				cv::circle(image, cv::Point(posX, posY), radius, WhiteStoneColor, cv::FILLED, cv::LINE_AA);
		}

	// 棋譜に基づいて、石の上に手番の数字を描画する
	// この時、盤面のデータと棋譜のデータが一致している前提で処理を行う
	// 同じ場所に複数回着手してある可能性があるため、棋譜を後ろ向きに読み取っていき、既に描画されているならスキップする
	if (bWithHistory)
	{
		uset<Pos> drawnPositions;  // 描画済みの位置を記録するためのセット

		for (sautosize i = HistoryLength - 1; i >= 0; --i)
		{
			const PosStone& posStone = History[i];

			// (石が取られていたせいで) その位置が空き点になっている場合は、スキップ
			if (board.GetStone(posStone.pos) == Stone::Empty)
				continue;

			// 既にその位置に描画済みならスキップ
			if (drawnPositions.contains(posStone.pos))
				continue;
			// 初めての描画なので、もうこの位置に数字が描画されないように、セットに追加しておく
			else
				drawnPositions.insert(posStone.pos);

			// 画像端からの位置
			uint16 posX = Margin + (posStone.pos.x - 1) * CellSize;
			uint16 posY = Margin + (posStone.pos.y - 1) * CellSize;

			// 手番の数字の色を決定
			const Stone stone = posStone.stone;
			cv::Scalar textColor;
			if (stone == Stone::Black) textColor = BlackStoneTextColor;
			else if (stone == Stone::White) textColor = WhiteStoneTextColor;
			else continue;

			// 手番の数字を決定
			const str numberText = std::to_string(i + 1);

			{
				// APIの型と合わせる
				constexpr int FontFace = cv::FONT_HERSHEY_SIMPLEX;
				constexpr double FontScaleMultiplier = 0.03;  // フォントサイズを、石の半径の何倍にするか
				constexpr double ThicknessMultiplier = 0.06;  // フォントの太さを、石の半径の何倍にするか

				const double FontScale = StoneRadius * FontScaleMultiplier;  // フォントサイズ (px)
				const int Thickness = static_cast<int>(std::floor(StoneRadius * ThicknessMultiplier));  // フォントの太さ 切り捨て (px)

				// テキストの位置を決定 (中央揃え)
				const cv::Size TextSize = cv::getTextSize(numberText, FontFace, FontScale, Thickness, nullptr);
				posX -= (TextSize.width >> 1);
				posY += (TextSize.height >> 1);

				// テキストを描画
				cv::putText(image, numberText, cv::Point(posX, posY), FontFace, FontScale, textColor, Thickness, cv::LINE_AA);
			}
		}
	}

	return image;
}

cv::Mat ConvertGraphToPngImage(const vec<double>& winRates)
{
	// 左にメモリ、右にグラフを描画
	// メモリは 10 刻みで、下が 0.0、上が 1.0 (1.0 なら黒が優勢、0.0 なら白が優勢, 0.5 なら互角)

	constexpr uint16 Width = 640;  // 画像の幅 (px)
	constexpr uint16 Height = 480;  // 画像の高さ (px)
	constexpr uint16 MemoryWidth = 50;  // メモリの幅 (px)
	constexpr uint16 PointRadius = 4;  // 点の半径 (px)
	constexpr uint8 MemoryLineCount = 11;  // メモリのライン数 (奇数の想定)

	const autosize PointCount = winRates.size();  // 点の数
	const double PointsMargin = 1.0 * (Width - MemoryWidth) / (PointCount + 1);  // 点の間隔
	const double MemoriesMargin = 1.0 * Height / (MemoryLineCount + 1);  // メモリの間隔

	static const cv::Scalar BackgroundColor = { 240, 180, 170 };  // 背景色 (BGR)
	static const cv::Scalar BlackPointColor = { 0, 0, 0 };  // 点の色 (黒) (BGR)
	static const cv::Scalar WhitePointColor = { 255, 255, 255 };  // 点の色 (白) (BGR)
	static const cv::Scalar GeneralColor = { 82, 61, 58 };  // 汎用色 (BGR)

	// 画像の初期化、背景を設定
	cv::Mat image(Height, Width, CV_8UC3, BackgroundColor);

	// メモリを描画
	{
		constexpr uint8 BorderLineWidth = 1;  // 境界線の太さ (px)
		constexpr uint8 LineWidth = 1;  // ラインの太さ (px)
		constexpr uint8 LineLength = 20;  // 境界線より左部分の、ラインの長さ (px)
		constexpr uint8 CenterIndex = MemoryLineCount / 2;  // 中央のラインのインデックス
		constexpr uint16 LeftX = MemoryWidth - LineLength;  // ラインの左端の X 座標

		static const cv::Scalar LineAccentColor = { 64, 48, 232 };  // ラインのアクセント色 (BGR)

		// 境界線を描画
		cv::line(image, cv::Point(MemoryWidth, 0), cv::Point(MemoryWidth, Height), GeneralColor, BorderLineWidth, cv::LINE_AA);

		// ラインを描画
		for (uint8 i = 0; i < MemoryLineCount; ++i)
		{
			// 最初と中央、最後のラインは長くする
			const bool IsLong = (i == 0 || i == CenterIndex || i == MemoryLineCount - 1);

			// 位置を計算
			const uint16 RightX = IsLong ? Width : MemoryWidth;
			const uint16 Y = static_cast<uint16>(std::round(Math::RemapClamped(i, 0, MemoryLineCount - 1, Height - MemoriesMargin, MemoriesMargin)));

			// 色を決定
			// 中央のラインはアクセント色、それ以外は汎用色
			const cv::Scalar& LineColor = (i == CenterIndex) ? LineAccentColor : GeneralColor;

			// 各ラインを描画
			cv::line(image, cv::Point(LeftX, Y), cv::Point(RightX, Y), LineColor, LineWidth, cv::LINE_AA);
		}
	}

	// グラフを描画
	{
		constexpr uint8 PointsLineWidth = 1;  // 点を結ぶ線の太さ (px)

		// 点を結ぶために、最初に点の座標の計算だけ行う
		vec<cv::Point> pointPos;
		pointPos.reserve(PointCount);
		for (autosize i = 0; i < PointCount; ++i)
		{
			const bool IsBlack = !(i & 1);
			const double WinRate = IsBlack ? winRates[i] : 1.0 - winRates[i];  // 黒石の勝率はそのまま、白石の勝率は反転させる

			// 座標を計算し、保存する
			const uint16 X = static_cast<uint16>(std::round(MemoryWidth + (i + 1) * PointsMargin));
			const uint16 Y = static_cast<uint16>(std::round(Math::RemapClamped(WinRate, 0.0, 1.0, Height - MemoriesMargin, MemoriesMargin)));
			pointPos.emplace_back(X, Y);
		}

		// 点を結ぶ線を描画 (点が 2 つ以上ある場合のみ)
		if (PointCount >= 2)
		{
			for (autosize i = 0; i < PointCount - 1; ++i)
			{
				const cv::Point& Start = pointPos[i];
				const cv::Point& End = pointPos[i + 1];
				cv::line(image, Start, End, GeneralColor, PointsLineWidth, cv::LINE_AA);
			}
		}

		// 点を描画
		for (autosize i = 0; i < PointCount; ++i)
		{
			const bool IsBlack = !(i & 1);
			const cv::Scalar& PointColor = IsBlack ? BlackPointColor : WhitePointColor;
			const cv::Point& Pos = pointPos[i];

			// 各点を描画
			cv::circle(image, Pos, PointRadius, PointColor, cv::FILLED, cv::LINE_AA);
		}
	}

	return image;
}
