#pragma once
#include "parser/Expr.h"
namespace em
{
	namespace parser
	{
		template <typename Out> struct Node;
		template <typename Out> struct Rule;
		struct CharSet; 
		struct CharSetExpr;
		struct ClearSkip {};
		extern const ClearSkip NoSkip;

		template <typename T>
		using WarpLambdaReturn = WarpVoid<LambdaReturn<T>>;
		template <typename F, typename Out>
		using CastNodeType = CheckedType<
			IsInvocable<F>::value
			&& (IsSame<TypeList<Out const&>, LambdaTakes<F>>
				|| (IsSame<Out, Void> && IsSame<TypeList<>, LambdaTakes<F>>)
				),
			Node<WarpLambdaReturn<F>>>;

		template <typename F, typename Out>
		using UnitNodeType = CheckedType<
			IsInvocable<F>::value
			&& (IsSame<TypeList<Out const&>, LambdaTakes<F>>
				|| (IsSame<Out, Void> && IsSame<TypeList<>, LambdaTakes<F>>)
				)
			&& IsSame<
			bool,
			LambdaReturn<F>>,
			Node<Out>>;

		template <typename F, typename Out>
		using CastNodeTypeWithLoc = CheckedType<
			IsInvocable<F>::value
			&& (IsSame<TypeList<Out const&, Location>, LambdaTakes<F>>
				|| (IsSame<Out, Void> && IsSame<TypeList<Location>, LambdaTakes<F>>)
				),
			Node<WarpLambdaReturn<F>>>;


		template <typename Out>
		struct Node
		{
			using Result = Out;
			Ptr<Expr<Out>> expr;
			Node(Node const& node) :expr(node.expr) {}
			Node(Node&& node) noexcept :expr(std::move(node.expr)) {}
			Node(Ptr<Expr<Out>> const& e) :expr(e) {}
			Node& operator=(Node const& node) { expr = node.expr; return *this; }
			Node& operator=(Node&& node) noexcept { expr = std::move(node.expr); return *this; }

			template <typename F>
			CastNodeType<F, Out> operator[](F const& f) const
			{
				using COut = LambdaReturn<F>;
				return Node<WarpVoid<COut>>{
					std::make_shared<Cast<Out, COut>>(
						expr, TFunc<Out, COut>(f))
				};
			}
			template <typename F>
			CastNodeTypeWithLoc<F, Out> operator[](F const& f) const
			{
				using COut = LambdaReturn<F>;
				return Node<WarpVoid<COut>>{
					std::make_shared<CastLoc<Out, COut>>(
						expr, TFuncLoc<Out, COut>(f))
				};
			}
			template <typename F>
			UnitNodeType<F, Out> operator()(F const& f) const
			{
				return Node<Out>{
					std::make_shared<Unit<Out>>(
						expr, TFunc<Out, bool>(f))
				};
			}
			template <typename F>
			CastNodeType<F, Out> operator^(F const& f) const
			{
				return operator[](f);
			}
			template <typename F>
			CastNodeTypeWithLoc<F, Out> operator^(F const& f) const
			{
				return operator[](f);
			}
			template <typename F>
			CastNodeType<F, Out> operator>>=(F const& f) const
			{
				return operator[](f);
			}
			template <typename F>
			CastNodeTypeWithLoc<F, Out> operator>>=(F const& f) const
			{
				return operator[](f);
			}

			Node<RepType<0, 1, Out>> operator-() const
			{
				return Node<RepType<0, 1, Out>>{
					std::make_shared<Rep<0, 1, Out>>(expr)
				};
			}
			Node<RepType<1, -1, Out>> operator+() const
			{
				return Node<RepType<1, -1, Out>>{
					std::make_shared<Rep<1, -1, Out>>(expr)
				};
			}
			Node<RepType<0, -1, Out>> operator*() const
			{
				return Node<RepType<0, -1, Out>>{
					std::make_shared<Rep<0, -1, Out>>(expr)
				};
			}

			template <typename COut>
			Node<SequType<Out, COut>>
				operator>(Node<COut> const& other) const
			{
				return Node<SequType<Out, COut>>{
					std::make_shared<Sequ<Out, COut>>(expr, other.expr)
				};
			}
			Node<Out> operator[](Node<Void> const& other) const
			{
				return Node<Out>{
					std::make_shared<Breaker<Out>>(expr, other.expr)
				};
			}
			template <typename Char>
			ValidCharReturn<Char, Node<SequType<Out, Void>>>
				operator>(Char const* other) const
			{
				return Node<SequType<Out, Void>>{
					std::make_shared<Sequ<Out, Void>>(
						expr, std::make_shared<TokenExpr<Char>>(other))
				};
			}
			Node<SequType<Out, CharT>> operator>(CharSet const& cset) const
			{
				return Node<SequType<Out, CharT>>{
					std::make_shared<Sequ<Out, CharT>>(
						expr, std::make_shared<CharSetExpr>(cset))
				};
			}
			template <typename COut>
			Node<SequType<Out, WarpVoid<COut>>>
				operator>(Rule<COut>& other) const
			{
				return Node<SequType<Out, WarpVoid<COut>>>{
					std::make_shared<Sequ<Out, WarpVoid<COut>>>(
						expr, std::make_shared<Ref<COut>>(other))
				};
			}

			Node<Out> operator|(Node<Out> const& other) const
			{
				return Node<Out>{
					std::make_shared<Alt<Out>>(expr, other.expr)
				};
			}

		};

		template <typename Char, typename Out>
		ValidCharReturn<Char, Node<SequType<Void, Out>>>
			operator>(Char const* left, Node<Out> const& right)
		{
			return Node<SequType<Void, Out>>{
				std::make_shared<Sequ<Void, Out>>(
					std::make_shared<TokenExpr<Char>>(left), right.expr)
			};
		}
		template <typename Char, typename Out>
		ValidCharReturn<Char, Node<SequType<Void, WarpVoid<Out>>>>
			operator>(Char const* left, Rule<Out> const& right)
		{
			return Node<SequType<Void, WarpVoid<Out>>>{
				std::make_shared<Sequ<Void, WarpVoid<Out>>>(
					std::make_shared<TokenExpr<Char>>(left),
					right.Alias())
			};
		}
		template <typename Char>
		ValidCharReturn<Char, Node<Void>>
			operator|(Char const* left, Node<Void> const& right)
		{
			return Node<Void>{
				std::make_shared<Alt<Void>>(
					std::make_shared<TokenExpr<Char>>(left), right.expr)
			};
		}
		template <typename Char, typename F>
		CastNodeType<F, Void> 
			operator^(Char const* left, F const& f)
		{
			return tk_(left) ^ f;
		}
		template<typename Out>
		Node<Out> 
			operator>>(Node<Void> const& skip, Node<Out> const& other)
		{
			return Node<Out>{
				std::make_shared<SkipExpr<Out>>(other.expr, skip.expr)
			};
		}
		template<typename Out>
		Node<Out> 
			operator>>(Node<Void> const& skip, Rule<Out> const& other)
		{
			return Node<Out>{
				std::make_shared<SkipExpr<Out>>(other.Alias(), skip.expr)
			};
		}
		template<typename Out>
		Node<Out> 
			operator>>(ClearSkip const& skip, Node<Out> const& other)
		{
			return Node<Out>{
				std::make_shared<SkipExpr<Out>>(other.expr)
			};
		}
	}
}