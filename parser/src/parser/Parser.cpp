#include "parser/Parser.h"

namespace em
{
	namespace parser
	{
		const ClearSkip NoSkip{};
		namespace p
		{
			TrieMap<Node<Void>> tokens;
			TrieMap<Node<Void>> skips;
			TrieMap<Node<CharT>> chars;
		}
	}
}