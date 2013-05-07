#include "ftvTestbench.h"

bool ftvTestbench::readPpmHeader(const char* filename, long& headerEndPos, int& imgDimX, int& imgDimY)
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

bool ftvTestbench::readPpmData(const char* filename, char* imageData, long dataBegin, int imgDimX, int imgDimY)
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

bool ftvTestbench::loadImageSequence()
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
		path = "../resources/textures/fault_tolerant_vis/ftle/ftle_f_";
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

	/*
	/	Clean up after yourself!
	*/
	delete[] imageData;
}

void ftvTestbench::initMasks()
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

void ftvTestbench::getTexture(GLuint& handle, int index)
{
	handle = textures_f[index];
}

void ftvTestbench::getFrameConfigA(framebufferObject* maskFbo, framebufferObject* imgFbo)
{
	imgFbo->bind();
	glViewport(0,0,imgFbo->getWidth(),imgFbo->getHeight());
	imageProcessor.imageToFBO(textures_f[0]);
}

void ftvTestbench::getFrameConfigB(framebufferObject* maskFbo, framebufferObject* imgFbo)
{
	if(currentFrame==0)
	{
		maskFbo->bind();
		glViewport(0,0,imgFbo->getWidth(),imgFbo->getHeight());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		imageProcessor.generateFtvMask(0,2);
	}
	else
	{
		maskFbo->bind();
		glViewport(0,0,imgFbo->getWidth(),imgFbo->getHeight());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		imageProcessor.generateFtvMask(maskConfigB,2);
	}

	imgFbo->bind();
	glViewport(0,0,imgFbo->getWidth(),imgFbo->getHeight());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	imageProcessor.applyMaskToImageToFBO(textures_f[currentFrame],maskFbo,400,400);

	currentFrame = (currentFrame++)%49;
}

void ftvTestbench::getFrameConfigC(framebufferObject* maskFbo, framebufferObject* imgFbo)
{
	if(currentFrame==0)
	{
		maskFbo->bind();
		glViewport(0,0,imgFbo->getWidth(),imgFbo->getHeight());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		imageProcessor.generateFtvMask(0,2);
	}
	else
	{
		if((currentFrame%3)==0)
		{
			maskFbo->bind();
			glViewport(0,0,imgFbo->getWidth(),imgFbo->getHeight());
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			imageProcessor.generateFtvMask(maskConfigC_1,4);
		}
		else if((currentFrame%3)==1)
		{
			maskFbo->bind();
			glViewport(0,0,imgFbo->getWidth(),imgFbo->getHeight());
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			imageProcessor.generateFtvMask(maskConfigC_2,4);
		}
		else if((currentFrame%3)==2)
		{
			maskFbo->bind();
			glViewport(0,0,imgFbo->getWidth(),imgFbo->getHeight());
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			imageProcessor.generateFtvMask(maskConfigC_3,4);
		}
	}

	imgFbo->bind();
	glViewport(0,0,imgFbo->getWidth(),imgFbo->getHeight());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	imageProcessor.applyMaskToImageToFBO(textures_f[currentFrame],maskFbo,400,400);

	currentFrame = (currentFrame++)%49;
}