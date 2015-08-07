#include "resourceManager.h"

ResourceManager::ResourceManager(){}

ResourceManager::~ResourceManager(){}


void ResourceManager::clearLists()
{
	geometry_list.clear();
	material_list.clear();
	texture_list.clear();
//	volume_list.clear();
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

std::shared_ptr<Mesh> ResourceManager::createBox()
{
	/*	Check list of vertexBufferObjects for default box object(Name="Box") */
	for(auto& mesh : geometry_list)
	{
		if(mesh->getName() == "Box")
			return mesh;
	}

	/*	if default box not already in list, continue here */
	Vertex_pntcub *vertexArray = new Vertex_pntcub[24];
	GLuint *indexArray = new GLuint[36];

	/*	front face */
	vertexArray[0]=Vertex_pntcub(-0.5,-0.5,0.5,0.0,0.0,1.0,1.0,0.0,0.0,(GLubyte)0.0,(GLubyte)0.0,(GLubyte)0.0,(GLubyte)1.0,0.0,0.0,0.0,1.0,0.0);
	vertexArray[1]=Vertex_pntcub(-0.5,0.5,0.5,0.0,0.0,1.0,1.0,0.0,0.0,(GLubyte)0.0,(GLubyte)1.0,(GLubyte)0.0,(GLubyte)1.0,0.0,1.0,0.0,1.0,0.0);
	vertexArray[2]=Vertex_pntcub(0.5,0.5,0.5,0.0,0.0,1.0,1.0,0.0,0.0,(GLubyte)1.0,(GLubyte)1.0,(GLubyte)0.0,(GLubyte)1.0,1.0,1.0,0.0,1.0,0.0);
	vertexArray[3]=Vertex_pntcub(0.5,-0.5,0.5,0.0,0.0,1.0,1.0,0.0,0.0,(GLubyte)1.0,(GLubyte)0.0,(GLubyte)0.0,(GLubyte)1.0,1.0,0.0,0.0,1.0,0.0);
	/*	right face */
	vertexArray[4]=Vertex_pntcub(0.5,-0.5,0.5,1.0,0.0,0.0,0.0,0.0,-1.0,(GLubyte)1.0,(GLubyte)0.0,(GLubyte)0.0,(GLubyte)1.0,0.0,0.0,0.0,1.0,0.0);
	vertexArray[5]=Vertex_pntcub(0.5,0.5,0.5,1.0,0.0,0.0,0.0,0.0,-1.0,(GLubyte)1.0,(GLubyte)1.0,(GLubyte)0.0,(GLubyte)1.0,0.0,1.0,0.0,1.0,0.0);
	vertexArray[6]=Vertex_pntcub(0.5,0.5,-0.5,1.0,0.0,0.0,0.0,0.0,-1.0,(GLubyte)1.0,(GLubyte)1.0,(GLubyte)1.0,(GLubyte)1.0,1.0,1.0,0.0,1.0,0.0);
	vertexArray[7]=Vertex_pntcub(0.5,-0.5,-0.5,1.0,0.0,0.0,0.0,0.0,-1.0,(GLubyte)1.0,(GLubyte)0.0,(GLubyte)1.0,(GLubyte)1.0,1.0,0.0,0.0,1.0,0.0);
	/*	left face */
	vertexArray[8]=Vertex_pntcub(-0.5,-0.5,-0.5,-1.0,0.0,0.0,0.0,0.0,1.0,(GLubyte)0.0,(GLubyte)0.0,(GLubyte)1.0,(GLubyte)1.0,0.0,0.0,0.0,1.0,0.0);
	vertexArray[9]=Vertex_pntcub(-0.5,0.5,-0.5,-1.0,0.0,0.0,0.0,0.0,1.0,(GLubyte)0.0,(GLubyte)1.0,(GLubyte)1.0,(GLubyte)1.0,0.0,1.0,0.0,1.0,0.0);
	vertexArray[10]=Vertex_pntcub(-0.5,0.5,0.5,-1.0,0.0,0.0,0.0,0.0,1.0,(GLubyte)0.0,(GLubyte)1.0,(GLubyte)0.0,(GLubyte)1.0,1.0,1.0,0.0,1.0,0.0);
	vertexArray[11]=Vertex_pntcub(-0.5,-0.5,0.5,-1.0,0.0,0.0,0.0,0.0,1.0,(GLubyte)0.0,(GLubyte)0.0,(GLubyte)0.0,(GLubyte)1.0,1.0,0.0,0.0,1.0,0.0);
	/*	back face */
	vertexArray[12]=Vertex_pntcub(0.5,-0.5,-0.5,0.0,0.0,-1.0,-1.0,0.0,0.0,(GLubyte)1.0,(GLubyte)0.0,(GLubyte)1.0,(GLubyte)1.0,0.0,0.0,0.0,1.0,0.0);
	vertexArray[13]=Vertex_pntcub(0.5,0.5,-0.5,0.0,0.0,-1.0,-1.0,0.0,0.0,(GLubyte)1.0,(GLubyte)1.0,(GLubyte)1.0,(GLubyte)1.0,0.0,1.0,0.0,1.0,0.0);
	vertexArray[14]=Vertex_pntcub(-0.5,0.5,-0.5,0.0,0.0,-1.0,-1.0,0.0,0.0,(GLubyte)0.0,(GLubyte)1.0,(GLubyte)1.0,(GLubyte)1.0,1.0,1.0,0.0,1.0,0.0);
	vertexArray[15]=Vertex_pntcub(-0.5,-0.5,-0.5,0.0,0.0,-1.0,-1.0,0.0,0.0,(GLubyte)0.0,(GLubyte)0.0,(GLubyte)1.0,(GLubyte)1.0,1.0,0.0,0.0,1.0,0.0);
	/*	bottom face */
	vertexArray[16]=Vertex_pntcub(-0.5,-0.5,0.5,0.0,-1.0,0.0,1.0,0.0,0.0,(GLubyte)0.0,(GLubyte)0.0,(GLubyte)0.0,(GLubyte)1.0,0.0,0.0,0.0,0.0,1.0);
	vertexArray[17]=Vertex_pntcub(-0.5,-0.5,-0.5,0.0,-1.0,0.0,1.0,0.0,0.0,(GLubyte)0.0,(GLubyte)0.0,(GLubyte)1.0,(GLubyte)1.0,0.0,1.0,0.0,0.0,1.0);
	vertexArray[18]=Vertex_pntcub(0.5,-0.5,-0.5,0.0,-1.0,0.0,1.0,0.0,0.0,(GLubyte)1.0,(GLubyte)0.0,(GLubyte)1.0,(GLubyte)1.0,1.0,1.0,0.0,0.0,1.0);
	vertexArray[19]=Vertex_pntcub(0.5,-0.5,0.5,0.0,-1.0,0.0,1.0,0.0,0.0,(GLubyte)1.0,(GLubyte)0.0,(GLubyte)0.0,(GLubyte)1.0,1.0,0.0,0.0,0.0,1.0);
	/*	top face */
	vertexArray[20]=Vertex_pntcub(-0.5,0.5,0.5,0.0,1.0,0.0,1.0,0.0,0.0,(GLubyte)0.0,(GLubyte)1.0,(GLubyte)0.0,(GLubyte)1.0,0.0,0.0,0.0,0.0,1.0);
	vertexArray[21]=Vertex_pntcub(-0.5,0.5,-0.5,0.0,1.0,0.0,1.0,0.0,0.0,(GLubyte)0.0,(GLubyte)1.0,(GLubyte)1.0,(GLubyte)1.0,0.0,1.0,0.0,0.0,1.0);
	vertexArray[22]=Vertex_pntcub(0.5,0.5,-0.5,0.0,1.0,0.0,1.0,0.0,0.0,(GLubyte)1.0,(GLubyte)1.0,(GLubyte)1.0,(GLubyte)1.0,1.0,1.0,0.0,0.0,1.0);
	vertexArray[23]=Vertex_pntcub(0.5,0.5,0.5,0.0,1.0,0.0,1.0,0.0,0.0,(GLubyte)1.0,(GLubyte)1.0,(GLubyte)0.0,(GLubyte)1.0,1.0,0.0,0.0,0.0,1.0);

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

	std::shared_ptr<Mesh> box_mesh(new Mesh("Box"));
	box_mesh->bufferDataFromArray(vertexArray,indexArray,sizeof(Vertex_pntcub)*24,sizeof(GLuint)*36,GL_TRIANGLES);
	box_mesh->setVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex_pntcub),0);
	box_mesh->setVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex_pntcub),(GLvoid*) sizeof(Vertex_p));
	box_mesh->setVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,sizeof(Vertex_pntcub),(GLvoid*) sizeof(Vertex_pn));
	box_mesh->setVertexAttribPointer(3,4,GL_UNSIGNED_BYTE,GL_FALSE,sizeof(Vertex_pntcub),(GLvoid*) sizeof(Vertex_pnt));
	box_mesh->setVertexAttribPointer(4,2,GL_FLOAT,GL_FALSE,sizeof(Vertex_pntcub),(GLvoid*) sizeof(Vertex_pntc));
	box_mesh->setVertexAttribPointer(5,3,GL_FLOAT,GL_FALSE,sizeof(Vertex_pntcub),(GLvoid*) sizeof(Vertex_pntcu));


	geometry_list.push_back(std::move(box_mesh));

	return geometry_list.back();
}

