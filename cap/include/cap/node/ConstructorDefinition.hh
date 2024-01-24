#ifndef CAP_NODE_CONSTRUCTOR_DEFINITION_HH
#define CAP_NODE_CONSTRUCTOR_DEFINITION_HH

#include <cap/node/FunctionDefinition.hh>

namespace cap
{

class ConstructorDefinition : public FunctionDefinition
{
public:
	ConstructorDefinition(Token token) : FunctionDefinition(token)
	{
	}

	virtual bool isConstructor() override
	{
		return true;
	}

	const char* getTypeString() override
	{
		return "Constructor";
	}
};

}

#endif
