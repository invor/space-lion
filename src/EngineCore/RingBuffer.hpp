#ifndef RingBuffer_hpp
#define RingBuffer_hpp

#include <vector>

namespace EngineCore
{
    namespace Utility
    {
        template<class T>
        class RingBuffer
        {
        public:
            RingBuffer(size_t ringbuffer_size)
                : storage(ringbuffer_size), current_idx(0)
            {}
            ~RingBuffer() = default;

            void push(T const& v)
            {
                // advance current index (with wrap around)
                current_idx = (current_idx + 1) % storage.size();
                // and write value to storage
                storage[current_idx] = v;
            }

            size_t size()
            {
                return storage.size();
            }

            size_t currentIndex()
            {
                return current_idx;
            }

            T& operator [](size_t idx)
            {
                return storage[idx];
            }

            T operator [](size_t idx) const
            {
                return storage[idx];
            }

        private:
            std::vector<T> storage;
            size_t current_idx;
        };
    }

}

#endif // !RingBuffer_hpp
