#ifndef CAP_STATEMENT_HH
#define CAP_STATEMENT_HH

#include <cap/Node.hh>

namespace cap
{

class Statement : public Node
{
public:
	enum class Type
	{
		VariableRoot,
		Return
	};

	/// Gets the type of this statement.
	///
	/// \return The type of this statement.
	Type getType() const;

	/// Determines the continuation of whether an expression will be parsed or not.
	/// This function implictly performs statement specific initialization.
	///
	/// \return This statement or the parent node if there's no further tokens on the same line.
	std::weak_ptr <Node> getContinuation(cap::ParserContext& ctx);

protected:
	Statement(Type type);

	/// Statement specific initialization.
	virtual bool onInitialize(cap::ParserContext& ctx, bool expectsTokens) = 0;

private:
	Type type;
};

}

#endif
