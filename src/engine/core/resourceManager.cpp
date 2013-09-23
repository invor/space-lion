#include "resourceManager.h"

ResourceManager::ResourceManager(){}

ResourceManager::~ResourceManager(){}


void ResourceManager::clearLists()
{
	geometry_list.clear();
	material_list.clear();
	texture_list.clear();
	volume_list.clear();
	shader_program_list.clear();
}

bool ResourceManager::createTriangle(std::shared_ptr<Mesh> &inOutGeomPtr)
{
	Vertex_pn *vertexArray = new Vertex_pn[3];
	GLuint *indexArray = new GLuint[3];

	vertexArray[0]=Vertex_pn(-0.5f,0.0f,0.0f,1.0f,0.0f,0.0f);
	vertexArray[1]=Vertex_pn(0.5f,0.0f,0.0f,0.0f,1.0f,0.0f);
	vertexArray[2]=Vertex_pn(0.0f,1.0f,0.0f,0.0f,0.0f,1.0f);

	indexArray[0]=0;indexArray[1]=1;indexArray[2]=2;

	std::shared_ptr<Mesh> triangle_mesh(new Mesh("0"));
	
	if(!(triangle_mesh->bufferDataFromArray(vertexArray,indexArray,sizeof(Vertex_pn)*3,sizeof(GLuint)*3,GL_TRIANGLES))) return false;
	triangle_mesh->setVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex_pn),0);
	triangle_mesh->setVertexAttribPointer(3,3,GL_FLOAT,GL_FALSE,sizeof(Vertex_pn),(GLvoid*) sizeof(Vertex_p));

	inOutGeomPtr = triangle_mesh;
	geometry_list.push_back(std::move(triangle_mesh));

	return true;
}

bool ResourceManager::createBox(std::shared_ptr<Mesh> &inOutGeomPtr)
{
	/*	Check list of vertexBufferObjects for default box object(filename="0") */
	for(std::list<std::shared_ptr<Mesh>>::iterator i = geometry_list.begin(); i != geometry_list.end(); ++i)
	{
		if((*i)->getFilename() == "0"){
			inOutGeomPtr = (*i);
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

	std::shared_ptr<Mesh> box_mesh(new Mesh("0"));
	if(!(box_mesh->bufferDataFromArray(vertexArray,indexArray,sizeof(Vertex_pntcu)*24,sizeof(GLuint)*36,GL_TRIANGLES))) return false;
	box_mesh->setVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex_pntcu),0);
	box_mesh->setVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex_pntcu),(GLvoid*) sizeof(Vertex_p));
	box_mesh->setVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,sizeof(Vertex_pntcu),(GLvoid*) sizeof(Vertex_pn));
	box_mesh->setVertexAttribPointer(3,4,GL_UNSIGNED_BYTE,GL_FALSE,sizeof(Vertex_pntcu),(GLvoid*) sizeof(Vertex_pnt));
	box_mesh->setVertexAttribPointer(4,2,GL_FLOAT,GL_FALSE,sizeof(Vertex_pntcu),(GLvoid*) sizeof(Vertex_pntc));

	inOutGeomPtr = box_mesh;
	geometry_list.push_back(std::move(box_mesh));
	return true;
}

bool ResourceManager::createMesh(const std::string path, std::shared_ptr<Mesh> &inOutGeomPtr)
{
	/*	Check list of vertexBufferObjects for filename */
	for(std::list<std::shared_ptr<Mesh>>::iterator i = geometry_list.begin(); i != geometry_list.end(); ++i)
	{
		if((*i)->getFilename() == path){
			inOutGeomPtr = (*i);
			return true;
		}
	}

	/*	Check file type before trying to load it */
	std::string file_type;
	std::size_t found = path.rfind('.');
	if (found != std::string::npos) file_type = path.substr(found + 1);

	if(file_type == "fbx")
	{
		/* Just some testing */
		if( !loadFbxGeometry(path, inOutGeomPtr) ) {return false;}

		geometry_list.push_back(inOutGeomPtr);
	}
	else if(file_type == "slraw")
	{
		/* Just some testing */
		if( !loadBinaryGeometry(path, inOutGeomPtr) ) {return false;}

		geometry_list.push_back(inOutGeomPtr);
	}
	else
	{
		return false;
	}

	return true;
}

