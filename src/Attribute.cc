#include <cap/Attribute.hh>

#include <cassert>

namespace cap
{

Attribute::Attribute()
	: Declaration(Type::Attribute)
{
}

std::weak_ptr <Node> Attribute::handleToken(Node::ParserContext& ctx, Token& token)
{
	(void)ctx;
	(void)token;

	// TODO: Read the attribute name and a possible signature.

	assert(false && "TODO");
	return {};
}

const char* Attribute::getTypeString() const
{
	return "Attribute";
}

bool Attribute::validate(Validator& validator)
{
	(void)validator;
	return true;
}

Attribute::Root::Root()
	: Expression::Root(Type::Attribute)
{
}

unsigned Attribute::Root::getPrecedence()
{
	return modifierPrecedence;
}

const char* Attribute::Root::getTypeString() const
{
	return "Attribute";
}

}
