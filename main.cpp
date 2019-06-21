#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <functional>
#include "ObjModel.h"
#pragma comment(lib, "glew32.lib")

void checkShaderErrors(GLuint shaderId, const std::string &filename);
void GLAPIENTRY MessageCallback(GLenum ,GLenum ,GLuint, GLenum,	GLsizei,const GLchar*,const void* );

int debugLoadCounter = 0;
//This class is very, very incomplete
class Shader
{
	std::map<std::string, GLuint> uniforms;
public:

	GLuint programId;
	std::function<void()> uniformSetter;
	
	Shader(std::string vs, std::string fs)
	{
		debugLoadCounter++;
		std::ifstream vertexShaderFile(vs);
		std::string vertexShaderData((std::istreambuf_iterator<char>(vertexShaderFile)), std::istreambuf_iterator<char>());
		const char* cvertexShaderData = vertexShaderData.data();

		std::ifstream fragShaderFile(fs);
		std::string fragShaderData((std::istreambuf_iterator<char>(fragShaderFile)), std::istreambuf_iterator<char>());
		const char* cfragShaderData = fragShaderData.c_str();

		programId = glCreateProgram();							// maak een shaderprogramma aan

		GLuint vertexId = glCreateShader(GL_VERTEX_SHADER);		// maak vertex shader aan
		glShaderSource(vertexId, 1, &cvertexShaderData, NULL);		// laat opengl de shader uit de variabele 'vertexShader' halen
		glCompileShader(vertexId);								// compileer de shader
		checkShaderErrors(vertexId, vs);							// controleer of er fouten zijn opgetreden bij het compileren
		glAttachShader(programId, vertexId);					// hang de shader aan het shaderprogramma


		GLuint fragmentId = glCreateShader(GL_FRAGMENT_SHADER);		// maak fragment shader aan
		glShaderSource(fragmentId, 1, &cfragShaderData, NULL);	// laat opengl de shader uit de variabele 'fragmentShader' halen
		glCompileShader(fragmentId);							// compileer de shader
		checkShaderErrors(fragmentId, fs);							// controleer of er fouten zijn opgetreden bij het compileren
		glAttachShader(programId, fragmentId);					// hang de shader aan het shaderprogramma

		glBindAttribLocation(programId, 0, "InputPosition");		// zet de positie op vertex attribuut 0
		glBindAttribLocation(programId, 2, "InputTexcoord");			// zet de kleur op vertex attribuut 1
		glBindAttribLocation(programId, 1, "InputNormal");		// zet de texcoord op vertex attribuut 2
		glLinkProgram(programId);								// link het programma, zorg dat alle attributes en varying gelinked zijn
		glUseProgram(programId);								// Zet dit als actieve programma
	}

	GLuint getUniform(const std::string &name)
	{
		auto it = uniforms.find(name);
		if (it != uniforms.end())
			return it->second;
		GLuint location = glGetUniformLocation(programId, name.c_str());
		uniforms[name] = location;
		return location;
	}

	Shader* setUniforms()
	{
		uniformSetter();
		return this;
	}

	Shader* setActive()
	{
		glUseProgram(programId);
		return this;
	}
};

std::vector<ObjModel*> models;
std::vector<float> distances;
int activeModel = 0;

glm::ivec2 screenSize;
float rotation;
int lastTime;

void checkShaderErrors(GLuint shaderId, const std::string &filename)
{
	GLint status;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &status);					//kijk of het compileren is gelukt
	if (status == GL_FALSE)
	{
		int length, charsWritten;
		glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &length);				//haal de lengte van de foutmelding op
		char* infolog = new char[length + 1];
		memset(infolog, 0, length + 1);
		glGetShaderInfoLog(shaderId, length, &charsWritten, infolog);		//en haal de foutmelding zelf op
		std::cout << "Error compiling shader "<<filename<<":\n" << infolog << std::endl;
		delete[] infolog;
	}
}


