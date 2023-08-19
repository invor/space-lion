#ifndef GENERIC_TEXTURE_LAYOUT_HPP
#define GENERIC_TEXTURE_LAYOUT_HPP

#include <vector>

struct GenericTextureLayout
{
    enum class InternalFormat {
        UNKNOWN,
        R8,
        R8_SNORM,
        R16,
        R16_SNORM,
        RG8,
        RG8_SNORM,
        RG16,
        RG16_SNORM,
        R3_G3_B2,
        RGB4,
        RGB5,
        RGB8,
        RGB8_SNORM,
        RGB10,
        RGB12,
        RGB16_SNORM,
        RGBA2,
        RGBA4,
        RGB5_A1,
        RGBA8,
        RGBA8_SNORM,
        RGB10_A2,
        RGB10_A2UI,
        RGBA12,
        RGBA16,
        SRGB8,
        SRGB8_ALPHA8,
        R16F,
        RG16F,
        RGB16F,
        RGBA16F,
        R32F,
        RG32F,
        RGB32F,
        RGBA32F,
        R11F_G11F_B10F,
        RGB9_E5,
        R8I,
        R8UI,
        R16I,
        R16UI,
        R32I,
        R32UI,
        RG8I,
        RG8UI,
        RG16I,
        RG16UI,
        RG32I,
        RG32UI,
        RGB8I,
        RGB8UI,
        RGB16I,
        RGB16UI,
        RGB32I,
        RGB32UI,
        RGBA8I,
        RGBA8UI,
        RGBA16I,
        RGBA16UI,
        RGBA32I,
        RGBA32UI
    };

    GenericTextureLayout()
        : width(0), internal_format(InternalFormat::UNKNOWN), height(0), depth(0), format(0), type(0), levels(0) {}
    /**
     * \param internal_format Specifies the (sized) internal format of a texture (e.g. GL_RGBA32F)
     * \param width Specifies the width of the texture in pixels.
     * \param height Specifies the height of the texture in pixels. Will be ignored by Texture1D.
     * \param depth Specifies the depth of the texture in pixels. Will be ignored by Texture1D and Texture2D.
     * \param format Specifies the format of the texture (e.g. GL_RGBA)
     * \param type Specifies the type of the texture (e.g. 5126 for GL_FLOAT)
     */
    GenericTextureLayout(InternalFormat internal_format, int width, int height, int depth, uint32_t format, uint32_t type, uint32_t levels)
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
    GenericTextureLayout(InternalFormat internal_format, int width, int height, int depth, uint32_t format, uint32_t type, uint32_t levels, std::vector<std::pair<uint32_t, int>> const& int_parameters, std::vector<std::pair<uint32_t, float>> const& float_parameters)
        : internal_format(internal_format), width(width), height(height), depth(depth), format(format), type(type), levels(levels), int_parameters(int_parameters), float_parameters(float_parameters) {}
    GenericTextureLayout(InternalFormat internal_format, int width, int height, int depth, uint32_t format, uint32_t type, uint32_t levels, std::vector<std::pair<uint32_t, int>> && int_parameters, std::vector<std::pair<uint32_t, float>> && float_parameters)
        : internal_format(internal_format), width(width), height(height), depth(depth), format(format), type(type), levels(levels), int_parameters(int_parameters), float_parameters(float_parameters) {}

    InternalFormat internal_format;
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
