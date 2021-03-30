#pragma once
#include <type_traits>
namespace em
{
	struct InvalidType {};

	template <typename T1, typename T2>
	constexpr bool IsSame = std::is_same<T1, T2>::value;
	template <typename T1, typename T2>
	constexpr bool IsNotSame = !IsSame<T1, T2>;
	template <bool b, typename T = void>
	using CheckedType = typename std::enable_if<b, T>::type;
	template <bool b, typename T1, typename T2>
	using CondType = typename std::conditional<b, T1, T2>::type;
	template <typename T>
	struct NotValid : std::is_same<T, InvalidType> {};

	template <typename T, typename... Types>
	struct Contains : std::false_type {};

	template <typename T, typename First, typename... Rest>
	struct Contains<T, First, Rest...>
		: CondType<IsSame<typename std::decay<T>::type,
		typename std::decay<First>::type>,
		std::true_type,
		Contains<T, Rest...>>
	{};

	template<typename... Types>
	struct Dulpicated : std::false_type {};
	template<typename T, typename... Rest>
	struct Dulpicated<T, Rest...>
	{
		static constexpr bool value = Dulpicated<Rest...>::value
			? true
			: Contains<T, Rest...>::value;
	};


	template<typename T, typename... Types>
	struct IndexOf : std::integral_constant<int, -1> {};
	template<typename T, typename First, typename... Rest>
	struct IndexOf<T, First, Rest...> : std::integral_constant<int,
		(IsSame<typename std::decay<T>::type, typename std::decay<First>::type>
			? 0 // found a new one
			: (IndexOf<T, Rest...>::value >= 0
				? IndexOf<T, Rest...>::value + 1// already have one
				: -1))
	> {};

	template<size_t Idx, typename... Types> struct TypeOfIndex {};
	template<size_t Idx, typename First, typename... Rest>
	struct TypeOfIndex<Idx, First, Rest...>
	{
		using type = CondType<Idx == 0, First, typename TypeOfIndex<Idx - 1, Rest...>::type>;
	};
	template<size_t Idx, typename Type>
	struct TypeOfIndex<Idx, Type>
	{
		using type = CondType<Idx == 0, Type, InvalidType>;
	};


	template<typename... Types>
	struct TypeList {};

	template<size_t Idx, typename List>
	struct TypeOfIndexInTypeList;
	template<size_t Idx, typename... Types>
	struct TypeOfIndexInTypeList<Idx, TypeList<Types...>>
	{
		using type = typename TypeOfIndex<Idx, Types...>::type;
	};

	template<typename Type, typename List>
	struct PushFrontToTypeList;
	template<typename Type, typename... Types>
	struct PushFrontToTypeList<Type, TypeList<Types...>>
	{
		using type = TypeList<Type, Types...>;
	};
	template<typename Type, typename List>
	struct PushBackToTypeList;
	template<typename Type, typename... Types>
	struct PushBackToTypeList<Type, TypeList<Types...>>
	{
		using type = TypeList<Types..., Type>;
	};

	template<typename Type, typename List>
	struct TypeListContains;
	template<typename Type, typename... Types>
	struct TypeListContains<Type, TypeList<Types...>> :Contains<Type, Types...> {};

	template<typename List>
	struct TypeListIterator 
	{
		static constexpr bool has_next = false;
		using next = InvalidType;
		using list = InvalidType;
		using type = InvalidType;
	};
	template<typename... Types>
	struct TypeListIterator<TypeList<Types...>>
	{
		static constexpr bool has_next = false;
		using next = InvalidType;
		using list = InvalidType;
		using type = InvalidType;
	};
	template<typename First, typename... Rest>
	struct TypeListIterator<TypeList<First, Rest...>>
	{
		static constexpr bool has_next = sizeof...(Rest) != 0;
		using next = TypeList<Rest...>;
		using list = TypeList<First, Rest...>;
		using type = First;
	};

	template<typename Type, typename List>
	struct IndexOfTypeInTypeList : std::integral_constant<int, -1> {};
	template<typename Type, typename... Types>
	struct IndexOfTypeInTypeList<Type, TypeList<Types...>> :IndexOf<Type, Types...> {};

	template <typename T, typename... Types>
	struct MaxAlignOf : std::integral_constant<int, 
		(alignof(T) > MaxAlignOf<Types...>::value
			? alignof(T) 
			: MaxAlignOf<Types...>::value)
	> {};
	template <typename T>
	struct MaxAlignOf<T> : std::integral_constant<int, alignof(T)> {};


