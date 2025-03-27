#version 330 core

out vec4 FragColor;

uniform vec3 lightColor;
in vec3 baseColor;
void main()
{
    FragColor = vec4(baseColor, 1.0f); // set all 4 vector values to 1.0
}
