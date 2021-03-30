#pragma once
#include <functional>
#include <set>

#include <tsl/htrie_set.h>
#include <tsl/htrie_map.h>

#include "utils/TypeUtil.h"
#include "utils/Optional.h"
namespace em
{
	namespace parser
	{
		template <typename T>
		using Vector = std::vector<T>;
		template <typename T>
		using Set = std::set<T>;
		using Trie = tsl::htrie_set<char>;
		template <typename T>
		using TrieMap = tsl::htrie_map<char, T>;
		template <typename T>
		using Ptr = std::shared_ptr<T>;
		template <typename T>
		using WPtr = std::weak_ptr<T>;
		template <typename... Types>
		using Tuple = std::tuple<Types...>;
		template <typename T>
		using Nullable = em::Optional<T>;			// std::optional (C++17)
		template <typename T>
		using Function = std::function<T>;
		using CharT = char32_t;
		using String = std::string;

		struct Void {};
		struct Location { uint32_t line, col; };

		template <typename In, typename Out>
		using TFuncLoc = Function<Out(In, Location)>;

		template <typename In, typename Out>
		struct FunctionType
		{
			using type = Function<CondType<IsSame<In, Void>, Out(), Out(In const&)>>;
		};

		template <typename Out>
		struct FunctionType<void, Out> { using type = Function<Out()>; };


		template <typename In, typename Out>
		using TFunc = typename FunctionType<In, Out>::type;

		template <typename In, typename Out>
		using TLambda = Out(*)(In const&);


		template <typename T>
		using RepVec = Vector<Vector<T>>;
		template <typename F>
		using LambdaTakes = typename LambdaInfo<F>::Takes;
		template <typename F>
		using LambdaReturn = typename LambdaInfo<F>::Return;
		template <typename Char, typename Ret>
		using ValidCharReturn = CheckedType<
			IsSame<Char, char>
			|| IsSame<Char, wchar_t>
			|| IsSame<Char, char32_t>,
			Ret>;

		template <typename T>
		using WarpVoid = CondType<IsSame<T, void>, Void, T>;
		template <typename T>
		using DeVoid = CondType<IsSame<T, Void>, void, T>;


		template <typename T1, typename T2>
		struct ConnectTuple;
		template <typename T, typename... Types>
		struct ConnectTuple<Tuple<Types...>, T>
		{
			static constexpr bool push = IsNotSame<T, Void>;
			using type = CondType<push, Tuple<Types..., T>, Tuple<Types...>>;
			template <typename U>
			CheckedType<IsSame<U, Void>, type>
				operator()(Tuple<Types...> const& t1, U const& t2) const
			{
				return t1;
			}
			template <typename U>
			CheckedType<IsNotSame<U, Void>, type>
				operator()(Tuple<Types...> const& t1, U const& t2) const
			{
				return std::tuple_cat(t1, Tuple<U>(t2));
			}
		};

		template <typename T1, typename T2>
		struct MakeTuple
		{
			static constexpr bool valid1 = IsNotSame<T1, Void>;
			static constexpr bool valid2 = IsNotSame<T2, Void>;
			using type = CondType<valid1,
				CondType<valid2, Tuple<T1, T2>, T1>,
				CondType<valid2, T2, Void>>;
			template <typename U1, typename U2>
			CheckedType<
				IsSame<U1, Void>&& IsSame<U2, Void>,
				type>
				operator()(U1 const& t1, U2 const& t2) const
			{
				return type();
			}
			template <typename U1, typename U2>
			CheckedType<
				IsNotSame<U1, Void>&& IsSame<U2, Void>,
				type>
				operator()(U1 const& t1, U2 const& t2) const
			{
				return type(t1);
			}
			template <typename U1, typename U2>
			CheckedType<
				IsSame<U1, Void>&& IsNotSame<U2, Void>,
				type>
				operator()(U1 const& t1, U2 const& t2) const
			{
				return type(t2);
			}
			template <typename U1, typename U2>
			CheckedType<
				IsNotSame<U1, Void>&& IsNotSame<U2, Void>,
				type>
				operator()(U1 const& t1, U2 const& t2) const
			{
				return type(t1, t2);
			}
		};

		template <typename T, typename... Types>
		struct MakeTuple<Tuple<Types...>, T>
		{
			using type = typename ConnectTuple<Tuple<Types...>, T>::type;
			type operator()(Tuple<Types...> const& t1, T const& t2) const
			{
				return ConnectTuple<Tuple<Types...>, T>()(t1, t2);
			}
		};
	}
}