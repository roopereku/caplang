#include <cap/test/SourceString.hh>

namespace cap::test
{

SourceString::SourceString(std::wstring&& str)
	: str(std::move(str))
{
}

wchar_t SourceString::operator[](size_t index) const
{
	return str[index];
}

std::wstring SourceString::getString(cap::Token token) const
{
	auto offset = str.begin() + token.getIndex();
	return std::wstring(offset, offset + token.getLength());
}

bool SourceString::match(cap::Token token, std::wstring_view value) const
{
	return str.compare(token.getIndex(), token.getLength(), value) == 0;
}

}