std::shared_ptr<Mesh> ResourceManager::createMesh(const std::string path)
{
	/*	Check list of vertexBufferObjects for filename */
	for(auto& mesh : geometry_list)
	{
		if(mesh->getName() == path)
			return mesh;
	}

	/*	Check file type before trying to load it */
	std::string file_type;
	std::size_t found = path.rfind('.');
	if (found != std::string::npos) file_type = path.substr(found + 1);

	if(file_type == "fbx")
	{
		/* Just some testing */
		geometry_list.push_back(loadFbxGeometry(path));
	}
	else if(file_type == "slraw")
	{
		/* Just some testing */
		geometry_list.push_back(loadBinaryGeometry(path));
	}
	else
	{
		// do something kid!
		//return
	}

	return geometry_list.back();
}

std::shared_ptr<Material> ResourceManager::createMaterial()
{
	/*	Check list of materials for default material*/
	for(auto& material : material_list)
	{
		if(material->getName() == "default")
			return material;
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
	prgPtr = createShaderProgram(SURFACE_LIGHTING);
	texPtr1 = createTexture2D(1,1,diffuseData);
	texPtr2 = createTexture2D(1,1,specularData);
	texPtr3 = createTexture2D(1,1,roughnessData);
	texPtr4 = createTexture2D(1,1,normalData);

	std::shared_ptr<Material> material(new Material("default",prgPtr,texPtr1,texPtr2,texPtr3,texPtr4));
	material_list.push_back(std::move(material));

	prgPtr.reset();
	texPtr1.reset();
	texPtr2.reset();
	texPtr3.reset();
	texPtr4.reset();

	return material_list.back();
}

std::shared_ptr<Material> ResourceManager::createMaterial(const std::string path)
{
	// copys a struct around by values, but hell it's easer to read for now
	MaterialInfo inOutMtlInfo = parseMaterial(path);

	for(auto& material : material_list)
	{
		if(material->getName() == path)
			return material;
	}

	std::shared_ptr<GLSLProgram> prgPtr;
	std::shared_ptr<Texture> texPtr1;
	std::shared_ptr<Texture> texPtr2;
	std::shared_ptr<Texture> texPtr3;
	std::shared_ptr<Texture> texPtr4;
	//if(!createShaderProgram(SURFACE_LIGHTING,prgPtr)) return false;
	prgPtr = createShaderProgram(static_cast<shaderType>(inOutMtlInfo.shader_type));
	texPtr1 = createTexture2D(inOutMtlInfo.diff_path);
	texPtr2 = createTexture2D(inOutMtlInfo.spec_path);
	texPtr3 = createTexture2D(inOutMtlInfo.roughness_path);
	texPtr4 = createTexture2D(inOutMtlInfo.normal_path);

	std::shared_ptr<Material> material(new Material(path,prgPtr,texPtr1,texPtr2,texPtr3,texPtr4));
	material_list.push_back(std::move(material));

	prgPtr.reset();
	texPtr1.reset();
	texPtr2.reset();
	texPtr3.reset();
	texPtr4.reset();

	return material_list.back();
}

std::shared_ptr<GLSLProgram> ResourceManager::createShaderProgram(shaderType type)
{
	/*	Check list of shader programs for the shader type */
	//	for(std::list<std::shared_ptr<GLSLProgram>>::iterator i = shader_program_list.begin(); i != shader_program_list.end(); ++i)
	//	{
	//		if(((*i)->getType())==type){
	//			inOutPrgPtr = (*i);
	//			return true;
	//		}
	//	}

	std::shared_ptr<GLSLProgram> shaderPrg(new GLSLProgram());
	shaderPrg->init();
	std::string vertSource;
	std::string tessContSource;
	std::string tessEvalSource;
	std::string fragSource;
	std::string computeSource;

	switch(type)
	{
	case SKY : {
		vertSource = readShaderFile("../resources/shaders/genericPostProc_v.glsl");
		fragSource = readShaderFile("../resources/shaders/fapra/sky_f.glsl");
		shaderPrg->bindAttribLocation(0,"v_position");
		shaderPrg->bindAttribLocation(1,"v_uvCoord");
		shaderPrg->bindFragDataLocation(0, "frag_colour");
		break; }
	case TRANSMITTANCE_COMPUTE : {
		computeSource = readShaderFile("../resources/shaders/fapra/transmittance_c.glsl");
		break; }
	case INSCATTER_SINGLE : {
		computeSource = readShaderFile("../resources/shaders/fapra/inscatter_single_c.glsl");
		break; }
	case IRRADIANCE_SINGLE : {
		computeSource = readShaderFile("../resources/shaders/fapra/irradiance_single_c.glsl");
		break; }
	case TERRAIN : {
		vertSource = readShaderFile("../resources/shaders/fapra/terrain_v.glsl");
		tessContSource = readShaderFile("../resources/shaders/fapra/terrain_tc.glsl");
		tessEvalSource = readShaderFile("../resources/shaders/fapra/terrain_te.glsl");
		fragSource = readShaderFile("../resources/shaders/fapra/terrain_f.glsl");
		shaderPrg->bindAttribLocation(0,"v_position");
		shaderPrg->bindFragDataLocation(0, "frag_colour");
		shaderPrg->bindFragDataLocation(1, "normal");
		shaderPrg->bindFragDataLocation(2, "tangent_bitangent");
		shaderPrg->bindFragDataLocation(3, "spec_colour_roughness");
		shaderPrg->bindFragDataLocation(4, "depth");
		break; }
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
		vertSource = readShaderFile("../resources/shaders/flat_v.glsl");
		fragSource = readShaderFile("../resources/shaders/flat_f.glsl");
		shaderPrg->bindAttribLocation(0,"vPosition");
		shaderPrg->bindAttribLocation(1,"vNormal");
		shaderPrg->bindAttribLocation(2,"vTangent");
		shaderPrg->bindAttribLocation(3,"vColour");
		shaderPrg->bindAttribLocation(4,"vUVCoord");
		break; }
	case FXAA : {
		vertSource = readShaderFile("../resources/shaders/genericPostProc_v.glsl");
		fragSource = readShaderFile("../resources/shaders/fxaa_f.glsl");
		shaderPrg->bindAttribLocation(0,"v_position");
		shaderPrg->bindAttribLocation(1,"v_uvCoord");
		break; }
	case IDLE : {
		vertSource = readShaderFile("../resources/shaders/genericPostProc_v.glsl");
		fragSource = readShaderFile("../resources/shaders/idle_f.glsl");
		shaderPrg->bindAttribLocation(0,"v_position");
		shaderPrg->bindAttribLocation(1,"v_uvCoord");
		break; }
	case VOLUME_RAYCASTING : {
		vertSource = readShaderFile("../resources/shaders/volRen_v.glsl");
		fragSource = readShaderFile("../resources/shaders/volRen_f.glsl");
		shaderPrg->bindAttribLocation(0,"vPosition");
		shaderPrg->bindAttribLocation(3,"vColour");
		break; }
	case GAUSSIAN : {
		vertSource = readShaderFile("../resources/shaders/genericPostProc_v.glsl");
		fragSource = readShaderFile("../resources/shaders/seperatedGaussian_f.glsl");
		shaderPrg->bindAttribLocation(0,"v_position");
		shaderPrg->bindAttribLocation(1,"v_uvCoord");
		break; }
	case GRADIENT : {
		vertSource = readShaderFile("../resources/shaders/genericPostProc_v.glsl");
		fragSource = readShaderFile("../resources/shaders/gradient_f.glsl");
		shaderPrg->bindAttribLocation(0,"v_position");
		shaderPrg->bindAttribLocation(1,"v_uvCoord");;
		break; }
	case STRUCTURE_TENSOR : {
		vertSource = readShaderFile("../resources/shaders/genericPostProc_v.glsl");
		fragSource = readShaderFile("../resources/shaders/structureTensor_f.glsl");
		shaderPrg->bindAttribLocation(0,"v_position");
		shaderPrg->bindAttribLocation(1,"v_uvCoord");
		break; }
	case HESSE : {
		vertSource = readShaderFile("../resources/shaders/genericPostProc_v.glsl");
		fragSource = readShaderFile("../resources/shaders/hesse_f.glsl");
		shaderPrg->bindAttribLocation(0,"v_position");
		shaderPrg->bindAttribLocation(1,"v_uvCoord");
		break; }
	default : {
		return false;
		break; }
	}

	if(!vertSource.empty())
		if(!shaderPrg->compileShaderFromString(&vertSource,GL_VERTEX_SHADER)){ std::cout<<shaderPrg->getLog(); /*TODO more error handling*/}
	if(!fragSource.empty())
		if(!shaderPrg->compileShaderFromString(&fragSource,GL_FRAGMENT_SHADER)){ std::cout<<shaderPrg->getLog(); /*TODO more error handling*/}
	if(!tessContSource.empty())
		if(!shaderPrg->compileShaderFromString(&tessContSource,GL_TESS_CONTROL_SHADER)){ std::cout<<shaderPrg->getLog(); /*TODO more error handling*/}
	if(!tessEvalSource.empty())
		if(!shaderPrg->compileShaderFromString(&tessEvalSource,GL_TESS_EVALUATION_SHADER)){ std::cout<<shaderPrg->getLog(); /*TODO more error handling*/}
	/*
	*	THIS SEEM TO BE OUTDATED IF IT WAS EVER TRUE
	*	A non-empty compute source string should only happen if all other sources are empty strings.
	*	I won't check for this though, assuming that nobody - i.e. me - fucked up a compute shader case above.
	*/
	if(!computeSource.empty())
		if(!shaderPrg->compileShaderFromString(&computeSource,GL_COMPUTE_SHADER)){ std::cout<<shaderPrg->getLog(); /*TODO more error handling*/}

	if(!shaderPrg->link()){ std::cout<<shaderPrg->getLog(); /*TODO more error handling*/}

	shader_program_list.push_back(std::move(shaderPrg));

	return shader_program_list.back();
}

