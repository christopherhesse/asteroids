precision highp float;

in highp vec2 uv;

uniform sampler2D original;
uniform sampler2D bloom;

out lowp vec4 color;

void main() {
    //    vec3 pixel = texture(original, uv).xyz;
    //    if (pixel.x > 0.5) {
    //        color = vec4((pixel - 0.5) * 2.0, 1.0);
    //    } else {
    //        color = vec4(0.0, 0.0, 0.0, 1.0);
    //    }

    //    color = clamp(texture(original, uv), 0.0, 0.5) + texture(bloom, uv) * 10.0;
    // original is 0-1
    //    color = clamp(texture(original, uv), 0.0, 0.5);
    //    color = texture(original, uv);
    //    color = texture(original, uv) + clamp(texture(bloom, uv) * 10.0, 0.0, 1.0);

    vec4 bloom = clamp(texture(bloom, uv) * 10.0, 0.0, 1.0);
    bloom *= vec4(0.5, 0.5, 1.0, 1.0);
    color = texture(original, uv) + bloom;
    //    color = texture(original, uv);
    //    vec4 pixel = texture(original, uv);
    //    if (pixel.x > 0.9) {
    //        color = vec4(1, 0, 0, 1);
    //        //        color = vec4((pixel - 0.5) * 2.0, 1.0);
    //    } else {
    //        color = pixel;
    //        //        color = vec4(0.0, 0.0, 0.0, 1.0);
    //    }

    //    if (color.r > 0.5) {
    //        color = vec4(1, 0, 0, 1);
    //    }

    //    color = texture(original, uv);
    //                color = texture(original, uv);
    //    vec4 pixel = texture(original, uv);
    //    if (pixel.x > 0.9) {
    //        color = pixel;
    //    } else {
    //        color = vec4(0.0, 0.0, 0.0, 1.0);
    //    }
    //    color = texture(bloom, uv);
}