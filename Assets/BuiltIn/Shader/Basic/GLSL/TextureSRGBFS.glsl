// File Generated by Assets/BuildShader.py - source: [TextureSRGBFS.d.glsl : _]
precision mediump float;
uniform sampler2D uTexDiffuse;
uniform vec4 uColor;
uniform vec2 uIntensity;
in vec2 varTexCoord0;
in vec4 varColor;
out vec4 FragColor;
const float gamma = 2.2;
const float invGamma = 1.0 / 2.2;
vec3 sRGB(vec3 color)
{
	return pow(color, vec3(gamma));
}
vec3 linearRGB(vec3 color)
{
	return pow(color, vec3(invGamma));
}
void main(void)
{
	vec4 result = textureLod(uTexDiffuse, varTexCoord0.xy, 0.0) * varColor * uColor;
	FragColor = vec4(sRGB(result.rgb * uIntensity.x), result.a);
}
