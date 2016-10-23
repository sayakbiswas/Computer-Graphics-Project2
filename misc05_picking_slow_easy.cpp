// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <array>
#include <stack>
#include <sstream>
// Include GLEW
#include <GL/glew.h>
// Include GLFW
#include <glfw3.h>
// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
using namespace glm;
// Include AntTweakBar
#include <AntTweakBar.h>

#include <common/shader.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>

#include <iostream>
#include <cstring>
using namespace std;

const int window_width = 800, window_height = 600;

typedef struct Vertex {
	float Position[4];
	float Color[4];
	float Normal[3];
	void SetPosition(float *coords) {
		Position[0] = coords[0];
		Position[1] = coords[1];
		Position[2] = coords[2];
		Position[3] = 1.0;
	}
	void SetColor(float *color) {
		Color[0] = color[0];
		Color[1] = color[1];
		Color[2] = color[2];
		Color[3] = color[3];
	}
	void SetNormal(float *coords) {
		Normal[0] = coords[0];
		Normal[1] = coords[1];
		Normal[2] = coords[2];
	}
};

// function prototypes
int initWindow(void);
void initOpenGL(void);
void loadObject(char*, glm::vec4, Vertex * &, GLushort* &, int);
void createVAOs(Vertex[], GLushort[], int);
void createObjects(void);
void pickObject(void);
void renderScene(void);
void cleanup(void);
static void keyCallback(GLFWwindow*, int, int, int, int);
static void mouseCallback(GLFWwindow*, int, int, int);
glm::vec4 highlightObject(Vertex*, float);

// GLOBAL VARIABLES
GLFWwindow* window;

glm::mat4 gProjectionMatrix;
glm::mat4 gViewMatrix;

GLuint gPickedIndex = -1;
std::string gMessage;

GLuint programID;
GLuint pickingProgramID;

const GLuint NumObjects = 250;	// ATTN: THIS NEEDS TO CHANGE AS YOU ADD NEW OBJECTS
GLuint VertexArrayId[NumObjects] = { 0 };
GLuint VertexBufferId[NumObjects] = { 0 };
GLuint IndexBufferId[NumObjects] = { 0 };

size_t NumIndices[NumObjects] = { 0 };
size_t VertexBufferSize[NumObjects] = { 0 };
size_t IndexBufferSize[NumObjects] = { 0 };

GLuint MatrixID;
GLuint ModelMatrixID;
GLuint ViewMatrixID;
GLuint ProjMatrixID;
GLuint PickingMatrixID;
GLuint pickingColorID;
GLuint pickingColorArrayID;
GLuint LightID;
GLuint Light1ID;
GLuint Light2ID;

GLint gX = 0.0;
GLint gZ = 0.0;

// animation control
bool animation = false;
GLfloat phi = 0.0;

bool selectCamera = false;
bool selectPen = false;
bool selectBase = false;
bool selectTop = false;
bool selectArm1 = false;
bool selectArm2 = false;
bool moveCameraLeft = false;
bool moveCameraRight = false;
bool moveCameraUp = false;
bool moveCameraDown = false;
bool rotatePenLeft = false;
bool rotatePenRight = false;
bool rotatePenUp = false;
bool rotatePenDown = false;
bool rotatePenAxisLeft = false;
bool rotatePenAxisRight = false;
bool slideBaseLeft = false;
bool slideBaseRight = false;
bool slideBaseForward = false;
bool slideBaseBackward = false;
bool rotateTopLeft = false;
bool rotateTopRight = false;
bool rotateArm1Up = false;
bool rotateArm1Down = false;
bool rotateArm2Up = false;
bool rotateArm2Down = false;
bool isPenSelected = false;
bool isBaseSelected = false;
bool isTopSelected = false;
bool isArm1Selected = false;
bool isArm2Selected = false;
bool isShiftPressed = false;
bool baseSlid = false;
bool penRotated = false;
bool penAxisRotated = false;
bool buttonRotated = false;
bool topRotated = false;
bool arm1Rotated = false;
bool arm2Rotated = false;
bool animated = false;

GLfloat cameraAngleTheta = M_PI/4;
GLfloat cameraAnglePhi = asin(1/(sqrt(3)));
GLfloat cameraSphereRadius = sqrt(300);
Vertex* Base_Verts;
GLushort* Base_Idcs;
Vertex* Top_Verts;
GLushort* Top_Idcs;
Vertex* Arm1_Verts;
GLushort* Arm1_Idcs;
Vertex* Joint_Verts;
GLushort* Joint_Idcs;
Vertex* Arm2_Verts;
GLushort* Arm2_Idcs;
Vertex* Pen_Verts;
GLushort* Pen_Idcs;
Vertex* Button_Verts;
GLushort* Button_Idcs;
Vertex* Proj_Verts;
GLushort* Proj_Idcs;

glm::mat4 baseTransform;
glm::mat4 penTransform;
glm::mat4 penAxisTransform;
glm::mat4 buttonTransform;
glm::mat4 projectileTransform;
glm::mat4 topTransform;
glm::mat4 arm1Transform;
glm::mat4 arm2Transform;
glm::mat4x4 ModelMatrixCopy;
GLfloat matrix[16];
float velocity = 30.0f;
float posX, posY, posZ;
double previousTime = glfwGetTime();
GLfloat arm1RotationAngle = 0.0f;
GLfloat arm2RotationAngle = 0.0f;
GLfloat topRotationAngle = 0.0f;
GLfloat penLeftRightRotationAngle = 0.0f;
GLfloat penUpDownRotationAngle = 0.0f;
double timer = 0.0f;
double timeStep = 0.01f;
float pickingColor[10] = { 0 / 255.0f, 1 / 255.0f, 2 / 255.0f, 3 / 255.0f,  4 / 255.0f, 5 / 255.0f, 6 / 255.0f, 7 / 255.0f, 8 / 255.0f, 9 / 255.0f };
vec3 projected;

