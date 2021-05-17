#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 UV;

out vec4 FragColor;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 objectColor;

uniform float ambientI;
uniform float diffuseI;
uniform float specularI;

void main()
{
	// luz ambiente
	vec3 ambiente = vec3(1.0f) * ambientI;

	// luz difusa
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(lightPos - FragPos); // No queremos tener en cuenta la posición del fragmento para que no se note la distancia en 3D
	float diferencia = max(dot(norm, lightDir), 0.0f);
	vec3 luzDifusa = vec3(diferencia) * diffuseI;

	vec3 lightIntensity = ambiente + luzDifusa;

	FragColor = vec4(lightIntensity * lightColor * objectColor , 1.0f);
}