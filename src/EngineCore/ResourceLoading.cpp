#include "ResourceLoading.hpp"

#include <fstream>
#include <sstream>

namespace ResourceLoading
{
    std::string readShaderFile(const char* const path)
    {
        std::ifstream inFile(path, std::ios::in);

        //TODO throw exception if file not found

        std::ostringstream source;
        while (inFile.good()) {
            int c = inFile.get();
            if (!inFile.eof()) source << (char)c;
        }
        inFile.close();

        return source.str();
    }

    //	MaterialInfo parseMaterial(std::string const& material_path)
    //	{
    //		std::ifstream file;
    //		file.open(material_path, std::ifstream::in);
    //	
    //		MaterialInfo mtl_info;
    //	
    //		std::string vs_path;
    //		std::string fs_path;
    //		std::string gs_path;
    //		std::string tessCtrl_path;
    //		std::string tessEval_path;
    //	
    //		if (file.is_open())
    //		{
    //			file.seekg(0, std::ifstream::beg);
    //	
    //			std::string line;
    //	
    //			while (!file.eof())
    //			{
    //				getline(file, line, '\n');
    //	
    //				std::stringstream ss(line);
    //				std::string buffer;
    //				std::string texture_path;
    //	
    //				ss >> buffer;
    //	
    //				if (std::strcmp("vs", buffer.c_str()) == 0)
    //					ss >> vs_path;
    //				else if (std::strcmp("fs", buffer.c_str()) == 0)
    //					ss >> fs_path;
    //				else if (std::strcmp("gs", buffer.c_str()) == 0)
    //					ss >> gs_path;
    //				else if (std::strcmp("tc", buffer.c_str()) == 0)
    //					ss >> tessCtrl_path;
    //				else if (std::strcmp("te", buffer.c_str()) == 0)
    //					ss >> tessEval_path;
    //				else if (std::strcmp("td", buffer.c_str()) == 0)
    //					ss >> texture_path;
    //				else if (std::strcmp("ts", buffer.c_str()) == 0)
    //					ss >> texture_path;
    //				else if (std::strcmp("tr", buffer.c_str()) == 0)
    //					ss >> texture_path;
    //				else if (std::strcmp("tn", buffer.c_str()) == 0)
    //					ss >> texture_path;
    //				else if (std::strcmp("th", buffer.c_str()) == 0)
    //					ss >> texture_path;
    //	
    //				// TODO detect USE_TEXTURE2DARRAY flag
    //	
    //				if (!texture_path.empty())
    //					mtl_info.texture_filepaths.push_back(texture_path);
    //			}
    //	
    //			if (!vs_path.empty() && !fs_path.empty())
    //			{
    //				mtl_info.shader_filepaths.push_back(vs_path);
    //				mtl_info.shader_filepaths.push_back(fs_path);
    //			}
    //	
    //			if (!gs_path.empty())
    //			{
    //				mtl_info.shader_filepaths.push_back(gs_path);
    //			}
    //	
    //			if (!tessCtrl_path.empty() && !tessEval_path.empty())
    //			{
    //				mtl_info.shader_filepaths.push_back(tessCtrl_path);
    //				mtl_info.shader_filepaths.push_back(tessEval_path);
    //			}
    //	
    //		}
    //	
    //		return mtl_info;
    //	}

    std::array<std::string, 4> parseDecalMaterial(std::string const& material_path)
    {
        std::ifstream file;
        file.open(material_path, std::ifstream::in);

        std::array<std::string, 4> rtn;
        int counter = 0;

        if (file.is_open())
        {
            file.seekg(0, std::ifstream::beg);

            std::string line;

            while (!file.eof())
            {
                getline(file, line, '\n');

                std::stringstream ss(line);

                ss >> rtn[counter];

                ++counter;

                if (counter > 3)
                    break;
            }

        }

        return rtn;
    }

