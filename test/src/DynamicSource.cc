#include <cap/test/DynamicSource.hh>

namespace cap::test
{

DynamicSource::DynamicSource() :
    cap::Source(L"")
{
}

DynamicSource::DynamicSource(std::wstring&& src) :
    cap::Source(std::move(src))
{
}

void DynamicSource::operator+=(std::wstring&& value)
{
    m_src += std::move(value);
}

void DynamicSource::operator+=(const std::wstring& value)
{
    m_src += value;
}

size_t DynamicSource::getLength() const
{
    return m_src.length();
}

} // namespace cap::test
