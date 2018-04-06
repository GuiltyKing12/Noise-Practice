/*
 *  CSCI 444, Advanced Computer Graphics, Spring 2017
 *
 *  Project: a2_solution
 *  File: main.cpp
 *
 *  Description:
 *      Tessellation and Geometry Shaders
 *
 *  Author:
 *      Dr. Jeffrey Paone, Colorado School of Mines
 *  
 *  Notes:
 *
 */

//*************************************************************************************

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <stdlib.h>
#include <stdio.h>

#include <string>

#include "./include/BezierSurfaceUtils.h"
#include "./include/OpenGLUtils.h"
#include "./include/ShaderUtils.h"
#include "./include/ShaderProgram.h"

//*************************************************************************************

struct VertexColored {
	float x, y, z;
	float r, g, b;
	float s, t;
};

static const VertexColored cubeVertices[8] = {
		{ -0.5f, -0.5f, -0.5f, 0.f, 0.f, 0.f, 0.0f, 0.0f }, // 0 - bln
		{  0.5f, -0.5f, -0.5f, 0.f, 0.f, 0.f, 0.0f, 0.0f }, // 1 - brn
		{  0.5f,  0.5f, -0.5f, 0.f, 0.f, 0.f, 0.0f, 0.0f }, // 2 - trn
		{ -0.5f,  0.5f, -0.5f, 0.f, 0.f, 0.f, 0.0f, 0.0f }, // 3 - tln
		{ -0.5f, -0.5f,  0.5f, 0.f, 0.f, 0.f, 0.0f, 0.0f }, // 4 - blf
		{  0.5f, -0.5f,  0.5f, 0.f, 0.f, 0.f, 0.0f, 0.0f }, // 5 - brf
		{  0.5f,  0.5f,  0.5f, 0.f, 0.f, 0.f, 0.0f, 0.0f }, // 6 - trf
		{ -0.5f,  0.5f,  0.5f, 0.f, 0.f, 0.f, 0.0f, 0.0f }  // 7 - tlf
};
static const unsigned short cubeIndices[36] = {
		0, 2, 1,   0, 3, 2, // near
		1, 2, 5,   5, 2, 6, // right
		2, 7, 6,   3, 7, 2, // top
		0, 1, 4,   1, 5, 4, // bottom
		4, 5, 6,   4, 6, 7, // back
		0, 4, 3,   4, 7, 3  // left
};

static const VertexColored groundVertices[4] = {
		{ -15.0f, -5.0f, -15.0f, 0.0f, 1.0f, 0.0f,  0.0f,  0.0f }, // 0 - BL
		{  15.0f, -5.0f, -15.0f, 0.0f, 1.0f, 0.0f, 10.0f,  0.0f }, // 1 - BR
		{  15.0f, -5.0f,  15.0f, 0.0f, 1.0f, 0.0f, 10.0f, 10.0f }, // 2 - TR
		{ -15.0f, -5.0f,  15.0f, 0.0f, 1.0f, 0.0f,  0.0f, 10.0f }  // 3 - TL
};
static const unsigned short groundIndices[6] = {
	0, 2, 1, 	0, 3, 2
};

//*************************************************************************************
//
// Global Parameters

int windowWidth, windowHeight;
int octavenum = 4;
float persistencenum = -.7;
bool shiftDown = false;
bool leftMouseDown = false;
glm::vec2 mousePosition( -9999.0f, -9999.0f );

bool rotate = false;
bool controlPoints = false;
bool mesh = true;

glm::vec3 cameraAngles( 1.82f, 2.01f, 15.0f );
glm::vec3 eyePoint(   10.0f, 10.0f, 10.0f );
glm::vec3 lookAtPoint( 0.0f,  0.0f,  0.0f );
glm::vec3 upVector(    0.0f,  1.0f,  0.0f );

glm::vec3 materialKd( 0.7, 0.0, 0.0 );
glm::vec3 materialKa( 0.2, 0.2, 0.2 );
glm::vec3 materialKs( 0.0, 0.2, 0.0 );
glm::vec3 colorMix( 0.2, 0.2, 0.2 );
float materialShininess = 32;

