#include <glad.h>
#include <glfw3.h>
#include <stdio.h>
#include <iomanip>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "lecturaShader.h"

double currentTime;
double lastTime = 0;

unsigned int VAOCubo, VAOPenrose;

unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 800;
unsigned int camara;

float anguloY = -39;
float anguloPlanoY = 32;

glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
glm::vec3 lightPos = glm::vec3(0.0f, 0.0f, 20.0f);
glm::vec3 vectorPerspectivaIsometrica;

extern GLuint setShaders(const char* nVertx, const char* nFrag);

void _printMatrix(glm::mat4 mat) {
	const float* pSource = (const float*)glm::value_ptr(mat);
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			std::cout << std::fixed << std::setw(4) << std::setprecision(2)
				<< std::setfill('0') << pSource[i * 4 + j] << "\t";
		}
		std::cout << "\n";
	}
	std::cout << "\n";
}

void tiempo() {
	currentTime = glfwGetTime();
}

GLuint cargarTextura(const char* nombreArchivo) {
	GLuint textura;
	stbi_set_flip_vertically_on_load(true);
	glGenTextures(1, &textura);
	glBindTexture(GL_TEXTURE_2D, textura);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, nrChannels;
	unsigned char* data = stbi_load(nombreArchivo, &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Error cargando textura" << std::endl;
	}
	stbi_image_free(data);
	return textura;
}

class Objeto {
public:
	glm::vec3 escalaBase = glm::vec3(100.0f, 100.0f, 100.0f);
	glm::vec3 color;
	glm::vec3 distorsion;
	glm::vec3 posicion_local;
	glm::mat4 rotacion_local;
	glm::mat4 matWorld;
	GLenum tipoPrimitivas = GL_TRIANGLES;
	GLenum modoPoligonos = GL_FILL;
	unsigned int VAO;
	unsigned int EBO_number_to_draw;
	GLuint shader;
	GLuint textura1;
	GLuint textura2;

	Objeto() {};

	Objeto(glm::vec3 color, unsigned int VAO, unsigned int EBO_number_to_draw, GLuint shader) {
		this->color = color;
		this->VAO = VAO;
		this->EBO_number_to_draw = EBO_number_to_draw;
		this->shader = shader;
		distorsion = glm::vec3(1.0f, 1.0f, 1.0f);
		posicion_local = glm::vec3(0.0f, 0.0f, 0.0f);
		rotacion_local = glm::mat4();
		matWorld = glm::mat4();
	};

	void setMatWorld(glm::mat4 matWorld) {
		this->matWorld = matWorld;
	}

	glm::mat4 getMatTransformacionLocal() {
		glm::mat4 transform = glm::mat4();
		transform = glm::scale(glm::mat4(), escalaBase) * glm::translate(glm::mat4(), posicion_local) * rotacion_local * glm::scale(glm::mat4(), distorsion);

		return transform;
	}

	glm::mat4 getMatTransformacionWorld() {
		return matWorld;
	}

	void setTipoPrimitivas(GLenum t) {
		tipoPrimitivas = t;
	}

	void setPosicionLocal(glm::vec3 pos) {
		posicion_local = pos;
	}

	void setDistorsion(glm::vec3 d) {
		distorsion = d;
	}

	void setRotacionLocal(glm::mat4 rotacionLocal) {
		this->rotacion_local = rotacionLocal;
	}

	void setTextura(GLuint tex) {
		this->textura1 = tex;
	}

	void dibujar(glm::vec3 eye, glm::vec3 center, glm::mat4 projection) {
		glUseProgram(shader);
		glPolygonMode(GL_FRONT_AND_BACK, modoPoligonos);
		glBindVertexArray(VAO);

		unsigned int modelLoc = glGetUniformLocation(shader, "model");
		unsigned int viewLoc = glGetUniformLocation(shader, "view");
		unsigned int projectionLoc = glGetUniformLocation(shader, "projection");
		unsigned int objectColorLoc = glGetUniformLocation(shader, "objectColor");
		unsigned int lightColorLoc = glGetUniformLocation(shader, "lightColor");
		unsigned int lightPosLoc = glGetUniformLocation(shader, "lightPos");
		unsigned int viewPosLoc = glGetUniformLocation(shader, "viewPos");

		glm::mat4 viewMatrix = glm::lookAt(eye, center, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 matCalculada = getMatTransformacionWorld() * getMatTransformacionLocal();
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(matCalculada));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
		glUniform3fv(objectColorLoc, 1, glm::value_ptr(color));
		glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
		glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));
		glUniform3fv(viewPosLoc, 1, glm::value_ptr(eye));
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, textura1);
		//glActiveTexture(GL_TEXTURE1);
		//glBindTexture(GL_TEXTURE_2D, textura2);

		glDrawElements(tipoPrimitivas, EBO_number_to_draw, GL_UNSIGNED_INT, NULL);
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glUseProgram(0);
	}
};

