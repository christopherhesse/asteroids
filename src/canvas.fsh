precision highp float;

in highp vec2 fragUV;

uniform sampler2D tex;
uniform vec4 color;

out lowp vec4 outputColor;

void main() {
    outputColor = color * texture(tex, fragUV);
}