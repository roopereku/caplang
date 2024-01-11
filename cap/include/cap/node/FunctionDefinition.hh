#ifndef CAP_NODE_FUNCTION_DEFINITION_HH
#define CAP_NODE_FUNCTION_DEFINITION_HH

#include <cap/Validator.hh>

#include <cap/node/ScopeDefinition.hh>

namespace cap
{

class FunctionSignature;

class FunctionDefinition : public ScopeDefinition
{
public:
	FunctionDefinition(Token name);

	std::shared_ptr <FunctionSignature> getSignature();
	bool initializeSignature(Validator& validator);

	const char* getTypeString() override
	{
		return "FunctionDefinition";
	}

private:
	std::shared_ptr <FunctionSignature> signature;
};

}

#endif
