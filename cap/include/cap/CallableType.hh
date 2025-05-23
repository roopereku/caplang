#ifndef CAP_CALLABLE_TYPE_HH
#define CAP_CALLABLE_TYPE_HH

#include <cap/TypeDefinition.hh>
#include <cap/ArgumentAccessor.hh>
#include <cap/Variable.hh>

namespace cap
{

// TODO: Rename to Signature? Does it have enough context without "function"?

/// Represents a callable type such as "func(int, string) -> bool".
/// The signature is owned by this CallableType.
class CallableType : public TypeDefinition
{
public:
	CallableType();

	/// Gets the parameter root.
	///
	/// \return The parameter root.
	std::shared_ptr <Variable::Root> getParameterRoot() const;

	/// Gets the return type root.
	///
	/// \return The return type root.
	std::shared_ptr <Expression::Root> getReturnTypeRoot() const;

	/// Initializes the parameter root.
	void initializeParameters();

	/// Initializes the return type root.
	void initializeReturnType();

	/// Matches the parameters within this callable type against others.
	///
	/// \param arguments The arguments to match against.
	/// \return Whether the parameters are compatible and how many are unidentical.
	std::pair <bool, size_t> matchParameters(ArgumentAccessor&& arguments) const;

	/// Validates the return type and parameters of this callable type.
	///
	/// \param validator The validator used for traversal.
	/// \return True if validation succeeded.
	bool validate(Validator& validator) override;

	const char* getTypeString() const override;

private:
	// TODO: Function should be the one holding the variable root.
	std::shared_ptr <Variable::Root> parameters;
	std::shared_ptr <Expression::Root> returnType;
};

}

#endif
