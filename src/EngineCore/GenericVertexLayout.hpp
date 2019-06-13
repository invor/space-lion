#ifndef GenericVertexLayout_hpp
#define GenericVertexLayout_hpp

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
            : size(size), type(type), normalized(normalized), offset(offset) {}

        int size;
        uint32_t type;
        bool normalized;
        uint32_t offset;
    };

    GenericVertexLayout() : byte_size(0), attributes() {}
    GenericVertexLayout(uint32_t byte_size, const std::vector<Attribute>& attributes)
        : byte_size(byte_size), attributes(attributes) {}
    GenericVertexLayout(uint32_t byte_size, std::vector<Attribute>&& attributes)
        : byte_size(byte_size), attributes(attributes) {}

    uint32_t byte_size;
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
    bool rtn = (lhs.byte_size == rhs.byte_size);

    if (lhs.attributes.size() == rhs.attributes.size())
    {
        for (size_t i = 0; i < lhs.attributes.size(); ++i)
        {
            rtn &= (lhs.attributes == rhs.attributes);
        }
    }
    else
    {
        rtn = false;
    }

    return rtn;
}

#endif // !GenericVertexLayout_hpp