glm::vec3 lightPosition( 3, 5, 3 );
glm::vec3 lightLd( 1, 1, 1 );
glm::vec3 lightLa( 1, 1, 1 );
glm::vec3 lightLs( 1, 1, 1 );

CSCI441::ShaderProgram *bezierPatchShaderProgram = NULL;
GLint modelview_location, mvp_location, vpos_location, normalmatrix_location, tessLevelOuter_location, tessLevelInner_location, octaves_location, persistence_location, mixColor_location;
GLint useOutline_location;
int useOutline = 0;

CSCI441::ShaderProgram *groundShaderProgram = NULL;
GLint gmodelview_location, gmvp_location;
GLint guseOutline_location;

CSCI441::ShaderProgram *pointShaderProgram = NULL;
GLint pmvp_location;

GLuint vertex_array_object_cube;
GLuint vertex_array_object_ground;

const unsigned int TEAPOT = 0, TEACUP = 1, TEASPOON = 2;
unsigned int CURRENT_TEA = 0;
GLuint vertex_array_object_teaset[3];

GLuint uniform_buffer_object_light, uniform_buffer_object_material;
GLuint lightBlockIndex, materialBlockIndex;
GLint lightBlockSize, *lightBlockOffset, materialBlockSize, *materialBlockOffset;
GLubyte *lightBlockBuffer, *materialBlockBuffer;

GLuint glightBlockIndex, gmaterialBlockIndex;
GLint glightBlockSize, *glightBlockOffset, gmaterialBlockSize, *gmaterialBlockOffset;
GLubyte *glightBlockBuffer, *gmaterialBlockBuffer;

glm::vec4 tessLevelOuter(4,4,4,4);
glm::vec2 tessLevelInner(4,4);
GLint maxTessLevel = 64;

//*************************************************************************************

void convertSphericalToCartesian() {
	eyePoint.x = cameraAngles.z * sinf( cameraAngles.x ) * sinf( cameraAngles.y );
	eyePoint.y = cameraAngles.z * -cosf( cameraAngles.y );
	eyePoint.z = cameraAngles.z * -cosf( cameraAngles.x ) * sinf( cameraAngles.y );
}

//*************************************************************************************

// Event Callbacks

static void error_callback(int error, const char* description) {
	fprintf(stderr, "[ERROR]: %s\n", description);
}

void modifyTessLevel( glm::vec4 &tessLevel, int pos, int change ) {
	if( change == 1 ) {
		switch( pos ) {
		case 0:
			tessLevel.x++;	if( tessLevel.x == maxTessLevel+1) tessLevel.x = maxTessLevel;	break;
		case 1:
			tessLevel.y++;	if( tessLevel.y == maxTessLevel+1) tessLevel.y = maxTessLevel;	break;
		case 2:
			tessLevel.z++;	if( tessLevel.z == maxTessLevel+1) tessLevel.z = maxTessLevel;	break;
		case 3:
			tessLevel.w++;	if( tessLevel.w == maxTessLevel+1) tessLevel.w = maxTessLevel;	break;
		}
	} else {
		switch( pos ) {
		case 0:
			tessLevel.x--;	if( tessLevel.x == 0 ) tessLevel.x = 1;	break;
		case 1:
			tessLevel.y--;	if( tessLevel.y == 0 ) tessLevel.y = 1;	break;
		case 2:
			tessLevel.z--;	if( tessLevel.z == 0 ) tessLevel.z = 1;	break;
		case 3:
			tessLevel.w--;	if( tessLevel.w == 0 ) tessLevel.w = 1;	break;
		}
	}
}

