#ifndef ComponentStorage_hpp
#define ComponentStorage_hpp

#include <memory>
#include <mutex>
#include <vector>

namespace EngineCore {
    namespace Utility
    {
        template<typename T, size_t PageCount, size_t PageSize>
        class ComponentStorage
        {
        public:
            ComponentStorage();
            ~ComponentStorage() = default;

            size_t addComponent();

            void deleteComponent();

        private:
            struct Page
            {
                std::unique_ptr<std::vector<T>> storage;
                std::mutex                      mutex;
            };

            std::vector<Page> components_;

            std::mutex add_component_mutex_;
            std::atomic_size_t component_cnt_ = std::atomic_size_t{ 0 };

        };

        template<typename T, size_t PageCount, size_t PageSize>
        inline ComponentStorage<T, PageCount, PageSize>::ComponentStorage()
            : components_(PageCount)
        {
            for (size_t page_idx = 0; page_idx < PageCount; ++page_idx)
            {
                components_[page_idx].storage = nullptr;
            }
        }

        template<typename T, size_t PageCount, size_t PageSize>
        inline size_t ComponentStorage<T, PageCount, PageSize>::addComponent()
        {
            std::unique_lock<std::mutex> lock(add_component_mutex_);

            //TODO implement and use free list

            auto component_cnt = component_cnt_.load();

            // check if no page is allocated or last allocated page is full
            if ((component_cnt % PageSize) == 0)
            {
                // add new page
            }
            else {
                // find correct page
            }
        }
    }
}

#endif // !ComponentStorage_Hpp
