#pragma once
#include "Node.h"
namespace em
{
	namespace parser
	{
		template <typename Out>
		struct Rule
		{
			using Result = CondType<IsSame<Out, void>, Void, Out>;
			Ptr<Expr<Result>> expr;
			Ptr<Ref<Out>> ref = std::make_shared<Ref<Out>>(*this);

			Rule() {};
			Rule(const Rule&) = delete;
			//Rule(const Node<Result>& n) { expr = n.expr; }
			Ptr<Ref<Out>> Alias() const { return ref; }

			Rule& operator=(const Node<Result>& n) 
			{
				expr = n.expr;
				return *this;
			}
			template <typename COut>
			Node<SequType<WarpVoid<Out>, COut>> operator>(Node<COut> const& n) const
			{
				return Node<SequType<WarpVoid<Out>, COut>>{
					std::make_shared<Sequ<WarpVoid<Out>, COut>>(Alias(), n.expr)
				};
			}
			template <typename Char>
			Node<SequType<WarpVoid<Out>, Void>> operator>(Char const* str) const
			{
				return Node<SequType<WarpVoid<Out>, Void>>{
					std::make_shared<Sequ<WarpVoid<Out>, Void>>(Alias(),
						std::make_shared<TokenExpr<Char>>(str))
				};
			}
		};
	}
}