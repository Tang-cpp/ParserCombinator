# Index
- [Index](#index)
- [Introduce](#introduce)
- [API](#api)
  - [\<parser/Parser.h\>](#parserparserh)
    - [`GrammarBase<ReturnType>`](#grammarbasereturntype)
    - [`Rule<ReturnType>`](#rulereturntype)
    - [`U32Itor`](#u32itor)
    - [`ReturnType Parse(U32Itor& begin, U32Itor const& end, GrammarBase<ReturnType>& grammar)`](#returntype-parseu32itor-begin-u32itor-const-end-grammarbasereturntype-grammar)
    - [`CharSet`](#charset)
    - [`char_(...)`](#char_)
    - [`set_(...)`](#set_)
    - [`tk_(...)`](#tk_)
    - [Sequence](#sequence)
    - [Alternative](#alternative)
    - [Repeat](#repeat)
    - [Action](#action)
    - [Skipper](#skipper)
  - [\<utils/Variant.h\>](#utilsvarianth)
  - [\<utils/Optional.h\>](#utilsoptionalh)
    - [`bool Optional<Type>::IsNull() const`](#bool-optionaltypeisnull-const)
    - [`Type& Optional<Type>::Get() const`](#type-optionaltypeget-const)
    - [`null`](#null)
# Introduce
A C++11 standard based parser combinator.
# API
All components are in the **`em::parser`** namespace.
## \<parser/Parser.h\>
### `GrammarBase<ReturnType>`
> Any grammar should inherit from this base class. `ReturnType` is the type of the parsing result.
```c++
struct Grammar : GrammarBase<int64_t>
{
public:
    // The constructor should be defined like this.
    Grammar() : Grammar::base_type(this->start)
	{
        // this->start is a Rule<ReturnType> inherit
        // from GrammarBase<ReturnType>.
        // The parsing will start from this rule.
        this->start = /* A rule that returns ReturnType*/;
    }
}
```
### `Rule<ReturnType>`
> Could be defined as a member of a grammar class. Useful when a rule need to be referenced before its definition.
```c++
struct Grammar : GrammarBase<int64_t>
{
private:
    Rule<int64_t> ruleA;
public:
    // The constructor should be defined like this.
    Grammar() : Grammar::base_type(this->start)
	{
        // ruleA referenced by ruleB before its assignment.
        auto ruleB = ruleA > "!";

        ruleA = char_("0") ^ [](){ return 0; };
        this->start = ruleB;
    }
}
```
### `U32Itor`
> Should be initialized with a `std::string::iterator`. It converts a UTF-8 string to a UTF-32 stream, calculates and stores the location information.
### `ReturnType Parse(U32Itor& begin, U32Itor const& end, GrammarBase<ReturnType>& grammar)` 
> Parse a input stream starts at `begin` and ends at `end` with grammar `grammar` and returns a [`Optinal<ReturnType>`](#utilsoptionalh).
### `CharSet`
> Stores a set of codepoint `CharRange`.
* `bool CharSet::Has(CharRange const& r) const`
> Returns `ture` if a `CharSet` has the range of `r`.
```c++
// Can be initialized using a regex-like string.
// But only support '-' and '^'.
CharSet bin_digit = "01";
CharSet hex_digit = "0-9a-fA-F";
CharSet not_space = "^ ";
// Can be initialized using a char32_t string
// or wchar_t string.
CharSet kanji = U"漢字";
// Can be initialized using a initializer like this.
CharSet range = {
    {0x0041, 0x005A}, {0x005F}, {0x0061, 0x007A}
};
```
### `char_(...)`
> Returns a rule that consume and skip a character in a specific charset(return type is void).
```c++
// Skip a character.
auto ruleA = char_();
// Skip a 'x' or a 'X'.
auto ruleB = char_("xX");
// Skip a character in the range of 'a' to 'f'.
auto ruleC = char_("a-f");
// Skip a character that is not a space.
auto ruleD = char_("^ ");
```
### `set_(...)`
> Same to [`CharSet`](#charset), but directly defined in a rule definition. Consume and return a `char32_t` character as result.
### `tk_(...)`
> Consume and skip a string.
```c++
// Match a string of "for".
auto ruleA = tk_("for");
// Can be directly use without tk_(), but because we can't
// overload a operator of basic types, sometimes we still
// need tk_().
auto ruleB = "for" > integer > "," > integer > "," > integer;
```
### Sequence
> Use `>` to connect rules into a sequence. The output of a sequence is a tuple of its components.
```c++
// The output of ruleA is a std::tuple<char32_t,char32_t,char32_t>.
auto ruleA = char_("a-f") > char_("a-f") > char_("a-f");
// The output of ruleB is a std::tuple<char32_t,char32_t>.
auto ruleB = char_("a-f") > "," > char_("a-f");
```
### Alternative
> Use `|` to separate alternative rules. All rules should have same output.
```c++
// The output of ruleA is a char32_t.
auto ruleA = char_("a-f") | char_("a-f") | char_("a-f");
```
### Repeat
> Use operator `*` to make a rule to match zero or more times. Use prefix operator `-` to make a rule to match zero or one time. Use prefix operator `+` to make a rule to match one or more times. The output of a repeated rule is a `std::vector<ReturnType>`.
```c++
// The outputs of ruleA, ruleB and ruleC are are
// std::vector<char32_t>.
auto ruleA = "0x" > *set_("0-9a-fA-F"); // 0x, 0xA, 0xFF, ...
auto ruleB = "0x" > -set_("0-9a-fA-F"); // 0x, 0xA, 0xF, ...
auto ruleC = "0x" > +set_("0-9a-fA-F"); // 0xA, 0xFF, ...
```
### Action
> Use `^` or `>>=` or `[]` to attach a static function or a lambda function as a action.
```c++
float C32ToF(char32_t const& c)
{
    return (float)c;
}
// The outputs of ruleA, ruleB and ruleC are float.
auto ruleA = char_("a-f") ^ [](char32_t const& c){ return (float)c; }
auto ruleB = char_("a-f")[C32ToF]
auto ruleC = char_("a-f") >>= C32ToF
```
### Skipper
> Use `>>` to set a rule as a skipper. Use `NoSkip` to clear current skipper.
``` c++
// In this example, "  0xabc  " matches the rulaB and the
// output should be {'a', 'b', 'c'} because space will be skipped.
// But "  0x a b c  " won't match the ruleB because, the 
// skipper has been temporarily cleared when matching
// '+char_("a-f")'.
auto skipper = set_(" ");
// The skipper won't affect '+char_("a-f")' part. 
auto ruleA = "0x" > NoSkip >> +char_("a-f");
// Any input that matches rule 'skipper' will be skipped.
auto ruleB = skipper >> ruleA;
```
## \<utils/Variant.h\>
> `Variant<Type1[,Type2, Type3, ...]>`, similar to [`std::variant`](https://en.cppreference.com/w/cpp/utility/variant).
## \<utils/Optional.h\>
> `Optional<Type>`, similar to [`std::optional`](https://en.cppreference.com/w/cpp/utility/optional). 
### `bool Optional<Type>::IsNull() const`
> Returns `true` if the object contains a value.
### `Type& Optional<Type>::Get() const`
> Returns the containing value.
### `null`
> Can be assigned to an Optional object and clear its containing state.