#version 330 core
in vec2 TexCoords;

uniform sampler2D text;
uniform vec3 textColor;
out vec4 FragColor;

void main()
{    
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r)*vec4(textColor,1.0);
    if(sampled.a < 0.1)
        discard;
	FragColor = sampled;
}  