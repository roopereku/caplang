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

	virtual bool applyCachedValue(std::shared_ptr <Expression>&& cached) = 0;

	virtual const char* getTypeString() = 0;
	virtual unsigned getPrecedence() = 0;

	//TypeDefinition& getResultType() override
	//{
	//	return resultType ? *resultType : Expression::getResultType();
	//}

	//void setResultType(Type& type)
	//{
	//	resultType = &type;
	//}

	bool handleExpressionNode(std::shared_ptr <Expression> node, Parser& parser) override;

	const Type type;

protected:
	virtual bool handleHigherPrecedence(std::shared_ptr <Operator> op) = 0;
	virtual bool handleSamePrecedence(std::shared_ptr <Operator> op) = 0;
	virtual bool handleValue(std::shared_ptr <Expression> value) = 0;

	//Type* resultType = nullptr;
};

}

#endif
