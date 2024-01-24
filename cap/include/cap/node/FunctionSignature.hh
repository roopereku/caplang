#ifndef CAP_NODE_FUNCTION_SIGNATURE_HH
#define CAP_NODE_FUNCTION_SIGNATURE_HH

#include <cap/node/ParameterDefinition.hh>
#include <cap/node/FunctionDefinition.hh>
#include <cap/node/TypeDefinition.hh>

namespace cap
{

class Validator;

class FunctionSignature : public TypeDefinition
{
public:
	FunctionSignature(std::shared_ptr <FunctionDefinition> target);

	/// Gets the return type specified in this signature.
	///
	/// \return The return type specified in this signature.
	std::shared_ptr <TypeDefinition> getReturnType();

	/// Sets the the return type of this signature.
	///
	/// \param node The return type.
	/// \param validator The current validator.
	/// \return True if the return type was set succesfully.
	bool setReturnType(std::shared_ptr <TypeDefinition> node, Validator& validator);

	/// Gets the parameter count.
	///
	/// \return The count of parameters.
	unsigned getParameterCount();

	/// Gets the parameter at the given index.
	///
	/// \param index The index of the parameter.
	/// \return The parameter or nullptr.
	std::shared_ptr <ParameterDefinition> getParameter(unsigned index);

	friend class FunctionDefinition;

private:
	std::weak_ptr <FunctionDefinition> target;
	std::shared_ptr <TypeDefinition> returnType;

	bool returnTypeIsDefault = true;
	unsigned parameterCount = 0;
};

}

#endif
