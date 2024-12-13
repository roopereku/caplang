#include <cap/test/DynamicSource.hh>

namespace cap::test
{

DynamicSource::DynamicSource()
{
}

DynamicSource::DynamicSource(std::wstring&& str)
	: str(std::move(str))
{
}

wchar_t DynamicSource::operator[](size_t index) const
{
	return str[index];
}

std::wstring DynamicSource::getString(cap::Token token) const
{
	auto offset = str.begin() + token.getIndex();
	return std::wstring(offset, offset + token.getLength());
}

bool DynamicSource::match(cap::Token token, std::wstring_view value) const
{
	return str.compare(token.getIndex(), token.getLength(), value) == 0;
}

void DynamicSource::operator+=(std::wstring&& value)
{
	str += std::move(value);
}

size_t DynamicSource::getLength() const
{
	return str.length();
}

}
