#include <Windows.h>
#include <iostream>
#include <GL/glew.h>
#include <GL/GL.h>
#include <GL/freeglut.h>

#define GLFW_INCLUDE_CLU
#define GLFW_DLL
#include <GLFW/glfw3.h>

#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace glm;
using namespace std;

// -------------------------------------------------
// Global Variables
// -------------------------------------------------
#define M_PI 3.1415926535f
#define INF 1'000'000'000
int Width = 512;
int Height = 512;
// -------------------------------------------------

void create_scene(vector<vec3>& circlePosition, vector<unsigned int>& index, int* vertexNum, int* triangleNum)
{
	int width = 32;
	int height = 16;
	int gNumVertices = (height - 2) * width + 2;
	int gNumTriangles = (height - 2) * (width - 1) * 2;
	*vertexNum = gNumVertices;
	*triangleNum = gNumTriangles;

	float theta, phi;

	for (int j = 1; j < height - 1; ++j)
	{
		for (int i = 0; i < width; ++i)
		{
			theta = (float)j / (height - 1) * M_PI;
			phi = (float)i / (width - 1) * M_PI * 2;

			float   x = sinf(theta) * cosf(phi);
			float   y = cosf(theta);
			float   z = -sinf(theta) * sinf(phi);

			circlePosition.push_back({ x,y,z });
		}
	}
	circlePosition.push_back({ 0,1,0 });

	circlePosition.push_back({ 0,-1,0 });
	for (int j = 0; j < height - 3; ++j)
	{
		for (int i = 0; i < width - 1; ++i)
		{
			index.push_back(j * width + i);
			index.push_back((j + 1) * width + (i + 1));
			index.push_back(j * width + (i + 1));
			index.push_back(j * width + i);
			index.push_back((j + 1) * width + i);
			index.push_back((j + 1) * width + (i + 1));
		}
	}

	for (int i = 0; i < width - 1; ++i)
	{
		index.push_back((height - 2) * width);
		index.push_back(i);
		index.push_back(i + 1);
		index.push_back((height - 2) * width + 1);
		index.push_back((height - 3) * width + (i + 1));
		index.push_back((height - 3) * width + i);
	}
}

