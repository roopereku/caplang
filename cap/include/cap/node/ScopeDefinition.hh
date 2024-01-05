#ifndef CAP_NODE_SCOPE_DEFINITON_HH
#define CAP_NODE_SCOPE_DEFINITON_HH

#include <cap/Node.hh>
#include <cap/Token.hh>

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

	ScopeDefinition(Type type, Token name)
		: Node(Node::Type::ScopeDefinition), type(type), name(name)
	{
	}

	ScopeDefinition() : ScopeDefinition(Type::None, Token::createInvalid())
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

	const char* getTypeString() override
	{
		return "Scope";
	}

	const Type type;
	const Token name;

private:
	std::shared_ptr <Node> root;
};

}

#endif
