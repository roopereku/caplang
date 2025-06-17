#include <cap/Validator.hh>
#include <cap/Source.hh>
#include <cap/Client.hh>
#include <cap/Function.hh>
#include <cap/ClassType.hh>
#include <cap/BinaryOperator.hh>
#include <cap/UnaryOperator.hh>
#include <cap/BracketOperator.hh>
#include <cap/Value.hh>
#include <cap/Identifier.hh>
#include <cap/Integer.hh>
#include <cap/String.hh>
#include <cap/Variable.hh>
#include <cap/Return.hh>

#include <cassert>

namespace cap
{

Validator::Validator(ParserContext& ctx)
	: ctx(ctx)
{
}

ParserContext& Validator::getParserContext() const
{
	return ctx;
}

void Validator::onNodeExited(std::shared_ptr <Node>, Result)
{
}

Traverser::Result Validator::onFunction(std::shared_ptr <Function> node)
{
	// Validate the node and make sure that it's unique.
	if(!node->validate(*this) ||
		!checkUniqueDeclaration(node))
	{
		return Result::Stop;
	}

	return Result::Exit;
}

Traverser::Result Validator::onClassType(std::shared_ptr <ClassType> node)
{
	if(!checkUniqueDeclaration(node))
	{
		return Result::Stop;
	}

	if(node->getBaseTypeRoot() &&
		!traverseExpression(node->getBaseTypeRoot()))
	{
		return Result::Stop;
	}

	auto scope = node->getParentScope();
	return Result::Continue;
}

Traverser::Result Validator::onExpressionRoot(std::shared_ptr <Expression::Root> node)
{
	if(node->getFirst())
	{
		if(!traverseExpression(node->getFirst()))
		{
			return Result::Stop;
		}

		if(node->getFirst()->getResultType())
		{
			node->setResultType(*node->getFirst()->getResultType());
		}
	}

	return Result::Exit;
}

Traverser::Result Validator::onVariable(std::shared_ptr <Variable> node)
{
	if(!checkUniqueDeclaration(node))
	{
		return Result::Stop;
	}

	if(!node->validate(*this))
	{
		return Result::Stop;
	}

	return Result::Continue;
}

Traverser::Result Validator::onBinaryOperator(std::shared_ptr <BinaryOperator> node)
{
	if(!traverseExpression(node->getLeft()))
	{
		return Result::Stop;
	}

	// If something is being accessed, store some context for
	// the right node to consume.
	if(node->getType() == BinaryOperator::Type::Access)
	{
		assert(node->getLeft()->getResultType());
		resolverCtx.accessedFrom.emplace(*node->getLeft()->getResultType());
	}

	if(!traverseExpression(node->getRight()))
	{
		return Result::Stop;
	}

	resolverCtx.reset();

	// Saving the result type doesn't make sense for commas.
	if(node->getType() != BinaryOperator::Type::Comma)
	{
        assert(node->getRight()->getResultType());

		// TODO: Forbid resetting of result type after initialization.
		if(node->getType() == BinaryOperator::Type::Assign)
		{
			node->getLeft()->setResultType(*node->getRight()->getResultType());
		}

		// TODO: Check if there is a binary operator overload and set the result
		// type based on that.
		node->setResultType(*node->getRight()->getResultType());
	}

	return Result::Exit;
}

Traverser::Result Validator::onUnaryOperator(std::shared_ptr <UnaryOperator> node)
{
	if(!traverseExpression(node->getExpression()))
	{
		return Result::Stop;
	}

    assert(node->getExpression()->getResultType());
	node->setResultType(*node->getExpression()->getResultType());
	return Result::Exit;
}

Traverser::Result Validator::onBracketOperator(std::shared_ptr <BracketOperator> node)
{
	// Temporarily steal the resolver context to hide it from the inner expression.
	auto contextResolver = std::move(resolverCtx);
	assert(!resolverCtx.accessedFrom);

	if(node->getType() != BracketOperator::Type::Call)
	{
		SourceLocation location(ctx.source, node->getToken());
		ctx.client.sourceError(location, "TODO: Bracket operator validation implemented for call operators only");
		return Result::Stop;
	}

	if(node->getContext()->getType() == Expression::Type::Value)
	{
		auto value = std::static_pointer_cast <Value> (node->getContext());
		if(value->getToken().getType() != Token::Type::Identifier)
		{
			SourceLocation location(ctx.source, value->getToken());
			ctx.client.sourceError(location, "Literal values cannot be called");
			return Result::Stop;
		}
	}

	// First validate the parameters so that they can be used for matching.
	if(!traverseExpression(node->getInnerRoot()))
	{
		return Result::Stop;
	}

	resolverCtx = std::move(contextResolver);
	resolverCtx.parameters = node->getInnerRoot();
	if(!traverseExpression(node->getContext()))
	{
		return Result::Stop;
	}

	auto callable = node->getContext()->getResultType();
	assert(callable);
	assert(callable->referenced.getType() == TypeDefinition::Type::Callable);

	// The call operator now results in the return type of the callable.
	auto returnTypeRoot = static_cast <CallableType&> (callable->referenced).getReturnTypeRoot();
	assert(returnTypeRoot);
	assert(returnTypeRoot->getResultType());
	node->setResultType(*returnTypeRoot->getResultType());
	return Result::Exit;
}

Traverser::Result Validator::onIdentifier(std::shared_ptr <Identifier> node)
{
	// Steal the resolver context so that it's not mistakenly used further on.
	ResolverContext resolve = std::move(resolverCtx);
	return validateIdentifier(node, resolve);
}

Traverser::Result Validator::onInteger(std::shared_ptr <Integer> node)
{
	// Set the initial type to the minimum that can hold the immediate value.
	// Outer operation such a negation can then alter this.
	node->setInitialType(ctx);
	return Result::Exit;
}

Traverser::Result Validator::onString(std::shared_ptr <String> node)
{
	node->setResultType(ctx.client.getBuiltin().get(Builtin::DataType::String));
	return Result::Exit;
}

Traverser::Result Validator::onReturn(std::shared_ptr <Return> node)
{
	if(!traverseExpression(node->getExpression()))
	{
		return Result::Stop;
	}

	if(!node->tryUpdatingReturnType(ctx))
	{
		return Result::Stop;
	}

	return Result::Exit;
}

bool Validator::checkUniqueDeclaration(std::shared_ptr <Declaration> decl)
{
	auto current = std::static_pointer_cast <Node> (decl);

	// Traverse the declaration storage hierarchy and check if something with
	// the same name as the given declaration exists.
	while(current)
	{
		// TODO: If we go beyond a named declaration, can there be any name shadowing?
		// Note that this would require the ability to reference the global scope or parent scopes.
		if(current->getDeclarationStorage().checkEquivalent(decl, *this))
		{
			return false;
		}

		// TODO: If current is a class type, check base classes here.
		// Note that checkUniqueDeclaration cannot be directly called without
		// fine tuning as then we'd implicitly check the parents of a base class
		// which might be totally irrelevant

		current = current->getParentWithDeclarationStorage();
	}

	return true;
}

Traverser::Result Validator::validateIdentifier(std::shared_ptr <Identifier> node, ResolverContext& resolve)
{
	Result result = Result::NotHandled;

	if(resolve.accessedFrom)
	{
		switch(resolve.accessedFrom->referenced.getType())
		{
			case cap::TypeDefinition::Type::Class:
			{
                auto& classDecl = static_cast <ClassType&> (resolve.accessedFrom->referenced);

				for(auto decl : classDecl.getBody()->declarations)
				{
					result = connectDeclaration(node, decl, resolve);
					if(result != Result::Continue)
					{
						break;
					}
				}

				// TODO: Iterate base classes. Modify the resolver context so that
				// accessedFrom references a base class.

				break;
			}

			case cap::TypeDefinition::Type::Callable:
			{
				SourceLocation location(ctx.source, node->getToken());
				ctx.client.sourceError(location, "Cannot access contents of a callable");
				return Result::Stop;
			}
		}
	}

	else
	{
		auto declContainer = node->getParentWithDeclarationStorage();

		// Check if the initial scope or any of its parents contain the desired declaration.
		while(declContainer)
		{
			auto& decls = declContainer->getDeclarationStorage();
			for(auto decl : decls)
			{
				result = connectDeclaration(node, decl, resolve);
				if(result != Result::Continue)
				{
					break;
				}
			}

			declContainer = declContainer->getParentWithDeclarationStorage();
		}
	}

	if(result == Result::Stop)
	{
		return result;
	}

	if(!node->getReferred())
	{
		// TODO: node->getValue() should also be something else in the
		// case of type constructors or type conversions.

		// TODO: Show a different message for generics? The param type can be
		// stored in ResolverContext.

		// If pararameters are supplied, we're looking for something more specific.
		const char* msg = resolve.parameters ?
			"No matching overload found for" :
			"Undeclared identifier";

		SourceLocation location(ctx.source, node->getToken());
		ctx.client.sourceError(location, msg, " '", node->getValue(), '\'');
		return Result::Stop;
	}

	return Result::Exit;
}

Traverser::Result Validator::connectDeclaration(std::shared_ptr <Identifier> node,
		std::shared_ptr <Declaration> decl, ResolverContext& resolve)
{
	if(decl->getName() != node->getValue())
	{
		return Result::Continue;
	}

	// Make sure that whatever is being matched against is validated.
	if(!decl->validate(*this))
	{
		return Result::Stop;
	}

	// Should parameters be matched as well?
	if(resolve.parameters)
	{
        CallableType* callable = nullptr;

		switch(decl->getType())
		{
			// Normal function calls.
			case Declaration::Type::Function:
			{
				callable = std::static_pointer_cast <Function> (decl).get();
				break;
			}

			// Constructor calls and type conversions.
			case Declaration::Type::Class:
			{
				assert(false && "TODO: Find a constructor or a type conversion");
				break;
			}

			// Operator overload call from an object.
			case Declaration::Type::Variable:
			{
				assert(false && "TODO: Find a callable in the context of an object");
				break;
			}
		}

		assert(callable);

		// TODO: Store the currently most fitting candidate based on unidentical
		// parameters select the most fitting one. This way more fitting function overloads
		// can be prioritized over those where parameters are implicitly casted.

		auto [compatible, unidentical] = callable->matchParameters(resolve.parameters);
		if(compatible)
		{
			node->setReferred(decl);
            node->updateResultType();
			return Result::Exit;
		}
	}

	else
	{
		node->setReferred(decl);
        node->updateResultType();
		return Result::Exit;
	}

	return Result::Continue;
}

Validator::ResolverContext::ResolverContext(ResolverContext&& rhs)
{
	this->operator=(std::move(rhs));
}

Validator::ResolverContext& Validator::ResolverContext::operator=(ResolverContext&& rhs)
{
	parameters = std::move(rhs.parameters);

	// TODO: Is this really necessary. std::move would be nice but
	// accessedFrom seems to report a valid value afterwards.
	if(rhs.accessedFrom)
	{
		accessedFrom.emplace(*rhs.accessedFrom);
		rhs.accessedFrom.reset();
	}

	return *this;
}

void Validator::ResolverContext::reset()
{
	accessedFrom.reset();
	parameters.reset();
}

}
