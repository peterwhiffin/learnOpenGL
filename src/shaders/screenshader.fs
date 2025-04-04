#version 460 core

#define EDGE_STEP_COUNT 10
#define EDGE_STEPS 1, 1.5, 2, 2, 2, 2, 2, 2, 2, 4
#define EDGE_GUESS 8

struct LuminanceData {
    float m, n, e, s, w;
    float ne, nw, se, sw;
    float highest, lowest, contrast;
};

struct EdgeData{
    bool isHorizontal;
    float pixelStep;
    float oppositeLuminance, gradient;
};

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform bool doFXAA;
uniform bool useAlpha;

const float offset = 1.0 / 300.0;

const float edgeSteps[EDGE_STEP_COUNT] = { EDGE_STEPS };

float sampleLuminance(vec2 uv){
    return texture(screenTexture, uv).a;
}

float sampleLuminance(vec2 uv, float uOffset, float vOffset){
    uv += vec2(1.0) / vec2(textureSize(screenTexture, 0)) * vec2(uOffset, vOffset);
    
    if(useAlpha){
        return texture(screenTexture, uv).a;
    }
    else{
        return texture(screenTexture, uv).g;
    }
}

LuminanceData SampleLuminanceNeighborhood (vec2 uv){
    LuminanceData l;
    l.m = sampleLuminance(uv, 0, 0);
    l.n = sampleLuminance(uv, 0, 1);
    l.e = sampleLuminance(uv, 1, 0);
    l.s = sampleLuminance(uv, 0, -1);
    l.w = sampleLuminance(uv, -1, 0);

    l.ne = sampleLuminance(uv, 1, 1);
    l.nw = sampleLuminance(uv, -1, 1);
    l.se = sampleLuminance(uv, 1, -1);
    l.sw = sampleLuminance(uv, -1, -1);

    l.highest = max(max(max(max(l.n, l.e), l.s), l.w), l.m);
    l.lowest = min(min(min(min(l.n, l.e), l.s), l.w), l.m);
    l.contrast = l.highest - l.lowest;

    return l;
}

bool shouldSkipPixel(LuminanceData l){
    float threshold = max(0.0312f, 0.063f * l.highest);
    return l.contrast < threshold;
}

float DeterminePixelBlendFactor(LuminanceData l){
    float blendFilter = 2 * (l.n + l.e + l.s + l.w);
    blendFilter += l.ne + l.nw + l.se + l.sw;
    blendFilter *= 1.0 / 12;
    blendFilter = abs(blendFilter - l.m);
    blendFilter = clamp(blendFilter / l.contrast, 0.0, 1.0);
    float blendFactor = smoothstep(0, 1, blendFilter);
    return blendFactor * blendFactor;
}

EdgeData DetermineEdge(LuminanceData l){
    EdgeData e;
    float horizontal =
        abs(l.n + l.s - 2 * l.m) * 2 +
        abs(l.ne + l.se - 2 * l.e) +
        abs(l.nw + l.sw - 2 * l.w);
    float vertical =
        abs(l.e + l.w - 2 * l.m) * 2 +
        abs(l.ne + l.nw - 2 * l.n) +
        abs(l.se + l.sw - 2 * l.s);
    e.isHorizontal = horizontal >= vertical;

    vec2 texelSize = vec2(1.0) / vec2(textureSize(screenTexture, 0));

    float pLuminance = e.isHorizontal ? l.n : l.e;
    float nLuminance = e.isHorizontal ? l.s : l.w;
    float pGradient = abs(pLuminance - l.m);
    float nGradient = abs(nLuminance - l.m);

    e.pixelStep = e.isHorizontal ? texelSize.y : texelSize.x;

    if(pGradient < nGradient){
        e.pixelStep = -e.pixelStep;
        e.oppositeLuminance = nLuminance;
        e.gradient = nGradient;
    }
    else{
        e.oppositeLuminance = pLuminance;
        e.gradient = pGradient;
    }

    return e;
}

