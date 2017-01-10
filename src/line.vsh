precision highp float;

in vec2 start;
in vec2 end;

uniform mat3 transform;
uniform vec2 resolution;
uniform float width;
uniform vec2 offset;

out vec2 screenStart;
out vec2 screenEnd;

void main() {
    vec2 clipStart = vec2(transform * vec3(start + offset, 1));
    vec2 clipEnd = vec2(transform * vec3(end + offset, 1));
    screenStart = (vec2(clipStart) + 1.0) / 2.0 * resolution;
    screenEnd = (vec2(clipEnd) + 1.0) / 2.0 * resolution;

    vec2 traverse = normalize(screenEnd - screenStart) * width * 5.0 / 2.0;
    vec2 normal = vec2(traverse.y, -traverse.x);
    // so these are orthogonal in screen space now, need to convert to clip space to apply to the line
    traverse = traverse / resolution * 2.0;
    normal = normal / resolution * 2.0;

    vec2 pos;
    switch (gl_VertexID % 6) {
        case 0:
            pos = clipStart - traverse - normal;
            break;
        case 1:
            pos = clipEnd + traverse - normal;
            break;
        case 2:
            pos = clipStart - traverse + normal;
            break;
        case 3:
            pos = clipEnd + traverse + normal;
            break;
        case 4:
            pos = clipEnd + traverse - normal;
            break;
        case 5:
            pos = clipStart - traverse + normal;
            break;
    }
    gl_Position = vec4(pos, 0, 1);
}