void modifyTessLevel( glm::vec2 &tessLevel, int pos, int change ) {
	if( change == 1 ) {
		switch( pos ) {
		case 0:
			tessLevel.x++;	if( tessLevel.x == maxTessLevel+1) tessLevel.x = maxTessLevel;	break;
		case 1:
			tessLevel.y++;	if( tessLevel.y == maxTessLevel+1) tessLevel.y = maxTessLevel;	break;
		}
	} else {
		switch( pos ) {
		case 0:
			tessLevel.x--;	if( tessLevel.x == 0 ) tessLevel.x = 1;	break;
		case 1:
			tessLevel.y--;	if( tessLevel.y == 0 ) tessLevel.y = 1;	break;
		}
	}
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if( action == GLFW_PRESS ) {
		switch( key ) {
		case GLFW_KEY_ESCAPE:
		case 'Q':
			glfwSetWindowShouldClose(window, GLFW_TRUE);
			break;

		case GLFW_KEY_LEFT_SHIFT:
			shiftDown = true;
			break;
		}
	} else if( action == GLFW_RELEASE ) {
		switch( key ) {
		case GLFW_KEY_LEFT_SHIFT:
			shiftDown = false;
			break;

		case 'W':
			useOutline = (useOutline ? 0 : 1);
			break;

		case '=':
			if( shiftDown ) {
				for( int i = 0; i < 4; i++ )
					modifyTessLevel( tessLevelOuter, i, 1 );
				for( int i = 0; i < 2; i++ )
					modifyTessLevel( tessLevelInner, i, 1 );
			}
			break;

		case '-':
			for( int i = 0; i < 4; i++ )
				modifyTessLevel( tessLevelOuter, i, -1 );
			for( int i = 0; i < 2; i++ )
				modifyTessLevel( tessLevelInner, i, -1 );
			break;

		case 'I':
			if( !shiftDown )
				for( int i = 0; i < 2; i++ )
					modifyTessLevel( tessLevelInner, i, 1 );
			else
				for( int i = 0; i < 2; i++ )
					modifyTessLevel( tessLevelInner, i, -1 );
			break;

		case 'O':
			if( !shiftDown )
				for( int i = 0; i < 4; i++ )
					modifyTessLevel( tessLevelOuter, i, 1 );
			else
				for( int i = 0; i < 4; i++ )
					modifyTessLevel( tessLevelOuter, i, -1 );
			break;

		case 'F':
			if( !shiftDown )
				modifyTessLevel( tessLevelOuter, 0, 1 );
			else
				modifyTessLevel( tessLevelOuter, 0, -1 );
			break;

		case 'G':
			if( !shiftDown )
				modifyTessLevel( tessLevelOuter, 1, 1 );
			else
				modifyTessLevel( tessLevelOuter, 1, -1 );
			break;

		case 'H':
			if( !shiftDown )
				modifyTessLevel( tessLevelOuter, 2, 1 );
			else
				modifyTessLevel( tessLevelOuter, 2, -1 );
			break;

		case 'J':
			if( !shiftDown )
				modifyTessLevel( tessLevelOuter, 3, 1 );
			else
				modifyTessLevel( tessLevelOuter, 3, -1 );
			break;

		case 'K':
			if( !shiftDown )
				modifyTessLevel( tessLevelInner, 0, 1 );
			else
				modifyTessLevel( tessLevelInner, 0, -1 );
			break;

		case 'L':
			if( !shiftDown )
				modifyTessLevel( tessLevelInner, 1, 1 );
			else
				modifyTessLevel( tessLevelInner, 1, -1 );
			break;

		case 'R':
			rotate = !rotate;
			break;

		case 'C':
			controlPoints = !controlPoints;
			break;

		case 'M':
			mesh = !mesh;
			break;

        case 'X':
            if( !shiftDown )
                octavenum++;
            else {
                octavenum--;
                if(octavenum == 0) octavenum = 1;
            }
                fprintf(stdout, "octaves: %d, persis: %2.2f\n", octavenum, persistencenum);
            break;
            
        case 'Z':
            if( !shiftDown )
                persistencenum -= .05;
            else {
                persistencenum += .05;
            }
                fprintf(stdout, "octaves: %d, persis: %2.2f\n", octavenum, persistencenum);
            break;
                
		case '1':
			CURRENT_TEA = TEAPOT;
			break;

		case '2':
			CURRENT_TEA = TEACUP;
			break;

		case '3':
			CURRENT_TEA = TEASPOON;
			break;
		}
	}

}

static void mouseClick_callback(GLFWwindow* window, int button, int action, int mods) {
	if( button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS ) {
		leftMouseDown = true;
	} else {
		leftMouseDown = false;
		mousePosition.x = -9999.0f;
		mousePosition.y = -9999.0f;
	}
}

