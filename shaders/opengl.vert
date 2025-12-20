
uniform mat4 mWorldViewProj;
uniform mat4 mInvWorld;
uniform mat4 mTransWorld;
uniform vec3 mViewerPos;

struct Material {
    sampler2D diffuseTex;
    vec3 specular;
    float shininess;
}; 
  
uniform Material material;

struct DirectionalLight {
    vec3 lightDir;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform DirectionalLight dirLight;   

varying vec3 FragPos;  
varying vec3 Normal;

void main(void)
{
	gl_Position = mWorldViewProj * gl_Vertex;
	vec4 worldpos = gl_Vertex * mTransWorld;
	
	FragPos = worldpos.xyz;
	Normal = gl_Normal;
	
	gl_TexCoord[0] = gl_MultiTexCoord0;
}
