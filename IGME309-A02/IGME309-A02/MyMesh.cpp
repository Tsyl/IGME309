#include "MyMesh.h"
void MyMesh::Init(void)
{
	m_bBinded = false;
	m_uVertexCount = 0;

	m_VAO = 0;
	m_VBO = 0;

	m_pShaderMngr = ShaderManager::GetInstance();
}
void MyMesh::Release(void)
{
	m_pShaderMngr = nullptr;

	if (m_VBO > 0)
		glDeleteBuffers(1, &m_VBO);

	if (m_VAO > 0)
		glDeleteVertexArrays(1, &m_VAO);

	m_lVertex.clear();
	m_lVertexPos.clear();
	m_lVertexCol.clear();
}
MyMesh::MyMesh()
{
	Init();
}
MyMesh::~MyMesh() { Release(); }
MyMesh::MyMesh(MyMesh& other)
{
	m_bBinded = other.m_bBinded;

	m_pShaderMngr = other.m_pShaderMngr;

	m_uVertexCount = other.m_uVertexCount;

	m_VAO = other.m_VAO;
	m_VBO = other.m_VBO;
}
MyMesh& MyMesh::operator=(MyMesh& other)
{
	if (this != &other)
	{
		Release();
		Init();
		MyMesh temp(other);
		Swap(temp);
	}
	return *this;
}
void MyMesh::Swap(MyMesh& other)
{
	std::swap(m_bBinded, other.m_bBinded);
	std::swap(m_uVertexCount, other.m_uVertexCount);

	std::swap(m_VAO, other.m_VAO);
	std::swap(m_VBO, other.m_VBO);

	std::swap(m_lVertex, other.m_lVertex);
	std::swap(m_lVertexPos, other.m_lVertexPos);
	std::swap(m_lVertexCol, other.m_lVertexCol);

	std::swap(m_pShaderMngr, other.m_pShaderMngr);
}
void MyMesh::CompleteMesh(vector3 a_v3Color)
{
	uint uColorCount = m_lVertexCol.size();
	for (uint i = uColorCount; i < m_uVertexCount; ++i)
	{
		m_lVertexCol.push_back(a_v3Color);
	}
}
void MyMesh::AddVertexPosition(vector3 a_v3Input)
{
	m_lVertexPos.push_back(a_v3Input);
	m_uVertexCount = m_lVertexPos.size();
}
void MyMesh::AddVertexColor(vector3 a_v3Input)
{
	m_lVertexCol.push_back(a_v3Input);
}
void MyMesh::CompileOpenGL3X(void)
{
	if (m_bBinded)
		return;

	if (m_uVertexCount == 0)
		return;

	CompleteMesh();

	for (uint i = 0; i < m_uVertexCount; i++)
	{
		//Position
		m_lVertex.push_back(m_lVertexPos[i]);
		//Color
		m_lVertex.push_back(m_lVertexCol[i]);
	}
	glGenVertexArrays(1, &m_VAO);//Generate vertex array object
	glGenBuffers(1, &m_VBO);//Generate Vertex Buffered Object

	glBindVertexArray(m_VAO);//Bind the VAO
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);//Bind the VBO
	glBufferData(GL_ARRAY_BUFFER, m_uVertexCount * 2 * sizeof(vector3), &m_lVertex[0], GL_STATIC_DRAW);//Generate space for the VBO

	// Position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)0);

	// Color attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)(1 * sizeof(vector3)));

	m_bBinded = true;

	glBindVertexArray(0); // Unbind VAO
}
void MyMesh::Render(matrix4 a_mProjection, matrix4 a_mView, matrix4 a_mModel)
{
	// Use the buffer and shader
	GLuint nShader = m_pShaderMngr->GetShaderID("Basic");
	glUseProgram(nShader); 

	//Bind the VAO of this object
	glBindVertexArray(m_VAO);

	// Get the GPU variables by their name and hook them to CPU variables
	GLuint MVP = glGetUniformLocation(nShader, "MVP");
	GLuint wire = glGetUniformLocation(nShader, "wire");

	//Final Projection of the Camera
	matrix4 m4MVP = a_mProjection * a_mView * a_mModel;
	glUniformMatrix4fv(MVP, 1, GL_FALSE, glm::value_ptr(m4MVP));
	
	//Solid
	glUniform3f(wire, -1.0f, -1.0f, -1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);  

	//Wire
	glUniform3f(wire, 1.0f, 0.0f, 1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-1.f, -1.f);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);
	glDisable(GL_POLYGON_OFFSET_LINE);

	glBindVertexArray(0);// Unbind VAO so it does not get in the way of other objects
}
void MyMesh::AddTri(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft)
{
	//C
	//| \
	//A--B
	//This will make the triangle A->B->C 
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);
}
void MyMesh::AddQuad(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft, vector3 a_vTopRight)
{
	//C--D
	//|  |
	//A--B
	//This will make the triangle A->B->C and then the triangle C->B->D
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);

	AddVertexPosition(a_vTopLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopRight);
}
void MyMesh::GenerateCube(float a_fSize, vector3 a_v3Color)
{
	if (a_fSize < 0.01f)
		a_fSize = 0.01f;

	Release();
	Init();

	float fValue = a_fSize * 0.5f;
	//3--2
	//|  |
	//0--1

	vector3 point0(-fValue,-fValue, fValue); //0
	vector3 point1( fValue,-fValue, fValue); //1
	vector3 point2( fValue, fValue, fValue); //2
	vector3 point3(-fValue, fValue, fValue); //3

	vector3 point4(-fValue,-fValue,-fValue); //4
	vector3 point5( fValue,-fValue,-fValue); //5
	vector3 point6( fValue, fValue,-fValue); //6
	vector3 point7(-fValue, fValue,-fValue); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCuboid(vector3 a_v3Dimensions, vector3 a_v3Color)
{
	Release();
	Init();

	vector3 v3Value = a_v3Dimensions * 0.5f;
	//3--2
	//|  |
	//0--1
	vector3 point0(-v3Value.x, -v3Value.y, v3Value.z); //0
	vector3 point1(v3Value.x, -v3Value.y, v3Value.z); //1
	vector3 point2(v3Value.x, v3Value.y, v3Value.z); //2
	vector3 point3(-v3Value.x, v3Value.y, v3Value.z); //3

	vector3 point4(-v3Value.x, -v3Value.y, -v3Value.z); //4
	vector3 point5(v3Value.x, -v3Value.y, -v3Value.z); //5
	vector3 point6(v3Value.x, v3Value.y, -v3Value.z); //6
	vector3 point7(-v3Value.x, v3Value.y, -v3Value.z); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCone(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// Replace this with your code
	float fValue = a_fHeight * 0.5f;
	vector3 pTop(0, fValue, 0);
	vector3 pBottom(0, -fValue, 0);
	vector3 *pSegments = new glm::vec3[a_nSubdivisions];

	double inc = glm::pi<double>() * 2 / a_nSubdivisions;
	double tht = 0.0;

	for (int i = 0; i < a_nSubdivisions; i++)
	{
		pSegments[i] = vector3(a_fRadius, 1, a_fRadius) * vector3(glm::cos(tht), -fValue, glm::sin(tht));
		tht += inc;
	}

	for (int i = 0; i < a_nSubdivisions; i++)
	{
		if (i == a_nSubdivisions - 1)
		{
			AddTri(pSegments[0], pSegments[i], pTop);
			AddTri(pSegments[i], pSegments[0], pBottom);
			break;
		}

		AddTri(pSegments[i+1], pSegments[i], pTop);
		AddTri(pSegments[i], pSegments[i+1], pBottom);
	}

	delete[] pSegments;
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCylinder(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// Replace this with your code
	float fValue = a_fHeight * 0.5f;
	vector3 pTop(0, fValue, 0);
	vector3 pBottom(0, -fValue, 0);
	vector3 *pSegmentsTop = new glm::vec3[a_nSubdivisions];
	vector3 *pSegmentsBottom = new glm::vec3[a_nSubdivisions];

	double inc = glm::pi<double>() * 2 / a_nSubdivisions;
	double tht = 0.0;

	for (int i = 0; i < a_nSubdivisions; i++)
	{
		pSegmentsTop[i] = vector3(a_fRadius, 1, a_fRadius) * vector3(glm::cos(tht), fValue, glm::sin(tht));
		pSegmentsBottom[i] = vector3(a_fRadius, 1, a_fRadius) * vector3(glm::cos(tht), -fValue, glm::sin(tht));
		tht += inc;
	}

	for (int i = 0; i < a_nSubdivisions; i++)
	{
		if (i == a_nSubdivisions - 1)
		{
			AddTri(pSegmentsTop[0], pSegmentsTop[i], pTop);
			AddTri(pSegmentsBottom[i], pSegmentsBottom[0], pBottom);
			AddQuad(pSegmentsBottom[0], pSegmentsBottom[i], pSegmentsTop[0], pSegmentsTop[i]);
			break;
		}
		AddTri(pSegmentsTop[i + 1], pSegmentsTop[i], pTop);
		AddTri(pSegmentsBottom[i], pSegmentsBottom[i + 1], pBottom);
		AddQuad(pSegmentsBottom[i + 1], pSegmentsBottom[i], pSegmentsTop[i + 1], pSegmentsTop[i]);
	}

	delete[] pSegmentsTop;
	delete[] pSegmentsBottom;
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTube(float a_fOuterRadius, float a_fInnerRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// Replace this with your code
	float fValue = a_fHeight * 0.5f;
	vector3 *pSegmentsOuterTop = new glm::vec3[a_nSubdivisions];
	vector3 *pSegmentsOuterBottom = new glm::vec3[a_nSubdivisions];
	vector3 *pSegmentsInnerTop = new glm::vec3[a_nSubdivisions];
	vector3 *pSegmentsInnerBottom = new glm::vec3[a_nSubdivisions];

	double inc = glm::pi<double>() * 2 / a_nSubdivisions;
	double tht = 0.0;

	for (int i = 0; i < a_nSubdivisions; i++)
	{
		pSegmentsOuterTop[i] = vector3(a_fOuterRadius, 1, a_fOuterRadius) * vector3(glm::cos(tht), fValue, glm::sin(tht));
		pSegmentsOuterBottom[i] = vector3(a_fOuterRadius, 1, a_fOuterRadius) * vector3(glm::cos(tht), -fValue, glm::sin(tht));
		pSegmentsInnerTop[i] = vector3(a_fInnerRadius, 1, a_fInnerRadius) * vector3(glm::cos(tht), fValue, glm::sin(tht));
		pSegmentsInnerBottom[i] = vector3(a_fInnerRadius, 1, a_fInnerRadius) * vector3(glm::cos(tht), -fValue, glm::sin(tht));

		tht += inc;
	}

	for (int i = 0; i < a_nSubdivisions; i++)
	{
		if (i == a_nSubdivisions - 1)
		{
			AddQuad(pSegmentsOuterBottom[0], pSegmentsOuterBottom[i], pSegmentsOuterTop[0], pSegmentsOuterTop[i]);
			AddQuad(pSegmentsInnerBottom[i], pSegmentsInnerBottom[0], pSegmentsInnerTop[i], pSegmentsInnerTop[0]);
			AddQuad(pSegmentsOuterBottom[i], pSegmentsOuterBottom[0], pSegmentsInnerBottom[i], pSegmentsInnerBottom[0]);
			AddQuad(pSegmentsOuterTop[0], pSegmentsOuterTop[i], pSegmentsInnerTop[0], pSegmentsInnerTop[i]);
			break;
		}
		AddQuad(pSegmentsOuterBottom[i + 1], pSegmentsOuterBottom[i], pSegmentsOuterTop[i + 1], pSegmentsOuterTop[i]);
		AddQuad(pSegmentsInnerBottom[i], pSegmentsInnerBottom[i + 1], pSegmentsInnerTop[i], pSegmentsInnerTop[i + 1]);
		AddQuad(pSegmentsOuterBottom[i], pSegmentsOuterBottom[i + 1], pSegmentsInnerBottom[i], pSegmentsInnerBottom[i + 1]);
		AddQuad(pSegmentsOuterTop[i + 1], pSegmentsOuterTop[i], pSegmentsInnerTop[i + 1], pSegmentsInnerTop[i]);
	}

	delete[] pSegmentsOuterTop;
	delete[] pSegmentsOuterBottom;
	delete[] pSegmentsInnerTop;
	delete[] pSegmentsInnerBottom;
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTorus(float a_fOuterRadius, float a_fInnerRadius, int a_nSubdivisionsA, int a_nSubdivisionsB, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_nSubdivisionsA < 3)
		a_nSubdivisionsA = 3;
	if (a_nSubdivisionsA > 360)
		a_nSubdivisionsA = 360;

	if (a_nSubdivisionsB < 3)
		a_nSubdivisionsB = 3;
	if (a_nSubdivisionsB > 360)
		a_nSubdivisionsB = 360;

	Release();
	Init();

	// Replace this with your code
	vector3 middle(0, 0, 0);
	float a_fSegmentRadius = (a_fOuterRadius - a_fInnerRadius)/2;
	vector3 *pSegmentsShape = new glm::vec3[a_nSubdivisionsA];
	vector3 *pSegmentsSection = new glm::vec3[a_nSubdivisionsB*a_nSubdivisionsB];

	double incShape = glm::pi<double>() * 2 / a_nSubdivisionsA;
	double incSection = glm::pi<double>() * 2 / a_nSubdivisionsB;
	double thtShape = 0.0;
	double thtSection = 0.0;

	for (int i = 0; i < a_nSubdivisionsA; i++)
	{
		pSegmentsShape[i] = vector3(a_fInnerRadius + a_fSegmentRadius, 1, a_fInnerRadius + a_fSegmentRadius) * vector3(glm::cos(thtShape), 0, glm::sin(thtShape));
		glm::mat4 rotation = ToMatrix4(glm::quat(thtShape, 0, 1, 0));
		for (int j = 0; j < a_nSubdivisionsB; j++)
		{
			int nOfSegment = (i*a_nSubdivisionsA) + j;
			pSegmentsSection[nOfSegment] = pSegmentsShape[i] + (a_fSegmentRadius * vector3(0, sin(thtSection), cos(thtSection)));
			// Here is where rotation would go.
			thtSection += incSection;
		}
		thtSection = 0.0;
		thtShape += incShape;
	}

	for (int i = 0; i < a_nSubdivisionsA; i++)
	{
		int currentSection;
		int nextSection;
		
		currentSection = i*a_nSubdivisionsA;

		if (i == a_nSubdivisionsA - 1)
		{
			// Testing Triangles to check why the torus is not displaying correctly.
			AddTri(pSegmentsShape[0], pSegmentsShape[i], middle);

			// Torus rendering
			for (int j = 0; j < a_nSubdivisionsB; j++)
			{
				AddQuad(pSegmentsSection[j + 1], pSegmentsSection[j], pSegmentsSection[currentSection + j + 1], pSegmentsSection[currentSection + j]);
			}
			break;
		}

		nextSection = (i + 1)*a_nSubdivisionsA;

		// Testing Triangles to check why the torus is not displaying correctly.
		AddTri(pSegmentsShape[i + 1], pSegmentsShape[i], middle);

		// Torus rendering
		for (int j = 0; j < a_nSubdivisionsB; j++)
		{
			AddQuad(pSegmentsSection[nextSection + j + 1], pSegmentsSection[nextSection + j], pSegmentsSection[currentSection + j + 1], pSegmentsSection[currentSection + j]);
		}
	}

	delete[] pSegmentsShape;
	delete[] pSegmentsSection;
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateSphere(float a_fRadius, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	//Sets minimum and maximum of subdivisions
	if (a_nSubdivisions < 1)
	{
		GenerateCube(a_fRadius * 2.0f, a_v3Color);
		return;
	}
	if (a_nSubdivisions > 6)
		a_nSubdivisions = 6;

	Release();
	Init();

	// Replace this with your code
	float fValue = a_fRadius * 0.5f;
	vector3 pTop(0, fValue, 0);
	vector3 pBottom(0, -fValue, 0);
	vector3 *pSegmentsPri = new glm::vec3[a_nSubdivisions];
	vector3 *pSegmentsSec = new glm::vec3[a_nSubdivisions];

	double inc = glm::pi<double>() * 2 / a_nSubdivisions;
	double tht = 0.0;

	for (int i = 0; i < a_nSubdivisions; i++)
	{
		double temptht = tht;
		pSegmentsPri[i] = vector3(a_fRadius, 1, a_fRadius) * vector3(glm::cos(tht), fValue, glm::sin(tht));
		tht = 0;
		for (int i = 0; i < a_nSubdivisions; i++)
		{
			pSegmentsSec[i] = vector3(a_fRadius, 1, a_fRadius) * vector3(pSegmentsPri[i].x, glm::cos(tht), glm::sin(tht));
			tht += inc;
		}
		tht = temptht;
		tht += inc;
	}

	for (int i = 0; i < a_nSubdivisions; i++)
	{
		if (i == a_nSubdivisions - 1)
		{
			AddTri(pSegmentsPri[0], pSegmentsPri[i], pTop);
			AddTri(pSegmentsSec[i], pSegmentsSec[0], pBottom);
			AddQuad(pSegmentsSec[0], pSegmentsSec[i], pSegmentsPri[0], pSegmentsPri[i]);
			break;
		}
		AddTri(pSegmentsPri[i + 1], pSegmentsPri[i], pTop);
		AddTri(pSegmentsSec[i], pSegmentsSec[i + 1], pBottom);
		AddQuad(pSegmentsSec[i + 1], pSegmentsSec[i], pSegmentsPri[i + 1], pSegmentsPri[i]);
	}

	delete[] pSegmentsPri;
	delete[] pSegmentsSec;
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}