#pragma once
#include "parser/Core.h"
#include "parser/Node.h"
namespace em
{
	namespace parser
	{
		struct CharRange
		{
			using CharType = CharT;
			CharT from, to;

			CharRange(CharT const& c) :from(c), to(c) {}
			CharRange(CharT const& f, CharT const& t) :from(f), to(t) {}
			bool operator<(CharRange const& other) const;
			bool operator==(CharRange const& other) const;
		};
		struct CharSet
		{
		private:
			Ptr<Set<CharRange>> set = std::make_shared<Set<CharRange>>();
			bool except = true;
			inline void _Add(CharRange const& r);
			template <typename Char>
			inline void _AddStr(Char const* str);
		public:
			using CharType = CharT;
			CharSet() {}
			CharSet(CharSet const& other)
				:set(other.set), except(other.except) {}
			CharSet(std::initializer_list<CharRange> l);
			CharSet(char const* str);
			CharSet(wchar_t const* str);
			CharSet(char32_t const* str);

			bool Has(CharRange const& r) const;
			Node<Vector<CharType>> operator*() const;
			Node<Vector<CharType>> operator-() const;
			Node<Vector<CharType>> operator+() const;

			Ptr<CharSetExpr> GetExpr() const;

			template <typename Out>
			Node<SequType<CharType, Out>> operator>(Node<Out> const& other) const
			{
				return Node<SequType<CharT, Out>>{
					std::make_shared<Sequ<CharT, Out>>(
						this->GetExpr(), other.expr)
				};
			}
			Node<Tuple<CharT, CharT>> operator>(CharSet const&) const;

			template <typename Out>
			Node<WarpVoid<Out>> operator[](Function<Out(CharType const&)>const& f) const
			{
				return Node<WarpVoid<Out>>{
					std::make_shared<Cast<CharT, Out>>(
						this->GetExpr(), f)
				};
			}
			template <typename Out>
			Node<WarpVoid<Out>> operator[](Function<Out(CharType const&, Location)>const& f) const
			{
				return Node<WarpVoid<Out>>{
					std::make_shared<CastLoc<CharT, Out>>(
						this->GetExpr(), f)
				};
			}
			template <typename Out>
			Node<Out> operator>>(Node<Out> const& other) const
			{
				return Node<Out>{
					std::make_shared<SkipExpr<Out>>(other.expr,
						std::make_shared<SkipCharExpr>(*this))
				};
			}
			Node<char32_t> operator>>(CharSet const&) const;

		};

		struct CharSetExpr : Expr<char32_t>
		{
			using Result = char32_t;
			CharSet set;
			CharSetExpr(const CharSet& set) : set(set) {}
			CharSetExpr(CharSet&& set) :set(std::move(set)) {}
			virtual Nullable<Result> Parse(U32Itor& input, U32Itor const& end, Ptr<Expr<Void>> const& s);
		};

		struct SkipCharExpr : Expr<Void>
		{
			using Result = Void;
			CharSet skip;
			SkipCharExpr(CharSet const& s) : skip(s) {}
			virtual Nullable<Result> Parse(U32Itor& input, U32Itor const& end, Ptr<Expr<Void>> const& s);
		};
	}
}