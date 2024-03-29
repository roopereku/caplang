#ifndef CAP_NODE_OPERATOR_HH
#define CAP_NODE_OPERATOR_HH

#include <cap/Parser.hh>

#include <cap/node/Expression.hh>

namespace cap
{

class Validator;

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

	/// Validates this operator.
	///
	/// \param validator The validator to use.
	virtual bool validate(Validator& validator) = 0;

	/// Handles a value.
	///
	/// \param node The expression node to handle.
	/// \return True if succesful.
	virtual bool handleValue(std::shared_ptr <Expression>&& node) = 0;

	/// Manipulates AST hierarchy depending on operator precedence.
	///
	/// \param node The new expression node to add.
	/// \param parser The current parser.
	/// \return True if succesful.
	bool handleExpressionNode(std::shared_ptr <Expression> node, Parser& parser) final override;

	/// Steals the most recent value and calls handleValue with the given node.
	///
	/// \param node The node to pass into handleValue.
	void replaceMostRecentValue(std::shared_ptr <Expression> node) override;

	/// Determines if the operator is complete.
	///
	/// \return True if the operator is complete.
	virtual bool isComplete() = 0;

	const Type type;

protected:
};

}

#endif
