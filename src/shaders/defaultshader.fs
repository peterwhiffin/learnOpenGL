#version 330 core

out vec4 FragColor;

in vec3 bigColor;
in vec3 vertPos;
in vec2 texCoord;

uniform sampler2D mainTex;
uniform sampler2D texture2;

void main()
{
    FragColor = mix(texture(mainTex, texCoord), texture(texture2, vec2(texCoord.x * .55, texCoord.y * .45)), 0.2);
}