static void mousePos_callback(GLFWwindow* window, double xpos, double ypos) {
	// make sure movement is in bounds of the window
	// glfw captures mouse movement on entire screen
	if( xpos > 0 && xpos < windowWidth ) {
		if( ypos > 0 && ypos < windowHeight ) {
			// active motion
			if( leftMouseDown ) {
				if( (mousePosition.x - -9999.0f) < 0.001f ) {
					mousePosition.x = xpos;
					mousePosition.y = ypos;
				} else {
					if( !shiftDown ) {
						cameraAngles.x += (xpos - mousePosition.x)*0.005f;
						cameraAngles.y += (ypos - mousePosition.y)*0.005f;

						if( cameraAngles.y < 0 ) cameraAngles.y = 0.0f + 0.001f;
						if( cameraAngles.y >= M_PI ) cameraAngles.y = M_PI - 0.001f;
					} else {
						double totChgSq = (xpos - mousePosition.x) + (ypos - mousePosition.y);
						cameraAngles.z += totChgSq*0.01f;

						if( cameraAngles.z <= 2.0f ) cameraAngles.z = 2.0f;
						if( cameraAngles.z >= 50.0f ) cameraAngles.z = 50.0f;
					}
					convertSphericalToCartesian();


					mousePosition.x = xpos;
					mousePosition.y = ypos;
				}
			}
			// passive motion
			else {

			}
		}
	}
}

static void scroll_callback(GLFWwindow* window, double xOffset, double yOffset ) {
	double totChgSq = yOffset;
	cameraAngles.z += totChgSq*0.01f;

	if( cameraAngles.z <= 2.0f ) cameraAngles.z = 2.0f;
	if( cameraAngles.z >= 50.0f ) cameraAngles.z = 50.0f;
	
	convertSphericalToCartesian();
}

//*************************************************************************************

void checkOpenGLError( std::string msg = "[WARN]" ) {
	GLenum error = glGetError();
	if(error != GL_NO_ERROR) {
		fprintf(stderr, "%s: OpenGL Error %d: %s\n", msg.c_str(), error, glewGetErrorString(error));
	}
}

GLFWwindow* setupGLFW() {
	glfwSetErrorCallback(error_callback);

	if (!glfwInit()) {
		fprintf( stderr, "[ERROR]: Could not initialize GLFW\n" );
		exit(EXIT_FAILURE);
	}

	//glfwWindowHint(GLFW_DEPTH_BITS, 24);

	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

	GLFWwindow *window = glfwCreateWindow(640, 480, "bHer_A3", NULL, NULL);
	if( !window ) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouseClick_callback);
	glfwSetCursorPosCallback(window, mousePos_callback);
	glfwSetScrollCallback(window, scroll_callback);

	return window;
}

void setupOpenGL() {
	glEnable( GL_DEPTH_TEST );
	glDepthFunc( GL_LESS );
	glFrontFace( GL_CCW );
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

	glewExperimental = GL_TRUE;
	GLenum glewResult = glewInit();

	/* check for an error */
	if( glewResult != GLEW_OK ) {
		printf( "[ERROR]: Error initalizing GLEW\n");
		exit(EXIT_FAILURE);
	}

	glGetIntegerv(GL_MAX_TESS_GEN_LEVEL, &maxTessLevel);

	CSCI441::OpenGLUtils::printOpenGLInfo();
}

