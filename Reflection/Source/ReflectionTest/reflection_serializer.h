namespace reflection_detail
{
	//Helpers
	template<class T>
	static auto array_to_const_pointer(T) -> const T* {}

	template<class T, size_t N>
	static auto array_to_const_pointer(T(&)[N]) -> const T* {}

	template<class T, size_t N, size_t M>
	static auto array_to_const_pointer(T(&)[N][M]) -> const T* {}

	template<class T>
	static constexpr size_t array_size(T) { return 1; }

	template<class T, size_t N>
	static constexpr size_t array_size(const T(&)[N]) { return N; }

	template<class T, size_t N, size_t M>
	static constexpr size_t array_size_2(const T(&)[N][M]) { return M; }

	template<typename R> static constexpr auto has_reflection(int) -> decltype(R::meta_reflection(), bool()) { return true; }
	template<typename R> static constexpr bool has_reflection(...) { return false; }

	template<typename R> static constexpr auto has_custom_serialization(int) -> decltype(R::custom_serialization(), bool()) { return true; }
	template<typename R> static constexpr bool has_custom_serialization(...) { return false; }

	template<typename R> static constexpr auto has_as_array(int) -> decltype(R::reflection_as_array(), bool()) { return true; }
	template<typename R> static constexpr bool has_as_array(...) { return false; }

	template<typename T> struct is_std_array_or_vector { static constexpr bool value = has_as_array<T>(0); };
	template<typename VT> struct is_std_array_or_vector<std::vector<VT>> { static constexpr bool value = true; };
	template<typename VT, size_t size> struct is_std_array_or_vector<std::array<VT, size>> { static constexpr bool value = true; };

	template<typename T>
	struct std_array_size_t
	{
		static size_t size(const T&) { return 1; }
	};

	template<typename VT>
	struct std_array_size_t<std::vector<VT>>
	{
		static size_t size(const std::vector<VT>& vec) { return vec.size(); }
	};
	template<typename VT, size_t size>
	struct std_array_size_t<std::array<VT, size>>
	{
		static size_t size(const std::array<VT, size>& arr) { return arr.size(); }
	};

	template<typename T>
	static constexpr size_t std_array_size(const T& var) { return std_array_size_t<T>::size(var); }

	template<typename custom_reflection_serializer_t>
	struct base_reflection_serializer
	{
		//serialize reflected type
		template<class T>
		struct reflected_serializer
		{
			template<typename reflection_serializer_t, size_t N>
			struct save_value_t
			{
				static void save_value(reflection_serializer_t& serializer, const T& var)
				{
					using meta = typename runtime_reflection<T>::meta;
					using member_type = typename meta::member_type_t<N>;

					auto value = meta::get_member_value(var, N);
					main_serializer<member_type>::save(serializer, (const member_type)value);
				}

				template<size_t I> static void save_array(reflection_serializer_t& serializer, const T& var) { static_assert(false, "NOT IMPLEMENTED"); }
				template<> static void save_array<0>(reflection_serializer_t& serializer, const T& var) { }

				template<>
				static void save_array<1>(reflection_serializer_t& serializer, const T& var)
				{
					using meta = typename static_reflection<T>::meta;
					using member_type = typename meta::member_type_t<N>;
					member_type empty{};

					auto value = meta::get_member_value(var, N);
					auto value_ptr = (decltype(array_to_const_pointer(empty)))value.ptr;

					array_serializer::save(serializer, value_ptr, value_ptr + array_size(empty));
				}

				template<>
				static void save_array<2>(reflection_serializer_t& serializer, const T& var)
				{
					using meta = typename static_reflection<T>::meta;
					using member_type = typename meta::member_type_t<N>;
					member_type empty{};

					auto value = meta::get_member_value(var, N);
					auto value_ptr = (decltype(array_to_const_pointer(empty)))value.ptr;

					for (size_t i = 0; i < array_size(empty); i++)
					{
						serializer.begin_array("", "", array_size(empty));
						array_serializer::save(serializer, value_ptr + i * array_size(empty), value_ptr + i * array_size(empty) + array_size_2(empty));
						serializer.end_array("");
					}
				}
			};

			template<typename reflection_serializer_t, size_t N>
			static void save_member(reflection_serializer_t& serializer, const T& var)
			{
				using meta = typename runtime_reflection<T>::meta;
				using member_type = typename meta::member_type_t<N>;

				member_type empty{};

				if (std::is_array<member_type>::value)
				{
					serializer.begin_array(meta::get_member_type_name(N), meta::get_member_name(N), array_size(empty));
					save_value_t<reflection_serializer_t, N>::save_array< std::rank_v<member_type> >(serializer, var);
					serializer.end_array(meta::get_member_type_name(N));
				}
				else if (is_std_array_or_vector<member_type>::value)
				{
					serializer.begin_array(meta::get_member_type_name(N), meta::get_member_name(N), std_array_size(empty));
					save_value_t<reflection_serializer_t, N>::save_value(serializer, var);
					serializer.end_array(meta::get_member_type_name(N));
				}
				else
				{
					serializer.begin_single(meta::get_member_type_name(N), meta::get_member_name(N));
					save_value_t<reflection_serializer_t, N>::save_value(serializer, var);
					serializer.end_single();
				}
			}

			//Helper to save individual members
			template<typename reflection_serializer_t, size_t N>
			struct save_members_t
			{
				static void save(reflection_serializer_t& serializer, const T& var)
				{
					save_members_t<reflection_serializer_t, N - 1>::save(serializer, var);
					if (N < static_reflection<T>::num_members)
						save_member<reflection_serializer_t, N>(serializer, var);
				}
			};

			template<typename reflection_serializer_t>
			struct save_members_t<reflection_serializer_t, 0>
			{
				static void save(reflection_serializer_t& serializer, const T& var)
				{
					save_member<reflection_serializer_t, 0>(serializer, var);
				}
			};

			//helper to save parent classes
			template<typename T>
			struct save_super_t
			{
				template<typename T>
				struct empty_save_self_t
				{
					template<typename reflection_serializer_t>
					static void save(reflection_serializer_t& serializer, const T& var)
					{
					}
				};

				template<typename T>
				struct save_self_t
				{
					template<typename reflection_serializer_t>
					static void save(reflection_serializer_t& serializer, const T& var)
					{
						using meta_t = runtime_reflection<T>;
						reflected_serializer<T>::template save_members_t<reflection_serializer_t, meta_t::num_members>::save(serializer, var);
					}
				};

				template<typename reflection_serializer_t>
				static void save(reflection_serializer_t& serializer, const T& var)
				{
					using super_t = typename T::reflection_super;

					using save_super = std::conditional_t<has_reflection<super_t>(0), save_super_t<super_t>, empty_serializer<T>>;
					using save_self = std::conditional_t<has_reflection<super_t>(0), save_self_t<super_t>, empty_save_self_t<T>>;

					//save parents
					save_super::save<reflection_serializer_t>(serializer, var);
					//save self
					save_self::save(serializer, var);
				}
			};

			template<typename reflection_serializer_t>
			static void save(reflection_serializer_t& serializer, const T& var)
			{
				using meta = runtime_reflection<T>;
				using super_meta = runtime_reflection<T::reflection_super>;

				serializer.begin_class(var.reflection_get_type_name());

				//save parents
				save_super_t<T>::save<reflection_serializer_t>(serializer, var);

				//save self
				save_members_t<reflection_serializer_t, meta::num_members>::save(serializer, var);

				serializer.end_class();
			}
		};

		//serialize array
		struct array_serializer
		{
			template<typename T, size_t I, typename reflection_serializer_t>
			static void save(reflection_serializer_t& serializer, const T(&var)[I])
			{
				array_serializer::save<T>(serializer, &var[0], &var[I]);
			}

			template<typename T, typename reflection_serializer_t>
			static void save(reflection_serializer_t& serializer, const T* begin, const T* end)
			{
				size_t size = (end - begin);
				for (size_t i = 0; i < size; i++)
					main_serializer<T>::save(serializer, *(begin + i));
			}
		};

		template<typename T>
		struct custom_type_serializer_t
		{
			template<typename reflection_serializer_t>
			static void save(reflection_serializer_t& serializer, const T& var)
			{
				var.custom_save(serializer);
			}
		};

		//select between reflected or basic type
		template<typename T>
		struct single_serializer
		{
			template<typename reflection_serializer_t>
			static void save(reflection_serializer_t& serializer, const T& var)
			{
				using serializer_t = std::conditional_t<has_reflection<T>(0), reflection_serializer_t::reflected_serializer<T>,
					std::conditional_t<has_custom_serialization<T>(0), custom_type_serializer_t<T>,
					custom_reflection_serializer_t::single_value_serializer<T>>>;
				serializer_t::save(serializer, var);
			}
		};

		//serialize std::vector
		template<typename VT>
		struct single_serializer<std::vector<VT>>
		{
			template<typename reflection_serializer_t>
			static void save(reflection_serializer_t& serializer, const std::vector<VT>& var)
			{
				array_serializer::save<VT>(serializer, var.data(), var.data() + var.size());
			}
		};

		//serialize std::array
		template<typename VT, size_t Size>
		struct single_serializer<std::array<VT, Size>>
		{
			template<typename reflection_serializer_t>
			static void save(reflection_serializer_t& serializer, const std::array<VT, Size>& var)
			{
				array_serializer::save<VT>(serializer, var.data(), var.data() + var.size());
			}
		};

		//serialize std::array
		template<>
		struct single_serializer<std::string>
		{
			template<typename reflection_serializer_t>
			static void save(reflection_serializer_t& serializer, const std::string& var)
			{
				custom_reflection_serializer_t::single_value_serializer<std::string_view>::save(serializer, var);
			}
		};

		//select between array or single
		template<typename T>
		struct value_serializer
		{
			template<typename reflection_serializer_t>
			static void save(reflection_serializer_t& serializer, const T& var)
			{
				using serializer_t = std::conditional_t<std::is_array<T>::value, reflection_serializer_t::array_serializer, reflection_serializer_t::single_serializer<T>>;
				serializer_t::save(serializer, var);
			}
		};

		template<typename T>
		struct pointer_serializer
		{
			template<typename reflection_serializer_t>
			static void save(reflection_serializer_t& serializer, const T& var)
			{
				if (var == nullptr)
				{
					reflection_serializer_t::null_serializer<T>::save(serializer, var);
				}
				else
				{
					using unpointered_type_t = std::remove_pointer<T>::type;
					using serializer_t = std::conditional_t<has_reflection<unpointered_type_t>(0), custom_reflection_serializer_t::reflected_pointer_serializer<T>, reflection_serializer_t::null_serializer<T>>;
					serializer_t::save(serializer, var);
				}
			}
		};

		template<typename T>
		struct variable_serializer
		{
			template<typename reflection_serializer_t>
			static void save(reflection_serializer_t& serializer, const T& var)
			{
				using serializer_t = std::conditional_t<std::is_pointer<T>::value, reflection_serializer_t::pointer_serializer<T>, reflection_serializer_t::value_serializer<T>>;
				serializer_t::save(serializer, var);
			}
		};

		template<typename T>
		struct empty_serializer
		{
			template<typename reflection_serializer_t>
			static void save(reflection_serializer_t& serializer, const T&) { }
		};

		template<typename T>
		struct null_serializer
		{
			template<typename reflection_serializer_t>
			static void save(reflection_serializer_t& serializer, const T&)
			{
				serializer.null_pointer();
			}
		};

		template<typename T>
		struct main_serializer
		{
			template<typename reflection_serializer_t>
			static void save(reflection_serializer_t& serializer, const T& var)
			{
				using serializer_t = std::conditional_t<std::is_member_function_pointer<T>::value, reflection_serializer_t::empty_serializer<T>, reflection_serializer_t::variable_serializer<T>>;
				serializer_t::save(serializer, var);
			}
		};

		/***

		main -> if(function) empty else variable
		variable -> if(pointer) pointer else value
		pointer -> if(null || !reflected) null else reflected
		value -> if(array) array else single
		array -> main_serializer
		array[2] -> array
		single -> if(has_reflection) reflected else if(custom_type) custom_type else custom
		single<std::vector> -> array
		single<std::array> -> array
		reflected -> main


		****/

		virtual void begin_single(std::string_view type, std::string_view name) = 0;
		virtual void end_single() = 0;

		virtual void begin_array(std::string_view type, std::string_view name, size_t size) = 0;
		virtual void end_array(std::string_view type) = 0;

		virtual void begin_class(std::string_view class_name) = 0;
		virtual void end_class() = 0;

		virtual void null_pointer() = 0;

		template<typename T>
		void save(const T& var)
		{
			main_serializer<T>::save(*this, var);
		}
	};

}