#include <cap/Attribute.hh>
#include <cap/Validator.hh>
#include <cap/BinaryOperator.hh>
#include <cap/BracketOperator.hh>
#include <cap/Source.hh>
#include <cap/Client.hh>
#include <cap/Value.hh>
#include <cap/Identifier.hh>

#include <cassert>

namespace cap
{

Attribute::Attribute()
	: Expression::Root(Type::Attribute)
{
}

unsigned Attribute::getPrecedence()
{
	return modifierPrecedence;
}

const char* Attribute::getTypeString() const
{
	return "Attribute";
}

bool Attribute::validate(Validator& validator)
{
	auto root = getFirst();

	assert(root);
	if(!validator.traverseExpression(root))
	{
		return false;
	}

	// Try to find a valid reference to a declaration.
	// Errors like "Unknown identifier" should already be handled by the validator.
	m_referredDeclaration = findReferred(root, validator);
	assert(!m_referredDeclaration.expired());

	return true;
}

std::shared_ptr <Declaration> Attribute::findReferred(std::shared_ptr <Expression> node, Validator& validator) const
{
	const auto& ctx = validator.getParserContext();

	if(node->getType() == Expression::Type::BinaryOperator)
	{
		// The final value within the chain of accesses should always be directly on the right.
		const auto op = std::static_pointer_cast <BinaryOperator> (node);
		node = op->getRight();
	}

	// All calls to findReferred should eventually hit here if there is a valid declaration reference.
	if(node->getType() == Expression::Type::Value)
	{
		const auto value = std::static_pointer_cast <Value> (node);
		if(value->getType() != Value::Type::Identifier)
		{
			SourceLocation location(ctx.m_source, node->getToken());
			ctx.m_client.sourceError(location, "Values used as attributes must be identifiers");
			return {};
		}

		const auto identifier = std::static_pointer_cast <Identifier> (node);
		auto result = identifier->getReferred();

		return result;
	}

	else if(node->getType() == Expression::Type::BracketOperator)
	{
		const auto op = std::static_pointer_cast <BracketOperator> (node);

		// TODO: What about generics? "@foo <int>". Do they make any sense?
		if(op->getType() != BracketOperator::Type::Call)
		{
			SourceLocation location(ctx.m_source, node->getToken());
			ctx.m_client.sourceError(location, "Bracket operators used as attributes must be function calls");
			return {};
		}

		return findReferred(op->getContext(), validator);
	}

	SourceLocation location(ctx.m_source, node->getToken());
	ctx.m_client.sourceError(location, "Attributes usages must be identifiers or function calls");
	return {};
}

}
