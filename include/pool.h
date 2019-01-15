#include <mutex>

// potentially do a SCSP kind of thing, we have sequential maybe
// https://thinkingeek.com/2017/11/19/simple-memory-pool/
// MR: this is a dirty copy of that, use that as base for good implementation

namespace exampi
{

template <typename T>
class MemoryPool
{
private:
	size_t size;
	
	std::unique_ptr<pool_area> area;

	std::mutex lock;

public:
	MemoryPool();
	MemoryPool(size_t size);

	template <typename... Args> T *alloc(Args &&... args);
	void free(T *t);
};

}
