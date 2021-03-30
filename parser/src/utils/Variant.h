#pragma once
#include <type_traits>
#include <algorithm>
#include <exception>
#include <memory>

#include "utils/TypeUtil.h"

namespace em
{
	class bad_variant_access : public std::exception
	{
	public:
		virtual char const* what() const noexcept
		{
			return "bad variant access.";
		}
		virtual ~bad_variant_access() {};
	};

	namespace v
	{
		template <typename T, typename... Args>
		struct IsCopyMoveConstructor
		{
			static constexpr bool value = false;
		};
		template <typename T, typename First, typename... Rest>
		struct IsCopyMoveConstructor<T, First, Rest...>
		{
			static constexpr bool value = sizeof...(Rest) == 0 && std::is_same<typename std::decay<T>::type
				, typename std::decay<First>::type>::value;
		};

		// Iter should be a TypeListIterator<...> template
		template <typename Iter, typename... Args>
		struct FirstConstructible
		{
			static constexpr bool value =
				!NotValid<typename Iter::type>::value
				&& std::is_constructible<typename Iter::type, Args...>::value
				? true
				: (Iter::has_next
					? FirstConstructible<TypeListIterator<typename Iter::next>, Args...>::value
					: false);
			using type = typename std::conditional<
				!NotValid<typename Iter::type>::value
				&& std::is_constructible<typename Iter::type, Args...>::value,
				typename std::decay<typename Iter::type>::type,
				typename std::conditional<Iter::has_next, 
					typename FirstConstructible<TypeListIterator<typename Iter::next>, Args...>::type,
					InvalidType
				>::type
			>::type;
		};
		template <typename... Args>
		struct FirstConstructible<TypeListIterator<InvalidType>, Args...>
		{
			static constexpr bool value = false;
			using type = InvalidType;
		};

		template <typename... Args>
		using Ptr = void(*)(Args...);
		template <typename T>
		void Destroy(void* obj)
		{
			reinterpret_cast<T*>(obj)->~T();
		}
		template <typename T>
		void Construct(void* obj, void const* value)
		{
			new(obj)T(*static_cast<T const*>(value));
		}
	}

	template <typename First, typename... Rest>
	struct alignas(MaxAlignOf<First, Rest...>::value) Variant
	{
		static_assert(!Dulpicated<First, Rest...>::value,
			"Types of a variant should not be dulpicated.");
		// Max align of all types
		static constexpr int _max_align = alignof(First) > Variant<Rest...>::_max_align ? alignof(First) : Variant<Rest...>::_max_align;
		// Max size of all types
		static constexpr int _max_size = sizeof(First) > Variant<Rest...>::_max_size ? sizeof(First) : Variant<Rest...>::_max_size;
		// Max size of all types
		static constexpr size_t _types_count = Variant<Rest...>::_types_count + 1;
		

