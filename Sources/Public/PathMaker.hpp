#pragma once

#include <Core.hpp>

class PathMaker final
{
public:

	inline PathMaker() = delete;

	// elements の情報を結合して、ユニークな識別子を生成する
	static str GetIdentifier(std::initializer_list<str> elements = {});

	// 現在の日時を含む、ユニークなパスを生成する (拡張子なし)
	static str CreateWithDatetime(const str& rawPath, const str& identifier = "");

	// 現在の日時を含む、ユニークなパスを生成する (拡張子なし)
	static str CreateWithDatetime(const str& rawPath, std::initializer_list<str> suffixes = {});
};
