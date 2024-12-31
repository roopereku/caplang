#ifndef CAP_VALUE_HH
#define CAP_VALUE_HH

#include <cap/Expression.hh>

namespace cap
{

class Value : public Expression
{
public:
	Value(std::wstring&& value);

	/// Should never be called as values cannot be the "current node".
	///
	/// \return False.
	bool isComplete() const override;

	const std::wstring& getValue();

	const char* getTypeString() override;

private:
	// TODO: Should this be a token?
	std::wstring value;
};

}

#endif
