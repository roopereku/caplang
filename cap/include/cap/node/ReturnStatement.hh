#ifndef CAP_NODE_RETURN_STATEMENT_HH
#define CAP_NODE_RETURN_STATEMENT_HH

#include <cap/node/ExpressionRoot.hh>

namespace cap
{

class ReturnStatement : public ExpressionRoot
{
public:
	ReturnStatement(Token& at)
		: ExpressionRoot(ExpressionRoot::Type::ReturnStatement, at)
	{
	}

	const char* getTypeString() override
	{
		return "Return";
	}
};

}

#endif
