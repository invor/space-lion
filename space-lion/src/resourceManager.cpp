#include "resourceManager.h"

ResourceManager::ResourceManager(){}

ResourceManager::~ResourceManager(){}

bool ResourceManager::createTriangle(Mesh*& inOutGeomPtr)
{
	Vertex_pn *vertexArray = new Vertex_pn[3];
	GLuint *indexArray = new GLuint[3];

	vertexArray[0]=Vertex_pn(-0.5f,0.0f,0.0f,1.0f,0.0f,0.0f);
	vertexArray[1]=Vertex_pn(0.5f,0.0f,0.0f,0.0f,1.0f,0.0f);
	vertexArray[2]=Vertex_pn(0.0f,1.0f,0.0f,0.0f,0.0f,1.0f);

	indexArray[0]=0;indexArray[1]=1;indexArray[2]=2;

	geometryList.push_back(Mesh("0"));
	std::list<Mesh>::iterator lastElement = --(geometryList.end());
	if(!(lastElement->bufferDataFromArray(vertexArray,indexArray,sizeof(Vertex_pn)*3,sizeof(GLuint)*3))) return false;
	lastElement->setVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex_pn),0);
	lastElement->setVertexAttribPointer(3,3,GL_FLOAT,GL_FALSE,sizeof(Vertex_pn),(GLvoid*) sizeof(Vertex_p));

	inOutGeomPtr = &(*lastElement);
	return true;
}

