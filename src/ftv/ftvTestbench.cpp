#include "ftvTestbench.h"

bool FtvTestbench::readPpmHeader(const char* filename, long& headerEndPos, int& imgDimX, int& imgDimY)
{
	/*
	/	C stlye
	*/
	//	/*
	//	/	Start off with reading the header of the ppm file.
	//	*/
	//	char buffer[80];
	//	FILE *file;
	//	
	//	file = fopen(filename,"rb");
	//	if(file==NULL) return false;
	//	
	//	/*
	//	/	Read image dimensions from header.
	//	/
	//	/	The header of our ppm files consists of a single line with the following layout:
	//	/	magic_number 'space' image_dimension_x 'space' image_dimension_y 'space' maximum_value
	//	/	e.g	F6 400 400 255
	//	*/
	//	fgets(buffer, 300, file);
	//	sscanf(buffer, "%*c %*d %d %d", &imgDimX, &imgDimY);
	//	
	//	/*
	//	/	Get the position of the header end.
	//	*/
	//	headerEndPos = ftell(file);
	//	
	//	return true;

	int currentComponent = 0;
	bool firstline = false;
	std::string::iterator itr1;
	std::string::iterator itr2;
	std::string buffer;
	std::string compBuffer;
	std::ifstream file (filename,std::ios::in | std::ios::binary);

	/*
	/	Check if the file could be opened.
	*/
	if(!( file.is_open() ))return false;

	/*
	/	Go to the beginning of the file and read the first line.
	*/
	file.seekg(0, file.beg);
	std::getline(file,buffer,'\n');
	itr1 = buffer.begin();
	for(itr2 = buffer.begin(); itr2 != buffer.end(); itr2++)
	{
		/*
		/	Check if the first line contains more than just ppm's magic number.
		/	If it does, it should look like this:
		/	"magic_number image_dimension_x image_dimension_y maximum_value"
		/	Therefore we scan the string for a space character and start parsing it.
		*/
		if(*itr2 == ' ')
		{
			if(currentComponent == 0)
			{
				/*	The first component is the magic number. We don't need it.	*/
				currentComponent++;
				firstline = true;
				itr1 = (itr2 + 1);
			}
			else if(currentComponent == 1)
			{
				/*	Get the image dimension in x.	*/
				compBuffer.assign(itr1, itr2);
				imgDimX = atoi(compBuffer.c_str());
				currentComponent++;
				itr1 = (itr2 + 1);
			}
			else if(currentComponent == 2)
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
	if(firstline)
	{
		headerEndPos = file.tellg();
		file.close();
		return true;
	}

	/*
	/	If the information wasn't inside the first line we have to keep reading lines.
	/	Skip all comment lines (first character = '#').
	*/
	while( buffer[0]=='#' || (buffer.size() < 1) )
	{
		std::getline(file,buffer,'\n');
	}

	/*
	/	Now we should have a string containing the image dimensions and can extract them.
	*/
	itr1 = buffer.begin();
	for(itr2 = buffer.begin(); itr2 != buffer.end(); itr2++)
	{
		/*	Get the image dimension in x.	*/
		if(*itr2 == ' ')
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
	std::getline(file,buffer,'\n');
	headerEndPos = file.tellg();
	file.close();
	return true;
}

bool FtvTestbench::readPpmData(const char* filename, char* imageData, long dataBegin, int imgDimX, int imgDimY)
{
	/*
	/	C stlye
	*/
	//	FILE *file;
	//	
	//	file = fopen(filename,"rb");
	//	if(file==NULL) return false;
	//	fseek(file,dataBegin,SEEK_SET);
	//	/*
	//	/	Store the complete image data in a single float array.
	//	/	Channel-wise storing is unnecessary for the openGL usage.
	//	*/
	//	for(int i=0; i < imageSize; i++)
	//	{
	//		imageData[i] = getc(file);
	//	}
	//	fclose(file);
	//	
	//	return true;

	std::ifstream file (filename,std::ios::in | std::ios::binary);

	/*
	/	Check if the file could be opened.
	*/
	if(!( file.is_open() ))return false;

	/*
	/	Determine the length from the beginning of the image data to the end of the file.
	*/
	file.seekg(0, file.end);
	long length = file.tellg();
	length = length - dataBegin;
	char* buffer = new char[length];

	file.seekg(dataBegin,std::ios::beg);
	file.read(buffer,length);

	/*
	/	Rearrange the image information so that the data begins with the lower left corner.
	*/
	int k = 0;
	for(int i=0; i < imgDimY; i++)
	{
		int dataLoc = (imgDimY-1-i)*imgDimX*3;
		for(int j=0; j < imgDimX; j++)
		{
			imageData[k]=buffer[dataLoc+(j*3)];
			k++;
			imageData[k]=buffer[dataLoc+(j*3)+1];
			k++;
			imageData[k]=buffer[dataLoc+(j*3)+2];
			k++;
		}
	}

	file.close();
	delete[] buffer;
	return true;
}

bool FtvTestbench::readRawImageF(const char* filename, float*& imageData, int& size)
{
	std::ifstream file(filename, std::ios::in | std::ios::binary);
	
	/*	Check if the file could be opened. */
	if (!(file.is_open()))return false;
	
	/*	Determine the length from the beginning of the image data to the end of the file. */
	file.seekg(0, file.end);
	long length = file.tellg();
	char* buffer = new char[length];
	
	file.seekg(0, file.beg);
	file.read(buffer, length);
	
	imageData = reinterpret_cast<float*>(buffer);
	/*	Length of the float array equals length in byte diveded by four, since sizeof(float)=4 */
	size = length / 4;
	
	return true;

	//FILE *pFile;
	//
	//fopen_s(&pFile,filename, "rb");
	//if (pFile==NULL) return false;
	//
	//fread(imageData,sizeof(GLfloat),size,pFile);
	//
	//return true;
}

bool FtvTestbench::loadImageSequence()
{
	char* imageData;
	long dataBegin;
	int imgDimX;
	int imgDimY;
	std::string path;

	/*
	/	Careful with accessing the image arrays here!
	*/
	for(int i = 100; i < 151; i++)
	{
		path = "../resources/textures/fault_tolerant_vis/ftle_images/ftle_f_";
		path += '1';
		path += ('0' + floor(((float)i-100.0f)/10.0f));
		path += ('0' + (i-100)%10);
		path += ".ppm";

		//std::cout<<"Now loading "<<path<<"\n";

		//if(!readPpmHeader("../resources/textures/fault_tolerant_vis/ftle/ftle_f_100.ppm",dataBegin,imgDimX,imgDimY)) return false;
		if(!readPpmHeader(path.c_str(),dataBegin,imgDimX,imgDimY)) return false;
		imageData = new char[3*imgDimX*imgDimY];
		//if(!readPpmData("../resources/textures/fault_tolerant_vis/ftle/ftle_f_100.ppm",imageData,dataBegin,(3*imgDimX*imgDimY))) return false;
		if(!readPpmData(path.c_str(),imageData,dataBegin,imgDimX,imgDimY)) return false;

		glGenTextures(1, &textures_f[i-100]);
		glBindTexture(GL_TEXTURE_2D, textures_f[i-100]);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,imgDimX,imgDimY,0,GL_RGB,GL_UNSIGNED_BYTE,imageData);
		glBindTexture(GL_TEXTURE_2D,0);
	}

	//for(int i = 100; i < 151; i++)
	//{
	//	path = "../resources/textures/fault_tolerant_vis/ftle/ftle_b_";
	//	path += '1';
	//	path += ('0' + floor((i-100)/10));
	//	path += ('0' + (i-100)%10);
	//	path += ".ppm";
	//
	//	std::cout<<"Now loading "<<path<<"\n";
	//
	//	//if(!readPpmHeader("../resources/textures/fault_tolerant_vis/ftle/ftle_f_100.ppm",dataBegin,imgDimX,imgDimY)) return false;
	//	if(!readPpmHeader(path.c_str(),dataBegin,imgDimX,imgDimY)) return false;
	//	//if(!readPpmData("../resources/textures/fault_tolerant_vis/ftle/ftle_f_100.ppm",imageData,dataBegin,(3*imgDimX*imgDimY))) return false;
	//	if(!readPpmData(path.c_str(),imageData,dataBegin,(3*imgDimX*imgDimY))) return false;
	//
	//	glGenTextures(1, &textures_b[i-100]);
	//	glBindTexture(GL_TEXTURE_2D, textures_b[i-100]);
	//	glGenerateMipmap(GL_TEXTURE_2D);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,imgDimX,imgDimY,0,GL_RGB,GL_UNSIGNED_BYTE,imageData);
	//	glBindTexture(GL_TEXTURE_2D,0);
	//}

	/*	Clean up after yourself! */
	delete[] imageData;

	return true;
	
}

bool FtvTestbench::loadVectorFieldSequence()
{
	/*	Go for the velo.lst file, as it contains a list of all vectorfields */
	std::vector<std::string> velo;
	std::string buffer;
	std::string::iterator itr0;
	std::string::iterator itr1;

	std::ifstream file ("../resources/textures/fault_tolerant_vis/ftle_vectorfield/velo.lst",std::ios::in);
	if(!( file.is_open() ))return false;

	/*	Go to the beginning of the file and read the first line */
	file.seekg(0, file.beg);
	while(!file.eof())
	{
		std::getline(file,buffer,'\n');

		itr0 = buffer.end();
		itr1 = buffer.end();
		while(*itr0 != '.') itr0--;
		buffer.replace(itr0,itr1,".raw");

		velo.push_back(buffer);
	}

	int imgDimX = 41;
	int imgDimY = 41;
	int size = imgDimX*imgDimY;
	//float* rawImageData = new float[size*3];
	float* rawImageData = NULL;
	float* rearrangedImageData = new float[size*2];
	std::string path;

	/*
	/	Careful with accessing the image arrays here!
	*/
	for(int i = 100; i < 151; i++)
	{
		path = "../resources/textures/fault_tolerant_vis/ftle_vectorfield/";
		path += velo[i];

		int float_array_length;
		readRawImageF(path.c_str(), rawImageData, float_array_length);

		/*	We only need two of its components per texel */
		for(int y=0;y<imgDimY;y++)
		{
			for(int x=0;x<imgDimX;x++)
			{
				rearrangedImageData[y*imgDimX*2 + x*2 + 0] = (rawImageData[(imgDimY-1-y)*imgDimX*3 + x*3 + 0]);
				/*	Since the y-axis is inverted, the vetorfield entries have to be inverted too */
				rearrangedImageData[y*imgDimX*2 + x*2 + 1] = -(rawImageData[(imgDimY-1-y)*imgDimX*3 + x*3 + 1]);
			}
		}

		glGenTextures(1, &textures_v[i-100]);
		glBindTexture(GL_TEXTURE_2D, textures_v[i-100]);
		//glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D,0,GL_RG32F,imgDimX,imgDimY,0,GL_RG,GL_FLOAT,rearrangedImageData);
		glBindTexture(GL_TEXTURE_2D,0);
	}

	delete rawImageData;
	delete rearrangedImageData;

	return true;
}

bool FtvTestbench::loadVectorFieldSequenceTo3D(int from, int to)
{
	if( from > to ) return false;

	/*	Go for the velo.lst file, as it contains a list of all vectorfields */
	std::vector<std::string> velo;
	std::string buffer;
	std::string::iterator itr0;
	std::string::iterator itr1;

	std::ifstream file ("../resources/textures/fault_tolerant_vis/ftle_vectorfield/velo.lst",std::ios::in);
	if(!( file.is_open() ))return false;

	/*	Go to the beginning of the file and read the first line */
	file.seekg(0, file.beg);
	while(!file.eof())
	{
		std::getline(file,buffer,'\n');

		itr0 = buffer.end();
		itr1 = buffer.end();
		while(*itr0 != '.') itr0--;
		buffer.replace(itr0,itr1,".raw");

		velo.push_back(buffer);
	}

	int imgDimX = 41;
	int imgDimY = 41;
	int imgDimZ = to - from;
	int size_of_slice = imgDimX*imgDimY;
	float* rawImageData = NULL;
	/*	enough storage for 50 slices of 2d vectorfield data */
	float* image_data_3d = new float[size_of_slice*2*imgDimZ];
	std::string path;



	/*	Careful with accessing the image arrays here! */
	for(int i = from; i < to+1; i++)
	{
		path = "../resources/textures/fault_tolerant_vis/ftle_vectorfield/" + velo[i];

		int float_array_length;
		readRawImageF(path.c_str(), rawImageData, float_array_length);

		for(int y=0;y<imgDimY;y++)
		{
			for(int x=0;x<imgDimX;x++)
			{
				/*
				/	Save the data into the storage for the 3d texture.
				/	We only need two of its components per texel.
				*/
				int z = i - from;
				image_data_3d[z*imgDimX*imgDimY*2 + y*imgDimX*2 + x*2 + 0] = (rawImageData[(imgDimY-1-y)*imgDimX*3 + x*3 + 0]);
				/*	Since the y-axis is inverted, the vetorfield entries have to be inverted too */
				image_data_3d[z*imgDimX*imgDimY*2 + y*imgDimX*2 + x*2 + 1] = -(rawImageData[(imgDimY-1-y)*imgDimX*3 + x*3 + 1]);

			}
		}
	}

	glGenTextures(1, &vector_fields);
	glBindTexture(GL_TEXTURE_3D, vector_fields);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage3D(GL_TEXTURE_3D,0,GL_RG32F,imgDimX,imgDimY,imgDimZ,0,GL_RG,GL_FLOAT,image_data_3d);
	glBindTexture(GL_TEXTURE_3D,0);

	//delete rawImageData;
	//delete image_data_3d;

	return true;
}

void FtvTestbench::initMasks()
{
	float maskData[] = {0.1f,0.1f,0.3f,0.3f,0.5f,0.4f,0.6f,0.6f};
	glGenTextures(1, &maskConfigB);
	glBindTexture(GL_TEXTURE_1D, maskConfigB);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage1D(GL_TEXTURE_1D,0,GL_RGBA32F,2,0,GL_RGBA,GL_FLOAT,maskData);
	glBindTexture(GL_TEXTURE_1D,0);

	float maskData1[] = {0.1f,0.1f,0.3f,0.3f,
						 0.5f,0.4f,0.7f,0.6f,
						 0.3f,0.7f,0.5f,0.9f,
						 0.7f,0.3f,0.9f,0.5f};
	glGenTextures(1, &maskConfigC_1);
	glBindTexture(GL_TEXTURE_1D, maskConfigC_1);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage1D(GL_TEXTURE_1D,0,GL_RGBA32F,3,0,GL_RGBA,GL_FLOAT,maskData1);
	glBindTexture(GL_TEXTURE_1D,0);

	float maskData2[] = {0.35f,0.2f,0.55f,0.4f,
						 0.75f,0.45f,0.95f,0.65f,
						 0.35f,0.6f,0.55f,0.8f,
						 0.5f,0.7f,0.7f,0.9f};
	glGenTextures(1, &maskConfigC_2);
	glBindTexture(GL_TEXTURE_1D, maskConfigC_2);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage1D(GL_TEXTURE_1D,0,GL_RGBA32F,3,0,GL_RGBA,GL_FLOAT,maskData2);
	glBindTexture(GL_TEXTURE_1D,0);

	float maskData3[] = {0.2f,0.2f,0.3f,0.3f,
						 0.7f,0.2f,0.8f,0.3f,
						 0.2f,0.7f,0.3f,0.8f,
						 0.7f,0.7f,0.8f,0.8f};
	glGenTextures(1, &maskConfigC_3);
	glBindTexture(GL_TEXTURE_1D, maskConfigC_3);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage1D(GL_TEXTURE_1D,0,GL_RGBA32F,4,0,GL_RGBA,GL_FLOAT,maskData3);
	glBindTexture(GL_TEXTURE_1D,0);
}

void FtvTestbench::getForwardTexture(GLuint& handle, int index)
{
	handle = textures_f[index];
}

void FtvTestbench::getBackwardTexture(GLuint& handle, int index)
{
	handle = textures_b[index];
}

void FtvTestbench::getVectorTexture(GLuint& handle, int index)
{
	handle = textures_v[index];
}

void FtvTestbench::getVectorTexture3D(GLuint& handle)
{
	handle = vector_fields;
}

void FtvTestbench::getFrameConfigA(FramebufferObject* maskFbo, FramebufferObject* imgFbo)
{
	imgFbo->bind();
	glViewport(0,0,imgFbo->getWidth(),imgFbo->getHeight());
	imageProcessor.imageToFBO(textures_f[0]);
}

void FtvTestbench::getFrameConfigB(FramebufferObject* maskFbo, FramebufferObject* imgFbo)
{
	if(currentFrame==0)
	{
		imageProcessor.generateFtvMask(maskFbo, 0,2);
	}
	else
	{
		imageProcessor.generateFtvMask(maskFbo, maskConfigB,2);
	}

	imgFbo->bind();
	glViewport(0,0,imgFbo->getWidth(),imgFbo->getHeight());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	imageProcessor.applyMaskToImageToFBO(textures_f[currentFrame],maskFbo,400,400);

	currentFrame = (currentFrame++)%49;
}

void FtvTestbench::getFrameConfigC(FramebufferObject* maskFbo, FramebufferObject* imgFbo)
{
	if(currentFrame==0)
	{
		imageProcessor.generateFtvMask(maskFbo,0,2);
	}
	else
	{
		if((currentFrame%3)==0)
		{
			imageProcessor.generateFtvMask(maskFbo,maskConfigC_1,4);
		}
		else if((currentFrame%3)==1)
		{
			imageProcessor.generateFtvMask(maskFbo,maskConfigC_2,4);
		}
		else if((currentFrame%3)==2)
		{
			imageProcessor.generateFtvMask(maskFbo,maskConfigC_3,4);
		}
	}

	imgFbo->bind();
	glViewport(0,0,imgFbo->getWidth(),imgFbo->getHeight());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	imageProcessor.applyMaskToImageToFBO(textures_f[currentFrame],maskFbo,400,400);

	currentFrame = (currentFrame++)%49;
}

void FtvTestbench::createVolumeMask(char*& volume_data, glm::ivec3 volume_dimension, glm::ivec3 lower_bb_corner, glm::ivec3 upper_bb_corner)
{
	int index = 0;
	glm::vec3 texture_coord = glm::vec3(0.0);

	for (int z = 0; z < volume_dimension.z; z++)
	{
		for (int y = 0; y < volume_dimension.y; y++)
		{
			for (int x = 0; x < volume_dimension.x; x++)
			{
				//texture_coord = glm::vec3( (float(x)/float(volume_dimension.x)),
				//							(float(y) / float(volume_dimension.y)),
				//							(float(z) / float(volume_dimension.z)) );

				//if ((texture_coord.x < lower_bb_corner.x || texture_coord.x > upper_bb_corner.x) &&
				//	(texture_coord.y < lower_bb_corner.y || texture_coord.y > upper_bb_corner.y) &&
				//	(texture_coord.z < lower_bb_corner.z || texture_coord.z > upper_bb_corner.z))
				if ((x >= lower_bb_corner.x && x <= upper_bb_corner.x) &&
					(y >= lower_bb_corner.y && y <= upper_bb_corner.y) &&
					(z >= lower_bb_corner.z && z <= upper_bb_corner.z))
				{
					//	Inside of the fault region.
					volume_data[index] = static_cast<char>(127);
				}
				else
				{
					//	Outside of the fault region.
					volume_data[index] = static_cast<char>(0);
				}

				if (z == 66) volume_data[index] = static_cast<char>(127);

				//if (z == 0 && y == 1)
				//{
				//	std::cout << "x:" << x << std::endl;
				//	std::cout << "index:" << index << std::endl << std::endl;
				//}

				index += 1;
			}
		}
	}
	std::cout << "index:" << index << std::endl << std::endl;
}