precision highp float;

in highp vec2 xy;
in highp vec2 uv;

uniform vec2 resolution;

out highp vec2 fragUV;

void main() {
    gl_Position = vec4((xy / resolution - 0.5) * 2.0, 0, 1);
    fragUV = uv;
}