bool ResourceManager::createBox(Mesh*& inOutGeomPtr)
{
	/*	Check list of vertexBufferObjects for default box object(filename="0") */
	for(std::list<Mesh>::iterator i = geometryList.begin(); i != geometryList.end(); ++i)
	{
		if(i->getFilename() == "0"){
			inOutGeomPtr = &(*i);
			return true;
		}
	}

	/*	if default box not already in list, continue here */
	Vertex_pntcu *vertexArray = new Vertex_pntcu[24];
	GLuint *indexArray = new GLuint[36];

	/*	front face */
	vertexArray[0]=Vertex_pntcu(-0.5,-0.5,0.5,0.0,0.0,1.0,1.0,0.0,0.0,(GLubyte)0.0,(GLubyte)0.0,(GLubyte)0.0,(GLubyte)1.0,0.0,0.0);
	vertexArray[1]=Vertex_pntcu(-0.5,0.5,0.5,0.0,0.0,1.0,1.0,0.0,0.0,(GLubyte)0.0,(GLubyte)1.0,(GLubyte)0.0,(GLubyte)1.0,0.0,1.0);
	vertexArray[2]=Vertex_pntcu(0.5,0.5,0.5,0.0,0.0,1.0,1.0,0.0,0.0,(GLubyte)1.0,(GLubyte)1.0,(GLubyte)0.0,(GLubyte)1.0,1.0,1.0);
	vertexArray[3]=Vertex_pntcu(0.5,-0.5,0.5,0.0,0.0,1.0,1.0,0.0,0.0,(GLubyte)1.0,(GLubyte)0.0,(GLubyte)0.0,(GLubyte)1.0,1.0,0.0);
	/*	right face */
	vertexArray[4]=Vertex_pntcu(0.5,-0.5,0.5,1.0,0.0,0.0,0.0,0.0,-1.0,(GLubyte)1.0,(GLubyte)0.0,(GLubyte)0.0,(GLubyte)1.0,0.0,0.0);
	vertexArray[5]=Vertex_pntcu(0.5,0.5,0.5,1.0,0.0,0.0,0.0,0.0,-1.0,(GLubyte)1.0,(GLubyte)1.0,(GLubyte)0.0,(GLubyte)1.0,0.0,1.0);
	vertexArray[6]=Vertex_pntcu(0.5,0.5,-0.5,1.0,0.0,0.0,0.0,0.0,-1.0,(GLubyte)1.0,(GLubyte)1.0,(GLubyte)1.0,(GLubyte)1.0,1.0,1.0);
	vertexArray[7]=Vertex_pntcu(0.5,-0.5,-0.5,1.0,0.0,0.0,0.0,0.0,-1.0,(GLubyte)1.0,(GLubyte)0.0,(GLubyte)1.0,(GLubyte)1.0,1.0,0.0);
	/*	left face */
	vertexArray[8]=Vertex_pntcu(-0.5,-0.5,-0.5,-1.0,0.0,0.0,0.0,0.0,1.0,(GLubyte)0.0,(GLubyte)0.0,(GLubyte)1.0,(GLubyte)1.0,0.0,0.0);
	vertexArray[9]=Vertex_pntcu(-0.5,0.5,-0.5,-1.0,0.0,0.0,0.0,0.0,1.0,(GLubyte)0.0,(GLubyte)1.0,(GLubyte)1.0,(GLubyte)1.0,0.0,1.0);
	vertexArray[10]=Vertex_pntcu(-0.5,0.5,0.5,-1.0,0.0,0.0,0.0,0.0,1.0,(GLubyte)0.0,(GLubyte)1.0,(GLubyte)0.0,(GLubyte)1.0,1.0,1.0);
	vertexArray[11]=Vertex_pntcu(-0.5,-0.5,0.5,-1.0,0.0,0.0,0.0,0.0,1.0,(GLubyte)0.0,(GLubyte)0.0,(GLubyte)0.0,(GLubyte)1.0,1.0,0.0);
	/*	back face */
	vertexArray[12]=Vertex_pntcu(0.5,-0.5,-0.5,0.0,0.0,-1.0,-1.0,0.0,0.0,(GLubyte)1.0,(GLubyte)0.0,(GLubyte)1.0,(GLubyte)1.0,0.0,0.0);
	vertexArray[13]=Vertex_pntcu(0.5,0.5,-0.5,0.0,0.0,-1.0,-1.0,0.0,0.0,(GLubyte)1.0,(GLubyte)1.0,(GLubyte)1.0,(GLubyte)1.0,0.0,1.0);
	vertexArray[14]=Vertex_pntcu(-0.5,0.5,-0.5,0.0,0.0,-1.0,-1.0,0.0,0.0,(GLubyte)0.0,(GLubyte)1.0,(GLubyte)1.0,(GLubyte)1.0,1.0,1.0);
	vertexArray[15]=Vertex_pntcu(-0.5,-0.5,-0.5,0.0,0.0,-1.0,-1.0,0.0,0.0,(GLubyte)0.0,(GLubyte)0.0,(GLubyte)1.0,(GLubyte)1.0,1.0,0.0);
	/*	bottom face */
	vertexArray[16]=Vertex_pntcu(-0.5,-0.5,0.5,0.0,-1.0,0.0,1.0,0.0,0.0,(GLubyte)0.0,(GLubyte)0.0,(GLubyte)0.0,(GLubyte)1.0,0.0,0.0);
	vertexArray[17]=Vertex_pntcu(-0.5,-0.5,-0.5,0.0,-1.0,0.0,1.0,0.0,0.0,(GLubyte)0.0,(GLubyte)0.0,(GLubyte)1.0,(GLubyte)1.0,0.0,1.0);
	vertexArray[18]=Vertex_pntcu(0.5,-0.5,-0.5,0.0,-1.0,0.0,1.0,0.0,0.0,(GLubyte)1.0,(GLubyte)0.0,(GLubyte)1.0,(GLubyte)1.0,1.0,1.0);
	vertexArray[19]=Vertex_pntcu(0.5,-0.5,0.5,0.0,-1.0,0.0,1.0,0.0,0.0,(GLubyte)1.0,(GLubyte)0.0,(GLubyte)0.0,(GLubyte)1.0,1.0,0.0);
	/*	top face */
	vertexArray[20]=Vertex_pntcu(-0.5,0.5,0.5,0.0,1.0,0.0,1.0,0.0,0.0,(GLubyte)0.0,(GLubyte)1.0,(GLubyte)0.0,(GLubyte)1.0,0.0,0.0);
	vertexArray[21]=Vertex_pntcu(-0.5,0.5,-0.5,0.0,1.0,0.0,1.0,0.0,0.0,(GLubyte)0.0,(GLubyte)1.0,(GLubyte)1.0,(GLubyte)1.0,0.0,1.0);
	vertexArray[22]=Vertex_pntcu(0.5,0.5,-0.5,0.0,1.0,0.0,1.0,0.0,0.0,(GLubyte)1.0,(GLubyte)1.0,(GLubyte)1.0,(GLubyte)1.0,1.0,1.0);
	vertexArray[23]=Vertex_pntcu(0.5,0.5,0.5,0.0,1.0,0.0,1.0,0.0,0.0,(GLubyte)1.0,(GLubyte)1.0,(GLubyte)0.0,(GLubyte)1.0,1.0,0.0);

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

	geometryList.push_back(Mesh("0"));
	std::list<Mesh>::iterator lastElement = --(geometryList.end());
	if(!(lastElement->bufferDataFromArray(vertexArray,indexArray,sizeof(Vertex_pntcu)*24,sizeof(GLuint)*36))) return false;
	lastElement->setVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex_pntcu),0);
	lastElement->setVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex_pntcu),(GLvoid*) sizeof(Vertex_p));
	lastElement->setVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,sizeof(Vertex_pntcu),(GLvoid*) sizeof(Vertex_pn));
	lastElement->setVertexAttribPointer(3,4,GL_UNSIGNED_BYTE,GL_FALSE,sizeof(Vertex_pntcu),(GLvoid*) sizeof(Vertex_pnt));
	lastElement->setVertexAttribPointer(4,2,GL_FLOAT,GL_FALSE,sizeof(Vertex_pntcu),(GLvoid*) sizeof(Vertex_pntc));

	inOutGeomPtr = &(*lastElement);
	return true;
}