std::shared_ptr<Texture> ResourceManager::createTexture2D(int dimX, int dimY, float* data)
{
	std::shared_ptr<Texture2D> texture(new Texture2D("",GL_RGB,dimX, dimY,GL_RGB,GL_FLOAT,data));

	texture_list.push_back(std::move(texture));

	return texture_list.back();
}

std::shared_ptr<Texture> ResourceManager::createTexture2D(const std::string path)
{
	for(auto& texture : texture_list)
	{
		if(texture->getName() == path)
			return texture;
	}

	char* imageData;
	unsigned long dataBegin;
	int imgDimX;
	int imgDimY;

	if(!readPpmHeader(path.c_str(),dataBegin,imgDimX,imgDimY)) return false;
	imageData = new char[3*imgDimX*imgDimY];
	if(!readPpmData(path.c_str(),imageData,dataBegin,imgDimX,imgDimY)) return false;

	std::shared_ptr<Texture2D> texture(new Texture2D(path,GL_RGB, imgDimX, imgDimY, GL_RGB, GL_UNSIGNED_BYTE, imageData));

	texture_list.push_back(std::move(texture));

	delete[] imageData;

	return texture_list.back();
}

//	bool ResourceManager::createTexture3D(const std::string path, glm::ivec3 textureRes, std::shared_ptr<Texture3D> &inOutTexPtr)
//	{
//		for(std::list<std::shared_ptr<Texture3D>>::iterator i = volume_list.begin(); i != volume_list.end(); ++i)
//		{
//			if(((*i)->getName())==path)
//			{
//				inOutTexPtr = (*i);
//				return true;
//			}
//		}
//	
//		std::shared_ptr<Texture3D> volume(new Texture3D(path));
//		if(!(volume->loadTextureFile(path,textureRes))) return false;
//		inOutTexPtr = volume;
//		volume_list.push_back(std::move(volume));
//	
//		return true;
//	}
//	
//	bool ResourceManager::createTexture3D(GLenum internalFormat, glm::ivec3 textureRes, GLenum format, GLenum type, GLvoid* volumeData, std::shared_ptr<Texture3D> &inOutTexPtr)
//	{
//		std::shared_ptr<Texture3D> volume(new Texture3D());
//		if (!(volume->load(internalFormat, textureRes.x, textureRes.y, textureRes.z, format, type, volumeData))) return false;
//		inOutTexPtr = volume;
//		volume_list.push_back(std::move(volume));
//	
//		return true;
//	}

 std::shared_ptr<Mesh> ResourceManager::loadFbxGeometry(const std::string &path)
{
	try
	{
		FBX::OpenGL::BindAttribLocations locations;

		std::shared_ptr<Mesh> mesh(new Mesh(path));

		std::shared_ptr<FBX::Geometry> geometry = FBX::Geometry::fbxLoadFirstGeometry(path);
		FBX::OpenGL::GeometrySerialize serialize(geometry->features);

		uint8_t* memory;
		size_t memory_size;
		serialize.serialize(memory, memory_size, geometry->vertices);

		if (sizeof(unsigned int) == sizeof(GLuint)) {
			mesh->bufferDataFromArray(reinterpret_cast<Vertex_p*>(memory),
				reinterpret_cast<const GLuint*>(geometry->triangle_indices.data()),
				static_cast<GLsizei>(memory_size),
				static_cast<GLsizei>(geometry->triangle_indices.size()) * sizeof(GLuint), GL_TRIANGLES);
		}
		else {
			std::vector<GLuint> gluint_indices(geometry->triangle_indices.begin(), geometry->triangle_indices.end());
			mesh->bufferDataFromArray(reinterpret_cast<Vertex_p*>(memory), gluint_indices.data(), static_cast<GLsizei>(memory_size), static_cast<GLsizei>(geometry->triangle_indices.size()) * sizeof(GLuint), GL_TRIANGLES);
		}

		delete [] memory;

		const FBX::OpenGL::GeometrySerialize::Settings &s(serialize.settings());

		if (locations.ndx_position >= 0) {
			mesh->setVertexAttribPointer(locations.ndx_position, 3, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(s.stride), (void*) s.offset_position);
		}
		if (s.features & FBX::Geometry::NORMAL && locations.ndx_normal >= 0) {
			mesh->setVertexAttribPointer(locations.ndx_normal, 3, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(s.stride), (void*) s.offset_normal);
		}
		if (s.features & FBX::Geometry::TANGENT && locations.ndx_tangent >= 0) {
			mesh->setVertexAttribPointer(locations.ndx_tangent, 3, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(s.stride), (void*) s.offset_tangent);
		}
		if (s.features & FBX::Geometry::COLOR && locations.ndx_color >= 0) {
			mesh->setVertexAttribPointer(locations.ndx_color, 4, GL_UNSIGNED_BYTE, GL_TRUE, static_cast<GLsizei>(s.stride), (void*) s.offset_color);
			// no static color support in Mesh
			// ndx_static_color = -1;
		}
		else {
			// no static color support in Mesh
			// ndx_static_color = locations.ndx_color;
		}
		if (s.features & FBX::Geometry::UVCOORD && locations.ndx_uvcoord >= 0) {
			mesh->setVertexAttribPointer(locations.ndx_uvcoord, 2, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(s.stride), (void*) s.offset_uvcoord);
		}
		if (s.features & FBX::Geometry::BINORMAL && locations.ndx_binormal >= 0) {
			mesh->setVertexAttribPointer(locations.ndx_binormal, 3, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(s.stride), (void*) s.offset_binormal);
		}

		return std::move(mesh);
	}
	catch (FBX::BaseException e)
	{
		std::cerr << "Couldn't load " << path << ": " << e.what() << "\n";
	}
}