#ifdef WIN32
void GLAPIENTRY onDebug(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
#else
void onDebug(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam)
#endif
{
	std::cout << "onDebug error:" << message << std::endl;
}

//lightData
std::vector<glm::vec3> LightLocation;
std::vector<GLfloat> LightAttanuation;
std::vector<glm::vec4> LightColour;

Shader* myShader;
void init()
{
	glewInit();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);
	glClearColor(0,0,0, 1.0f);

	myShader = new Shader("assets/shaders/My_subsurface_scattering.vert", "assets/shaders/My_subsurface_scattering.frag");
	models.push_back(new ObjModel("assets/models/ape/ape.obj"));
	distances.push_back(2);
	models.push_back(new ObjModel("assets/models/ship/shipA_OBJ.obj"));
	distances.push_back(50);
	models.push_back(new ObjModel("assets/models/car/honda_jazz.obj"));
	distances.push_back(150);
	models.push_back(new ObjModel("assets/models/normalstuff/normaltest.obj"));
	distances.push_back(2);
	models.push_back(new ObjModel("assets/models/normalstuff/normaltest2.obj"));

	distances.push_back(2);

	LightLocation = std::vector<glm::vec3>();
	LightAttanuation = std::vector<GLfloat>();
	LightColour = std::vector<glm::vec4>();

	LightLocation.push_back(glm::vec3(-4, 0, -4));
	LightAttanuation.push_back(0.001);
	LightColour.push_back(glm::vec4(1, 0, 0, 1));

	LightLocation.push_back(glm::vec3(4, 0, 0));
	LightAttanuation.push_back(0.001);
	LightColour.push_back(glm::vec4(0, 1, 0, 1));

	LightLocation.push_back(glm::vec3(0, 0, 1));
	LightAttanuation.push_back(0.01);
	LightColour.push_back(glm::vec4(0, 0, 1, 1));

	/*
	LightLocation.push_back(glm::vec3(0, 4, 0));
	LightAttanuation.push_back(0.01);
	LightColour.push_back(glm::vec4(0, 0, 1, 1));
	*/
	/*
	LightLocation.push_back(glm::vec3(4, 0, 0));
	LightAttanuation.push_back(0.01);
	LightColour.push_back(glm::vec4(1, 1, 0, 1));

	LightLocation.push_back(glm::vec3(4, 0, 0));
	LightAttanuation.push_back(0.01);
	LightColour.push_back(glm::vec4(1, 1, 0, 1));
	*/

	glEnableVertexAttribArray(0);							// positie
	glEnableVertexAttribArray(1);							// texcoord
	glEnableVertexAttribArray(2);							// normal

	if (glDebugMessageCallback)
	{
		glDebugMessageCallback(&onDebug, NULL);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
		glEnable(GL_DEBUG_OUTPUT);
	}


	rotation = 0;
	lastTime = glutGet(GLUT_ELAPSED_TIME);
}