	template <typename T, typename... Types>
	struct MaxSizeOf : std::integral_constant<int,
		(sizeof(T) > MaxSizeOf<Types...>::value
			? sizeof(T)
			: MaxSizeOf<Types...>::value)
	> {};
	template <typename T>
	struct MaxSizeOf<T> : std::integral_constant<int, sizeof(T)> {};

	template <typename T>
	struct HasValueType
	{
		struct NoValueType {};
		using value_type = NoValueType;
		struct Impl : CondType<std::is_class<T>::value, T, NoValueType>
		{
			using __type = value_type;
		};
		static constexpr bool value = IsNotSame<typename Impl::__type, NoValueType>;
		using type = typename HasValueType::Impl::__type;
	};

	template<typename, typename T>
	struct HasAllocate : std::false_type {};
	template <typename T, typename Ret, typename... Args>
	struct HasAllocate<T, Ret(Args...)>
	{
	private:
		template <typename U>
		static constexpr std::false_type test(...);
		template <typename U>
		static constexpr auto test(U*)
			-> typename std::is_same<decltype(std::declval<U>().allocate(std::declval<Args>()...)), Ret>::type;
		using type = decltype(test<T>(0));
	public:
		static constexpr bool value = type::value;
	};

	template<typename, typename T>
	struct HasDeallocate : std::false_type {};
	template <typename T, typename Ret, typename... Args>
	struct HasDeallocate<T, Ret(Args...)>
	{
	private:
		template <typename U>
		static constexpr std::false_type test(...);
		template <typename U>
		static constexpr auto test(U*)
			-> typename std::is_same<decltype(std::declval<U>().deallocate(std::declval<Args>()...)), Ret>::type;
		using type = decltype(test<T>(0));
	public:
		static constexpr bool value = type::value;
	};

	template<typename, typename T>
	struct HasInvoke : std::false_type {};
	template <typename T, typename Ret, typename... Args>
	struct HasInvoke<T, Ret(Args...)>
	{
	private:
		template <typename U>
		static constexpr std::false_type test(...);
		template <typename U>
		static constexpr auto test(U*)
			-> typename std::is_same<decltype(std::declval<U>()(std::declval<Args>()...)), Ret>::type;
		using type = decltype(test<T>(0));
	public:
		static constexpr bool value = type::value;
	};

	template <typename T>
	struct FuncPtrInfo
	{
		using Return = InvalidType;
		using Takes = InvalidType;
		static constexpr bool value = false;
	};
	template <typename Ret, typename... Args>
	struct FuncPtrInfo<Ret(Args...)>
	{
		using Return = Ret;
		using Takes = TypeList<Args...>;
		static constexpr bool value = true;
	};


	template <typename T>
	struct IsInvocable
	{
	private:
		template <typename U>
		static constexpr std::false_type test(...);
		template <typename U>
		static constexpr std::true_type test(decltype(&U::operator()));
		using type = decltype(test<T>(0));
	public:
		static constexpr bool value = type::value || FuncPtrInfo<T>::value;
	};


	template <typename T>
	struct LambdaInfoTrait : LambdaInfoTrait<decltype(T::operator())>
	{ };
	template <typename Ret, typename Cls, typename... Args>
	struct LambdaInfoTrait<Ret(Cls::*)(Args...)>
	{
		using Return = Ret;
		using Takes = TypeList<Args...>;
	};
	template <typename Ret, typename Cls, typename... Args>
	struct LambdaInfoTrait<Ret(Cls::*)(Args...) const>
	{
		using Return = Ret;
		using Takes = TypeList<Args...>;
	};
	struct InvalidLambdaInfo
	{
		using Return = InvalidType;
		using Takes = InvalidType;
	};

	template <typename T>
	struct LambdaInfo
	{
	private:
		template <typename U>
		static constexpr InvalidLambdaInfo test(...);
		template <typename U>
		static constexpr LambdaInfoTrait<decltype(&U::operator())> 
			test(decltype(&U::operator()));
		using type = decltype(test<T>(0));
	public:
		using Return = CondType<FuncPtrInfo<T>::value, 
			typename FuncPtrInfo<T>::Return,
			typename type::Return>;
		using Takes = CondType<FuncPtrInfo<T>::value,
			typename FuncPtrInfo<T>::Takes,
			typename type::Takes>;
	};

	template <typename T>
	struct IsAllocator
	{
	private:
		using hasValueType = HasValueType<T>;
		using U = typename hasValueType::type;
		using hasAllocate = HasAllocate<T, U*(size_t, const void*)>;
		using hasDeallocate = HasDeallocate<T, void(U*, size_t)>;
	public:
		static constexpr bool value = hasValueType::value && hasAllocate::value && hasDeallocate::value;
	};
}