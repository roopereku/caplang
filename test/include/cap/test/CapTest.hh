#ifndef CAP_TEST_CAP_TEST_HH
#define CAP_TEST_CAP_TEST_HH

#include <cap/Client.hh>
#include <cap/test/NodeMatcher.hh>

#include <gtest/gtest.h>

#include <queue>

namespace cap::test
{

class TestBase : public cap::Client
{
protected:
    bool parse(std::wstring&& src);
};

class ErrorTest : public TestBase
{
public:
    void onSourceError(cap::SourceLocation&, const std::wstring& msg) override;
    void reportsError(std::wstring&& src, const std::wstring& error, bool inGlobalScope = false);

private:
    std::queue<std::wstring> errors;
};

class PreValidationTest : public TestBase
{
public:
    void matches(std::wstring&& str, std::vector<ExpectedNode>&& expected);
    void enclosedMatches(std::wstring&& str, std::vector<ExpectedNode>&& expected);
};

class PostValidationTest : public TestBase
{
public:
    void enclosedMatches(std::wstring&& str, std::vector<ExpectedNode>&& expected);

    void setup(std::wstring_view str) { setupSrc = std::wstring(str); }

private:
    std::wstring setupSrc;
};

} // namespace cap::test

#define CAP_TEST_IMPL_NAME(testType, testName) run##testType##Test##testName

#define CAP_TEST_SIGNATURE(testType, testName) \
    void CAP_TEST_IMPL_NAME(testType, testName)(cap::test::testType##Test & test)

#define CAP_TEST(testType, testName)                  \
    CAP_TEST_SIGNATURE(testType, testName);           \
    TEST(testType##Tests, testName)                   \
    {                                                 \
        cap::test::testType##Test test;               \
        CAP_TEST_IMPL_NAME(testType, testName)(test); \
    }                                                 \
    CAP_TEST_SIGNATURE(testType, testName)

#endif
