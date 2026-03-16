#include <cap/Return.hh>
#include <cap/ParserContext.hh>
#include <cap/Source.hh>
#include <cap/Function.hh>
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
	return m_expression->handleToken(ctx, token);
}

std::weak_ptr <Node> Return::invokedNodeExited(Node::ParserContext& ctx, Token&)
{
	assert(ctx.m_exitedFrom == m_expression);
	return getParent();
}

std::shared_ptr <Expression::Root> Return::getExpression() const
{
	return m_expression;
}

std::shared_ptr <Node> Return::getReturnedFrom() const
{
	assert(!m_returnedFrom.expired());
	return m_returnedFrom.lock();
}

bool Return::tryUpdatingReturnType(cap::ParserContext& ctx)
{
	auto from = getReturnedFrom();
	assert(m_fromType != FromType::None);

	// TODO: Once non-functions can be returned from, don't assume function.

	// Ensure that a type context exists for the function return type.
	auto func = std::static_pointer_cast <Function> (from);
	assert(func->getReturnTypeRoot());

	// If the returning expression doesn't exist, default to void.
	auto& ret = m_expression->getFirst() ?
		*m_expression->getFirst()->getResultType() :
		ctx.m_client.getBuiltin().get(Builtin::DataType::Void);

	// If no return type is set for what's being returned from, initialize
	// to whatever this return statement wants to return.
	if(!func->getReturnTypeRoot()->getResultType())
	{
		func->getReturnTypeRoot()->setResultType(ret);
		return true;
	}

	// If what's being returned from has a return type, make sure that whatever
	// this return statement returns is compatible.
	auto& existing = func->getReturnTypeRoot()->getResultType();
	assert(existing);

	if(!ret.isCompatible(*existing))
	{
		// TODO: Give more context in the error message.
		SourceLocation location(ctx.m_source, getToken());
		ctx.m_client.sourceError(location, "Incompatible return type");
		return false;
	}

	return true;
}

const char* Return::getTypeString() const
{
	return "Return";
}

bool Return::onInitialize(cap::ParserContext& ctx, bool)
{
	assert(m_returnedFrom.expired());
	if(!findReturnedFrom())
	{
		SourceLocation location(ctx.m_source, getToken());
		ctx.m_client.sourceError(location, "Cannot return here");
		return false;
	}

	// Initialize the expression here to make the assumption
	// of its existence valid.
	assert(!m_expression);
	m_expression = std::make_shared <Expression::Root> ();
	adopt(m_expression);

	return true;
}

bool Return::findReturnedFrom()
{
	auto current = getParent();

	// TODO: Support returning from scoped initializers.
	// Find the first parent that can or can't be returned from.
	while(!current.expired())
	{
		auto node = current.lock();
		if(node->getType() == Node::Type::Declaration)
		{
			auto decl = std::static_pointer_cast <Declaration> (node);

			// In the case of declarations only functions can be returned from.
			if(decl->getType() == Declaration::Type::Function)
			{
				m_fromType = FromType::Function;
				m_returnedFrom = decl;
			}

			// If returnedFrom wasn't set, this function will now return false.
			break;
		}

		current = node->getParent();
	}

	return !m_returnedFrom.expired();
}

}
