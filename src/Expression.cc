#include <cap/Expression.hh>
#include <cap/ParserContext.hh>
#include <cap/Variable.hh>
#include <cap/BinaryOperator.hh>
#include <cap/UnaryOperator.hh>
#include <cap/BracketOperator.hh>
#include <cap/Declaration.hh>
#include <cap/TypeReference.hh>
#include <cap/Source.hh>
#include <cap/Value.hh>
#include <cap/Client.hh>
#include <cap/ArgumentAccessor.hh>
#include <cap/Scope.hh>
#include <cap/Attribute.hh>

#include <cassert>

namespace cap
{

std::weak_ptr <Node> Expression::handleToken(Node::ParserContext& ctx, Token& token)
{
	// handleToken should never be called for a value.
	assert(m_type != Type::Value);

	DBG_MESSAGE(ctx.m_client, "Expr token '", ctx.m_source.getString(token), "' handled by ", getTypeString());

	if(token.getType() == Token::Type::ClosingBracket)
	{
		// The state can get messed up if this goes below 0.
		if(ctx.m_subExpressionDepth > 0)
		{
			ctx.m_subExpressionDepth--;
		}

		// TODO: If the next token is an operator, maybe the expression should be continued?

		return exitExpression(ctx, token);
	}

	std::weak_ptr <Node> newCurrent = weak_from_this();
	std::shared_ptr <Expression> newNode;

	bool complete = isComplete();

	// When outside an attribute, anything else but the beginning of an attribute
	// requires that the expression contains things other than attributes.
	if(!ctx.m_inAttribute && token.getType() != Token::Type::Attribute)
	{
		ctx.setMoreThanAttributes();
	}

	if(token.getType() == Token::Type::Operator)
	{
		if(complete)
		{
			newNode = UnaryOperator::createPostfix(ctx, token);

			if(!newNode)
			{
				newNode = BinaryOperator::create(ctx, token);
			}
		}

		else
		{
			newNode = UnaryOperator::createPrefix(ctx, token);
		}
	}

	else if(token.getType() == Token::Type::Attribute)
	{
		newNode = std::make_shared <Attribute> ();
		ctx.m_inAttribute = true;
	}

	else if(token.getType() == Token::Type::OpeningBracket)
	{
		// Prevent the depth from being increment in recursive calls.
		if(ctx.m_canOpenSubexpression)
		{
			ctx.m_subExpressionDepth++;
			ctx.m_canOpenSubexpression = false;
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
			// TODO: Once "@foo() ()" can be differentiated from "@foo()()" make sure that expressions
			// don't end in attributes as there will be a regular value in the expression.
			//if (!ctx.inAttribute)
			//{
			//	ctx.allowExpressionEndingInAttributes = false;
			//}

			newNode = std::make_shared <Root> ();
		}
	}
	
	// Treat anything else as a value or a keyword.
	else
	{
		if(ctx.m_source.match(token, L"type"))
		{
			newNode = std::make_shared <TypeReference> ();
		}

		// Anything else is a value.
		else
		{
			newNode = Value::create(ctx, token);
			if(!newNode)
			{
				return {};
			}
		}
	}

	assert(newNode);
	newNode->setToken(token);

	// If this node isn't complete, handle the new node as a value.
	if(!complete)
	{
		// TODO: Apply active attributes to upcoming expression nodes once decided how.
		if(!ctx.m_inAttribute)
		{
			// Assign attributes from the current depth to the new node.
			while(!ctx.m_activeAttributes.empty() && ctx.m_activeAttributes.top().m_depth == ctx.m_subExpressionDepth)
			{
				// TODO: Somehow append instead of overwriting.
				newNode->setAttributeRange(ctx.m_activeAttributes.top().m_range);
				ctx.m_activeAttributes.pop();
			}
		}

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
		auto parent = getParent().lock();

		const auto newPrecedence = newNode->getPrecedence();

		// The new node is outside the current attribute if it breaks a larger value or starts an attribute.
		// A larger value is broken if the precedence is below modifier precesence wich implies a binary operator
		// excluding '.'. An example of a larger value would be -foo.bar().results[0]
		const bool newOutsideCurrentAttribute = newPrecedence < modifierPrecedence || newNode->m_type == Type::Attribute;

		// Save attributes upon leaving them and disconnect them from the node tree.
		if(m_type == Type::Attribute && newOutsideCurrentAttribute)
		{
			finalizeCurrentAttribute(ctx);
		}

		// If we're leaving out of an expression, we've failed to find a node
		// which has a lower precedence than the newly created node.
		// This only makes sense with consecutive values as values have precedence 0
		// and will never be above other nodes.
		else if(parent->getType() != Node::Type::Expression)
		{
			SourceLocation location(ctx.m_source, token);
			ctx.m_client.sourceError(location, "Consecutive values are not allowed");
			return {};
		}

		// TODO: Considering that newNode is set here, an optimization would be
		// to pass it directly to the parent.
		return parent->handleToken(ctx, token);
	}

	// If the current token is the last of the line and no subexpressions
	// are active, end the expression.
	// TODO: If the new node represents a binary operator, extend to the next line.
	if(ctx.m_subExpressionDepth == 0 && token.isLastOfLine(ctx))
	{
		return exitExpression(ctx, token);
	}

	return newCurrent;
}

void Expression::finalizeCurrentAttribute(ParserContext& ctx)
{
	auto parent = getParent().lock();

	assert(parent->getType() == Node::Type::Expression);
	auto attribute = std::static_pointer_cast <Expression> (parent)->stealLatestValue();

	assert(attribute->getType() == Expression::Type::Attribute);
	ctx.storeAttribute(std::static_pointer_cast <Attribute> (attribute));
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
	return m_type;
}

const std::optional <TypeContext>& Expression::getResultType() const
{
	return m_resultType;
}

void Expression::setResultType(const TypeContext& ctx)
{
	m_resultType.emplace(ctx);
}

Expression::Expression(Type type)
	: Node(Node::Type::Expression), m_type(type)
{
}

std::weak_ptr <Node> Expression::exitExpression(ParserContext& ctx, Token& token)
{
	// If we're not inside a subexpression and there's no more relevant tokens
	// on the current line, exit to a non-expression parent node.
	bool recursive = ctx.m_subExpressionDepth == 0 && token.isLastOfLine(ctx);

	auto exitedExpression = getExitedExpression(ctx, recursive);
	if(exitedExpression.expired())
	{
		return {};
	}

	// Make sure that expressions don't end in attributes when not allowed.
	if(!ctx.m_activeAttributes.empty())
	{
		if (ctx.m_activeAttributes.top().m_depth > 0 || !ctx.m_allowExpressionEndingInAttributes)
		{
			SourceLocation location(ctx.m_source, token);
			ctx.m_client.sourceError(location, "Expression must not end in an attribute here");
			return {};
		}
	}

	ctx.m_exitedFrom = exitedExpression.lock();
	auto exitNode = ctx.m_exitedFrom->getParent();

	assert(!exitNode.expired());
	return exitNode.lock()->invokedNodeExited(ctx, token);
}

std::weak_ptr <Node> Expression::getExitedExpression(ParserContext& ctx, bool recursive)
{
	// Expression root is the only exit point.
	if(m_type == Type::Root)
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

	else if(m_type == Type::Attribute)
	{
		finalizeCurrentAttribute(ctx);
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
	if(node->m_type != Type::Value)
	{
		// Bracket operators are special. Instead of returning itself as
		// the new "current node", the underlying root is returned itself
		// so that the expression within brackets is parsed properly.
		// After the bracketed subexpression ends, control will go back to
		// the bracket operator and to its parent as the bracket operator
		// should always have the highest possible precedence.
		if(node->m_type == Type::BracketOperator)
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
	return static_cast <bool> (m_first);
}

void Expression::Root::handleValue(std::shared_ptr <Expression> node)
{
	assert(!m_first);
	m_first = std::move(node);
}

std::shared_ptr <Expression> Expression::Root::getFirst()
{
	return m_first;
}

std::shared_ptr <Expression> Expression::Root::stealLatestValue()
{
	auto ret = std::move(m_first);
	return ret;
}

const char* Expression::Root::getTypeString() const
{
	return "Expression root";
}

}
