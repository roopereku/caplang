#ifndef CAP_NODE_HH
#define CAP_NODE_HH

#include <cap/Token.hh>

#include <memory>

namespace cap
{

class ParserState;

class Node : public std::enable_shared_from_this <Node>
{
public:
	Node(Token&& token) : token(std::move(token))
	{
		static unsigned ids = 0;
		ids++;

		id = ids;
		//printf("Create node %u\n", id);
	}

	~Node()
	{
	}

	template <typename T>
	std::shared_ptr <T> as()
	{
		return std::static_pointer_cast <T> (shared_from_this());
	}

	template <typename T>
	std::shared_ptr <T> createNext(Token&& token)
	{
		next = std::make_shared <T> (std::move(token));
		next->parent = shared_from_this();

		return std::static_pointer_cast <T> (next);
	}

	virtual bool handleToken(Token&& token, ParserState& state);

	std::shared_ptr <Node> getParent()
	{
		return parent;
	}

	bool hasNext()
	{
		return static_cast <bool> (next);
	}

	std::shared_ptr <Node> getNext()
	{
		return next;
	}

	std::shared_ptr <Node> findLastNode();

	virtual bool isExpression()
	{
		return false;
	}

	virtual bool isDeclaration()
	{
		return false;
	}

	virtual bool isStatement()
	{
		return false;
	}

	std::string getToken()
	{
		return token.getString();
	}

	void adopt(std::shared_ptr <Node> node);

	unsigned id;

protected:
	Token token;

	std::shared_ptr <Node> parent;
	std::shared_ptr <Node> next;
};

}

#endif
