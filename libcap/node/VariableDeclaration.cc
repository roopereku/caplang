#include <cap/node/VariableDeclaration.hh>

namespace cap
{

bool VariableDeclaration::handleToken(Token&& token, ParserState& state)
{
	printf("[VariableDeclaration] Try '%s'\n", token.getString().c_str());

	// If there's not a next node, try to create one.
	if(initialization)
	{
		printf("??? VariableDeclaration already has initialization\n");
		return false;
	}

	// The node following a variable declaration has to be an assignment.
	if(token.getType() != Token::Type::Operator || !(token == "="))
	{
		printf("Expected '=' after a variable declaration\n");
		return false;
	}

	// Create the assignment node and make this variable its target.
	initialization = parseToken(std::move(token), state);
	//initialization->setTarget(std::static_pointer_cast <Expression> (shared_from_this()));

	return true;
}

}
