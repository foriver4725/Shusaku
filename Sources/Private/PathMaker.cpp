#include <PathMaker.hpp>

using namespace Shusaku;

str PathMaker::GetIdentifier(std::initializer_list<str> elements)
{
	std::ostringstream oss;

	for (bool first = true; const auto& element : elements)
	{
		// 要素の間にアンダースコアを挿入して、識別子を作成する
		if (first) first = false;
		else oss << "_";

		oss << element;
	}

	return oss.str();
}

str PathMaker::CreateWithDatetime(const str& rawPath, const str& identifier)
{
	auto now = std::chrono::system_clock::now();
	std::time_t nowTime = std::chrono::system_clock::to_time_t(now);

	std::tm localTm{};
#ifdef _WIN32
	localtime_s(&localTm, &nowTime);
#else
	localtime_r(&now_time, &local_tm);
#endif

	std::ostringstream oss;
	oss << rawPath << "_";
	oss << std::put_time(&localTm, "%Y-%m-%d_%H-%M-%S") << "_";
	oss << identifier;

	return oss.str();
}

str PathMaker::CreateWithDatetime(const str& rawPath, std::initializer_list<str> suffixes)
{
	return CreateWithDatetime(rawPath, GetIdentifier(suffixes));
}