		// The allocator type
		using Contents = typename PushFrontToTypeList<First, typename Variant<Rest...>::Contents>::type;
	private:
		struct alignas(_max_align) Index { uint8_t idx{ (uint8_t)-1 }; };
		Index index{};
		char data[_max_size]{};
		using type = Variant<First, Rest...>;
		static inline void _Destroy(uint8_t idx, void* ptr)
		{
			static v::Ptr<void*> const destructors[_types_count]
			{
				std::addressof(v::Destroy<First>),
				std::addressof(v::Destroy<Rest>)...
			};
			if (idx != (uint8_t)-1) { destructors[idx](ptr); idx = (uint8_t)-1; }
		}
		static inline void _Copy(uint8_t idx, void* ptr, void const* v)
		{
			static v::Ptr<void*, void const*> const copy_constructors[_types_count]
			{
				std::addressof(v::Construct<First>),
				std::addressof(v::Construct<Rest>)...
			};
			copy_constructors[idx](ptr, v);
		}
		inline void _CopyToThis(Variant const& v)
		{
			_Destroy(index.idx, data);
			index = v.index;
			if (index.idx != (uint8_t)-1) _Copy(index.idx, data, v.data);
		}
		inline void _MoveToThis(Variant&& v)
		{
			_Destroy(index.idx, data);
			index = std::move(v.index);
			std::memcpy(data, v.data, _max_size);
		}
	public:
		template <typename... Args>
		Variant(Args... args)
		{
			static_assert(v::FirstConstructible<TypeListIterator<Contents>, Args...>::value,
				"No matching constructor for initialization.");
			using T = typename v::FirstConstructible<TypeListIterator<Contents>, Args...>::type;
			new(data)T(args...);
			index.idx = IndexOfTypeInTypeList<T, Contents>::value;
		}
		template <typename U, typename = typename std::enable_if<
			TypeListContains<U, Contents>::value
			&& std::is_constructible<U, U const&>::value
		>::type>
		Variant(U const& v)
		{
			index.idx = IndexOfTypeInTypeList<U, Contents>::value;
			new(data)U(v);
		}
		Variant(Variant const& v)
		{
			_CopyToThis(v);
		}
		Variant(Variant&& v) noexcept
		{
			_MoveToThis(std::move(v));
		}
		Variant() {}
		~Variant()
		{
			_Destroy(index.idx, data);
		}
		template <typename U>
		typename std::enable_if<
			TypeListContains<U, Contents>::value
			&& std::is_constructible<U, U const&>::value,
			Variant&
		>::type operator=(U const& v)
		{
			_Destroy(index.idx, data);
			index.idx = IndexOfTypeInTypeList<U, Contents>::value;
			new(data)U(v);
		}
		Variant& operator=(Variant const& v)
		{
			_CopyToThis(v);
			return *this;
		}
		Variant& operator=(Variant&& v)
		{
			_MoveToThis(std::forward(v));
			return *this;
		}

		template <size_t Idx>
		typename std::enable_if < Idx < _types_count, typename TypeOfIndexInTypeList<Idx, Contents>::type>::type&
			Get()
		{
			if (index.idx != Idx) throw bad_variant_access();
			return *reinterpret_cast<typename TypeOfIndexInTypeList<Idx, Contents>::type*>(data);
		}
		template <typename Type>
		typename std::enable_if<TypeListContains<Type, Contents>::value, Type>::type&
			Get()
		{
			if (index.idx != IndexOfTypeInTypeList<Type, Contents>::value) throw bad_variant_access();
			return *reinterpret_cast<Type*>(data);
		}
		template <size_t Idx>
		typename std::enable_if < Idx < _types_count, typename TypeOfIndexInTypeList<Idx, Contents>::type>::type const&
			Get() const
		{
			if (index.idx != Idx) throw bad_variant_access();
			return *reinterpret_cast<typename TypeOfIndexInTypeList<Idx, Contents>::type*>(data);
		}
		template <typename Type>
		typename std::enable_if<TypeListContains<Type, Contents>::value, Type>::type const&
			Get() const
		{
			if (index.idx != IndexOfTypeInTypeList<Type, Contents>::value) throw bad_variant_access();
			return *reinterpret_cast<Type*>(data);
		}
		template <typename Type>
		typename std::enable_if<TypeListContains<Type, Contents>::value, bool>::type
			Is() const
		{
			return index.idx == IndexOfTypeInTypeList<Type, Contents>::value;
		}
	};

	template <typename T>
	struct alignas(alignof(T)) Variant<T>
	{
		// Max align of all types
		static constexpr int _max_align = alignof(T);
		// Max size of all types
		static constexpr int _max_size = sizeof(T);
		// Max size of all types
		static constexpr size_t _types_count = 1;

