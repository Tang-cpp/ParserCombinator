#pragma once
#include <iterator>

#include "parser/Core.h"
namespace em
{
	namespace parser
	{
		struct CharSet;
		extern const CharSet Control_Char, Full_Width;
		template <typename BaseIter>
		struct U8toU32Iterator : std::iterator<std::input_iterator_tag, CharT>
		{
			using value_type = CharT;
			using pointer = CharT*;
			using reference = const CharT&;

			template <typename U, typename = CheckedType<IsSame<U, BaseIter>>>
			U8toU32Iterator(U const& i)
				: _up(i), _next(i),
				_line(1), _col(1), _pos(0) {}
			U8toU32Iterator(U8toU32Iterator& i)
				: _up(i._up), _next(i._next), _buf(i._buf),
				_line(i._line), _col(i._col), _pos(i._pos) {}

			U8toU32Iterator& operator++()
			{
				SkipBOM();
				if (_next != _up) _up = _next;
				else _up = Parse();
				CalcPos();
				_next = _up;
				return *this;
			}
			U8toU32Iterator operator++(int)
			{
				U8toU32Iterator retval = *this;
				SkipBOM();
				if (_next != _up) _up = _next;
				else _up = Parse();
				CalcPos();
				_next = _up;
				return retval;
			}
			bool operator==(U8toU32Iterator const& o)
			{
				if (_up != o._up) SkipBOM();
				return _up == o._up;
			}
			bool operator!=(U8toU32Iterator const& o) { return !(*this == o); }
			reference operator*()
			{
				if (_next != _up) return _buf;
				_next = Parse();
				return _buf;
			}
			uint32_t Line() const { return _line; }
			uint32_t Col() const { return _col; }
			uint32_t Pos() const { return _pos; }
		private:
			inline void SkipBOM()
			{
				if (0xEF == (uint8_t)*_up) // BOM
				{
					if (0xBB == (uint8_t) * ++_up)
						if (0xBF == (uint8_t) * ++_up)
						{
							_pos += 3;
							return;
						}
					_ASSERT_EXPR(false, "Invalid utf-8 string.");
				}
			}
			inline BaseIter ParseAChar()
			{
				BaseIter ret = _up;
				bool failed = false;
				if (0x7F >= (uint8_t)*ret)							// 1 byte
				{
					_buf = (uint8_t)*ret++;
					_pos++;
				}
				else if (0xC0 <= (uint8_t)*ret && (uint8_t)*ret <= 0xDF)		// 2 byte
				{
					_buf = (0x1F & (uint8_t)*ret++) << 6;
					_buf |= 0x3F & (uint8_t)*ret++;
					_pos += 2;
				}
				else if (0xE0 <= (uint8_t)*ret && (uint8_t)*ret <= 0xEF)		// 3 byte
				{
					_buf = (0xF & (uint8_t)*ret++) << 12;
					_buf |= (0x3F & (uint8_t)*ret++) << 6;
					_buf |= 0x3F & (uint8_t)*ret++;
					_pos += 3;
				}
				else if (0xF0 <= (uint8_t)*ret && (uint8_t)*ret <= 0xF7)		// 4 byte
				{
					_buf = (0x7 & (uint8_t)*ret++) << 18;
					_buf |= (0x3F & (uint8_t)*ret++) << 12;
					_buf |= (0x3F & (uint8_t)*ret++) << 6;
					_buf |= 0x3F & (uint8_t)*ret++;
					_pos += 4;
				}
				else if (0xF8 <= (uint8_t)*ret && (uint8_t)*ret <= 0xFB)		// 5 byte
				{
					_buf = (0x3 & (uint8_t)*ret++) << 24;
					_buf |= (0x3F & (uint8_t)*ret++) << 18;
					_buf |= (0x3F & (uint8_t)*ret++) << 12;
					_buf |= (0x3F & (uint8_t)*ret++) << 6;
					_buf |= 0x3F & (uint8_t)*ret++;
					_pos += 5;
				}
				else if (0xFC == (uint8_t)*ret || 0xFD == (uint8_t)*ret)		// 6 byte
				{
					_buf = (0x1 & (uint8_t)*ret++) << 30;
					_buf |= (0x3F & (uint8_t)*ret++) << 24;
					_buf |= (0x3F & (uint8_t)*ret++) << 18;
					_buf |= (0x3F & (uint8_t)*ret++) << 12;
					_buf |= (0x3F & (uint8_t)*ret++) << 6;
					_buf |= 0x3F & (uint8_t)*ret++;
					_pos += 6;
				}
				else failed = true;
				_ASSERT_EXPR(!failed, "Invalid utf-8 string.");
				return ret;
			}
			inline BaseIter Parse()
			{
				BaseIter ret = ParseAChar();
				while (_buf == '\r') ret = ParseAChar();
				return ret;
			}
			inline void CalcPos()
			{
				if (_buf == '\t') _col += 4;
				else if (_buf == '\n') { _line++; _col = 1; }
				else if (!Control_Char.Has(_buf)) // control code
					_col += Full_Width.Has(_buf) ? 2 : 1;
			}
			BaseIter _up, _next;
			CharT _buf{};
			uint32_t _line, _col, _pos;
		};

		using U32Itor = U8toU32Iterator<std::string::iterator>;
	}
}