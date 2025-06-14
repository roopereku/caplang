#ifndef CAP_INTEGER_HH
#define CAP_INTEGER_HH

#include <cap/Value.hh>

namespace cap
{

class Integer : public Value
{
public:
	Integer(std::wstring&& value);

	const char* getTypeString() const override;

private:
	uint64_t value;
};

}

#endif
