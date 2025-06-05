#pragma once

#include <vector>
#include "TypeAlias.hpp"
#include "BoardSizeEnum.hpp"
#include "StoneEnum.hpp"
#include "PosStone.hpp"

namespace Shusaku
{
	// TODO: 特定の盤面に対しても処理を行えるように、拡張したい
	class Board final
	{
	public:

		inline Board() = delete;

		inline static Board Create9x9() { return Board(BoardSize::_9x9); }
		inline static Board Create13x13() { return Board(BoardSize::_13x13); }
		inline static Board Create19x19() { return Board(BoardSize::_19x19); }

		// 左上角が (1, 1), 右下角が (size, size) の座標系で石を取得する
		inline Stone GetStone(uint8 x, uint8 y) const
		{
			return Get(x - 1, y - 1);
		}

		// 左上角が (1, 1), 右下角が (size, size) の座標系で石を置く
		// 石を置けるなら true を、置けないなら false を返す
		// 石を置ける時、棋譜にも追加する (黒白交互であるかは気にしない)
		// 2手前までの盤面を保存し、同形反復になるなら着手できない. ただし、長手数のコウ・同形反復は、チェックしない
		// コンピュータ同士の自動対戦を行うときは、手数の上限を設けるよう、強く推奨する
		inline bool PutStone(uint8 x, uint8 y, Stone stone)
		{
			// 空き点でなかったら、着手できない
			if (Get(x - 1, y - 1) != Stone::Empty)
				return false;

			// 試しに着手してみる! この後この着手が無効となったら、元に戻す
			Set(x - 1, y - 1, stone);

			// 周囲の座標を取得 (2-4個)
			uset<Pos> surroundings = GetSurroundingPositions({ x, y });

			// 相手の石に囲まれているか?
			bool surrounded = CheckSurrounded({ x, y });

			// 相手の石を取ることが出来るか?
			bool canTake = false;  // 最終フラグ
			uset<Pos> takenStones;
			takenStones.reserve(positionsCount);
			for (const Pos& pos : surroundings)
			{
				uset<Pos> takenStonesEach;
				bool canTakeEach = CheckCanTake(stone, pos, takenStonesEach);
				if (canTakeEach)
				{
					canTake = true;
					takenStones.insert(takenStonesEach.begin(), takenStonesEach.end());
				}
			}

			// 着手禁止点には打てない
			// (着手禁止点でも、相手の石を取れるなら打てる)
			if (surrounded && !canTake)
			{
				// 試しに着手していたものを、元に戻す
				Set(x - 1, y - 1, Stone::Empty);
				return false;
			}

			// 着手できる

			// 相手の石を取ることが出来るなら、取る
			if (canTake)
			{
				for (const Pos& pos : takenStones)
					Set(pos.x - 1, pos.y - 1, Stone::Empty);

				// アゲハマを増やす
				const autosize hamaCount = takenStones.size();
				if (stone == Stone::Black) hamaBlack += hamaCount;
				else if (stone == Stone::White) hamaWhite += hamaCount;
			}

			// 同形反復なら、やっぱり着手できない
			// 3手目より前なら、そもそもこのチェックは必要ないのでスキップ
			// 1手前の盤面と比較する必要はないが、念のため比較する
			if (history.size() >= 2 && (board == boardPre2 || board == boardPre1))
			{
				// 試しに着手していたものを、元に戻す
				// 石を取るなど盤面がガッツリ変わっていることが想定されるので、
				// 1手前の盤面のデータ (この時点では、石を置いてみる前の盤面の状態と一致している) で上書きして戻す
				// ただし、コピーコストが高め.
				board = boardPre1;
				return false;
			}

			// 棋譜に追加する
			history.emplace_back(PosStone{ { x, y }, stone });

			// 盤面を保存する (1,2 手前の盤面を保存)
			if (history.size() >= 2)
			{
				boardPre2 = boardPre1;  // 2手前の盤面を保存 (先ほどまで、1手前だったもの)
				boardPre1 = board;  // 1手前の盤面を保存 (先ほどまで、現在の盤面だったもの)
			}
			else if (history.size() == 1)
			{
				boardPre1 = board;  // 1手目なら、1手前の盤面を保存 (先ほどまで、現在の盤面だったもの)
			}

			return true;
		}

