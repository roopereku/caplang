#ifndef CAP_VARIABLE_HH
#define CAP_VARIABLE_HH

#include <cap/Declaration.hh>

namespace cap
{

class BinaryOperator;
class Value;

class Variable : public Declaration
{
public:
	enum class Type
	{
		Local,
		Generic,
		Parameter,
	};

	Variable(Type type, std::weak_ptr <BinaryOperator> initialization);

	/// Validates the initialization of this variable.
	///
	/// \param validator The validator used for traversal.
	/// \return True if validation succeeded.
	bool validate(Validator& validator) override;

	/// Gets the right side of the initializing binary operator.
	///
	/// \return The expression initializing this variable.
	std::shared_ptr <Expression> getInitialization();

	static const char* getTypeString(Type type);
	const char* getTypeString() const override;

	class Root;

private:
	Type type;
	std::weak_ptr <BinaryOperator> initialization;
};

class Variable::Root : public Expression::Root
{
public:
	Root(Variable::Type type);

	/// Gets the type of this declaration root.
	///
	/// \return The type of this declaration root.
	Variable::Type getType() const;

	const char* getTypeString() const override;

private:
	Variable::Type type;
};

}

#endif