Objeto penrose, cubo;

void crearPenrose() {
	unsigned int VBO, EBO;

	float vertices[] = {
40.000000f, 40.000000f, 0.000000f,
40.000000f, 40.000000f, 27.386452f,
50.000000f, 40.000000f, 27.386452f,
50.000000f, 50.000000f, 39.823074f,
50.000000f, 40.000000f, -0.000000f,
50.000000f, 50.000000f, -10.000000f,
50.000000f, 40.000000f, -10.000000f,
50.000000f, 0.000000f, 0.000000f,
50.000000f, 0.000000f, -10.000000f,
40.000000f, 30.000000f, -0.000000f,
10.000000f, -0.000000f, -60.000000f,
0.000000f, -0.000000f, -60.000000f,
10.000000f, 10.000000f, -60.000000f,
0.000000f, 10.000000f, -60.000000f,
10.000000f, 0.000000f, -10.000000f,
0.000000f, 0.000000f, 0.000000f,
10.000000f, -0.000000f, -50.000000f,
40.000000f, 10.000000f, -10.000000f,
40.000000f, 30.000000f, -10.000000f,
10.000000f, 10.000000f, -10.000000f,
40.000000f, 40.000000f, -10.000000f,
40.000000f, 10.000000f, -0.000000f,
0.000000f, 10.000000f, -0.000000f,
10.000000f, 10.000000f, -50.000000f
	};

	float normals[] = {
0.0000, -1.0000, 0.0000,
0.0000, -0.7793, 0.6266,
1.0000, 0.0000, 0.0000,
0.0000, 0.0000, -1.0000,
0.0000, -0.0000, 1.0000,
0.0000, 0.0000, 0.0000,
-1.0000, 0.0000, 0.0000,
0.0000, 1.0000, 0.0000,
-0.7071, 0.7071, 0.0000,
-0.8000, 0.0000, 0.6000,
0.7071, -0.7071, 0.0000,
0.7809, 0.0000, -0.6247
	};

	float uv[] = {
0.484344, 0.446315,
0.311893, 0.510358,
0.681988, 0.743145,
0.313673, 0.510378,
0.854439, 0.679102,
0.986972, 0.933878,
0.989577, 0.764102,
0.864862, 0.000000,
1.000000, 0.085001,
0.985885, 0.766181,
0.813434, 0.830225,
0.983279, 0.935956,
0.810828, 1.000000,
0.310195, 0.341177,
0.002606, 0.320220,
0.850747, 0.681180,
0.824943, 0.318820,
0.819732, 0.658371,
0.307589, 0.510952,
0.684594, 0.573370,
0.817126, 0.828147,
0.689805, 0.233819,
0.000000, 0.489995,
0.848141, 0.850955
	};

	unsigned int faces_info[] = {
		// (id_v1, vt_v1, vn_v1), (id_v2, vt_v2, vn_v2)...
		3, 1, 1, 2, 2, 1, 1, 3, 1,
2, 2, 2, 3, 1, 2, 4, 4, 2,
3, 1, 3, 5, 5, 3, 4, 4, 3,
4, 4, 3, 5, 5, 3, 6, 6, 3,
6, 6, 3, 5, 5, 3, 7, 7, 3,
7, 7, 3, 5, 5, 3, 8, 8, 3,
7, 7, 3, 8, 8, 3, 9, 9, 3,
5, 5, 1, 3, 1, 1, 1, 3, 1,
11, 10, 4, 12, 11, 4, 13, 12, 4,
13, 12, 4, 12, 11, 4, 14, 13, 4,
9, 9, 1, 8, 8, 1, 15, 14, 1,
15, 14, 1, 8, 8, 1, 16, 15, 1,
15, 14, 1, 16, 15, 1, 12, 11, 1,
11, 10, 1, 17, 16, 1, 12, 11, 1,
12, 11, 1, 17, 16, 1, 15, 14, 1,
15, 14, 4, 18, 17, 4, 9, 9, 4,
9, 9, 4, 18, 17, 4, 19, 18, 4,
9, 9, 4, 19, 18, 4, 7, 7, 4,
15, 14, 4, 20, 19, 4, 18, 17, 4,
10, 20, 5, 5, 5, 5, 1, 3, 5,
2, 2, 6, 21, 21, 6, 1, 3, 6,
1, 3, 7, 21, 21, 7, 10, 20, 7,
10, 20, 7, 21, 21, 7, 19, 18, 7,
10, 20, 7, 19, 18, 7, 18, 17, 7,
18, 17, 7, 22, 22, 7, 10, 20, 7,
10, 20, 5, 22, 22, 5, 5, 5, 5,
5, 5, 5, 22, 22, 5, 8, 8, 5,
8, 8, 5, 22, 22, 5, 16, 15, 5,
16, 15, 5, 22, 22, 5, 23, 23, 5,
14, 13, 7, 12, 11, 7, 23, 23, 7,
23, 23, 7, 12, 11, 7, 16, 15, 7,
18, 17, 8, 20, 19, 8, 22, 22, 8,
22, 22, 8, 20, 19, 8, 23, 23, 8,
23, 23, 8, 20, 19, 8, 14, 13, 8,
14, 13, 8, 20, 19, 8, 24, 24, 8,
14, 13, 8, 24, 24, 8, 13, 12, 8,
4, 4, 9, 6, 6, 9, 2, 2, 9,
2, 2, 9, 6, 6, 9, 21, 21, 9,
21, 21, 9, 6, 6, 9, 13, 12, 9,
21, 21, 9, 13, 12, 9, 24, 24, 9,
17, 16, 10, 19, 18, 10, 24, 24, 10,
24, 24, 10, 19, 18, 10, 21, 21, 10,
11, 10, 11, 7, 7, 11, 17, 16, 11,
17, 16, 11, 7, 7, 11, 19, 18, 11,
11, 10, 12, 13, 12, 12, 7, 7, 12,
7, 7, 12, 13, 12, 12, 6, 6, 12,
15, 14, 3, 17, 16, 3, 20, 19, 3,
20, 19, 3, 17, 16, 3, 24, 24, 3
	};

	glGenVertexArrays(1, &VAOPenrose);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glBindVertexArray(VAOPenrose);

	unsigned int vertices_number = ((sizeof(faces_info) / sizeof(*faces_info)) / 3);

	unsigned int *indices = (unsigned int*) malloc(sizeof(unsigned int) * vertices_number);
	float *processed_normals = (float*) malloc(sizeof(float) * vertices_number * 3);
	float *processed_uv = (float*) malloc(sizeof(float) * vertices_number * 2);
	float* processed_vertices = (float*)malloc(sizeof(float) * vertices_number * 3);

	int i, j;
	for (j = 0, i = 0 ; j < vertices_number; i+=3, j++) {
		indices[j] = j;
		// Copiamos las coordenadas UV
		processed_uv[j * 2 + 0] = uv[(faces_info[i + 1] - 1) * 2 + 0];
		processed_uv[j * 2 + 1] = uv[(faces_info[i + 1] - 1) * 2 + 1];
		// Copiamos las normales
		processed_normals[j * 3 + 0] = normals[(faces_info[i + 2] - 1) * 3 + 0];
		processed_normals[j * 3 + 1] = normals[(faces_info[i + 2] - 1) * 3 + 1];
		processed_normals[j * 3 + 2] = normals[(faces_info[i + 2] - 1) * 3 + 2];
		// Copiamos el vertice
		processed_vertices[j * 3 + 0] = vertices[(faces_info[i] - 1) * 3 + 0];
		processed_vertices[j * 3 + 1] = vertices[(faces_info[i] - 1) * 3 + 1];
		processed_vertices[j * 3 + 2] = vertices[(faces_info[i] - 1) * 3 + 2];
	};

	for (j = 0, i = 0; i < (sizeof(faces_info) / sizeof(*faces_info)); i += 3, j++) {
		std::cout << "Vertice " << j << ": " << indices[j] << std::endl;
		std::cout << "uv[0] " << j * 2 + 0 << ": " << processed_uv[j * 2 + 0] << std::endl;
		std::cout << "uv[1] " << j * 2 + 1 << ": " << processed_uv[j * 2 + 1] << std::endl;
		std::cout << "n[0] " << j * 3 + 0 << ": " << processed_normals[j * 3 + 0] << std::endl;
		std::cout << "n[1] " << j * 3 + 1 << ": " << processed_normals[j * 3 + 1] << std::endl;
		std::cout << "n[2] " << j * 3 + 2 << ": " << processed_normals[j * 3 + 2] << std::endl;
		std::cout << "x " << j * 3 + 0 << ": " << processed_vertices[j * 3 + 0] << std::endl;
		std::cout << "y " << j * 3 + 1 << ": " << processed_vertices[j * 3 + 1] << std::endl;
		std::cout << "z " << j * 3 + 2 << ": " << processed_vertices[j * 3 + 2] << std::endl << std::endl;
	};

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices_number * sizeof(float) * 3, processed_vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(0 * sizeof(float)));
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertices_number * sizeof(unsigned int), indices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices_number * sizeof(float) * 3, processed_normals, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(0 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices_number * sizeof(float) * 2, processed_uv, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(0 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	free(indices);
	free(processed_normals);
	free(processed_uv);
}

void crearCubo() {
	unsigned int VBO, EBO;

	float vertices[] = {
	-0.5f, -0.5f, +0.5f, +0.0f, +0.0f, +1.0f, +0.0f, +0.0f, // 0  vertice inferior izquierdo de la cara frontal, normal hacia +z
	-0.5f, -0.5f, -0.5f, +0.0f, +0.0f, -1.0f, +1.0f, +0.0f, // 1  vertice inferior derecho de la cara trasera, normal hacia -z
	-0.5f, +0.5f, +0.5f, +0.0f, +0.0f, +1.0f, +0.0f, +1.0f, // 2  vertice superior izquierdo de la cara frontal, normal hacia +z
	-0.5f, +0.5f, -0.5f, +0.0f, +0.0f, -1.0f, +1.0f, +1.0f, // 3  vertice superior derecho de la cara trasera, normal hacia -z
	+0.5f, -0.5f, +0.5f, +0.0f, +0.0f, +1.0f, +1.0f, +0.0f, // 4  vertice inferior derecho de la cara frontal, normal hacia +z
	+0.5f, -0.5f, -0.5f, +0.0f, +0.0f, -1.0f, +0.0f, +0.0f, // 5  vertice inferior izquierdo de la cara trasera, normal hacia -z
	+0.5f, +0.5f, +0.5f, +0.0f, +0.0f, +1.0f, +1.0f, +1.0f, // 6  vertice superior derecho de la cara frontal, normal hacia +z
	+0.5f, +0.5f, -0.5f, +0.0f, +0.0f, -1.0f, +0.0f, +1.0f, // 7  vertice superior izquierdo de la cara trasera, normal hacia -z

	-0.5f, -0.5f, +0.5f, -1.0f, +0.0f, +0.0f, +0.0f, +0.0f, // 8  vertice inferior derecho de la cara izquierda, normal hacia -x
	-0.5f, -0.5f, -0.5f, -1.0f, +0.0f, +0.0f, +1.0f, +0.0f, // 9  vertice inferior izquierdo de la cara izquierda, normal hacia -x
	-0.5f, +0.5f, +0.5f, -1.0f, +0.0f, +0.0f, +0.0f, +1.0f, // 10 vertice superior derecho de la cara izquierda, normal hacia -x
	-0.5f, +0.5f, -0.5f, -1.0f, +0.0f, +0.0f, +1.0f, +1.0f, // 11 vertice superior izquierdo de la cara izquierda, normal hacia -x
	+0.5f, -0.5f, +0.5f, +1.0f, +0.0f, +0.0f, +1.0f, +0.0f, // 12 vertice inferior izquierdo de la cara derecha, normal hacia +x
	+0.5f, -0.5f, -0.5f, +1.0f, +0.0f, +0.0f, +0.0f, +0.0f, // 13 vertice inferior derecho de la cara derecha, normal hacia +x
	+0.5f, +0.5f, +0.5f, +1.0f, +0.0f, +0.0f, +1.0f, +1.0f, // 14 vertice superior izquierdo de la cara derecha, normal hacia +x
	+0.5f, +0.5f, -0.5f, +1.0f, +0.0f, +0.0f, +0.0f, +1.0f, // 15 vertice superior derecho de la cara derecha, normal hacia +x

	-0.5f, -0.5f, +0.5f, +0.0f, -1.0f, +0.0f, +0.0f, +0.0f, // 16 vertice superior izquierdo de la cara inferior, normal hacia -y
	-0.5f, -0.5f, -0.5f, +0.0f, -1.0f, +0.0f, +1.0f, +0.0f, // 17 vertice inferior izquierdo de la cara inferior, normal hacia -y
	-0.5f, +0.5f, +0.5f, +0.0f, +1.0f, +0.0f, +0.0f, +1.0f, // 18 vertice inferior izquierdo de la cara superior, normal hacia +y
	-0.5f, +0.5f, -0.5f, +0.0f, +1.0f, +0.0f, +1.0f, +1.0f, // 19 vertice superior izquierdo de la cara superior, normal hacia +y
	+0.5f, -0.5f, +0.5f, +0.0f, -1.0f, +0.0f, +1.0f, +0.0f, // 20 vertice superior derecho de la cara inferior, normal hacia -y
	+0.5f, -0.5f, -0.5f, +0.0f, -1.0f, +0.0f, +0.0f, +0.0f, // 21 vertice inferior derecho de la cara inferior, normal hacia -y
	+0.5f, +0.5f, +0.5f, +0.0f, +1.0f, +0.0f, +1.0f, +1.0f, // 22 vertice inferior derecho de la cara superior, normal hacia +y
	+0.5f, +0.5f, -0.5f, +0.0f, +1.0f, +0.0f, +0.0f, +1.0f, // 23 vertice superior derecho de la cara superior, normal hacia +y
	};

	unsigned int indices[] = {
		1,7,5, // cara trasera
		1,3,7, // cara trasera
		0,6,2, // cara delantera
		0,4,6, // cara delantera
		13,14,12, // cara derecha
		13,15,14, // cara derecha
		9,10,11, // cara izquierda
		9,8,10, // cara izquierda
		19,22,23, // cara superior
		19,18,22, // cara superior
		16,21,20, // cara inferior
		16,17,21  // cara inferior
	};

	glGenVertexArrays(1, &VAOCubo);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glBindVertexArray(VAOCubo);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

void openGlInit() {
	glClearDepth(1.0f); //Valor z-buffer
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); //Borro el Buffer the la ventana
	glEnable(GL_DEPTH_TEST); // z-buffer
	//glEnable(GL_CULL_FACE); //ocultacion caras back
	glCullFace(GL_BACK);
	glEnable(GL_TEXTURE_2D);
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	glm::mat4 rotacionY = glm::rotate(glm::mat4(1), glm::radians(anguloPlanoY), glm::vec3(0, 1, 0));
	glm::mat4 rotacionX = glm::rotate(glm::mat4(1), glm::radians(anguloY), glm::vec3(1, 0, 0));
	vectorPerspectivaIsometrica = rotacionX * rotacionY * glm::vec4(0, 0, 1, 1);
	//std::cout << vectorPerspectivaIsometrica.r << vectorPerspectivaIsometrica.g << vectorPerspectivaIsometrica.b << std::endl;
	//std::cout << anguloY << anguloPlanoY << std::endl << std::endl;

	glm::mat4 projection = glm::ortho(-((float)SCR_WIDTH / 2.0f), ((float)SCR_WIDTH / 2.0f), -((float)SCR_HEIGHT / 2.0f), ((float)SCR_HEIGHT / 2.0f), -100.0f, 400.0f);

	penrose.dibujar(vectorPerspectivaIsometrica*200.0f + glm::vec3(50,0,0), glm::vec3(50,0,0), projection);
	cubo.dibujar(vectorPerspectivaIsometrica * 200.0f + glm::vec3(50, 0, 0), glm::vec3(50, 0, 0), projection);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void window_size_callback(GLFWwindow* window, int width, int height);

int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Creo la ventana
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "AldanCreo", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, key_callback);
	glfwSetWindowSizeCallback(window, window_size_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	openGlInit();

	crearCubo();
	crearPenrose();

	GLuint shaderSinTexturas = setShaders("shader.vert", "shader.frag");
	GLuint shaderConTexturas = setShaders("shaderTextura.vert", "shaderTextura.frag");

	GLuint cemento = cargarTextura("cemento.jpg");

	glUniform1i(glGetUniformLocation(shaderConTexturas, "tex1"), 0);
	glUniform1i(glGetUniformLocation(shaderConTexturas, "tex2"), 1);

	penrose = Objeto(glm::vec3(1.0f, 0.0f, 1.0f), VAOPenrose, 144, shaderSinTexturas);
	penrose.setDistorsion(glm::vec3(1, 1, 1));
	//penrose.modoPoligonos = GL_LINE;

	cubo = Objeto(glm::vec3(1.0f, 0.0f, 1.0f), VAOCubo, 144, shaderSinTexturas);
	//penrose.modoPoligonos = GL_LINE;
	

	while (!glfwWindowShouldClose(window))
	{
		display();
		tiempo();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

void window_size_callback(GLFWwindow* window, int width, int height)
{
	SCR_WIDTH = width;
	SCR_HEIGHT = height;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		anguloY++;
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		anguloY--;
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		anguloPlanoY++;
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		anguloPlanoY--;
}