/** The usual stuff... */
#ifndef COMMON_CPP
#define COMMON_CPP

#include <array>
#include <fstream>
#include <GL/glew.h>
#include <gl/GLU.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <iostream>
/** Szükséges az M_PI használatához. */
/** Needed for using M_PI. */
#define _USE_MATH_DEFINES
#include <math.h>
#include <SOIL2/SOIL2.h>
#include <sstream>
#include <vector>

enum eProjection {
	Orthographic,
	Perspective
};

using namespace	std;
using namespace glm;
/** Egyedi implementációk az egyes példaprogramokban... */
/** These are unique for the samples, defined there... */
extern void		framebufferSizeCallback(GLFWwindow* window, int width, int height);
extern void		keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
extern void		cursorPosCallback(GLFWwindow* window, double xPos, double yPos);
extern void		mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
extern GLchar	windowTitle[];
/** ShaderInfo tárolja a típust (GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, etc...), a filet és az ID-t. */
/** ShaderInfo defines the type (GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, etc...), the file for it and ID. */
typedef struct {
	GLenum       type;
	const GLchar *fileName;
	GLuint       shader;
} ShaderInfo;
/** Vertex buffer object ID, vertex array object ID és program ID az adattároláshoz. */
/** Vertex buffer object ID, vertex array object ID and program ID for data storing. */
GLuint			VAO[VAOCount]			= {0};
GLuint			BO[BOCount]				= {0};
GLuint			program[ProgramCount]	= {0};
GLuint			texture[TextureCount]	= {0};
/** Az OpenGL ablak szélesség és magasság értéke. */
/** Width and height of the OpenGL window. */
const GLdouble	worldSize		= 1.0f;
GLint			windowWidth		= 600;
GLint			windowHeight	= 600;
/** A shader változók location-jei és a CPU oldali változóik. */
/** Shader variable locations and the corresponding CPU variables. */
GLuint			locationMatModel, locationMatView, locationMatProjection, locationMatModelView;
mat4			matModel, matView, matProjection, matModelView;
eProjection		projectionType = Orthographic;
/** A normál billentyûk a [0..255] tartományban vannak, a nyilak és a speciális billentyûk pedig a [256..511] tartományban helyezkednek el. */
/** Normal keys are fom [0..255], arrow and special keys are from [256..511]. */
GLboolean		keyboard[512]	= { GL_FALSE };
GLint			modifiers;		// handles CTRL, ALT, SHIFT, etc...
GLFWwindow		*window			= nullptr;

/** Felesleges objektumok törlése. */
/** Clenup the unnecessary objects. */
void cleanUpScene(int returnCode) {
	/** Töröljük a texture objektumokat. */
	/** Destroy the texture objects. */
	glDeleteTextures(TextureCount, texture);
	/** Töröljük a vertex array és a vertex buffer objektumokat. */
	/** Destroy the vertex array and vertex buffer objects. */
	glDeleteVertexArrays(VAOCount, VAO);
	glDeleteBuffers(BOCount, BO);
	/** Töröljük a shader programo(ka)t. */
	/** Let's delete the shader program(s). */
	for (int enumItem = 0; enumItem < ProgramCount; enumItem++)
		glDeleteProgram(enumItem);
	/** Töröljük a GLFW ablakot. Leállítjuk a GLFW-t. */
	/** Destroy the GLFW window. Stop the GLFW system. */
	glfwTerminate();
	/** Kilépés returnCode kóddal. */
	/** Stop the software and exit with returnCode code. */
	exit(returnCode);
}

GLboolean checkOpenGLError() {
	GLboolean	foundError = GL_FALSE;
	GLenum		glErr;

	while ((glErr = glGetError()) != GL_NO_ERROR) {
		cerr << "glError: " << gluErrorString(glErr) << endl;
		foundError = GL_TRUE;
	}

	return foundError;
}

