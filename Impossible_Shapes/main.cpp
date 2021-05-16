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
#include "esfera.h"
#include "penrose.h"

double currentTime;
double lastTime = 0;

unsigned int VAOPenroseFrontal, VAOPenroseTrasero, VAOPenroseLateral, VAOEsfera;

unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 800;
unsigned int camara;

float anguloY = -38.72;
float anguloPlanoY = 32;

float posEsferaX = 0;
float posEsferaY = 0;
float posEsferaZ = 0;

int pintarAlgoritmoPintor = 0;

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

class Interpolador {
	glm::vec3 posicionInicio;
	glm::vec3 posicionDestino;
	double tiempoFin;
	double tiempoInicio;
public:
	void comenzar(double tiempoActual, double tiempoInterpolacion, glm::vec3 posicionInicio, glm::vec3 posicionDestino) {
		this->posicionInicio = posicionInicio;
		this->posicionDestino = posicionDestino;
		tiempoInicio = tiempoActual;
		tiempoFin = tiempoInicio + tiempoInterpolacion;
	}

	glm::vec3 getSiguientePosicion(double tiempoActual) {
		if (tiempoActual >= tiempoFin) {
			return posicionDestino;
		}

		double tiempoInterpolacion = tiempoFin - tiempoInicio;
		double tiempoTranscurrido = tiempoActual - tiempoInicio;
		double porcentajeInterpolacion = tiempoTranscurrido / tiempoInterpolacion;

		glm::vec3 deltaPosicion = posicionDestino - posicionInicio;
		float posicionX = deltaPosicion.x * porcentajeInterpolacion;
		float posicionY = deltaPosicion.y * porcentajeInterpolacion;
		float posicionZ = deltaPosicion.z * porcentajeInterpolacion;

		return posicionInicio + glm::vec3(posicionX, posicionY, posicionZ);
	}

	int finalizado(double tiempoActual) {
		if (tiempoActual >= tiempoFin) {
			return 1;
		}
		return 0;
	}
};

class ControladorEsfera {
	Interpolador interpoladorEsfera;
	int estado = 0;
	int forward = 1;
	double tiempoActual;
public:
	void comenzar(double tiempoActual) {
		estado = 1;
		forward = 1;
		this->tiempoActual = tiempoActual;
		procesarCambioEstado();
	}

	glm::vec3 getPosicion() {
		return interpoladorEsfera.getSiguientePosicion(tiempoActual);
	}

	void actualizar(double tiempoActual) {
		this->tiempoActual = tiempoActual;
		if (interpoladorEsfera.finalizado(tiempoActual)) {
			cambiarEstado();
		}
	}

private:
	void cambiarEstado() {
		if (forward) {
			estado++;
		}
		else {
			estado--;
		}
		procesarCambioEstado();
	}

	void procesarCambioEstado() {
		glm::vec3 destino = getPosicionEstado(estado);
		glm::vec3 inicio = getPosicionEstado(forward ? estado - 1 : estado + 1);
		double tiempo;
		switch (estado)
		{
		case 0:
			tiempo = 3;
			forward = 1;
			break;
		case 1:
			tiempo = 3;
			break;
		case 2:
			tiempo = 3;
			break;
		case 3:
			tiempo = 3;
			break;
		case 4:
			tiempo = 3;
			forward = 0;
			break;
		default:
			estado = 0;
			return procesarCambioEstado();
		}
		interpoladorEsfera.comenzar(tiempoActual, tiempo, inicio, destino);
	}

	glm::vec3 getPosicionEstado(int estado) {
		switch (estado)
		{
		case 0:
			return glm::vec3(35, 70, -270);
		case 1:
			return glm::vec3(35, 70, -20);
		case 2:
			return glm::vec3(190, 70, -20);
		case 3:
			return glm::vec3(190, 160, -20);
		case 4:
			return glm::vec3(190, 160, -20);
		default:
			return glm::vec3(0, 0, 0);
		}
	}
};