std::shared_ptr<Mesh> ResourceManager::loadBinaryGeometry(const std::string &path)
{
	std::shared_ptr<Mesh> mesh(new Mesh(path));

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

		if( !mesh->bufferDataFromArray(vertices,indices,sizeof(Vertex_pntcub)*num_vertices,sizeof(unsigned int)*num_indices,GL_TRIANGLES) ) return false;
	}
	else
	{
		// do something kid!
		//return
	}

	mesh->setVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex_pntcub),0);
	mesh->setVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex_pntcub),(GLvoid*) sizeof(Vertex_p));
	mesh->setVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,sizeof(Vertex_pntcub),(GLvoid*) sizeof(Vertex_pn));
	mesh->setVertexAttribPointer(3,4,GL_UNSIGNED_BYTE,GL_FALSE,sizeof(Vertex_pntcub),(GLvoid*) sizeof(Vertex_pnt));
	mesh->setVertexAttribPointer(4,2,GL_FLOAT,GL_FALSE,sizeof(Vertex_pntcub),(GLvoid*) sizeof(Vertex_pntc));
	mesh->setVertexAttribPointer(5,3,GL_FLOAT,GL_FALSE,sizeof(Vertex_pntcub),(GLvoid*) sizeof(Vertex_pntcu));

	return std::move(mesh);
}

