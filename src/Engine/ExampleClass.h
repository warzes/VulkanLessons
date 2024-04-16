#pragma once

// Example operator over
struct Foo
{
	constexpr Foo() = default;
	constexpr Foo(Foo&&) = default;
	constexpr Foo(const Foo&) = default;
	constexpr Foo(int scalar) : a(scalar) {}

	constexpr Foo& operator=(Foo&&) = default;
	constexpr Foo& operator=(const Foo&) = default;

	constexpr Foo operator+(int f) const { return { a + f }; }
	constexpr Foo operator+(const Foo& v) const { return { a + v.a }; }
	constexpr Foo& operator+=(const Foo& v) { a += v.a; return(*this); }

	constexpr int& operator[](size_t i) { return (&a)[i]; }
	constexpr const int operator[](size_t i) const { return (&a)[i]; }

	int a = 0;
};
inline bool operator==(const Foo& Left, const Foo& Right) noexcept { return Left.a == Right.a; }
inline Foo operator-(const Foo& In) noexcept { return { -In.a }; }
inline Foo operator-(int Left, const Foo& Right) noexcept { return { Left - Right.a }; }
inline Foo operator-(const Foo& Left, int Right) noexcept { return { Left.a - Right }; }
inline Foo operator-(const Foo& Left, const Foo& Right) noexcept { return { Left.a - Right.a }; }
inline Foo& operator-=(Foo& Left, int Right) noexcept { return Left = Left - Right; }
inline Foo& operator-=(Foo& Left, const Foo& Right) noexcept { return Left = Left - Right; }