		using Contents = TypeList<T>;
	private:
		using type = Variant<T>;
		struct alignas(_max_align) Index { uint8_t idx{ (uint8_t)-1 }; };
		Index index{};
		char data[_max_size]{};
		static inline void _Destroy(uint8_t& idx, void* ptr)
		{
			static v::Ptr<void*> const destructor = std::addressof(v::Destroy<T>);
			if (idx != (uint8_t)-1) { destructor(ptr); idx = (uint8_t)-1; }
		}
		static inline void _Copy(uint8_t idx, void* ptr, void const* v)
		{
			static v::Ptr<void*, void const*> const copy_constructors
				= std::addressof(v::Construct<T>);
			copy_constructors(ptr, v);
		}
		inline void _CopyToThis(Variant const& v)
		{
			_Destroy(index.idx, data);
			index = v.index;
			if (index.idx != (uint8_t)-1) _Copy(index.idx, data, v.data);
		}
		inline void _MoveToThis(Variant&& v)
		{
			_Destroy(index.idx, data);
			index = std::move(v.index);
			std::memcpy(data, v.data, _max_size);
		}
	public:
		template <typename... Args>
		Variant(Args... args)
		{
			static_assert(v::FirstConstructible<TypeListIterator<Contents>, Args...>::value,
				"No matching constructor for initialization.");
			using U = typename v::FirstConstructible<TypeListIterator<Contents>, Args...>::type;
			new(data)U(args...);
			index.idx = IndexOfTypeInTypeList<U, Contents>::value;
		}
		template <typename U, typename = typename std::enable_if<
			TypeListContains<U, Contents>::value
			&& std::is_constructible<U, U const&>::value
		>::type>
			Variant(U const& v)
		{
			index.idx = IndexOfTypeInTypeList<U, Contents>::value;
			new(data)U(v);
		}
		Variant(Variant const& v)
		{
			_CopyToThis(v);
		}
		Variant(Variant& v)
		{
			_MoveToThis(std::forward(v));
		}
		Variant() {}
		~Variant()
		{
			_Destroy(index.idx, data);
		}

		template <typename U>
		typename std::enable_if<
			TypeListContains<U, Contents>::value
			&& std::is_constructible<U, U const&>::value,
			Variant&
		>::type operator=(U const& v)
		{
			_Destroy(index.idx, data);
			index.idx = IndexOfTypeInTypeList<U, Contents>::value;
			new(data)U(v);
		}
		Variant& operator=(Variant const& v)
		{
			_CopyToThis(v);
			return *this;
		}
		Variant& operator=(Variant&& v)
		{
			_MoveToThis(std::forward(v));
			return *this;
		}
		template <size_t Idx>
		typename std::enable_if < Idx < _types_count, typename TypeOfIndexInTypeList<Idx, Contents>::type>::type&
			Get()
		{
			if (index != Idx) throw bad_variant_access();
			return *reinterpret_cast<typename TypeOfIndexInTypeList<Idx, Contents>::type*>(data);
		}
		template <typename Type>
		typename std::enable_if<TypeListContains<Type, Contents>::value, Type>::type&
			Get()
		{
			if (index.idx != IndexOfTypeInTypeList<Type, Contents>::value) throw bad_variant_access();
			return *reinterpret_cast<Type*>(data);
		}
		template <size_t Idx>
		typename std::enable_if < Idx < _types_count, typename TypeOfIndexInTypeList<Idx, Contents>::type>::type const&
			Get() const
		{
			if (index.idx != Idx) throw bad_variant_access();
			return *reinterpret_cast<typename TypeOfIndexInTypeList<Idx, Contents>::type*>(data);
		}
		template <typename Type>
		typename std::enable_if<TypeListContains<Type, Contents>::value, Type>::type const&
			Get() const
		{
			if (index.idx != IndexOfTypeInTypeList<Type, Contents>::value) throw bad_variant_access();
			return *reinterpret_cast<Type*>(data);
		}
		template <typename Type>
		typename std::enable_if<TypeListContains<Type, Contents>::value, bool>::type
			Is() const
		{
			return index.idx == IndexOfTypeInTypeList<Type, Contents>::value;
		}
	};

	template <size_t Idx, typename... Types>
	typename std::enable_if < Idx < sizeof...(Types), typename TypeOfIndex<Idx, Types...>::type>::type&
		get(Variant<Types...>& v) { return v.template Get<Idx>(); }

	template <typename Type, typename... Types>
	typename std::enable_if < Contains<Type, Types...>::value, Type>::type&
		get(Variant<Types...>& v) { return v.template Get<Type>(); }

	template <typename Type, typename... Types>
	typename std::enable_if < Contains<Type, Types...>::value, bool>::type
		is(Variant<Types...>& v) { return v.template Is<Type>(); }
}