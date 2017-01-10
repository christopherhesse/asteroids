precision highp float;

uniform sampler2D tex;
uniform bool horizontal;

// http://dev.theomader.com/gaussian-kernel-calculator/
// sigma=2 kernel=9
const float weight[5] = float[](0.20236, 0.179044, 0.124009, 0.067234, 0.028532);

out lowp vec4 color;

void main() {
    ivec2 center = ivec2(gl_FragCoord.xy);

    vec3 result = texelFetch(tex, center, 0).rgb * weight[0];

    if (horizontal) {
        result += texelFetchOffset(tex, center, 0, ivec2(1, 0)).rgb * weight[1];
        result += texelFetchOffset(tex, center, 0, ivec2(-1, 0)).rgb * weight[1];
        result += texelFetchOffset(tex, center, 0, ivec2(2, 0)).rgb * weight[2];
        result += texelFetchOffset(tex, center, 0, ivec2(-2, 0)).rgb * weight[2];
        result += texelFetchOffset(tex, center, 0, ivec2(3, 0)).rgb * weight[3];
        result += texelFetchOffset(tex, center, 0, ivec2(-3, 0)).rgb * weight[3];
        result += texelFetchOffset(tex, center, 0, ivec2(4, 0)).rgb * weight[4];
        result += texelFetchOffset(tex, center, 0, ivec2(-4, 0)).rgb * weight[4];
    } else {
        result += texelFetchOffset(tex, center, 0, ivec2(0, 1)).rgb * weight[1];
        result += texelFetchOffset(tex, center, 0, ivec2(0, -1)).rgb * weight[1];
        result += texelFetchOffset(tex, center, 0, ivec2(0, 2)).rgb * weight[2];
        result += texelFetchOffset(tex, center, 0, ivec2(0, -2)).rgb * weight[2];
        result += texelFetchOffset(tex, center, 0, ivec2(0, 3)).rgb * weight[3];
        result += texelFetchOffset(tex, center, 0, ivec2(0, -3)).rgb * weight[3];
        result += texelFetchOffset(tex, center, 0, ivec2(0, 4)).rgb * weight[4];
        result += texelFetchOffset(tex, center, 0, ivec2(0, -4)).rgb * weight[4];
    }

    color = vec4(result, 1.0);
}