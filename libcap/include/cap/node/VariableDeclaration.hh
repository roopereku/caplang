#ifndef CAP_NODE_VARIABLE_DECLARATION_HH
#define CAP_NODE_VARIABLE_DECLARATION_HH

#include <cap/node/Declaration.hh>
#include <cap/node/ExpressionRoot.hh>

namespace cap
{

class VariableDeclaration : public Declaration
{
public:
	VariableDeclaration(Token&& token) : Declaration(std::move(token))
	{
	}

	bool isVariable() override
	{
		return true;
	}

	bool isParameter;
	std::shared_ptr <ExpressionRoot> initialization;
};

}

#endif
