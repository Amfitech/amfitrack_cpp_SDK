//-----------------------------------------------------------------------------
//
//                              AMFITECH APS
//
//                          ALL RIGHTS RESERVED
//
//-----------------------------------------------------------------------------

#pragma once

#ifdef __cplusplus

//-----------------------------------------------------------------------------
// Section: Includes
//-----------------------------------------------------------------------------
#include <array>
#include <cstddef>

//-----------------------------------------------------------------------------
// Section: Define
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Section: Typedef
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Section: Macro
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Section: Variables
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Section: Class
//-----------------------------------------------------------------------------

template <typename T, std::size_t Capacity>
class lib_fifo
{
  public:
	static_assert(Capacity > 0, "FIFO capacity must be greater than 0");

	bool put(const T &item);
	bool pop(T &item);
	bool peek(T &item) const;
	T *peek();
	const T *peek() const;

	void clear();

	bool isFull() const;
	bool isEmpty() const;

	std::size_t size() const;
	constexpr std::size_t capacity() const;
	std::size_t remaining() const;

  private:
	std::size_t increment(std::size_t index) const;

	std::array<T, Capacity> buffer{};

	std::size_t head{ 0 };
	std::size_t tail{ 0 };
	std::size_t count{ 0 };
};

//-----------------------------------------------------------------------------
// Section: Template implementation
//-----------------------------------------------------------------------------

template <typename T, std::size_t Capacity>
bool lib_fifo<T, Capacity>::put(const T &item)
{
	if (isFull())
	{
		return false;
	}

	buffer[head] = item;
	head = increment(head);
	count++;

	return true;
}

template <typename T, std::size_t Capacity>
bool lib_fifo<T, Capacity>::pop(T &item)
{
	if (isEmpty())
	{
		return false;
	}

	item = buffer[tail];
	tail = increment(tail);
	count--;

	return true;
}

template <typename T, std::size_t Capacity>
bool lib_fifo<T, Capacity>::peek(T &item) const
{
	if (isEmpty())
	{
		return false;
	}

	item = buffer[tail];

	return true;
}

template <typename T, std::size_t Capacity>
T *lib_fifo<T, Capacity>::peek()
{
	if (isEmpty())
	{
		return nullptr;
	}

	return &buffer[tail];
}

template <typename T, std::size_t Capacity>
const T *lib_fifo<T, Capacity>::peek() const
{
	if (isEmpty())
	{
		return nullptr;
	}

	return &buffer[tail];
}

template <typename T, std::size_t Capacity>
void lib_fifo<T, Capacity>::clear()
{
	head = 0;
	tail = 0;
	count = 0;
}

template <typename T, std::size_t Capacity>
bool lib_fifo<T, Capacity>::isFull() const
{
	return count >= Capacity;
}

template <typename T, std::size_t Capacity>
bool lib_fifo<T, Capacity>::isEmpty() const
{
	return count == 0;
}

template <typename T, std::size_t Capacity>
std::size_t lib_fifo<T, Capacity>::size() const
{
	return count;
}

template <typename T, std::size_t Capacity>
constexpr std::size_t lib_fifo<T, Capacity>::capacity() const
{
	return Capacity;
}

template <typename T, std::size_t Capacity>
std::size_t lib_fifo<T, Capacity>::remaining() const
{
	return Capacity - count;
}

template <typename T, std::size_t Capacity>
std::size_t lib_fifo<T, Capacity>::increment(std::size_t index) const
{
	index++;

	if (index >= Capacity)
	{
		index = 0;
	}

	return index;
}

#endif

/** @} */ // end of module