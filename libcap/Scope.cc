#include <cap/Type.hh>
#include <cap/Scope.hh>
#include <cap/Function.hh>

#include <cap/node/ExpressionRoot.hh>
#include <cap/node/VariableDeclaration.hh>
#include <cap/node/FunctionDeclaration.hh>
#include <cap/node/TypeDeclaration.hh>
#include <cap/node/FunctionCall.hh>
#include <cap/node/Subscript.hh>
#include <cap/node/Return.hh>

namespace cap
{

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
	state.node = state.node->createNext <FunctionDeclaration> (std::move(token));

	Token name = consumeName(state.tokens);
	scopes.emplace_back(std::make_shared <Function> (*this, std::move(name)));

	printf("-------------------------- START PARSING FUNCTION --------------------------------------------\n");

	auto function = std::static_pointer_cast <Function> (scopes.back());
	ParserState newState(state.tokens, function->getRoot());
	std::static_pointer_cast <FunctionDeclaration> (state.node)->function = function;

	bool ret = function->parse(newState);
	printf("-------------------------- STOP PARSING FUNCTION ---------------------------------------------\n");

	return ret;
}

bool Scope::createType(Token&& token, ParserState& state)
{
	state.node = state.node->createNext <TypeDeclaration> (std::move(token));

	Token name = consumeName(state.tokens);
	scopes.emplace_back(std::make_shared <Type> (*this, std::move(name)));

	printf("-------------------------- START PARSING TYPE---- --------------------------------------------\n");

	auto type = std::static_pointer_cast <Type> (scopes.back());
	ParserState newState(state.tokens, type->getRoot());
	std::static_pointer_cast <TypeDeclaration> (state.node)->type = type;

	bool ret = type->parse(newState);
	printf("-------------------------- STOP PARSING TYPE --------------------------------------------------\n");

	return ret;

}

bool Scope::createVariable(ParserState& state)
{
	Token name = consumeName(state.tokens);
	state.node = state.node->createNext <VariableDeclaration> (std::move(name));

	return true;
}

bool Scope::parse(Tokenizer& tokens)
{
	ParserState state(tokens, root);
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
			if(!state.braces.close(std::move(token)))
				return false;

			// End the expression when a brace is closed
			if(state.inExpression && !state.endExpression())
				return false;
		}

		// The current token is not a brace.
		else
		{
			printf("Token %s '%s'\n", token.getTypeString(), token.getString().c_str());

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
					if(!createVariable(state))
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
					auto returnNode = std::static_pointer_cast <Return> (state.node);

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
					printf("??? No current node\n");
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
		printf("Unterminated brace '%c'\n", state.braces.getOpener()[0]);
		return false;
	}

	return true;
}

bool Scope::parseBracket(Token&& token, ParserState& state)
{
	Token::Type t = token.getType();
	auto inBraces = std::make_shared <ExpressionRoot> (Token::createInvalid());

	ParserState newState(state.tokens, inBraces);
	newState.initExpression(token.getRow());
	newState.canEndExpression = false;

	// If there was a value previously, the brackets might indicate
	// a function call or a subscript operator.
	if(state.previousIsValue)
	{
		if(!state.inExpression)
		{
			printf("??? Not in expression\n");
			return false;
		}

		std::shared_ptr <OneSidedOperator> op;

		if(t == Token::Type::Parenthesis)
		{
			printf("FUNCTION CALL\n");
			op = std::make_shared <FunctionCall> (Token::createInvalid());
			std::static_pointer_cast <FunctionCall> (op)->setParameters(inBraces);
		}

		else if(t == Token::Type::SquareBracket)
		{
			printf("SUBSCRIPT\n");
			op = std::make_shared <Subscript> (Token::createInvalid());
			std::static_pointer_cast <Subscript> (op)->setContents(inBraces);
		}

		else
		{
			printf("??? {} after value not implemented\n");
			return false;
		}

		if(!std::static_pointer_cast <Expression> (state.node)->handleExpressionNode(op, state))
			return false;
	}

	state.previousIsValue = true;
	Token::IndexType row = token.getRow();

	if(!newState.braces.open(std::move(token)))
		return false;

	printf("------------------- START BRACES ------------------------\n");

	if(!Scope::parse(newState))
		return false;

	printf("------------------- END BRACES --------------------------\n");

	if(state.inExpression)
	{
		auto expr = std::static_pointer_cast <Expression> (state.node);
		bool setAfterCurrent = true;

		if(expr->isOperator())
		{
			auto op = std::static_pointer_cast <Operator> (expr);

			if(op->isOneSided())
			{
				auto oneSided = std::static_pointer_cast <OneSidedOperator> (op);

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
		if(!std::static_pointer_cast <Expression> (state.node)->handleExpressionNode(std::move(inBraces), state))
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

}
