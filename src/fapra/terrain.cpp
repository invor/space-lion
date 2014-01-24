#include "terrain.h"

Terrain::Terrain() : m_size(0), m_range(0)
{
}

Terrain::~Terrain()
{
}

Terrain::Terrain(int size, GLfloat range) : m_size(size), m_range(range)
{
}


bool Terrain::init(std::shared_ptr<Material> material, std::shared_ptr<Texture> heightmap)
{
	/*	create vertices for base quad & create mesh */
	Vertex_p *vertexArray = new Vertex_p[4];
	GLuint *indexArray = new GLuint[4];

	vertexArray[0]=Vertex_p(0.0f,0.0f,0.0f);
	vertexArray[1]=Vertex_p(1.0f,0.0f,0.0f);
	vertexArray[2]=Vertex_p(1.0f,0.0f,1.0f);
	vertexArray[3]=Vertex_p(0.0f,0.0f,1.0f);

	indexArray[0]=3;indexArray[1]=2;indexArray[2]=1;indexArray[3]=0;
	
	if(!(m_quad.bufferDataFromArray(vertexArray,indexArray,sizeof(Vertex_p)*4,sizeof(GLuint)*4,GL_PATCHES))) return false;
	m_quad.setVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex_p),0);

	/*	set material - check if shader program is of valid type! */
	if(material == nullptr || (material->getShaderProgram()->getType() != TERRAIN))
		return false;

	m_base_material = material;

	/*	set heightmap */
	if(heightmap == nullptr)
		return false;
	m_heightmap = heightmap;
	m_heightmap->texParameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	m_heightmap->texParameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	m_heightmap->texParameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_heightmap->texParameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	return true;
}

void Terrain::render()
{
	/*	
	*	shader program is activated outside this method in scene code because of transformation matrices 
	*	might want to rethink this though...
	*/

	m_base_material->use();

	/*	
	*	A material uses texture units 0 to 3, so after a call to Material::use() 
	*	one should contine with unit 4. Also, assuming to know the name of the uniform
	*	slot in the shader should properbly be discouraged, but its doable in this
	*	context, because the terrain uses a specific shader anyway (init() also checks for this..)
	*/
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE4);
	m_base_material->getShaderProgram()->setUniform("heightmap_tx2D",4);
	m_heightmap->bindTexture();

	m_base_material->getShaderProgram()->setUniform("size", m_size);
	m_base_material->getShaderProgram()->setUniform("range", m_range);

	/*	assume square terrain - render a quad per square meter */
	glPatchParameteri(GL_PATCH_VERTICES, 4);
	m_quad.draw(m_size*m_size);
}

void Terrain::setSize(int size)
{
	m_size = size;
}

int Terrain::getSize()
{
	return m_size;
}

void Terrain::setRange(GLfloat range)
{
	m_range = range;
}

GLfloat Terrain::getRange()
{
	return m_range;
}

std::shared_ptr<Material> Terrain::getMaterial()
{
	return m_base_material;
}