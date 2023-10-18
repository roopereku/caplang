#include <cap/Token.hh>

namespace cap
{

Token::Token(Type t, std::string_view data, IndexType row, IndexType column) :
	type(t), data(std::move(data)), row(row), column(column)
{
}

Token::Token(Token&& rhs) :
	type(rhs.type), data(std::move(rhs.data)),
	row(rhs.row), column(rhs.column)
{
	rhs.type = Type::Invalid;
	rhs.row = 0;
	rhs.column = 0;
}

Token Token::createInvalid()
{
	Token invalid;
	invalid.type = Type::Invalid;

	return invalid;
}

const char* Token::getTypeString() const
{
	switch(type)
	{
		case Type::Identifier: return "Identifier";
		case Type::Comment: return "Comment";
		case Type::Operator: return "Operator";
		case Type::Character: return "Character";
		case Type::String: return "String";
		case Type::CurlyBrace: return "Curly brace";
		case Type::Parenthesis: return "Parenthesis";
		case Type::SquareBracket: return "Square bracker";
		case Type::Invalid: return "Invalid";
		case Type::Float: return "Float";
		case Type::Integer: return "Integer";
	}

	return "???";
}

std::string Token::getString() const
{
	return data.empty() ? "<empty token>" : std::string(data);
}

bool Token::operator==(std::string_view rhs) const
{
	return data == rhs;
}

bool Token::operator==(const Token& rhs) const
{
	return data == rhs.data;
}

std::string_view::value_type Token::operator[](size_t index) const
{
	return data[index];
}

}
