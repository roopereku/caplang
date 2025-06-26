#include <cap/test/IsolatedTester.hh>
#include <cap/Function.hh>

#include <gtest/gtest.h>

namespace cap::test
{

std::shared_ptr <cap::Function> IsolatedTester::getIsolatedFunction(std::wstring&& src)
{
	source = cap::test::DynamicSource();

	source += setupSrc;
	source += L"\nfunc test()\n{\n";
	source += std::move(src);
	source += L"\n}\n";

	EXPECT_TRUE(source.parse(*this, true));

	std::shared_ptr <cap::Scope> root;
	for(auto decl : source.getGlobal()->declarations)
	{
		if(decl->getName() == L"test")
		{
			EXPECT_STREQ(decl->getTypeString(), "Function");
			auto func = std::static_pointer_cast <cap::Function> (decl);
			return func;
		}
	}

	ADD_FAILURE() << "Failed to find 'test' function";
	return nullptr;
}

}
