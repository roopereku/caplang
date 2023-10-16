#ifndef CAP_NODE_VARIABLE_REFERENCE_HH
#define CAP_NODE_VARIABLE_REFERENCE_HH

#include <cap/Node.hh>
#include <cap/Variable.hh>

namespace cap
{

class VariableReference : public Node
{
public:
	VariableReference(Token&& token, Variable& referred)
		: Node(std::move(token)), referred(referred)
	{
	}

private:
	Variable& referred;
};

}

#endif
