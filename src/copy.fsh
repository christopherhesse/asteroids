precision highp float;

in highp vec2 uv;

uniform sampler2D tex;

out lowp vec4 color;

void main() {
    color = texture(tex, uv);
}