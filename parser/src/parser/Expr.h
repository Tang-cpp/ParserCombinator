#pragma once
#include "parser/Core.h"
#include "parser/Iterator.h"
namespace em
{
	namespace parser
	{
		template <typename Out> struct Rule;

		template <typename Out>
		struct Expr
		{
			virtual Nullable<Out> Parse(U32Itor& input, U32Itor const& end, Ptr<Expr<Void>> const& s) = 0;
			virtual ~Expr() {};
		};


		template <typename Out1, typename Out2>
		struct Sequ : Expr<typename MakeTuple<Out1, Out2>::type>
		{
			using Result = typename MakeTuple<Out1, Out2>::type;
			Ptr<Expr<Out1>> first;
			Ptr<Expr<Out2>> second;
			Sequ(Ptr<Expr<Out1>> const& e1, Ptr<Expr<Out2>> const& e2)
				: first(e1), second(e2) {}
			virtual Nullable<Result> Parse(U32Itor& input, U32Itor const& end, Ptr<Expr<Void>> const& s)
			{
				Nullable<Out1> res1 = first->Parse(input, end, s);
				if (!res1.IsNull())
				{
					Nullable<Out2> res2 = second->Parse(input, end, s);
					if (!res2.IsNull())
						return MakeTuple<Out1, Out2>()(res1.Get(), res2.Get());
					return null;
				}
				return null;
			}
		};
		template <typename Out1, typename Out2>
		using SequType = typename Sequ<Out1, Out2>::Result;

		template <typename Out>
		struct Alt : Expr<Out>
		{
			using Result = Out;
			Ptr<Expr<Out>> left;
			Ptr<Expr<Out>> right;
			Alt(Ptr<Expr<Out>> const& l, Ptr<Expr<Out>> const& r)
				: left(l), right(r) {}
			virtual Nullable<Result> Parse(U32Itor& input, U32Itor const& end, Ptr<Expr<Void>> const& s)
			{
				U32Itor save = input;
				Nullable<Out> res1 = left->Parse(input, end, s);
				if (res1.IsNull())
				{
					input = save;
					Nullable<Out> res2 = right->Parse(input, end, s);
					if (res2.IsNull())
					{
						return null;
					}
					return res2.Get();
				}
				return res1.Get();
			}
		};

		template <int32_t Min, int32_t Max, typename Out>
		struct Rep : Expr<Vector<Out>>
		{
			using Result = Vector<Out>;
			Ptr<Expr<Out>> expr;
			Rep(Ptr<Expr<Out>> const& e)
				:expr(e) {}

			virtual Nullable<Result> Parse(U32Itor& input, U32Itor const& end, Ptr<Expr<Void>> const& s)
			{
				Vector<Out> ret;
				while (Max == -1 || ret.size() < Max)
				{
					Nullable<Out> res = expr->Parse(input, end, s);
					if (res.IsNull()) break;
					else ret.push_back(res.Get());
				}
				if ((Max == -1 || ret.size() <= Max) && ret.size() >= Min)
					return ret;
				else return null;
			}
		};
		template <int32_t Min, int32_t Max>
		struct Rep<Min, Max, Void> : Expr<Void>
		{
			using Result = Void;
			Ptr<Expr<Void>> expr;
			Rep(Ptr<Expr<Void>> const& e)
				:expr(e) {}

			virtual Nullable<Result> Parse(U32Itor& input, U32Itor const& end, Ptr<Expr<Void>> const& s)
			{
				uint32_t count{};
				while (Max == -1 || count < Max)
				{
					Nullable<Void> res = expr->Parse(input, end, s);
					if (res.IsNull()) break;
					else count++;
				}
				if ((Max == -1 || count <= Max) && count >= Min)
					return Void();
				else return null;
			}
		};
		template <int32_t Min, int32_t Max, typename Out>
		using RepType = typename Rep<Min, Max, Out>::Result;

		template <typename Out>
		struct Ref : Expr<WarpVoid<Out>>
		{
			using Result = WarpVoid<Out>;
			Rule<Out>& ref;
			Ref(Ref const& r) :ref(r.ref) {}
			Ref(Ref&& r) :ref(std::move(r.ref)) {}
			Ref(Rule<Out>& rule) :ref(rule) {}
			virtual Nullable<Result> Parse(U32Itor& input, U32Itor const& end, Ptr<Expr<Void>> const& s)
			{
				return ref.expr->Parse(input, end, s);
			}
		};

		template <typename In, typename Out>
		struct CastLoc : Expr<Out>
		{
			using Result = Out;
			TFuncLoc<In, Out> func;
			Ptr<Expr<In>> expr;
			CastLoc(Ptr<Expr<In>> const& e, TFuncLoc<In, Out> const& f)
				: func(f), expr(e) {}
			virtual Nullable<Result> Parse(U32Itor& input, U32Itor const& end, Ptr<Expr<Void>> const& s)
			{
				Nullable<In> res = expr->Parse(input, end, s);
				if (!res.IsNull()) return func(res.Get(), Location{
					input.Line(), input.Col() });
				return null;
			}
		};

