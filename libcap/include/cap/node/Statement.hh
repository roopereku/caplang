#ifndef CAP_NODE_STATEMENT_HH
#define CAP_NODE_STATEMENT_HH

#include <cap/Node.hh>

namespace cap
{

class Statement : public Node
{
public:
	Statement(Token&& token) : Node(std::move(token))
	{
	}

	bool isStatement() final override
	{
		return true;
	}

	virtual bool isReturn()
	{
		return false;
	}
};

}

#endif
