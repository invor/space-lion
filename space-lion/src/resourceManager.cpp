#include "resourceManager.h"

resourceManager::resourceManager(){}

resourceManager::~resourceManager(){}

bool resourceManager::createTriangle(vertexGeometry*& inOutGeomPtr)
{
	vertex_pn *vertexArray = new vertex_pn[3];
	GLubyte *indexArray = new GLubyte[3];

	vertexArray[0]=vertex_pn(-0.5f,0.0f,0.0f,1.0f,0.0f,0.0f);
	vertexArray[1]=vertex_pn(0.5f,0.0f,0.0f,0.0f,1.0f,0.0f);
	vertexArray[2]=vertex_pn(0.0f,1.0f,0.0f,0.0f,0.0f,1.0f);

	indexArray[0]=0;indexArray[1]=1;indexArray[2]=2;

	geometryList.push_back(vertexGeometry("0"));
	std::list<vertexGeometry>::iterator lastElement = --(geometryList.end());
	if(!(lastElement->bufferDataFromArray(vertexArray,indexArray,sizeof(vertex_pn)*3,sizeof(GLubyte)*3))) return false;
	lastElement->setVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(vertex_pn),0);
	lastElement->setVertexAttribPointer(3,3,GL_FLOAT,GL_FALSE,sizeof(vertex_pn),(GLvoid*) sizeof(vertex_p));

	inOutGeomPtr = &(*lastElement);
	return true;
}

