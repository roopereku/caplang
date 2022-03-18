#ifndef FLAG_LIST_HEADER
#define FLAG_LIST_HEADER

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
		value |= (1 << static_cast <unsigned> (type));
	}

	constexpr bool contains(T type) const
	{
		const unsigned result = 1 << static_cast <unsigned> (type);
		return (value & result) == result;
	}

	constexpr bool empty() const
	{
		return value == 0;
	}

private:
	unsigned value = 0;
};

}

#endif