bool ResourceManager::createMesh(const std::string path, Mesh*& inOutGeomPtr)
{
	/*	Check list of vertexBufferObjects for filename */
	for(std::list<Mesh>::iterator i = geometryList.begin(); i != geometryList.end(); ++i)
	{
		if(i->getFilename() == path){
			inOutGeomPtr = &(*i);
			return true;
		}
	}

	/*	Check file type before trying to load it */
	std::string file_type;
	std::string::const_iterator itr0 = path.end();
	std::string::const_iterator itr1;
	for(itr1 = path.end(); *itr1 != '.'; --itr1);
	file_type.assign(++itr1,itr0);
	std::cout<<file_type<<std::endl;

	if(file_type == "fbx")
	{
		geometryList.push_back(Mesh(path));
		std::list<Mesh>::iterator lastElement = --(geometryList.end());

		/* Just some testing */
		if( !loadFbxGeometry(path.c_str(),&(*lastElement)) ) {return false;}

		inOutGeomPtr = &(*lastElement);
	}
	else if(file_type == "slraw")
	{
		geometryList.push_back(Mesh(path));
		std::list<Mesh>::iterator lastElement = --(geometryList.end());

		/* Just some testing */
		if( !loadBinaryGeometry(path.c_str(),&(*lastElement)) ) {return false;}

		inOutGeomPtr = &(*lastElement);
	}
	else
	{
		return false;
	}

	return true;
}

bool ResourceManager::createMaterial(Material*& inOutMtlPtr)
{
	/*	Check list of materials for default material(id=0) */
	for(std::list<Material>::iterator i = materialList.begin(); i != materialList.end(); ++i)
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
	normalData[0]=0.5f; normalData[1]=0.5f; normalData[2]=1.0f; normalData[3]=0.0f;
	
	GLSLProgram* prgPtr;
	Texture* texPtr1;
	Texture* texPtr2;
	Texture* texPtr3;
	if(!createShaderProgram(PHONG,prgPtr)) return false;
	if(!createTexture2D(1,1,diffuseData,texPtr1)) return false;
	if(!createTexture2D(1,1,specularData,texPtr2)) return false;
	if(!createTexture2D(1,1,normalData,texPtr3)) return false;
	materialList.push_back(Material(0,prgPtr,texPtr1,texPtr2,texPtr3));

	std::list<Material>::iterator lastElement = --(materialList.end());
	inOutMtlPtr = &(*lastElement);
	return true;
}

bool ResourceManager::createMaterial(const char * const path, Material*& inOutMtlPtr)
{
	MaterialInfo inOutMtlInfo;
	if(!parseMaterial(path,inOutMtlInfo))return false;

	for(std::list<Material>::iterator i = materialList.begin(); i != materialList.end(); ++i)
	{
		if((i->getId())==inOutMtlInfo.id)
		{
			inOutMtlPtr = &*i;
			return true;
		}
	}

	GLSLProgram* prgPtr;
	Texture* texPtr1;
	Texture* texPtr2;
	Texture* texPtr3;
	if(!createShaderProgram(PHONG,prgPtr)) return false;
	if(!createTexture2D(inOutMtlInfo.diff_path,texPtr1)) return false;
	if(!createTexture2D(inOutMtlInfo.spec_path,texPtr2)) return false;
	if(!createTexture2D(inOutMtlInfo.normal_path,texPtr3)) return false;
	materialList.push_back(Material(inOutMtlInfo.id,prgPtr,texPtr1,texPtr2,texPtr3));

	std::list<Material>::iterator lastElement = --(materialList.end());
	inOutMtlPtr = &(*lastElement);
	return true;
}

