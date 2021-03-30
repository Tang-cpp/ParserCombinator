#include <iostream>
#include <fstream>
#include "parser/Parser.h"
using namespace em;
using namespace em::parser;
struct Grammar : GrammarBase<int64_t>
{
private:
	enum class Operator { Add, Sub, Mul, Div };
	static int64_t DecVecToInt(Tuple<CharT, Vector<CharT>> const& t)
	{
		int64_t res = std::get<0>(t) - '0';
		Vector<CharT> const& ns = std::get<1>(t);
		for (const auto& ch : ns)
			if (ch != '`' && ch != '_')
				res = res * 10 + (ch - '0');
		return res;
	}
	static int64_t HexVecToInt(Vector<CharT> const& ns)
	{
		int64_t res = 0;
		for (const auto& ch : ns)
		{
			if (ch != '`' && ch != '_')
				res = res * 16 + (
					ch >= '0' && ch <= '9'
					? ch - '0'
					: (ch >= 'a' && ch <= 'f'
						? ch - 'a' + 10
						: ch - 'A' + 10));
		}
		return res;
	}
	static int64_t BinVecToInt(Vector<CharT> const& ns)
	{
		int64_t res = 0;
		for (const auto& ch : ns)
		{
			if (ch != '`' && ch != '_')
				res = res * 2 + (ch - '0');
		}
		return res;
	}
	static int64_t Calc(Tuple<int64_t, Vector<Tuple<Operator, int64_t>>> const& ns)
	{
		const auto& follows = std::get<1>(ns);
		int64_t res = std::get<0>(ns);
		for (const auto& item : follows)
		{
			const auto& op = std::get<0>(item);
			const auto& value = std::get<1>(item);
			switch (op)
			{
			case Operator::Add: res += value; break;
			case Operator::Sub: res -= value; break;
			case Operator::Mul: res *= value; break;
			case Operator::Div: res /= value; break;
			}
		}
		return res;
	}
public:
	using Result = int64_t;
	Rule<Result> start;
	Rule<int64_t> Expr;
	Grammar() : Grammar::base_type(start)
	{
		CharSet
			BinNumP = "01`_",
			DecNum = "0-9",
			DecNumP = "0-9`_",
			HexNumP = "0-9A-Fa-f`_";

		auto LF = char_("\n");
		auto WS = char_(U" \t\u3000");
		auto Comment
			= "//" > *char_("^\n") > LF
			| "/*" > *char_()[tk_("*/")] > "*/"
			;
		auto Skipper = LF | WS | Comment;

		auto Dec = DecNum > NoSkip >> *DecNumP ^ DecVecToInt;
		auto Bin = "0" > NoSkip >> (char_("bB") > +BinNumP) ^ BinVecToInt;
		auto Hex = "0" > NoSkip >> (char_("xX") > +HexNumP) ^ HexVecToInt;
		auto Integer = Bin | Hex | Dec;

		auto MultiplicativeOp
			= tk_("*") ^ []() { return Operator::Mul; }
			| tk_("/") ^ []() { return Operator::Div; }
			;
		auto AdditiveOp
			= tk_("+") ^ []() { return Operator::Add; }
			| tk_(U"プラス") ^ []() { return Operator::Add; }
			| tk_("-") ^ []() { return Operator::Sub; }
			;
		auto PrimaryExpr
			= Integer
			| "(" > Expr > ")"
			;
		auto MultiplicativeExpr = PrimaryExpr > *(MultiplicativeOp > PrimaryExpr) ^ Calc;
		Expr = MultiplicativeExpr > *(AdditiveOp > MultiplicativeExpr) ^ Calc;
		start = Skipper >> Expr;
	}
};
int main(int argc, char* argv[])
{
	using namespace std;
	string code;
	Grammar g;
	while (true)
	{
		{
			ifstream fs("input.txt");
			code = string(istreambuf_iterator<char>(fs), istreambuf_iterator<char>());
			fs.close();
			U32Itor begin(code.begin()), end(code.end());
			auto result = parser::Parse(begin, end, g);
			if (!result.IsNull() && begin == end)
			{
				cout << "Success:" << result.Get();
			}
			else
			{
				cout << "[Error]Line " << begin.Line() << ", Col " << begin.Col() << ".";
			}
		}
		
		getchar();
	}
	return 0;
}