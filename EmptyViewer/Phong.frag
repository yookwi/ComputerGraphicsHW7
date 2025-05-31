#version 330 core
out vec4 FragColor;

in vec3 wPosition;
in vec3 wNormal;
in vec3 wColor;

uniform vec3 cameraPos;
uniform vec3 lightPos;
uniform float la;
uniform float p;
uniform float intensity;

vec4 Shading(vec3 wPosition, vec3 wNormal, vec3 wColor);

void main() {
	FragColor = Shading(wPosition, wNormal, wColor);
}

vec4 Shading(vec3 wPosition, vec3 wNormal, vec3 wColor){
	wNormal = normalize(wNormal);

	vec3 ka = vec3(0,1,0);
	vec3 kd = vec3(0, 0.5, 0);
	vec3 ks = vec3(0.5, 0.5, 0.5);
	float r = 2.2;

	vec3 lay = lightPos - wPosition;
	lay = normalize(lay);
	vec3 view = cameraPos - wPosition;
	view = normalize(view);
	vec3 h = view + lay;
	h = normalize(h);
	vec3 light = ka * la + kd * intensity * max(0.0, dot(wNormal, lay)) + ks * intensity * pow(max(0.0, dot(wNormal, h)), p);
	light = pow(light, vec3(1.0 / r));
	return vec4(light, 1.0f);
}