bool resourceManager::createBox(vertexGeometry*& inOutGeomPtr)
{
	/*	Check list of vertexBufferObjects for default box object(filename="0") */
	for(std::list<vertexGeometry>::iterator i = geometryList.begin(); i != geometryList.end(); ++i)
	{
		if(i->getFilename() == "0"){
			inOutGeomPtr = &(*i);
			return true;
		}
	}

	/*	if default box not already in list, continue here */
	vertex_pntcu *vertexArray = new vertex_pntcu[24];
	GLubyte *indexArray = new GLubyte[36];

	/*	front face */
	vertexArray[0]=vertex_pntcu(-0.5,-0.5,0.5,0.0,0.0,1.0,1.0,0.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0);vertexArray[1]=vertex_pntcu(-0.5,0.5,0.5,0.0,0.0,1.0,1.0,0.0,0.0,0.0,1.0,0.0,1.0,0.0,1.0);
	vertexArray[2]=vertex_pntcu(0.5,0.5,0.5,0.0,0.0,1.0,1.0,0.0,0.0,1.0,1.0,0.0,1.0,1.0,1.0);vertexArray[3]=vertex_pntcu(0.5,-0.5,0.5,0.0,0.0,1.0,1.0,0.0,0.0,1.0,0.0,0.0,1.0,1.0,0.0);
	/*	right face */
	vertexArray[4]=vertex_pntcu(0.5,-0.5,0.5,1.0,0.0,0.0,0.0,0.0,-1.0,1.0,0.0,0.0,1.0,0.0,0.0);vertexArray[5]=vertex_pntcu(0.5,0.5,0.5,1.0,0.0,0.0,0.0,0.0,-1.0,1.0,1.0,0.0,1.0,0.0,1.0);
	vertexArray[6]=vertex_pntcu(0.5,0.5,-0.5,1.0,0.0,0.0,0.0,0.0,-1.0,1.0,1.0,1.0,1.0,1.0,1.0);vertexArray[7]=vertex_pntcu(0.5,-0.5,-0.5,1.0,0.0,0.0,0.0,0.0,-1.0,1.0,0.0,1.0,1.0,1.0,0.0);
	/*	left face */
	vertexArray[8]=vertex_pntcu(-0.5,-0.5,-0.5,-1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,1.0,1.0,0.0,0.0);vertexArray[9]=vertex_pntcu(-0.5,0.5,-0.5,-1.0,0.0,0.0,0.0,0.0,1.0,0.0,1.0,1.0,1.0,0.0,1.0);
	vertexArray[10]=vertex_pntcu(-0.5,0.5,0.5,-1.0,0.0,0.0,0.0,0.0,1.0,0.0,1.0,0.0,1.0,1.0,1.0);vertexArray[11]=vertex_pntcu(-0.5,-0.5,0.5,-1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,1.0,1.0,0.0);
	/*	back face */
	vertexArray[12]=vertex_pntcu(0.5,-0.5,-0.5,0.0,0.0,-1.0,-1.0,0.0,0.0,1.0,0.0,1.0,1.0,0.0,0.0);vertexArray[13]=vertex_pntcu(0.5,0.5,-0.5,0.0,0.0,-1.0,-1.0,0.0,0.0,1.0,1.0,1.0,1.0,0.0,1.0);
	vertexArray[14]=vertex_pntcu(-0.5,0.5,-0.5,0.0,0.0,-1.0,-1.0,0.0,0.0,0.0,1.0,1.0,1.0,1.0,1.0);vertexArray[15]=vertex_pntcu(-0.5,-0.5,-0.5,0.0,0.0,-1.0,-1.0,0.0,0.0,0.0,0.0,1.0,1.0,1.0,0.0);
	/*	bottom face */
	vertexArray[16]=vertex_pntcu(-0.5,-0.5,0.5,0.0,-1.0,0.0,1.0,0.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0);vertexArray[17]=vertex_pntcu(-0.5,-0.5,-0.5,0.0,-1.0,0.0,1.0,0.0,0.0,0.0,0.0,1.0,1.0,0.0,1.0);
	vertexArray[18]=vertex_pntcu(0.5,-0.5,-0.5,0.0,-1.0,0.0,1.0,0.0,0.0,1.0,0,1.0,1.0,1.0,1.0);vertexArray[19]=vertex_pntcu(0.5,-0.5,0.5,0.0,-1.0,0.0,1.0,0.0,0.0,1.0,0,0,1.0,1.0,0.0);
	/*	top face */
	vertexArray[20]=vertex_pntcu(-0.5,0.5,0.5,0.0,1.0,0.0,1.0,0.0,0.0,0,1.0,0.0,1.0,0.0,0.0);vertexArray[21]=vertex_pntcu(-0.5,0.5,-0.5,0.0,1.0,0.0,1.0,0.0,0.0,0.0,1.0,1.0,1.0,0.0,1.0);
	vertexArray[22]=vertex_pntcu(0.5,0.5,-0.5,0.0,1.0,0.0,1.0,0.0,0.0,1.0,1.0,1.0,1.0,1.0,1.0);vertexArray[23]=vertex_pntcu(0.5,0.5,0.5,0.0,1.0,0.0,1.0,0.0,0.0,1.0,1.0,0,1.0,1.0,0.0);

	indexArray[0]=0;indexArray[1]=2;indexArray[2]=1;
	indexArray[3]=2;indexArray[4]=0;indexArray[5]=3;
	indexArray[6]=4;indexArray[7]=6;indexArray[8]=5;
	indexArray[9]=6;indexArray[10]=4;indexArray[11]=7;
	indexArray[12]=8;indexArray[13]=10;indexArray[14]=9;
	indexArray[15]=10;indexArray[16]=8;indexArray[17]=11;
	indexArray[18]=12;indexArray[19]=14;indexArray[20]=13;
	indexArray[21]=14;indexArray[22]=12;indexArray[23]=15;
	indexArray[24]=16;indexArray[25]=17;indexArray[26]=18;
	indexArray[27]=18;indexArray[28]=19;indexArray[29]=16;
	indexArray[30]=20;indexArray[31]=22;indexArray[32]=21;
	indexArray[33]=22;indexArray[34]=20;indexArray[35]=23;

	geometryList.push_back(vertexGeometry("0"));
	std::list<vertexGeometry>::iterator lastElement = --(geometryList.end());
	if(!(lastElement->bufferDataFromArray(vertexArray,indexArray,sizeof(vertex_pntcu)*24,sizeof(GLubyte)*36))) return false;
	lastElement->setVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(vertex_pntcu),0);
	lastElement->setVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(vertex_pntcu),(GLvoid*) sizeof(vertex_p));
	lastElement->setVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,sizeof(vertex_pntcu),(GLvoid*) sizeof(vertex_pn));
	lastElement->setVertexAttribPointer(3,4,GL_UNSIGNED_BYTE,GL_FALSE,sizeof(vertex_pntcu),(GLvoid*) sizeof(vertex_pnt));
	lastElement->setVertexAttribPointer(4,2,GL_FLOAT,GL_FALSE,sizeof(vertex_pntcu),(GLvoid*) sizeof(vertex_pntc));

	inOutGeomPtr = &(*lastElement);
	return true;
}