GLvoid checkShaderLog(GLuint shader) {
	GLint	compiled;
	GLint	length = 0;

	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (compiled) return;

	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
	if (length > 0) {
		GLchar	*log = (GLchar*)calloc((size_t)length + 1, sizeof(GLchar)); // length + 1 for zero terminated string

		glGetShaderInfoLog(shader, length, nullptr, log);
		cerr << "Shader compiling failed, info log: " << log << endl;
		free(log);
	}
	cleanUpScene(EXIT_FAILURE);
}

GLvoid checkProgramLog(GLint prog, ShaderInfo *shaders) {
	GLint	linked;
	GLint	length = 0;

	glGetProgramiv(prog, GL_LINK_STATUS, &linked);
	if (linked) return;
	for (ShaderInfo	*entry = shaders; entry->type != GL_NONE; ++entry) {
		glDeleteShader(entry->shader);
		entry->shader = 0;
	}

	glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &length);
	if (length > 0) {
		GLchar	*log = (GLchar*)calloc((size_t)length + 1, sizeof(GLchar)); // length + 1 for zero terminated string

		glGetProgramInfoLog(prog, length, nullptr, log);
		cerr << "Shader linking failed, info log: " << log << endl;
		free(log);
	}
	cleanUpScene(EXIT_FAILURE);
}

string ReadShader(const GLchar *filename) {

	ifstream		t(filename);
	stringstream	buffer;

	if (t.fail()) cleanUpScene(EXIT_FAILURE);

	buffer << t.rdbuf();

	return buffer.str();
}

