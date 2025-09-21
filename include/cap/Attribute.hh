#ifndef CAP_ATTRIBUTE_HH
#define CAP_ATTRIBUTE_HH

#include <cap/Expression.hh>

namespace cap
{

class Attribute : public Expression::Root
{
public:
	Attribute();

	unsigned getPrecedence() override;

	const char* getTypeString() const override;
};

}

#endif
