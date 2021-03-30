#pragma once
#include <cuchar>
#include <locale>
#include "parser/Rule.h"
namespace em
{
	namespace parser
	{

		inline const char* ToUtf8(char32_t const* str, size_t size = -1)
		{
			static Vector<char> res;
			std::locale::global(std::locale(".utf8"));
			std::mbstate_t state{};
			res.clear();
			for (int i = 0; i < size && *str; i++, str++)
			{
				char out[MB_LEN_MAX]{};
				std::size_t rc = std::c32rtomb(out, *str, &state);
				for (int i = 0; i < rc; i++) res.push_back(out[i]);
			}
			std::locale::global(std::locale(""));
			res.push_back(0);
			return res.data();
		}

		template <typename Out>
		struct GrammarBase
		{
		private:
			Rule<Out>& startNode;
			Ptr<TrieMap<uint32_t>>
				strings = std::make_shared<TrieMap<uint32_t>>(),
				ids = std::make_shared<TrieMap<uint32_t>>();
			Ptr<Vector<String>>
				strings_store = std::make_shared<Vector<String>>(),
				ids_store = std::make_shared<Vector<String>>();

			uint32_t _StoredString(Vector<CharT> const& v)
			{
				String str(ToUtf8(v.data(), v.size()));
				auto i = strings->find(str.data());
				if (i == strings->end())
				{
					uint32_t ret = (uint32_t)strings_store->size();
					strings->insert(str.data(),
						strings_store->size());
					strings_store->push_back(str);
					return ret;
				}
				return *i;
			}
			uint32_t _StoredId(Vector<CharT> const& v)
			{
				String str(ToUtf8(v.data(), v.size()));
				auto i = ids->find(str.data());
				if (i == ids->end())
				{
					uint32_t ret = (uint32_t)ids_store->size();
					ids->insert(str.data(),
						ids_store->size());
					ids_store->push_back(str);
					return ret;
				}
				return *i;
			}
		protected:
			using base_type = GrammarBase<Out>;
			const Function<uint32_t(Vector<CharT> const&)>
				StoreString = std::bind(&base_type::_StoredString, this, std::placeholders::_1),
				StoreId = std::bind(&base_type::_StoredId, this, std::placeholders::_1);

		public:
			GrammarBase(Rule<Out>& start) : startNode(start) {}
			virtual ~GrammarBase() {}
			virtual Nullable<WarpVoid<Out>> Parse(U32Itor& input, U32Itor const& end, Ptr<Expr<Void>> const& s)
			{
				strings->clear();
				ids->clear();
				ids_store->clear();
				strings_store->clear();
				return startNode.expr->Parse(input, end, s);
			}
			String& GetId(uint32_t const& idx) { return ids_store->at(idx); }
			String& GetStr(uint32_t const& idx) { return strings_store->at(idx); }
		};
	}
}