#ifndef GENERIC_TEXTURE_LAYOUT_HPP
#define GENERIC_TEXTURE_LAYOUT_HPP

#include <vector>

struct GenericTextureLayout
{
    GenericTextureLayout()
        : width(0), internal_format(0), height(0), depth(0), format(0), type(0), levels(0) {}
    /**
     * \param internal_format Specifies the (sized) internal format of a texture (e.g. GL_RGBA32F)
     * \param width Specifies the width of the texture in pixels.
     * \param height Specifies the height of the texture in pixels. Will be ignored by Texture1D.
     * \param depth Specifies the depth of the texture in pixels. Will be ignored by Texture1D and Texture2D.
     * \param format Specifies the format of the texture (e.g. GL_RGBA)
     * \param type Specifies the type of the texture (e.g. 5126 for GL_FLOAT)
     */
    GenericTextureLayout(int internal_format, int width, int height, int depth, uint32_t format, uint32_t type, uint32_t levels)
        : internal_format(internal_format), width(width), height(height), depth(depth), format(format), type(type), levels(levels) {}

    /**
    * \param internal_format Specifies the (sized) internal format of a texture (e.g. GL_RGBA32F)
    * \param width Specifies the width of the texture in pixels.
    * \param height Specifies the height of the texture in pixels. Will be ignored by Texture1D.
    * \param depth Specifies the depth of the texture in pixels. Will be ignored by Texture1D and Texture2D.
    * \param format Specifies the format of the texture (e.g. GL_RGBA)
    * \param type Specifies the type of the texture (e.g. GL_FLOAT)
    * \param int_parameters A list of integer texture parameters, each given by a pair of name and value (e.g. {{GL_TEXTURE_SPARSE_ARB,GL_TRUE},{...},...}
    * \param int_parameters A list of float texture parameters, each given by a pair of name and value (e.g. {{GL_TEXTURE_MAX_ANISOTROPY_EX,4.0f},{...},...}
    */
    GenericTextureLayout(int internal_format, int width, int height, int depth, uint32_t format, uint32_t type, uint32_t levels, std::vector<std::pair<uint32_t, int>> const& int_parameters, std::vector<std::pair<uint32_t, float>> const& float_parameters)
        : internal_format(internal_format), width(width), height(height), depth(depth), format(format), type(type), levels(levels), int_parameters(int_parameters), float_parameters(float_parameters) {}
    GenericTextureLayout(int internal_format, int width, int height, int depth, uint32_t format, uint32_t type, uint32_t levels, std::vector<std::pair<uint32_t, int>> && int_parameters, std::vector<std::pair<uint32_t, float>> && float_parameters)
        : internal_format(internal_format), width(width), height(height), depth(depth), format(format), type(type), levels(levels), int_parameters(int_parameters), float_parameters(float_parameters) {}

    int internal_format;
    int width;
    int height;
    int depth;
    uint32_t format;
    uint32_t type;

    uint32_t levels;

    std::vector<std::pair<uint32_t, int>> int_parameters;
    std::vector<std::pair<uint32_t, float>> float_parameters;
};

#endif // !GENERIC_TEXTURE_LAYOUT_HPP
