#ifndef CAP_NODE_VARIABLE_DECLARATION_HH
#define CAP_NODE_VARIABLE_DECLARATION_HH

#include <cap/node/Expression.hh>

namespace cap
{

class VariableDeclaration : public Expression
{
public:
	VariableDeclaration(Token&& token) : Expression(std::move(token))
	{
		printf("Create VariableDeclaration %s\n", getToken().c_str());
	}

	bool handleToken(Token&& token, ParserState& state) override;

	bool isVariableDeclaration() override
	{
		return true;
	}

private:
	std::shared_ptr <Expression> initialization;
};

}

#endif
