#ifndef CAP_REFERENCE_HH
#define CAP_REFERENCE_HH

#include <cap/node/TypeDefinition.hh>

namespace cap
{

class Node;

/// Reference points to a node and keeps context of what kind of
/// an entity it is. For an example a type definition could be referred.
class Reference
{
public:
	enum class Type
	{
		Variable,
		Parameter,
		Alias,

		TypeDefinition,
		FunctionDefinition,

		None
	};

	Reference();
	Reference(std::shared_ptr <Node> node);
	Reference(Type type, std::shared_ptr <Node> node);

	/// Implicit conversion to boolean.
	///
	/// \return True if there is a type other than None.
	operator bool();

	/// Gets the associated type of the reference.
	///
	/// \return The associated type or nullptr if there is none.
	std::shared_ptr <TypeDefinition> getAssociatedType();

	/// Gets the referred node.
	///
	/// \return The referred node.
	std::shared_ptr <Node> getReferred();

	/// Gets the name of the referred node if any.
	///
	/// \return The name of the referred node or an invalid token.
	Token getReferredName();

	/// Gets the reference type.
	///
	/// \return The reference type.
	Type getType();

	/// Gets the reference type as a string.
	///
	/// \return The reference type as a string.
	const char* getTypeString();

private:
	std::shared_ptr <Node> referred;
	Type type;
};

}

#endif
