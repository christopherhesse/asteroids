precision highp float;

in vec2 screenStart;
in vec2 screenEnd;

uniform float width;
uniform float intensity;

out vec4 color;

float minimum_distance(vec2 a, vec2 b, vec2 p) {
    // line: (b - a)*t + a
    // calculate t first to see if we're on the line segment
    float t = dot(normalize(b - a), p - a) / distance(a, b);
    if (t > 1.0) {
        return distance(p, b);
    }
    if (t < 0.0) {
        return distance(p, a);
    }
    vec2 intersection = (b - a) * t + a;
    return distance(intersection, p);
}

vec4 draw_line(vec2 start, vec2 end, vec2 point, float width) {
    float d = minimum_distance(start, end, point);
    float factor = exp(-pow(d / width * 4.0, 4.0));
    return vec4(intensity * factor);

    //    float d = minimum_distance(start, end, point);
    //    if (d <= width / 2.0) {
    //        float factor = 1.0;
    //        if (d > width / 2.0 - 1.0) {
    //            // between 0 and 1 depending on distance from line segment
    //            factor = width / 2.0 - d;
    //        }
    //        return vec4(intensity * factor);
    //    } else {
    //        return vec4(0, 0, 0, 0);
    //    }
}

void main() {
    color = draw_line(screenStart, screenEnd, vec2(gl_FragCoord.x, gl_FragCoord.y), width);
}
