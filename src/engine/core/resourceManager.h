/**
* \class ResourceManager
* 
* \brief Loads and manages graphic resources
* 
* This class loads, stores and manages graphic resources (e.g. textures, meshes, shader programs)
* Resources managed by this class are supposed to be used by one or more scenes, for a more
* efficient memory managment.
* 
* \author Michael Becher
* 
* \date 10th September 2013
*/

#ifndef resourceManager_h
#define resourceManager_h

#define _CRT_SECURE_NO_DEPRECATE

/*	std includes */
#include <list>
#include <fstream>
#include <sstream>
#include <memory>

/*	Include space-lion headers */
#include "material.h"
#include "texture2D.h"
#include "texture3D.h"
#include "mesh.h"

#define DEBUG_OUTPUT 0

class ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();

	/** Returns log string */
	const std::string& getLog() {return resourcelog;}

	/**
	* \brief Clear lists containing graphics resources
	*/
	void clearLists();

	/**
	 * \brief Creates a triangle for debugging purposes
	 * \param inOutGeomPtr A pointer set to the newly created vertex geometry via in-out parameter
	 * \return Returns true if the triangle was succesfully created, false otherwise
	 */
	bool createTriangle(std::shared_ptr<Mesh> &inOutGeomPtr);

	/**
	 * \brief Creates a simple box object for debugging purposes
	 * \param inOutGeomPtr A pointer set to the newly created vertex geometry via in-out parameter
	 * \return Returns true if box was succesfully created, false otherwise
	 */
	bool createBox(std::shared_ptr<Mesh> &inOutGeomPtr);

	/**
	 * \brief Creates a Mesh object from a local file.
	 * \note Not yet implemented!
	 * \param path Location of the mesh file
	 * \param inOutGeomPtr A pointer set to the newly created vertex geometry via in-out parameter
	 * \return Returns true if Mesh was succesfully created, false otherwise
	 */
	bool createMesh(const std::string path, std::shared_ptr<Mesh> &inOutGeomPtr);

	/**
	 * \brief Creates default material object for debugging purposes
	 * \param inOutMtlPtr A pointer set to the newly created material via in-out parameter
	 * \return Returns true if material was succesfully created, false otherwise
	 */
	bool createMaterial(std::shared_ptr<Material> &inOutMtlPtr);

	/**
	 * \brief Creates a material object from a local file
	 * \param path Location of the material file
	 * \param inOutMtlPtr A pointer set to the newly created material via in-out parameter
	 * \return Returns true if material was succesfully created, false otherwise
	 */
	bool createMaterial(const char * const path, std::shared_ptr<Material> &inOutMtlPtr);

	/**
	 * \brief Reload a material object from file
	 * \note Not yet implemented. For later use, when some kind of editor allows to change material properties at runtime
	 * \return Returns true if material was succesfully reloaded, false otherwise
	 */
	bool reloadMaterial();

	/**
	 * Creates a GLSLprogram object
	 * \param type Specifies which shader program should be created
	 * \param inOutPrgPtr A pointer set to the newly created shader program via in-out parameter
	 * \retrun Returns true if GLSLprogram was succesfully created, false otherwise
	 */
	bool createShaderProgram(shaderType type, std::shared_ptr<GLSLProgram> &inOutPrgPtr);

	/**
	 * \brief Creates a 2D texture from a given float array
	 *
	 * Exercise some caution when using this function. It will always create a new texture object,
	 * since for now there is no reasonable way to check if an identical texture already exsists.
	 *
	 * \param dimX Texture resolution in texels in x-direction
	 * \param dimY Texture resolution in texels in y-direction
	 * \param data Float array containing the texel information
	 * \param inOutTexPtr A pointer set to the newly created texture via in-out parameter
	 * \return Returns true if texture was succesfully created, false otherwise
	 */
	bool createTexture2D(int dimX, int dimY, float* data, std::shared_ptr<Texture> &inOutTexPtr);

	/**
	 * \brief Creates a 2D texture from a file
	 * \param path Location of the texture file
	 * \param inOutTexPtr A pointer set to the newly created texture via in-out parameter
	 * \return Returns true if texture was succesfully created, false otherwise
	 */
	bool createTexture2D(const std::string path, std::shared_ptr<Texture> &inOutTexPtr);

	/**
	 * \brief Reloads a texture in case a texture file is changed during runtime
	 * \note Not yet implemented!
	 * \return Returns true if texture was succesfully reloaded, false otherwise
	 */
	bool reloadTexture();

	/**
	 * \brief Creates a 3D texture for volume rendering from a file
	 * \param path Location of the volume texture file
	 * \param textureRes Resolution of the volume
	 * \param inOutTexPtr A pointer set to the newly created
	 * \return Returns true if volume texture was succesfully created, false otherwise
	 */
	bool createTexture3D(const std::string path, glm::ivec3 textureRes, std::shared_ptr<Texture3D> &inOutTexPtr);

	/**
	 * \brief Creates a 3D texture for volume rendering from a given float array
	 * \param volumeData Float array containing the voxel information
	 * \param textureRes Resolution of the volume
	 * \param internalFormat Internal format of the volume data
	 * \param format Format of the volume data
	 * \param inOutTexPtr A pointer set to the newly created
	 * \return Returns true if volume texture was succesfully created, false otherwise
	 */
	bool createTexture3D(float* volumeData, glm::ivec3 textureRes, GLenum internalFormat, GLenum format, std::shared_ptr<Texture3D> &inOutTexPtr);

protected:
	/** Log string */
	std::string resourcelog;

	/*
	/	The following lists contain all resources that are managed by an instance of this class.
	/	There is only a single "instance" of any (uniquely identifiable) resouce kept/referenced in these lists.
	/	Different scene entities making use of the same resource, will both be refering to the single
	/	instance kept/referenced within one of these lists.
	*/

	/** List containing all Mesh objects */
	std::list<std::shared_ptr<Mesh>> geometry_list;
	/** List containing all materials */
	std::list<std::shared_ptr<Material>> material_list;
	/** List containing all 2D textures */
	std::list<std::shared_ptr<Texture2D>> texture_list;
	/** List containing all 3D textures */
	std::list<std::shared_ptr<Texture3D>> volume_list;
	/** List containing all shader programs */
	std::list<std::shared_ptr<GLSLProgram>> shader_program_list;

	/**
	 * \brief Load geometry information from an fbx file
	 * \param path Location of the fbx file
	 * \param goemPtr Pointer to the Mesh object where the loaded geometry will be stored
	 * \return Returns true if the geometry was succesfully loaded, false otherwise
	 */
	bool loadFbxGeometry(const std::string &path, std::shared_ptr<Mesh> &geomPtr);

	/**
	 * \brief Load geometry information from a binary file
	 * \note Not yet implemented
	 * \param path Location of the binary file
	 * \param goemPtr Pointer to the Mesh object where the loaded geometry will be stored
	 * \return Returns true if the geometry was succesfully loaded, false otherwise
	 */
	bool loadBinaryGeometry(const std::string &path, std::shared_ptr<Mesh> &geomPtr);

	/**
	 * \brief Parses a material file
	 * \param materialPath Location of the material file
	 * \param inOutMtlInfo Contains the material information after the method is called (in-out parameter)
	 * \return Returns true if the material file was succesfully parsed, false otherwise
	 */
	bool parseMaterial(const char* const materialPath, MaterialInfo& inOutMtlInfo);

	/**
	 * \brief Read a shader source file
	 * \param path Location of the shader file
	 * \return Returns a string containing the shader source
	 */
	const std::string readShaderFile(const char* const path);

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
};

#endif