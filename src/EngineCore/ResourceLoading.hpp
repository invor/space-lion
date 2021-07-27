#ifndef ResourceLoading_hpp
#define ResourceLoading_hpp

#include <string>
#include <array>
#include <vector>
#include <filesystem>
#include <fstream>
#include <format>

#include <lodepng.h>

#include "GenericTextureLayout.hpp"

struct SimulationState;

namespace EngineCore {
namespace Utility {
    // Copyright (c) Microsoft Corporation.
    // Licensed under the MIT License.
    //
    // Modified to use std::filesystem
    inline std::vector<uint8_t> ReadFileBytes(const std::filesystem::path& path) {
        bool fileExists = false;
        try {
            std::ifstream file;
            file.exceptions(std::ios::failbit | std::ios::badbit);
            file.open(path, std::ios::binary | std::ios::ate);
            fileExists = true;
            // If tellg fails then it will throw an exception instead of returning -1.
            std::vector<uint8_t> data(static_cast<size_t>(file.tellg()));
            file.seekg(0, std::ios::beg);
            file.read(reinterpret_cast<char*>(data.data()), data.size());
            return data;
        }
        catch (const std::ios::failure&) {
            // The exception only knows that the failbit was set so it doesn't contain anything useful.
            throw std::runtime_error(std::format("Failed to {} file: {}", fileExists ? "read" : "open", path.string()));
        }
    }

    // Copyright (c) Microsoft Corporation.
    // Licensed under the MIT License.
    //
    // Modified to use only UWP version
    inline std::filesystem::path GetAppFolder() {
#ifdef UWP
        HMODULE thisModule;
        thisModule = nullptr;

        wchar_t moduleFilename[MAX_PATH];
        ::GetModuleFileName(thisModule, moduleFilename, (DWORD)std::size(moduleFilename));
        std::filesystem::path fullPath(moduleFilename);
        return fullPath.remove_filename();
#else
        return "";
        //TODO throw exception?
#endif
    }

    // Copyright (c) Microsoft Corporation.
    // Licensed under the MIT License.
    inline std::filesystem::path GetPathInAppFolder(const std::filesystem::path& filename) {
        return GetAppFolder() / filename;
    }
}
}

namespace ResourceLoading
{
    /**
    * \brief Read a shader source file
    * \param path Location of the shader file
    * \return Returns a string containing the shader source
    */
    std::string readShaderFile(const char* const path);

    std::array<std::string, 4> parseDecalMaterial(std::string const& material_path);

    void loadPngImage(std::string const& path, std::vector<unsigned char>& image_data, GenericTextureLayout& image_layout);

    /**
    * \brief Load image data from a ppm image file into a CPU-side buffer
    * \param path Location of the ppm file
    * \param image_data Vector for storing the loaded image data
    * \param image_layout Store the layout of the loaded image (i.e. size, channels, bit-depth)
    */
    void loadPpmImage(std::string const& path, std::vector<uint8_t>& image_data, GenericTextureLayout& image_layout);

    /**
    * Load image data from a ppm image file into padded RGBA format buffer (with alpha=1.0).
    * Required for image load/store, possibly useful for texture compression.
    */
    void loadPpmImageRGBA(std::string const& path, std::vector<uint8_t>& image_data, GenericTextureLayout& image_layout);

    /**
    * \brief Read a the header of a ppm image file. Courtesy to the computer vision lecture I attended.
    * \param filename Location of the image file
    * \param headerEndPos Out parameter, marks the point where the header of the ppm file ends
    * \param imgDimX Out parameter, containing the dimension of the image in X direction in pixels
    * \param imgDimY Out parameter, containing the dimension of the image in Y direction in pixels
    * \return Returns true if the ppm header was succesfully read, false otherwise
    */
    bool readPpmHeader(const char* filename, unsigned long& headerEndPos, int& imgDimX, int& imgDimY);

    /**
    * \brief Read a the data of a ppm image file. Courtesy to the computer vision lecture I attended.
    * \param filename Location of the image file
    * \param imageData Pointer to the data buffer, that the image data will be written to
    * \param dataBegin Marks the location within the ppm file, where the data block begins
    * \param imgDimX Dimension of the image in X direction in pixels
    * \param imgDimY Dimension of the image in Y direction in pixels
    * \return Returns true if the ppm header was succesfully read, false otherwise
    */
    bool readPpmData(const char* filename, char* imageData, unsigned long dataBegin, int imgDimX, int imgDimY);

    /**
    * \brief Read a the data of a ppm image file with padded alpha value. Courtesy to the computer vision lecture I attended.
    * \param filename Location of the image file
    * \param imageData Pointer to the data buffer, that the image data will be written to
    * \param dataBegin Marks the location within the ppm file, where the data block begins
    * \param imgDimX Dimension of the image in X direction in pixels
    * \param imgDimY Dimension of the image in Y direction in pixels
    * \return Returns true if the ppm header was succesfully read, false otherwise
    */
    bool readPpmDataPaddedAlpha(const char* filename, char* imageData, unsigned long dataBegin, int imgDimX, int imgDimY);

    void loadBikeSimulationData(std::string const & path, std::vector<SimulationState>& data);
}

#endif // !ResourceLoading_hpp
