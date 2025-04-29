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
		VariableRoot
	};

	/// Gets the type of this statement.
	///
	/// \return The type of this statement.
	Type getType() const
	{
		return type;
	}

protected:
	Statement(Type type)
		: Node(Node::Type::Statement), type(type)
	{
	}

private:
	Type type;
};

}

#endif
