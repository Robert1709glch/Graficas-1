#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormal;

uniform mat4 view; 
uniform mat4 projection;
uniform mat4 model;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

out vec3 v_Color;

void main()
{	
    v_Color = vec3(0.4f,0.1f,0.2f);
	FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;  
    TexCoords = aTexCoords;
	gl_Position = projection * view  * model* vec4(aPos, 1.0f);
}
