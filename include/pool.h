#ifndef __EXAMPI_POOL_H
#define __EXAMPI_POOL_H

#include <mutex>
#include <memory>
#include <assert.h>

// potentially do a SCSP kind of thing, we have sequential maybe
// https://thinkingeek.com/2017/11/19/simple-memory-pool/
// MR: this is a dirty copy of that, use that as base for good implementation

namespace exampi
{

template <typename T> struct minipool
{
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
			}
			storage[arena_size - 1].set_next_item(nullptr);
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
	std::mutex sharedlock;

	minipool(size_t arena_size)
		: arena_size(arena_size), arena(new minipool_arena(arena_size)),
		  free_list(arena->get_storage()) {}

	template <typename... Args> T *alloc(Args &&... args)
	{
		std::lock_guard<std::mutex> lock(sharedlock);

		if (free_list == nullptr)
		{
			std::unique_ptr<minipool_arena> new_arena(new minipool_arena(arena_size));

			new_arena->set_next_arena(std::move(arena));
			arena.reset(new_arena.release());
			free_list = arena->get_storage();
		}

		minipool_item *current_item = free_list;
		free_list = current_item->get_next_item();

		T *result = current_item->get_storage();
		new (result) T(std::forward<Args>(args)...);

		return result;
	}

	void free(T *t)
	{
		std::lock_guard<std::mutex> lock(sharedlock);

		t->T::~T();

		minipool_item *current_item = minipool_item::storage_to_item(t);

		current_item->set_next_item(free_list);
		free_list = current_item;
	}
};

}

#endif
