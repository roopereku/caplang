#ifndef CAP_VALUE_HH
#define CAP_VALUE_HH

#include <cap/Expression.hh>

namespace cap
{

class Value : public Expression
{
public:
	Value() : Expression(Type::Value)
	{
	}

	/// Should never be called as values cannot be the "current node".
	///
	/// \return False.
	bool isComplete() const override;
};

}

#endif
