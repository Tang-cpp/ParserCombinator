#pragma once
#include <utility>
#include <type_traits>
namespace em
{
	struct NullType {};
	extern NullType null;

	template <typename T, typename = typename std::enable_if<std::is_default_constructible<T>::value>::type>
	struct Optional
	{
		Optional() : p(std::make_pair(T(), true)) {}
		Optional(T const& v) : p(std::make_pair(v, false)) {}
		Optional(NullType& v) : Optional() {}

		Optional(Optional const& other) : p(other.p) {}
		Optional(Optional&& other) noexcept : p(std::move(other.p)) {}

		Optional& operator=(NullType& v) { p = std::make_pair(T(), true); return *this; }
		Optional& operator=(T const& v) { p = std::make_pair(v, false); return *this; }
		Optional& operator=(Optional const& other) { p = other.p; return *this; }
		Optional& operator=(Optional&& other) noexcept { p = std::move(other.p); return *this; }

		bool IsNull() const { return p.second; }
		T& Get() { return p.first; }
		T const& Get() const { return p.first; }
	private:
		std::pair<T, bool> p;
	};
}