    void loadPngImage(std::string const& path, std::vector<unsigned char>& image_data, GenericTextureLayout& image_layout)
    {
        // Load file and decode image.
        unsigned int width, height;
        unsigned int error = lodepng::decode(image_data, width, height, path);

        image_layout.width = width;
        image_layout.height = height;
        image_layout.depth = 1;
        image_layout.internal_format = GenericTextureLayout::InternalFormat::RGBA8;// 0x8058; /*GL_RGBA8*/
        image_layout.format = 0x1908; /*GL_RGBA*/
        image_layout.type = 0x1401; /*GL_UNSIGNED_BYTE*/
    }

    void loadPpmImage(std::string const& path, std::vector<uint8_t>& image_data, GenericTextureLayout& image_layout)
    {
        unsigned long headerEnd;
        int imgDimX, imgDimY;

        readPpmHeader(path.c_str(), headerEnd, imgDimX, imgDimY);

        image_data.resize(imgDimX*imgDimY * 3);
        image_layout.width = imgDimX;
        image_layout.height = imgDimY;
        image_layout.depth = 1;
        image_layout.internal_format = GenericTextureLayout::InternalFormat::RGB8;//0x8051; /*GL_RGB8*/
        image_layout.format = 0x1907; /*GL_RGB*/
        image_layout.type = 0x1401; /*GL_UNSIGNED_BYTE*/

        readPpmData(path.c_str(), reinterpret_cast<char*>(image_data.data()), headerEnd, imgDimX, imgDimY);
    }

    void loadPpmImageRGBA(std::string const& path, std::vector<uint8_t>& image_data, GenericTextureLayout& image_layout)
    {
        unsigned long headerEnd;
        int imgDimX, imgDimY;

        readPpmHeader(path.c_str(), headerEnd, imgDimX, imgDimY);

        image_data.resize(imgDimX*imgDimY * 4);
        image_layout.width = imgDimX;
        image_layout.height = imgDimY;
        image_layout.depth = 1;
        image_layout.internal_format = GenericTextureLayout::InternalFormat::RGBA8;//0x8058; /*GL_RGBA8*/
        image_layout.format = 0x1908; /*GL_RGBA*/
        image_layout.type = 0x1401; /*GL_UNSIGNED_BYTE*/

        readPpmDataPaddedAlpha(path.c_str(), reinterpret_cast<char*>(image_data.data()), headerEnd, imgDimX, imgDimY);
    }

    bool readPpmHeader(const char* filename, unsigned long& headerEndPos, int& imgDimX, int& imgDimY)
    {
        int currentComponent = 0;
        bool firstline = false;
        std::string::iterator itr1;
        std::string::iterator itr2;
        std::string buffer;
        std::string compBuffer;
        std::ifstream file(filename, std::ios::in | std::ios::binary);

        /*
        /	Check if the file could be opened.
        */
        if (!(file.is_open()))return false;

        /*
        /	Go to the beginning of the file and read the first line.
        */
        file.seekg(0, file.beg);
        std::getline(file, buffer, '\n');
        itr1 = buffer.begin();
        for (itr2 = buffer.begin(); itr2 != buffer.end(); itr2++)
        {
            /*
            /	Check if the first line contains more than just ppm's magic number.
            /	If it does, it should look like this:
            /	"magic_number image_dimension_x image_dimension_y maximum_value"
            /	Therefore we scan the string for a space character and start parsing it.
            */
            if (*itr2 == ' ')
            {
                if (currentComponent == 0)
                {
                    /*	The first component is the magic number. We don't need it.	*/
                    currentComponent++;
                    firstline = true;
                    itr1 = (itr2 + 1);
                }
                else if (currentComponent == 1)
                {
                    /*	Get the image dimension in x.	*/
                    compBuffer.assign(itr1, itr2);
                    imgDimX = atoi(compBuffer.c_str());
                    currentComponent++;
                    itr1 = (itr2 + 1);
                }
                else if (currentComponent == 2)
                {
                    /*	Get the image dimension in y.	*/
                    compBuffer.assign(itr1, itr2);
                    imgDimY = atoi(compBuffer.c_str());
                    currentComponent++;
                    itr1 = (itr2 + 1);
                }
            }
        }

        /*
        /	If the information we were looking for was inside the first line, we are done here.
        /	Note the position where we left off and exit with return true after closing the file.
        */
        if (firstline)
        {
            headerEndPos = static_cast<long>(file.tellg());
            file.close();
            return true;
        }

        /*
        /	If the information wasn't inside the first line we have to keep reading lines.
        /	Skip all comment lines (first character = '#').
        */
        std::getline(file, buffer, '\n');
        while (buffer[0] == '#' || (buffer.size() < 1))
        {
            std::getline(file, buffer, '\n');
        }

        /*
        /	Now we should have a string containing the image dimensions and can extract them.
        */
        itr1 = buffer.begin();
        for (itr2 = buffer.begin(); itr2 != buffer.end(); itr2++)
        {
            /*	Get the image dimension in x.	*/
            if (*itr2 == ' ')
            {
                compBuffer.assign(itr1, itr2);
                imgDimX = atoi(compBuffer.c_str());
                currentComponent++;
                itr1 = (itr2 + 1);
            }
        }

        /*
        /	The last component of a line can't be parsed within the loop since it isn't followed by
        /	a space character, but an end-of-line.
        /
        /	Get the image dimension in x.
        */
        compBuffer.assign(itr1, itr2);
        imgDimY = atoi(compBuffer.c_str());

        /*
        /	Read one more line. This should contain the maximum value of the image, but we don't need
        /	that.
        /	Note down the position after this line and exit with return true after closing the file.
        */
        std::getline(file, buffer, '\n');
        headerEndPos = static_cast<unsigned long>(file.tellg());
        file.close();
        return true;
    }

