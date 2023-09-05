#ifndef CAP_SCOPE_HH
#define CAP_SCOPE_HH

#include <cap/BraceMatcher.hh>
#include <cap/Tokenizer.hh>
#include <cap/Node.hh>

#include <memory>
#include <vector>

namespace cap
{

class Scope
{
public:
	Scope(Scope& parent) : parent(&parent)
	{
	}

	Scope() : parent(nullptr)
	{
	}

	/// Creates a new function from the next tokens.
	///
	/// \param tokens Tokenizer to get tokens from.
	/// \returns True if the function was created succesfully.
	bool createFunction(Tokenizer& tokens);

	/// Creates a new type from the next tokens.
	///
	/// \param tokens Tokenizer to get tokens from.
	/// \returns True if the type was created succesfully.
	bool createType(Tokenizer& tokens);


	virtual bool parse(Tokenizer& tokens, BraceMatcher& braces);

private:
	static Token consumeName(Tokenizer& tokens);

	std::vector <std::shared_ptr <Scope>> scopes;
	Scope* parent;
	Node root;
};

}

#endif
