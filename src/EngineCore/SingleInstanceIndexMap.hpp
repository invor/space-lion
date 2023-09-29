#ifndef SingleInstanceIndexMap_hpp
#define SingleInstanceIndexMap_hpp

#include <atomic>
#include <list>
#include <mutex>
#include <vector>

namespace EngineCore {
    namespace Utility {
        class SingleInstanceIndexMap
        {
        public:
            SingleInstanceIndexMap();
            ~SingleInstanceIndexMap() = default;

            void addIndex(unsigned int entity_id, size_t index);

            size_t getIndex(unsigned int entity_id) const;

        private:
            struct Page {
                std::unique_ptr<std::vector<std::atomic_size_t>> storage;
                std::atomic_flag loaded;
            };

            std::vector<Page> index_map_;

            static const size_t page_cnt_{ 10000 };
            static const size_t page_size_{ 16000 };

            std::mutex add_index_mutex_;
        };

        inline SingleInstanceIndexMap::SingleInstanceIndexMap()
            : index_map_(page_cnt_)
        {
            for (size_t page_idx = 0; page_idx < page_cnt_; ++page_idx)
            {
                index_map_[page_idx].storage = nullptr;
                index_map_[page_idx].loaded.clear();
            }
        }

        inline void SingleInstanceIndexMap::addIndex(unsigned int entity_id, size_t component_index)
        {
            std::unique_lock<std::mutex> lock(add_index_mutex_);

            //find correct page
            auto div = std::div(static_cast<long>(entity_id), static_cast<long>(page_size_));
            auto page_index = div.quot;
            auto index_in_page = div.rem;

            assert(page_index < page_cnt_);

            if (!(index_map_[page_index].loaded.test()))
            {
                //add need new page
                index_map_[page_index].storage = std::make_unique<std::vector<std::atomic_size_t>>(page_size_);
                index_map_[page_index].loaded.test_and_set();

                //set init value in new page
                for (size_t j = 0; j < page_size_; ++j) {
                    index_map_[page_index].storage->at(j).store((std::numeric_limits<size_t>::max)());
                }

                //flag new page as loaded
                index_map_[page_index].loaded.test_and_set();
            }

            //store index for entity
            index_map_[page_index].storage->at(index_in_page).store(component_index);
        }
        
        inline size_t SingleInstanceIndexMap::getIndex(unsigned int entity_id) const
        {
            size_t retval = (std::numeric_limits<size_t>::max)();

            //find correct page
            auto div = std::div(static_cast<long>(entity_id), static_cast<long>(page_size_));
            auto page_index = div.quot;
            auto index_in_page = div.rem;

            assert(page_index < page_cnt_);

            if (index_map_[page_index].loaded.test())
            {
                //store index for entity
                retval = index_map_[page_index].storage->at(index_in_page).load();
            }

            return retval;
        }
    }
}

#endif // !SingleInstanceIndexMap_hpp
