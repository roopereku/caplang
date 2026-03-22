#include <cap/ArgumentAccessor.hh>
#include <cap/BinaryOperator.hh>
#include <cap/Variable.hh>

#include <cassert>

namespace cap
{

CommaAccessor::CommaAccessor(std::shared_ptr <Expression::Root> root)
{
    assert(root);

    m_origin = root;
    m_current = root->getFirst();

    locateFirst();
}

CommaAccessor::CommaAccessor(std::shared_ptr <Variable::Root> root)
{
    assert(root);
    assert(root->getInitializer());

    m_origin = root->getInitializer();
    m_current = root->getInitializer()->getFirst();

    locateFirst();
}

std::shared_ptr <Expression> CommaAccessor::getNext()
{
    // Are we at the stopping point?
    if (m_current == m_origin)
    {
        return nullptr;
    }

    auto result = m_current;
    assert(m_current);

    if (m_current->getType() == Expression::Type::BinaryOperator)
    {
        auto op = std::static_pointer_cast<BinaryOperator>(m_current);
        if (op->getType() == BinaryOperator::Type::Comma)
        {
            // Since the current will always be initially a non-comma node,
            // if the current node is a comma we can assume that the first
            // node which would be the only left side value is handled.
            result = op->getRight();
        }
    }

    // Move on to the next argument if commas are present. Else the next
    // getNext call should hit the origin.
    m_current = std::static_pointer_cast<Expression>(m_current->getParent().lock());
    return result;
}

void CommaAccessor::locateFirst()
{
    // Nothing to locate.
    if (m_current == nullptr)
    {
        m_origin = nullptr;
        return;
    }

    // Find the value of the leftmost comma if any.
    // This will result in the first argument in case there are multiple.
    while (m_current->getType() == Expression::Type::BinaryOperator)
    {
        auto op = std::static_pointer_cast<BinaryOperator>(m_current);
        if (op->getType() == BinaryOperator::Type::Comma)
        {
            m_current = op->getLeft();
            continue;
        }

        break;
    }
}

} // namespace cap
