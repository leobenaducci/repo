#pragma once

namespace detail
{
	//Helpers
	template<class T>
	static auto remove_array(T) -> const T {}

	template<class T, size_t N>
	static auto remove_array(const T(&)[N]) -> const T {}

	template<class T, size_t N, size_t M>
	static auto remove_array(const T(&)[N][M]) -> const T {}
}

#if 0

#include "compile_time_string.h"

namespace detail
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
		constexpr size_t padleft_ = 96 + type_size;
		constexpr size_t padleft = padleft_ + (__FUNCSIG__[padleft_] == ' ' ? 1 : 0);
		constexpr size_t padright = 8 + padleft;
		constexpr size_t size = sizeof(__FUNCSIG__) - padright;
		constexpr CT_SUB_STRING(__FUNCSIG__, padleft, size) type_name;
		constexpr CT_STRING(__FUNCSIG__) func_sig;
		return type_name.string_v;
		return func_sig.string_v;
	}
}

template<typename T>
constexpr std::string_view get_type_name(const T type)
{
	return detail::get_type_name<T>();
}

template<typename T>
constexpr std::string_view get_type_name()
{
	return detail::get_type_name<T>();
}

#else

template<typename T>
constexpr std::string_view get_type_name()
{
	using base_type = std::remove_pointer_t< decltype(detail::remove_array(std::declval<T>())) >;
	constexpr size_t type_size = (std::is_class<base_type>() ? 6 : std::is_enum<base_type>() ? 5 : 0);
	constexpr size_t padleft_ = 88 + type_size;
	constexpr size_t padleft = padleft_ + (__FUNCSIG__[padleft_] == ' ' ? 1 : 0);
	constexpr size_t padright = 8 + padleft;
	constexpr size_t size = sizeof(__FUNCSIG__) - padright;
	return std::string_view(__FUNCSIG__ + padleft, size);
}

template<typename T>
constexpr std::string_view get_type_name(T type)
{
	return get_type_name<T>();
}

#endif