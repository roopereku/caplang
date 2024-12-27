#include <cap/Expression.hh>
#include <cap/BinaryOperator.hh>
#include <cap/Source.hh>
#include <cap/Value.hh>

#include <cassert>

namespace cap
{

std::weak_ptr <Node> Expression::handleToken(Node::ParserContext& ctx, Token& token)
{
	// handleToken should never be called for a value.
	assert(type != Type::Value);

	printf("(%s) Expr token '%ls'\n", getTypeString(), ctx.source.getString(token).c_str());

	std::weak_ptr <Node> newCurrent = weak_from_this();
	std::shared_ptr <Expression> newNode;
	bool complete = isComplete();

	// TODO: Handle opening brackets.

	if(token.getType() == Token::Type::Operator)
	{
		// TODO: If the current node is a binary operator, expect an unary operator.
		newNode = BinaryOperator::create(ctx, token);
	}
	
	// TODO: When it's guaranteed that no comments can appear here,
	// it's almost given that anything not operator can be a value.
	else if(token.canBeValue())
	{
		newNode = std::make_shared <Value> (ctx.source.getString(token));
	}

	// If this node isn't complete, handle the new node as a value.
	if(!complete)
	{
		newCurrent = adoptValue(newNode);
	}

	// If the precedence of the new node is higher than that of the current
	// node, give the latest value to the new node and replace the
	// dent with the new node.
	else if(newNode->getPrecedence() > getPrecedence())
	{
		newNode->handleValue(stealLatestValue());
		newCurrent = adoptValue(newNode);
	}

	// If the precedence isn't higher, delegate the handling of the token
	// to the parent expression until equal or lower precedence is found.
	else
	{
		assert(!getParent().expired());
		return getParent().lock()->handleToken(ctx, token);
	}

	// If the current token is the last of the line, end the expression.
	// TODO: If the new node represents a binary operator, extend to the next line.
	if(token.isBeforeNewline(ctx))
	{
		newCurrent = exitCurrentExpression();
	}

	return newCurrent;
}

void Expression::handleValue(std::shared_ptr <Expression>)
{
	assert(false);
}

bool Expression::isRoot() const
{
	return false;
}

unsigned Expression::getPrecedence()
{
	return 0;
}

Expression::Type Expression::getType()
{
	return type;
}

const char* Expression::getTypeString(Type type)
{
	switch(type)
	{
		case Type::Root: return "Root";
		case Type::Value: return "Value";
		case Type::UnaryOperator: return "UnaryOperator";
		case Type::BinaryOperator: return "BinaryOperator";
	}

	return "???";
}

const char* Expression::getTypeString()
{
	return Expression::getTypeString(type);
}

Expression::Expression(Type type)
	: Node(Node::Type::Expression), type(type)
{
}

std::weak_ptr <Node> Expression::exitCurrentExpression()
{
	// Make sure that the parent node exists and is an expression.
	assert(!getParent().expired());

	// If a root is found, the expression can be exited by setting
	// the parent of the root as the new current node.
	if(type == Type::Root)
	{
		return getParent();
	}

	assert(std::dynamic_pointer_cast <Expression> (getParent().lock()));
	return std::static_pointer_cast <Expression> (getParent().lock())->exitCurrentExpression();
}

std::weak_ptr <Node> Expression::adoptValue(std::shared_ptr <Expression> node)
{
	adopt(node);
	handleValue(node);

	// Anything but values can be the new "current node".
	if(node->type != Type::Value)
	{
		return node;
	}

	return weak_from_this();
}

std::shared_ptr <Expression> Expression::stealLatestValue()
{
	printf("CALLED default stealLatestValue for '%s'\n", getTypeString());
	assert(false);
	return nullptr;
}

bool Expression::Root::isComplete() const
{
	return static_cast <bool> (first);
}

bool Expression::Root::isRoot() const
{
	return true;
}

void Expression::Root::handleValue(std::shared_ptr <Expression> node)
{
	assert(!first);
	first = std::move(node);
}

std::shared_ptr <Expression> Expression::Root::getFirst()
{
	return first;
}

std::shared_ptr <Expression> Expression::Root::stealLatestValue()
{
	auto ret = std::move(first);
	return ret;
}

}
