#ifndef CAP_NODE_SCOPE_DEFINITON_HH
#define CAP_NODE_SCOPE_DEFINITON_HH

#include <cap/Node.hh>

namespace cap
{

class ScopeDefinition : public Node
{
public:
	enum class Type
	{
		FunctionDefinition,
		TypeDefinition,
		None
	};

	ScopeDefinition(Type type) : Node(Node::Type::ScopeDefinition), type(type)
	{
	}

	ScopeDefinition() : ScopeDefinition(Type::None)
	{
	}

	std::shared_ptr <Node> getRoot() const
	{
		return root;
	}

	void initializeRoot(std::shared_ptr <Node>&& node)
	{
		if(!root)
		{
			root = std::move(node);	
		}
	}

	const Type type;

private:
	std::shared_ptr <Node> root;
};

}

#endif
