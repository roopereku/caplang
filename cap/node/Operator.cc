#include <cap/node/Operator.hh>
#include <cap/node/TwoSidedOperator.hh>
#include <cap/node/OneSidedOperator.hh>

#include <cap/event/DebugMessage.hh>

#include <cassert>

namespace cap
{

bool Operator::handleExpressionNode(std::shared_ptr <Expression> node, Parser& parser)
{
	assert(node);
	parser.events.emit(DebugMessage(std::string("Handle expression in operator ") + getTypeString(), node->token));

	// Adopt the expression node.
	adopt(node);

	if(node->type == Expression::Type::Value)
	{
		if(!handleValue(std::move(node)))
		{
			return false;
		}
	}

	// Is the new node an operator.
	else if(node->type == Expression::Type::Operator)
	{
		auto op = node->as <Operator> ();

		// The concept of high and low is inverted here as 0 is the
		// highest priority, therefore highest precedence.
		if(op->getPrecedence() < getPrecedence())
		{
			parser.events.emit(DebugMessage("New operator has higher precedence", token));
			if(!handleHigherPrecedence(op))
			{
				return false;
			}
		}

		else
		{
			// If the precedence is not the same, make the parent the current node and try again.
			if(op->getPrecedence() != getPrecedence())
			{
				parser.events.emit(DebugMessage("New operator has lower precedence", token));

				// If there is no parent, the new node takes control of the current node.
				if(getParent().expired())
				{
					// Adopt the current node.
					op->adopt(parser.getCurrentNode());

					if(!op->handleValue(parser.getCurrentNode()->as <Expression> ()))
					{
						return false;
					}

					parser.setCurrentNode(op);
				}

				else
				{
					// The parent node has to be an expression.
					assert(getParent().lock()->type == Node::Type::Expression);
					auto parentExpr = getParent().lock()->as <Expression> ();

					// If the parent expression is the root, make the new node adopt the current
					// one and set the new node as the expression root.
					if(parentExpr->type == Expression::Type::Root)
					{
						op->adopt(parser.getCurrentNode());
						if(!op->handleValue(parser.getCurrentNode()->as <Expression> ()))
						{
							return false;
						}

						parentExpr->replaceExpression(op);
					}

					// If the parent isn't the root, switch to it.
					else
					{
						parser.setCurrentNode(parentExpr);

						if(!parentExpr->handleExpressionNode(op, parser))
						{
							return false;
						}
					}
				}
			}

			// The precedence is the same.
			else
			{
				parser.events.emit(DebugMessage("New operator has same precedence", token));
				if(!handleSamePrecedence(op))
				{
					return false;
				}
			}
		}
			
		parser.setCurrentNode(node);
	}

	else
	{
		assert(false && "node in Operator::handleExpressionNode() has a weird type");
	}

	return true;
}

bool Operator::handleSamePrecedence(std::shared_ptr <Operator> op)
{
	if(op->type == Operator::Type::TwoSided)
	{
		// Make this operator the lhs of the new operator.
		auto twoSided = op->as <TwoSidedOperator> ();
		twoSided->setLeft(shared_from_this()->as <Expression> ());

		if(!getParent().expired())
		{
			auto parentExpr = getParent().lock()->as <Expression> ();

			// Replace this operator with the new two sided operator.
			parentExpr->adopt(twoSided);
			if(!parentExpr->replaceExpression(twoSided))
				return false;
		}

		twoSided->adopt(twoSided->getLeft());
	}

	else if(op->type == Operator::Type::OneSided)
	{
		// Make this the expression of the new operator.
		auto oneSided = op->as <OneSidedOperator> ();
		oneSided->setExpression(shared_from_this()->as <Expression> ());

		if(!getParent().expired())
		{
			auto parentExpr = getParent().lock()->as <Expression> ();

			// Replace this operator with the new one sided operator.
			parentExpr->adopt(oneSided);
			if(!parentExpr->replaceExpression(oneSided))
				return false;
		}

		oneSided->adopt(oneSided->getExpression());
	}

	return true;
}



}
