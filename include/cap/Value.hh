#ifndef CAP_VALUE_HH
#define CAP_VALUE_HH

#include <cap/Expression.hh>

namespace cap
{

class Declaration;

class Value : public Expression
{
public:
	enum class Type
	{
		Identifier,
		Integer,
		String
	};

	/// Creates a new value node from the given token.
	///
	/// \param ctx The parsing context to get the source from.
	/// \param token The token to parse a value from.
	/// \return New value or null.
	static std::shared_ptr <Value> create(ParserContext& ctx, Token& token);

	/// Should never be called as values cannot be the "current node".
	///
	/// \return False.
	bool isComplete() const override;

	/// Gets the type of this value node.
	///
	/// \return The type of this value node.
	Type getType() const;

	/// Gets the contained value as a string.
	///
	/// \return The contained value as a string.
	virtual std::wstring getString() const = 0;

protected:
	Value(Type type);

private:
	Type type;
};

}

#endif