void setupShaders() {
	// ground program
	groundShaderProgram      = new CSCI441::ShaderProgram( "shaders/wireframe.v.glsl",
														   "shaders/wireframe.g.glsl",
														   "shaders/wireframe.f.glsl" );
	gmodelview_location      = groundShaderProgram->getUniformLocation( "modelviewMatrix" );
	gmvp_location            = groundShaderProgram->getUniformLocation( "MVP_Matrix" );
	guseOutline_location     = groundShaderProgram->getUniformLocation( "useOutline" );

	// bezier program
	bezierPatchShaderProgram = new CSCI441::ShaderProgram( "shaders/bezier.v.glsl",
														   "shaders/bezier.tc.glsl",
														   "shaders/bezier.te.glsl",
														   "shaders/bezier.g.glsl",
														   "shaders/bezier.f.glsl" );
	modelview_location       = bezierPatchShaderProgram->getUniformLocation( "modelviewMatrix" );
	mvp_location             = bezierPatchShaderProgram->getUniformLocation( "MVP_Matrix" );
	normalmatrix_location    = bezierPatchShaderProgram->getUniformLocation( "normalMatrix" );
	useOutline_location      = bezierPatchShaderProgram->getUniformLocation( "useOutline" );
	tessLevelOuter_location  = bezierPatchShaderProgram->getUniformLocation( "tessLevelOuter" );
	tessLevelInner_location  = bezierPatchShaderProgram->getUniformLocation( "tessLevelInner" );
    octaves_location = bezierPatchShaderProgram->getUniformLocation( "numoctaves" );
    persistence_location = bezierPatchShaderProgram->getUniformLocation( "persistenceset" );
    mixColor_location = bezierPatchShaderProgram->getUniformLocation( "mixColor" );

	vpos_location            = bezierPatchShaderProgram->getAttributeLocation( "vPos" );


	// uniform blocks
	lightBlockSize           = bezierPatchShaderProgram->getUniformBlockSize( "LightInfo" );
	lightBlockBuffer         = bezierPatchShaderProgram->getUniformBlockBuffer( "LightInfo" );
	bezierPatchShaderProgram->setUniformBlockBinding( "LightInfo", 0 );

	glightBlockSize          = groundShaderProgram->getUniformBlockSize( "LightInfo" );
	glightBlockBuffer        = groundShaderProgram->getUniformBlockBuffer( "LightInfo" );
	groundShaderProgram->setUniformBlockBinding( "LightInfo", 0 );

	const GLchar *names[] = { "LightInfo.position", "LightInfo.La", "LightInfo.Ld", "LightInfo.Ls" };
	lightBlockOffset         = bezierPatchShaderProgram->getUniformBlockOffsets( "LightInfo", names );
	glightBlockOffset        = groundShaderProgram->getUniformBlockOffsets( "LightInfo", names );


	materialBlockSize        = bezierPatchShaderProgram->getUniformBlockSize( "MaterialInfo" );
	materialBlockBuffer      = bezierPatchShaderProgram->getUniformBlockBuffer( "MaterialInfo" );
	bezierPatchShaderProgram->setUniformBlockBinding( "MaterialInfo", 1 );

	gmaterialBlockSize       = groundShaderProgram->getUniformBlockSize( "MaterialInfo" );
	gmaterialBlockBuffer     = groundShaderProgram->getUniformBlockBuffer( "MaterialInfo" );
	groundShaderProgram->setUniformBlockBinding( "MaterialInfo", 1 );

	const GLchar *names2[] = { "Ka", "Kd", "Ks", "shininess" };
	materialBlockOffset      = bezierPatchShaderProgram->getUniformBlockOffsets( "MaterialInfo", names2 );
	gmaterialBlockOffset     = groundShaderProgram->getUniformBlockOffsets( "MaterialInfo", names2 );


	pointShaderProgram       = new CSCI441::ShaderProgram( "shaders/point.v.glsl",
														   "shaders/point.f.glsl" );
	pmvp_location            = pointShaderProgram->getUniformLocation( "MVP_Matrix" );
}

