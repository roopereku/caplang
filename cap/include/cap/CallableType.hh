#ifndef CAP_CALLABLE_TYPE_HH
#define CAP_CALLABLE_TYPE_HH

#include <cap/TypeDefinition.hh>

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
	std::shared_ptr <Expression::Root> getParameters();

	/// Gets the return type root.
	///
	/// \return The return type root.
	std::shared_ptr <Expression::Root> getReturnType();

	/// Initializes the parameter root.
	void initializeParameters();

	/// Initializes the return type root.
	void initializeReturnType();

	const char* getTypeString() override;

private:
	std::shared_ptr <Expression::Root> parameters;
	std::shared_ptr <Expression::Root> returnType;
};

}

#endif