ControladorEsfera myControlador;

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
	float ambientI = 0.4;
	float diffuseI = 0.6;
	float specularI = 0.8;

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
		unsigned int ambientILoc = glGetUniformLocation(shader, "ambientI");
		unsigned int diffuseILoc = glGetUniformLocation(shader, "diffuseI");
		unsigned int specularILoc = glGetUniformLocation(shader, "specularI");

		glm::mat4 viewMatrix = glm::lookAt(eye, center, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 matCalculada = getMatTransformacionWorld() * getMatTransformacionLocal();
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(matCalculada));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
		glUniform3fv(objectColorLoc, 1, glm::value_ptr(color));
		glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
		glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));
		glUniform3fv(viewPosLoc, 1, glm::value_ptr(eye));
		glUniform1f(ambientILoc, ambientI);
		glUniform1f(diffuseILoc, ambientI);
		glUniform1f(specularILoc, ambientI);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textura1);

		glDrawElements(tipoPrimitivas, EBO_number_to_draw, GL_UNSIGNED_INT, NULL);
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glUseProgram(0);
	}
};

Objeto penroseFrontal, penroseTrasero, penroseLateral, esfera;

void crearPenroseTrasero() {
	unsigned int VBO, EBO;

	unsigned int faces_info[] = {
		// (id_v1, vt_v1, vn_v1), (id_v2, vt_v2, vn_v2)...
		12, 10, 3, 13, 11, 3, 14, 12, 3,
14, 12, 3, 13, 11, 3, 15, 13, 3,
15, 13, 3, 13, 11, 3, 11, 14, 3,
11, 14, 3, 13, 11, 3, 16, 15, 3,
11, 14, 3, 16, 15, 3, 10, 16, 3
	};

	glGenVertexArrays(1, &VAOPenroseTrasero);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glBindVertexArray(VAOPenroseTrasero);

	unsigned int vertices_number = ((sizeof(faces_info) / sizeof(*faces_info)) / 3);

	unsigned int *indices = (unsigned int*) malloc(sizeof(unsigned int) * vertices_number);
	float* processed_buffer = (float*)malloc(sizeof(float) * vertices_number * (3+3+2));

	unsigned int i, j;
	for (i = 0; i < vertices_number; i++) {
		int ubicacion_vector_faces = i * 3;
		int ubicacion_processed_buffer = i * (3 + 3 + 2);
		indices[i] = i;
		// Copiamos el vertice
		processed_buffer[ubicacion_processed_buffer + 0] = vertices_penrose[(faces_info[ubicacion_vector_faces] - 1) * 3 + 0];
		processed_buffer[ubicacion_processed_buffer + 1] = vertices_penrose[(faces_info[ubicacion_vector_faces] - 1) * 3 + 1];
		processed_buffer[ubicacion_processed_buffer + 2] = vertices_penrose[(faces_info[ubicacion_vector_faces] - 1) * 3 + 2];
		// Copiamos las normales
		processed_buffer[ubicacion_processed_buffer + 3] = normales_penrose[(faces_info[ubicacion_vector_faces + 2] - 1) * 3 + 0];
		processed_buffer[ubicacion_processed_buffer + 4] = normales_penrose[(faces_info[ubicacion_vector_faces + 2] - 1) * 3 + 1];
		processed_buffer[ubicacion_processed_buffer + 5] = normales_penrose[(faces_info[ubicacion_vector_faces + 2] - 1) * 3 + 2];
		// Copiamos las coordenadas UV
		processed_buffer[ubicacion_processed_buffer + 6] = uv_penrose[(faces_info[ubicacion_vector_faces + 1] - 1) * 2 + 0];
		processed_buffer[ubicacion_processed_buffer + 7] = uv_penrose[(faces_info[ubicacion_vector_faces + 1] - 1) * 2 + 1];
	};

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* vertices_number * (3 + 3 + 2), processed_buffer, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (3 + 3 + 2) * sizeof(float), (void*)(0 * sizeof(float)));
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (3 + 3 + 2) * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, (3 + 3 + 2) * sizeof(float), (void*)((3+3) * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertices_number * sizeof(unsigned int), indices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	free(indices);
	free(processed_buffer);
}

