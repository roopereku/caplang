#ifndef CAP_VARIABLE_HH
#define CAP_VARIABLE_HH

#include <cap/Declaration.hh>

namespace cap
{

class BinaryOperator;

class Variable : public Declaration
{
public:
	Variable(std::weak_ptr <BinaryOperator> initialization);

	/// Validates the initialization of this variable.
	///
	/// \param validator The validator used for traversal.
	/// \return True if validation succeeded.
	bool validate(Validator& validator) override;

	const char* getTypeString() const override;

private:
	std::weak_ptr <BinaryOperator> initialization;
};

}

#endif
