#ifndef ResourceLoading_hpp
#define ResourceLoading_hpp

#include <string>
#include <array>
#include <vector>

#include <lodepng/lodepng.h>

#include "Mesh.hpp"
#include "Texture.hpp"

struct SimulationState;

namespace ResourceLoading
{
	/**
	* \brief Read a shader source file
	* \param path Location of the shader file
	* \return Returns a string containing the shader source
	*/
	std::string readShaderFile(const char* const path);

	std::array<std::string, 4> parseDecalMaterial(const std::string& material_path);

	void loadPngImage(const std::string& path, std::vector<unsigned char>& image_data, TextureLayout& image_layout);

	/**
	* \brief Load image data from a ppm image file into a CPU-side buffer
	* \param path Location of the ppm file
	* \param image_data Vector for storing the loaded image data
	* \param image_layout Store the layout of the loaded image (i.e. size, channels, bit-depth)
	*/
	void loadPpmImage(const std::string& path, std::vector<uint8_t>& image_data, TextureLayout& image_layout);

	/**
	* Load image data from a ppm image file into padded RGBA format buffer (with alpha=1.0).
	* Required for image load/store, possibly useful for texture compression.
	*/
	void loadPpmImageRGBA(const std::string& path, std::vector<uint8_t>& image_data, TextureLayout& image_layout);

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
