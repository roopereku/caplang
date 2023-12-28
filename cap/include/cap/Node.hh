#ifndef CAP_NODE_HH
#define CAP_NODE_HH

#include <memory>

namespace cap
{

/// Node is a part of an abstract syntax tree.
class Node : public std::enable_shared_from_this <Node>
{
public:
	enum class Type
	{
		Empty,
		ScopeDefinition
	};

	Node();
	Node(Type type);

	std::shared_ptr <Node> findLast();
	void adopt(std::shared_ptr <Node> node);
	std::weak_ptr <Node> getParent() const;

	std::shared_ptr <Node> getNext() const;
	void setNext(std::shared_ptr <Node>&& node);

	/// The type of the node.
	const Type type;

private:
	std::shared_ptr <Node> next;
	std::weak_ptr <Node> parent;
};

}

#endif