void crearPenroseLateral() {
	unsigned int VBO, EBO;

	unsigned int faces_info[] = {
		// (id_v1, vt_v1, vn_v1), (id_v2, vt_v2, vn_v2)...
5, 1, 1, 3, 2, 1, 1, 3, 1,
2, 4, 2, 7, 5, 2, 1, 3, 2,
5, 1, 1, 8, 6, 1, 3, 2, 1,
3, 2, 1, 8, 6, 1, 4, 7, 1,
4, 7, 1, 8, 6, 1, 6, 8, 1,
6, 8, 1, 8, 6, 1, 9, 9, 1
	};

	glGenVertexArrays(1, &VAOPenroseLateral);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glBindVertexArray(VAOPenroseLateral);

	unsigned int vertices_number = ((sizeof(faces_info) / sizeof(*faces_info)) / 3);

	unsigned int* indices = (unsigned int*)malloc(sizeof(unsigned int) * vertices_number);
	float* processed_buffer = (float*)malloc(sizeof(float) * vertices_number * (3 + 3 + 2));

	unsigned int i, j;
	for (i = 0; i < vertices_number; i++) {
		int ubicacion_vector_faces = i * 3;
		int ubicacion_processed_buffer = i * (3 + 3 + 2);
		indices[i] = i;
		// Copiamos el vertice
		processed_buffer[ubicacion_processed_buffer + 0] = vertices_penrose[(faces_info[ubicacion_vector_faces] - 1) * 3 + 0];
		processed_buffer[ubicacion_processed_buffer + 1] = vertices_penrose[(faces_info[ubicacion_vector_faces] - 1) * 3 + 1];
		processed_buffer[ubicacion_processed_buffer + 2] = vertices_penrose[(faces_info[ubicacion_vector_faces] - 1) * 3 + 2];
		// Copiamos las normales
		processed_buffer[ubicacion_processed_buffer + 3] = normales_penrose[(faces_info[ubicacion_vector_faces + 2] - 1) * 3 + 0];
		processed_buffer[ubicacion_processed_buffer + 4] = normales_penrose[(faces_info[ubicacion_vector_faces + 2] - 1) * 3 + 1];
		processed_buffer[ubicacion_processed_buffer + 5] = normales_penrose[(faces_info[ubicacion_vector_faces + 2] - 1) * 3 + 2];
		// Copiamos las coordenadas UV
		processed_buffer[ubicacion_processed_buffer + 6] = uv_penrose[(faces_info[ubicacion_vector_faces + 1] - 1) * 2 + 0];
		processed_buffer[ubicacion_processed_buffer + 7] = uv_penrose[(faces_info[ubicacion_vector_faces + 1] - 1) * 2 + 1];
	};

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices_number * (3 + 3 + 2), processed_buffer, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (3 + 3 + 2) * sizeof(float), (void*)(0 * sizeof(float)));
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (3 + 3 + 2) * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, (3 + 3 + 2) * sizeof(float), (void*)((3 + 3) * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertices_number * sizeof(unsigned int), indices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	free(indices);
	free(processed_buffer);
}