void loadObject(char* file, glm::vec4 color, Vertex * &out_Vertices, GLushort* &out_Indices, int ObjectId)
{
	// Read our .obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	bool res = loadOBJ(file, vertices, normals);

	std::vector<GLushort> indices;
	std::vector<glm::vec3> indexed_vertices;
	std::vector<glm::vec2> indexed_uvs;
	std::vector<glm::vec3> indexed_normals;
	indexVBO(vertices, normals, indices, indexed_vertices, indexed_normals);

	const size_t vertCount = indexed_vertices.size();
	const size_t idxCount = indices.size();

	// populate output arrays
	out_Vertices = new Vertex[vertCount];
	for (int i = 0; i < vertCount; i++) {
		out_Vertices[i].SetPosition(&indexed_vertices[i].x);
		out_Vertices[i].SetNormal(&indexed_normals[i].x);
		out_Vertices[i].SetColor(&color[0]);
	}
	out_Indices = new GLushort[idxCount];
	for (int i = 0; i < idxCount; i++) {
		out_Indices[i] = indices[i];
	}

	// set global variables!!
	NumIndices[ObjectId] = idxCount;
	VertexBufferSize[ObjectId] = sizeof(out_Vertices[0]) * vertCount;
	IndexBufferSize[ObjectId] = sizeof(GLushort) * idxCount;
}


