#ifndef CAP_TEST_SOURCE_STRING_HH
#define CAP_TEST_SOURCE_STRING_HH

#include <cap/Source.hh>

namespace cap::test
{

class SourceString : public cap::Source
{
public:
	SourceString(std::wstring&& str);
	wchar_t operator[](size_t index) const override;
	std::wstring getString(cap::Token token) const override;
	bool match(cap::Token token, std::wstring_view value) const override;

private:
	std::wstring str;
};

}

#endif
