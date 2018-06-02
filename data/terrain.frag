#version 150 core
// fragment shader for simple terrain application

// per-frame data
layout(std140)                  // standard layout
uniform SceneData {             // like a class name
    mat4 modelViewMatrix, modelViewInverse;
    mat4 projectionMatrix, projectionInverse;
    vec4 fog;
	vec4 normRelief;
};

// shader settings
uniform sampler2D colorTexture;

// input from vertex shader
in vec3 normal;
in vec2 texcoord;
in vec3 normalMap;
in vec4 position;
in mat3 TBNMat;

// output to frame buffer
out vec4 fragColor;

void main() {
    vec3 N = normalize(normal);      // surface normal
	if(normRelief.x != 0)
	{
		N = normalize(normalMap); //normal map instead of surface normal
	}
    vec3 L = normalize(vec3(-1,1,1)); // light direction
    float diff = max(0., dot(N,L));  // diffuse lighting

    // color from texture and diffuse
    vec3 color = texture(colorTexture, texcoord).rgb * diff;

    // fade to white with fog
    if (fog.a != 0)
        color = mix(fog.rgb, color, exp2(.005 * position.z));

//	color = normalize(normalMap);
    // final color
    fragColor = vec4(color, 1);
}