bool resourceManager::createVertexGeometry(const char * const path, vertexGeometry*& inOutGeomPtr)
{
	return false;
}

bool resourceManager::createMaterial(material*& inOutMtlPtr)
{
	/*	Check list of materials for default material(id=0) */
	for(std::list<material>::iterator i = materialList.begin(); i != materialList.end(); ++i)
	{
		if((i->getId())==0)
		{
			inOutMtlPtr = &*i;
			return true;
		}
	}

	/*	If default material is not already in list, create it and add it to list */

	float* diffuseData = new float[4];
	float* specularData = new float[4];
	float* normalData = new float[4];
	/*	white diffuse texture */
	diffuseData[0]=1.0f; diffuseData[1]=1.0f; diffuseData[2]=1.0f; diffuseData[3]=1.0f;
	/*	dark grey specular texture */
	specularData[0]=0.3f; specularData[1]=0.3f; specularData[2]=0.3f; specularData[3]=1.0f;
	/*	normal pointing upwards */
	normalData[0]=0.0f; normalData[1]=0.0f; normalData[2]=1.0f; normalData[3]=0.0f;
	
	GLSLProgram* prgPtr;
	texture* texPtr1;
	texture* texPtr2;
	texture* texPtr3;
	if(!createShaderProgram(FLAT,prgPtr)) return false;
	if(!createTexture2D(1,1,diffuseData,texPtr1)) return false;
	if(!createTexture2D(1,1,specularData,texPtr2)) return false;
	if(!createTexture2D(1,1,normalData,texPtr3)) return false;
	materialList.push_back(material(0,prgPtr,texPtr1,texPtr2,texPtr3));

	std::list<material>::iterator lastElement = --(materialList.end());
	inOutMtlPtr = &(*lastElement);
	return true;
}

bool resourceManager::createMaterial(const char * const path, material*& inOutMtlPtr)
{
	materialInfo inOutMtlInfo;
	if(!parseMaterial(path,inOutMtlInfo))return false;

	for(std::list<material>::iterator i = materialList.begin(); i != materialList.end(); ++i)
	{
		if((i->getId())==inOutMtlInfo.id)
		{
			inOutMtlPtr = &*i;
			return true;
		}
	}

	GLSLProgram* prgPtr;
	texture* texPtr1;
	texture* texPtr2;
	texture* texPtr3;
	if(!createShaderProgram(PHONG,prgPtr)) return false;
	if(!createTexture2D(inOutMtlInfo.diff_path,texPtr1)) return false;
	if(!createTexture2D(inOutMtlInfo.spec_path,texPtr2)) return false;
	if(!createTexture2D(inOutMtlInfo.normal_path,texPtr3)) return false;
	materialList.push_back(material(inOutMtlInfo.id,prgPtr,texPtr1,texPtr2,texPtr3));

	std::list<material>::iterator lastElement = --(materialList.end());
	inOutMtlPtr = &(*lastElement);
	return true;
}

