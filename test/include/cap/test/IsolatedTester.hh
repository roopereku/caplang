#ifndef CAP_TEST_ISOLATED_TESTER_HH
#define CAP_TEST_ISOLATED_TESTER_HH

#include <cap/Client.hh>
#include <cap/test/NodeMatcher.hh>
#include <cap/test/DynamicSource.hh>

namespace cap::test
{

// Isolated tester allows for testing activities within an isolated function.
// Additional setup can be done and will be excluded from testing.
class IsolatedTester : public Client
{
public:
	void setup(std::wstring&& src)
	{
		setupSrc = std::move(src);
	}
	
protected:
	std::shared_ptr <cap::Function> getIsolatedFunction(std::wstring&& src);

private:
	cap::test::DynamicSource source;
	std::wstring setupSrc;
};

}

#endif
