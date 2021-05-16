#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 UV;

out vec4 FragColor;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 objectColor;

void main()
{
	// luz ambiente
	vec3 ambiente = vec3(0.4f);

	// luz difusa
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(lightPos - FragPos);
	float diferencia = max(dot(norm, lightDir), 0.0f);
	vec3 luzDifusa = vec3(diferencia);

	vec3 lightIntensity = ambiente + luzDifusa;

	FragColor = vec4(lightIntensity * lightColor, 1.0f);
}