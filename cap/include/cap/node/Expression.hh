#ifndef CAP_NODE_EXPRESSION_HH
#define CAP_NODE_EXPRESSION_HH

#include <cap/Node.hh>
#include <cap/Reference.hh>
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
		Root
	};

	Expression(Type type, Token token);

	/// Custom handler for when an existing expression node should be handled.
	///
	/// \param node The expression node that will be handled.
	/// \param parser The parser holding a state.
	/// \return True if handling succeeded.
	virtual bool handleExpressionNode(std::shared_ptr <Expression> node, Parser& parser);

	/// Steals the most recently added value. The held value will be null after.
	///
	/// \return The most recent value stolen.
	virtual std::shared_ptr <Expression> stealMostRecentValue();

	/// Replaces the most recently added value.
	///
	/// \param The node to replace the most recent value with.
	virtual void replaceMostRecentValue(std::shared_ptr <Expression> node);

	virtual const char* getTypeString() override = 0;

	/// Sets the reference of this expression node.
	///
	/// \param reference What to refer to.
	void setReference(Reference ref);

	/// Gets the entity that this node refers to.
	///
	/// \return The entity that this node refers to.
	Reference getReference();

	/// Sets the result type.
	///
	/// \param node The node containing the result type.
	void setResultType(std::shared_ptr <TypeDefinition> node);

	/// Gets the result type.
	///
	/// \return The result type.
	std::weak_ptr <TypeDefinition> getResultType();

	const Type type;

private:
	std::weak_ptr <TypeDefinition> resultType;
	Reference reference;
};

}

#endif
