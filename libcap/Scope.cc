#include <cap/Type.hh>
#include <cap/Scope.hh>
#include <cap/Function.hh>
#include <cap/PrimitiveType.hh>

#include <cap/event/UnknownIdentifier.hh>
#include <cap/event/InvalidAccess.hh>
#include <cap/event/InvalidOperatorOverload.hh>
#include <cap/event/GenericMessage.hh>

#include <cap/node/ExpressionRoot.hh>
#include <cap/node/TwoSidedOperator.hh>
#include <cap/node/OneSidedOperator.hh>
#include <cap/node/TypedConstant.hh>
#include <cap/node/VariableDefinition.hh>
#include <cap/node/VariableDeclaration.hh>
#include <cap/node/FunctionDeclaration.hh>
#include <cap/node/TypeDeclaration.hh>
#include <cap/node/FunctionCall.hh>
#include <cap/node/Subscript.hh>
#include <cap/node/Return.hh>
#include <cap/node/DeclarationReference.hh>

namespace cap
{

Scope::Scope() : Scope(nullptr)
{
	PrimitiveType::registerBuiltins(*this);
}

Scope& Scope::getSharedScope()
{
	static Scope sharedScope;
	return sharedScope;
}

std::shared_ptr <Declaration> Scope::getMember(std::string_view name)
{
	for(auto& member : members)
	{
		if(member->getName() == name)
		{
			return member;
		}
	}

	return nullptr;
}

Token Scope::consumeName(Tokenizer& tokens)
{
	Token name = tokens.next();

	if(name.getType() != Token::Type::Identifier)
	{
		printf("%lu:%lu Expected an identifier, got '%s'\n", name.getRow(), name.getColumn(), name.getTypeString());
		return Token::createInvalid();
	}

	return name;
}

bool Scope::createFunction(Token&& token, ParserState& state)
{
	members.emplace_back(state.node->createNext <FunctionDeclaration> (std::move(token)));
	Token name = consumeName(state.tokens);

	printf("-------------------------- START PARSING FUNCTION --------------------------------------------\n");

	auto function = std::make_shared <Function> (*this, std::move(name));
	ParserState newState(state.tokens, state.events, function->getRoot());

	state.node = members.back();
	state.node->as <FunctionDeclaration> ()->function = function;

	bool ret = function->parse(newState);

	printf("-------------------------- STOP PARSING FUNCTION ---------------------------------------------\n");

	return ret;
}

bool Scope::createType(Token&& token, ParserState& state)
{
	members.emplace_back(state.node->createNext <TypeDeclaration> (std::move(token)));
	Token name = consumeName(state.tokens);

	printf("-------------------------- START PARSING TYPE---- --------------------------------------------\n");

	auto type = std::make_shared <Type> (*this, std::move(name));
	ParserState newState(state.tokens, state.events, type->getRoot());

	state.node = members.back();
	state.node->as <TypeDeclaration> ()->type = type;

	bool ret = type->parse(newState);
	printf("-------------------------- STOP PARSING TYPE --------------------------------------------------\n");

	return ret;
}

Type& Scope::createPrimitiveType(std::string_view name, size_t size)
{
	printf("Creating primitive '%s'\n", name.data());

	Token typeName(Token::Type::Identifier, name, 0, 0);
	members.emplace_back(getRoot()->findLastNode()->createNext <TypeDeclaration> (std::move(typeName)));

	auto type = std::make_shared <PrimitiveType> (*this, std::move(name), size);
	members.back()->as <TypeDeclaration> ()->type = type;

	return *type;
}

bool Scope::createVariable(Token&& token, ParserState& state, bool isParameter)
{
	if(!state.initExpression(token.getRow()))
		return false;

	state.node = state.node->createNext <VariableDeclaration> (std::move(token));
	auto variable = state.node->as <VariableDeclaration> ();
	variable->isParameter = isParameter;

	variable->initialization = std::make_shared <ExpressionRoot> (Token::createInvalid());
	state.node = variable->initialization;

	return true;
}

bool Scope::parse(Tokenizer& tokens, EventEmitter& events)
{
	ParserState state(tokens, events, root);
	return parse(state);
}

bool Scope::parse(ParserState& state)
{
	printf("Scope parse\n");

	// Braces are only tracked if there were none open initially.
	// For example the global scope utilizes this.
	const bool trackBraces = state.braces.isOpened();

	// Loop while there are tokens and there are braces to track.
	while(!state.tokens.empty())
	{
		if(trackBraces && !state.braces.isOpened())
			break;

		Token token = state.tokens.next();
		BraceType braceType = BraceMatcher::getBraceType(token);

		//printf("Token %s '%s' %lu %lu\n", token.getTypeString(), token.getString().c_str(), token.getRow(), token.getColumn());

		// Is the current token an opening brace?
		if(braceType == BraceType::Opening)
		{
			if(!checkRowChange(token.getRow(), state))
				return false;

			if(!parseBracket(std::move(token), state))
				return false;
		}

		// Is the current token a closing brace?
		else if(braceType == BraceType::Closing)
		{
			if(!state.braces.close(std::move(token), state.events))
				return false;

			// End the expression when a brace is closed
			if(state.inExpression && !state.endExpression())
				return false;
		}

		// The current token is not a brace.
		else
		{
			if(!checkRowChange(token.getRow(), state))
				return false;

			if(!state.inExpression)
			{
				printf("Not in expression\n");

				// "func" indicates that a function should be created.
				if(token == "func")
				{
					// If the function creation fails, stop parsing.
					if(!createFunction(std::move(token), state))
						return false;
				}

				// "type" indicates that a type should be created.
				else if(token == "type")
				{
					// If the type creation fails, stop parsing.
					if(!createType(std::move(token), state))
						return false;
				}

				// "var" indicates that a variable should be created.
				else if(token == "var")
				{
					// If the variable creation fails, stop parsing.
					if(!createVariable(std::move(token), state, false))
						return false;

					// Since variables require assignment, we can assume that we're in an expression.
					state.inExpression = true;
				}

				else if(token == "return")
				{
					auto row = token.getRow();
					state.node = state.node->createNext <Return> (std::move(token));

					if(!state.initExpression(row))
						return false;

					auto exprRoot = std::make_unique <ExpressionRoot> (Token::createInvalid());
					auto returnNode = state.node->as <Return> ();

					returnNode->expression = std::move(exprRoot);
					state.node = returnNode->expression;
				}

				// If the token belongs to an expression, initialize an expression.
				else
				{
					if(!state.initExpression(token.getRow()))
						return false;

					state.node = state.node->createNext <ExpressionRoot> (Token::createInvalid());

					if(!state.node->handleToken(std::move(token), state))
						return false;
				}
			}

			// Pass the token to an expression node.
			else
			{
				if(!state.node)
				{
					state.events.emit(GenericMessage(token, "??? No current node", Message::Type::Error));
					return false;
				}

				if(!state.node->handleToken(std::move(token), state))
					return false;
			}
		}
	}

	// If we're not in a global scope and braces are still
	// being tracked, we have unterminated braces.
	if(parent && state.braces.isOpened())
	{
		state.events.emit(GenericMessage(state.braces.getOpener(), "Unterminated brace", Message::Type::Error));
		return false;
	}

	return true;
}

bool Scope::parseBracket(Token&& token, ParserState& state)
{
	Token::Type t = token.getType();
	auto inBraces = std::make_shared <ExpressionRoot> (Token::createInvalid());

	ParserState newState(state.tokens, state.events, inBraces);
	newState.initExpression(token.getRow());
	newState.canEndExpression = false;

	// If there was a value previously, the brackets might indicate
	// a function call or a subscript operator.
	if(state.previousIsValue)
	{
		if(!state.inExpression)
		{
			state.events.emit(GenericMessage(token, "??? Not in expression", Message::Type::Error));
			return false;
		}

		std::shared_ptr <OneSidedOperator> op;

		if(t == Token::Type::Parenthesis)
		{
			printf("FUNCTION CALL\n");
			op = std::make_shared <FunctionCall> (Token(token));
			op->as <FunctionCall> ()->setParameters(inBraces);
		}

		else if(t == Token::Type::SquareBracket)
		{
			printf("SUBSCRIPT\n");
			op = std::make_shared <Subscript> (Token(token));
			op->as <Subscript> ()->setContents(inBraces);
		}

		else
		{
			state.events.emit(GenericMessage(token, "TODO: {} after a value", Message::Type::Error));
			return false;
		}

		if(!state.node->as <Expression> ()->handleExpressionNode(op, state))
			return false;
	}

	state.previousIsValue = true;
	Token::IndexType row = token.getRow();

	if(!newState.braces.open(std::move(token), state.events))
		return false;

	printf("------------------- START BRACES ------------------------\n");

	if(!Scope::parse(newState))
		return false;

	printf("------------------- END BRACES --------------------------\n");

	if(state.inExpression)
	{
		auto expr = state.node->as <Expression> ();
		bool setAfterCurrent = true;

		if(expr->isOperator())
		{
			auto op = expr->as <Operator> ();

			if(op->isOneSided())
			{
				auto oneSided = op->as <OneSidedOperator> ();

				if(oneSided->affectsPreviousValue())
				{
					printf("Don't set after current\n");
					setAfterCurrent = false;
				}
			}
		}

		if(setAfterCurrent && !expr->handleExpressionNode(inBraces, state))
			return false;
	}

	else
	{
		if(!state.initExpression(row))
			return false;

		state.node = state.node->createNext <ExpressionRoot> (Token::createInvalid());

		// Try to cache the contents inside the braces.
		if(!state.node->as <Expression> ()->handleExpressionNode(std::move(inBraces), state))
			return false;
	}

	return true;
}

bool Scope::checkRowChange(Token::IndexType currentRow, ParserState& state)
{
	if(state.inExpression && currentRow > state.expressionStartRow)
	{
		if(!state.endExpression())
			return false;
	}

	return true;
}

bool Scope::validate(EventEmitter& events)
{
	if(isNamed())
	{
		printf("Validating scope '%s'\n", ((NamedScope*)this)->getName().getString().c_str());
	}

	ValidationState state(events);

	// Give the validation state some context about this scope.
	if(isNamed())
	{
		auto& named = static_cast <NamedScope&> (*this);
		state.inFunction = named.isFunction();
	}

	bool ret = validateNode(root, state);

	// If this scope is a function, set the return type.
	if(state.inFunction)
	{
		printf("RETURN TYPE IS %p\n", state.returnType);

		// If there was a return type, use that for initialization.
		if(state.returnType)
		{
			static_cast <Function*> (this)->initializeReturnType(*state.returnType);
		}

		// If no return statement was encountered, assume void.
		else
		{
			state.events.emit(GenericMessage(root->getToken(), "TODO: Initialize return type as void", Message::Type::Error));
			return false;
		}
	}

	if(isNamed())
	{
		printf("Done validating scope '%s'\n", ((NamedScope*)this)->getName().getString().c_str());
	}

	return ret;
}

bool Scope::handleVariableDeclaration(std::shared_ptr <Expression> node, ValidationState& state)
{
	if(node->isOperator())
	{
		if(node->as <Operator> ()->isTwoSided())
		{
			auto twoSided = node->as <TwoSidedOperator> ();

			if(twoSided->getType() == TwoSidedOperator::Type::Assignment)
			{
				if(twoSided->getLeft()->isValue() && twoSided->getLeft()->getToken().getType() == Token::Type::Identifier)
				{
					state.findMembersInParent = true;
					twoSided->setRight(validateExpression(twoSided->getRight(), state));
					if(!twoSided->getRight()) return false;

					Variable variable(twoSided->getLeft()->as <Value> (), twoSided->getRight()->getResultType());
					members.emplace_back(std::make_shared <VariableDefinition> (std::move(variable)));

					twoSided->setLeft(validateExpression(twoSided->getLeft(), state));
					if(!twoSided->getLeft()) return false;

					twoSided->setResultType(twoSided->getLeft()->getResultType());
					
					printf("Create variable '%s'\n", twoSided->getLeft()->getToken().getString().c_str());
					return true;
				}

				else
				{
					printf("Variable name has to be an identifier\n");
					return false;
				}
			}

			// If the operator following a variable declaration is a comma, handle variable
			// declarations on both sides of the comma.
			else if(twoSided->getType() == TwoSidedOperator::Type::Comma)
			{
				printf("HANDLE COMMA\n");

				if(!handleVariableDeclaration(twoSided->getLeft(), state))
					return false;

				if(!handleVariableDeclaration(twoSided->getRight(), state))
					return false;

				return true;
			}
		}
	}

	state.events.emit(GenericMessage(node->getToken(), "Expected '=' after a variable declaration", Message::Type::Error));
	return false;
}

bool Scope::validateNode(std::shared_ptr <Node> node, ValidationState& state)
{
	// TODO: Handle anonymous scopes. Call scope.validateNode(state).

	if(node->isDeclaration())
	{
		auto decl = node->as <Declaration> ();

		if(decl->isVariable())
		{
			if(state.inVariable)
			{
				state.events.emit(GenericMessage(decl->getToken(), "Can't nest variable declarations", Message::Type::Error));
				return false;
			}

			auto varDecl = decl->as <VariableDeclaration> ();

			if(varDecl->initialization->getRoot())
			{
				state.inVariable = true;
				printf("Handle variable declaration\n");

				if(!handleVariableDeclaration(varDecl->initialization->getRoot(), state))
					return false;

				printf("Variable declaration done\n");

				state.inVariable = false;
			}

			else if(!varDecl->isParameter)
			{
				state.events.emit(GenericMessage(varDecl->getToken(), "??? Expression root empty", Message::Type::Error));
				return false;
			}

			else
			{
				printf("Skip empty parameters\n");
			}
		}

		else if(decl->isType())
		{
			auto typeDecl = decl->as <TypeDeclaration> ();
			if(!typeDecl->type->validate(state.events))
				return false;
		}

		else if(decl->isFunction())
		{
			auto funcDecl = decl->as <FunctionDeclaration> ();
			if(!funcDecl->function->validate(state.events))
				return false;
		}
	}

	else if(node->isExpression())
	{
		auto expr = node->as <Expression> ();

		if(expr->isExpressionRoot())
		{
			state.findMembersInParent = true;
			auto exprRoot = expr->as <ExpressionRoot> ();

			exprRoot->setRoot(validateExpression(exprRoot->getRoot(), state));
			if(!exprRoot->getRoot()) return false;
		}

		else
		{
			state.events.emit(GenericMessage(expr->getToken(), "??? Expression in validateNode isn't a root", Message::Type::Error));
			return false;
		}

	}

	else if(node->isStatement())
	{
		auto statement = node->as <Statement> ();

		if(statement->isReturn())
		{
			auto returnExpr = statement->as <Return> ()->expression;
			if(!validateNode(returnExpr, state))
				return false;

			// Are we in a function?
			if(state.inFunction)
			{
				// If the return type has not been set, initialize it.
				if(!state.returnType)
				{
					// TODO: If a return statement without an expression is encountered, returnExpr->getRoot(),
					// will return a null and the return type should be set as void.

					// The expression root contained in the return statement holds the return type.
					state.returnType = &returnExpr->getRoot()->getResultType();

					printf("INITIALIZED RETURN TYPE TO '%s'\n", state.returnType->getName().getString().c_str());
				}

				// If the return result type doesn't match the initialized type, error out.
				else if(&returnExpr->getRoot()->getResultType() != state.returnType)
				{
					state.events.emit(GenericMessage(statement->getToken(), "Inconsistent return type", Message::Type::Error));
					return false;
				}
			}
		}
	}

	if(node->hasNext() && !validateNode(node->getNext(), state))
		return false;

	return true;
}

std::shared_ptr <Expression> Scope::validateExpression(std::shared_ptr <Expression> expr, ValidationState& state)
{
	if(expr->isExpressionRoot())
	{
		auto exprRoot = expr->as <ExpressionRoot> ();
		exprRoot->setRoot(validateExpression(exprRoot->getRoot(), state));
		if(!exprRoot->getRoot()) return nullptr;

		return exprRoot->getRoot();
	}

	printf("Validate expr '%s'\n", expr->getToken().getString().c_str());

	if(expr->isValue())
	{
		if(expr->getToken().getType() == Token::Type::Identifier)
		{
			for(auto member : members)
			{
				printf("Check member '%s'\n", member->getName().getString().c_str());

				if(expr->getToken() == member->getName())
				{
					printf("Found member '%s'\n", expr->getToken().getString().c_str());
					auto ref = std::make_shared <DeclarationReference> (member);

					expr->getParent()->adopt(ref);
					return ref;
				}
			}

			printf("Look up in parent %d\n", state.findMembersInParent);

			if(parent && state.findMembersInParent)
			{
				printf("Check in parent scope\n");
				return parent->validateExpression(expr, state);
			}

			state.events.emit(UnknownIdentifier(expr->getToken(), *this));
			return nullptr;
		}

		else
		{
			return std::make_shared <TypedConstant> (expr->as <Value> ());
		}
	}

	else if(expr->isOperator())
	{
		if(expr->as <Operator> ()->isTwoSided())
		{
			auto twoSided = expr->as <TwoSidedOperator> ();

			if(twoSided->getType() == TwoSidedOperator::Type::Access)
			{
				printf("Get lhs of access\n");
				auto lhs = validateExpression(twoSided->getLeft(), state);
				if(!lhs) return nullptr;

				printf("NOTE: Members are no longer looked up in parent\n");
				state.findMembersInParent = false;

				if(lhs->isDeclarationReference())
				{
					auto ref = lhs->as <DeclarationReference> ();

					if(ref->getDeclaration()->isFunction())
					{
						state.events.emit(InvalidAccess(expr->getToken(), ref));
						return nullptr;
					}

					printf("Check for '%s' in '%s'\n",
							twoSided->getRight()->getToken().getString().c_str(),
							ref->getResultType().getName().getString().c_str());

					return ref->getResultType().validateExpression(twoSided->getRight(), state);
				}

				state.events.emit(GenericMessage(twoSided->getLeft()->getToken(), "??? Not a declaration reference", Message::Type::Error));
				return nullptr;
			}

			else
			{
				twoSided->setLeft(validateExpression(twoSided->getLeft(), state));
				if(!twoSided->getLeft()) return nullptr;

				twoSided->setRight(validateExpression(twoSided->getRight(), state));
				if(!twoSided->getRight()) return nullptr;

				auto& lhsType = twoSided->getLeft()->getResultType();

				if(!lhsType.hasOperator(twoSided->getType()))
				{
					state.events.emit(InvalidOperatorOverload(twoSided, lhsType));
					return nullptr;
				}

				twoSided->setResultType(twoSided->getLeft()->getResultType());
			}
		}

		else if(expr->as <Operator> ()->isOneSided())
		{
			auto oneSided = expr->as <OneSidedOperator> ();

			oneSided->setExpression(validateExpression(oneSided->getExpression(), state));
			if(!oneSided->getExpression()) return nullptr;

			// If the one sided operator is a function call, special handling might be needed.
			if(oneSided->getType() == OneSidedOperator::Type::FunctionCall)
			{
				// Are we calling a reference to a declaration?
				if(oneSided->getExpression()->isDeclarationReference())
				{
					auto ref = oneSided->getExpression()->as <DeclarationReference> ();

					// Functions are always callable.
					if(ref->getDeclaration()->isFunction())
					{
						oneSided->setResultType(ref->getResultType());
						return oneSided;
					}
				}
			}

			auto& resultType = oneSided->getExpression()->getResultType();

			if(!resultType.hasOperator(oneSided->getType()))
			{
				state.events.emit(InvalidOperatorOverload(oneSided, resultType));
				return nullptr;
			}

			oneSided->setResultType(oneSided->getExpression()->getResultType());
		}
	}

	return expr;
}

}