void crearPenroseFrontal() {
	unsigned int VBO, EBO;

	unsigned int faces_info[] = {
		// (id_v1, vt_v1, vn_v1), (id_v2, vt_v2, vn_v2)...
		18, 17, 4, 17, 18, 4, 19, 19, 4,
19, 19, 4, 17, 18, 4, 20, 20, 4,
19, 19, 4, 20, 20, 4, 21, 21, 4,
22, 22, 4, 24, 23, 4, 23, 24, 4,
23, 24, 4, 24, 23, 4, 25, 25, 4
	};

	glGenVertexArrays(1, &VAOPenroseFrontal);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glBindVertexArray(VAOPenroseFrontal);

	unsigned int vertices_number = ((sizeof(faces_info) / sizeof(*faces_info)) / 3);

	unsigned int* indices = (unsigned int*)malloc(sizeof(unsigned int) * vertices_number);
	float* processed_buffer = (float*)malloc(sizeof(float) * vertices_number * (3 + 3 + 2));

	unsigned int i, j;
	for (i = 0; i < vertices_number; i++) {
		int ubicacion_vector_faces = i * 3;
		int ubicacion_processed_buffer = i * (3 + 3 + 2);
		indices[i] = i;
		// Copiamos el vertice
		processed_buffer[ubicacion_processed_buffer + 0] = vertices_penrose[(faces_info[ubicacion_vector_faces] - 1) * 3 + 0];
		processed_buffer[ubicacion_processed_buffer + 1] = vertices_penrose[(faces_info[ubicacion_vector_faces] - 1) * 3 + 1];
		processed_buffer[ubicacion_processed_buffer + 2] = vertices_penrose[(faces_info[ubicacion_vector_faces] - 1) * 3 + 2];
		// Copiamos las normales
		processed_buffer[ubicacion_processed_buffer + 3] = normales_penrose[(faces_info[ubicacion_vector_faces + 2] - 1) * 3 + 0];
		processed_buffer[ubicacion_processed_buffer + 4] = normales_penrose[(faces_info[ubicacion_vector_faces + 2] - 1) * 3 + 1];
		processed_buffer[ubicacion_processed_buffer + 5] = normales_penrose[(faces_info[ubicacion_vector_faces + 2] - 1) * 3 + 2];
		// Copiamos las coordenadas UV
		processed_buffer[ubicacion_processed_buffer + 6] = uv_penrose[(faces_info[ubicacion_vector_faces + 1] - 1) * 2 + 0];
		processed_buffer[ubicacion_processed_buffer + 7] = uv_penrose[(faces_info[ubicacion_vector_faces + 1] - 1) * 2 + 1];
	};

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices_number * (3 + 3 + 2), processed_buffer, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (3 + 3 + 2) * sizeof(float), (void*)(0 * sizeof(float)));
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (3 + 3 + 2) * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, (3 + 3 + 2) * sizeof(float), (void*)((3 + 3) * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertices_number * sizeof(unsigned int), indices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	free(indices);
	free(processed_buffer);
}

