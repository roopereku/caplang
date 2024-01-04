#ifndef CAP_NODE_EXPRESSION_ROOT_HH
#define CAP_NODE_EXPRESSION_ROOT_HH

#include <cap/node/Expression.hh>

namespace cap
{

/// ExpressionRoot is an expression node that begins an expression.
class ExpressionRoot : public Expression
{
public:
	enum class Type
	{
		VariableDefinition,
		Expression
	};

	ExpressionRoot(Type type, Token& token)
		: Expression(Expression::Type::Root, token), type(type)
	{
	}

	ExpressionRoot(Token& token)
		: Expression(Expression::Type::Root, token), type(Type::Expression)
	{
	}

	/// Gets the root expression node.
	///
	/// \return The root expression node.
	std::shared_ptr <Expression> getRoot();

	/// Sets the given expression node as the root node and makes it the current node.
	///
	/// \param node The node to set as the expression root.
	/// \parser The current parser.
	/// \return True if succesful.
	bool handleExpressionNode(std::shared_ptr <Expression> node, Parser& parser) override;

	const char* getTypeString() override;

	const Type type;

private:
	void setRoot(std::shared_ptr <Expression>&& node, Parser& parser);

	std::shared_ptr <Expression> root;
};

}

#endif