bool resourceManager::createShaderProgram(shaderType type, GLSLProgram*& inOutPrgPtr)
{
	/*	Check list of shader programs for the shader type */
	for(std::list<GLSLProgram>::iterator i = shaderProgramList.begin(); i != shaderProgramList.end(); ++i)
	{
		if((i->getType())==type){
			inOutPrgPtr = &*i;
			return true;
		}
	}

	GLSLProgram shaderPrg;
	std::string vertSource;
	std::string fragSource;

	switch(type)
	{
	case PHONG : {
		vertSource = readShaderFile("../resources/shaders/v_phong.glsl");
		fragSource = readShaderFile("../resources/shaders/f_phong.glsl");
		shaderPrg.bindAttribLocation(0,"vPosition");
		shaderPrg.bindAttribLocation(1,"vNormal");
		shaderPrg.bindAttribLocation(2,"vTangent");
		shaderPrg.bindAttribLocation(3,"vColour");
		shaderPrg.bindAttribLocation(4,"vUVCoord");
		break; }
	case FLAT : {
		vertSource = readShaderFile("../resources/shaders/v_flat.glsl");
		fragSource = readShaderFile("../resources/shaders/f_flat.glsl");
		shaderPrg.bindAttribLocation(0,"vPosition");
		shaderPrg.bindAttribLocation(1,"vNormal");
		shaderPrg.bindAttribLocation(2,"vTangent");
		shaderPrg.bindAttribLocation(3,"vColour");
		shaderPrg.bindAttribLocation(4,"vUVCoord");
		break; }
	case FTV_POISSON : {
		vertSource = readShaderFile("../resources/shaders/v_genericPostProc.glsl");
		fragSource = readShaderFile("../resources/shaders/f_ftv_poisson.glsl");
		shaderPrg.bindAttribLocation(0,"vPosition");
		shaderPrg.bindAttribLocation(1,"vUVCoord");
		break; }
	case FXAA : {
		vertSource = readShaderFile("../resources/shaders/v_genericPostProc.glsl");
		fragSource = readShaderFile("../resources/shaders/f_fxaa.glsl");
		shaderPrg.bindAttribLocation(0,"vPosition");
		shaderPrg.bindAttribLocation(1,"vUVCoord");
		break; }
	case IDLE : {
		vertSource = readShaderFile("../resources/shaders/v_genericPostProc.glsl");
		fragSource = readShaderFile("../resources/shaders/f_idle.glsl");
		shaderPrg.bindAttribLocation(0,"vPosition");
		shaderPrg.bindAttribLocation(1,"vUVCoord");
		break; }
	case STAMP : {
		vertSource = readShaderFile("../resources/shaders/v_genericPostProc.glsl");
		fragSource = readShaderFile("../resources/shaders/f_stamp.glsl");
		shaderPrg.bindAttribLocation(0,"vPosition");
		shaderPrg.bindAttribLocation(1,"vUVCoord");
		break; }
	case FTV_INPAINTING : {
		vertSource = readShaderFile("../resources/shaders/v_genericPostProc.glsl");
		fragSource = readShaderFile("../resources/shaders/f_ftv_imageInpainting.glsl");
		shaderPrg.bindAttribLocation(0,"vPosition");
		shaderPrg.bindAttribLocation(1,"vUVCoord");
		break; }
	case DISTANCEMAPPING : {
		vertSource = readShaderFile("../resources/shaders/v_genericPostProc.glsl");
		fragSource = readShaderFile("../resources/shaders/f_distanceMapping.glsl");
		shaderPrg.bindAttribLocation(0,"vPosition");
		shaderPrg.bindAttribLocation(1,"vUVCoord");
		break; }
	case FTV_MASK : {
		vertSource = readShaderFile("../resources/shaders/v_genericPostProc.glsl");
		fragSource = readShaderFile("../resources/shaders/f_ftv_mask.glsl");
		shaderPrg.bindAttribLocation(0,"vPosition");
		shaderPrg.bindAttribLocation(1,"vUVCoord");
		shaderPrg.bindFragDataLocation(0,"inpaintingMask");
		shaderPrg.bindFragDataLocation(1,"distanceMap");
		break; }
	case VOLUME_RAYCASTING : {
		vertSource = readShaderFile("../resources/shaders/v_volRen.glsl");
		fragSource = readShaderFile("../resources/shaders/f_volRen.glsl");
		shaderPrg.bindAttribLocation(0,"vPosition");
		shaderPrg.bindAttribLocation(3,"vColour");
		break; }
	case FTV_VOLUME_RAYCASTING : {
		vertSource = readShaderFile("../resources/shaders/v_ftv_volRen.glsl");
		fragSource = readShaderFile("../resources/shaders/f_ftv_volRen.glsl");
		shaderPrg.bindAttribLocation(0,"vPosition");
		shaderPrg.bindAttribLocation(3,"vColour");
		break; }
	case GAUSSIAN : {
		vertSource = readShaderFile("../resources/shaders/v_genericPostProc.glsl");
		fragSource = readShaderFile("../resources/shaders/f_seperatedGaussian.glsl");
		shaderPrg.bindAttribLocation(0,"vPosition");
		shaderPrg.bindAttribLocation(1,"vUVCoord");
		break; }
	case GRADIENT : {
		vertSource = readShaderFile("../resources/shaders/v_genericPostProc.glsl");
		fragSource = readShaderFile("../resources/shaders/f_gradient.glsl");
		shaderPrg.bindAttribLocation(0,"vPosition");
		shaderPrg.bindAttribLocation(1,"vUVCoord");
		break; }
	case COHERENCE : {
		vertSource = readShaderFile("../resources/shaders/v_genericPostProc.glsl");
		fragSource = readShaderFile("../resources/shaders/f_ftv_coherence.glsl");
		shaderPrg.bindAttribLocation(0,"vPosition");
		shaderPrg.bindAttribLocation(1,"vUVCoord");
		break; }
	case FTV_IMPROVED_INPAINTING : {
		vertSource = readShaderFile("../resources/shaders/v_genericPostProc.glsl");
		fragSource = readShaderFile("../resources/shaders/f_ftv_improvedInpainting.glsl");
		shaderPrg.bindAttribLocation(0,"vPosition");
		shaderPrg.bindAttribLocation(1,"vUVCoord");
		break; }
	case HESSE : {
		vertSource = readShaderFile("../resources/shaders/v_genericPostProc.glsl");
		fragSource = readShaderFile("../resources/shaders/f_hesse.glsl");
		shaderPrg.bindAttribLocation(0,"vPosition");
		shaderPrg.bindAttribLocation(1,"vUVCoord");
		break; }
	case FTV_GAUSSIAN : {
		vertSource = readShaderFile("../resources/shaders/v_genericPostProc.glsl");
		fragSource = readShaderFile("../resources/shaders/f_ftv_seperatedGaussian.glsl");
		shaderPrg.bindAttribLocation(0,"vPosition");
		shaderPrg.bindAttribLocation(1,"vUVCoord");
		break; }
	case FTV_MASK_SHRINK : {
		vertSource = readShaderFile("../resources/shaders/v_genericPostProc.glsl");
		fragSource = readShaderFile("../resources/shaders/f_ftv_shrinkMask.glsl");
		shaderPrg.bindAttribLocation(0,"vPosition");
		shaderPrg.bindAttribLocation(1,"vUVCoord");
		shaderPrg.bindFragDataLocation(0,"inpaintingMask");
		shaderPrg.bindFragDataLocation(1,"distanceMap");
		break; }
	default : {
		return false;
		break; }
	}

	if(!shaderPrg.compileShaderFromString(vertSource,GL_VERTEX_SHADER)) return false;
	if(!shaderPrg.compileShaderFromString(fragSource,GL_FRAGMENT_SHADER)) return false;
	if(!shaderPrg.link()) return false;
	std::cout<<shaderPrg.getLog();

	shaderProgramList.push_back(shaderPrg);
	std::list<GLSLProgram>::iterator lastElement = --(shaderProgramList.end());
	inOutPrgPtr = &(*lastElement);
	return true;
}

