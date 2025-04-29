#include <cap/Expression.hh>
#include <cap/ParserContext.hh>
#include <cap/Variable.hh>
#include <cap/BinaryOperator.hh>
#include <cap/BracketOperator.hh>
#include <cap/Declaration.hh>
#include <cap/Source.hh>
#include <cap/Value.hh>
#include <cap/Client.hh>
#include <cap/ArgumentAccessor.hh>
#include <cap/ModifierRoot.hh>
#include <cap/Scope.hh>

#include <cassert>

namespace cap
{

std::weak_ptr <Node> Expression::handleToken(Node::ParserContext& ctx, Token& token)
{
	// handleToken should never be called for a value.
	assert(type != Type::Value);

	DBG_MESSAGE(ctx.client, "Expr token '", ctx.source.getString(token), "' handled by ", getTypeString());

	if(token.getType() == Token::Type::ClosingBracket)
	{
		// The state can get messed up if this goes below 0.
		if(ctx.subExpressionDepth > 0)
		{
			ctx.subExpressionDepth--;
		}

		// TODO: If the next token is an operator, maybe the expression should be continued?

		return exitExpression(ctx, token);
	}

	std::weak_ptr <Node> newCurrent = weak_from_this();
	std::shared_ptr <Expression> newNode;
	bool complete = isComplete();

	if(token.getType() == Token::Type::Operator)
	{
		// TODO: If the current node is a binary operator, expect an unary operator.
		newNode = BinaryOperator::create(ctx, token);
	}
	
	else if(token.getType() == Token::Type::OpeningBracket)
	{
		// Prevent the depth from being increment in recursive calls.
		if(ctx.canOpenSubexpression)
		{
			ctx.subExpressionDepth++;
			ctx.canOpenSubexpression = false;
		}

		// If the current expression node (Presumably an operator),
		// has all of the values that it needs, the previous token
		// should represent a value of some sort. In such case a bracket
		// operator is made which will steal that previous node and
		// treat it as the context.
		if(complete)
		{
			newNode = BracketOperator::create(ctx, token);
		}
		
		// The current expression node is incomplete which means that
		// the previous token should have been an operator.
		else
		{
			newNode = std::make_shared <Root> ();
		}
	}
	
	// Treat anything else as a value or a keyword.
	else
	{
		// Parse modifiers.
		if(auto modifier = ModifierRoot::create(ctx, token))
		{
			newNode = modifier;
		}

		// Anything else is a value.
		else
		{
			newNode = std::make_shared <Value> (ctx.source.getString(token));
		}
	}

	assert(newNode);
	newNode->setToken(token);

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
		auto stolen = stealLatestValue();
		newNode->adopt(stolen);

		newNode->handleValue(std::move(stolen));
		newCurrent = adoptValue(newNode);
	}

	// If the precedence isn't higher, delegate the handling of the token
	// to the parent expression until equal or lower precedence is found.
	else
	{
		assert(!getParent().expired());
		return getParent().lock()->handleToken(ctx, token);
	}

	// If the current token is the last of the line and no subexpressions
	// are active, end the expression.
	// TODO: If the new node represents a binary operator, extend to the next line.
	if(ctx.subExpressionDepth == 0 && token.isLastOfLine(ctx))
	{
		// Expressions cannot end at a modifier.
		if(newNode->getType() == Type::ModifierRoot)
		{
			SourceLocation location(ctx.source, getToken());
			ctx.client.sourceError(location, "Expected an expression after '", ctx.source.getString(newNode->getToken()), "'");
			return {};
		}

		return exitExpression(ctx, token);
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

const TypeContext& Expression::getResultType() const
{
	return resultType;
}

void Expression::setResultType(const TypeContext& ctx)
{
	resultType = ctx;
}

Expression::Expression(Type type)
	: Node(Node::Type::Expression), type(type)
{
}

std::weak_ptr <Node> Expression::exitExpression(ParserContext& ctx, Token& token)
{
	// If this closing bracket ends all subexpressions and is the final token
	// on a line, recursively break out of the expression.
	bool recursive = ctx.subExpressionDepth == 0 && token.isLastOfLine(ctx);

	auto exitedExpression = getExitedExpression(ctx, recursive);
	if(exitedExpression.expired())
	{
		return {};
	}

	ctx.exitedFrom = exitedExpression.lock();
	auto exitNode = ctx.exitedFrom->getParent();

	assert(!exitNode.expired());
	return exitNode.lock()->invokedNodeExited(ctx, token);
}

std::weak_ptr <Node> Expression::getExitedExpression(ParserContext& ctx, bool recursive)
{
	// Expression root is the only exit point.
	if(type == Type::Root)
	{
		if(recursive)
		{
			assert(!getParent().expired());
			auto parent = getParent().lock();
			if(parent->getType() == Node::Type::Expression)
			{
				return std::static_pointer_cast <Expression> (parent)->getExitedExpression(ctx, recursive);
			}
		}

		// If the parent isn't an expression, return this root.
		// It should be the first expression node that parsing was invoked for.
		return weak_from_this();
	}

	assert(!getParent().expired());
	assert(getParent().lock()->getType() == Node::Type::Expression);
	return std::static_pointer_cast <Expression> (getParent().lock())->getExitedExpression(ctx, recursive);
}

std::weak_ptr <Node> Expression::adoptValue(std::shared_ptr <Expression> node)
{
	adopt(node);
	handleValue(node);

	// Anything but values can be the new "current node".
	if(node->type != Type::Value)
	{
		// Bracket operators are special. Instead of returning itself as
		// the new "current node", the underlying root is returned itself
		// so that the expression within brackets is parsed properly.
		// After the bracketed subexpression ends, control will go back to
		// the bracket operator and to its parent as the bracket operator
		// should always have the highest possible precedence.
		if(node->type == Type::BracketOperator)
		{
			return std::static_pointer_cast <BracketOperator> (node)->getInnerRoot();
		}

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

Expression::Root::Root()
	: Expression(Type::Root)
{
}

Expression::Root::Root(Type type)
	: Expression(type)
{
}

bool Expression::Root::isComplete() const
{
	return static_cast <bool> (first);
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

const char* Expression::Root::getTypeString() const
{
	return "Expression root";
}

}
