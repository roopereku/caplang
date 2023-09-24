#ifndef CAP_FUNCTION_DECLARATION_HH
#define CAP_FUNCTION_DECLARATION_HH

#include <cap/node/Declaration.hh>
#include <cap/Function.hh>

namespace cap
{

class FunctionDeclaration : public Declaration
{
public:
	FunctionDeclaration(Token&& token) : Declaration(std::move(token))
	{
	}

	bool isFunction() override
	{
		return true;
	}

	std::shared_ptr <Function> function;
};

}

#endif