void setupBuffers() {
	// Draw Cube
	glGenVertexArrays(1, &vertex_array_object_cube);
	glBindVertexArray(vertex_array_object_cube);

	GLuint vertex_buffer_cube;
	glGenBuffers(1, &vertex_buffer_cube);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_cube);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

	GLuint element_buffer_cube;
	glGenBuffers(1, &element_buffer_cube);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_cube);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(vpos_location);
	glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*) 0);

	// Draw Ground
	glGenVertexArrays(1, &vertex_array_object_ground);
	glBindVertexArray(vertex_array_object_ground);

	GLuint vertex_buffer_ground;
	glGenBuffers(1, &vertex_buffer_ground);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_ground);
	glBufferData(GL_ARRAY_BUFFER, sizeof(groundVertices), groundVertices, GL_STATIC_DRAW);

	GLuint element_buffer_ground;
	glGenBuffers(1, &element_buffer_ground);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_ground);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(groundIndices), groundIndices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(vpos_location);
	glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*) 0);




	memcpy( lightBlockBuffer + lightBlockOffset[0], &lightPosition[0], 3*sizeof(float) );
	memcpy( lightBlockBuffer + lightBlockOffset[1], &lightLa[0], 3*sizeof(float) );
	memcpy( lightBlockBuffer + lightBlockOffset[2], &lightLd[0], 3*sizeof(float) );
	memcpy( lightBlockBuffer + lightBlockOffset[3], &lightLs[0], 3*sizeof(float) );

	glGenBuffers( 1, &uniform_buffer_object_light );
	glBindBuffer( GL_UNIFORM_BUFFER, uniform_buffer_object_light );
	glBufferData( GL_UNIFORM_BUFFER, lightBlockSize, lightBlockBuffer, GL_DYNAMIC_DRAW );
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniform_buffer_object_light);


	memcpy( materialBlockBuffer + materialBlockOffset[0], &materialKa[0], 3*sizeof(float));
	memcpy( materialBlockBuffer + materialBlockOffset[1], &materialKd[0], 3*sizeof(float));
	memcpy( materialBlockBuffer + materialBlockOffset[2], &materialKs[0], 3*sizeof(float));
	memcpy( materialBlockBuffer + materialBlockOffset[3], &materialShininess, sizeof(float));

	glGenBuffers( 1, &uniform_buffer_object_material );
	glBindBuffer( GL_UNIFORM_BUFFER, uniform_buffer_object_material );
	glBufferData( GL_UNIFORM_BUFFER, materialBlockSize, materialBlockBuffer, GL_DYNAMIC_DRAW );
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, uniform_buffer_object_material);


	// load teapot
	vertex_array_object_teaset[TEAPOT] = CSCI441::BezierSurfaceUtils::readSurfaceFile( "models/teapot" );
	glBindVertexArray( vertex_array_object_teaset[TEAPOT] );

	glEnableVertexAttribArray(vpos_location);
	glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), 0);

	// load teacup
	vertex_array_object_teaset[TEACUP] = CSCI441::BezierSurfaceUtils::readSurfaceFile( "models/teacup" );
	glBindVertexArray( vertex_array_object_teaset[TEACUP] );

	glEnableVertexAttribArray(vpos_location);
	glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), 0);

	// load teaspoon
	vertex_array_object_teaset[TEASPOON] = CSCI441::BezierSurfaceUtils::readSurfaceFile( "models/teaspoon" );
	glBindVertexArray( vertex_array_object_teaset[TEASPOON] );

	glEnableVertexAttribArray(vpos_location);
	glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), 0);
}

