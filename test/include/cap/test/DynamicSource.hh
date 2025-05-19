#ifndef CAP_TEST_DYNAMIC_SOURCE_HH
#define CAP_TEST_DYNAMIC_SOURCE_HH

#include <cap/Source.hh>

namespace cap::test
{

class DynamicSource : public cap::Source
{
public:
	DynamicSource();
	DynamicSource(std::wstring&& src);

	void operator+=(std::wstring&& value);
	void operator+=(const std::wstring& value);
	size_t getLength() const;
};

}

#endif
