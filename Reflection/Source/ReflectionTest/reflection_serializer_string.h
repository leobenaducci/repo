#include "reflection_serializer.h"

namespace reflection_serializers
{
	struct string_reflection_serializer_t
	{
		template<typename T, bool>
		struct single_value_serializer_impl
		{
			template<typename reflection_serializer_t>
			static void save(reflection_serializer_t& serializer, const T& var)
			{
				((string_serializer*)&serializer)->append(std::to_string(var));
			}
		};

		template<typename T>
		struct single_value_serializer_impl<T, true>
		{
			template<typename reflection_serializer_t>
			static void save(reflection_serializer_t& serializer, const T& var)
			{
				((string_serializer*)&serializer)->append(enum_to_string(var));
			}
		};

		//serialize other types
		template<typename T>
		struct single_value_serializer
		{
			template<typename reflection_serializer_t>
			static void save(reflection_serializer_t& serializer, const T& var)
			{
				single_value_serializer_impl<T, std::is_enum_v<T>>::save(serializer, var);
				((string_serializer*)&serializer)->append(", ");
			}
		};

		//serialize std::string_view
		template<>
		struct single_value_serializer<std::string_view>
		{
			template<typename reflection_serializer_t>
			static void save(reflection_serializer_t& serializer, const std::string_view& var)
			{
				((string_serializer*)&serializer)->append("\"");
				((string_serializer*)&serializer)->append(var);
				((string_serializer*)&serializer)->append("\", ");
			}
		};

		template<class T>
		struct reflected_pointer_serializer
		{
			template<typename reflection_serializer_t>
			static void save(reflection_serializer_t& serializer, const T& var)
			{
				std::string tmp;
				var->reflection_serialize_to(tmp);

				size_t index = 0;
				size_t scope = ((string_serializer*)&serializer)->scope;
				std::string scoped = "\n" + std::string(scope, '\t');
				while (true)
				{
					index = tmp.find("\n", index);
					if (index == std::string::npos) 
						break;

					tmp.replace(index, 1, scoped);
					index += scope + 1;
				}

				((string_serializer*)&serializer)->append(tmp);
			}
		};
	};

	struct string_serializer : public reflection_detail::base_reflection_serializer<string_reflection_serializer_t>
	{
		std::string result;
		size_t scope = 0;
		
		void append(std::string_view var)
		{
			if (result.size() > 0 && result.back() == '\n')
				result += std::string(scope, '\t');
			result += var;
		}

		virtual void begin_single(std::string_view type, std::string_view name)
		{
			result += std::string(scope, '\t') + std::string(type) + " " + std::string(name) + " = ";
			scope++;
		}

		virtual void end_single()
		{
			scope--;
			result += '\n';
		}

		virtual void begin_array(std::string_view type, std::string_view name, size_t size)
		{
			if (type.empty())
			{
				result += "[";
				return;
			}

			result += std::string(scope, '\t') + std::string(type) + " " + std::string(name) + " = [\n";
			scope++;
		}

		virtual void end_array(std::string_view type)
		{
			if (type.empty())
			{
				result += "],";
				return;
			}

			scope--;
			result += "\n" + std::string(scope, '\t') + "],\n";
		}

		virtual void begin_class(std::string_view class_name)
		{
			result += std::string(class_name) + " {\n ";
			scope++;
		}

		virtual void end_class()
		{
			scope--;
			if (result.back() == '\n')
				result += std::string(scope, '\t');
			result += "},\n";
		}

		virtual void null_pointer()
		{
			result += "nullptr, ";
		}
	};
}