GLuint LoadShaders(const string& vertex_file_path, const string& fragment_file_path) {
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	string VertexShaderCode;
	ifstream VertexShaderStream(vertex_file_path.c_str(), ios::in);
	if (VertexShaderStream.is_open()) {
		stringstream sstr;
		sstr << VertexShaderStream.rdbuf();
		VertexShaderCode = sstr.str();
		VertexShaderStream.close();
	}
	else {
		cout << "VertexShader file open errer";
		return 0;
	}

	string FragmentShaderCode;
	ifstream FragmentShaderStream(fragment_file_path.c_str(), ios::in);
	if (FragmentShaderStream.is_open()) {
		stringstream sstr;
		sstr << FragmentShaderStream.rdbuf();
		FragmentShaderCode = sstr.str();
		FragmentShaderStream.close();
	}
	else {
		cout << "FragmentShader file open errer";
		return 0;
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	char const* VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// Compile Fragment Shader
	char const* FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	// Link the program
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);


	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

void Update_mesh(const GLuint& VAO, const vector<GLuint>& GLBuffers,
	vector<vec3>& Positions,
	vector<vec3> Normals,
	vector<vec3> Colors,
	vector<unsigned int>& Indices) {
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, GLBuffers[0]);
	glBufferData(GL_ARRAY_BUFFER, Positions.size() * sizeof(Positions[0]), &Positions[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

	glBindBuffer(GL_ARRAY_BUFFER, GLBuffers[1]);
	glBufferData(GL_ARRAY_BUFFER, Normals.size() * sizeof(Normals[0]), &Normals[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, GLBuffers[2]);
	glBufferData(GL_ARRAY_BUFFER, Colors.size() * sizeof(Colors[0]), &Colors[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GLBuffers[3]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, Indices.size() * sizeof(Indices[0]), &Indices[0], GL_STATIC_DRAW);

	// Unbind
	glBindVertexArray(0);
}

void resize_callback(GLFWwindow* , int w, int h) {
	Width = w;
	Height = h;

	glViewport(0, 0, w, h);
}

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS ||
		glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}

int main(int argc, char** argv) {
	GLFWwindow* window;

	if (!glfwInit()) return -1;

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(Width, Height, "Phong Shader", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, resize_callback);

	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		return -1;
	}

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);


	GLuint shaderProgram = LoadShaders("Phong.vert", "Phong.frag");

	// Circle
	vector<vec3> Positions;
	vector<unsigned int> Indices;

	int vertexNum = 0, triangleNum = 0;
		
	create_scene(Positions, Indices, &vertexNum, &triangleNum);

	//Compute Normal
	vector<vec3> Normals = vector<vec3>(vertexNum, { 0,0,0 });
	vector<int> triangleSum = vector<int>(vertexNum, 0);

	for (int i = 0; i < triangleNum; i++) {
		vec3 t1 = Positions[Indices[3 * i + 0]];
		vec3 t2 = Positions[Indices[3 * i + 1]];
		vec3 t3 = Positions[Indices[3 * i + 2]];
		vec3 mid = ((t1 + t2 + t3) / 3.0f);
		vec3 v1 = vec3(t2 - t1);
		vec3 v2 = vec3(t3 - t1);
		vec3 normal = normalize(cross(v1, v2));

		triangleSum[Indices[3 * i + 0]]++;
		triangleSum[Indices[3 * i + 1]]++;
		triangleSum[Indices[3 * i + 2]]++;
		Normals[Indices[3 * i + 0]] += normal;
		Normals[Indices[3 * i + 1]] += normal;
		Normals[Indices[3 * i + 2]] += normal;
	}

	for (int i = 0; i < vertexNum; i++) {
		vec3 t = (vec3)Positions[i];
		if (triangleSum[i] > 0) {
			Normals[i] = normalize(Normals[i] / (float)(triangleSum[i]));
		}
	}
	vector<vec3> Colors(vertexNum, vec3(1.0f, 1.0f, 1.0f));

	mat4 model = mat4(1.0f);

	GLuint VAO;
	glGenVertexArrays(1, &VAO);

	const int numBuffers = 4;
	vector<GLuint> GLBuffers;
	GLBuffers.resize(numBuffers, 0);
	glGenBuffers(numBuffers, &GLBuffers[0]);
	Update_mesh(VAO, GLBuffers, Positions, Normals, Colors, Indices);

	while (!glfwWindowShouldClose(window)) {
		processInput(window);

		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(shaderProgram);

		// Transform Matrix
		// 
		// Model
		mat4 modelScale(
			2, 0, 0, 0,
			0, 2, 0, 0,
			0, 0, 2, 0,
			0, 0, 0, 1
		);
		mat4 modeltranslate(
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, -7, 1
		);
		mat4 modeling = modeltranslate * modelScale;

		vec3 e(0, 0, 0);
		vec3 u(1, 0, 0);
		vec3 v(0, 1, 0);
		vec3 w(0, 0, 1);

		// Camera
		mat4 camera(
			u.x, u.y, u.z, 0,
			v.x, v.y, v.z, 0,
			w.x, w.y, w.z, 0,
			e.x, e.y, e.z, 1
		);
		camera = inverse(camera);

		// Projection
		float l = -0.1;
		float r = 0.1;
		float b = -0.1;
		float t = 0.1;
		float n = 0.1;
		float f = 1000;

		mat4 projection(
			(2.0f * n) / (r - l), 0, 0, 0,
			0, (2.0f * n) / (t - b), 0, 0,
			(r + l) / (r - l), (t + b) / (t - b), -(f + n) / (f - n), -1,
			0, 0, -(2.0f * f * n) / (f - n), 0
		);

		GLuint modelingLoc = glGetUniformLocation(shaderProgram, "modeling");
		GLuint cameraLoc = glGetUniformLocation(shaderProgram, "camera");
		GLuint projectionLoc = glGetUniformLocation(shaderProgram, "projection");

		glUniformMatrix4fv(modelingLoc, 1, GL_FALSE, &modeling[0][0]);
		glUniformMatrix4fv(cameraLoc, 1, GL_FALSE, &camera[0][0]);
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, &projection[0][0]);

		GLuint cameraPosLoc = glGetUniformLocation(shaderProgram, "cameraPos");
		GLuint lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
		GLuint laLoc = glGetUniformLocation(shaderProgram, "la");
		GLuint pLoc = glGetUniformLocation(shaderProgram, "p");
		GLuint intensityLoc = glGetUniformLocation(shaderProgram, "intensity");

		vec3 cameraPos(0, 0, 0);
		glUniform3fv(cameraPosLoc, 1, &cameraPos[0]);
		vec3 lightPos(-4, 4, -3);
		glUniform3fv(lightPosLoc, 1, &lightPos[0]);
		float la = 0.2f;
		glUniform1f(laLoc, la);
		float p = 32.0f;
		glUniform1f(pLoc, p);
		float intensity = 1.0f;
		glUniform1f(intensityLoc, intensity);


		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, Indices.size(), GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &VAO);
	if (GLBuffers[0] != 0) {
		glDeleteBuffers(numBuffers, &GLBuffers[0]);
	}
	glDeleteProgram(shaderProgram);

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