void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 projection = glm::perspective(glm::radians(70.0f), screenSize.x / (float)screenSize.y, 0.01f, 200.0f);		//begin met een perspective matrix
	glm::vec3 eye = glm::vec3(0, distances[activeModel], distances[activeModel]);
	glm::vec3 centre = glm::vec3(0, 0, 0);
	
	glm::mat4 view = glm::lookAt(eye,centre , glm::vec3(0, 1, 0));//vermenigvuldig met een lookat
	glm::mat4 model = glm::mat4(1);
	// glm::translate(glm::mat4(1), glm::vec3(2, 0, -1)); //of verplaats de camera gewoon naar achter
	model = glm::rotate(model, rotation, glm::vec3(0, 1, 0));//roteer het object een beetje
	glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(view* model)));

	myShader->setActive();

	auto setMatrixUniforms = [&]() {
		glUniformMatrix4fv(myShader->getUniform("modView"), 1, 0, glm::value_ptr( view* model));//en zet de matrix in opengl
		glUniformMatrix4fv(myShader->getUniform("Proj"), 1, 0, glm::value_ptr(projection));//en zet de matrix in opengl
		glUniformMatrix3fv(myShader->getUniform("normal"), 1, 0, glm::value_ptr(normalMatrix));//en zet de matrix in opengl
	};

	setMatrixUniforms();
	myShader->uniformSetter = [&]() {

		// object params
		glUniform1f(myShader->getUniform("fLTScale"), 0.5);// how hard the light scales with elimination drop off from distance. 0.0 to 4 is acceptable
		glUniform1i(myShader->getUniform("iLTPower"), 2);// defuse directionality. 0 lights up hole model 40 only part between light and eye

		// scene params
		glm::vec3 look = centre - eye;
		glUniform3f(myShader->getUniform("vViewDirection"), look.x, look.y, look.z);
		glUniform2f(myShader->getUniform("vWindowWidth"), screenSize.x, screenSize.y);
		eye = glm::vec3(glm::vec4(eye, 1) *model);
		glUniform3f(myShader->getUniform("vEye"), eye.x, eye.y, eye.z);
		
		// light params
		glUniform1f(myShader->getUniform("fAmbientScale"), 0.0);// ambient light intensity 0.0 to 1.0
		glUniform1i(myShader->getUniform("iLightCount"), LightLocation.size());

		const auto lightLocationData = LightLocation.data();
		glUniform3fv(myShader->getUniform("vLightLocation"), LightLocation.size(), glm::value_ptr(lightLocationData[0]));
		const auto LightAttanuationData = LightAttanuation.data();
		glUniform1fv(myShader->getUniform("fLTattenuation"), LightAttanuation.size(), glm::value_ptr(lightLocationData[0]));
		const auto LightColourData = LightColour.data();
		glUniform4fv(myShader->getUniform("vLTColour"), LightColour.size(), glm::value_ptr(LightColourData[0]));

	};
	myShader->setUniforms();
	models[activeModel]->draw();//en tekenen :)

	/*
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 11 * 4, &vertices[0]);									//geef aan dat de posities op deze locatie zitten
	glVertexAttribPointer(1, 3, GL_FLOAT, false, 11 * 4, ((float*)&vertices[0]) + 3);					//geef aan dat de kleuren op deze locatie zitten
	glVertexAttribPointer(2, 2, GL_FLOAT, false, 11 * 4, ((float*)&vertices[0]) + 6);					//geef aan dat de texcoords op deze locatie zitten
	glVertexAttribPointer(3, 3, GL_FLOAT, true, 11 * 4, ((float*)&vertices[0]) + 8);					//geef aan dat de texcoords op deze locatie zitten

	glDrawArrays(GL_TRIANGLES, 0, vertices.size());																//en tekenen :)
	*/

	glutSwapBuffers();
}

void reshape(int newWidth, int newHeight)
{
	screenSize.x = newWidth;
	screenSize.y = newHeight;
	glutPostRedisplay();
}

float lightLocation = 0.0;
void keyboard(unsigned char key, int x, int y)
{
	if (key == VK_ESCAPE)
		glutLeaveMainLoop();
	/*
	if (key == '[')
	{
		currentShader = (currentShader + shaders.size() - 1) % shaders.size();
		std::cout << "Shader " << currentShader << std::endl;
	}
	if (key == ']')
	{
		currentShader = (currentShader + 1) % shaders.size();
		std::cout << "Shader " << currentShader << std::endl;
	}
	*/
	if (key == ',' || key == '.') {
		activeModel = (activeModel + 1) % models.size();
		std::cout << "next!";
	}
	if (key == 'a') {
		lightLocation+=0.09;
	}
	if (key == 'd') {
		lightLocation-=0.09;
	}
}

void update()
{
	int time = glutGet(GLUT_ELAPSED_TIME);
	float elapsed = time - lastTime;
	
	auto workingTime= elapsed / 1000.0f;
	rotation += workingTime;
    LightLocation[0] = glm::vec3(1, 0, 0) * lightLocation;

	glutPostRedisplay();
	lastTime = time;
}

void GLAPIENTRY MessageCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
		type, severity, message);
}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitWindowSize(1900, 1000);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow("Visualisatietechnieken");

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(update);

	init();
	
	glutMainLoop();

}