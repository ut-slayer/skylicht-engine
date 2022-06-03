in vec4 inPosition;
in vec4 inColor;
in vec3 inNormal;
in vec2 inTexCoord0;

uniform mat4 uMvpMatrix;
uniform mat4 uWorldMatrix;
uniform mat4 uView;
uniform vec4 uUVScale;

out vec3 vWorldNormal;
out vec4 vWorldPosition;

out vec2 vTexCoord0;

void main(void)
{
	vWorldPosition = uWorldMatrix*inPosition;

	vec4 sampleFragPos = uView * vWorldPosition;
	vWorldPosition.w = sampleFragPos.z;

	vec4 worldNormal = uWorldMatrix * vec4(inNormal, 0.0);
	vWorldNormal = normalize(worldNormal.xyz);

	vTexCoord0 = inTexCoord0 * uUVScale.xy + uUVScale.zw;

	gl_Position = uMvpMatrix * inPosition;
}