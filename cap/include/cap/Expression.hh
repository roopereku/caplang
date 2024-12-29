#ifndef CAP_EXPRESSION_HH
#define CAP_EXPRESSION_HH

#include <cap/Node.hh>

namespace cap
{

class Expression : public Node
{
public:
	enum class Type
	{
		Root,
		Value,
		UnaryOperator,
		BinaryOperator,
		BracketOperator
	};

	/// Parses an expression.
	///
	/// \param ctx The parser context.
	/// \param token The token to handle.
	/// \return This node or the parent.
	std::weak_ptr <Node> handleToken(Node::ParserContext& ctx, Token& token) final override;

	/// Implementation defined check that indicates whether this node is complete.
	///
	/// \return True if this node is completed.
	virtual bool isComplete() const = 0;

	/// Implementation defined behaviour for how values are handled.
	///
	/// \param node The expression node to handle as a value.
	virtual void handleValue(std::shared_ptr <Expression> node);

	/// Checks whether this node is the root of an expression.
	/// Defaults to false.
	///
	/// \return True if this node is an expression root.
	virtual bool isRoot() const;

	/// Gets the precedence of this expression node.
	/// Defaults to 0.
	///
	/// \return The precedence of this expression node.
	virtual unsigned getPrecedence();

	/// Gets the type of this expression node.
	///
	/// \return The type of this expression node.
	Type getType();

	static const char* getTypeString(Type type);
	const char* getTypeString();

	class Root;

protected:
	Expression(Type type);

	std::weak_ptr <Node> exitCurrentExpression(bool recursive);
	std::weak_ptr <Node> adoptValue(std::shared_ptr <Expression> node);

	virtual std::shared_ptr <Expression> stealLatestValue();

private:
	Type type;
};

class Expression::Root : public Expression
{
public:
	Root();

	/// Checks whether the first expression node exists.
	///
	/// \return True if the first expression node exists.
	bool isComplete() const override;

	void handleValue(std::shared_ptr <Expression> node) override;

	/// Gets the first node of this expression.
	///
	/// \return The first node of this expression.
	std::shared_ptr <Expression> getFirst();

protected:
	std::shared_ptr <Expression> stealLatestValue() override;

	std::shared_ptr <Expression> first;
};

}

#endif