float DetermineEdgeBlendFactor(LuminanceData l, EdgeData e, vec2 uv){
    vec2 uvEdge = uv;
    vec2 edgeStep;
    vec2 texelSize = vec2(1.0) / vec2(textureSize(screenTexture, 0));

    if(e.isHorizontal){
        uvEdge.y += e.pixelStep * 0.5f;
        edgeStep = vec2(texelSize.x, 0);
    }
    else{
        uvEdge.x += e.pixelStep * 0.5f;
        edgeStep = vec2(0, texelSize);
    }

    float edgeLuminance = (l.m + e.oppositeLuminance) * 0.5;
    float gradientThreshold = e.gradient * 0.25;

    vec2 puv = uvEdge + edgeStep * edgeSteps[0];
    float pLuminanceDelta = sampleLuminance(puv) - edgeLuminance;
    bool pAtEnd = abs(pLuminanceDelta) >= gradientThreshold;

    for(int i = 1; i < EDGE_STEP_COUNT && !pAtEnd; i++){
        puv += edgeStep * edgeSteps[i];
        pLuminanceDelta = sampleLuminance(puv) - edgeLuminance;
        pAtEnd = abs(pLuminanceDelta) >= gradientThreshold;
    }

    if(!pAtEnd){
        puv += edgeStep * EDGE_GUESS;
    }

    vec2 nuv = uvEdge - edgeStep * edgeSteps[0];
    float nLuminanceDelta = sampleLuminance(nuv) - edgeLuminance;
    bool nAtEnd = abs(nLuminanceDelta) >= gradientThreshold;

    for(int i = 1; i < EDGE_STEP_COUNT && !nAtEnd; i++){
        nuv -= edgeStep * edgeSteps[i];
        nLuminanceDelta = sampleLuminance(nuv) - edgeLuminance;
        nAtEnd = abs(nLuminanceDelta) >= gradientThreshold;
    }    

    if(!nAtEnd){
        nuv -= edgeStep * EDGE_GUESS;
    }

    float pDistance, nDistance;

    if(e.isHorizontal){
        pDistance = puv.x - uv.x;
        nDistance = uv.x - nuv.x;
    }
    else{
        pDistance = puv.y - uv.y;
        nDistance = uv.y - nuv.y;
    }

    float shortestDistance;
    bool deltaSign;

    if(pDistance <= nDistance){
        shortestDistance = pDistance;
        deltaSign = pLuminanceDelta >= 0;
    }
    else{
        shortestDistance = nDistance;
        deltaSign = nLuminanceDelta >= 0;
    }

    if(deltaSign == (l.m - edgeLuminance >= 0)){
        return 0;
    }

    return 0.5 - shortestDistance / (pDistance + nDistance);
}

void main(){
//vec4 color = texture(screenTexture, TexCoords);

LuminanceData l = SampleLuminanceNeighborhood(TexCoords);

if(shouldSkipPixel(l)){
    FragColor = textureLod(screenTexture, TexCoords, 0);
    //FragColor = vec4(0);
    return;
}

vec2 uv = TexCoords;
float pixelBlend = DeterminePixelBlendFactor(l);
EdgeData e = DetermineEdge(l);

//FragColor = vec4(vec3(DetermineEdgeBlendFactor(l, e, uv) - pixelBlend), 1.0f);

float edgeBlend = DetermineEdgeBlendFactor(l, e, uv);
float finalBlend = max(pixelBlend, edgeBlend);

if(e.isHorizontal){
    uv.y += e.pixelStep * finalBlend;
}
else{
    uv.x += e.pixelStep * finalBlend;
}


FragColor = vec4(vec3(textureLod(screenTexture, uv, 0)), 1.0f);

return;

float gamma = 2.2;
//FragColor = vec4(vec3(1.0 - texture(screenTexture, TexCoords)), 1.0);
FragColor = texture(screenTexture, TexCoords * 2);
FragColor.rgb = pow(FragColor.rgb, vec3(1.0/gamma));
//float depthValue = texture(screenTexture, TexCoords).r;
//FragColor = vec4(vec3(depthValue), 1.0);
return;
vec2 offsets[9] = vec2[](
        vec2(-offset,  offset), // top-left
        vec2( 0.0f,    offset), // top-center
        vec2( offset,  offset), // top-right
        vec2(-offset,  0.0f),   // center-left
        vec2( 0.0f,    0.0f),   // center-center
        vec2( offset,  0.0f),   // center-right
        vec2(-offset, -offset), // bottom-left
        vec2( 0.0f,   -offset), // bottom-center
        vec2( offset, -offset)  // bottom-right    
    );

    //float kernel[9] = float[](
    //    -1, -1, -1,
    //    -1,  9, -1,
    //    -1, -1, -1
   // );

   // float kernel[9] = float[](
   // 1.0 / 16, 2.0 / 16, 1.0 / 16,
   // 2.0 / 16, 4.0 / 16, 2.0 / 16,
   // 1.0 / 16, 2.0 / 16, 1.0 / 16  
      //);
      
    //float kernel[9] = float[](
    //    1, 1, 1, 
    //    1, -8, 1,
    //    1, 1, 1
    //);
    
    float kernel[9] = float[](
        1, 1, 1, 
        1, -8, 1,
        1, 1, 1
    );

    vec3 sampleTex[9];
    for(int i = 0; i < 9; i++)
    {
        sampleTex[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
    }
    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++)
        col += sampleTex[i] * kernel[i];
    
    FragColor = vec4(col, 1.0);
    }
