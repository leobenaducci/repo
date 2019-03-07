#include <string>
#include <string_view>
#include <type_traits>
#include <any>

#include "compile_time_string.h"
#include "compile_time_type_name.h"
#include "compile_time_enum_to_string.h"

#include "reflection_detail.h"
#include "reflection_serializer_string.h"

#define REFLECTION_EXPAND(x) x
#define REFLECTION_GET_NTH_ARG(  _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, \
								_11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
								_21, _22, _23, _24, _25, _26, _27, _28, _29, _30, \
								_31, _32, _33, _34, _35, _36, _37, _38, _39, _40, \
								_41, _42, _43, _44, _45, _46, _47, _48, _49, _50, \
								_51, _52, _53, _54, _55, _56, _57, _58, _59, _60, \
								N, ...) N

#define REFLECTION_DECLTYPE(C, V) decltype(::reflection_detail::reflection_decltype<C, ::reflection_detail::GetMemberType<decltype(&V)>()>::type(&V).type_v)
#define REFLECTION_OFFSETOFF(C, V) ::reflection_detail::reflection_offsetof_t<C, REFLECTION_DECLTYPE(C, V), decltype(&C::V), &C::V>
#define REFLECTION_SIZEOF(C, V) ::reflection_detail::reflection_sizeof_t<C, REFLECTION_DECLTYPE(C, V), decltype(&C::V), &C::V>
#define REFLECTION_TYPE_T(var) ::reflection_detail::reflection_type<this_class, REFLECTION_DECLTYPE(this_class, var), REFLECTION_OFFSETOFF(this_class, var), REFLECTION_SIZEOF(this_class, var), CT_STRING(#var), ::reflection_detail::GetMemberType<decltype(&var)>()>

#define REFLECTION_TYPE0() 
#define REFLECTION_TYPE1(var1) REFLECTION_TYPE_T(var1)
#define REFLECTION_TYPE2(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE1(__VA_ARGS__))
#define REFLECTION_TYPE3(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE2(__VA_ARGS__))
#define REFLECTION_TYPE4(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE3(__VA_ARGS__))
#define REFLECTION_TYPE5(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE4(__VA_ARGS__))
#define REFLECTION_TYPE6(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE5(__VA_ARGS__))
#define REFLECTION_TYPE7(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE6(__VA_ARGS__))
#define REFLECTION_TYPE8(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE7(__VA_ARGS__))
#define REFLECTION_TYPE9(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE8(__VA_ARGS__))
#define REFLECTION_TYPE10(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE9(__VA_ARGS__))
#define REFLECTION_TYPE11(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE10(__VA_ARGS__))
#define REFLECTION_TYPE12(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE11(__VA_ARGS__))
#define REFLECTION_TYPE13(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE12(__VA_ARGS__))
#define REFLECTION_TYPE14(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE13(__VA_ARGS__))
#define REFLECTION_TYPE15(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE14(__VA_ARGS__))
#define REFLECTION_TYPE16(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE15(__VA_ARGS__))
#define REFLECTION_TYPE17(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE16(__VA_ARGS__))
#define REFLECTION_TYPE18(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE17(__VA_ARGS__))
#define REFLECTION_TYPE19(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE18(__VA_ARGS__))
#define REFLECTION_TYPE20(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE19(__VA_ARGS__))
#define REFLECTION_TYPE21(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE20(__VA_ARGS__))
#define REFLECTION_TYPE22(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE21(__VA_ARGS__))
#define REFLECTION_TYPE23(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE22(__VA_ARGS__))
#define REFLECTION_TYPE24(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE23(__VA_ARGS__))
#define REFLECTION_TYPE25(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE24(__VA_ARGS__))
#define REFLECTION_TYPE26(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE25(__VA_ARGS__))
#define REFLECTION_TYPE27(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE26(__VA_ARGS__))
#define REFLECTION_TYPE28(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE27(__VA_ARGS__))
#define REFLECTION_TYPE29(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE28(__VA_ARGS__))
#define REFLECTION_TYPE30(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE29(__VA_ARGS__))
#define REFLECTION_TYPE31(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE30(__VA_ARGS__))
#define REFLECTION_TYPE32(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE31(__VA_ARGS__))
#define REFLECTION_TYPE33(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE32(__VA_ARGS__))
#define REFLECTION_TYPE34(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE33(__VA_ARGS__))
#define REFLECTION_TYPE35(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE34(__VA_ARGS__))
#define REFLECTION_TYPE36(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE35(__VA_ARGS__))
#define REFLECTION_TYPE37(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE36(__VA_ARGS__))
#define REFLECTION_TYPE38(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE37(__VA_ARGS__))
#define REFLECTION_TYPE39(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE38(__VA_ARGS__))
#define REFLECTION_TYPE40(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE39(__VA_ARGS__))
#define REFLECTION_TYPE41(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE40(__VA_ARGS__))
#define REFLECTION_TYPE42(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE41(__VA_ARGS__))
#define REFLECTION_TYPE43(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE42(__VA_ARGS__))
#define REFLECTION_TYPE44(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE43(__VA_ARGS__))
#define REFLECTION_TYPE45(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE44(__VA_ARGS__))
#define REFLECTION_TYPE46(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE45(__VA_ARGS__))
#define REFLECTION_TYPE47(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE46(__VA_ARGS__))
#define REFLECTION_TYPE48(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE47(__VA_ARGS__))
#define REFLECTION_TYPE49(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE48(__VA_ARGS__))
#define REFLECTION_TYPE50(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE49(__VA_ARGS__))
#define REFLECTION_TYPE51(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE50(__VA_ARGS__))
#define REFLECTION_TYPE52(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE51(__VA_ARGS__))
#define REFLECTION_TYPE53(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE52(__VA_ARGS__))
#define REFLECTION_TYPE54(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE53(__VA_ARGS__))
#define REFLECTION_TYPE55(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE54(__VA_ARGS__))
#define REFLECTION_TYPE56(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE55(__VA_ARGS__))
#define REFLECTION_TYPE57(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE56(__VA_ARGS__))
#define REFLECTION_TYPE58(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE57(__VA_ARGS__))
#define REFLECTION_TYPE59(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE58(__VA_ARGS__))
#define REFLECTION_TYPE60(var1, ...) REFLECTION_TYPE_T(var1), REFLECTION_EXPAND(REFLECTION_TYPE59(__VA_ARGS__))

