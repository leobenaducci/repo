#include <stdio.h>
#include <string>
#include <functional>
#include <typeinfo>
#include <string_view>
#include <type_traits>
#include <vector>
#include <array>

#include <Windows.h>

#include "compile_time_string.h"
#include "compile_time_type_name.h"

#include "reflection.h"

typedef int int4[4];

template<typename T>
class vector : public std::vector<T, std::allocator<T>>
{
public:
	static void custom_serialization() {}
	static void reflection_as_array() {}

	vector() {}
	
	template<typename A>
	vector(std::vector<T, A> v) : std::vector<T, A>(v) {}

	template<typename reflection_serializer_t>
	void custom_save(reflection_serializer_t& serializer) const
	{
		reflection_serializer_t::value_serializer<std::vector<T>>::save(serializer, *this);
	}
};

class string : public std::string
{
public:
	static void custom_serialization() {}

	string() {}
	string(std::string v) : std::string(v) {}

	template<typename reflection_serializer_t>
	void custom_save(reflection_serializer_t& serializer) const
	{
		reflection_serializer_t::value_serializer<std::string>::save(serializer, *this);
	}
};

enum class ETestEnum
{
	Value1,
	Value2,
	ThirdValue,
	Max,
};


enum class EAnotherTestEnum
{
	MyValue1,
	MyValue2,
	MyThirdValue,
	Max,
};

struct sub_class
{
	int int_var = 0;
	char text[16] = { "HELLO WORLD" };

	sub_class(int v = 0) : int_var(v) {}

	REFLECT(sub_class, int_var, text)
};

class sub_class_v1 : public sub_class
{
public:
	short short_var = 0;

	sub_class_v1(short v = 0) : short_var(v) {}
	REFLECT(sub_class_v1, short_var)
};

class sub_class_v2 : public sub_class
{
public:
	int int_var = 0;

	sub_class_v2(short v = 0) : int_var(v) {}
	REFLECT(sub_class_v2, int_var)

};
struct test_reflection_parent : public sub_class
{
	int test_int = 1;
	int test_int_2;
	REFLECT(test_reflection_parent, test_int, test_int_2);
};

struct sub_s
{
	int i = 8;

	static void custom_serialization() {}

	template<typename reflection_serializer_t>
	void custom_save(reflection_serializer_t& serializer) const
	{
	}
};

struct test_reflection_parent2 : test_reflection_parent
{
	int4 intarr[3];
	int4 intarr2[3];
	int4 intarr3[3];
	int4 intarr4[3];
	int4 intarr5[3];

	REFLECT(test_reflection_parent2, intarr, intarr2, intarr3, intarr4, intarr5)
};

struct test_reflection : test_reflection_parent2
{
	sub_s sub;
	int val;
	const int const_int = 3;
	float power;
	ETestEnum Enum = ETestEnum::ThirdValue;

	int GetVal(int multiplier) { return multiplier; }
	int ConstFunc() const { return 4; }

	sub_class* ptr = new sub_class_v1;
	sub_class* ptr_null = nullptr;
	
	sub_class* ptr_arr[4] = { new sub_class(3), new sub_class_v1(1), new sub_class_v2(5), nullptr };
	
	float* float_ptr[2] = { &power, &power };
	float* float_power_ptr = &power;
	float** float_power_ptr_ptr = &float_power_ptr;
	
	char arr[4]{ 30, 31, 32, 33 };
	string str;
	std::string_view str_view = "HELLO WORLD";
	//std::vector<short> stdvec;
	//vector<short> vec;
	//std::array<float, 4> fvec;
    //std::vector<vector<int>> vecvec;

	REFLECT(test_reflection, sub, val, const_int, power, Enum, GetVal, ConstFunc, ptr, ptr_null, ptr_arr, float_ptr, float_power_ptr, float_power_ptr_ptr, arr)
};

int main(int argc, char** argv)
{
	static_reflection<test_reflection> s;

	printf("STATIC REFLECTION:\n");
	for (size_t i = 0; i < s.num_members; i++)
	{
		auto member_type = enum_to_string(s.get_member_type(i));
		printf("%.*s -> %s %.*s %s (size: %i bytes - offset: %i bytes)\n", (int)member_type.size(), member_type.data(), s.is_member_const(i) ? "(const)" : "", (int)s.get_member_type_name(i).size(), s.get_member_type_name(i).data(), s.get_member_name(i), (int)s.get_member_size(i), (int)s.get_member_offset(i));
	}
	
	//vector<int> vec;
	//std::vector<int> vv = vec;
	//
	//printf("RUNTIME REFLECTION:\n");
	test_reflection t;
	t.val = 1;
	t.power = 2.5f;
	//t.str = std::string("TEST STRING");
	//t.stdvec = { 1,4,2,5 };
	//t.vec = t.stdvec;

	char v[4] = { 1,2,3,4 };

	//t.fvec = { 1.1f, 2.2f, 3.3f, 4.4f };
	
    memcpy(t.intarr[0], int4{ 10, 10, 10, 10 }, sizeof(int4));
    memcpy(t.intarr[1], int4{ 9,1,1,1 }, sizeof(int4));
    memcpy(t.intarr[2], int4{ 8, 0, 3, 4 }, sizeof(int4));
   	
	vector<int> v1; v1.push_back(511);  v1.push_back(513);  v1.push_back(515);
    vector<int> v2; v2.push_back(765);  v2.push_back(766);  v2.push_back(767); v2.push_back(768);
	
    //t.vecvec.push_back(v1);
    //t.vecvec.push_back(v2);
	
	std::string result;
	t.reflection_serialize_to(result);
	
	printf("%s", result.c_str());

    return 0;
}
