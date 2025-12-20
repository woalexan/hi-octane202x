
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
uniform vec3 mViewerPos;

in vec3 FragPos;
in vec3 Normal;
in vec3 viewPos;

void main (void)
{
    vec3 nlightDir = normalize(-dirLight.lightDir);
    vec3 ambient = dirLight.ambient * vec3(texture2D(material.diffuseTex, vec2(gl_TexCoord[0]))); 
    
    vec3 norm = normalize(Normal);
    
    vec3 viewDir    = normalize(mViewerPos - FragPos);
    vec3 halfwayDir = normalize(nlightDir + viewDir);
    
    float diff = max(dot(norm, nlightDir), 0.0);
    vec3 diffuse = dirLight.diffuse * diff * vec3(texture2D(material.diffuseTex, vec2(gl_TexCoord[0]))); 
    
    float spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
    vec3 specular = dirLight.specular * (spec * material.specular); 
    
    vec3 result = (ambient + diffuse + specular);
    gl_FragColor = vec4(result, 1.0);
}
