#ifndef CAP_NODE_HH
#define CAP_NODE_HH

#include <cap/Token.hh>

#include <memory>
#include <vector>

namespace cap
{

class Node
{
public:
	void associateToken(Token&& token);

private:
	std::vector <Token> tokens;

	std::unique_ptr <Node> left;
	std::unique_ptr <Node> right;
};

}

#endif
