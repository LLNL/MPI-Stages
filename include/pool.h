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

template <typename T> 
class MemoryPool
{
	union MemoryPool_item
	{
	private:
		using StorageType = alignas(alignof(T)) char[sizeof(T)];

		MemoryPool_item *next;
		StorageType datum;
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

	std::unique_ptr<MemoryPool_arena> arena;
	MemoryPool_item *free_list;

	std::mutex sharedlock;

public:
	typedef std::unique_ptr<T, std::function<void(T*)>> unique_ptr;

	MemoryPool(size_t arena_size)
	: arena_size(arena_size), 
	  arena(new MemoryPool_arena(arena_size)),
	  free_list(arena->get_storage())
	{
	}

	template <typename... Args> 
	unique_ptr alloc(Args &&... args)
	{
		std::lock_guard<std::mutex> lock(this->sharedlock);

		// allocate a new arena if needed
    	if (free_list == nullptr)
    	{   
        	std::unique_ptr<MemoryPool_arena> new_arena(new MemoryPool_arena(arena_size));

        	new_arena->set_next_arena(std::move(arena));
        	arena.reset(new_arena.release());
        	free_list = arena->get_storage();
    	}   

		// fetch next empty
    	MemoryPool_item *current_item = free_list;
    	free_list = current_item->get_next_item();

    	T *result = current_item->get_storage();

		// construct object in allocated space
    	new (result) T(std::forward<Args>(args)...);

    	//return std::unique_ptr<T, std::function<void(T*)>>(result, [this](T* t)->void {this->free(t);});
    	return unique_ptr(result, [this](T* t)->void {this->free(t);});
	}
	

	void free(T* t)
	{
        std::lock_guard<std::mutex> lock(this->sharedlock);
        
        t->T::~T();
        
        MemoryPool_item *current_item = MemoryPool_item::storage_to_item(t);
        
        current_item->set_next_item(this->free_list);
        this->free_list = current_item;
	}
};

}

#endif
