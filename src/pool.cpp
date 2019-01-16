#include "pool.h"

namespace exampi
{

template <typename T>
MemoryPool<T>::MemoryPool(size_t arena_size) : arena_size(arena_size), arena(new MemoryPool::MemoryPool_arena(arena_size)), free_list(arena->get_storage())
{
}

template <typename T>
template <typename... Args>
std::unique_ptr<T> MemoryPool<T>::alloc(Args &&... args)
{
	std::lock_guard<std::mutex> lock(this->sharedlock);

	// allocate a new arena if needed
    if (free_list == nullptr)
    {   
        std::unique_ptr<MemoryPool::MemoryPool_arena> new_arena(new MemoryPool::MemoryPool_arena(arena_size));

        new_arena->set_next_arena(std::move(arena));
        arena.reset(new_arena.release());
        free_list = arena->get_storage();
    }   

	// fetch next empty
    MemoryPool::MemoryPool_item *current_item = free_list;
    free_list = current_item->get_next_item();

    T *result = current_item->get_storage();

	// construct object in allocated space
    new (result) T(std::forward<Args>(args)...);

    return std::unique_ptr<T>(result, MemoryPool::free);
}

template <typename T>
void MemoryPool<T>::free(std::unique_ptr<T> t)
{
        std::lock_guard<std::mutex> lock(this->sharedlock);
        
        t->T::~T();
        
        MemoryPool::MemoryPool_item *current_item = MemoryPool::MemoryPool_item::storage_to_item(t);
        
        current_item->set_next_item(this->free_list);
        this->free_list = current_item;
}

}
