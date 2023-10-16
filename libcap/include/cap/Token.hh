#ifndef CAP_TOKEN_HH
#define CAP_TOKEN_HH

#include <string_view>
#include <string>

namespace cap
{

class Token
{
public:
	using IndexType = size_t;

	enum class Type
	{
		Comment,
		Identifier,
		Operator,

		Character,
		String,
		Integer,
		Float,

		CurlyBrace,
		SquareBracket,
		Parenthesis,

		Invalid
	};

	Token(Type t, std::string_view data, IndexType row, IndexType column);
	Token(Token&& rhs);

	Token(const Token& rhs) = default;
	Token& operator=(const Token& rhs) = default;

	/// Creates an invalid token. This can be used to create an invalid
	/// fake token when next() returns something incorrect.
	///
	/// \returns Token with the type TokenType::Invalid.
	static Token createInvalid();

	/// Gets the token type represented as a string.
	///
	/// \returns The token type as a string.
	const char* getTypeString() const;

	/// Gets the contents of this token.
	///
	/// \returns std::string containing the contents of this token.
	std::string getString() const;

	/// Checks if the contents of this token matches a string.
	///
	/// \param rhs The string to compare against.
	/// \returns True if this token matches the given string.
	bool operator==(std::string_view rhs) const;

	/// Checks if the contents of this token matches the contents of another token.
	///
	/// \param rhs The token to compare against.
	/// \returns True if this token matches the given token.
	bool operator==(const Token& rhs) const;


	/// Gets the character at the given index.
	///
	/// \returns The character at index.
	std::string_view::value_type operator[](size_t index) const;

	Type getType() const
	{
		return type;
	}

	IndexType getRow() const
	{
		return row;
	}

	IndexType getColumn() const
	{
		return column;
	}

private:
	Token() {}

	Type type;
	std::string_view data;

	IndexType row;
	IndexType column;
};

}

#endif
