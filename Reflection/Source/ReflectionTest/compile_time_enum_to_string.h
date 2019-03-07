#include <limits>

#pragma once

namespace enum_to_string_detail
{
	//Helpers
	template<class T>
	static auto remove_array(T) -> const T {}

	template<class T, size_t N>
	static auto remove_array(const T(&)[N]) -> const T {}

	template<class T, size_t N, size_t M>
	static auto remove_array(const T(&)[N][M]) -> const T {}

	template<typename T>
	constexpr std::string_view get_type_name()
	{
		using base_type = std::remove_pointer_t< decltype(remove_array(std::declval<T>())) >;
		constexpr size_t type_size = (std::is_class<base_type>() ? 6 : std::is_enum<base_type>() ? 5 : 0);
		constexpr size_t padleft_ = 111 + type_size;
		constexpr size_t padleft = padleft_ + (__FUNCSIG__[padleft_] == ' ' ? 1 : 0);
		constexpr size_t padright = 8 + padleft;
		constexpr size_t size = sizeof(__FUNCSIG__) - padright;
		return std::string_view(__FUNCSIG__ + padleft, size);
	}

	template<typename T, T N>
	constexpr std::string_view enum_to_string_func()
	{
		constexpr std::string_view type_name = get_type_name<T>();
		constexpr size_t padleft = 123 + type_name.size() * 2 + 2;
		constexpr size_t padright = 8;
		constexpr size_t size = sizeof(__FUNCSIG__) - padright - padleft;
		return std::string_view(__FUNCSIG__ + padleft, size);
	}

	/*template<typename T>
	struct enum_to_string
	{
		template<size_t S>
		struct get
		{
			constexpr static std::string_view value(T Value)
			{
				if ((size_t)Value == S)
					return enum_to_string_func<T, (T)S>();
				return enum_to_string<T>::get<(S - 1)>::value(Value);
			}
		};

		template<>
		struct get<0>
		{
			constexpr static std::string_view value(T Value)
			{
				return enum_to_string_func<T, (T)0>();
			}
		};
	};*/

	template<typename T, int Min, int Max>
	constexpr std::string_view enum_to_string(T Value)
	{
		constexpr int Mid = Min/2 + Max/2;
		if ((int)Value == Min)
			return enum_to_string_func<T, (T)Min>();
		else if ((int)Value == Mid)
			return enum_to_string_func<T, (T)Mid>();
		else if ((int)Value < Mid)
			return enum_to_string<T, Min, Mid-1>(Value);
		else if ((int)Value > Mid)
			return enum_to_string<T, Mid+1, Max>(Value);
		return "";
	}
}


template<typename T>
constexpr std::string_view enum_to_string(T Value)
{
	return enum_to_string_detail::enum_to_string<T, 0, (int)T::Max>(Value);
}