bool resourceManager::createTexture2D(int dimX, int dimY, float* data, texture*& inOutTexPtr)
{
	textureList.push_back(texture2D());
	std::list<texture2D>::iterator lastElement = --(textureList.end());
	if(!(lastElement->loadArrayF(dimX, dimY, data))) return false;

	inOutTexPtr = &(*lastElement);
	return true;
}

bool resourceManager::createTexture2D(const std::string path, texture*& inOutTexPtr)
{
	for(std::list<texture2D>::iterator i = textureList.begin(); i != textureList.end(); ++i)
	{
		if((i->getFilename())==path)
		{
			inOutTexPtr = &*i;
			return true;
		}
	}

	textureList.push_back(texture2D());
	std::list<texture2D>::iterator lastElement = --(textureList.end());
	if(!(lastElement->loadTextureFile(path))) return false;

	inOutTexPtr = &(*lastElement);
	return true;
}

bool resourceManager::createTexture3D(const std::string path, glm::ivec3 textureRes, texture3D*& inOutTexPtr)
{
	for(std::list<texture3D>::iterator i = volumeList.begin(); i != volumeList.end(); ++i)
	{
		if((i->getFilename())==path)
		{
			inOutTexPtr = &*i;
			return true;
		}
	}

	volumeList.push_back(texture3D());
	std::list<texture3D>::iterator lastElement = --(volumeList.end());
	if(!(lastElement->loadTextureFile(path,textureRes))) return false;

	inOutTexPtr = &(*lastElement);
	return true;
}

