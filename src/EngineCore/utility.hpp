#ifndef utility_hpp
#define utility_hpp

#include <utility>

#include "types.hpp"

namespace utility
{
    template <typename IndexMap>
    std::pair<bool, uint> entityToIndex(uint eID, IndexMap const& map)
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
}

#endif // !utility_hpp
