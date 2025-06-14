#ifndef CAP_INTEGER_HH
#define CAP_INTEGER_HH

#include <cap/Value.hh>

#include <cstdint>

namespace cap
{

class Integer : public Value
{
public:
	Integer(uint64_t value);

	/// Creates a new integer node from the given token.
	///
	/// \param ctx The parsing context to get the source from.
	/// \param token The token to parse an integer from.
	/// \return New integer or null.
	static std::shared_ptr <Integer> parse(ParserContext& ctx, Token& token);

	/// Gets the internal representation of the value.
	///
	/// \return The internal representation of the value.
	uint64_t getInternal() const;

	/// Sets the result type to the minimum which can hold the contained value.
	/// 
	/// \param ctx The context to retrieve builtins from.
	void updateResultType(cap::ParserContext& ctx);

	const char* getTypeString() const override;

private:
	uint64_t value = 0;
};

}

#endif
