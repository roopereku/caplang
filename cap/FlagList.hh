#ifndef FLAG_LIST_HEADER
#define FLAG_LIST_HEADER

#include <cstdint>

namespace Cap {

template <typename T>
class FlagList
{
public:
	template <typename... Args>
	constexpr FlagList(Args... args)
	{
		//	FIXME maybe support something older than C++17
		(add(args), ...);
	}

	constexpr FlagList()
	{
	}

	constexpr void add(T type)
	{
		value |= (1UL << static_cast <uint64_t> (type));
	}

	constexpr bool contains(T type) const
	{
		const uint64_t result = 1UL << static_cast <uint64_t> (type);
		return (value & result) == result;
	}

	constexpr bool empty() const
	{
		return value == 0UL;
	}

private:
	uint64_t value = 0UL;
};

}

#endif