bool resourceManager::createTexture3D(float* volumeData, glm::ivec3 textureRes, GLenum internalFormat, GLenum format, texture3D*& inOutTexPtr)
{
	volumeList.push_back(texture3D());
	std::list<texture3D>::iterator lastElement = --(volumeList.end());
	if(!(lastElement->loadArrayF(volumeData,textureRes,internalFormat,format))) return false;

	inOutTexPtr = &(*lastElement);
	return true;
}

bool resourceManager::loadFbxGeometry(const char* const path, vertexGeometry* goemPtr)
{
	return false;
}

bool resourceManager::parseMaterial(const char* const materialPath, materialInfo& inOutMtlInfo)
{
	std::string buffer;
	std::string tempStr;
	std::string::iterator iter1;
	std::string::iterator iter2;

	std::ifstream file;
	file.open(materialPath, std::ifstream::in);

	if( file.is_open() )
	{
		file.seekg(0, std::ifstream::beg);

		std::getline(file,buffer,'\n');
		inOutMtlInfo.id = atoi(buffer.c_str());

		while(!file.eof())
		{
			std::getline(file,buffer,'\n');
			
			iter2 = buffer.begin();
			iter1 = buffer.begin();
			iter1++;iter1++;
			tempStr.assign(iter2,iter1);

			if(tempStr == "td")
			{
				iter2 = (iter1 + 1);
				iter1 = buffer.end();
				tempStr.assign(iter2,iter1);
				inOutMtlInfo.diff_path = new char[tempStr.length()];
				strcpy((inOutMtlInfo.diff_path),tempStr.c_str());
			}
			else if(tempStr == "ts")
			{
				iter2 = (iter1 + 1);
				iter1 = buffer.end();
				tempStr.assign(iter2,iter1);
				inOutMtlInfo.spec_path = new char[tempStr.length()];
				strcpy((inOutMtlInfo.spec_path),tempStr.c_str());
			}
			else if(tempStr == "tn")
			{
				iter2 = (iter1 + 1);
				iter1 = buffer.end();
				tempStr.assign(iter2,iter1);
				inOutMtlInfo.normal_path = new char[tempStr.length()];
				strcpy((inOutMtlInfo.normal_path),tempStr.c_str());
			}
			else
			{
				return false;
			}
		}
	}
	else
	{
		return false;
	}
	return true;
}

const std::string resourceManager::readShaderFile(const char* const path)
{
	std::ifstream inFile( path, std::ios::in );
    if( !inFile ) {
        return false;
    }

	std::ostringstream source;
    while( inFile.good() ) {
        int c = inFile.get();
        if( ! inFile.eof() ) source << (char) c;
    }
    inFile.close();

	return source.str();
}