bool ResourceManager::createShaderProgram(shaderType type, GLSLProgram*& inOutPrgPtr)
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
		shaderPrg.bindAttribLocation(5,"vBitangent");
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
	case STRUCTURE_TENSOR : {
		vertSource = readShaderFile("../resources/shaders/v_genericPostProc.glsl");
		fragSource = readShaderFile("../resources/shaders/f_structureTensor.glsl");
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

	if(!shaderPrg.compileShaderFromString(&vertSource,GL_VERTEX_SHADER)){ std::cout<<shaderPrg.getLog(); return false;}
	if(!shaderPrg.compileShaderFromString(&fragSource,GL_FRAGMENT_SHADER)){ std::cout<<shaderPrg.getLog(); return false;}
	if(!shaderPrg.link()){ std::cout<<shaderPrg.getLog(); return false;}

	shaderProgramList.push_back(shaderPrg);
	std::list<GLSLProgram>::iterator lastElement = --(shaderProgramList.end());
	inOutPrgPtr = &(*lastElement);
	return true;
}

bool ResourceManager::createTexture2D(int dimX, int dimY, float* data, Texture*& inOutTexPtr)
{
	textureList.push_back(Texture2D());
	std::list<Texture2D>::iterator lastElement = --(textureList.end());
	if(!(lastElement->loadArrayF(dimX, dimY, data))) return false;

	inOutTexPtr = &(*lastElement);
	return true;
}

bool ResourceManager::createTexture2D(const std::string path, Texture*& inOutTexPtr)
{
	for(std::list<Texture2D>::iterator i = textureList.begin(); i != textureList.end(); ++i)
	{
		if((i->getFilename())==path)
		{
			inOutTexPtr = &*i;
			return true;
		}
	}

	textureList.push_back(Texture2D());
	std::list<Texture2D>::iterator lastElement = --(textureList.end());
	if(!(lastElement->loadTextureFile(path))) return false;

	inOutTexPtr = &(*lastElement);
	return true;
}

bool ResourceManager::createTexture3D(const std::string path, glm::ivec3 textureRes, Texture3D*& inOutTexPtr)
{
	for(std::list<Texture3D>::iterator i = volumeList.begin(); i != volumeList.end(); ++i)
	{
		if((i->getFilename())==path)
		{
			inOutTexPtr = &*i;
			return true;
		}
	}

	volumeList.push_back(Texture3D());
	std::list<Texture3D>::iterator lastElement = --(volumeList.end());
	if(!(lastElement->loadTextureFile(path,textureRes))) return false;

	inOutTexPtr = &(*lastElement);
	return true;
}

bool ResourceManager::createTexture3D(float* volumeData, glm::ivec3 textureRes, GLenum internalFormat, GLenum format, Texture3D*& inOutTexPtr)
{
	volumeList.push_back(Texture3D());
	std::list<Texture3D>::iterator lastElement = --(volumeList.end());
	if(!(lastElement->loadArrayF(volumeData,textureRes,internalFormat,format))) return false;

	inOutTexPtr = &(*lastElement);
	return true;
}