bool ResourceManager::createMaterial(std::shared_ptr<Material> &inOutMtlPtr)
{
	/*	Check list of materials for default material(id=0) */
	for(std::list<std::shared_ptr<Material>>::iterator i = material_list.begin(); i != material_list.end(); ++i)
	{
		if( (*i)->getId() == 0 )
		{
			inOutMtlPtr = (*i);
			return true;
		}
	}

	/*	If default material is not already in list, create it and add it to list */

	float* diffuseData = new float[4];
	float* specularData = new float[4];
	float* roughnessData = new float[4];
	float* normalData = new float[4];
	/*	white diffuse texture */
	diffuseData[0]=1.0f; diffuseData[1]=1.0f; diffuseData[2]=1.0f; diffuseData[3]=1.0f;
	/*	dark grey specular texture */
	specularData[0]=0.3f; specularData[1]=0.3f; specularData[2]=0.3f; specularData[3]=1.0f;
	/*	dark grey roughness texture */
	roughnessData[0]=0.3f; roughnessData[1]=0.3f; roughnessData[2]=0.3f; roughnessData[3]=1.0f;
	/*	normal pointing upwards */
	normalData[0]=0.5f; normalData[1]=0.5f; normalData[2]=1.0f; normalData[3]=0.0f;
	
	std::shared_ptr<GLSLProgram> prgPtr;
	std::shared_ptr<Texture> texPtr1;
	std::shared_ptr<Texture> texPtr2;
	std::shared_ptr<Texture> texPtr3;
	std::shared_ptr<Texture> texPtr4;
	if(!createShaderProgram(SURFACE_LIGHTING,prgPtr)) return false;
	if(!createTexture2D(1,1,diffuseData,texPtr1)) return false;
	if(!createTexture2D(1,1,specularData,texPtr2)) return false;
	if(!createTexture2D(1,1,roughnessData,texPtr3)) return false;
	if(!createTexture2D(1,1,normalData,texPtr4)) return false;

	std::shared_ptr<Material> material(new Material(0,prgPtr,texPtr1,texPtr2,texPtr3,texPtr4));
	inOutMtlPtr = material;
	material_list.push_back(std::move(material));

	prgPtr.reset();
	texPtr1.reset();
	texPtr2.reset();
	texPtr3.reset();
	texPtr4.reset();
	return true;
}

bool ResourceManager::createMaterial(const char * const path, std::shared_ptr<Material> &inOutMtlPtr)
{
	MaterialInfo inOutMtlInfo;
	if(!parseMaterial(path,inOutMtlInfo))return false;

	for(std::list<std::shared_ptr<Material>>::iterator i = material_list.begin(); i != material_list.end(); ++i)
	{
		if( (*i)->getId() == inOutMtlInfo.id )
		{
			inOutMtlPtr = (*i);
			return true;
		}
	}

	std::shared_ptr<GLSLProgram> prgPtr;
	std::shared_ptr<Texture> texPtr1;
	std::shared_ptr<Texture> texPtr2;
	std::shared_ptr<Texture> texPtr3;
	std::shared_ptr<Texture> texPtr4;
	if(!createShaderProgram(SURFACE_LIGHTING,prgPtr)) return false;
	if(!createTexture2D(inOutMtlInfo.diff_path,texPtr1)) return false;
	if(!createTexture2D(inOutMtlInfo.spec_path,texPtr2)) return false;
	if(!createTexture2D(inOutMtlInfo.roughness_path,texPtr3)) return false;
	if(!createTexture2D(inOutMtlInfo.normal_path,texPtr4)) return false;

	std::shared_ptr<Material> material(new Material(inOutMtlInfo.id,prgPtr,texPtr1,texPtr2,texPtr3,texPtr4));
	inOutMtlPtr = material;
	material_list.push_back(std::move(material));

	prgPtr.reset();
	texPtr1.reset();
	texPtr2.reset();
	texPtr3.reset();
	texPtr4.reset();
	return true;
}