    bool readPpmData(const char* filename, char* imageData, unsigned long dataBegin, int imgDimX, int imgDimY)
    {
        std::ifstream file(filename, std::ios::in | std::ios::binary);

        /*
        /	Check if the file could be opened.
        */
        if (!(file.is_open()))return false;

        /*
        /	Determine the length from the beginning of the image data to the end of the file.
        */
        file.seekg(0, file.end);
        unsigned long length = static_cast<unsigned long>(file.tellg());
        length = length - dataBegin;
        char* buffer = new char[length];

        file.seekg(dataBegin, std::ios::beg);
        file.read(buffer, length);

        /*
        /	Rearrange the image information so that the data begins with the lower left corner.
        */
        int k = 0;
        for (int i = 0; i < imgDimY; i++)
        {
            int dataLoc = (imgDimY - 1 - i)*imgDimX * 3;
            for (int j = 0; j < imgDimX; j++)
            {
                imageData[k] = buffer[dataLoc + (j * 3)];
                k++;
                imageData[k] = buffer[dataLoc + (j * 3) + 1];
                k++;
                imageData[k] = buffer[dataLoc + (j * 3) + 2];
                k++;
            }
        }

        file.close();
        delete[] buffer;
        return true;
    }

    bool readPpmDataPaddedAlpha(const char* filename, char* imageData, unsigned long dataBegin, int imgDimX, int imgDimY)
    {
        std::ifstream file(filename, std::ios::in | std::ios::binary);

        /*
        /	Check if the file could be opened.
        */
        if (!(file.is_open()))return false;

        /*
        /	Determine the length from the beginning of the image data to the end of the file.
        */
        file.seekg(0, file.end);
        unsigned long length = static_cast<unsigned long>(file.tellg());
        length = length - dataBegin;
        char* buffer = new char[length];

        file.seekg(dataBegin, std::ios::beg);
        file.read(buffer, length);

        /*
        /	Rearrange the image information so that the data begins with the lower left corner.
        */
        int k = 0;
        for (int i = 0; i < imgDimY; i++)
        {
            int dataLoc = (imgDimY - 1 - i)*imgDimX * 3;
            for (int j = 0; j < imgDimX; j++)
            {
                imageData[k] = buffer[dataLoc + (j * 3)];
                k++;
                imageData[k] = buffer[dataLoc + (j * 3) + 1];
                k++;
                imageData[k] = buffer[dataLoc + (j * 3) + 2];
                k++;

                imageData[k] = std::numeric_limits<char>::max();
                k++;
            }
        }

        file.close();
        delete[] buffer;
        return true;
    }

}