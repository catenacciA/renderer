#include <metal_stdlib>
using namespace metal;

struct v2f {
    float4 position [[position]];
    float3 normal;
    half3 color;
    float3 worldPos;
};

struct VertexData {
    float3 position;
    float3 normal;
};

struct InstanceData {
    float4x4 instanceTransform;
    float3x3 instanceNormalTransform;
    float4 instanceColor;
};

struct CameraData {
    float4x4 perspectiveTransform;
    float4x4 worldTransform;
    float3x3 worldNormalTransform;
    float3 cameraPosition;
};

struct LightData {
    float3 position;
    half3 color;
    float intensity;
    float range;
    float pulseSpeed;
    float time;
};

v2f vertex vertexMain(device const VertexData* vertexData [[buffer(0)]],
                      device const InstanceData* instanceData [[buffer(1)]],
                      device const CameraData& cameraData [[buffer(2)]],
                      uint vertexId [[vertex_id]],
                      uint instanceId [[instance_id]]) {
    v2f o;
    
    const device VertexData& vd = vertexData[vertexId];
    float4 pos = float4(vd.position, 1.0);
    
    float4 worldPos = instanceData[instanceId].instanceTransform * pos;
    o.worldPos = worldPos.xyz;
    o.position = cameraData.perspectiveTransform * cameraData.worldTransform * worldPos;
    
    float3 worldNormal = instanceData[instanceId].instanceNormalTransform * vd.normal;
    o.normal = normalize(worldNormal);
    
    o.color = half3(instanceData[instanceId].instanceColor.rgb);
    return o;
}

half4 fragment fragmentMain(v2f in [[stage_in]], 
                          constant CameraData& cameraData [[buffer(0)]],
                          constant LightData& lightData [[buffer(1)]]) {
    float3 normal = normalize(in.normal);
    
    float3 lightVec = lightData.position - in.worldPos;
    float distance = length(lightVec);
    float3 lightDir = normalize(lightVec);
    
    float attenuation = saturate(1.0 - distance / lightData.range);
    attenuation *= attenuation;
    
    half ndotl = saturate(dot(normal, lightDir));
    
    float3 viewDir = normalize(cameraData.cameraPosition - in.worldPos);
    float3 halfVector = normalize(lightDir + viewDir);
    half specular = half(pow(float(saturate(dot(normal, halfVector))), 16.0)) * 0.3h;
    
    half pulse = lightData.pulseSpeed > 0.0 ? 
                 (sin(lightData.time * lightData.pulseSpeed) * 0.5 + 0.5) : 
                 1.0h;
    
    half3 ambient = in.color * 0.2h;
    half lightIntensity = lightData.intensity * attenuation * pulse;
    half3 diffuse = in.color * ndotl * lightIntensity * lightData.color;
    half3 specularContrib = specular * lightIntensity * lightData.color;
    
    half3 finalColor = ambient + diffuse + specularContrib;
    
    return half4(finalColor, 1.0h);
}
