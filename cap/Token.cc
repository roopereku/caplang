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
	invalid.row = 0;
	invalid.column = 0;
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
		case Type::SquareBracket: return "Square bracket";
		case Type::Invalid: return "Invalid";
		case Type::Binary: return "Binary";
		case Type::Float: return "Float";
		case Type::Double: return "Double";
		case Type::Integer: return "Integer";
		case Type::Hexadecimal: return "Hexadecimal";
	}

	return "???";
}

std::string_view Token::getStringView() const
{
	return data.empty() ? "<empty token>" : data;
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

bool Token::operator==(const Type rhsType) const
{
	return type == rhsType;
}

std::string_view::value_type Token::operator[](size_t index) const
{
	return data[index];
}

}
