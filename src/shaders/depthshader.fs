#version 460 core
in vec4 fragPos;
in vec2 texCoords;

uniform sampler2D texture1;
uniform vec2 tiling;

out vec4 FragColor;

float near = 0.1; 
float far  = 100.0; 
  
float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

void main()
{             
    //float depth = LinearizeDepth(gl_FragCoord.z) / far; // divide by far for demonstration
    //FragColor = vec4(vec3(depth), 1.0);
    vec4 texColor = texture(texture1, texCoords);

    if(texColor.a < 0.1f)
        discard;
    FragColor = texColor;
}
