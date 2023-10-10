#ifndef ComponentStorage_hpp
#define ComponentStorage_hpp

#include <memory>
#include <shared_mutex>
#include <vector>
#include <queue>

namespace EngineCore {
    namespace Utility {

        template<typename T, size_t PageCount, size_t PageSize>
        class ComponentStorage
        {
        public:
            ComponentStorage();
            ~ComponentStorage() = default;

            constexpr size_t getPageCount();

            constexpr size_t getPageSize();

            size_t addComponent(T component);

            void deleteComponent(size_t component_index);

            size_t getComponentCount() const;

            T& operator()(size_t page_index, size_t index_in_page);
            
            T const& operator()(size_t page_index, size_t index_in_page) const;

            T getComponentCopy(size_t component_index) const;

            std::pair<size_t, size_t> getIndices(size_t component_index) const;

            std::unique_lock<std::shared_mutex> accquirePageLock(size_t page_index) const;

        private:
            struct Page
            {
                std::unique_ptr<std::vector<std::pair<bool,T>>> storage;
                mutable std::shared_mutex                       mutex;
            };

            std::vector<Page>  components_;
            std::queue<size_t> free_list_;

            std::mutex         add_component_mutex_;
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
        inline constexpr size_t ComponentStorage<T, PageCount, PageSize>::getPageCount()
        {
            return PageCount;
        }

        template<typename T, size_t PageCount, size_t PageSize>
        inline constexpr size_t ComponentStorage<T, PageCount, PageSize>::getPageSize()
        {
            return PageSize;
        }

        template<typename T, size_t PageCount, size_t PageSize>
        inline size_t ComponentStorage<T, PageCount, PageSize>::addComponent(T component)
        {
            std::unique_lock<std::mutex> lock(add_component_mutex_);

            size_t component_cnt = component_cnt_.load();
            size_t component_index;

            // check for free component slots to overwrite
            if (!free_list_.empty())
            {
                component_index = free_list_.front();
                free_list_.pop();
            }
            else
            {
                component_index = component_cnt;
            }

            auto [page_index, index_in_page] = getIndices(component_index);
            assert(page_index < PageCount); // TODO handle full storage with exception?
            assert(index_in_page < PageSize);

            std::unique_lock<std::shared_mutex> page_lock(components_[page_index].mutex);

            // check if page is allocated
            if (components_[page_index].storage == nullptr)
            {
                // add new page
                components_[page_index].storage = std::make_unique<std::vector<std::pair<bool,T>>>(PageSize);

                for (auto& c : (*components_[page_index].storage))
                {
                    c.first = false;
                }
            }

            // write component to page
            components_[page_index].storage->operator[](index_in_page) = { true, std::move(component) };

            // if component slot was not re-used, increment component count
            if (component_index == component_cnt) {
                ++component_cnt_;
            }

            return component_index;
        }

        template<typename T, size_t PageCount, size_t PageSize>
        inline void ComponentStorage<T, PageCount, PageSize>::deleteComponent(size_t component_index)
        {
            std::unique_lock<std::mutex> lock(add_component_mutex_);

            free_list_.push(component_index);

            // TODO actually somehow mark components as invalid?
        }

        template<typename T, size_t PageCount, size_t PageSize>
        inline size_t ComponentStorage<T, PageCount, PageSize>::getComponentCount() const
        {
            return component_cnt_.load();
        }

        template<typename T, size_t PageCount, size_t PageSize>
        inline T& ComponentStorage<T, PageCount, PageSize>::operator()(size_t page_index, size_t index_in_page)
        {
            assert(components_[page_index].storage != nullptr);
        
            return components_[page_index].storage->operator[](index_in_page).second;
        }
        
        template<typename T, size_t PageCount, size_t PageSize>
        inline T const& ComponentStorage<T, PageCount, PageSize>::operator()(size_t page_index, size_t index_in_page) const
        {
            assert(components_[page_index].storage != nullptr);
        
            return components_[page_index].storage->operator[](index_in_page).second;
        }

        template<typename T, size_t PageCount, size_t PageSize>
        inline T ComponentStorage<T, PageCount, PageSize>::getComponentCopy(size_t component_index) const
        {
            auto [page_index, index_in_page] = getIndices(component_index);

            assert(components_[page_index].storage != nullptr);

            return components_[page_index].storage->at(index_in_page).second;
        }

        template<typename T, size_t PageCount, size_t PageSize>
        inline std::unique_lock<std::shared_mutex> ComponentStorage<T, PageCount, PageSize>::accquirePageLock(size_t page_index) const
        {
            //TODO better error handling?
            assert(page_index < PageCount);

            return std::unique_lock<std::shared_mutex>(components_[page_index].mutex);;
        }

        template<typename T, size_t PageCount, size_t PageSize>
        inline std::pair<size_t, size_t> ComponentStorage<T, PageCount, PageSize>::getIndices(size_t component_index) const
        {
            //auto div = std::div(static_cast<long>(component_index), static_cast<long>(PageSize));
            // return { page_index, index_in_page }
            //return std::pair<size_t, size_t>{div.quot, div.rem};
            return std::pair<size_t, size_t>{component_index / PageSize, component_index % PageSize};
        }

    }
}

#endif // !ComponentStorage_Hpp
