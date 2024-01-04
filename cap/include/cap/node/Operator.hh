#ifndef CAP_NODE_OPERATOR_HH
#define CAP_NODE_OPERATOR_HH

#include <cap/Parser.hh>

#include <cap/node/Expression.hh>

namespace cap
{

class Operator : public Expression
{
public:
	enum class Type
	{
		OneSided,
		TwoSided
	};

	Operator(Type type, Token token)
		: Expression(Expression::Type::Operator, token), type(type)
	{
	}

	virtual const char* getTypeString() override = 0;
	virtual unsigned getPrecedence() = 0;

	virtual bool handleValue(std::shared_ptr <Expression>&& value) = 0;

	bool handleExpressionNode(std::shared_ptr <Expression> node, Parser& parser) final override;

	const Type type;

protected:
};

}

#endif
