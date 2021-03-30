#pragma once
#include "parser/GrammarBase.h"
#include "parser/CharSet.h"
namespace em
{
	namespace parser
	{
		namespace p
		{
			extern TrieMap<Node<Void>> tokens;
			extern TrieMap<Node<Void>> skips;
			extern TrieMap<Node<CharT>> chars;

			template <typename Ret, typename Char>
			inline Ret& _get_from_map(
				TrieMap<Ret>& map, char const* str, Ret(*func)(Char const*), Char const* arg)
			{
				auto i = map.find(str);
				if (i == map.end())
				{
					auto p = map.insert(str, func(arg));
					return *p.first;
				}
				return *i;
			}
			template <typename Ret>
			inline Ret& _get_from_map(
				TrieMap<Ret>& map, Ret(*func)())
			{
				auto i = map.find("");
				if (i == map.end())
				{
					auto p = map.insert("", func());
					return *p.first;
				}
				return *i;
			}
			template <typename Char>
			inline Node<Void> makeToken(Char const* str)
			{
				return Node<Void>{
					std::make_shared<TokenExpr<Char>>(str)
				};
			}
			template <typename Char>
			inline Node<Void> skip_a_char_(Char const* str)
			{
				return Node<Void>{
					std::make_shared<SkipCharExpr>(CharSet(str))
				};
			}
			inline Node<Void> skip_a_char_()
			{
				return Node<Void>{
					std::make_shared<SkipCharExpr>(CharSet())
				};
			}

			template <typename Char>
			inline Node<CharT> char_from_set_(Char const* str)
			{
				return Node<CharT>{
					std::make_shared<CharSetExpr>(CharSet(str))
				};
			}
			inline Node<CharT> char_from_set_()
			{
				return Node<CharT>{
					std::make_shared<CharSetExpr>(CharSet())
				};
			}
		}

		inline Node<Void>& tk_(char const* str)
		{
			using F = Node<Void>(*)(char const*);
			return p::_get_from_map(p::tokens, str,
				static_cast<F>(p::makeToken<char>),
				str);
		}
		inline Node<Void>& tk_(char32_t const* str)
		{
			char const* utf8 = ToUtf8(str);
			using F = Node<Void>(*)(char32_t const*);
			return p::_get_from_map(p::tokens, utf8,
				static_cast<F>(p::makeToken<char32_t>),
				str);
		}
		inline Node<Void>& char_(char32_t const* str)
		{
			using F = Node<Void>(*)(char32_t const*);
			char const* utf8 = ToUtf8(str);
			return p::_get_from_map(p::skips, utf8,
				static_cast<F>(p::skip_a_char_<char32_t>),
				str);
		}
		inline Node<Void>& char_(char const* str)
		{
			using F = Node<Void>(*)(char const*);
			return p::_get_from_map(p::skips, str,
				static_cast<F>(p::skip_a_char_<char>),
				str);
		}
		inline Node<Void>& char_()
		{
			return p::_get_from_map(p::skips, p::skip_a_char_);
		}
		inline Node<CharT>& set_(char32_t const* str)
		{
			using F = Node<CharT>(*)(char32_t const*);
			char const* utf8 = ToUtf8(str);
			return p::_get_from_map(p::chars, utf8,
				static_cast<F>(p::char_from_set_<char32_t>),
				str);
		}
		inline Node<CharT>& set_(char const* str)
		{
			using F = Node<CharT>(*)(char const*);
			return p::_get_from_map(p::chars, str,
				static_cast<F>(p::char_from_set_<char>),
				str);
		}
		inline Node<CharT>& set_()
		{
			return p::_get_from_map(p::chars, p::char_from_set_);
		}

		template <typename Out>
		inline Nullable<WarpVoid<Out>> Parse(U32Itor& input, U32Itor const& end, GrammarBase<Out>& g)
		{
			return g.Parse(input, end, nullptr);
		}
		inline void Clear()
		{
			p::tokens.clear();
		}
		inline Vector<String> Tokens()
		{
			Vector<String> ret;
			for (auto i = p::tokens.begin(); i != p::tokens.end(); i++)
				ret.push_back(i.key());
			return ret;
		}
	}
}