MaterialInfo ResourceManager::parseMaterial(const std::string materialPath)
{
	std::string buffer;
	std::string tempStr;
	std::string::iterator iter1;
	std::string::iterator iter2;

	std::ifstream file;
	file.open(materialPath, std::ifstream::in);

	MaterialInfo mat_info;

	if( file.is_open() )
	{
		file.seekg(0, std::ifstream::beg);

		std::getline(file,buffer,'\n');
		mat_info.id = atoi(buffer.c_str());

		std::getline(file,buffer,'\n');
		mat_info.shader_type = atoi(buffer.c_str());

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
				mat_info.diff_path = new char[tempStr.length()+1];
				strcpy((mat_info.diff_path),tempStr.c_str());
			}
			else if(tempStr == "ts")
			{
				iter2 = (iter1 + 1);
				iter1 = buffer.end();
				tempStr.assign(iter2,iter1);
				mat_info.spec_path = new char[tempStr.length()+1];
				strcpy((mat_info.spec_path),tempStr.c_str());
			}
			else if(tempStr == "tr")
			{
				iter2 = (iter1 + 1);
				iter1 = buffer.end();
				tempStr.assign(iter2,iter1);
				mat_info.roughness_path = new char[tempStr.length()+1];
				strcpy((mat_info.roughness_path),tempStr.c_str());
			}
			else if(tempStr == "tn")
			{
				iter2 = (iter1 + 1);
				iter1 = buffer.end();
				tempStr.assign(iter2,iter1);
				mat_info.normal_path = new char[tempStr.length()+1];
				strcpy((mat_info.normal_path),tempStr.c_str());
			}
		}
	}

	return mat_info;
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

bool ResourceManager::readPpmHeader(const char* filename, unsigned long& headerEndPos, int& imgDimX, int& imgDimY)
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
		headerEndPos = static_cast<long>(file.tellg());
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
	headerEndPos = static_cast<unsigned long>(file.tellg());
	file.close();
	return true;
}

bool ResourceManager::readPpmData(const char* filename, char* imageData, unsigned long dataBegin, int imgDimX, int imgDimY)
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
	unsigned long length = static_cast<unsigned long>(file.tellg());
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
