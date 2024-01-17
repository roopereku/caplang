#include <cap/node/GenericInstantiation.hh>
#include <cap/node/TwoSidedOperator.hh>

#include <cap/event/DebugMessage.hh>

#include <cassert>

namespace cap
{

// TODO: Pass the rightmost node of target to Value as the token.
GenericInstantiation::GenericInstantiation(std::shared_ptr <Expression> target)
	: Value(target->token), target(target)
{
}

bool GenericInstantiation::parse(Parser& parser)
{
	const auto getTwoSided = [](std::shared_ptr <Node> node)
	{
		if(node->type == Node::Type::Expression &&
			node->as <Expression> ()->type == Expression::Type::Operator &&
			node->as <Operator> ()->type == Operator::Type::TwoSided)
		{
			return node->as <TwoSidedOperator> ();
		}

		return std::shared_ptr <TwoSidedOperator> (nullptr);
	};

	// If the current operator isn't ">", there's no generic instantiation.
	auto genericCloser = getTwoSided(parser.getCurrentNode()->as <Expression> ());
	if(!genericCloser || genericCloser->type != TwoSidedOperator::Type::GreaterThan)
	{
		return true;
	}

	// If the left hand side operator isn't "<", there might be a comma between.
	auto genericOpener = getTwoSided(genericCloser->getLeft());
	if(!genericOpener || genericOpener->type != TwoSidedOperator::Type::LessThan)
	{
		assert(!parser.getCurrentNode()->getParent().expired());
		auto parent = getTwoSided(parser.getCurrentNode()->getParent().lock());

		// If the parent isn't a comma or the the instantiation closer isn't
		// its right side operator, there's no generic instantiation.
		if(parent->type != TwoSidedOperator::Type::Comma || genericCloser != parent->getRight())
		{
			return true;
		}

		//auto argument = genericCloser->stealMostRecentValue();

		// Look for a "<" operator within the left side of the comma.
		genericOpener = parent;
		while(genericOpener && genericOpener->type != TwoSidedOperator::Type::LessThan)
		{
			// Comma should always contain a value on the right side.
			if(genericOpener->type == TwoSidedOperator::Type::Comma)
			{
				genericOpener = getTwoSided(genericOpener->getLeft());
			}

			// Anything else could contain the "<" operator.
			// TODO: Check if this is always valid.
			else
			{
				genericOpener = getTwoSided(genericOpener->getRight());
			}
		}
	}

	// If the closing arrow was found, insert a generic instantation into the AST as a value.
	if(genericOpener && genericOpener->type == TwoSidedOperator::Type::LessThan)
	{
		assert(genericOpener->getLeft());
		assert(genericOpener->getRight());

		// Create the instantiation and treat it as a value.
		parser.previousWasValue();
		auto instantiation = std::make_shared <GenericInstantiation> (
			genericOpener->getLeft()
		);

		// The first argument should be the right side value of the opening arrow.
		instantiation->arguments = genericOpener->stealMostRecentValue();
		//instantiation->arguments = genericOpener->getRight();

		// If the opener is the left side operator of the closer, there is only a single argument.
		// Otherwise there are multiple arguments as commas come into play.
		if(genericOpener != genericCloser->getLeft())
		{
			// Until the right side operator is the closing arrow, collect arguments.
			auto current = genericOpener;
			while(current->getRight() != genericCloser)
			{
				// If the current node is a comma, save an argument.
				// The argument should exist on the right side.
				if(current->type == TwoSidedOperator::Type::Comma)
				{
					instantiation->arguments->findLast()->setNext(current->stealMostRecentValue());
				}

				// Switch to the parent node assuming it's a two sided operator.
				assert(!current->getParent().expired());
				current = getTwoSided(current->getParent().lock());
				assert(current);
			}

			// The last argument is on the left side of the closing arrow.
			// If this function is called properly which is after the ">" is
			// put into the AST, it should contain no right side value and the left side is stolen.
			assert(!genericCloser->getRight());
			instantiation->arguments->findLast()->setNext(genericCloser->stealMostRecentValue());

			// The parent of the closing arrow should be a comma.
			auto closerParent = getTwoSided(genericCloser->getParent().lock());
			assert(closerParent);
			assert(closerParent->type == TwoSidedOperator::Type::Comma);

			// The parent of the previously acquire comma operator. This is used
			// to replace the comma with whatever came before the opening arrow.
			auto commaParent = closerParent->getParent().lock()->as <Expression> ();

			// Get the parent node of the opening arrow.
			assert(!genericOpener->getParent().expired());
			auto openerParent = genericOpener->getParent().lock()->as <Expression> ();

			// Replace the comma with whatever expression came before the opening arrow.
			commaParent->replaceMostRecentValue(openerParent);

			// Switch back to the generic opener which should be contained in the expression
			// that previously replaced the comma. This is done to make sure that the generic
			// opener node is no longer tracked and is replaced with the generic instantiation.
			parser.setCurrentNode(genericOpener);
		}

		// Switch to the parent and remove ">" from the AST.
		parser.setCurrentNode(parser.getCurrentNode()->getParent().lock());
		parser.getCurrentNode()->as <Expression> ()->replaceMostRecentValue(instantiation);
	}

	return true;
}

std::shared_ptr <Expression> GenericInstantiation::getArguments()
{
	return arguments;
}

const char* GenericInstantiation::getTypeString()
{
	return "Generic instantiation";
}

bool GenericInstantiation::isGeneric()
{
	return true;
}

}
