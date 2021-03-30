#include "parser/CharSet.h"
namespace em
{
	namespace parser
	{
		bool CharRange::operator<(CharRange const& other) const
		{
			return to < other.from;
		}
		bool CharRange::operator==(CharRange const& other) const
		{
			return to == other.to && from == other.from;
		}

		template<typename Char>
		inline void CharSet::_AddStr(Char const* str)
		{
			while (*str)
			{
				if ('\\' == *str && *(str + 1))
				{
					_Add({ (CharT) * (str + 1) });
					str += 2;
				}
				else if (*(str + 1) == '-' && *(str + 2) > *str)
				{
					_Add({ (CharT)*str, (CharT) * (str + 2) });
					str += 3;
				}
				else
				{
					_Add({ (CharT)*str });
					str++;
				}
			}
		}
		inline void CharSet::_Add(const CharRange& range)
		{
			CharRange r = range;
			auto i = set->find(r);
			if (i != set->end() && !(r == *i))
			{
				do
				{
					r.from = std::min(r.from, i->from);
					r.to = std::max(r.to, i->to);
					set->erase(i);
					i = set->find(r);
				} while (i != set->end());
			}
			i = set->find({ r.from - 1, r.to });
			if (i != set->end()) { r.from = i->from; set->erase(i); }
			i = set->find({ r.from, r.to + 1 });
			if (i != set->end()) { r.to = i->to; set->erase(i); }
			set->insert(r);
		}

		CharSet::CharSet(std::initializer_list<CharRange> l)
			: except(false)
		{
			for (auto& r : l) _Add(r);
		}
		CharSet::CharSet(char const* str)
			: except('^' == *str)
		{
			_AddStr<char>(except ? str + 1 : str);
		}
		CharSet::CharSet(wchar_t const* str)
			: except('^' == *str)
		{
			_AddStr<wchar_t>(except ? str + 1 : str);
		}
		CharSet::CharSet(char32_t const* str)
			: except('^' == *str)
		{
			_AddStr<char32_t>(except ? str + 1 : str);
		}

		bool CharSet::Has(CharRange const& r) const
		{
			return except ^ (set->find(r) != set->cend());
		}

		Node<RepType<0, -1, CharT>> CharSet::operator*() const
		{
			return Node<RepType<0, -1, CharT>>{
				std::make_shared<Rep<0, -1, CharT>>(this->GetExpr())
			};
		}
		Node<RepType<0, 1, CharT>> CharSet::operator-() const
		{
			return Node<RepType<0, 1, CharT>>{
				std::make_shared<Rep<0, 1, CharT>>(this->GetExpr())
			};
		}
		Node<RepType<1, -1, CharT>> CharSet::operator+() const
		{
			return Node<RepType<1, -1, CharT>>{
				std::make_shared<Rep<1, -1, CharT>>(this->GetExpr())
			};
		}

		Ptr<CharSetExpr> CharSet::GetExpr() const
		{
			return std::make_shared<CharSetExpr>(*this);
		}
		Node<CharT> CharSet::operator>>(CharSet const& s) const
		{
			return operator>>(Node<CharT>{s.GetExpr()});
		}

		Node<Tuple<CharT, CharT>> CharSet::operator>(CharSet const& s) const
		{
			return operator>(Node<CharT>{s.GetExpr()});
		}

		Nullable<CharSetExpr::Result> CharSetExpr::Parse(U32Itor& input, U32Itor const& end, Ptr<Expr<Void>> const& s)
		{
			while (s && !s->Parse(input, end, nullptr).IsNull());
			if (input == end) return null;
			CharT ret = *input;
			if (set.Has(ret)) { input++; return ret; }
			return null;
		}

		Nullable<SkipCharExpr::Result> SkipCharExpr::Parse(U32Itor& input, U32Itor const& end, Ptr<Expr<Void>> const& s)
		{
			while (s && !s->Parse(input, end, nullptr).IsNull());
			if (input == end) return null;
			if (skip.Has(*input))
			{
				input++;
				return Void();
			}
			return null;
		}


