#ifndef CAP_EVENT_HH
#define CAP_EVENT_HH

#include <cap/Token.hh>

namespace cap
{

class Event
{
public:
	enum class Type
	{
		Message
	};

	Event(Type type, Token at)
		: type(type), at(at)
	{
	}

	/// Checks if this event is of the given type.
	///
	/// \param rhsType The type to check.
	/// \return True if the type matches.
	bool operator==(Type rhsType) const
	{
		return type == rhsType;
	}

	/// Helper function for converting an event to another type.
	///
	/// \return The converted event.
	template <typename T>
	T& as()
	{
		// TODO: Do a static assert that makes sure T is an event.
		return static_cast <T&> (*this);
	}

	/// The type of this event.
	const Type type;

	/// The location of this event.
	const Token at;
};

}

#endif
