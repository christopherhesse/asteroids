precision highp float;

uniform float decay;
uniform sampler2D tex;

out lowp vec4 color;

void main() {
    vec3 pixel = texelFetch(tex, ivec2(gl_FragCoord.xy), 0).xyz;
    color = vec4(pixel * decay, 1);
}