		// 左上角が (1, 1), 右下角が (size, size) の座標系で石を取得する
		inline Stone GetStone(const Pos& pos) const { return GetStone(pos.x, pos.y); }

		// 左上角が (1, 1), 右下角が (size, size) の座標系で石を置く
		// 石を置けるなら true を、置けないなら false を返す
		// 石を置ける時、棋譜にも追加する (黒白交互であるかは気にしない)
		// 2手前までの盤面を保存し、同形反復になるなら着手できない. ただし、長手数のコウ・同形反復は、チェックしない
		// コンピュータ同士の自動対戦を行うときは、手数の上限を設けるよう、強く推奨する
		inline bool PutStone(const Pos& pos, Stone stone) { return PutStone(pos.x, pos.y, stone); }

		// 盤面を空に戻し、棋譜もクリアする.
		inline void Clear()
		{
			hamaBlack = 0;
			hamaWhite = 0;

			std::fill(board.begin(), board.end(), Stone::Empty);
			boardPre1.clear();
			boardPre2.clear();

			history.clear();
		}

		inline uint8 GetSize() const { return size; }
		inline BoardSize GetBoardSize() const { return boardSize; }
		inline uint16 GetPositionsCount() const { return positionsCount; }
		inline uint64 GetHamaBlack() const { return hamaBlack; }
		inline uint64 GetHamaWhite() const { return hamaWhite; }
		inline const vec<Stone>& GetBoard() const { return board; }
		inline const vec<PosStone>& GetHistory() const { return history; }

	private:

		uint8 size;
		uint16 positionsCount;  // 盤面の交点数 (size * size)
		BoardSize boardSize;

		uint64 hamaBlack = 0;  // 黒が取ったアゲハマ (白石) の数
		uint64 hamaWhite = 0;  // 白が取ったアゲハマ (黒石) の数

		vec<Stone> board;
		// 棋譜 (黒 → 白 → 黒 → ... の順番で置かれた座標の履歴)
		// 左上角が (1, 1), 右下角が (size, size) の座標系
		vec<PosStone> history;

		// 1,2 手前の盤面を保存するための変数
		vec<Stone> boardPre1;
		vec<Stone> boardPre2;

		inline Board(BoardSize boardSize)
		{
			uint8 size = 0;
			if (boardSize == BoardSize::_9x9) size = 9;
			else if (boardSize == BoardSize::_13x13) size = 13;
			else if (boardSize == BoardSize::_19x19) size = 19;
			else throw std::invalid_argument("Invalid board size.");

			this->size = size;
			this->positionsCount = size * size;
			this->boardSize = boardSize;

			this->board.resize(positionsCount, Stone::Empty);
			this->history.reserve(static_cast<autosize>(positionsCount) << 2);  // 同形反復があるので、一応4倍程度の容量を確保しておく
		}

		// 左上角が (0, 0), 右下角が (size - 1, size - 1) の座標系で石を取得する
		// 内部処理用 (シンプルに盤面だけにアクセス)
		inline Stone Get(uint8 x, uint8 y) const
		{
			int32 idx = GetIndex({ x, y });
			return idx != -1 ? board[idx] : Stone::Empty;  // 範囲外なら空石を返す
		}

		// 左上角が (0, 0), 右下角が (size - 1, size - 1) の座標系で石を置く
		// 内部処理用 (シンプルに盤面だけにアクセス)
		inline void Set(uint8 x, uint8 y, Stone stone)
		{
			int32 idx = GetIndex({ x, y });
			if (idx == -1) return;  // 範囲外なら何もしない
			board[idx] = stone;
		}

		// 盤面の座標から、配列のインデックスを取得する
		// 左上角が (0, 0), 右下角が (size - 1, size - 1) の座標系
		inline int32 GetIndex(const Pos& pos) const
		{
			// 範囲外なら無効なインデックスを返す
			if (pos.x < 0 || size <= pos.x) return -1;
			if (pos.y < 0 || size <= pos.y) return -1;

			return pos.x + pos.y * size;
		}

