#include <cap/Variable.hh>
#include <cap/Value.hh>

#include <cassert>

namespace cap
{

Variable::Variable(std::weak_ptr <Value> at)
	: Declaration(Type::Variable), at(at)
{
	assert(!at.expired());
	assert(at.lock()->getToken().getType() == Token::Type::Identifier);

	// TODO: Use move?
	name = at.lock()->getValue();
}

}
