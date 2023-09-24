#ifndef CAP_NODE_DECLARATION_HH
#define CAP_NODE_DECLARATION_HH

#include <cap/Node.hh>

namespace cap
{

class Declaration : public Node
{
public:
	Declaration(Token&& token) : Node(std::move(token))
	{
	}

	bool isDeclaration() final override
	{
		return true;
	}

	virtual bool isVariable()
	{
		return false;
	}

	virtual bool isFunction()
	{
		return false;
	}

	virtual bool isType()
	{
		return false;
	}
};

}

#endif
