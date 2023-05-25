#version 330 core
layout(location = 0) in vec3 vertex;


out vec3 TexCoords;


uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;
uniform vec3 campos;

out vec3 FragPos;
out vec3 Normal;
void main()
{
    FragPos = vec3(model * vec4(vertex, 1.0));
    Normal = mat3(transpose(inverse(model))) * vec3(1.0f);  
    gl_Position = projection * view  * model * vec4(vertex, 1.0f);
    TexCoords = vertex; 
}