in highp vec2 position;
out highp vec2 uv;

void main() {
    gl_Position = vec4(position, 0, 1);
    uv = (position.xy + vec2(1, 1)) / 2.0;
}