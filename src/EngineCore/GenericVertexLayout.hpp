#ifndef GenericVertexLayout_hpp
#define GenericVertexLayout_hpp

#include <string>
#include <vector>

/**
* \struct GenericVertexLayout
*
* \brief API agnostic container for vertex layout descritions.
*
* \author Michael Becher
*/
struct GenericVertexLayout
{
    struct Attribute
    {
        Attribute(int size, uint32_t type, bool normalized, uint32_t offset)
            : semantic_name(""), size(size), type(type), normalized(normalized), offset(offset) {}

        Attribute(std::string const& semantic_name, int size, uint32_t type, bool normalized, uint32_t offset)
            : semantic_name(semantic_name), size(size), type(type), normalized(normalized), offset(offset) {}

        std::string semantic_name;
        int         size;
        uint32_t    type;
        bool        normalized;
        uint32_t    offset;
    };

    GenericVertexLayout() : stride(0), attributes() {}
    GenericVertexLayout(uint32_t stride, std::vector<Attribute> const& attributes)
        : stride(stride), attributes(attributes) {}
    GenericVertexLayout(uint32_t stride, std::vector<Attribute>&& attributes)
        : stride(stride), attributes(attributes) {}

    uint32_t               stride;
    std::vector<Attribute> attributes;
};

inline
bool operator==(GenericVertexLayout::Attribute const& lhs, GenericVertexLayout::Attribute const& rhs)
{
    return lhs.normalized == rhs.normalized && lhs.offset == rhs.offset && lhs.size == rhs.size && lhs.type == rhs.type;
}

inline
bool operator==(GenericVertexLayout const& lhs, GenericVertexLayout const& rhs)
{
    bool rtn = true;
    
    rtn &= lhs.stride == rhs.stride;
    
    if (lhs.attributes.size() == rhs.attributes.size())
    {
        for (size_t i = 0; i < lhs.attributes.size(); ++i)
        {
            rtn &= (lhs.attributes[i] == rhs.attributes[i]);
        }
    }
    else
    {
        rtn = false;
    }

    return rtn;
}

#endif // !GenericVertexLayout_hpp
