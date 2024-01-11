#ifndef CAP_NODE_FUNCTION_SIGNATURE_HH
#define CAP_NODE_FUNCTION_SIGNATURE_HH

#include <cap/node/FunctionDefinition.hh>
#include <cap/node/TypeDefinition.hh>

namespace cap
{

class FunctionSignature : public TypeDefinition
{
public:
	FunctionSignature(std::shared_ptr <FunctionDefinition> target);

	/// Gets the return type specified in this signature.
	///
	/// \return The return type specified in this signature.
	std::shared_ptr <TypeDefinition> getReturnType();

	friend class FunctionDefinition;

private:
	std::shared_ptr <TypeDefinition> returnType;
};

}

#endif
