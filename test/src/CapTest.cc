#include <cap/Function.hh>
#include <cap/Scope.hh>
#include <cap/test/CapTest.hh>
#include <cap/test/DynamicSource.hh>

namespace cap::test
{

bool TestBase::parse(std::wstring&& src)
{
    SCOPED_TRACE(src.c_str());

    DynamicSource source(std::move(src));
    return source.parse(*this);
}

void ErrorTest::onSourceError(cap::SourceLocation&, const std::wstring& msg)
{
    // TODO: Store location?
    errors.emplace(msg);
}

void ErrorTest::reportsError(std::wstring&& src, const std::wstring& error, bool inGlobalScope)
{
    if (inGlobalScope)
    {
        ASSERT_FALSE(parse(std::move(src)));
    }

    else
    {
        ASSERT_FALSE(parse(L"func test()\n{\n" + std::move(src) + L"\n}"));
    }

    ASSERT_FALSE(errors.empty());
    ASSERT_STREQ(error.c_str(), errors.front().c_str());
    errors.pop();
}

void PreValidationTest::matches(std::wstring&& str, std::vector<ExpectedNode>&& expected)
{
    cap::test::DynamicSource source;
    source += std::move(str);

    ASSERT_TRUE(source.parse(*this, false));

    expected.insert(expected.begin(), Scope());
    NodeMatcher matcher(std::move(expected));
    matcher.traverseNode(source.getGlobal());
}

void PreValidationTest::enclosedMatches(std::wstring&& str, std::vector<ExpectedNode>&& expected)
{
    static const auto expectedEnclosure = {Function(L"capTestEnclosure"), Scope()};

    str = L"func capTestEnclosure()\n{\n" + std::move(str) + L"\n}\n";
    expected.insert(expected.begin(), expectedEnclosure.begin(), expectedEnclosure.end());

    matches(std::move(str), std::move(expected));
}

void PostValidationTest::enclosedMatches(std::wstring&& str, std::vector<ExpectedNode>&& expected)
{
    cap::test::DynamicSource source;
    source += setupSrc;
    source += L"\nfunc capTestEnclosure()\n{\n";
    source += std::move(str);
    source += L"\n}\n";

    ASSERT_TRUE(source.parse(*this, true));

    expected.insert(expected.begin(), Scope());
    NodeMatcher matcher(std::move(expected));

    std::shared_ptr<cap::Function> enclosure;
    for (auto decl : source.getGlobal()->declarations)
    {
        if (decl->getName() == L"capTestEnclosure")
        {
            ASSERT_TRUE(decl->getType() == cap::Declaration::Type::Function);
            enclosure = std::static_pointer_cast<cap::Function>(decl);
        }
    }

    ASSERT_TRUE(enclosure);
    ASSERT_STREQ(enclosure->getName().c_str(), L"capTestEnclosure");
    ASSERT_TRUE(enclosure->getBody());
    matcher.traverseWithContext(enclosure->getBody(), this);
}

} // namespace cap::test
