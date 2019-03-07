namespace reflection
{
	//holds any type
	struct anytype
	{
		const void* ptr;

		template<typename T>
		anytype(const T& p) : ptr(&p) { }

		template<typename T>
		operator const T&() const { return *(const T*)ptr; }
	};

	enum class EMemberType
	{
		MemberFunction,
		MemberVariable,
		Function,
		Variable,
		Max,
	};
}

namespace reflection_detail
{
	using EMemberType = reflection::EMemberType;
	using anytype = reflection::anytype;

	template<typename VT>
	constexpr EMemberType GetMemberType()
	{
		if (std::is_member_function_pointer_v<VT>) return EMemberType::MemberFunction;
		if (std::is_member_object_pointer_v<VT>) return EMemberType::MemberVariable;
		if (std::is_function_v<std::remove_pointer_t<VT>>) return EMemberType::Function;
		return EMemberType::Variable;
	}

	template<typename C, typename T, typename VT, VT V, EMemberType> struct reflection_offsetof { static constexpr size_t offset() { auto ptr = &V; return *(size_t*)&ptr; } };
	template<typename C, typename T, typename VT, VT V> struct reflection_offsetof<C, T, VT, V, EMemberType::MemberVariable> { static constexpr size_t offset() { return (size_t)&(((C*)0)->*V); } };
	template<typename C, typename T, typename VT, VT V> struct reflection_offsetof<C, T, VT, V, EMemberType::Variable> { static constexpr size_t offset() { return (size_t)V; } };
	template<typename C, typename T, typename VT, VT V> using reflection_offsetof_t = reflection_offsetof<C, T, VT, V, GetMemberType<VT>()>;

	template<typename C, typename T, typename VT, VT V, EMemberType> struct reflection_sizeof { static constexpr size_t size() { return 0; } };
	template<typename C, typename T, typename VT, VT V> struct reflection_sizeof<C, T, VT, V, EMemberType::MemberVariable> { static constexpr size_t size() { return sizeof(T); } };
	template<typename C, typename T, typename VT, VT V> struct reflection_sizeof<C, T, VT, V, EMemberType::Variable> { static constexpr size_t size() { return sizeof(T); } };
	template<typename C, typename T, typename VT, VT V> using reflection_sizeof_t = reflection_sizeof<C, T, VT, V, GetMemberType<VT>()>;

	template<typename C, EMemberType> struct reflection_decltype
	{
		template<typename T> static auto type(T var)
		{
			struct Tmp { T type_v; };
			return Tmp();
		}
	};

	template<typename C> struct reflection_decltype<C, EMemberType::MemberVariable>
	{
		template<typename T> static auto type(T C::* var)
		{
			struct Tmp { T type_v; };
			return Tmp();
		}

		template<typename T> static auto type(const T C::* var)
		{
			struct Tmp { const T type_v = std::declval<T>(); };
			return Tmp();
		}
	};

	template<typename C> struct reflection_decltype<C, EMemberType::Variable>
	{
		template<typename T> static auto type(T* var)
		{
			struct Tmp { T type_v; };
			return Tmp();
		}
	};
	
    template<typename first, typename... others>
    struct reflection_type_array
    {
        using first_type = first;
        using next_type = reflection_type_array<others...>;

        static constexpr size_t num_members = 1 + next_type::num_members;

		template<size_t N, typename first, typename... others>
		struct get_member_type_t
		{
			using type_t = typename get_member_type_t<N-1, others...>::type_t;
		};

		template<size_t N, typename first>
		struct get_member_type_t<N, first>
		{
			using type_t = typename first::type_t;
		};

		template<typename first, typename... others>
		struct get_member_type_t<0, first, others...>
		{
			using type_t = typename first::type_t;
		};

		template<typename first>
		struct get_member_type_t<0, first>
		{
			using type_t = typename first::type_t;
		};

		template<size_t N>
		using member_type_t = typename get_member_type_t<N, first, others...>::type_t;

		static constexpr auto get_member_name(size_t N)
        {
            if (N == 0) return first_type::name_v;
            return next_type::get_member_name(N - 1);
        }

        static constexpr size_t get_member_size(size_t N)
        {
            if (N == 0) return first_type::size();
            return next_type::get_member_size(N - 1);
        }

        static constexpr std::string_view get_member_type_name(size_t N)
        {
            if (N == 0) return ::get_type_name<first_type::type_t>();
            return next_type::get_member_type_name(N - 1);
        }

        static constexpr size_t get_member_offset(size_t N)
        {
            if (N == 0) return first_type::offset();
            return next_type::get_member_offset(N - 1);
        }

		static constexpr EMemberType get_member_type(size_t N)
		{
			if (N == 0) return first_type::member_type();
			return next_type::get_member_type(N - 1);
		}

		static constexpr bool is_member_const(size_t N)
		{
			if (N == 0) return first_type::is_const();
			return next_type::is_member_const(N - 1);
		}

        template<typename T>
        static auto get_member_value(const T& obj, size_t N)
        {
            if (N == 0) return first_type::get_value(obj);
            return next_type::get_member_value(obj, N - 1);
        }
    };

    template<typename first>
    struct reflection_type_array<first>
    {
        using first_type = first;

		template<size_t N>
		using member_type_t = typename first::type_t;

        static constexpr size_t num_members = 1;

        static constexpr auto get_member_name(size_t N) { return first_type::name_v; }
        static constexpr size_t get_member_size(size_t N) { return first_type::size(); }
        static constexpr auto get_member_type_name(size_t N) { return ::get_type_name<first_type::type_t>(); }
		static constexpr size_t get_member_offset(size_t N) { return first_type::offset(); }
		static constexpr EMemberType get_member_type(size_t N) { return first_type::member_type(); }
		static constexpr bool is_member_const(size_t N) { return first_type::is_const(); }

        template<typename T>
        static auto get_member_value(const T& obj, size_t N)
        {
            return first_type::get_value(obj);
        }

		template<typename T, typename T2>
		static auto set_member_value(const T& obj, size_t N, const T2& V)
		{
			return first_type::set_value(obj, &V);
		}

		template<typename OBJ_TYPE, typename ...TArgs>
		static anytype call_function(OBJ_TYPE* obj, size_t N, TArgs... Args)
		{
			return first_type::call_function<OBJ_TYPE, TArgs...>(obj, Args...);
		}
	};

    template<class C, typename T, typename O, typename S, typename N, EMemberType MT>
    struct reflection_type
    {
        using type_t = T;
        using name_t = N;

        static constexpr auto name_v = name_t::data_v;

		static constexpr size_t size() { return S::size(); }
		static constexpr size_t offset() { return O::offset(); }
		static constexpr EMemberType member_type() { return MT; }
		static constexpr bool is_const() { return std::is_const_v<T>; }

        template<typename OBJ_TYPE>
        static auto get_value(const OBJ_TYPE& obj)
        {
            std::byte* cobj = (std::byte*)&obj;
            std::byte* ptr = cobj + offset();
		
			return anytype(*(type_t*)ptr);
        }
	};

    template<typename T> struct reflection : decltype(T::meta_reflection()) {};
}
