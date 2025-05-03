#include <cap/Return.hh>
#include <cap/ParserContext.hh>

#include <cap/Source.hh>
#include <cap/Client.hh>

#include <cassert>

namespace cap
{

Return::Return()
	: Statement(Type::Return)
{
}

std::weak_ptr <Node> Return::handleToken(Node::ParserContext& ctx, Token& token)
{
	bool isValidReturn = false;
	auto returnFrom = getReturnedFrom();

	// Return from a function is valid.
	if(returnFrom->getType() == Node::Type::Declaration)
	{
		auto decl = std::static_pointer_cast <Declaration> (returnFrom);
		isValidReturn = decl->getType() == Declaration::Type::Function;
	}

	// TODO: Handle expression case with scoped initializer.

	if(!isValidReturn)
	{
		SourceLocation location(ctx.source, getToken());
		ctx.client.sourceError(location, "Cannot return here");
		return {};
	}

	assert(!expression);
	expression = std::make_shared <Expression::Root> ();
	adopt(expression);
	return expression->handleToken(ctx, token);
}

std::weak_ptr <Node> Return::invokedNodeExited(Node::ParserContext& ctx, Token&)
{
	assert(ctx.exitedFrom == expression);
	return getParent();
}

std::shared_ptr <Expression::Root> Return::getExpression() const
{
	return expression;
}

std::shared_ptr <Node> Return::getReturnedFrom() const
{
	assert(!getParent().expired());

	auto parent = getParent().lock();
	assert(parent->getType() == Node::Type::Scope);

	assert(!parent->getParent().expired());
	return parent->getParent().lock();
}

const char* Return::getTypeString() const
{
	return "Return";
}

}