		template <typename In>
		struct CastLoc<In, void> : Expr<Void>
		{
			using Result = Void;
			TFuncLoc<In, void> func;
			Ptr<Expr<In>> expr;
			CastLoc(Ptr<Expr<In>> const& e, TFuncLoc<In, void> const& f)
				: func(f), expr(e) {}
			virtual Nullable<Result> Parse(U32Itor& input, U32Itor const& end, Ptr<Expr<Void>> const& s)
			{
				Nullable<In> res = expr->Parse(input, end, s);
				if (!res.IsNull())
				{
					func(res.Get(), Location{
					input.Line(), input.Col() });
					return Void();
				}
				return null;
			}
		};

		template <typename In, typename Out>
		struct Cast : Expr<Out>
		{
			using Result = Out;
			TFunc<In, Out> func;
			Ptr<Expr<In>> expr;
			Cast(Ptr<Expr<In>> const& e, TFunc<In, Out> const& f)
				: func(f), expr(e) {}
			virtual Nullable<Result> Parse(U32Itor& input, U32Itor const& end, Ptr<Expr<Void>> const& s)
			{
				Nullable<In> res = expr->Parse(input, end, s);
				if (!res.IsNull()) return func(res.Get());
				return null;
			}
		};
		template <typename Out>
		struct Cast<Void, Out> : Expr<Out>
		{
			using Result = Out;
			TFunc<void, Out> func;
			Ptr<Expr<Void>> expr;
			Cast(Ptr<Expr<Void>> const& e, TFunc<void, Out> const& f)
				: func(f), expr(e) {}
			virtual Nullable<Result> Parse(U32Itor& input, U32Itor const& end, Ptr<Expr<Void>> const& s)
			{
				Nullable<Void> res = expr->Parse(input, end, s);
				if (!res.IsNull()) return func();
				return null;
			}
		};
		template <typename In>
		struct Cast<In, void> : Expr<Void>
		{
			using Result = Void;
			TFunc<In, void> func;
			Ptr<Expr<In>> expr;
			Cast(Ptr<Expr<In>> const& e, TFunc<In, void> const& f)
				: func(f), expr(e) {}
			virtual Nullable<Result> Parse(U32Itor& input, U32Itor const& end, Ptr<Expr<Void>> const& s)
			{
				Nullable<In> res = expr->Parse(input, end, s);
				if (!res.IsNull())
				{
					func(res.Get());
					return Void();
				}
				return null;
			}
		};
		template<>
		struct Cast<Void, void> : Expr<Void>
		{
			using Result = Void;
			TFunc<void, void> func;
			Ptr<Expr<Void>> expr;
			Cast(Ptr<Expr<Void>> const& e, TFunc<void, void> const& f)
				: func(f), expr(e) {}
			virtual Nullable<Result> Parse(U32Itor& input, U32Itor const& end, Ptr<Expr<Void>> const& s)
			{
				Nullable<Void> res = expr->Parse(input, end, s);
				if (!res.IsNull())
				{
					func();
					return Void();
				}
				return null;
			}
		};

		template <typename Out>
		struct Breaker : Expr<Out>
		{
			using Result = Out;
			Ptr<Expr<Out>> expr;
			Ptr<Expr<Void>> breaker;
			Breaker(Ptr<Expr<Out>> const& e1, Ptr<Expr<Void>> const& e2)
				:expr(e1), breaker(e2) {}
			virtual Nullable<Result> Parse(U32Itor& input, U32Itor const& end, Ptr<Expr<Void>> const& s)
			{
				U32Itor save = input;
				Nullable<Void> bk = breaker->Parse(save, end, s);
				if (!bk.IsNull()) return null;
				return expr->Parse(input, end, s);
			}
		};

		template <typename Out>
		struct SkipExpr : Expr<Out>
		{
			using Result = Out;
			Ptr<Expr<Out>> expr;
			Ptr<Expr<Void>> skip;
			SkipExpr(Ptr<Expr<Out>> const& e, CharSet const& s)
				: expr(e), skip(std::make_shared<SkipCharExpr>(s)) {}
			SkipExpr(Ptr<Expr<Out>> const& e)
				: expr(e), skip(nullptr) {}
			SkipExpr(Ptr<Expr<Out>> const& e1, Ptr<Expr<Void>> const& e2)
				: expr(e1), skip(e2) {}
			virtual Nullable<Result> Parse(U32Itor& input, U32Itor const& end, Ptr<Expr<Void>> const& s)
			{
				return expr->Parse(input, end, skip);
			}
		};

		template <typename Char>
		struct TokenExpr : Expr<Void>
		{
			using Result = Void;
			Vector<Char> str;
			TokenExpr(Char const* s)
			{
				while (*s) str.push_back(*s++);
			}
			virtual Nullable<Result> Parse(U32Itor& input, U32Itor const& end, Ptr<Expr<Void>> const& s)
			{
				while (s && !s->Parse(input, end, nullptr).IsNull());
				auto save = input;
				for (auto& ch : str)
				{
					if (save == end) return null;
					if (ch != *save) return null;
					save++;
				}
				input = save;
				return Void();
			}
		};
	}
}