template<typename first, typename... others>
struct type_array
{
	using first_type = first;
	using next_type = type_array<others...>;

	template<unsigned int N>
	using get = typename std::conditional<N == 0, first_type, typename next_type::template get<N - 1>>::type;

	static size_t get_size(size_t N)
	{
		if (N == 0)
			return sizeof(get<0>);
		return next_type::get_size(N - 1);
	}

	static const char* get_name(size_t N)
	{
		if (N == 0)
			return typeid(get<0>).name();
		return next_type::get_name(N - 1);
	}

	static size_t size()
	{
		return 1 + next_type::size();
	}
};

template<typename first>
struct type_array<first>
{
	using type = first;

	template<unsigned int Num>
	using get = first;

	static size_t get_size(size_t N)
	{
		if (N == 0)
			return sizeof(get<0>);
		return 0;
	}

	static const char* get_name(size_t N)
	{
		if (N == 0)
			return typeid(type).name();
		return "";
	}

	static size_t size()
	{
		return 1;
	}
};


template<typename first, typename... others>
struct variable_array
{
	using types = type_array<first, others...>;
};

//Usage
//class Object
//{
//private:
//	template<typename T>
//	class _reflection
//	{
//	private:
//		using variable_types = type_array<
//			decltype(T::Value),
//			decltype(T::d),
//			decltype(T::vec)
//		>;
//
//	public:
//		template<int N>
//		using get_variable_type = typename variable_types::template get<N>;
//
//		static const size_t get_num_variables() { return variable_types::size(); }
//		static const size_t get_variable_size(size_t N) { return variable_types().get_size(N); }
//		static const char* get_variable_type_name(size_t N) { return variable_types().get_name(N); }
//	};
//
//
//private:
//	int Value;
//	double d;
//	float vec[1];
//};