#ifndef CAP_NODE_TYPE_DECLARATION_HH
#define CAP_NODE_TYPE_DECLARATION_HH

#include <cap/node/Declaration.hh>
#include <cap/Type.hh>

namespace cap
{

class TypeDeclaration : public Declaration
{
public:
	TypeDeclaration(Token&& token) : Declaration(std::move(token))
	{
	}

	bool isType() override
	{
		return true;
	}

	std::shared_ptr <Type> type;
};

}

#endif