#define REFLECTION_TYPE(...) \
    using meta = reflection_detail::reflection_type_array< \
    REFLECTION_EXPAND(REFLECTION_GET_NTH_ARG(__VA_ARGS__, \
	REFLECTION_TYPE60, REFLECTION_TYPE59, REFLECTION_TYPE58, REFLECTION_TYPE57, REFLECTION_TYPE56, \
    REFLECTION_TYPE55, REFLECTION_TYPE54, REFLECTION_TYPE53, REFLECTION_TYPE52, REFLECTION_TYPE51, \
	REFLECTION_TYPE50, REFLECTION_TYPE49, REFLECTION_TYPE48, REFLECTION_TYPE47, REFLECTION_TYPE46, \
    REFLECTION_TYPE45, REFLECTION_TYPE44, REFLECTION_TYPE43, REFLECTION_TYPE42, REFLECTION_TYPE41, \
	REFLECTION_TYPE40, REFLECTION_TYPE39, REFLECTION_TYPE38, REFLECTION_TYPE37, REFLECTION_TYPE36, \
    REFLECTION_TYPE35, REFLECTION_TYPE34, REFLECTION_TYPE33, REFLECTION_TYPE32, REFLECTION_TYPE31, \
	REFLECTION_TYPE30, REFLECTION_TYPE29, REFLECTION_TYPE28, REFLECTION_TYPE27, REFLECTION_TYPE26, \
    REFLECTION_TYPE25, REFLECTION_TYPE24, REFLECTION_TYPE23, REFLECTION_TYPE22, REFLECTION_TYPE21, \
	REFLECTION_TYPE20, REFLECTION_TYPE19, REFLECTION_TYPE18, REFLECTION_TYPE17, REFLECTION_TYPE16, \
    REFLECTION_TYPE15, REFLECTION_TYPE14, REFLECTION_TYPE13, REFLECTION_TYPE12, REFLECTION_TYPE11, \
	REFLECTION_TYPE10, REFLECTION_TYPE9, REFLECTION_TYPE8, REFLECTION_TYPE7, REFLECTION_TYPE6, REFLECTION_TYPE5, \
    REFLECTION_TYPE4, REFLECTION_TYPE3, REFLECTION_TYPE2, REFLECTION_TYPE1, REFLECTION_TYPE0)(__VA_ARGS__) \
    )>;

#define REFLECTION_SUPER(x) \
    using reflection_super = reflection_super_t<x>::type;\
    template<typename T> struct reflection_super_t { using type = x; };\
    template<> struct reflection_super_t<x> { using type = reflection_super_t<x>; };

#define REFLECT(THIS_CLASS, ...) \
    REFLECTION_SUPER(THIS_CLASS) \
    constexpr static auto meta_reflection() { \
        using this_class = THIS_CLASS; \
        struct meta_t : reflection_meta_t{ \
        REFLECTION_EXPAND(REFLECTION_TYPE(__VA_ARGS__)); }; \
        return meta_t(); } \
    using this_reflection = reflection_detail::reflection<THIS_CLASS>; \
	virtual std::string_view reflection_get_type_name() const { return get_type_name<THIS_CLASS>(); }; \
	virtual void reflection_serialize_to(std::string& result) const { reflection_serializers::string_serializer serializer; serializer.save<THIS_CLASS>(*this); result = serializer.result; };

struct reflection_meta_t {};
template<typename T> struct reflection_super_t { using type = void; };

template<typename T>
class static_reflection
{
public:
	using meta = typename reflection_detail::reflection<T>::meta;

	static constexpr size_t num_members = meta::num_members;

	static auto get_member_name(size_t N) { return meta::get_member_name(N); }
	static auto get_member_size(size_t N) { return meta::get_member_size(N); }
	static auto get_member_type_name(size_t N) { return meta::get_member_type_name(N); }
	static auto get_member_offset(size_t N) { return meta::get_member_offset(N); }
	static auto get_member_type(size_t N) { return meta::get_member_type(N); }
	static auto is_member_const(size_t N) { return meta::is_member_const(N); }
};

template<typename T>
class runtime_reflection : public static_reflection<T>
{
public:
	using anytype = reflection_detail::anytype;
	using meta = typename reflection_detail::reflection<T>::meta;

	static auto get_member_value(const T& obj, size_t N)
	{
		return meta::get_member_value(obj, N);
	}

	static auto get_member_value(const T& obj, std::string_view name)
	{
		for (size_t i = 0; i < num_members; i++)
		{
			if (get_member_name(i) == name)
				return meta::get_member_value(obj, i);
		}

		return reflection_detail::anytype(nullptr);
	}
};