		// pos の周囲の座標を取得する
		// 左上角が (1, 1), 右下角が (size, size) の座標系
		inline uset<Pos> GetSurroundingPositions(const Pos& pos) const
		{
			uset<Pos> out;
			out.reserve(4);

			if (pos.y > 1) out.insert(Pos(pos.x, pos.y - 1));  // 上
			if (pos.y < size) out.insert(Pos(pos.x, pos.y + 1));  // 下
			if (pos.x > 1) out.insert(Pos(pos.x - 1, pos.y));  // 左
			if (pos.x < size) out.insert(Pos(pos.x + 1, pos.y));  // 右

			return out;
		}

		// groupElementPos とつながっている石を探索する
		// groupPos にはつながっている石の座標を、surroundingPos にはその周囲の座標を格納する
		// 左上角が (1, 1), 右下角が (size, size) の座標系
		inline void GetGroup(const Pos& groupElementPos, uset<Pos>& outGroupPos, uset<Pos>& outSurroundingPos) const
		{
			// このグループの石の種類
			Stone groupStone = Get(groupElementPos.x - 1, groupElementPos.y - 1);

			outGroupPos.clear();
			outSurroundingPos.clear();

			{
				// 今探索中の座標
				que<Pos> q;
				q.push(groupElementPos);

				// 全ての探索が終了するまで繰り返す
				while (!q.empty())
				{
					// 探索するべき座標を取得し、キューから削除
					Pos pos = q.front();
					q.pop();

					// 既に探索済みならスキップ
					if (outGroupPos.contains(pos)) continue;

					// つながっている石だと判明

					// グループに追加
					outGroupPos.insert(pos);

					// 周囲の座標を取得し、次探索に情報を引き継ぐ
					{
						uset<Pos> surroundingPos = GetSurroundingPositions(pos);

						for (const Pos& p : surroundingPos)
						{
							// 探索済みならスキップ
							if (outGroupPos.contains(p)) continue;

							// 今見ている、周囲の石の種類
							Stone stone = Get(p.x - 1, p.y - 1);

							// グループの石なら、キューに追加
							if (stone == groupStone) q.push(p);
							// そうでないなら、グループを囲む点であるので、追加
							else outSurroundingPos.insert(p);
						}
					}
				}
			}
		}

		// elementPos にある石・及びそれとつながっている石のグループについて、相手の石に囲まれているかどうかをチェックする
		// 囲まれているなら true を、囲まれていないなら false を返す
		// 左上角が (1, 1), 右下角が (size, size) の座標系
		inline bool CheckSurrounded(const Pos& elementPos) const
		{
			const Stone GroupStone = Get(elementPos.x - 1, elementPos.y - 1);
			if (GroupStone == Stone::Empty) return false;  // 空き点が指定されたら判定不可
			const Stone OppoStone = ReverseStone(GroupStone);

			uset<Pos> _, surroundingPos;
			GetGroup(elementPos, _, surroundingPos);

			for (const Pos& pos : surroundingPos)
			{
				if (Get(pos.x - 1, pos.y - 1) != OppoStone)
					return false;
			}
			return true;
		}

		// stone が targetPos にある石を、取れるかどうかをチェックする
		// 取れるなら true を、取れないなら false を返す
		// その際、取ることができる相手の石の座標を outTakenStones に格納する
		// 左上角が (1, 1), 右下角が (size, size) の座標系
		inline bool CheckCanTake(Stone stone, const Pos& targetPos, uset<Pos>& outTakenStones) const
		{
			outTakenStones.clear();

			if (Get(targetPos.x - 1, targetPos.y - 1) != ReverseStone(stone))
				return false;  // 対象の位置に相手の石がないなら取れない

			// 対象の石とつながっている石を探索
			uset<Pos> groupPos, surroundingPos;
			GetGroup(targetPos, groupPos, surroundingPos);

			// 対象の石とつながっている石の周囲が、全て自分の石で囲まれているなら取れる
			for (const Pos& pos : surroundingPos)
			{
				if (Get(pos.x - 1, pos.y - 1) != stone)
					return false;
			}

			// 相手の石を取ることができる

			outTakenStones = groupPos;
			return true;
		}
	};
}