void crearEsfera() {
	unsigned int VBO, EBO;

	unsigned int indices[] = {
	0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255,256,257,258,259,260,261,262,263,264,265,266,267,268,269,270,271,272,273,274,275,276,277,278,279,280,281,282,283,284,285,286,287,288,289,290,291,292,293,294,295,296,297,298,299,300,301,302,303,304,305,306,307,308,309,310,311,312,313,314,315,316,317,318,319,320,321,322,323,324,325,326,327,328,329,330,331,332,333,334,335,336,337,338,339,340,341,342,343,344,345,346,347,348,349,350,351,352,353,354,355,356,357,358,359,360,361,362,363,364,365,366,367,368,369,370,371,372,373,374,375,376,377,378,379,380,381,382,383,384,385,386,387,388,389,390,391,392,393,394,395,396,397,398,399,400,401,402,403,404,405,406,407,408,409,410,411,412,413,414,415,416,417,418,419,420,421,422,423,424,425,426,427,428,429,430,431,432,433,434,435,436,437,438,439,440,441,442,443,444,445,446,447,448,449,450,451,452,453,454,455,456,457,458,459,460,461,462,463,464,465,466,467,468,469,470,471,472,473,474,475,476,477,478,479,480,481,482,483,484,485,486,487,488,489,490,491,492,493,494,495,496,497,498,499,500,501,502,503,504,505,506,507,508,509,510,511,512,513,514,515,516,517,518,519,520,521,522,523,524,525,526,527,528,529,530,531,532,533,534,535,536,537,538,539,540,541,542,543,544,545,546,547,548,549,550,551,552,553,554,555,556,557,558,559,560,561,562,563,564,565,566,567,568,569,570,571,572,573,574,575,576,577,578,579,580,581,582,583,584,585,586,587,588,589,590,591,592,593,594,595,596,597,598,599,600,601,602,603,604,605,606,607,608,609,610,611,612,613,614,615,616,617,618,619,620,621,622,623,624,625,626,627,628,629,630,631,632,633,634,635,636,637,638,639,640,641,642,643,644,645,646,647,648,649,650,651,652,653,654,655,656,657,658,659,660,661,662,663,664,665,666,667,668,669,670,671,672,673,674,675,676,677,678,679,680,681,682,683,684,685,686,687,688,689,690,691,692,693,694,695,696,697,698,699,700,701,702,703,704,705,706,707,708,709,710,711,712,713,714,715,716,717,718,719,720,721,722,723,724,725,726,727,728,729,730,731,732,733,734,735,736,737,738,739,740,741,742,743,744,745,746,747,748,749,750,751,752,753,754,755,756,757,758,759,760,761,762,763,764,765,766,767,768,769,770,771,772,773,774,775,776,777,778,779,780,781,782,783,784,785,786,787,788,789,790,791,792,793,794,795,796,797,798,799,800,801,802,803,804,805,806,807,808,809,810,811,812,813,814,815,816,817,818,819,820,821,822,823,824,825,826,827,828,829,830,831,832,833,834,835,836,837,838,839,840,841,842,843,844,845,846,847,848,849,850,851,852,853,854,855,856,857,858,859,860,861,862,863,864,865,866,867,868,869,870,871,872,873,874,875,876,877,878,879,880,881,882,883,884,885,886,887,888,889,890,891,892,893,894,895,896,897,898,899,900,901,902,903,904,905,906,907,908,909,910,911,912,913,914,915,916,917,918,919,920,921,922,923,924,925,926,927,928,929,930,931,932,933,934,935,936,937,938,939,940,941,942,943,944,945,946,947,948,949,950,951,952,953,954,955,956,957,958,959,960,961,962,963,964,965,966,967,968,969,970,971,972,973,974,975,976,977,978,979,980,981,982,983,984,985,986,987,988,989,990,991,992,993,994,995,996,997,998,999,1000,1001,1002,1003,1004,1005,1006,1007,1008,1009,1010,1011,1012,1013,1014,1015,1016,1017,1018,1019,1020,1021,1022,1023,1024,1025,1026,1027,1028,1029,1030,1031,1032,1033,1034,1035,1036,1037,1038,1039,1040,1041,1042,1043,1044,1045,1046,1047,1048,1049,1050,1051,1052,1053,1054,1055,1056,1057,1058,1059,1060,1061,1062,1063,1064,1065,1066,1067,1068,1069,1070,1071,1072,1073,1074,1075,1076,1077,1078,1079
	};

	glGenVertexArrays(1, &VAOEsfera);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	// bind the Vertex Array Object first.
	glBindVertexArray(VAOEsfera);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_esfera), vertices_esfera, GL_STATIC_DRAW);

	// Normales
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	// Textura
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// Vertices
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