bool ResourceManager::createShaderProgram(shaderType type, std::shared_ptr<GLSLProgram> &inOutPrgPtr)
{
	/*	Check list of shader programs for the shader type */
	for(std::list<std::shared_ptr<GLSLProgram>>::iterator i = shader_program_list.begin(); i != shader_program_list.end(); ++i)
	{
		if(((*i)->getType())==type){
			inOutPrgPtr = (*i);
			return true;
		}
	}

	std::shared_ptr<GLSLProgram> shaderPrg(new GLSLProgram());
	shaderPrg->init();
	std::string vertSource;
	std::string fragSource;

	switch(type)
	{
	case SURFACE_LIGHTING : {
		vertSource = readShaderFile("../resources/shaders/surface_lighting_v.glsl");
		fragSource = readShaderFile("../resources/shaders/surface_lighting_f.glsl");
		shaderPrg->bindAttribLocation(0,"v_position");
		shaderPrg->bindAttribLocation(1,"v_normal");
		shaderPrg->bindAttribLocation(2,"v_tangent");
		shaderPrg->bindAttribLocation(3,"v_colour");
		shaderPrg->bindAttribLocation(4,"v_uv_coord");
		shaderPrg->bindAttribLocation(5,"v_bitangent");
		break; }
	case PICKING: {
		vertSource = readShaderFile("../resources/shaders/picking_v.glsl");
		fragSource = readShaderFile("../resources/shaders/picking_f.glsl");
		shaderPrg->bindAttribLocation(0, "v_position");
		shaderPrg->bindFragDataLocation(0, "frag_colour");
		break; }
	case FLAT : {
		vertSource = readShaderFile("../resources/shaders/v_flat.glsl");
		fragSource = readShaderFile("../resources/shaders/f_flat.glsl");
		shaderPrg->bindAttribLocation(0,"vPosition");
		shaderPrg->bindAttribLocation(1,"vNormal");
		shaderPrg->bindAttribLocation(2,"vTangent");
		shaderPrg->bindAttribLocation(3,"vColour");
		shaderPrg->bindAttribLocation(4,"vUVCoord");
		break; }
	case FXAA : {
		vertSource = readShaderFile("../resources/shaders/v_genericPostProc.glsl");
		fragSource = readShaderFile("../resources/shaders/f_fxaa.glsl");
		shaderPrg->bindAttribLocation(0,"vPosition");
		shaderPrg->bindAttribLocation(1,"vUVCoord");
		break; }
	case IDLE : {
		vertSource = readShaderFile("../resources/shaders/v_genericPostProc.glsl");
		fragSource = readShaderFile("../resources/shaders/f_idle.glsl");
		shaderPrg->bindAttribLocation(0,"vPosition");
		shaderPrg->bindAttribLocation(1,"vUVCoord");
		break; }
	case VOLUME_RAYCASTING : {
		vertSource = readShaderFile("../resources/shaders/v_volRen.glsl");
		fragSource = readShaderFile("../resources/shaders/f_volRen.glsl");
		shaderPrg->bindAttribLocation(0,"vPosition");
		shaderPrg->bindAttribLocation(3,"vColour");
		break; }
	case GAUSSIAN : {
		vertSource = readShaderFile("../resources/shaders/v_genericPostProc.glsl");
		fragSource = readShaderFile("../resources/shaders/f_seperatedGaussian.glsl");
		shaderPrg->bindAttribLocation(0,"vPosition");
		shaderPrg->bindAttribLocation(1,"vUVCoord");
		break; }
	case GRADIENT : {
		vertSource = readShaderFile("../resources/shaders/v_genericPostProc.glsl");
		fragSource = readShaderFile("../resources/shaders/f_gradient.glsl");
		shaderPrg->bindAttribLocation(0,"vPosition");
		shaderPrg->bindAttribLocation(1,"vUVCoord");
		break; }
	case STRUCTURE_TENSOR : {
		vertSource = readShaderFile("../resources/shaders/v_genericPostProc.glsl");
		fragSource = readShaderFile("../resources/shaders/f_structureTensor.glsl");
		shaderPrg->bindAttribLocation(0,"vPosition");
		shaderPrg->bindAttribLocation(1,"vUVCoord");
		break; }
	case HESSE : {
		vertSource = readShaderFile("../resources/shaders/v_genericPostProc.glsl");
		fragSource = readShaderFile("../resources/shaders/f_hesse.glsl");
		shaderPrg->bindAttribLocation(0,"vPosition");
		shaderPrg->bindAttribLocation(1,"vUVCoord");
		break; }
	default : {
		return false;
		break; }
	}

	if(!shaderPrg->compileShaderFromString(&vertSource,GL_VERTEX_SHADER)){ std::cout<<shaderPrg->getLog(); return false;}
	if(!shaderPrg->compileShaderFromString(&fragSource,GL_FRAGMENT_SHADER)){ std::cout<<shaderPrg->getLog(); return false;}
	if(!shaderPrg->link()){ std::cout<<shaderPrg->getLog(); return false;}

	inOutPrgPtr = shaderPrg;
	shader_program_list.push_back(std::move(shaderPrg));

	return true;
}

bool ResourceManager::createTexture2D(int dimX, int dimY, float* data, std::shared_ptr<Texture> &inOutTexPtr)
{
	std::shared_ptr<Texture2D> texture(new Texture2D());
	if(!(texture->load(GL_RGB,dimX, dimY,GL_RGB,GL_FLOAT,data))) return false;
	inOutTexPtr = texture;
	texture_list.push_back(std::move(texture));

	return true;
}