void createObjects(void)
{
	//-- COORDINATE AXES --//
	Vertex CoordVerts[] =
	{
		{ { 0.0, 0.0, 0.0, 1.0 }, { 1.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, 0.0, 1.0 }, { 1.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 0.0, 0.0, 0.0, 1.0 }, { 0.0, 1.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 0.0, 5.0, 0.0, 1.0 }, { 0.0, 1.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 0.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 0.0, 0.0, 5.0, 1.0 }, { 0.0, 0.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
	};

	VertexBufferSize[0] = sizeof(CoordVerts);	// ATTN: this needs to be done for each hand-made object with the ObjectID (subscript)
	createVAOs(CoordVerts, NULL, 0);

	//-- GRID --//

	// ATTN: create your grid vertices here!
	Vertex GridVerts[44];
	int k = 0;
	for(int i = -5; i <= 5; i++) {
		GridVerts[4 * k] = {{i, 0, -5, 1.0}, {1.0, 1.0, 1.0, 1.0}, {0.0, 0.0, 1.0}};
		GridVerts[4 * k + 1] = {{i, 0, 5, 1.0}, {1.0, 1.0, 1.0, 1.0}, {0.0, 0.0, 1.0}};
		GridVerts[4 * k + 2] = {{-5, 0, i, 1.0}, {1.0, 1.0, 1.0, 1.0}, {0.0, 0.0, 1.0}};
		GridVerts[4 * k + 3] = {{5, 0, i, 1.0}, {1.0, 1.0, 1.0, 1.0}, {0.0, 0.0, 1.0}};
		k++;
	}

	VertexBufferSize[1] = sizeof(GridVerts);
	createVAOs(GridVerts, NULL, 1);

	//-- .OBJs --//

	// ATTN: load your models here
	loadObject("base.obj", glm::vec4(1.0, 1.0, 0.0, 1.0), Base_Verts, Base_Idcs, 2);
	createVAOs(Base_Verts, Base_Idcs, 2);

	loadObject("top.obj", glm::vec4(0.0, 1.0, 1.0, 1.0), Top_Verts, Top_Idcs, 3);
	createVAOs(Top_Verts, Top_Idcs, 3);

	loadObject("arm1.obj", glm::vec4(1.0, 0.0, 1.0, 1.0), Arm1_Verts, Arm1_Idcs, 4);
	createVAOs(Arm1_Verts, Arm1_Idcs, 4);

	loadObject("joint.obj", glm::vec4(1.0, 0.5, 0.5, 1.0), Joint_Verts, Joint_Idcs, 5);
	createVAOs(Joint_Verts, Joint_Idcs, 5);

	loadObject("arm2.obj", glm::vec4(0.5, 0.0, 0.5, 1.0), Arm2_Verts, Arm2_Idcs, 6);
	createVAOs(Arm2_Verts, Arm2_Idcs, 6);

	loadObject("pen.obj", glm::vec4(0.1, 0.3, 0.4, 1.0), Pen_Verts, Pen_Idcs, 7);
	createVAOs(Pen_Verts, Pen_Idcs, 7);

	loadObject("button.obj", glm::vec4(1.0, 0.0, 0.0, 1.0), Button_Verts, Button_Idcs, 8);
	createVAOs(Button_Verts, Button_Idcs, 8);

	loadObject("projectile.obj", glm::vec4(1.0, 1.0, 1.0, 1.0), Proj_Verts, Proj_Idcs, 9);
	createVAOs(Proj_Verts, Proj_Idcs, 9);
}

void renderScene(void)
{
	//ATTN: DRAW YOUR SCENE HERE. MODIFY/ADAPT WHERE NECESSARY!
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	glm::vec4 vp = glm::vec4(viewport[0], viewport[1], viewport[2], viewport[3]);

	double currentTime = glfwGetTime();
	double deltaTime = currentTime - previousTime;
	previousTime = currentTime;
	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.2f, 0.0f);
	// Re-clear the screen for real rendering
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if(moveCameraLeft) {
		cameraAngleTheta -= 0.01f;
	}

	if(moveCameraRight) {
		cameraAngleTheta += 0.01f;
	}

	if(moveCameraUp) {
		cameraAnglePhi -= 0.01f;
	}

	if(moveCameraDown) {
		cameraAnglePhi += 0.01f;
	}

	if(selectCamera && (moveCameraLeft || moveCameraRight || moveCameraDown || moveCameraUp)) {
		float camX = cameraSphereRadius * cos(cameraAnglePhi) * sin(cameraAngleTheta);
		float camY = cameraSphereRadius * sin(cameraAnglePhi);
		float camZ = cameraSphereRadius * cos(cameraAnglePhi) * cos(cameraAngleTheta);
		gViewMatrix = glm::lookAt(glm::vec3(camX, camY, camZ),	// eye
			glm::vec3(0.0, 0.0, 0.0),	// center
			glm::vec3(0.0, 1.0, 0.0));	// up
	}

	if(selectBase && !isBaseSelected) {
		loadObject("base.obj", highlightObject(Base_Verts, 0.5), Base_Verts, Base_Idcs, 2);
		createVAOs(Base_Verts, Base_Idcs, 2);
		isBaseSelected = true;
	} else if(!selectBase && isBaseSelected) {
		loadObject("base.obj", glm::vec4(1.0, 1.0, 0.0, 1.0), Base_Verts, Base_Idcs, 2);
		createVAOs(Base_Verts, Base_Idcs, 2);
		isBaseSelected = false;
	} else if(selectBase && isBaseSelected) {
		if(slideBaseLeft) {
			baseTransform = glm::translate(baseTransform, glm::vec3(-0.01f, 0.0f, 0.0f));
			baseSlid = true;
		}
		if(slideBaseRight) {
			baseTransform = glm::translate(baseTransform, glm::vec3(0.01f, 0.0f, 0.0f));
			baseSlid = true;
		}
		if(slideBaseForward) {
			baseTransform = glm::translate(baseTransform, glm::vec3(0.0f, 0.0f, 0.01f));
			baseSlid = true;
		}
		if(slideBaseBackward) {
			baseTransform = glm::translate(baseTransform, glm::vec3(0.0f, 0.0f, -0.01f));
			baseSlid = true;
		}
	}

	if(selectTop && !isTopSelected) {
		loadObject("top.obj", highlightObject(Top_Verts, 0.5), Top_Verts, Top_Idcs, 3);
		createVAOs(Top_Verts, Top_Idcs, 3);
		isTopSelected = true;
	} else if(!selectTop && isTopSelected) {
		loadObject("top.obj", glm::vec4(0.0, 1.0, 1.0, 1.0), Top_Verts, Top_Idcs, 3);
		createVAOs(Top_Verts, Top_Idcs, 3);
		isTopSelected = false;
	} else if(selectTop && isTopSelected) {
		if(rotateTopLeft) {
			topTransform = glm::rotate(topTransform, glm::radians(-5.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			topRotationAngle -= 5.0f;
			if(topRotationAngle < -360) {
				topRotationAngle += 360;
			}
			topRotated = true;
		}
		if(rotateTopRight) {
			topTransform = glm::rotate(topTransform, glm::radians(5.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			topRotationAngle += 5.0f;
			if(topRotationAngle > 360) {
				topRotationAngle -= 360;
			}
			topRotated = true;
		}
	}

	if(selectArm1 && !isArm1Selected) {
		loadObject("arm1.obj", highlightObject(Arm1_Verts, 0.5), Arm1_Verts, Arm1_Idcs, 4);
		createVAOs(Arm1_Verts, Arm1_Idcs, 4);
		isArm1Selected = true;
	} else if(!selectArm1 && isArm1Selected) {
		loadObject("arm1.obj", glm::vec4(1.0, 0.0, 1.0, 1.0), Arm1_Verts, Arm1_Idcs, 4);
		createVAOs(Arm1_Verts, Arm1_Idcs, 4);
		isArm1Selected = false;
	} else if(selectArm1 && isArm1Selected) {
		if(rotateArm1Up) {
			arm1Transform = glm::translate(arm1Transform, glm::vec3(-0.05f, 2.0f, 0.01f));
			arm1Transform = glm::rotate(arm1Transform, glm::radians(-2.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			arm1Transform = glm::translate(arm1Transform, glm::vec3(0.05f, -2.0f, -0.01f));
			arm1RotationAngle -= 2.0f;
			arm1Rotated = true;
		}
		if(rotateArm1Down) {
			arm1Transform = glm::translate(arm1Transform, glm::vec3(-0.05f, 2.0f, 0.01f));
			arm1Transform = glm::rotate(arm1Transform, glm::radians(2.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			arm1Transform = glm::translate(arm1Transform, glm::vec3(0.05f, -2.0f, -0.01f));
			arm1RotationAngle += 2.0f;
			arm1Rotated = true;
		}
	}

	if(selectArm2 && !isArm2Selected) {
		loadObject("arm2.obj", highlightObject(Arm2_Verts, 0.5), Arm2_Verts, Arm1_Idcs, 6);
		createVAOs(Arm2_Verts, Arm1_Idcs, 6);
		isArm2Selected = true;
	} else if(!selectArm2 && isArm2Selected) {
		loadObject("arm2.obj", glm::vec4(0.5, 0.0, 0.5, 1.0), Arm2_Verts, Arm2_Idcs, 6);
		createVAOs(Arm2_Verts, Arm2_Idcs, 6);
		isArm2Selected = false;
	} else if(selectArm2 && isArm2Selected) {
		if(rotateArm2Up) {
			arm2Transform = glm::translate(arm2Transform, glm::vec3(-0.05f, 2.85f, 2.06f));
			arm2Transform = glm::rotate(arm2Transform, glm::radians(-2.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			arm2Transform = glm::translate(arm2Transform, glm::vec3(0.05f, -2.85f, -2.06f));
			arm2RotationAngle -= 2.0f;
			arm2Rotated = true;
		}
		if(rotateArm2Down) {
			arm2Transform = glm::translate(arm2Transform, glm::vec3(-0.05f, 2.85f, 2.06f));
			arm2Transform = glm::rotate(arm2Transform, glm::radians(2.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			arm2Transform = glm::translate(arm2Transform, glm::vec3(0.05f, -2.85f, -2.06f));
			arm2RotationAngle += 2.0f;
			arm2Rotated = true;
		}
	}

	if(selectPen && !isPenSelected) {
		loadObject("pen.obj", highlightObject(Pen_Verts, 0.5), Pen_Verts, Pen_Idcs, 7);
		createVAOs(Pen_Verts, Pen_Idcs, 7);
		isPenSelected = true;
	} else if(!selectPen && isPenSelected) {
		loadObject("pen.obj", glm::vec4(0.1, 0.3, 0.4, 1.0), Pen_Verts, Pen_Idcs, 7);
		createVAOs(Pen_Verts, Pen_Idcs, 7);
		isPenSelected = false;
	} else if(selectPen && isPenSelected) {
		if(rotatePenLeft) {
			cout << "penLeftRightRotationAngle :: " << penLeftRightRotationAngle;
			if(penLeftRightRotationAngle > -90) {
				penTransform = glm::translate(penTransform, glm::vec3(0.02f, 1.04f, 2.25f));
				penTransform = glm::rotate(penTransform, glm::radians(-(penUpDownRotationAngle)), glm::vec3(1.0f, 0.0f, 0.0f));
				penTransform = glm::rotate(penTransform, glm::radians(-(penLeftRightRotationAngle)), glm::vec3(0.0f, 1.0f, 0.0f));
				penTransform = glm::rotate(penTransform, glm::radians(-2.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				penTransform = glm::rotate(penTransform, glm::radians(penLeftRightRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
				penTransform = glm::rotate(penTransform, glm::radians(penUpDownRotationAngle), glm::vec3(1.0f, 0.0f, 0.0f));
				penTransform = glm::translate(penTransform, glm::vec3(-0.02f, -1.04f, -2.25f));
				penLeftRightRotationAngle -= 2.0f;
				if(penLeftRightRotationAngle < -360) {
					penLeftRightRotationAngle += 360;
				}
				penRotated = true;
			}
		}
		if(rotatePenRight) {
			if(penLeftRightRotationAngle < 90) {
				penTransform = glm::translate(penTransform, glm::vec3(0.02f, 1.04f, 2.25f));
				penTransform = glm::rotate(penTransform, glm::radians(-(penUpDownRotationAngle)), glm::vec3(1.0f, 0.0f, 0.0f));
				penTransform = glm::rotate(penTransform, glm::radians(-(penLeftRightRotationAngle)), glm::vec3(0.0f, 1.0f, 0.0f));
				penTransform = glm::rotate(penTransform, glm::radians(2.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				penTransform = glm::rotate(penTransform, glm::radians(penLeftRightRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
				penTransform = glm::rotate(penTransform, glm::radians(penUpDownRotationAngle), glm::vec3(1.0f, 0.0f, 0.0f));
				penTransform = glm::translate(penTransform, glm::vec3(-0.02f, -1.04f, -2.25f));
				penLeftRightRotationAngle += 2.0f;
				if(penLeftRightRotationAngle > 360) {
					penLeftRightRotationAngle -= 360;
				}
				penRotated = true;
			}
		}
		if(rotatePenUp) {
			if(penUpDownRotationAngle > -50) {
				penTransform = glm::translate(penTransform, glm::vec3(0.02f, 1.04f, 2.25f));
				penTransform = glm::rotate(penTransform, glm::radians(-(penLeftRightRotationAngle)), glm::vec3(0.0f, 1.0f, 0.0f));
				penTransform = glm::rotate(penTransform, glm::radians(-(penUpDownRotationAngle)), glm::vec3(1.0f, 0.0f, 0.0f));
				penTransform = glm::rotate(penTransform, glm::radians(-2.0f), glm::vec3(1.0f, 0.0f, 0.0f));
				penTransform = glm::rotate(penTransform, glm::radians(penUpDownRotationAngle), glm::vec3(1.0f, 0.0f, 0.0f));
				penTransform = glm::rotate(penTransform, glm::radians(penLeftRightRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
				penTransform = glm::translate(penTransform, glm::vec3(-0.02f, -1.04f, -2.25f));
				penUpDownRotationAngle -= 2.0f;
				if(penUpDownRotationAngle < -360) {
					penUpDownRotationAngle += 360;
				}
				penRotated = true;
			}
		}
		if(rotatePenDown) {
			if(penUpDownRotationAngle < 140) {
				penTransform = glm::translate(penTransform, glm::vec3(0.02f, 1.04f, 2.25f));
				penTransform = glm::rotate(penTransform, glm::radians(-(penLeftRightRotationAngle)), glm::vec3(0.0f, 1.0f, 0.0f));
				penTransform = glm::rotate(penTransform, glm::radians(-(penUpDownRotationAngle)), glm::vec3(1.0f, 0.0f, 0.0f));
				penTransform = glm::rotate(penTransform, glm::radians(2.0f), glm::vec3(1.0f, 0.0f, 0.0f));
				penTransform = glm::rotate(penTransform, glm::radians(penUpDownRotationAngle), glm::vec3(1.0f, 0.0f, 0.0f));
				penTransform = glm::rotate(penTransform, glm::radians(penLeftRightRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
				penTransform = glm::translate(penTransform, glm::vec3(-0.02f, -1.04f, -2.25f));
				penUpDownRotationAngle += 2.0f;
				if(penUpDownRotationAngle > 360) {
					penUpDownRotationAngle -= 360;
				}
				penRotated = true;
			}
		}
		if(rotatePenAxisLeft) {
			penAxisTransform = glm::translate(penAxisTransform, glm::vec3(0.02f, 1.04f, 2.25f));
			penAxisTransform = glm::rotate(penAxisTransform, glm::radians(2.0f), glm::vec3(0.0f, 1.0f, 1.0f));
			penAxisTransform = glm::translate(penAxisTransform, glm::vec3(-0.02f, -1.04f, -2.25f));
			penAxisRotated = true;
		}
		if(rotatePenAxisRight) {
			penAxisTransform = glm::translate(penAxisTransform, glm::vec3(-0.02818f, 1.55654f, 2.81459f));
			penAxisTransform = glm::rotate(penAxisTransform, glm::radians(-2.0f), glm::vec3(0.0f, 1.0f, 1.0f));
			penAxisTransform = glm::translate(penAxisTransform, glm::vec3(0.02818f, -1.55654f, -2.81459f));
			penAxisRotated = true;
		}
	}

	if(animation) {
		timer += timeStep;
		glm::vec3 origin = glm::vec3(-0.02f, 0.76f, 2.2f);
		glm::vec3 penPos = glm::vec3(-0.04f, 2.21f, 3.37f);
		glm::vec3 direction = glm::normalize(origin - penPos);
		double penAngle = (atan2(direction.y, direction.x) * 180.0f / (float)M_PI) + arm1RotationAngle + arm2RotationAngle;
		if(penAngle > 360) {
			penAngle -= 360;
		}
		penAngle = glm::radians(penAngle);
		/*if(!animated) {
			posX = penPos.x;
			posY = penPos.y;
			posZ = penPos.z;
			animated = true;
		}*/

		cout << "topRotationAngle :: " << topRotationAngle << endl;

		if(2.21f + ModelMatrixCopy[3][1] < 0) {
			projectileTransform = glm::translate(projectileTransform, glm::vec3(penPos.x, penPos.y, penPos.z));
			if(topRotationAngle < 0 && topRotationAngle >= -90) {
				cout << "inside if1" << endl;
				baseTransform = glm::translate(baseTransform, glm::vec3(-(0.04f + baseTransform[3][0] - ModelMatrixCopy[3][0]),
																		0.0f,
																		3.37f + baseTransform[3][2] - ModelMatrixCopy[3][2]));
			} else if(topRotationAngle < -90 && topRotationAngle >= -180) {
				cout << "inside if2" << endl;
				baseTransform = glm::translate(baseTransform, glm::vec3(-(0.04f + baseTransform[3][0] - ModelMatrixCopy[3][0]),
																		0.0f,
																		-(3.37f + baseTransform[3][2] - ModelMatrixCopy[3][2])));
			} else if(topRotationAngle < -180 && topRotationAngle >= -270) {
				cout << "inside if3" << endl;
				baseTransform = glm::translate(baseTransform, glm::vec3(0.04f + baseTransform[3][0] - ModelMatrixCopy[3][0],
																		0.0f,
																		-(3.37f + baseTransform[3][2] -  ModelMatrixCopy[3][2])));
			} else if(topRotationAngle < -270 && topRotationAngle >= -360) {
				cout << "inside if4" << endl;
				baseTransform = glm::translate(baseTransform, glm::vec3(0.04f + baseTransform[3][0] - ModelMatrixCopy[3][0],
																		0.0f,
																		3.37f + baseTransform[3][2] - ModelMatrixCopy[3][2]));
			} else if(topRotationAngle > 0 && topRotationAngle <= 90) {
				baseTransform = glm::translate(baseTransform, glm::vec3(0.04f + ModelMatrixCopy[3][0],
																		0.0f,
																		3.37f + ModelMatrixCopy[3][2]));
			} else if(topRotationAngle > 90 && topRotationAngle <= 180) {
				baseTransform = glm::translate(baseTransform, glm::vec3(0.04f + ModelMatrixCopy[3][0],
																		0.0f,
																		-(3.37f + ModelMatrixCopy[3][2])));
			} else if(topRotationAngle > 180 && topRotationAngle <= 270) {
				baseTransform = glm::translate(baseTransform, glm::vec3(-(0.04f + ModelMatrixCopy[3][0]),
																		0.0f,
																		-(3.37f + ModelMatrixCopy[3][2])));
			} else if(topRotationAngle > 270 && topRotationAngle <= 360) {
				baseTransform = glm::translate(baseTransform, glm::vec3(-(0.04f + ModelMatrixCopy[3][0]),
																		0.0f,
																		3.37f + ModelMatrixCopy[3][2]));
			} else {
				cout << "inside else" << endl;
				baseTransform = glm::translate(baseTransform, glm::vec3(0.04f + ModelMatrixCopy[3][0],
																		0.0f,
																		3.37f + ModelMatrixCopy[3][2]));
			}
			//baseTransform = ModelMatrixCopy * baseTransform;
			//baseTransform = glm::translate(baseTransform, glm::vec3(0.0f, -ModelMatrixCopy[3][1], 0.0f));
			baseSlid = true;
			animation = false;
		} else {
			posX += direction.x * velocity * cos(penAngle) * (GLfloat)deltaTime;
			posY += ((direction.y * velocity * sin(penAngle)) - (0.5 * 9.8 * (GLfloat)deltaTime)) * (GLfloat)deltaTime;
			posZ += direction.z * velocity * cos(penAngle) * (GLfloat)deltaTime;
			projectileTransform = glm::translate(projectileTransform, glm::vec3(((penPos.x - posX) * 0.03f),
																				((penPos.y - posY) * 0.03f),
																				((penPos.z - posZ) * 0.03f)));
			for(int i = 0; i < 4; i++) {
				for(int j = 0; j < 4; j++) {
					cout << "projectileTransform["<<i<<"]["<<j<<"] :: "<< projectileTransform[i][j] << endl;
				}
			}

			for(int i = 0; i < 4; i++) {
				for(int j = 0; j < 4; j++) {
					cout << "arm1Transform["<<i<<"]["<<j<<"] :: "<< arm1Transform[i][j] << endl;
				}
			}

			for(int i = 0; i < 4; i++) {
				for(int j = 0; j < 4; j++) {
					cout << "arm2Transform["<<i<<"]["<<j<<"] :: "<< arm2Transform[i][j] << endl;
				}
			}
		}

		cout << "Coordinate stuff :: " << posX << " " << posY << " " << posZ << " " << penPos.y - posY << endl;
		cout << "Timer ::" << timer << endl;
		cout << "Angle stuff :: " << penAngle << " " << cos(penAngle) << " " << sin(penAngle) << " " << arm1RotationAngle << " " << arm2RotationAngle << " " << topRotationAngle << endl;

		//if(posY >= 0) { //TODO: -8.94047
			/*projectileTransform = glm::translate(projectileTransform, glm::vec3(0.0f, posY, posZ));
			posX = direction.x * 0.6 * (GLfloat)timer;
			posY = direction.y * (0.6 - (0.5 * 9.8 * (GLfloat)timer)) * (GLfloat)timer;
			posZ = direction.z * 0.6 * (GLfloat)timer;*/
		/*} else {
			projectileTransform = glm::translate(projectileTransform, glm::vec3(penPos.x, penPos.y, penPos.z));
			animation = false;
		}*/
	}

	glUseProgram(programID);
	{
		glm::vec3 lightPos = glm::vec3(4, 4, 4);
		glm::vec3 light1Pos = glm::vec3(5, 10, 10);
		glm::vec3 light2Pos = glm::vec3(10, 10, 5);
		glm::mat4x4 ModelMatrix = glm::mat4(1.0);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(Light1ID, light1Pos.x, light1Pos.y, light1Pos.z);
		glUniform3f(Light2ID, light2Pos.x, light2Pos.y, light2Pos.z);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &gViewMatrix[0][0]);
		glUniformMatrix4fv(ProjMatrixID, 1, GL_FALSE, &gProjectionMatrix[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

		glBindVertexArray(VertexArrayId[0]);	// draw CoordAxes
		glDrawArrays(GL_LINES, 0, 6);

		glBindVertexArray(VertexArrayId[1]);
		glDrawArrays(GL_LINES, 0, 44);

		if(baseSlid) {
			ModelMatrix = baseTransform;
		}
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glBindVertexArray(VertexArrayId[2]);
		glDrawElements(GL_TRIANGLES, NumIndices[2], GL_UNSIGNED_SHORT, (void*)0);

		if(topRotated) {
			ModelMatrix = topTransform;
			if(baseSlid) {
				ModelMatrix = baseTransform * ModelMatrix;
			}
		}
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glBindVertexArray(VertexArrayId[3]);
		glDrawElements(GL_TRIANGLES, NumIndices[3], GL_UNSIGNED_SHORT, (void*)0);

		if(arm1Rotated) {
			ModelMatrix = arm1Transform;
			if(topRotated) {
				ModelMatrix = topTransform * ModelMatrix;
			}
			if(baseSlid) {
				ModelMatrix = baseTransform * ModelMatrix;
			}
		}
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glBindVertexArray(VertexArrayId[4]);
		glDrawElements(GL_TRIANGLES, NumIndices[4], GL_UNSIGNED_SHORT, (void*)0);

		glBindVertexArray(VertexArrayId[5]);
		glDrawElements(GL_TRIANGLES, NumIndices[5], GL_UNSIGNED_SHORT, (void*)0);

		if(arm2Rotated) {
			ModelMatrix = arm2Transform;
			if(arm1Rotated) {
				ModelMatrix = arm1Transform * ModelMatrix;
			}
			if(topRotated) {
				ModelMatrix = topTransform * ModelMatrix;
			}
			if(baseSlid) {
				ModelMatrix = baseTransform * ModelMatrix;
			}
		}
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glBindVertexArray(VertexArrayId[6]);
		glDrawElements(GL_TRIANGLES, NumIndices[6], GL_UNSIGNED_SHORT, (void*)0);

		if(penRotated) {
			ModelMatrix = penTransform;
			if(arm2Rotated) {
				ModelMatrix = arm2Transform * ModelMatrix;
			}
			if(arm1Rotated) {
				ModelMatrix = arm1Transform * ModelMatrix;
			}
			if(topRotated) {
				ModelMatrix = topTransform * ModelMatrix;
			}
			if(baseSlid) {
				ModelMatrix = baseTransform * ModelMatrix;
			}
		}
		if(penAxisRotated) {
			ModelMatrix = penAxisTransform;
			if(penRotated) {
				ModelMatrix = penTransform * ModelMatrix;
			}
			if(arm2Rotated) {
				ModelMatrix = arm2Transform * ModelMatrix;
			}
			if(arm1Rotated) {
				ModelMatrix = arm1Transform * ModelMatrix;
			}
			if(topRotated) {
				ModelMatrix = topTransform * ModelMatrix;
			}
			if(baseSlid) {
				ModelMatrix = baseTransform * ModelMatrix;
			}
		}
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glBindVertexArray(VertexArrayId[7]);
		glDrawElements(GL_TRIANGLES, NumIndices[7], GL_UNSIGNED_SHORT, (void*)0);

		glBindVertexArray(VertexArrayId[8]);
		glDrawElements(GL_TRIANGLES, NumIndices[8], GL_UNSIGNED_SHORT, (void*)0);

		if(animation) {
			ModelMatrix = projectileTransform;
			if(penRotated) {
				ModelMatrix = penTransform * ModelMatrix;
			}
			if(arm2Rotated) {
				ModelMatrix = arm2Transform * ModelMatrix;
			}
			if(arm1Rotated) {
				ModelMatrix = arm1Transform * ModelMatrix;
			}
			if(topRotated) {
				ModelMatrix = topTransform * ModelMatrix;
			}
			if(baseSlid) {
				ModelMatrix = baseTransform * ModelMatrix;
			}
		}
		projected = glm::project(vec3(projectileTransform[3][0], projectileTransform[3][1], projectileTransform[3][2]), ModelMatrix, gProjectionMatrix, vp);
		cout << "projected :: " << projected.x  << " " << projected.y << " " << projected.z << endl;

		for(int i = 0; i < 4; i++) {
			for(int j = 0; j < 4; j++) {
				cout << "ModelMatrix["<<i<<"]["<<j<<"]:: " << ModelMatrix[i][j] << endl;
			}
		}

		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glBindVertexArray(VertexArrayId[9]);
		glDrawElements(GL_TRIANGLES, NumIndices[9], GL_UNSIGNED_SHORT, (void*)0);

		ModelMatrixCopy = ModelMatrix;

		glBindVertexArray(0);
	}
	glUseProgram(0);
	// Draw GUI
	TwDraw();

	// Swap buffers
	glfwSwapBuffers(window);
	glfwPollEvents();
}

void pickObject(void)
{
	// Clear the screen in white
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(pickingProgramID);
	{
		glm::mat4 ModelMatrix = glm::mat4(1.0); // TranslationMatrix * RotationMatrix;
		glm::mat4 MVP = gProjectionMatrix * gViewMatrix * ModelMatrix;

		// Send our transformation to the currently bound shader, in the "MVP" uniform
		glUniformMatrix4fv(PickingMatrixID, 1, GL_FALSE, &MVP[0][0]);

		// ATTN: DRAW YOUR PICKING SCENE HERE. REMEMBER TO SEND IN A DIFFERENT PICKING COLOR FOR EACH OBJECT BEFOREHAND
		glUniform1f(pickingColorID, pickingColor[2]);
		glBindVertexArray(VertexArrayId[2]);
		glDrawElements(GL_TRIANGLES, NumIndices[2], GL_UNSIGNED_SHORT, (void*)0);

		glUniform1f(pickingColorID, pickingColor[3]);
		glBindVertexArray(VertexArrayId[3]);
		glDrawElements(GL_TRIANGLES, NumIndices[3], GL_UNSIGNED_SHORT, (void*)0);

		glUniform1f(pickingColorID, pickingColor[4]);
		glBindVertexArray(VertexArrayId[4]);
		glDrawElements(GL_TRIANGLES, NumIndices[4], GL_UNSIGNED_SHORT, (void*)0);

		glUniform1f(pickingColorID, pickingColor[6]);
		glBindVertexArray(VertexArrayId[6]);
		glDrawElements(GL_TRIANGLES, NumIndices[6], GL_UNSIGNED_SHORT, (void*)0);

		glUniform1f(pickingColorID, pickingColor[7]);
		glBindVertexArray(VertexArrayId[7]);
		glDrawElements(GL_TRIANGLES, NumIndices[7], GL_UNSIGNED_SHORT, (void*)0);

		glBindVertexArray(0);

	}
	glUseProgram(0);
	// Wait until all the pending drawing commands are really done.
	// Ultra-mega-over slow !
	// There are usually a long time between glDrawElements() and
	// all the fragments completely rasterized.
	glFlush();
	glFinish();

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Read the pixel at the center of the screen.
	// You can also use glfwGetMousePos().
	// Ultra-mega-over slow too, even for 1 pixel,
	// because the framebuffer is on the GPU.
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	unsigned char data[4];
	glReadPixels(xpos, window_height - ypos, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data); // OpenGL renders with (0,0) on bottom, mouse reports with (0,0) on top

	// Convert the color back to an integer ID
	gPickedIndex = int(data[0]);
	//cout << "pickedIndex :: " << gPickedIndex << " " << data[0] << endl;

	if (gPickedIndex == 255){ // Full white, must be the background !
		gMessage = "background";
	}
	else {
		std::ostringstream oss;
		oss << "point " << gPickedIndex;
		gMessage = oss.str();
		if(gPickedIndex == 2) {
			if(selectBase) {
				selectBase = false;
			} else {
				selectBase = true;
			}
		}
		if(gPickedIndex == 3) {
			if(selectTop) {
				selectTop = false;
			} else {
				selectTop = true;
			}
		}
		if(gPickedIndex == 4) {
			if(selectArm1) {
				selectArm1 = false;
			} else {
				selectArm1 = true;
			}
		}
		if(gPickedIndex == 6) {
			if(selectArm2) {
				selectArm2 = false;
			} else {
				selectArm2 = true;
			}
		}
		if(gPickedIndex == 7) {
			if(selectPen) {
				selectPen = false;
				rotatePenDown = false;
				rotatePenUp = false;
				rotatePenLeft = false;
				rotatePenRight = false;
			} else {
				selectPen = true;
			}
		}
	}

	// Uncomment these lines to see the picking shader in effect
	//glfwSwapBuffers(window);
	//continue; // skips the normal rendering
}

glm::vec4 highlightObject(Vertex* Verts, float brightnessFactor) {
	float r = Verts->Color[0] + (1.0 - Verts->Color[0]) * brightnessFactor;
	float g = Verts->Color[1] + (1.0 - Verts->Color[1]) * brightnessFactor;
	float b = Verts->Color[2] + (1.0 - Verts->Color[2]) * brightnessFactor;
	glm::vec4 color = glm::vec4(r, g, b, 1.0);
	return color;
}

int initWindow(void)
{
	// Initialise GLFW
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(window_width, window_height, "Biswas, Sayak(54584911)", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	// Initialize the GUI
	TwInit(TW_OPENGL_CORE, NULL);
	TwWindowSize(window_width, window_height);
	TwBar * GUI = TwNewBar("Picking");
	TwSetParam(GUI, NULL, "refresh", TW_PARAM_CSTRING, 1, "0.1");
	TwAddVarRW(GUI, "Last picked object", TW_TYPE_STDSTRING, &gMessage, NULL);

	// Set up inputs
	glfwSetCursorPos(window, window_width / 2, window_height / 2);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetMouseButtonCallback(window, mouseCallback);

	return 0;
}

void initOpenGL(void)
{

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	// Projection matrix : 45\B0 Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	gProjectionMatrix = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	// Or, for an ortho camera :
	//gProjectionMatrix = glm::ortho(-4.0f, 4.0f, -3.0f, 3.0f, 0.0f, 100.0f); // In world coordinates

	// Camera matrix
	gViewMatrix = glm::lookAt(glm::vec3(10.0, 10.0, 10.0f),	// eye
		glm::vec3(0.0, 0.0, 0.0),	// center
		glm::vec3(0.0, 1.0, 0.0));	// up

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");
	pickingProgramID = LoadShaders("Picking.vertexshader", "Picking.fragmentshader");

	// Get a handle for our "MVP" uniform
	MatrixID = glGetUniformLocation(programID, "MVP");
	ModelMatrixID = glGetUniformLocation(programID, "M");
	ViewMatrixID = glGetUniformLocation(programID, "V");
	ProjMatrixID = glGetUniformLocation(programID, "P");

	PickingMatrixID = glGetUniformLocation(pickingProgramID, "MVP");
	// Get a handle for our "pickingColorID" uniform
	pickingColorArrayID = glGetUniformLocation(pickingProgramID, "PickingColorArray");
	pickingColorID = glGetUniformLocation(pickingProgramID, "PickingColor");
	// Get a handle for our "LightPosition" uniform
	LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
	Light1ID = glGetUniformLocation(programID, "Light1Position_worldspace");
	Light2ID = glGetUniformLocation(programID, "Light2Position_worldspace");

	createObjects();
}

void createVAOs(Vertex Vertices[], unsigned short Indices[], int ObjectId) {

	GLenum ErrorCheckValue = glGetError();
	const size_t VertexSize = sizeof(Vertices[0]);
	const size_t RgbOffset = sizeof(Vertices[0].Position);
	const size_t Normaloffset = sizeof(Vertices[0].Color) + RgbOffset;

	// Create Vertex Array Object
	glGenVertexArrays(1, &VertexArrayId[ObjectId]);	//
	glBindVertexArray(VertexArrayId[ObjectId]);		//

	// Create Buffer for vertex data
	glGenBuffers(1, &VertexBufferId[ObjectId]);
	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[ObjectId]);
	glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[ObjectId], Vertices, GL_STATIC_DRAW);

	// Create Buffer for indices
	if (Indices != NULL) {
		glGenBuffers(1, &IndexBufferId[ObjectId]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferId[ObjectId]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndexBufferSize[ObjectId], Indices, GL_STATIC_DRAW);
	}

	// Assign vertex attributes
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, VertexSize, 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)RgbOffset);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)Normaloffset);

	glEnableVertexAttribArray(0);	// position
	glEnableVertexAttribArray(1);	// color
	glEnableVertexAttribArray(2);	// normal

	// Disable our Vertex Buffer Object
	glBindVertexArray(0);

	ErrorCheckValue = glGetError();
	if (ErrorCheckValue != GL_NO_ERROR)
	{
		fprintf(
			stderr,
			"ERROR: Could not create a VBO: %s \n",
			gluErrorString(ErrorCheckValue)
			);
	}
}

void cleanup(void)
{
	// Cleanup VBO and shader
	for (int i = 0; i < NumObjects; i++) {
		glDeleteBuffers(1, &VertexBufferId[i]);
		glDeleteBuffers(1, &IndexBufferId[i]);
		glDeleteVertexArrays(1, &VertexArrayId[i]);
	}
	glDeleteProgram(programID);
	glDeleteProgram(pickingProgramID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// ATTN: MODIFY AS APPROPRIATE
	if (action == GLFW_PRESS) {
		switch (key)
		{
		case GLFW_KEY_1:
			if(selectArm1) {
				selectArm1 = false;
			} else {
				selectArm1 = true;
			}
			break;
		case GLFW_KEY_2:
			if(selectArm2) {
				selectArm2 = false;
			} else {
				selectArm2 = true;
			}
		case GLFW_KEY_A:
			break;
		case GLFW_KEY_B:
			if(selectBase) {
				selectBase = false;
			} else {
				selectBase = true;
			}
			break;
		case GLFW_KEY_C:
			if(selectCamera) {
				selectCamera = false;
				moveCameraDown = false;
				moveCameraUp = false;
				moveCameraLeft = false;
				moveCameraRight = false;
			} else {
				selectCamera = true;
			}
			break;
		case GLFW_KEY_D:
			break;
		case GLFW_KEY_J:
			animation = true;
			break;
		case GLFW_KEY_P:
			if(selectPen) {
				selectPen = false;
				rotatePenDown = false;
				rotatePenUp = false;
				rotatePenLeft = false;
				rotatePenRight = false;
			} else {
				selectPen = true;
			}
			break;
		case GLFW_KEY_W:
			break;
		case GLFW_KEY_S:
			break;
		case GLFW_KEY_T:
			if(selectTop) {
				selectTop = false;
			} else {
				selectTop = true;
			}
			break;
		case GLFW_KEY_SPACE:
			break;
		case GLFW_KEY_LEFT:
			if(selectCamera) {
				moveCameraLeft = true;
			}
			if(selectPen) {
				if(isShiftPressed) {
					rotatePenAxisLeft = true;
				} else {
					rotatePenLeft = true;
				}
			}
			if(selectBase) {
				slideBaseLeft = true;
			}
			if(selectTop) {
				rotateTopLeft = true;
			}
			break;
		case GLFW_KEY_RIGHT:
			if(selectCamera) {
				moveCameraRight = true;
			}
			if(selectPen) {
				if(isShiftPressed) {
					rotatePenAxisRight = true;
				} else {
					rotatePenRight = true;
				}
			}
			if(selectBase) {
				slideBaseRight = true;
			}
			if(selectTop) {
				rotateTopRight = true;
			}
			break;
		case GLFW_KEY_UP:
			if(selectCamera) {
				moveCameraUp = true;
			}
			if(selectPen) {
				rotatePenUp = true;
			}
			if(selectBase) {
				slideBaseBackward = true;
			}
			if(selectArm1) {
				rotateArm1Up = true;
			}
			if(selectArm2) {
				rotateArm2Up = true;
			}
			break;
		case GLFW_KEY_DOWN:
			if(selectCamera) {
				moveCameraDown = true;
			}
			if(selectPen) {
				rotatePenDown = true;
			}
			if(selectBase) {
				slideBaseForward = true;
			}
			if(selectArm1) {
				rotateArm1Down = true;
			}
			if(selectArm2) {
				rotateArm2Down = true;
			}
			break;
		case GLFW_KEY_LEFT_SHIFT:
			isShiftPressed = true;
			break;
		case GLFW_KEY_RIGHT_SHIFT:
			isShiftPressed = true;
			break;
		default:
			break;
		}
	} else if(action == GLFW_RELEASE) {
		switch(key) {
		case GLFW_KEY_LEFT:
			if(selectCamera) {
				moveCameraLeft = false;
			}
			if(selectBase) {
				slideBaseLeft = false;
			}
			if(selectPen) {
				rotatePenAxisLeft = false;
				rotatePenLeft = false;
			}
			if(selectTop) {
				rotateTopLeft = false;
			}
			break;
		case GLFW_KEY_RIGHT:
			if(selectCamera) {
				moveCameraRight = false;
			}
			if(selectBase) {
				slideBaseRight = false;
			}
			if(selectPen) {
				rotatePenAxisRight = false;
				rotatePenRight = false;
			}
			if(selectTop) {
				rotateTopRight = false;
			}
			break;
		case GLFW_KEY_UP:
			if(selectCamera) {
				moveCameraUp = false;
			}
			if(selectBase) {
				slideBaseBackward = false;
			}
			if(selectArm1) {
				rotateArm1Up = false;
			}
			if(selectArm2) {
				rotateArm2Up = false;
			}
			if(selectPen) {
				rotatePenUp = false;
			}
			break;
		case GLFW_KEY_DOWN:
			if(selectCamera) {
				moveCameraDown = false;
			}
			if(selectBase) {
				slideBaseForward = false;
			}
			if(selectArm1) {
				rotateArm1Down = false;
			}
			if(selectArm2) {
				rotateArm2Down = false;
			}
			if(selectPen) {
				rotatePenDown = false;
			}
			break;
		case GLFW_KEY_LEFT_SHIFT:
			isShiftPressed = false;
			break;
		case GLFW_KEY_RIGHT_SHIFT:
			isShiftPressed = false;
			break;
		default:
			break;
		}
	}
}

static void mouseCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		pickObject();
	}
}

int main(void)
{
	// initialize window
	int errorCode = initWindow();
	if (errorCode != 0)
		return errorCode;

	// initialize OpenGL pipeline
	initOpenGL();

	// For speed computation
	double lastTime = glfwGetTime();
	int nbFrames = 0;
	do {
		//// Measure speed
		//double currentTime = glfwGetTime();
		//nbFrames++;
		//if (currentTime - lastTime >= 1.0){ // If last prinf() was more than 1sec ago
		//	// printf and reset
		//	printf("%f ms/frame\n", 1000.0 / double(nbFrames));
		//	nbFrames = 0;
		//	lastTime += 1.0;
		//}

		if (animation){
			phi += 0.01;
			if (phi > 360)
				phi -= 360;
		}

		// DRAWING POINTS
		renderScene();


	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
	glfwWindowShouldClose(window) == 0);

	cleanup();

	return 0;
}