void openGlInit() {
	glClearDepth(1.0f); //Valor z-buffer
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); //Borro el Buffer the la ventana
	glEnable(GL_DEPTH_TEST); // z-buffer
	glEnable(GL_CULL_FACE); //ocultacion caras back
	glCullFace(GL_BACK);
	glEnable(GL_TEXTURE_2D);
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	glm::mat4 rotacionY = glm::rotate(glm::mat4(1), glm::radians(anguloPlanoY), glm::vec3(0, 1, 0));
	glm::mat4 rotacionX = glm::rotate(glm::mat4(1), glm::radians(anguloY), glm::vec3(1, 0, 0));
	vectorPerspectivaIsometrica = rotacionX * rotacionY * glm::vec4(0, 0, 1, 1);
	glm::vec3 eye = vectorPerspectivaIsometrica * 200.0f + glm::vec3(50, 0, 0);
	glm::vec3 lookAt = glm::vec3(50, 0, 0);

	lightPos = glm::vec3(50*sin(glm::radians(15*glfwGetTime())), 20 * (0.5-sin(glm::radians(15 * glfwGetTime()))), 50*cos(glm::radians(15*glfwGetTime())));

	glm::mat4 projection = glm::ortho(-((float)SCR_WIDTH / 2.0f), ((float)SCR_WIDTH / 2.0f), -((float)SCR_HEIGHT / 2.0f), ((float)SCR_HEIGHT / 2.0f), -200.0f, 400.0f);

	esfera.setMatWorld(glm::translate(glm::mat4(), myControlador.getPosicion()));
	if (pintarAlgoritmoPintor) {
		glDisable(GL_DEPTH_TEST);
		penroseFrontal.dibujar(eye, lookAt, projection);
		penroseTrasero.dibujar(eye, lookAt, projection);
		esfera.dibujar(eye, lookAt, projection);
		penroseLateral.dibujar(eye, lookAt, projection);
		glEnable(GL_DEPTH_TEST);
	}
	else {
		penroseTrasero.dibujar(eye, lookAt, projection);
		penroseLateral.dibujar(eye, lookAt, projection);
		penroseFrontal.dibujar(eye, lookAt, projection);
		esfera.dibujar(eye, lookAt, projection);
	}
}

void movimiento() {
	myControlador.actualizar(glfwGetTime());
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

	crearPenroseFrontal();
	crearPenroseLateral();
	crearPenroseTrasero();
	crearEsfera();

	GLuint shaderSinTexturas = setShaders("shader.vert", "shader.frag");
	GLuint shaderConTexturas = setShaders("shaderTextura.vert", "shaderTextura.frag");

	GLuint cemento = cargarTextura("2k_venus_surface.jpg");

	glUniform1i(glGetUniformLocation(shaderConTexturas, "tex"), 0);

	penroseFrontal = Objeto(glm::vec3(1.0f, 1.0f, 1.0f), VAOPenroseFrontal, 99, shaderSinTexturas);
	penroseFrontal.setDistorsion(glm::vec3(0.05, 0.05, 0.05));

	penroseTrasero = Objeto(glm::vec3(1.0f, 1.0f, 1.0f), VAOPenroseTrasero, 99, shaderSinTexturas);
	penroseTrasero.setDistorsion(glm::vec3(0.05, 0.05, 0.05));

	penroseLateral = Objeto(glm::vec3(1.0f, 1.0f, 1.0f), VAOPenroseLateral, 99, shaderSinTexturas);
	penroseLateral.setDistorsion(glm::vec3(0.05, 0.05, 0.05));

	esfera = Objeto(glm::vec3(1.0f, 1.0f, 1.0f), VAOEsfera, 1080, shaderConTexturas);
	esfera.setDistorsion(glm::vec3(0.2, 0.2, 0.2));
	
	esfera.setTextura(cemento);

	myControlador.comenzar(glfwGetTime());

	while (!glfwWindowShouldClose(window))
	{
		display();
		tiempo();
		movimiento();

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
		anguloY+= 0.01;
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		anguloY-= 0.01;
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		anguloPlanoY+=0.01;
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		anguloPlanoY-= 0.01;

	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		posEsferaX++;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		posEsferaX--;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		posEsferaY++;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		posEsferaY--;
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		posEsferaZ++;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		posEsferaZ--;
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
		pintarAlgoritmoPintor = !pintarAlgoritmoPintor;

	std::cout << "x " << posEsferaX;
	std::cout << ", y " << posEsferaY;
	std::cout << ", z " << posEsferaZ << std::endl << std::endl;

	//std::cout << "alfaPY " << anguloPlanoY;
	//std::cout << ", alfaY " << anguloY << std::endl << std::endl;
}