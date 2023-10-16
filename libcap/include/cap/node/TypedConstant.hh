#ifndef CAP_NODE_TYPED_CONSTANT_HH
#define CAP_NODE_TYPED_CONSTANT_HH

#include <cap/node/Expression.hh>
#include <cap/node/Value.hh>
#include <cap/Type.hh>

namespace cap
{

class TypedConstant : public Expression
{
public:
	TypedConstant(std::shared_ptr <Value> value) :
		Expression(Token(value->getToken())), type(Type::getPrimitive(value->getToken().getType()))
	{
	}

	Type& getResultType() override;

	bool isTypedConstant() final override
	{
		return true;
	}

private:
	PrimitiveType& type;
};

}

#endif
