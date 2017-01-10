precision highp float;

in highp vec2 uv;

uniform sampler2D tex;

out lowp vec4 color;

#define THRESHOLD 0.5

void main() {
    vec3 pixel = texture(tex, uv).xyz;
    if (pixel.x > THRESHOLD) {
        color = vec4((pixel - THRESHOLD) / (1.0 - THRESHOLD), 1.0);
    } else {
        color = vec4(0.0, 0.0, 0.0, 1.0);
    }
}