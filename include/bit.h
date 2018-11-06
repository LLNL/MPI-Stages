#ifndef __EXAMPI_BITS_H
#define __EXAMPI_BITS_H

#include <utility>
#include <type_traits>
#include <unistd.h>
#include <stdint.h>

// bits.h
// helper classes for bitfields
// shane farmer auburn university
namespace exampi
{
namespace Bit
{


// SF:  This uses templates and typenames to assign a data type
// based on bit width (provided in template) at compile-time.
// see std::conditional
template<size_t sz>
using type =
    typename std::conditional<sz==0, void,
    typename std::conditional<sz<=8, uint8_t,
    typename std::conditional<sz<=16, uint16_t,
    typename std::conditional<sz<=32, uint32_t,
    typename std::conditional<sz<=64, uint64_t,
    void>::type>::type>::type>::type>::type;

// Note const member access and constexpr -- this is 0 overhead and
// evaluates fully at compile time
// Class over functions for type safety at compile-time
// Use like a templated function, i.e. int a = Mask<4>();
// Storing the created object isn't necessary (and could be harmful)
template<size_t sz>
class Mask
{
private:
	const type<sz> data;
public:
	constexpr Mask() : data((1u << sz) - 1u) {;}
	constexpr Mask(type<sz> t) : data(t & ((1u << sz) - 1u)) {;}
	constexpr operator type<sz>() const
	{
		return data;
	}
};

template<size_t sz>
class At
{
private:
	const type<sz> data;
public:
	constexpr At() : data((1u << sz)) {;}
	constexpr At(type<sz> t) : data(t & (1u << sz)) {;}
	constexpr operator type<sz>() const
	{
		return data;
	}
};

// This is distict from a "field" (below) in that it is fully evaluated;
// a "field" is a semantic for setting/retrieving later
template<size_t start, size_t width>
class Slice
{
private:
	const type<width> data;
public:
	constexpr Slice(const type<start+width> target) : data(target & (Mask<width+start>() ^ Mask<start>())) {;}
	constexpr operator type<width>() const
	{
		return data;
	}
};


// end constexpr


template<size_t O, size_t W>   // Offset, Width
class Field
{
private:
	type<O+W> &data;
	const type<O+W> mask;
public:
	Field(type<O+W> &i) : data(i), mask(Mask<O+W>(W) << O) {;}
	Field(Field &i)
	{
		data = i.data;
	}
	Field(Field &&i)
	{
		std::move(i.data);
	}

	type<O+W> get() const
	{
		return data & mask;
	}
	void set(type<O+W> v)
	{
		data &= !mask; // erase field
		data |= (v & mask);
	}

	type<O+W> operator=(type<O+W> t)
	{
		set(t);
		return data;
	}
	type<O+W> operator+(type<O+W> t)
	{
		set(get() + t);
		return data;
	}
	type<O+W> operator-(type<O+W> t)
	{
		set(get() - t);
		return data;
	}
	type<O+W> operator*(type<O+W> t)
	{
		set(get() * t);
		return data;
	}
	type<O+W> operator/(type<O+W> t)
	{
		set(get() / t);
		return data;
	}

	constexpr operator type<O+W>() const
	{
		return data;
	}
};

} // Bits
} // exampi
#endif