bool ResourceManager::createTexture2D(const std::string path, std::shared_ptr<Texture> &inOutTexPtr)
{
	for(std::list<std::shared_ptr<Texture2D>>::iterator i = texture_list.begin(); i != texture_list.end(); ++i)
	{
		if(((*i)->getFilename())==path)
		{
			inOutTexPtr = (*i);
			return true;
		}
	}

	char* imageData;
	long dataBegin;
	int imgDimX;
	int imgDimY;

	if(!readPpmHeader(path.c_str(),dataBegin,imgDimX,imgDimY)) return false;
	imageData = new char[3*imgDimX*imgDimY];
	if(!readPpmData(path.c_str(),imageData,dataBegin,imgDimX,imgDimY)) return false;

	std::shared_ptr<Texture2D> texture(new Texture2D(path));
	if (!(texture->load(GL_RGB, imgDimX, imgDimY, GL_RGB, GL_UNSIGNED_BYTE, imageData))) return false;
	inOutTexPtr = texture;
	texture_list.push_back(std::move(texture));

	delete imageData;
	return true;
}

bool ResourceManager::createTexture3D(const std::string path, glm::ivec3 textureRes, std::shared_ptr<Texture3D> &inOutTexPtr)
{
	for(std::list<std::shared_ptr<Texture3D>>::iterator i = volume_list.begin(); i != volume_list.end(); ++i)
	{
		if(((*i)->getFilename())==path)
		{
			inOutTexPtr = (*i);
			return true;
		}
	}

	std::shared_ptr<Texture3D> volume(new Texture3D(path));
	if(!(volume->loadTextureFile(path,textureRes))) return false;
	inOutTexPtr = volume;
	volume_list.push_back(std::move(volume));

	return true;
}

bool ResourceManager::createTexture3D(float* volumeData, glm::ivec3 textureRes, GLenum internalFormat, GLenum format, std::shared_ptr<Texture3D> &inOutTexPtr)
{
	std::shared_ptr<Texture3D> volume(new Texture3D());
	if(!(volume->loadArrayF(volumeData,textureRes,internalFormat,format))) return false;
	inOutTexPtr = volume;
	volume_list.push_back(std::move(volume));

	return true;
}

bool ResourceManager::loadFbxGeometry(const std::string &path, std::shared_ptr<Mesh> &geomPtr)
{
	try {
		geomPtr = Mesh::loadFromFBX(path);
	} catch (FBX::BaseException e) {
		std::cerr << "Couldn't load " << path << ": " << e.what() << "\n";
		return false;
	}

	return true;
}

bool ResourceManager::loadBinaryGeometry(const std::string &path, std::shared_ptr<Mesh> &geomPtr)
{
	geomPtr.reset(new Mesh(path));

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

		if( !geomPtr->bufferDataFromArray(vertices,indices,sizeof(Vertex_pntcub)*num_vertices,sizeof(unsigned int)*num_indices,GL_TRIANGLES) ) return false;
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
				inOutMtlInfo.diff_path = new char[tempStr.length()+1];
				strcpy((inOutMtlInfo.diff_path),tempStr.c_str());
			}
			else if(tempStr == "ts")
			{
				iter2 = (iter1 + 1);
				iter1 = buffer.end();
				tempStr.assign(iter2,iter1);
				inOutMtlInfo.spec_path = new char[tempStr.length()+1];
				strcpy((inOutMtlInfo.spec_path),tempStr.c_str());
			}
			else if(tempStr == "tr")
			{
				iter2 = (iter1 + 1);
				iter1 = buffer.end();
				tempStr.assign(iter2,iter1);
				inOutMtlInfo.roughness_path = new char[tempStr.length()+1];
				strcpy((inOutMtlInfo.roughness_path),tempStr.c_str());
			}
			else if(tempStr == "tn")
			{
				iter2 = (iter1 + 1);
				iter1 = buffer.end();
				tempStr.assign(iter2,iter1);
				inOutMtlInfo.normal_path = new char[tempStr.length()+1];
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

	std::ostringstream source;
	while( inFile.good() ) {
		int c = inFile.get();
		if( ! inFile.eof() ) source << (char) c;
	}
	inFile.close();

	return source.str();
}

bool ResourceManager::readPpmHeader(const char* filename, long& headerEndPos, int& imgDimX, int& imgDimY)
{
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
	std::getline(file,buffer,'\n');
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

bool ResourceManager::readPpmData(const char* filename, char* imageData, long dataBegin, int imgDimX, int imgDimY)
{
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