int main( int argc, char *argv[] ) {
	GLFWwindow *window = setupGLFW();
	setupOpenGL();
	setupShaders();
	setupBuffers();
	
	convertSphericalToCartesian();

    glEnable( GL_TEXTURE_2D );
	glActiveTexture( GL_TEXTURE0 );

	printf("\n[INFO]: ------------------------------------------\n" );
	printf("[INFO]: Controls:\n" );
	printf("[INFO]: \t1-3: Change Object\n\n" );
	printf("[INFO]: \t-/+: Decrease/Increase All Tess Levels\n\n" );
	printf("[INFO]: \tO/o: Decrease/Increase Outer Tess Levels\n" );
	printf("[INFO]: \tF/f: Decrease/Increase Outer Tess Level 1\n" );
	printf("[INFO]: \tG/g: Decrease/Increase Outer Tess Level 2\n" );
	printf("[INFO]: \tH/h: Decrease/Increase Outer Tess Level 3\n" );
	printf("[INFO]: \tJ/j: Decrease/Increase Outer Tess Level 4\n\n" );
	printf("[INFO]: \tI/i: Decrease/Increase Inner Tess Levels\n" );
	printf("[INFO]: \tK/k: Decrease/Increase Inner Tess Level 1\n" );
	printf("[INFO]: \tL/l: Decrease/Increase Inner Tess Level 2\n\n" );
	printf("[INFO]: \tc:   Toggle Control Points On/Off\n" );
	printf("[INFO]: \tr:   Toggle Rotation On/Off\n" );
	printf("[INFO]: \tw:   Toggle Wireframe On/Off\n" );
	printf("[INFO]: \tm:   Toggle Mesh On/Off\n" );
	printf("\n[INFO]: ------------------------------------------\n" );

	while( !glfwWindowShouldClose(window) ) {
		float ratio;
		glm::mat4 m, v, p;
		glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
		float time = glfwGetTime();


		// to change light position and color
		lightPosition.x = 3 * sin(time);
		lightPosition.z = 3 * cos(time);
		memcpy( lightBlockBuffer + lightBlockOffset[0], &lightPosition[0], 3*sizeof(float) );
		glBindBuffer( GL_UNIFORM_BUFFER, uniform_buffer_object_light );
		glBufferData( GL_UNIFORM_BUFFER, lightBlockSize, lightBlockBuffer, GL_DYNAMIC_DRAW );

		materialKd = glm::vec3( 0.2, 0.7, 0.6 );
		memcpy( materialBlockBuffer + materialBlockOffset[1], &materialKd[0], 3*sizeof(float));
		glBindBuffer( GL_UNIFORM_BUFFER, uniform_buffer_object_material );
		glBufferData( GL_UNIFORM_BUFFER, materialBlockSize, materialBlockBuffer, GL_DYNAMIC_DRAW );

		ratio = windowWidth / (float) windowHeight;
		glViewport(0, 0, windowWidth, windowHeight);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		p = glm::perspective( 45.0f, ratio, 0.001f, 100.0f );
		v = glm::lookAt( eyePoint,lookAtPoint, upVector );

		glm::mat4 mv = v * m;
		glm::mat4 mvp = p * mv;

		groundShaderProgram->useProgram();
		glUniformMatrix4fv(gmodelview_location, 1, GL_FALSE, &mv[0][0]);
		glUniformMatrix4fv(gmvp_location, 1, GL_FALSE, &mvp[0][0]);
		glUniform1i( guseOutline_location, useOutline );

		glBindVertexArray(vertex_array_object_ground);
		glDrawElements(GL_TRIANGLES, sizeof(groundIndices)/sizeof(unsigned short), GL_UNSIGNED_SHORT, (void*)0);

		materialKd = glm::vec3( 0.4, 0.9, 0.9 );
		memcpy( materialBlockBuffer + materialBlockOffset[1], &materialKd[0], 3*sizeof(float));
		glBindBuffer( GL_UNIFORM_BUFFER, uniform_buffer_object_material );
		glBufferData( GL_UNIFORM_BUFFER, materialBlockSize, materialBlockBuffer, GL_DYNAMIC_DRAW );

		if( rotate )
			m = glm::rotate(m, time, glm::vec3(sin(time), cos(time), sin(time)-cos(time)));

		mv = v * m;
		mvp = p * mv;

		glm::mat3 nm = glm::transpose( glm::inverse( glm::mat3( v * m ) ) );

		bezierPatchShaderProgram->useProgram();
		glUniformMatrix4fv(modelview_location, 1, GL_FALSE, &mv[0][0]);
		glUniformMatrix4fv(mvp_location, 1, GL_FALSE, &mvp[0][0]);
		glUniformMatrix3fv(normalmatrix_location, 1, GL_FALSE, &nm[0][0]);
		glUniform1i( useOutline_location, useOutline );
		glUniform4fv( tessLevelOuter_location, 1, &tessLevelOuter[0] );
		glUniform2fv( tessLevelInner_location, 1, &tessLevelInner[0] );
        glUniform3fv( mixColor_location, 1, &colorMix[0] );
        glUniform1i( octaves_location, octavenum );
        glUniform1f( persistence_location, persistencenum );

		glBindVertexArray( vertex_array_object_teaset[ CURRENT_TEA ] );
		glPatchParameteri(GL_PATCH_VERTICES, 16);
		if( mesh )
			glDrawElements( GL_PATCHES, 32*16, GL_UNSIGNED_INT, (void*)0 );

		if( controlPoints ) {
			pointShaderProgram->useProgram();
			glPointSize(4.0);
			glUniformMatrix4fv(pmvp_location, 1, GL_FALSE, &mvp[0][0]);
			glDrawArrays( GL_POINTS, 0, 32*16*3 );
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	delete bezierPatchShaderProgram;
	delete pointShaderProgram;
	delete groundShaderProgram;

	return EXIT_SUCCESS;
}
