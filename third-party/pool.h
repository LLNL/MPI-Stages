/*
MIT License

Copyright (c) 2017, Roger Ferrer Ibanez

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

// https://thinkingeek.com/2017/11/19/simple-memory-pool/
// https://gist.github.com/rofirrim/eed6b9ef43d362c186534cac5f46baf7

/*
 * Very simple memory pool.
 *
 * This memory pool allocates arenas where each arena holds up to arena_size
 * elements. All the arenas of the pool are singly-linked.
 *
 * Each element in an arena has a pointer to the next free item, except the last
 * one which is set to null. This forms another singly-linked list of available
 * storage called free_list. The free_list points to the first free storage
 * item, or null if the current arena is full.
 *
 * When an object is allocated, if the arena is not full, the storage pointed
 * by free_list is returned. Before returning, free_list is updated to point
 * to the next free item. If the arena is full, free_list is null so an arena is
 * created first which will point to the currently (full) arena. Then the
 * free_list
 * is updated to point the new storage of the arena.
 *
 * When a pointer is freed, we recover the element of the arena (using pointer
 * arithmetic) and then we put this item at the beginning of the free list.
 *
 */

#ifndef __EXAMPI_MEMORY_POOL_H
#define __EXAMPI_MEMORY_POOL_H

#include <cassert>
#include <cstddef>
#include <memory>
#include <new>
#include <utility>
#include <mutex>

#include "debug.h"


namespace exampi
{

template <typename T> class MemoryPool
{
private:
	union minipool_item
	{
private:
		using StorageType = alignas(alignof(T)) char[sizeof(T)];

		minipool_item *next;
		StorageType datum;

public:
		minipool_item *get_next_item() const
		{
			return next;
		}
		void set_next_item(minipool_item *n)
		{
			next = n;
		}

		T *get_storage()
		{
			return reinterpret_cast<T *>(datum);
		}

		static minipool_item *storage_to_item(T *t)
		{
			minipool_item *current_item = reinterpret_cast<minipool_item *>(t);

			return current_item;
		}
	};

	struct minipool_arena
	{
	private:
		std::unique_ptr<minipool_item[]> storage;
		std::unique_ptr<minipool_arena> next;

	public:
		minipool_arena(size_t arena_size) : storage(new minipool_item[arena_size])
		{
			for (size_t i = 1; i < arena_size; i++)
			{
				storage[i - 1].set_next_item(&storage[i]);
				debug("item " << i-1 << ": init " << &storage[i-1] << " -> " << &storage[i]);
			}

			storage[arena_size - 1].set_next_item(nullptr);
			debug("item " << arena_size-1 << ": init " << &storage[arena_size-1] <<
			      " -> 0x0");
		}

		minipool_item *get_storage() const
		{
			return storage.get();
		}

		void set_next_arena(std::unique_ptr<minipool_arena> &&n)
		{
			assert(!next);

			next.reset(n.release());
		}
	};

	size_t arena_size;
	std::unique_ptr<minipool_arena> arena;
	minipool_item *free_list;

	std::mutex guard;

public:
	MemoryPool(size_t arena_size)
		: arena_size(arena_size), arena(new minipool_arena(arena_size)),
		  free_list(arena->get_storage())
	{
		debug("free_list " << free_list);
		debug("this " << this);
	}

	template <typename... Args> T *allocate(Args &&... args)
	//T *allocate()
	{
		std::lock_guard<std::mutex> lock(guard);

		if(nullptr == free_list)
		{
			debug("allocating additional arena");
			std::unique_ptr<minipool_arena> new_arena(new minipool_arena(arena_size));
			new_arena->set_next_arena(std::move(arena));

			arena.reset(new_arena.release());
			free_list = arena->get_storage();
		}

		debug("free_list " << free_list << " -> next -> " <<
		      free_list->get_next_item());
		minipool_item *current_item = free_list;
		free_list = current_item->get_next_item();

		T *result = current_item->get_storage();

		debug("constructing new object in allocated space");
		new (result) T(std::forward<Args>(args)...);
		//new (result) T();

		return result;
	}

	void deallocate(T *t)
	{
		if(nullptr == t)
		{
			debug("ignoring nullptr deallocation");
			return;
		}

		std::lock_guard<std::mutex> lock(guard);

		debug("deconstructing object at " << t);
		t->T::~T();

		minipool_item *current_item = minipool_item::storage_to_item(t);

		debug("returning " << t << " to the pool, current free_list " << free_list);
		current_item->set_next_item(free_list);
		free_list = current_item;
		debug("free_list " << free_list << " -> next -> " <<
		      free_list->get_next_item());
	}
};

}

#endif // MINIPOOL_H