GLuint LoadShaders(ShaderInfo *shaders) {
	if (shaders == nullptr) return 0; // 0 = NOT valid program
	GLuint		program	= glCreateProgram();
	ShaderInfo	*entry	= shaders;
	while (entry->type != GL_NONE) {
		GLuint			shader = glCreateShader(entry->type);
		string			sourceString = ReadShader(entry->fileName);
		const GLchar	*source = sourceString.c_str();

		entry->shader = shader;

		if (source == nullptr) {
			for (entry = shaders; entry->type != GL_NONE; ++entry) {
				glDeleteShader(entry->shader);
				entry->shader = 0;
			}
			cleanUpScene(EXIT_FAILURE);
		}

		glShaderSource(shader, 1, &source, nullptr);
		glCompileShader(shader);
		checkShaderLog(shader);

		glAttachShader(program, shader);

		++entry;
	}

	glLinkProgram(program);
	checkProgramLog(program, shaders);

	for (entry = shaders; entry->type != GL_NONE; ++entry) {
		glDeleteShader(entry->shader);
		entry->shader = 0;
	}

	return program;
}
/** Az alkalmazáshoz kapcsolódó elõkészítõ lépések. */
/** The first initialization steps of the program. */
void init(GLint major, GLint minor, GLint profile) {
	/** Próbáljuk meg inicializálni a GLFW-t! */
	/** Try to initialize GLFW! */
	if (!glfwInit()) cleanUpScene(EXIT_FAILURE);
	/** A használni kívánt OpenGL verziót beállítjuk. */
	/** The needed OpenGL version is set up. */
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
	/** GLFW_OPENGL_CORE_PROFILE-t használunk alapesetben. A közvetlen mód glBegin és glEnd között glVertex hívásokat használ, ekkor GLFW_OPENGL_COMPAT_PROFILE szükséges. */
	/** GLFW_OPENGL_CORE_PROFILE is default. Immediate mode is using glBegin and glEnd with glVertex in between them, then GLFW_OPENGL_COMPAT_PROFILE is needed. */
	glfwWindowHint(GLFW_OPENGL_PROFILE, profile);
#ifdef __APPLE__	// To make macOS happy; should not be needed.
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	/** Próbáljuk meg létrehozni az ablakunkat. */
	/** Let's try to create a window for drawing. */
	/** GLFWwindow* glfwCreateWindow(int width, int height, const char* title, GLFWmonitor * monitor, GLFWwindow * share) */
	if ((window = glfwCreateWindow(windowWidth, windowHeight, windowTitle, nullptr, nullptr)) == nullptr) {
		cerr << "Failed to create GLFW window." << endl;
		cleanUpScene(EXIT_FAILURE);
	}
	/** Válasszuk ki az ablakunk OpenGL kontextusát, hogy használhassuk. */
	/** Select the OpenGL context (window) for drawing. */
	glfwMakeContextCurrent(window);
	/** A képernyõ átméretezés kezelése. */
	/** Callback function for window size change. */
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	/** Billentyûzethez köthetõ események kezelése. */
	/** Callback function for keyboard events. */
	glfwSetKeyCallback(window, keyCallback);
	/** Az egér mutató helyét kezelõ függvény megadása. */
	/** Callback function for mouse position change. */
	glfwSetCursorPosCallback(window, cursorPosCallback);
	/** Az egér gombjaihoz köthetõ események kezelése. */
	/** Callback function for mouse button events. */
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	/** Incializáljuk a GLEW-t, hogy elérhetõvé váljanak az OpenGL függvények, probléma esetén kilépés EXIT_FAILURE értékkel. */
	/** Initalize GLEW, so the OpenGL functions will be available, on problem exit with EXIT_FAILURE code. */
	if (glewInit() != GLEW_OK) {
		cerr << "Failed to init the GLEW system." << endl;
		cleanUpScene(EXIT_FAILURE);
	}
	/** 0 = v-sync kikapcsolva, 1 = v-sync bekapcsolva, n = n db képkockányi idõt várakozunk */
	/** 0 = v-sync off, 1 = v-sync on, n = n pieces frame time waiting */
	glfwSwapInterval(1);
	/** A window ablak minimum és maximum szélességének és magasságának beállítása. */
	/** The minimum and maximum width and height values of the window object. */
	glfwSetWindowSizeLimits(window, 400, 400, 3840, 2160);
	/** Elsődleges monitor és annak üzemmód lekérdezése. */
	/** Query primary monitor and its working resolution. */
	GLFWmonitor			*monitor	= glfwGetPrimaryMonitor();
	const GLFWvidmode	*mode		= glfwGetVideoMode(monitor);
	/** Ablak középre helyezése. */
	/** Putting window in the center. */
	glfwSetWindowPos(window, (mode->width - windowWidth) / 2, (mode->height - windowHeight) / 2);
	/** Létrehozzuk a szükséges vertex buffer és vertex array objektumokat. */
	/** Create the vertex buffer and vertex array objects. */
	glGenBuffers(BOCount, BO);
	glGenVertexArrays(VAOCount, VAO);
	/** A GL_POINT_SMOOTH funkció bekapcsolása után négyzet helyett kör alakú pontokat kapunk. */
	/** Enabling GL_POINT_SMOOTH produce circle points instead of rectangles. */
	//glEnable(GL_POINT_SMOOTH);
	//glEnable(GL_LINE_SMOOTH);
	/** Ha pontokat rajzolunk, kísérletezzünk a pont méretének növelésével! */
	/** When drawing points, we can make experiments with different point sizes! */
	//glPointSize(32.0);
	/** Ha vonalakat rajzolunk, módosítsuk a vastagságát! */
	/** When drawing lines, we can make experiments with different widths! */
	//glLineWidth(16.0);
	/** Állítsuk be a törlési színt az áttetszõségi értékkel együtt! [0.0, 1.0] */
	/** Set the clear color (red, green, blue, alpha), where alpha is transparency! [0.0, 1.0] */
	glClearColor(GLclampf(212.0 / 255.0), GLclampf(175.0 / 255.0), GLclampf(55.0 / 255.0), 1.0); // gold
	/** Állítsuk be a számok kiírási pontosságát 2 tizedesre. */
	/** Set feedback number precision to 2 decimal. */
	cout.precision(2);
	cout << fixed;
}
#endif