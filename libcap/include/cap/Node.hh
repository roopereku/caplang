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
	}

	~Node()
	{
	}

	template <typename T>
	std::shared_ptr <T> createNext(Token&& token)
	{
		auto ptr = std::make_shared <T> (std::move(token));
		ptr->parent = shared_from_this();

		return std::static_pointer_cast <T> (ptr);
	}

	virtual bool handleToken(Token&& token, ParserState& state);

	bool hasNext()
	{
		return static_cast <bool> (next);
	}

	std::shared_ptr <Node> getNext()
	{
		return next;
	}

	virtual bool isVariable()
	{
		return false;
	}

	virtual bool isArithmeticOperation()
	{
		return false;
	}

	virtual bool isAssignment()
	{
		return false;
	}

	std::string getToken()
	{
		return token.getString();
	}

	void adopt(std::shared_ptr <Node> node);

protected:
	Token token;

	std::shared_ptr <Node> parent;
	std::shared_ptr <Node> next;
};

}

#endif
