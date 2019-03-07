#pragma once

#define CTS_ARRAY_GET(i, b, l, x) ((i<l)?x[b+i]:0)

#define CTS_STRING_ARRAY_4(i, x, b, l) CTS_ARRAY_GET(0+4*(i), b, l, x), CTS_ARRAY_GET(1+4*(i), b, l, x), CTS_ARRAY_GET(2+4*(i), b, l, x), CTS_ARRAY_GET(3+4*(i), b, l, x)
#define CTS_STRING_ARRAY_8(i, x, b, l) CTS_STRING_ARRAY_4(0+2*(i), x, b, l), CTS_STRING_ARRAY_4(1+2*(i), x, b, l)
#define CTS_STRING_ARRAY_16(i, x, b, l) CTS_STRING_ARRAY_8(0+2*(i), x, b, l), CTS_STRING_ARRAY_8(1+2*(i), x, b, l)
#define CTS_STRING_ARRAY_24(i, x, b, l) CTS_STRING_ARRAY_8(0+3*(i), x, b, l), CTS_STRING_ARRAY_8(1+3*(i), x, b, l), CTS_STRING_ARRAY_8(2+3*(i), x, b, l)
#define CTS_STRING_ARRAY_32(i, x, b, l) CTS_STRING_ARRAY_16(0+2*(i), x, b, l), CTS_STRING_ARRAY_16(1+2*(i), x, b, l)
#define CTS_STRING_ARRAY_48(i, x, b, l) CTS_STRING_ARRAY_24(0+2*(i), x, b, l), CTS_STRING_ARRAY_24(1+2*(i), x, b, l)
#define CTS_STRING_ARRAY_64(i, x, b, l) CTS_STRING_ARRAY_32(0+2*(i), x, b, l), CTS_STRING_ARRAY_32(1+2*(i), x, b, l)
#define CTS_STRING_ARRAY_96(i, x, b, l) CTS_STRING_ARRAY_48(0+2*i, x, b, l), CTS_STRING_ARRAY_48(1+2*i, x, b, l)
#define CTS_STRING_ARRAY_128(i, x, b, l) CTS_STRING_ARRAY_64(0+2*i, x, b, l), CTS_STRING_ARRAY_64(1+2*i, x, b, l)
#define CTS_STRING_ARRAY_256(i, x, b, l) CTS_STRING_ARRAY_128(0+2*i, x, b, l), CTS_STRING_ARRAY_128(1+2*i, x, b, l)
#define CTS_STRING_ARRAY_512(i, x, b, l) CTS_STRING_ARRAY_256(0+2*i, x, b, l), CTS_STRING_ARRAY_256(1+2*i, x, b, l)
#define CTS_STRING_ARRAY_1024(i, x, b, l) CTS_STRING_ARRAY_512(0+2*i, x, b, l), CTS_STRING_ARRAY_512(1+2*i, x, b, l)

#define CTS_STRING_TYPE(s, x, b, l) ct_string<l, CTS_STRING_ARRAY_##s(0, x, b, l) >

template<size_t L, char... Chars>
struct ct_string
{
public:
	static constexpr char data_v[sizeof...(Chars)] = { Chars... };
	static constexpr std::string_view string_v = std::string_view(data_v, L);

	static constexpr size_t size = sizeof(data_v);
	static constexpr size_t length = L;

	static const char* data() { return data_v; }
	
	static const char* begin() { return data_v; }
	static const char* cbegin() { return data_v; }

	static const char* end() { return &data_v[length]; }
	static const char* cend() { return &data_v[length]; }

	static const std::string_view& view() { return string_v; }

	template<typename T>
	static constexpr bool equals() { return std::is_same_v<ct_string<L, Chars...>, T>; }

	template<typename T>
	static constexpr bool equals(const T& S) { return equals<T>(); }

	template<size_t S>
	static constexpr bool equals(const char(&str)[S])
	{
		if (S != length) return false;
		for (size_t i = 0; i < S; i++)
			if (data_v[i] != str[i])
				return false;
		return true;
	}
};

#define CT_SUB_STRING(x, b, l)	std::conditional_t<l < 4, CTS_STRING_TYPE(4, x, b, l), \
								std::conditional_t<l < 8, CTS_STRING_TYPE(8, x, b, l), \
								std::conditional_t<l < 16, CTS_STRING_TYPE(16, x, b, l), \
								std::conditional_t<l < 24, CTS_STRING_TYPE(24, x, b, l), \
								std::conditional_t<l < 32, CTS_STRING_TYPE(32, x, b, l), \
								std::conditional_t<l < 48, CTS_STRING_TYPE(48, x, b, l), \
								std::conditional_t<l < 64, CTS_STRING_TYPE(64, x, b, l), \
								std::conditional_t<l < 96, CTS_STRING_TYPE(96, x, b, l), \
								std::conditional_t<l < 128, CTS_STRING_TYPE(128, x, b, l), \
								std::conditional_t<l < 256, CTS_STRING_TYPE(256, x, b, l), \
								std::conditional_t<l < 512, CTS_STRING_TYPE(512, x, b, l), \
								CTS_STRING_TYPE(1024, x, b, l)>>>>>>>>>>>

#define CT_STRING(x) CT_SUB_STRING(x, 0, sizeof(x))
