#version 330 core

// Data from vertex shader.
// --------------------------------------------------------
// Add your data for interpolation.
in vec3 iPosWorld;
in vec3 iNormalWorld;
in vec2 iTexCoord;
// --------------------------------------------------------

// --------------------------------------------------------
// Add your uniform variables.
uniform sampler2D mapKd;
//uniform bool hasMapKd;

uniform mat4 worldMatrix;
uniform mat4 normalMatrix;
uniform vec3 cameraPos;

uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;
uniform float Ns;
// Light data.
uniform vec3 ambientLight;
uniform vec3 dirLightDir;
uniform vec3 dirLightRadiance;
uniform vec3 pointLightPos;
uniform vec3 pointLightIntensity;
uniform vec3 SpotLightPos;
uniform vec3 SpotLightIntensity;
uniform vec3 SpotlightDirection;
uniform float Cutoff;
uniform float Totalwidth;
// --------------------------------------------------------

out vec4 FragColor;

// --------------------------------------------------------
vec3 Diffuse(vec3 Kd, vec3 I, vec3 N, vec3 lightDir)
{
    return Kd * I * max(0, dot(N, lightDir));
}
vec3 Specular(vec3 Ks, vec3 I, vec3 N, vec3 lightDir, vec3 view,float Ns, const float mode)
{
    if(mode == 0){
        //Phong
        vec3 vR = 2 * (dot(N,lightDir)*N)-lightDir;
        return Ks * I * pow(max(0.0,dot(view,vR)),Ns);
    }
    else{
        //Blinn-Phong
        vec3 vH = (lightDir+view)/length(lightDir+view);
        return Ks * I *pow(max(0.0 , dot(N,vH)), Ns);
    }
    
}


void main()
{
    vec3 N=normalize(iNormalWorld);
    vec3 view = normalize(cameraPos - iPosWorld);
    vec3 texColor;
    texColor = texture2D(mapKd, iTexCoord).rgb;
   
    // Ambient light.
    vec3 ambient = Ka * ambientLight;
    // -------------------------------------------------------------
    // Directional light.
    vec3 diffuse;
    vec3 specular;
    // Diffuse.
    //if(hasMapKd){
    //    diffuse = Diffuse(texColor, dirLightRadiance, N, normalize(-dirLightDir));
    //}
    //else{
    //    diffuse = Diffuse(Kd, dirLightRadiance, N, normalize(-dirLightDir));
    //}
    diffuse = Diffuse(texColor*Kd, dirLightRadiance, N, normalize(-dirLightDir));
    //Specular
    specular = Specular(Ks, dirLightRadiance, N, normalize(-dirLightDir), view, Ns, 1);
    vec3 dirLight = diffuse + specular;
    // -------------------------------------------------------------
    // Point light.
    vec3 vspLightDir = normalize(pointLightPos - iPosWorld);
    float distSurfaceToLight = distance(pointLightPos, iPosWorld);
    float attenuation_Dist = 1.0 / (distSurfaceToLight * distSurfaceToLight);
    vec3 radiance = pointLightIntensity * attenuation_Dist;
    // Diffuse.
    //if(hasMapKd){
    //    diffuse = Diffuse(texColor, radiance, N, vspLightDir);
    //}
    //else{
    //    diffuse = Diffuse(Kd, radiance, N, vspLightDir);
    //}
    diffuse = Diffuse(texColor*Kd, radiance, N, vspLightDir);
    // Specular.
    specular = Specular(Ks, radiance, N, vspLightDir, view, Ns, 1);
    vec3 pointLight = diffuse + specular ;
    // -------------------------------------------------------------
    // Spotlight.
    vec3 vssLightDir = normalize(SpotLightPos - iPosWorld);
    float COStheta = (dot(vssLightDir,-SpotlightDirection))/(length(vssLightDir)*length(SpotlightDirection));
    float COScutoff = cos(Cutoff);
    float COStotalwidth = cos(Totalwidth);
    float attenuation_Spot;
    if(COStheta >= COScutoff)
    {
        attenuation_Spot = 1.0;
    }
    else if(COStheta >= COStotalwidth)
    {
        attenuation_Spot = 1.0 - (COStheta - COScutoff)/(COStotalwidth - COScutoff);
    }
    else
    {
        attenuation_Spot = 0.0;
    }
    distSurfaceToLight = distance(SpotLightPos, iPosWorld);
    attenuation_Dist = 1.0f / (distSurfaceToLight * distSurfaceToLight);
    radiance = SpotLightIntensity * attenuation_Dist * attenuation_Spot;
    // Diffuse.
    //if(hasMapKd){
    //    diffuse = Diffuse(texColor, radiance, N, vssLightDir);
    //}
    //else{
    //    diffuse = Diffuse(Kd, radiance, N, vssLightDir);
    //}
    diffuse = Diffuse(texColor*Kd, radiance, N, vssLightDir);
    // Specular.
    specular = Specular(Ks, radiance, N, vssLightDir ,view , Ns, 1);
    vec3 spotLight = diffuse + specular;
    // --------------------------------------------------------
    FragColor = vec4(ambient+dirLight+pointLight+spotLight, 1.0);
    // --------------------------------------------------------
}
