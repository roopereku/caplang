#ifndef CAP_STRING_HH
#define CAP_STRING_HH

#include <cap/Value.hh>

namespace cap
{

class String : public Value
{
public:
	String(std::wstring&& value);

	// TODO: Support string interpolation.

	/// Gets the string value.
	///
	/// \return String value.
	const std::wstring& getValue() const;

	const char* getTypeString() const override;

private:
	/// The string vallue with the outer quotes removed.
	std::wstring value;
};

}

#endif
