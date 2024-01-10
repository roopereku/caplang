#ifndef CAP_NODE_EXPLICIT_RETURN_TYPE_HH
#define CAP_NODE_EXPLICIT_RETURN_TYPE_HH

#include <cap/node/ExpressionRoot.hh>

namespace cap
{

/// ExplicitReturnType holds the expression for the explicit
/// return type of a function.
class ExplicitReturnType : public ExpressionRoot
{
public:
	ExplicitReturnType(Token& token)
		: ExpressionRoot(ExpressionRoot::Type::ExplicitReturnType, token)
	{
	}

	const char* getTypeString() override
	{
		return "Explicit return type";
	}
};

}

#endif
