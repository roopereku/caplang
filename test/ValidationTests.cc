#include <gtest/gtest.h>

#include <cap/test/DynamicSource.hh>
#include <cap/test/NodeMatcher.hh>
#include <cap/Function.hh>
#include <cap/Client.hh>

using namespace cap::test;

class ValidationTester : public cap::Client
{
public:
	void setup(std::wstring&& src)
	{
		setupSrc = std::move(src);
	}

	void test(std::wstring&& src, std::vector <ExpectedNode>&& expectedExpr)
	{
		cap::test::DynamicSource source;
		source += setupSrc;
		source += L"\nfunc test()\n{\n";
		source += std::move(src);
		source += L"\n}\n";

		ASSERT_TRUE(source.parse(*this, true));

		expectedExpr.insert(expectedExpr.begin(), Scope());
		NodeMatcher matcher(std::move(expectedExpr));

		std::shared_ptr <cap::Scope> root;
		for(auto decl : source.getGlobal()->declarations)
		{
			if(decl->getName() == L"test")
			{
				ASSERT_TRUE(decl->getType() == cap::Declaration::Type::Function);
				auto func = std::static_pointer_cast <cap::Function> (decl);
				root = func->getBody();
			}
		}

		matcher.traverseNode(root);
	}

private:
	std::wstring setupSrc;
};

TEST(ValidationTests, AccessOperatorResultType)
{
	ValidationTester tester;

	tester.setup(LR"SRC(
		type Foo
		{
			type Baz
			{
				let a = Bar.a * 10
			}

			type Bar
			{
				let a = 10
			}
		}
	)SRC");

	tester.test(L"let a = Foo.Bar.a",
	{
		Expression(),
			LocalVariable(L"a"),
			cap::BinaryOperator::Type::Access > L"int64",
				cap::BinaryOperator::Type::Access > L"Foo.Bar",
					Value(L"Foo") > L"Foo",
					Value(L"Bar") > L"Foo.Bar",
				Value(L"a") > L"int64"
	});
}
