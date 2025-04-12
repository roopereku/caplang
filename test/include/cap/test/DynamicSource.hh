#ifndef CAP_TEST_DYNAMIC_SOURCE_HH
#define CAP_TEST_DYNAMIC_SOURCE_HH

#include <cap/Source.hh>

namespace cap::test
{

class DynamicSource : public cap::Source
{
public:
	DynamicSource();
	DynamicSource(std::wstring&& str);
	wchar_t operator[](size_t index) const override;
	std::wstring getString(cap::Token token) const override;
	bool match(cap::Token token, std::wstring_view value) const override;

	void operator+=(std::wstring&& value);
	void operator+=(const std::wstring& value);
	size_t getLength() const;

private:
	std::wstring str;
};

}

#endif