bool ResourceManager::loadFbxGeometry(const char* const path, Mesh* geomPtr)
{
	/*	Initialize an fbx sdk manager. It handles memory management */
	FbxManager *fbxMngr = FbxManager::Create();

	/*	Create the IO settings object. */
	FbxIOSettings *ios = FbxIOSettings::Create(fbxMngr, IOSROOT);
	fbxMngr->SetIOSettings(ios);

	/*	Create an importer */
	FbxImporter *fbxImprtr = FbxImporter::Create(fbxMngr,""); 

	/*	Intialize the importer with the path to the file */
	if( !fbxImprtr->Initialize(path, -1, fbxMngr->GetIOSettings()) )
	{
		printf("Call to FbxImporter::Initialize() failed.\n"); 
		printf("Error returned: %s\n\n", fbxImprtr->GetStatus().GetErrorString()); 
        return false;
	}

	/*	Create an fbx scene to populate with the imported file */
	FbxScene * fbxScn = FbxScene::Create(fbxMngr,"importScene");
	fbxImprtr->Import(fbxScn);
	fbxImprtr->Destroy();

	/*
	/	Get the root node of the scene and its (first) child.
	/	Should the scene contain more than a single object, I will propably
	/	end up in hell for this...
	*/
	FbxNode* fbxRootNode = fbxScn->GetRootNode();
	FbxNode* fbxChildNode = fbxRootNode->GetChild(0);

	/*
	/	To offer at least some safety, check if the child is a mesh.
	/	However, even then I should pray to someone that the mesh is
	/	triangulated.
	*/
	if( !(fbxChildNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eMesh) ) return false;
	
	FbxMesh* fbxMesh = fbxChildNode->GetMesh();

	const int fbxPolyCount = fbxMesh->GetPolygonCount();

	bool hasNormal = fbxMesh->GetElementNormalCount() > 0;
	bool hasTangent = fbxMesh->GetElementTangentCount() > 0;
	bool hasColor = fbxMesh->GetElementVertexColorCount() > 0;
	bool hasUV = fbxMesh->GetElementUVCount() > 0;

	FbxGeometryElement::EMappingMode fbxNormalMappingMode = FbxGeometryElement::eNone;
	FbxGeometryElement::EMappingMode fbxTangentMappingMode = FbxGeometryElement::eNone;
	FbxGeometryElement::EMappingMode fbxVertexColorMappingMode = FbxGeometryElement::eNone;
    FbxGeometryElement::EMappingMode fbxUVMappingMode = FbxGeometryElement::eNone;
	
	bool allByControlPoint;
	if (hasNormal)
    {
		fbxNormalMappingMode = fbxMesh->GetElementNormal(0)->GetMappingMode();
        if (fbxNormalMappingMode == FbxGeometryElement::eNone) hasNormal = false;
        if (hasNormal && (fbxNormalMappingMode != FbxGeometryElement::eByControlPoint) ) allByControlPoint = false;
    }
	if (hasTangent)
    {
		fbxTangentMappingMode = fbxMesh->GetElementNormal(0)->GetMappingMode();
        if (fbxTangentMappingMode == FbxGeometryElement::eNone) hasTangent = false;
        if (hasTangent && (fbxTangentMappingMode != FbxGeometryElement::eByControlPoint) ) allByControlPoint = false;
    }
	if (hasColor)
    {
		fbxVertexColorMappingMode = fbxMesh->GetElementNormal(0)->GetMappingMode();
        if (fbxVertexColorMappingMode == FbxGeometryElement::eNone) hasColor = false;
        if (hasColor && (fbxVertexColorMappingMode != FbxGeometryElement::eByControlPoint) ) allByControlPoint = false;
    }
	if (hasUV)
    {
		fbxUVMappingMode = fbxMesh->GetElementNormal(0)->GetMappingMode();
        if (fbxUVMappingMode == FbxGeometryElement::eNone) hasUV = false;
        if (hasUV && (fbxUVMappingMode != FbxGeometryElement::eByControlPoint) ) allByControlPoint = false;
    }

	int vertexCount = fbxMesh->GetControlPointsCount();
	/*	Triangles are assumed, meaning three vertices per polygon */
	if(!allByControlPoint) vertexCount = fbxPolyCount * 3;
	std::cout<<"Vertex count: "<<vertexCount<<"\n";

	/*	For reasons of simplicity I use the "full" vertex format in any case for now */
	Vertex_pntcub *vertices = new Vertex_pntcub[vertexCount];
	unsigned int *indices = new unsigned int[fbxPolyCount * 3];

	float *uvs = NULL;
	FbxStringList uvNames;
	fbxMesh->GetUVSetNames(uvNames);
	const char *uvName = NULL;
	if(hasUV && uvNames.GetCount())
	{
		uvs = new float[vertexCount *2];
		uvName = uvNames[0];
	}

	/*	Temporary storage for the vertex attributes */
	const FbxVector4 * controlPoints = fbxMesh->GetControlPoints();
	FbxVector4 currentVertex;
	FbxVector4 currentNormal;
	FbxVector4 currentTangent;
	FbxVector4 currentBitangent;
	FbxColor currentColor;
	FbxVector2 currentUV;

	/*	Now read the vertex attributes */
	if (allByControlPoint)
    {
        const FbxGeometryElementNormal *fbxNormalElement = NULL;
		const FbxGeometryElementTangent *fbxTangentElement = NULL;
		const FbxGeometryElementVertexColor * fbxVertexColorElement = NULL;
        const FbxGeometryElementUV *fbxUVElement = NULL;

        if (hasNormal) fbxNormalElement = fbxMesh->GetElementNormal(0);
		if (hasTangent) fbxTangentElement = fbxMesh->GetElementTangent(0);
		if (hasColor) fbxVertexColorElement = fbxMesh->GetElementVertexColor(0);
        if (hasUV) fbxUVElement = fbxMesh->GetElementUV(0);

        for (int index = 0; index < vertexCount; ++index)
        {
            /* Save the vertex position */
            currentVertex = controlPoints[index];

			vertices[index].x = static_cast<float>(currentVertex[0]);
            vertices[index].y = static_cast<float>(currentVertex[1]);
            vertices[index].z = static_cast<float>(currentVertex[2]);

			#if DEBUG_OUTPUT == 1
				std::cout<<"----------------------------------------------------------------------\n";
				std::cout<<"Vertex#"<<index<<" Position: "<<vertices[index].x<<" "<<vertices[index].y<<" "<<vertices[index].z<<"\n";
			#endif

            /* Save the normal */
            if (hasNormal)
            {
                int normalIndex = index;
                if (fbxNormalElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
                {
                    normalIndex = fbxNormalElement->GetIndexArray().GetAt(index);
                }
                currentNormal = fbxNormalElement->GetDirectArray().GetAt(normalIndex);
				vertices[index].nx = static_cast<float>(currentNormal[0]);
				vertices[index].ny = static_cast<float>(currentNormal[1]);
				vertices[index].nz = static_cast<float>(currentNormal[2]);

				#if DEBUG_OUTPUT == 1
					std::cout<<"Vertex#"<<index<<" Normal: "<<vertices[index].nx<<" "<<vertices[index].ny<<" "<<vertices[index].nz<<"\n";
				#endif
            }

			/* Save the tangent */
			if (hasTangent)
			{
				int tangentIndex = index;
				if (fbxTangentElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
                {
                    tangentIndex = fbxTangentElement->GetIndexArray().GetAt(index);
                }
				currentTangent = fbxTangentElement->GetDirectArray().GetAt(tangentIndex);
				vertices[index].tx = static_cast<float>(currentTangent[0]);
				vertices[index].ty = static_cast<float>(currentTangent[1]);
				vertices[index].tz = static_cast<float>(currentTangent[2]);

				#if DEBUG_OUTPUT == 1
					std::cout<<"Vertex#"<<index<<" Tangent: "<<vertices[index].tx<<" "<<vertices[index].ty<<" "<<vertices[index].tz<<"\n";
				#endif
			}

			/* Save the color */
			if (hasColor)
			{
				int vertexColorIndex = index;
				if (fbxVertexColorElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
                {
                    vertexColorIndex = fbxVertexColorElement->GetIndexArray().GetAt(index);
                }
				currentColor = fbxVertexColorElement->GetDirectArray().GetAt(vertexColorIndex);
				vertices[index].r = static_cast<GLubyte>(currentColor[0]);
				vertices[index].g = static_cast<GLubyte>(currentColor[1]);
				vertices[index].b = static_cast<GLubyte>(currentColor[2]);
				vertices[index].a = static_cast<GLubyte>(currentColor[3]);

				#if DEBUG_OUTPUT == 1
					std::cout<<"Vertex#"<<index<<" Color: "<<vertices[index].r<<" "<<vertices[index].g<<" "<<vertices[index].b<<vertices[index].a<<"\n";
				#endif
			}

            /* Save the UV */
            if (hasUV)
            {
                int uVIndex = index;
                if (fbxUVElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
                {
                    uVIndex = fbxUVElement->GetIndexArray().GetAt(index);
                }
                currentUV = fbxUVElement->GetDirectArray().GetAt(uVIndex);
				vertices[index].u = static_cast<float>(currentUV[0]);
				vertices[index].v = static_cast<float>(currentUV[1]);
				
				#if DEBUG_OUTPUT == 1
					std::cout<<"Vertex#"<<index<<" UV: "<<vertices[index].u<<" "<<vertices[index].v<<"\n";
					std::cout<<"----------------------------------------------------------------------\n";
				#endif
            }
        }
    }

	//std::cout<<"Filled vertex buffer.\n";

	int vertexCounter = 0;

	const FbxGeometryElementTangent *fbxTangentElement = NULL;
	const FbxGeometryElementBinormal *fbxBitangentElement = NULL;
	const FbxGeometryElementVertexColor * fbxVertexColorElement = NULL;

	if (hasTangent) fbxTangentElement = fbxMesh->GetElementTangent(0);
	fbxBitangentElement = fbxMesh->GetElementBinormal(0);
	if (hasColor) fbxVertexColorElement = fbxMesh->GetElementVertexColor(0);

	for(int polyIndex = 0; polyIndex < fbxPolyCount; ++polyIndex)
	{
		int indexOffset = polyIndex * 3;

		#if DEBUG_OUTPUT == 1
					std::cout<<"****************\n";
					std::cout<<"Polygon#"<<polyIndex<<"\n"; 
					std::cout<<"****************\n";
		#endif

		for(int vertIndex = 0; vertIndex < 3; ++vertIndex)
		{
			const int controlPointIndex = fbxMesh->GetPolygonVertex(polyIndex, vertIndex);

			if(allByControlPoint)
			{
				indices[indexOffset+vertIndex] = static_cast<unsigned int>(controlPointIndex);
			}
			else
			{
				indices[indexOffset+vertIndex] = static_cast<unsigned int>(vertexCounter);

				currentVertex = controlPoints[controlPointIndex];
				vertices[vertexCounter].x = static_cast<float>(currentVertex[0]);
				vertices[vertexCounter].y = static_cast<float>(currentVertex[1]);
				vertices[vertexCounter].z = static_cast<float>(currentVertex[2]);

				#if DEBUG_OUTPUT == 1
					std::cout<<"----------------------------------------------------------------------\n";
					std::cout<<"Vertex#"<<vertexCounter<<" Position: "<<vertices[vertexCounter].x<<" "<<vertices[vertexCounter].y<<" "<<vertices[vertexCounter].z<<"\n"; 
				#endif

				/* Save the normal */
				if (hasNormal)
				{
					fbxMesh->GetPolygonVertexNormal(polyIndex,vertIndex,currentNormal);
					vertices[vertexCounter].nx = static_cast<float>(currentNormal[0]);
					vertices[vertexCounter].ny = static_cast<float>(currentNormal[1]);
					vertices[vertexCounter].nz = static_cast<float>(currentNormal[2]);

					#if DEBUG_OUTPUT == 1
						std::cout<<"Vertex#"<<vertexCounter<<" Normal: "<<vertices[vertexCounter].nx<<" "<<vertices[vertexCounter].ny<<" "<<vertices[vertexCounter].nz<<"\n";
					#endif
				}

				/* Save the tangent */
				if (hasTangent)
				{
					int tangentIndex = vertexCounter;
					if (fbxTangentElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
				    {
				      tangentIndex = fbxTangentElement->GetIndexArray().GetAt(vertexCounter);
				    }
					currentTangent = fbxTangentElement->GetDirectArray().GetAt(tangentIndex);
					vertices[vertexCounter].tx = static_cast<float>(currentTangent[0]);
					vertices[vertexCounter].ty = static_cast<float>(currentTangent[1]);
					vertices[vertexCounter].tz = static_cast<float>(currentTangent[2]);

					#if DEBUG_OUTPUT == 1
						std::cout<<"Vertex#"<<vertexCounter<<" Tangent: "<<vertices[vertexCounter].tx<<" "<<vertices[vertexCounter].ty<<" "<<vertices[vertexCounter].tz<<"\n";
					#endif
				}

				/* Save the bitangent/binormal */
				if (true)
				{
					int bitangentIndex = vertexCounter;
					if (fbxBitangentElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
				    {
				      bitangentIndex = fbxBitangentElement->GetIndexArray().GetAt(vertexCounter);
				    }
					currentTangent = fbxBitangentElement->GetDirectArray().GetAt(bitangentIndex);
					vertices[vertexCounter].bx = static_cast<float>(currentTangent[0]);
					vertices[vertexCounter].by = static_cast<float>(currentTangent[1]);
					vertices[vertexCounter].bz = static_cast<float>(currentTangent[2]);

					#if DEBUG_OUTPUT == 1
						std::cout<<"Vertex#"<<vertexCounter<<" Bitangent: "<<vertices[vertexCounter].bx<<" "<<vertices[vertexCounter].by<<" "<<vertices[vertexCounter].bz<<"\n";
					#endif
				}

				/* Save the color */
				if (hasColor)
				{
					int vertexColorIndex = vertexCounter;
					if (fbxVertexColorElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
				    {
				        vertexColorIndex = fbxVertexColorElement->GetIndexArray().GetAt(vertexCounter);
				    }
					currentColor = fbxVertexColorElement->GetDirectArray().GetAt(vertexColorIndex);
					vertices[vertexCounter].r = static_cast<GLubyte>(currentColor[0]);
					vertices[vertexCounter].g = static_cast<GLubyte>(currentColor[1]);
					vertices[vertexCounter].b = static_cast<GLubyte>(currentColor[2]);
					vertices[vertexCounter].a = static_cast<GLubyte>(currentColor[3]);

					#if DEBUG_OUTPUT == 1
						std::cout<<"Vertex#"<<vertexCounter<<" Color: "<<vertices[vertexCounter].r<<" "<<vertices[vertexCounter].g<<" "<<vertices[vertexCounter].b<<vertices[vertexCounter].a<<"\n";
					#endif
				}
				else
				{
					vertices[vertexCounter].r = static_cast<GLubyte>(1.0);
					vertices[vertexCounter].g = static_cast<GLubyte>(1.0);
					vertices[vertexCounter].b = static_cast<GLubyte>(1.0);
					vertices[vertexCounter].a = static_cast<GLubyte>(1.0);
				}

				/* Save the UV */
				if (hasUV)
				{
					bool unmappedUV;
					fbxMesh->GetPolygonVertexUV(polyIndex,vertIndex,uvName,currentUV,unmappedUV);
					vertices[vertexCounter].u = static_cast<float>(currentUV[0]);
					vertices[vertexCounter].v = static_cast<float>(currentUV[1]);

					#if DEBUG_OUTPUT == 1
						std::cout<<"Vertex#"<<vertexCounter<<" UV: "<<vertices[vertexCounter].u<<" "<<vertices[vertexCounter].v<<"\n";
						std::cout<<"----------------------------------------------------------------------\n";
					#endif
				}
			}
			++vertexCounter;
		}
	}

	//std::cout<<"Filled index buffer.\n";

	if( !geomPtr->bufferDataFromArray(vertices,indices,sizeof(Vertex_pntcub)*vertexCount,sizeof(unsigned int)*(fbxPolyCount * 3)) ) return false;

	geomPtr->setVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex_pntcub),0);
	geomPtr->setVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex_pntcub),(GLvoid*) sizeof(Vertex_p));
	geomPtr->setVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,sizeof(Vertex_pntcub),(GLvoid*) sizeof(Vertex_pn));
	geomPtr->setVertexAttribPointer(3,4,GL_UNSIGNED_BYTE,GL_FALSE,sizeof(Vertex_pntcub),(GLvoid*) sizeof(Vertex_pnt));
	geomPtr->setVertexAttribPointer(4,2,GL_FLOAT,GL_FALSE,sizeof(Vertex_pntcub),(GLvoid*) sizeof(Vertex_pntc));
	geomPtr->setVertexAttribPointer(5,3,GL_FLOAT,GL_FALSE,sizeof(Vertex_pntcub),(GLvoid*) sizeof(Vertex_pntcu));

	return true;
}

bool ResourceManager::loadBinaryGeometry(const std::string path, Mesh* geomPtr)
{
	std::ifstream dat_file (path,std::ios::in | std::ios::binary);

	/*	Check if the file could be opened */
	if(!( dat_file.is_open() ))return false;

	/*	Parse the dat file */
	std::string buffer;

	std::getline(dat_file,buffer,'\n');
	int num_indices = atoi(buffer.c_str());
	std::getline(dat_file,buffer,'\n');
	int num_vertices = atoi(buffer.c_str());
	std::getline(dat_file,buffer,'\n');
	std::string vertex_type = buffer;

	dat_file.close();

	
	/*	Create path to the index raw file */
	std::string path_without_filename;
	std::string::const_iterator itr0 = path.begin();
	std::string::const_iterator itr1;
	for(itr1 = path.end(); *itr1 != '.'; --itr1);
	path_without_filename.assign(itr0,itr1);
	std::string path_to_iraw = path_without_filename + ".sliraw";

	/*	Load the index raw file */
	unsigned int *indices = new unsigned int[num_indices];

	FILE * iraw_file;

	iraw_file = fopen (path_to_iraw.c_str(), "rb");
	if (iraw_file==NULL) return false;

	fread(indices,sizeof(unsigned int),num_indices,iraw_file);

	
	/*	Create path to the vertex raw file */
	std::string path_to_vraw = path_without_filename + ".slvraw";

	/*	Load the vertex raw file */
	if(vertex_type == "vertex_pntcub")
	{
		Vertex_pntcub *vertices = new Vertex_pntcub[num_vertices];

		FILE * vraw_file;
		vraw_file = fopen (path_to_vraw.c_str(), "rb");
		if (vraw_file==NULL) return false;

		fread(vertices,sizeof(Vertex_pntcub),num_vertices,vraw_file);

		if( !geomPtr->bufferDataFromArray(vertices,indices,sizeof(Vertex_pntcub)*num_vertices,sizeof(unsigned int)*num_indices) ) return false;
	}
	else
	{
		return false;
	}

	geomPtr->setVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex_pntcub),0);
	geomPtr->setVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex_pntcub),(GLvoid*) sizeof(Vertex_p));
	geomPtr->setVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,sizeof(Vertex_pntcub),(GLvoid*) sizeof(Vertex_pn));
	geomPtr->setVertexAttribPointer(3,4,GL_UNSIGNED_BYTE,GL_FALSE,sizeof(Vertex_pntcub),(GLvoid*) sizeof(Vertex_pnt));
	geomPtr->setVertexAttribPointer(4,2,GL_FLOAT,GL_FALSE,sizeof(Vertex_pntcub),(GLvoid*) sizeof(Vertex_pntc));
	geomPtr->setVertexAttribPointer(5,3,GL_FLOAT,GL_FALSE,sizeof(Vertex_pntcub),(GLvoid*) sizeof(Vertex_pntcu));

	return false;
}

bool ResourceManager::parseMaterial(const char* const materialPath, MaterialInfo& inOutMtlInfo)
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

const std::string ResourceManager::readShaderFile(const char* const path)
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