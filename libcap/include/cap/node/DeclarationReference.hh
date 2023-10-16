#ifndef CAP_NODE_DECLARATION_REFERENCE_HH
#define CAP_NODE_DECLARATION_REFERENCE_HH

#include <cap/node/Expression.hh>
#include <cap/node/Declaration.hh>

namespace cap
{

class DeclarationReference : public Expression
{
public:
	DeclarationReference(std::shared_ptr <Declaration> decl) :
		Expression(Token(decl->getToken())), declaration(std::move(decl))
	{
	}

	std::shared_ptr <Declaration> getDeclaration()
	{
		return declaration;
	}

	bool isDeclarationReference() final override
	{
		return true;
	}

	Type& getResultType() override;

private:
	std::shared_ptr <Declaration> declaration;
};

}

#endif
