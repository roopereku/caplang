#include <gtest/gtest.h>

#include <cap/SourceFile.hh>
#include <cap/event/ErrorMessage.hh>

#include <iostream>

class EventValidator : public cap::EventEmitter
{
public:
	void emit(cap::Event&& event) override
	{
		if(event == cap::Event::Type::Message)
		{
			auto msg = event.as <cap::Message> ();

			if(msg.type == cap::Message::Type::Error)
			{
				std::cerr << "Error: " << msg.contents << '\n';
			}
		}
	}
};

TEST(CapTest, BasicProgram) {
	std::string source = R"(
		func main()
		{
			var foo = 12 * 50
			doSomething(foo)
		}	
	)";

	cap::SourceFile sourceFile(source);
	EventValidator events;

	ASSERT_TRUE(sourceFile.prepare(events));
}
