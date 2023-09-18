#ifndef CAP_FUNCTION_DECLARATION_HH
#define CAP_FUNCTION_DECLARATION_HH

#include <cap/Node.hh>
#include <cap/Function.hh>

namespace cap
{

class FunctionDeclaration : public Node
{
public:
	FunctionDeclaration(Token&& token) : Node(std::move(token))
	{
	}

	bool isFunctionDeclaration() override
	{
		return true;
	}

	std::shared_ptr <Function> function;
};

}

#endif
