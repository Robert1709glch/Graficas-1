#version 330 core
out vec4 FragColor;
in vec2 TexCoord;

// texture samplers
uniform sampler2D texture1;

void main()
{    
    vec4 texColor = texture(texture1, TexCoord*1.2);
    if(texColor.a < 0.1)
        discard;
	FragColor = vec4(texColor.xyz,texColor.a);
}