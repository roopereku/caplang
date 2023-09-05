#ifndef CAP_NAMED_SCOPE_HH
#define CAP_NAMED_SCOPE_HH

#include <cap/Scope.hh>
#include <cap/Token.hh>

namespace cap
{

class NamedScope : public Scope
{
public:
	NamedScope(Scope& parent, Token&& name) : Scope(parent), name(std::move(name))
	{
	}

protected:
	const Token name;
};

}

#endif
