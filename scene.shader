#shader vertex
#version 120

attribute vec4 a_Position;
attribute vec4 a_Normal;
attribute vec2 a_TexCoord;

varying vec4 fragPos;
varying vec4 normal;
varying vec2 texCoord;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Proj;

void main(){
	fragPos = u_Model * a_Position;
	normal = u_Model * a_Normal;
	texCoord = a_TexCoord;
	gl_Position = u_Proj * u_View * fragPos;
}

#shader fragment
#version 120

varying vec4 fragPos;
varying vec4 normal;
varying vec2 texCoord;

uniform vec3 u_LightPos;
uniform vec3 u_LightColour;
uniform float u_LightBrightness;
uniform vec3 u_ObjectColour;
uniform float u_AmbientStrength;
uniform bool u_IsTextured;
uniform sampler2D u_Texture;

void main(){
	vec3 ambient = u_AmbientStrength * u_LightColour;
	vec4 norm = normalize(normal);
	vec4 texColour;
	if (u_IsTextured)
		texColour = texture2D(u_Texture, texCoord);
	else
		texColour = vec4(1, 1, 1, 1);
	vec4 lightDir = normalize(vec4(u_LightPos, 1.f) - fragPos);
	float diffuseStrength = max(dot(norm, lightDir), 0.0f) * u_LightBrightness;	
	vec3 diffuse = diffuseStrength * u_LightColour;
	vec3 result = (ambient + diffuse) * u_ObjectColour;
	gl_FragColor = vec4(result, 1.0f) * texColour;
}
