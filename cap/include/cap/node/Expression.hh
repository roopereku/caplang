#ifndef CAP_NODE_EXPRESSION_HH
#define CAP_NODE_EXPRESSION_HH

#include <cap/Node.hh>
#include <cap/node/TypeDefinition.hh>

namespace cap
{

class Parser;

class Expression : public Node
{
public:
	enum class Type
	{
		Operator,
		Value,
	};

	Expression(Type type, Token token);

	//virtual TypeDefinition& getResultType();

	/// Custom handler for when an operator should replace what's currently stored.
	///
	/// \param The node that will replace the currently stored node.
	/// \return True if replacing succeeded.
	virtual bool replaceExpression(std::shared_ptr <Expression> node);

	/// Custom handler for when an existing expression node should be handled.
	///
	/// \param node The expression node that will be handled.
	/// \param parser The parser holding a state.
	/// \return True if handling succeeded.
	virtual bool handleExpressionNode(std::shared_ptr <Expression> node, Parser& parser);

	/// Sets the brace depth.
	///
	/// \param depth The brace depth.
	void setBraceDepth(size_t depth);

	/// Checks whether two expression nodes are on the same level (i.e. have the same brace depth)
	///
	/// \return True If the expressions are on the same level.
	bool isOnSameLevel(std::shared_ptr <Expression> expr);

	size_t getBraceDepth()
	{
		return braceDepth;
	}

	const Type type;

private:
	size_t braceDepth = 0;
};

}

#endif