		const CharSet XID_Start{
			{0x0041, 0x005A},
			{0x005F},
			{0x0061, 0x007A},
			{0x00AA},
			{0x00B5},
			{0x00BA},
			{0x00C0, 0x00D6},
			{0x00D8, 0x00F6},
			{0x00F8, 0x01BA},
			{0x01BB},
			{0x01BC, 0x01BF},
			{0x01C0, 0x01C3},
			{0x01C4, 0x0241},
			{0x0250, 0x02AF},
			{0x02B0, 0x02C1},
			{0x02C6, 0x02D1},
			{0x02E0, 0x02E4},
			{0x02EE},
			{0x0386},
			{0x0388, 0x038A},
			{0x038C},
			{0x038E, 0x03A1},
			{0x03A3, 0x03CE},
			{0x03D0, 0x03F5},
			{0x03F7, 0x0481},
			{0x048A, 0x04CE},
			{0x04D0, 0x04F9},
			{0x0500, 0x050F},
			{0x0531, 0x0556},
			{0x0559},
			{0x0561, 0x0587},
			{0x05D0, 0x05EA},
			{0x05F0, 0x05F2},
			{0x0621, 0x063A},
			{0x0640},
			{0x0641, 0x064A},
			{0x066E, 0x066F},
			{0x0671, 0x06D3},
			{0x06D5},
			{0x06E5, 0x06E6},
			{0x06EE, 0x06EF},
			{0x06FA, 0x06FC},
			{0x06FF},
			{0x0710},
			{0x0712, 0x072F},
			{0x074D, 0x076D},
			{0x0780, 0x07A5},
			{0x07B1},
			{0x0904, 0x0939},
			{0x093D},
			{0x0950},
			{0x0958, 0x0961},
			{0x097D},
			{0x0985, 0x098C},
			{0x098F, 0x0990},
			{0x0993, 0x09A8},
			{0x09AA, 0x09B0},
			{0x09B2},
			{0x09B6, 0x09B9},
			{0x09BD},
			{0x09CE},
			{0x09DC, 0x09DD},
			{0x09DF, 0x09E1},
			{0x09F0, 0x09F1},
			{0x0A05, 0x0A0A},
			{0x0A0F, 0x0A10},
			{0x0A13, 0x0A28},
			{0x0A2A, 0x0A30},
			{0x0A32, 0x0A33},
			{0x0A35, 0x0A36},
			{0x0A38, 0x0A39},
			{0x0A59, 0x0A5C},
			{0x0A5E},
			{0x0A72, 0x0A74},
			{0x0A85, 0x0A8D},
			{0x0A8F, 0x0A91},
			{0x0A93, 0x0AA8},
			{0x0AAA, 0x0AB0},
			{0x0AB2, 0x0AB3},
			{0x0AB5, 0x0AB9},
			{0x0ABD},
			{0x0AD0},
			{0x0AE0, 0x0AE1},
			{0x0B05, 0x0B0C},
			{0x0B0F, 0x0B10},
			{0x0B13, 0x0B28},
			{0x0B2A, 0x0B30},
			{0x0B32, 0x0B33},
			{0x0B35, 0x0B39},
			{0x0B3D},
			{0x0B5C, 0x0B5D},
			{0x0B5F, 0x0B61},
			{0x0B71},
			{0x0B83},
			{0x0B85, 0x0B8A},
			{0x0B8E, 0x0B90},
			{0x0B92, 0x0B95},
			{0x0B99, 0x0B9A},
			{0x0B9C},
			{0x0B9E, 0x0B9F},
			{0x0BA3, 0x0BA4},
			{0x0BA8, 0x0BAA},
			{0x0BAE, 0x0BB9},
			{0x0C05, 0x0C0C},
			{0x0C0E, 0x0C10},
			{0x0C12, 0x0C28},
			{0x0C2A, 0x0C33},
			{0x0C35, 0x0C39},
			{0x0C60, 0x0C61},
			{0x0C85, 0x0C8C},
			{0x0C8E, 0x0C90},
			{0x0C92, 0x0CA8},
			{0x0CAA, 0x0CB3},
			{0x0CB5, 0x0CB9},
			{0x0CBD},
			{0x0CDE},
			{0x0CE0, 0x0CE1},
			{0x0D05, 0x0D0C},
			{0x0D0E, 0x0D10},
			{0x0D12, 0x0D28},
			{0x0D2A, 0x0D39},
			{0x0D60, 0x0D61},
			{0x0D85, 0x0D96},
			{0x0D9A, 0x0DB1},
			{0x0DB3, 0x0DBB},
			{0x0DBD},
			{0x0DC0, 0x0DC6},
			{0x0E01, 0x0E30},
			{0x0E32},
			{0x0E40, 0x0E45},
			{0x0E46},
			{0x0E81, 0x0E82},
			{0x0E84},
			{0x0E87, 0x0E88},
			{0x0E8A},
			{0x0E8D},
			{0x0E94, 0x0E97},
			{0x0E99, 0x0E9F},
			{0x0EA1, 0x0EA3},
			{0x0EA5},
			{0x0EA7},
			{0x0EAA, 0x0EAB},
			{0x0EAD, 0x0EB0},
			{0x0EB2},
			{0x0EBD},
			{0x0EC0, 0x0EC4},
			{0x0EC6},
			{0x0EDC, 0x0EDD},
			{0x0F00},
			{0x0F40, 0x0F47},
			{0x0F49, 0x0F6A},
			{0x0F88, 0x0F8B},
			{0x1000, 0x1021},
			{0x1023, 0x1027},
			{0x1029, 0x102A},
			{0x1050, 0x1055},
			{0x10A0, 0x10C5},
			{0x10D0, 0x10FA},
			{0x10FC},
			{0x1100, 0x1159},
			{0x115F, 0x11A2},
			{0x11A8, 0x11F9},
			{0x1200, 0x1248},
			{0x124A, 0x124D},
			{0x1250, 0x1256},
			{0x1258},
			{0x125A, 0x125D},
			{0x1260, 0x1288},
			{0x128A, 0x128D},
			{0x1290, 0x12B0},
			{0x12B2, 0x12B5},
			{0x12B8, 0x12BE},
			{0x12C0},
			{0x12C2, 0x12C5},
			{0x12C8, 0x12D6},
			{0x12D8, 0x1310},
			{0x1312, 0x1315},
			{0x1318, 0x135A},
			{0x1380, 0x138F},
			{0x13A0, 0x13F4},
			{0x1401, 0x166C},
			{0x166F, 0x1676},
			{0x1681, 0x169A},
			{0x16A0, 0x16EA},
			{0x16EE, 0x16F0},
			{0x1700, 0x170C},
			{0x170E, 0x1711},
			{0x1720, 0x1731},
			{0x1740, 0x1751},
			{0x1760, 0x176C},
			{0x176E, 0x1770},
			{0x1780, 0x17B3},
			{0x17D7},
			{0x17DC},
			{0x1820, 0x1842},
			{0x1843},
			{0x1844, 0x1877},
			{0x1880, 0x18A8},
			{0x1900, 0x191C},
			{0x1950, 0x196D},
			{0x1970, 0x1974},
			{0x1980, 0x19A9},
			{0x19C1, 0x19C7},
			{0x1A00, 0x1A16},
			{0x1D00, 0x1D2B},
			{0x1D2C, 0x1D61},
			{0x1D62, 0x1D77},
			{0x1D78},
			{0x1D79, 0x1D9A},
			{0x1D9B, 0x1DBF},
			{0x1E00, 0x1E9B},
			{0x1EA0, 0x1EF9},
			{0x1F00, 0x1F15},
			{0x1F18, 0x1F1D},
			{0x1F20, 0x1F45},
			{0x1F48, 0x1F4D},
			{0x1F50, 0x1F57},
			{0x1F59},
			{0x1F5B},
			{0x1F5D},
			{0x1F5F, 0x1F7D},
			{0x1F80, 0x1FB4},
			{0x1FB6, 0x1FBC},
			{0x1FBE},
			{0x1FC2, 0x1FC4},
			{0x1FC6, 0x1FCC},
			{0x1FD0, 0x1FD3},
			{0x1FD6, 0x1FDB},
			{0x1FE0, 0x1FEC},
			{0x1FF2, 0x1FF4},
			{0x1FF6, 0x1FFC},
			{0x2071},
			{0x207F},
			{0x2090, 0x2094},
			{0x2102},
			{0x2107},
			{0x210A, 0x2113},
			{0x2115},
			{0x2118},
			{0x2119, 0x211D},
			{0x2124},
			{0x2126},
			{0x2128},
			{0x212A, 0x212D},
			{0x212E},
			{0x212F, 0x2131},
			{0x2133, 0x2134},
			{0x2135, 0x2138},
			{0x2139},
			{0x213C, 0x213F},
			{0x2145, 0x2149},
			{0x2160, 0x2183},
			{0x2C00, 0x2C2E},
			{0x2C30, 0x2C5E},
			{0x2C80, 0x2CE4},
			{0x2D00, 0x2D25},
			{0x2D30, 0x2D65},
			{0x2D6F},
			{0x2D80, 0x2D96},
			{0x2DA0, 0x2DA6},
			{0x2DA8, 0x2DAE},
			{0x2DB0, 0x2DB6},
			{0x2DB8, 0x2DBE},
			{0x2DC0, 0x2DC6},
			{0x2DC8, 0x2DCE},
			{0x2DD0, 0x2DD6},
			{0x2DD8, 0x2DDE},
			{0x3005},
			{0x3006},
			{0x3007},
			{0x3021, 0x3029},
			{0x3031, 0x3035},
			{0x3038, 0x303A},
			{0x303B},
			{0x303C},
			{0x3041, 0x3096},
			{0x309D, 0x309E},
			{0x309F},
			{0x30A1, 0x30FA},
			{0x30FC, 0x30FE},
			{0x30FF},
			{0x3105, 0x312C},
			{0x3131, 0x318E},
			{0x31A0, 0x31B7},
			{0x31F0, 0x31FF},
			{0x3400, 0x4DB5},
			{0x4E00, 0x9FBB},
			{0xA000, 0xA014},
			{0xA015},
			{0xA016, 0xA48C},
			{0xA800, 0xA801},
			{0xA803, 0xA805},
			{0xA807, 0xA80A},
			{0xA80C, 0xA822},
			{0xAC00, 0xD7A3},
			{0xF900, 0xFA2D},
			{0xFA30, 0xFA6A},
			{0xFA70, 0xFAD9},
			{0xFB00, 0xFB06},
			{0xFB13, 0xFB17},
			{0xFB1D},
			{0xFB1F, 0xFB28},
			{0xFB2A, 0xFB36},
			{0xFB38, 0xFB3C},
			{0xFB3E},
			{0xFB40, 0xFB41},
			{0xFB43, 0xFB44},
			{0xFB46, 0xFBB1},
			{0xFBD3, 0xFC5D},
			{0xFC64, 0xFD3D},
			{0xFD50, 0xFD8F},
			{0xFD92, 0xFDC7},
			{0xFDF0, 0xFDF9},
			{0xFE71},
			{0xFE73},
			{0xFE77},
			{0xFE79},
			{0xFE7B},
			{0xFE7D},
			{0xFE7F, 0xFEFC},
			{0xFF21, 0xFF3A},
			{0xFF41, 0xFF5A},
			{0xFF66, 0xFF6F},
			{0xFF70},
			{0xFF71, 0xFF9D},
			{0xFFA0, 0xFFBE},
			{0xFFC2, 0xFFC7},
			{0xFFCA, 0xFFCF},
			{0xFFD2, 0xFFD7},
			{0xFFDA, 0xFFDC},
			{0x10000, 0x1000B},
			{0x1000D, 0x10026},
			{0x10028, 0x1003A},
			{0x1003C, 0x1003D},
			{0x1003F, 0x1004D},
			{0x10050, 0x1005D},
			{0x10080, 0x100FA},
			{0x10140, 0x10174},
			{0x10300, 0x1031E},
			{0x10330, 0x10349},
			{0x1034A},
			{0x10380, 0x1039D},
			{0x103A0, 0x103C3},
			{0x103C8, 0x103CF},
			{0x103D1, 0x103D5},
			{0x10400, 0x1044F},
			{0x10450, 0x1049D},
			{0x10800, 0x10805},
			{0x10808},
			{0x1080A, 0x10835},
			{0x10837, 0x10838},
			{0x1083C},
			{0x1083F},
			{0x10A00},
			{0x10A10, 0x10A13},
			{0x10A15, 0x10A17},
			{0x10A19, 0x10A33},
			{0x1D400, 0x1D454},
			{0x1D456, 0x1D49C},
			{0x1D49E, 0x1D49F},
			{0x1D4A2},
			{0x1D4A5, 0x1D4A6},
			{0x1D4A9, 0x1D4AC},
			{0x1D4AE, 0x1D4B9},
			{0x1D4BB},
			{0x1D4BD, 0x1D4C3},
			{0x1D4C5, 0x1D505},
			{0x1D507, 0x1D50A},
			{0x1D50D, 0x1D514},
			{0x1D516, 0x1D51C},
			{0x1D51E, 0x1D539},
			{0x1D53B, 0x1D53E},
			{0x1D540, 0x1D544},
			{0x1D546},
			{0x1D54A, 0x1D550},
			{0x1D552, 0x1D6A5},
			{0x1D6A8, 0x1D6C0},
			{0x1D6C2, 0x1D6DA},
			{0x1D6DC, 0x1D6FA},
			{0x1D6FC, 0x1D714},
			{0x1D716, 0x1D734},
			{0x1D736, 0x1D74E},
			{0x1D750, 0x1D76E},
			{0x1D770, 0x1D788},
			{0x1D78A, 0x1D7A8},
			{0x1D7AA, 0x1D7C2},
			{0x1D7C4, 0x1D7C9},
			{0x20000, 0x2A6D6},
			{0x2F800, 0x2FA1D}
		};
		const CharSet XID_Continue{
			{0x0030, 0x0039},
			{0x0041, 0x005A},
			{0x005F},
			{0x0061, 0x007A},
			{0x00AA},
			{0x00B5},
			{0x00B7},
			{0x00BA},
			{0x00C0, 0x00D6},
			{0x00D8, 0x00F6},
			{0x00F8, 0x01BA},
			{0x01BB},
			{0x01BC, 0x01BF},
			{0x01C0, 0x01C3},
			{0x01C4, 0x0241},
			{0x0250, 0x02AF},
			{0x02B0, 0x02C1},
			{0x02C6, 0x02D1},
			{0x02E0, 0x02E4},
			{0x02EE},
			{0x0300, 0x036F},
			{0x0386},
			{0x0388, 0x038A},
			{0x038C},
			{0x038E, 0x03A1},
			{0x03A3, 0x03CE},
			{0x03D0, 0x03F5},
			{0x03F7, 0x0481},
			{0x0483, 0x0486},
			{0x048A, 0x04CE},
			{0x04D0, 0x04F9},
			{0x0500, 0x050F},
			{0x0531, 0x0556},
			{0x0559},
			{0x0561, 0x0587},
			{0x0591, 0x05B9},
			{0x05BB, 0x05BD},
			{0x05BF},
			{0x05C1, 0x05C2},
			{0x05C4, 0x05C5},
			{0x05C7},
			{0x05D0, 0x05EA},
			{0x05F0, 0x05F2},
			{0x0610, 0x0615},
			{0x0621, 0x063A},
			{0x0640},
			{0x0641, 0x064A},
			{0x064B, 0x065E},
			{0x0660, 0x0669},
			{0x066E, 0x066F},
			{0x0670},
			{0x0671, 0x06D3},
			{0x06D5},
			{0x06D6, 0x06DC},
			{0x06DF, 0x06E4},
			{0x06E5, 0x06E6},
			{0x06E7, 0x06E8},
			{0x06EA, 0x06ED},
			{0x06EE, 0x06EF},
			{0x06F0, 0x06F9},
			{0x06FA, 0x06FC},
			{0x06FF},
			{0x0710},
			{0x0711},
			{0x0712, 0x072F},
			{0x0730, 0x074A},
			{0x074D, 0x076D},
			{0x0780, 0x07A5},
			{0x07A6, 0x07B0},
			{0x07B1},
			{0x0901, 0x0902},
			{0x0903},
			{0x0904, 0x0939},
			{0x093C},
			{0x093D},
			{0x093E, 0x0940},
			{0x0941, 0x0948},
			{0x0949, 0x094C},
			{0x094D},
			{0x0950},
			{0x0951, 0x0954},
			{0x0958, 0x0961},
			{0x0962, 0x0963},
			{0x0966, 0x096F},
			{0x097D},
			{0x0981},
			{0x0982, 0x0983},
			{0x0985, 0x098C},
			{0x098F, 0x0990},
			{0x0993, 0x09A8},
			{0x09AA, 0x09B0},
			{0x09B2},
			{0x09B6, 0x09B9},
			{0x09BC},
			{0x09BD},
			{0x09BE, 0x09C0},
			{0x09C1, 0x09C4},
			{0x09C7, 0x09C8},
			{0x09CB, 0x09CC},
			{0x09CD},
			{0x09CE},
			{0x09D7},
			{0x09DC, 0x09DD},
			{0x09DF, 0x09E1},
			{0x09E2, 0x09E3},
			{0x09E6, 0x09EF},
			{0x09F0, 0x09F1},
			{0x0A01, 0x0A02},
			{0x0A03},
			{0x0A05, 0x0A0A},
			{0x0A0F, 0x0A10},
			{0x0A13, 0x0A28},
			{0x0A2A, 0x0A30},
			{0x0A32, 0x0A33},
			{0x0A35, 0x0A36},
			{0x0A38, 0x0A39},
			{0x0A3C},
			{0x0A3E, 0x0A40},
			{0x0A41, 0x0A42},
			{0x0A47, 0x0A48},
			{0x0A4B, 0x0A4D},
			{0x0A59, 0x0A5C},
			{0x0A5E},
			{0x0A66, 0x0A6F},
			{0x0A70, 0x0A71},
			{0x0A72, 0x0A74},
			{0x0A81, 0x0A82},
			{0x0A83},
			{0x0A85, 0x0A8D},
			{0x0A8F, 0x0A91},
			{0x0A93, 0x0AA8},
			{0x0AAA, 0x0AB0},
			{0x0AB2, 0x0AB3},
			{0x0AB5, 0x0AB9},
			{0x0ABC},
			{0x0ABD},
			{0x0ABE, 0x0AC0},
			{0x0AC1, 0x0AC5},
			{0x0AC7, 0x0AC8},
			{0x0AC9},
			{0x0ACB, 0x0ACC},
			{0x0ACD},
			{0x0AD0},
			{0x0AE0, 0x0AE1},
			{0x0AE2, 0x0AE3},
			{0x0AE6, 0x0AEF},
			{0x0B01},
			{0x0B02, 0x0B03},
			{0x0B05, 0x0B0C},
			{0x0B0F, 0x0B10},
			{0x0B13, 0x0B28},
			{0x0B2A, 0x0B30},
			{0x0B32, 0x0B33},
			{0x0B35, 0x0B39},
			{0x0B3C},
			{0x0B3D},
			{0x0B3E},
			{0x0B3F},
			{0x0B40},
			{0x0B41, 0x0B43},
			{0x0B47, 0x0B48},
			{0x0B4B, 0x0B4C},
			{0x0B4D},
			{0x0B56},
			{0x0B57},
			{0x0B5C, 0x0B5D},
			{0x0B5F, 0x0B61},
			{0x0B66, 0x0B6F},
			{0x0B71},
			{0x0B82},
			{0x0B83},
			{0x0B85, 0x0B8A},
			{0x0B8E, 0x0B90},
			{0x0B92, 0x0B95},
			{0x0B99, 0x0B9A},
			{0x0B9C},
			{0x0B9E, 0x0B9F},
			{0x0BA3, 0x0BA4},
			{0x0BA8, 0x0BAA},
			{0x0BAE, 0x0BB9},
			{0x0BBE, 0x0BBF},
			{0x0BC0},
			{0x0BC1, 0x0BC2},
			{0x0BC6, 0x0BC8},
			{0x0BCA, 0x0BCC},
			{0x0BCD},
			{0x0BD7},
			{0x0BE6, 0x0BEF},
			{0x0C01, 0x0C03},
			{0x0C05, 0x0C0C},
			{0x0C0E, 0x0C10},
			{0x0C12, 0x0C28},
			{0x0C2A, 0x0C33},
			{0x0C35, 0x0C39},
			{0x0C3E, 0x0C40},
			{0x0C41, 0x0C44},
			{0x0C46, 0x0C48},
			{0x0C4A, 0x0C4D},
			{0x0C55, 0x0C56},
			{0x0C60, 0x0C61},
			{0x0C66, 0x0C6F},
			{0x0C82, 0x0C83},
			{0x0C85, 0x0C8C},
			{0x0C8E, 0x0C90},
			{0x0C92, 0x0CA8},
			{0x0CAA, 0x0CB3},
			{0x0CB5, 0x0CB9},
			{0x0CBC},
			{0x0CBD},
			{0x0CBE},
			{0x0CBF},
			{0x0CC0, 0x0CC4},
			{0x0CC6},
			{0x0CC7, 0x0CC8},
			{0x0CCA, 0x0CCB},
			{0x0CCC, 0x0CCD},
			{0x0CD5, 0x0CD6},
			{0x0CDE},
			{0x0CE0, 0x0CE1},
			{0x0CE6, 0x0CEF},
			{0x0D02, 0x0D03},
			{0x0D05, 0x0D0C},
			{0x0D0E, 0x0D10},
			{0x0D12, 0x0D28},
			{0x0D2A, 0x0D39},
			{0x0D3E, 0x0D40},
			{0x0D41, 0x0D43},
			{0x0D46, 0x0D48},
			{0x0D4A, 0x0D4C},
			{0x0D4D},
			{0x0D57},
			{0x0D60, 0x0D61},
			{0x0D66, 0x0D6F},
			{0x0D82, 0x0D83},
			{0x0D85, 0x0D96},
			{0x0D9A, 0x0DB1},
			{0x0DB3, 0x0DBB},
			{0x0DBD},
			{0x0DC0, 0x0DC6},
			{0x0DCA},
			{0x0DCF, 0x0DD1},
			{0x0DD2, 0x0DD4},
			{0x0DD6},
			{0x0DD8, 0x0DDF},
			{0x0DF2, 0x0DF3},
			{0x0E01, 0x0E30},
			{0x0E31},
			{0x0E32, 0x0E33},
			{0x0E34, 0x0E3A},
			{0x0E40, 0x0E45},
			{0x0E46},
			{0x0E47, 0x0E4E},
			{0x0E50, 0x0E59},
			{0x0E81, 0x0E82},
			{0x0E84},
			{0x0E87, 0x0E88},
			{0x0E8A},
			{0x0E8D},
			{0x0E94, 0x0E97},
			{0x0E99, 0x0E9F},
			{0x0EA1, 0x0EA3},
			{0x0EA5},
			{0x0EA7},
			{0x0EAA, 0x0EAB},
			{0x0EAD, 0x0EB0},
			{0x0EB1},
			{0x0EB2, 0x0EB3},
			{0x0EB4, 0x0EB9},
			{0x0EBB, 0x0EBC},
			{0x0EBD},
			{0x0EC0, 0x0EC4},
			{0x0EC6},
			{0x0EC8, 0x0ECD},
			{0x0ED0, 0x0ED9},
			{0x0EDC, 0x0EDD},
			{0x0F00},
			{0x0F18, 0x0F19},
			{0x0F20, 0x0F29},
			{0x0F35},
			{0x0F37},
			{0x0F39},
			{0x0F3E, 0x0F3F},
			{0x0F40, 0x0F47},
			{0x0F49, 0x0F6A},
			{0x0F71, 0x0F7E},
			{0x0F7F},
			{0x0F80, 0x0F84},
			{0x0F86, 0x0F87},
			{0x0F88, 0x0F8B},
			{0x0F90, 0x0F97},
			{0x0F99, 0x0FBC},
			{0x0FC6},
			{0x1000, 0x1021},
			{0x1023, 0x1027},
			{0x1029, 0x102A},
			{0x102C},
			{0x102D, 0x1030},
			{0x1031},
			{0x1032},
			{0x1036, 0x1037},
			{0x1038},
			{0x1039},
			{0x1040, 0x1049},
			{0x1050, 0x1055},
			{0x1056, 0x1057},
			{0x1058, 0x1059},
			{0x10A0, 0x10C5},
			{0x10D0, 0x10FA},
			{0x10FC},
			{0x1100, 0x1159},
			{0x115F, 0x11A2},
			{0x11A8, 0x11F9},
			{0x1200, 0x1248},
			{0x124A, 0x124D},
			{0x1250, 0x1256},
			{0x1258},
			{0x125A, 0x125D},
			{0x1260, 0x1288},
			{0x128A, 0x128D},
			{0x1290, 0x12B0},
			{0x12B2, 0x12B5},
			{0x12B8, 0x12BE},
			{0x12C0},
			{0x12C2, 0x12C5},
			{0x12C8, 0x12D6},
			{0x12D8, 0x1310},
			{0x1312, 0x1315},
			{0x1318, 0x135A},
			{0x135F},
			{0x1369, 0x1371},
			{0x1380, 0x138F},
			{0x13A0, 0x13F4},
			{0x1401, 0x166C},
			{0x166F, 0x1676},
			{0x1681, 0x169A},
			{0x16A0, 0x16EA},
			{0x16EE, 0x16F0},
			{0x1700, 0x170C},
			{0x170E, 0x1711},
			{0x1712, 0x1714},
			{0x1720, 0x1731},
			{0x1732, 0x1734},
			{0x1740, 0x1751},
			{0x1752, 0x1753},
			{0x1760, 0x176C},
			{0x176E, 0x1770},
			{0x1772, 0x1773},
			{0x1780, 0x17B3},
			{0x17B6},
			{0x17B7, 0x17BD},
			{0x17BE, 0x17C5},
			{0x17C6},
			{0x17C7, 0x17C8},
			{0x17C9, 0x17D3},
			{0x17D7},
			{0x17DC},
			{0x17DD},
			{0x17E0, 0x17E9},
			{0x180B, 0x180D},
			{0x1810, 0x1819},
			{0x1820, 0x1842},
			{0x1843},
			{0x1844, 0x1877},
			{0x1880, 0x18A8},
			{0x18A9},
			{0x1900, 0x191C},
			{0x1920, 0x1922},
			{0x1923, 0x1926},
			{0x1927, 0x1928},
			{0x1929, 0x192B},
			{0x1930, 0x1931},
			{0x1932},
			{0x1933, 0x1938},
			{0x1939, 0x193B},
			{0x1946, 0x194F},
			{0x1950, 0x196D},
			{0x1970, 0x1974},
			{0x1980, 0x19A9},
			{0x19B0, 0x19C0},
			{0x19C1, 0x19C7},
			{0x19C8, 0x19C9},
			{0x19D0, 0x19D9},
			{0x1A00, 0x1A16},
			{0x1A17, 0x1A18},
			{0x1A19, 0x1A1B},
			{0x1D00, 0x1D2B},
			{0x1D2C, 0x1D61},
			{0x1D62, 0x1D77},
			{0x1D78},
			{0x1D79, 0x1D9A},
			{0x1D9B, 0x1DBF},
			{0x1DC0, 0x1DC3},
			{0x1E00, 0x1E9B},
			{0x1EA0, 0x1EF9},
			{0x1F00, 0x1F15},
			{0x1F18, 0x1F1D},
			{0x1F20, 0x1F45},
			{0x1F48, 0x1F4D},
			{0x1F50, 0x1F57},
			{0x1F59},
			{0x1F5B},
			{0x1F5D},
			{0x1F5F, 0x1F7D},
			{0x1F80, 0x1FB4},
			{0x1FB6, 0x1FBC},
			{0x1FBE},
			{0x1FC2, 0x1FC4},
			{0x1FC6, 0x1FCC},
			{0x1FD0, 0x1FD3},
			{0x1FD6, 0x1FDB},
			{0x1FE0, 0x1FEC},
			{0x1FF2, 0x1FF4},
			{0x1FF6, 0x1FFC},
			{0x203F, 0x2040},
			{0x2054},
			{0x2071},
			{0x207F},
			{0x2090, 0x2094},
			{0x20D0, 0x20DC},
			{0x20E1},
			{0x20E5, 0x20EB},
			{0x2102},
			{0x2107},
			{0x210A, 0x2113},
			{0x2115},
			{0x2118},
			{0x2119, 0x211D},
			{0x2124},
			{0x2126},
			{0x2128},
			{0x212A, 0x212D},
			{0x212E},
			{0x212F, 0x2131},
			{0x2133, 0x2134},
			{0x2135, 0x2138},
			{0x2139},
			{0x213C, 0x213F},
			{0x2145, 0x2149},
			{0x2160, 0x2183},
			{0x2C00, 0x2C2E},
			{0x2C30, 0x2C5E},
			{0x2C80, 0x2CE4},
			{0x2D00, 0x2D25},
			{0x2D30, 0x2D65},
			{0x2D6F},
			{0x2D80, 0x2D96},
			{0x2DA0, 0x2DA6},
			{0x2DA8, 0x2DAE},
			{0x2DB0, 0x2DB6},
			{0x2DB8, 0x2DBE},
			{0x2DC0, 0x2DC6},
			{0x2DC8, 0x2DCE},
			{0x2DD0, 0x2DD6},
			{0x2DD8, 0x2DDE},
			{0x3005},
			{0x3006},
			{0x3007},
			{0x3021, 0x3029},
			{0x302A, 0x302F},
			{0x3031, 0x3035},
			{0x3038, 0x303A},
			{0x303B},
			{0x303C},
			{0x3041, 0x3096},
			{0x3099, 0x309A},
			{0x309D, 0x309E},
			{0x309F},
			{0x30A1, 0x30FA},
			{0x30FC, 0x30FE},
			{0x30FF},
			{0x3105, 0x312C},
			{0x3131, 0x318E},
			{0x31A0, 0x31B7},
			{0x31F0, 0x31FF},
			{0x3400, 0x4DB5},
			{0x4E00, 0x9FBB},
			{0xA000, 0xA014},
			{0xA015},
			{0xA016, 0xA48C},
			{0xA800, 0xA801},
			{0xA802},
			{0xA803, 0xA805},
			{0xA806},
			{0xA807, 0xA80A},
			{0xA80B},
			{0xA80C, 0xA822},
			{0xA823, 0xA824},
			{0xA825, 0xA826},
			{0xA827},
			{0xAC00, 0xD7A3},
			{0xF900, 0xFA2D},
			{0xFA30, 0xFA6A},
			{0xFA70, 0xFAD9},
			{0xFB00, 0xFB06},
			{0xFB13, 0xFB17},
			{0xFB1D},
			{0xFB1E},
			{0xFB1F, 0xFB28},
			{0xFB2A, 0xFB36},
			{0xFB38, 0xFB3C},
			{0xFB3E},
			{0xFB40, 0xFB41},
			{0xFB43, 0xFB44},
			{0xFB46, 0xFBB1},
			{0xFBD3, 0xFC5D},
			{0xFC64, 0xFD3D},
			{0xFD50, 0xFD8F},
			{0xFD92, 0xFDC7},
			{0xFDF0, 0xFDF9},
			{0xFE00, 0xFE0F},
			{0xFE20, 0xFE23},
			{0xFE33, 0xFE34},
			{0xFE4D, 0xFE4F},
			{0xFE71},
			{0xFE73},
			{0xFE77},
			{0xFE79},
			{0xFE7B},
			{0xFE7D},
			{0xFE7F, 0xFEFC},
			{0xFF10, 0xFF19},
			{0xFF21, 0xFF3A},
			{0xFF3F},
			{0xFF41, 0xFF5A},
			{0xFF66, 0xFF6F},
			{0xFF70},
			{0xFF71, 0xFF9D},
			{0xFF9E, 0xFF9F},
			{0xFFA0, 0xFFBE},
			{0xFFC2, 0xFFC7},
			{0xFFCA, 0xFFCF},
			{0xFFD2, 0xFFD7},
			{0xFFDA, 0xFFDC},
			{0x10000, 0x1000B},
			{0x1000D, 0x10026},
			{0x10028, 0x1003A},
			{0x1003C, 0x1003D},
			{0x1003F, 0x1004D},
			{0x10050, 0x1005D},
			{0x10080, 0x100FA},
			{0x10140, 0x10174},
			{0x10300, 0x1031E},
			{0x10330, 0x10349},
			{0x1034A},
			{0x10380, 0x1039D},
			{0x103A0, 0x103C3},
			{0x103C8, 0x103CF},
			{0x103D1, 0x103D5},
			{0x10400, 0x1044F},
			{0x10450, 0x1049D},
			{0x104A0, 0x104A9},
			{0x10800, 0x10805},
			{0x10808},
			{0x1080A, 0x10835},
			{0x10837, 0x10838},
			{0x1083C},
			{0x1083F},
			{0x10A00},
			{0x10A01, 0x10A03},
			{0x10A05, 0x10A06},
			{0x10A0C, 0x10A0F},
			{0x10A10, 0x10A13},
			{0x10A15, 0x10A17},
			{0x10A19, 0x10A33},
			{0x10A38, 0x10A3A},
			{0x10A3F},
			{0x1D165, 0x1D166},
			{0x1D167, 0x1D169},
			{0x1D16D, 0x1D172},
			{0x1D17B, 0x1D182},
			{0x1D185, 0x1D18B},
			{0x1D1AA, 0x1D1AD},
			{0x1D242, 0x1D244},
			{0x1D400, 0x1D454},
			{0x1D456, 0x1D49C},
			{0x1D49E, 0x1D49F},
			{0x1D4A2},
			{0x1D4A5, 0x1D4A6},
			{0x1D4A9, 0x1D4AC},
			{0x1D4AE, 0x1D4B9},
			{0x1D4BB},
			{0x1D4BD, 0x1D4C3},
			{0x1D4C5, 0x1D505},
			{0x1D507, 0x1D50A},
			{0x1D50D, 0x1D514},
			{0x1D516, 0x1D51C},
			{0x1D51E, 0x1D539},
			{0x1D53B, 0x1D53E},
			{0x1D540, 0x1D544},
			{0x1D546},
			{0x1D54A, 0x1D550},
			{0x1D552, 0x1D6A5},
			{0x1D6A8, 0x1D6C0},
			{0x1D6C2, 0x1D6DA},
			{0x1D6DC, 0x1D6FA},
			{0x1D6FC, 0x1D714},
			{0x1D716, 0x1D734},
			{0x1D736, 0x1D74E},
			{0x1D750, 0x1D76E},
			{0x1D770, 0x1D788},
			{0x1D78A, 0x1D7A8},
			{0x1D7AA, 0x1D7C2},
			{0x1D7C4, 0x1D7C9},
			{0x1D7CE, 0x1D7FF},
			{0x20000, 0x2A6D6},
			{0x2F800, 0x2FA1D},
			{0xE0100, 0xE01EF}
		};
		const CharSet Full_Width{
			{0x1100, 0x115F},
			{0x231A, 0x231B},
			{0x2329},
			{0x232A},
			{0x23E9, 0x23EC},
			{0x23F0},
			{0x23F3},
			{0x25FD, 0x25FE},
			{0x2614, 0x2615},
			{0x2648, 0x2653},
			{0x267F},
			{0x2693},
			{0x26A1},
			{0x26AA, 0x26AB},
			{0x26BD, 0x26BE},
			{0x26C4, 0x26C5},
			{0x26CE},
			{0x26D4},
			{0x26EA},
			{0x26F2, 0x26F3},
			{0x26F5},
			{0x26FA},
			{0x26FD},
			{0x2705},
			{0x270A, 0x270B},
			{0x2728},
			{0x274C},
			{0x274E},
			{0x2753, 0x2755},
			{0x2757},
			{0x2795, 0x2797},
			{0x27B0},
			{0x27BF},
			{0x2B1B, 0x2B1C},
			{0x2B50},
			{0x2B55},
			{0x2E80, 0x2E99},
			{0x2E9B, 0x2EF3},
			{0x2F00, 0x2FD5},
			{0x2FF0, 0x2FFB},
			{0x3001, 0x3003},
			{0x3004},
			{0x3005},
			{0x3006},
			{0x3007},
			{0x3008},
			{0x3009},
			{0x300A},
			{0x300B},
			{0x300C},
			{0x300D},
			{0x300E},
			{0x300F},
			{0x3010},
			{0x3011},
			{0x3012, 0x3013},
			{0x3014},
			{0x3015},
			{0x3016},
			{0x3017},
			{0x3018},
			{0x3019},
			{0x301A},
			{0x301B},
			{0x301C},
			{0x301D},
			{0x301E, 0x301F},
			{0x3020},
			{0x3021, 0x3029},
			{0x302A, 0x302D},
			{0x302E, 0x302F},
			{0x3030},
			{0x3031, 0x3035},
			{0x3036, 0x3037},
			{0x3038, 0x303A},
			{0x303B},
			{0x303C},
			{0x303D},
			{0x303E},
			{0x3041, 0x3096},
			{0x3099, 0x309A},
			{0x309B, 0x309C},
			{0x309D, 0x309E},
			{0x309F},
			{0x30A0},
			{0x30A1, 0x30FA},
			{0x30FB},
			{0x30FC, 0x30FE},
			{0x30FF},
			{0x3105, 0x312F},
			{0x3131, 0x318E},
			{0x3190, 0x3191},
			{0x3192, 0x3195},
			{0x3196, 0x319F},
			{0x31A0, 0x31BF},
			{0x31C0, 0x31E3},
			{0x31F0, 0x31FF},
			{0x3200, 0x321E},
			{0x3220, 0x3229},
			{0x322A, 0x3247},
			{0x3250},
			{0x3251, 0x325F},
			{0x3260, 0x327F},
			{0x3280, 0x3289},
			{0x328A, 0x32B0},
			{0x32B1, 0x32BF},
			{0x32C0, 0x32FF},
			{0x3300, 0x33FF},
			{0x3400, 0x4DBF},
			{0x4E00, 0x9FFC},
			{0x9FFD, 0x9FFF},
			{0xA000, 0xA014},
			{0xA015},
			{0xA016, 0xA48C},
			{0xA490, 0xA4C6},
			{0xA960, 0xA97C},
			{0xAC00, 0xD7A3},
			{0xF900, 0xFA6D},
			{0xFA6E, 0xFA6F},
			{0xFA70, 0xFAD9},
			{0xFADA, 0xFAFF},
			{0xFE10, 0xFE16},
			{0xFE17},
			{0xFE18},
			{0xFE19},
			{0xFE30},
			{0xFE31, 0xFE32},
			{0xFE33, 0xFE34},
			{0xFE35},
			{0xFE36},
			{0xFE37},
			{0xFE38},
			{0xFE39},
			{0xFE3A},
			{0xFE3B},
			{0xFE3C},
			{0xFE3D},
			{0xFE3E},
			{0xFE3F},
			{0xFE40},
			{0xFE41},
			{0xFE42},
			{0xFE43},
			{0xFE44},
			{0xFE45, 0xFE46},
			{0xFE47},
			{0xFE48},
			{0xFE49, 0xFE4C},
			{0xFE4D, 0xFE4F},
			{0xFE50, 0xFE52},
			{0xFE54, 0xFE57},
			{0xFE58},
			{0xFE59},
			{0xFE5A},
			{0xFE5B},
			{0xFE5C},
			{0xFE5D},
			{0xFE5E},
			{0xFE5F, 0xFE61},
			{0xFE62},
			{0xFE63},
			{0xFE64, 0xFE66},
			{0xFE68},
			{0xFE69},
			{0xFE6A, 0xFE6B},
			{0x16FE0, 0x16FE1},
			{0x16FE2},
			{0x16FE3},
			{0x16FE4},
			{0x16FF0, 0x16FF1},
			{0x17000, 0x187F7},
			{0x18800, 0x18AFF},
			{0x18B00, 0x18CD5},
			{0x18D00, 0x18D08},
			{0x1B000, 0x1B0FF},
			{0x1B100, 0x1B11E},
			{0x1B150, 0x1B152},
			{0x1B164, 0x1B167},
			{0x1B170, 0x1B2FB},
			{0x1F004},
			{0x1F0CF},
			{0x1F18E},
			{0x1F191, 0x1F19A},
			{0x1F200, 0x1F202},
			{0x1F210, 0x1F23B},
			{0x1F240, 0x1F248},
			{0x1F250, 0x1F251},
			{0x1F260, 0x1F265},
			{0x1F300, 0x1F320},
			{0x1F32D, 0x1F335},
			{0x1F337, 0x1F37C},
			{0x1F37E, 0x1F393},
			{0x1F3A0, 0x1F3CA},
			{0x1F3CF, 0x1F3D3},
			{0x1F3E0, 0x1F3F0},
			{0x1F3F4},
			{0x1F3F8, 0x1F3FA},
			{0x1F3FB, 0x1F3FF},
			{0x1F400, 0x1F43E},
			{0x1F440},
			{0x1F442, 0x1F4FC},
			{0x1F4FF, 0x1F53D},
			{0x1F54B, 0x1F54E},
			{0x1F550, 0x1F567},
			{0x1F57A},
			{0x1F595, 0x1F596},
			{0x1F5A4},
			{0x1F5FB, 0x1F5FF},
			{0x1F600, 0x1F64F},
			{0x1F680, 0x1F6C5},
			{0x1F6CC},
			{0x1F6D0, 0x1F6D2},
			{0x1F6D5, 0x1F6D7},
			{0x1F6EB, 0x1F6EC},
			{0x1F6F4, 0x1F6FC},
			{0x1F7E0, 0x1F7EB},
			{0x1F90C, 0x1F93A},
			{0x1F93C, 0x1F945},
			{0x1F947, 0x1F978},
			{0x1F97A, 0x1F9CB},
			{0x1F9CD, 0x1F9FF},
			{0x1FA70, 0x1FA74},
			{0x1FA78, 0x1FA7A},
			{0x1FA80, 0x1FA86},
			{0x1FA90, 0x1FAA8},
			{0x1FAB0, 0x1FAB6},
			{0x1FAC0, 0x1FAC2},
			{0x1FAD0, 0x1FAD6},
			{0x20000, 0x2A6DD},
			{0x2A6DE, 0x2A6FF},
			{0x2A700, 0x2B734},
			{0x2B735, 0x2B73F},
			{0x2B740, 0x2B81D},
			{0x2B81E, 0x2B81F},
			{0x2B820, 0x2CEA1},
			{0x2CEA2, 0x2CEAF},
			{0x2CEB0, 0x2EBE0},
			{0x2EBE1, 0x2F7FF},
			{0x2F800, 0x2FA1D},
			{0x2FA1E, 0x2FA1F},
			{0x2FA20, 0x2FFFD},
			{0x30000, 0x3134A},
			{0x3134B, 0x3FFFD}
		};
		const CharSet Control_Char{
			{0, 0x1f},
			{0x7f},
			{0x80, 0x9f}
		};
	}
}