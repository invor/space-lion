#ifndef utility_hpp
#define utility_hpp

#include <utility>

#include "types.hpp"

namespace utility
{
    template <typename IndexMap>
    inline std::pair<bool, uint> entityToIndex(uint eID, IndexMap const& map)
    {
        std::pair<bool, uint> rtn(std::pair<bool, uint>(false, -1));

        auto query = map.find(eID);

        if (query != map.end())
        {
            rtn.first = true;
            rtn.second = query->second;
        }

        return rtn;
    }

    inline std::vector<std::pair<size_t, size_t>> buildComponentProcessingRanges(size_t component_cnt, size_t bucket_cnt) {
        std::vector<std::pair<size_t, size_t>> from_to_pairs;
        from_to_pairs.reserve(bucket_cnt);
        for (size_t i = 0; i < bucket_cnt; ++i) {
            from_to_pairs.push_back({ (component_cnt * i) / bucket_cnt, (component_cnt * (i + 1)) / bucket_cnt });
        }
        return from_to_pairs;
    }
}

#endif // !utility_hpp
