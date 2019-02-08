#ifndef __EXAMPI_POOL_H
#define __EXAMPI_POOL_H

#include <mutex>
#include <memory>
#include <functional>
#include <assert.h>

#include "debug.h"

// https://thinkingeek.com/2017/11/19/simple-memory-pool/
// MR: this is a dirty copy of that, use that as base for good implementation

// todo use mutex only to reset free set
// free set is a collection of 1 link with an atomic to iterate

// todo make this smarter
// use a free list to point to non reset ones, avoid doing mutex if we are not low of items.

namespace exampi
{

template <typename T>
class MemoryPool
{
	union MemoryPool_item
	{
private:
		MemoryPool_item *next;
		alignas(alignof(T)) char datum[sizeof(T)];

public:
		MemoryPool_item *get_next_item() const
		{
			return next;
		}
		void set_next_item(MemoryPool_item *n)
		{
			next = n;
		}

		T *get_storage()
		{
			return reinterpret_cast<T *>(datum);
		}

		static MemoryPool_item *storage_to_item(T *t)
		{
			MemoryPool_item *current_item = reinterpret_cast<MemoryPool_item *>(t);
			return current_item;
		}
	};

	struct MemoryPool_arena
	{
	private:
		std::unique_ptr<MemoryPool_item[]> storage;
		std::unique_ptr<MemoryPool_arena> next;

	public:
		MemoryPool_arena(size_t arena_size) : storage(new MemoryPool_item[arena_size])
		{
			for (size_t i = 1; i < arena_size; i++)
			{
				storage[i - 1].set_next_item(&storage[i]);
			}
			storage[arena_size - 1].set_next_item(nullptr);
		}

		MemoryPool_item *get_storage() const
		{
			return storage.get();
		}

		void set_next_arena(std::unique_ptr<MemoryPool_arena> &&n)
		{
			assert(!next);

			next.reset(n.release());
		}
	};


	size_t arena_size;
	size_t allocated_items;
	size_t allocated_arenas;

	std::unique_ptr<MemoryPool_arena> arena;
	MemoryPool_item *free_list;

	std::mutex sharedlock;

public:
	// shortcut unique ptr type for simplicity
	typedef typename std::unique_ptr<T, std::function<void(T *)>> unique_ptr;

	MemoryPool(size_t arena_size)
		: arena_size(arena_size),
		  allocated_items(0),
		  allocated_arenas(1),
		  arena(new MemoryPool_arena(arena_size)),
		  free_list(arena->get_storage())
	{;}

	template <typename... Args>
	T* allocate(Args &&... args)
	{
		debug("allocating " << typeid(T).name() << " from " << this->allocated_arenas
		        << " arenas");

		std::lock_guard<std::mutex> lock(this->sharedlock);

		// allocate a new arena if needed
		if (free_list == nullptr)
		{
			debug("allocating additional arena");

			std::unique_ptr<MemoryPool_arena> new_arena(new MemoryPool_arena(arena_size));

			new_arena->set_next_arena(std::move(arena));
			arena.reset(new_arena.release());
			free_list = arena->get_storage();

			this->allocated_arenas++;
		}

		// fetch next empty
		MemoryPool_item *current_item = free_list;
		free_list = current_item->get_next_item();

		T *result = current_item->get_storage();

		// construct object in allocated space
		new (result) T(std::forward<Args>(args)...);

		this->allocated_items++;
		debug("item is number " << this->allocated_items);

		return result;
	}

	template <typename... Args>
	MemoryPool::unique_ptr allocate_unique(Args &&... args)
	{
		T* result = allocate(std::forward<Args>(args)...);

		return MemoryPool::unique_ptr(result, [this](T* t) -> void { this->deallocate(t); });
	}

	void deallocate(T *t)
	{
		debug("freeing item, now at " << this->allocated_items << " : " <<
		        this->allocated_arenas);

		std::lock_guard<std::mutex> lock(this->sharedlock);
		this->allocated_items--;

		// call deconstructor
		t->T::~T();

		MemoryPool_item *current_item = MemoryPool_item::storage_to_item(t);

		// return item to pool
		current_item->set_next_item(this->free_list);
		this->free_list = current_item;
	}
};

}

#endif
