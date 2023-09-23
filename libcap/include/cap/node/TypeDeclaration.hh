#ifndef CAP_NODE_TYPE_DECLARATION_HH
#define CAP_NODE_TYPE_DECLARATION_HH

#include <cap/Node.hh>
#include <cap/Type.hh>

namespace cap
{

class TypeDeclaration : public Node
{
public:
	TypeDeclaration(Token&& token) : Node(std::move(token))
	{
	}

	bool isTypeDeclaration() override
	{
		return true;
	}

	std::shared_ptr <Type> type;
};

}

#endif
