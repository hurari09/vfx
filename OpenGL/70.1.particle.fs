#version 330 core
out vec4 FragColor;

in vec4 Color;
in vec2 TexCoords;

uniform sampler2D texture1;
uniform bool useTexture;

void main()
{
    if(useTexture)
    {
        FragColor = texture(texture1, TexCoords);
    }
    else
    {
        FragColor = Color;
    }
}