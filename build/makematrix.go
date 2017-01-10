package main

import (
	"fmt"
	"log"
	"os"
	"strings"
)

func main() {
	outpath := os.Args[1]

	out, err := os.Create(outpath)
	if err != nil {
		log.Fatal(err)
	}
	defer out.Close()

	fmt.Fprintln(out, `
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include "matrix.h"

#if INTERFACE
typedef union {
    struct {
        float32 x;
        float32 y;
    };
    float32 m[2];
} V2;

typedef union {
    struct {
        float32 x;
        float32 y;
        float32 z;
    };
    float32 m[3];
} V3;

typedef union {
    struct {
        float32 x;
        float32 y;
        float32 z;
        float32 w;
    };
    float32 m[4];
} V4;

typedef struct {
    float32 m[9];
} M3;

typedef struct {
    float32 m[16];
} M4;

typedef union {
    struct {
        float32 x;
        float32 y;
        float32 z;
        float32 w;
    };
    float32 m[4];
} Q4;
#endif

float32 DegreesToRadians(float32 degrees) {
	return (degrees / 180.0f) * M_PI;
}
`)

	components := []string{"x", "y", "z", "w"}

	for i := 2; i < 5; i++ {
		t := fmt.Sprintf("V%d", i)

		makeFunc := func(template string, args ...interface{}) {
			template = strings.Replace(template, "%t", t, -1)
			out.Write([]byte(fmt.Sprintf(template, args...)))
		}

		makeEq := func(template string, sep string) string {
			parts := []string{}
			for _, c := range components[:i] {
				part := ""
				if strings.Count(template, "%") == 1 {
					part = fmt.Sprintf(template, c)
				} else {
					part = fmt.Sprintf(template, c, c)
				}
				parts = append(parts, part)
			}
			return strings.Join(parts, " "+sep+" ")
		}

		makeFunc(`
float32 %tLength(%t v) {
    return sqrtf(%s);
}
`, makeEq("v.%s * v.%s", " + "))

		makeFunc(`
%t %tNegate(%t v) {
    return (%t){%s};
}
`, makeEq("-v.%s", ","))

		makeFunc(`
%t %tNormalize(%t v) {
    float32 len = sqrtf(%s);
    return (%t){%s};
}
`, makeEq("v.%s * v.%s", " + "), makeEq("v.%s/len", ","))

		makeFunc(`
%t %tAddScalar(%t v, float32 s) {
    return (%t){%s};
}
`, makeEq("v.%s + s", ","))

		makeFunc(`
%t %tSubtractScalar(%t v, float32 s) {
    return (%t){%s};
}
`, makeEq("v.%s - s", ","))

		makeFunc(`
%t %tMultiplyScalar(%t v, float32 s) {
    return (%t){%s};
}
`, makeEq("v.%s * s", ","))

		makeFunc(`
%t %tDivideScalar(%t v, float32 s) {
    return (%t){%s};
}
`, makeEq("v.%s / s", ","))

		makeFunc(`
float32 %tDistance(%t vl, %t vr) {
    %t v = {%s};
    return sqrtf(%s);
}
`, makeEq("vl.%s - vr.%s", ","), makeEq("v.%s * v.%s", " + "))

		makeFunc(`
%t %tAdd(%t vl, %t vr) {
    return (%t){%s};
}
`, makeEq("vl.%s + vr.%s", ","))

		makeFunc(`
%t %tSubtract(%t vl, %t vr) {
    return (%t){%s};
}
`, makeEq("vl.%s - vr.%s", ","))

		makeFunc(`
%t %tMultiply(%t vl, %t vr) {
    return (%t){%s};
}
`, makeEq("vl.%s * vr.%s", ","))

		makeFunc(`
%t %tDivide(%t vl, %t vr) {
    return (%t){%s};
}
`, makeEq("vl.%s / vr.%s", ","))

		makeFunc(`
float32 %tDotProduct(%t vl, %t vr) {
    return %s;
}
`, makeEq("vl.%s * vr.%s", "+"))

		makeFunc(`
%t %tLerp(%t start, %t end, float32 t) {
    return (%t){%s};
}
`, makeEq("start.%s * (1.0 - t) + end.%s * t", ","))

		makeFunc(`
%t %tProject(%t vl, %t vr) {
    %t unit = %tNormalize(vr);
    float32 magnitude = %tDotProduct(vl, unit);
    return %tMultiplyScalar(unit, magnitude);
}
`)

		makeFunc(`
%t %tMaximum(%t vl, %t vr) {
    return (%t){%s};
}
`, makeEq("(vl.%s > vr.%s) ? vl.x : vr.x", ","))

		makeFunc(`
%t %tMinimum(%t vl, %t vr) {
    return (%t){%s};
}
`, makeEq("(vl.%s < vr.%s) ? vl.x : vr.x", ","))
	}

	fmt.Fprint(out, `
const M3 M3Identity = {
  1, 0, 0,
  0, 1, 0,
  0, 0, 1,
};

M3 M3Inverse(M3 m, bool* success) {
	M3 tmp = {
		m.m[4]*m.m[8] - m.m[7]*m.m[5],
		m.m[2]*m.m[7] - m.m[8]*m.m[1],
		m.m[1]*m.m[5] - m.m[4]*m.m[2],
		m.m[5]*m.m[6] - m.m[8]*m.m[3],
		m.m[0]*m.m[8] - m.m[6]*m.m[2],
		m.m[2]*m.m[3] - m.m[5]*m.m[0],
		m.m[3]*m.m[7] - m.m[6]*m.m[4],
		m.m[1]*m.m[6] - m.m[7]*m.m[0],
		m.m[0]*m.m[4] - m.m[3]*m.m[1],
	};

	float32 det = m.m[0]*tmp.m[0] + m.m[1]*tmp.m[3] + m.m[2]*tmp.m[6];

	if (det == 0) {
		if (success != NULL) {
    	*success = false;
		}
		return M3Identity;
	}

	for (int i = 0; i < 9; i++) {
		tmp.m[i] = tmp.m[i] * 1.0 / det;
	}

	if (success != NULL) {
  	*success = true;
	}
	return tmp;
}

M3 M3Transpose(M3 m) {
	return (M3){
		m.m[0], m.m[3], m.m[6],
		m.m[1], m.m[4], m.m[7],
		m.m[2], m.m[5], m.m[8],
	};
}

M3 M3InverseTranspose(M3 m, bool* s) {
  bool success;
	M3 inverse = M3Inverse(m, &success);
	if (s != NULL) {
		*s = success;
	}

	if (!success) {
		return M3Identity;
	}
	return M3Transpose(inverse);
}

M3 M3Translate(M3 m, V2 v) {
	return M3Multiply(m, (M3){
		1, 0, 0,
		0, 1, 0,
		v.x, v.y, 1,
	});
}

M3 M3Rotate(M3 m, float32 radians) {
	return M3Multiply(m, (M3){
		cosf(radians), sinf(radians), 0,
		-sinf(radians), cosf(radians), 0,
		0, 0, 1,
	});
}

M3 M3Scale(M3 m, V2 v) {
	return M3Multiply(m,
		(M3){
			v.x, 0, 0,
			0, v.y, 0,
			0, 0, 1,
		});
}

M3 M3Multiply(M3 ml, M3 mr) {
	return (M3){
		ml.m[0]*mr.m[0] + ml.m[3]*mr.m[1] + ml.m[6]*mr.m[2],
		ml.m[1]*mr.m[0] + ml.m[4]*mr.m[1] + ml.m[7]*mr.m[2],
		ml.m[2]*mr.m[0] + ml.m[5]*mr.m[1] + ml.m[8]*mr.m[2],
		ml.m[0]*mr.m[3] + ml.m[3]*mr.m[4] + ml.m[6]*mr.m[5],
		ml.m[1]*mr.m[3] + ml.m[4]*mr.m[4] + ml.m[7]*mr.m[5],
		ml.m[2]*mr.m[3] + ml.m[5]*mr.m[4] + ml.m[8]*mr.m[5],
		ml.m[0]*mr.m[6] + ml.m[3]*mr.m[7] + ml.m[6]*mr.m[8],
		ml.m[1]*mr.m[6] + ml.m[4]*mr.m[7] + ml.m[7]*mr.m[8],
		ml.m[2]*mr.m[6] + ml.m[5]*mr.m[7] + ml.m[8]*mr.m[8],
	};
}

M3 M3Add(M3 ml, M3 mr) {
	return (M3){
		ml.m[0] + mr.m[0],
		ml.m[1] + mr.m[1],
		ml.m[2] + mr.m[2],
		ml.m[3] + mr.m[3],
		ml.m[4] + mr.m[4],
		ml.m[5] + mr.m[5],
		ml.m[6] + mr.m[6],
		ml.m[7] + mr.m[7],
		ml.m[8] + mr.m[8],
	};
}

M3 M3Subtract(M3 ml, M3 mr) {
	return (M3){
		ml.m[0] - mr.m[0],
		ml.m[1] - mr.m[1],
		ml.m[2] - mr.m[2],
		ml.m[3] - mr.m[3],
		ml.m[4] - mr.m[4],
		ml.m[5] - mr.m[5],
		ml.m[6] - mr.m[6],
		ml.m[7] - mr.m[7],
		ml.m[8] - mr.m[8],
	};
}

V2 M3MultiplyV2(M3 ml, V2 vr) {
	return (V2){
		vr.m[0]*ml.m[0] + vr.m[1]*ml.m[3] + 1*ml.m[6],
		vr.m[0]*ml.m[1] + vr.m[1]*ml.m[4] + 1*ml.m[7],
	};
}

V3 M3MultiplyV3(M3 ml, V3 vr) {
	return (V3){
		vr.m[0]*ml.m[0] + vr.m[1]*ml.m[3] + 1*ml.m[6],
		vr.m[0]*ml.m[1] + vr.m[1]*ml.m[4] + 1*ml.m[7],
		vr.m[0]*ml.m[2] + vr.m[1]*ml.m[5] + 1*ml.m[8],
	};
}

M3 M3FromM4(M4 m) {
	return (M3){
		m.m[0],
		m.m[1],
		m.m[2],

		m.m[4],
		m.m[5],
		m.m[6],

		m.m[8],
		m.m[9],
		m.m[10],
	};
}

const M4 M4Identity ={
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1,
};

M4 M4Inverse(M4 m, bool* success) {
	// http://stackoverflow.com/questions/1148309/inverting-a-4x4-M4
	M4 tmp = {};

	tmp.m[0] = m.m[5]*m.m[10]*m.m[15] -
		m.m[5]*m.m[11]*m.m[14] -
		m.m[9]*m.m[6]*m.m[15] +
		m.m[9]*m.m[7]*m.m[14] +
		m.m[13]*m.m[6]*m.m[11] -
		m.m[13]*m.m[7]*m.m[10];

	tmp.m[4] = -m.m[4]*m.m[10]*m.m[15] +
		m.m[4]*m.m[11]*m.m[14] +
		m.m[8]*m.m[6]*m.m[15] -
		m.m[8]*m.m[7]*m.m[14] -
		m.m[12]*m.m[6]*m.m[11] +
		m.m[12]*m.m[7]*m.m[10];

	tmp.m[8] = m.m[4]*m.m[9]*m.m[15] -
		m.m[4]*m.m[11]*m.m[13] -
		m.m[8]*m.m[5]*m.m[15] +
		m.m[8]*m.m[7]*m.m[13] +
		m.m[12]*m.m[5]*m.m[11] -
		m.m[12]*m.m[7]*m.m[9];

	tmp.m[12] = -m.m[4]*m.m[9]*m.m[14] +
		m.m[4]*m.m[10]*m.m[13] +
		m.m[8]*m.m[5]*m.m[14] -
		m.m[8]*m.m[6]*m.m[13] -
		m.m[12]*m.m[5]*m.m[10] +
		m.m[12]*m.m[6]*m.m[9];

	tmp.m[1] = -m.m[1]*m.m[10]*m.m[15] +
		m.m[1]*m.m[11]*m.m[14] +
		m.m[9]*m.m[2]*m.m[15] -
		m.m[9]*m.m[3]*m.m[14] -
		m.m[13]*m.m[2]*m.m[11] +
		m.m[13]*m.m[3]*m.m[10];

	tmp.m[5] = m.m[0]*m.m[10]*m.m[15] -
		m.m[0]*m.m[11]*m.m[14] -
		m.m[8]*m.m[2]*m.m[15] +
		m.m[8]*m.m[3]*m.m[14] +
		m.m[12]*m.m[2]*m.m[11] -
		m.m[12]*m.m[3]*m.m[10];

	tmp.m[9] = -m.m[0]*m.m[9]*m.m[15] +
		m.m[0]*m.m[11]*m.m[13] +
		m.m[8]*m.m[1]*m.m[15] -
		m.m[8]*m.m[3]*m.m[13] -
		m.m[12]*m.m[1]*m.m[11] +
		m.m[12]*m.m[3]*m.m[9];

	tmp.m[13] = m.m[0]*m.m[9]*m.m[14] -
		m.m[0]*m.m[10]*m.m[13] -
		m.m[8]*m.m[1]*m.m[14] +
		m.m[8]*m.m[2]*m.m[13] +
		m.m[12]*m.m[1]*m.m[10] -
		m.m[12]*m.m[2]*m.m[9];

	tmp.m[2] = m.m[1]*m.m[6]*m.m[15] -
		m.m[1]*m.m[7]*m.m[14] -
		m.m[5]*m.m[2]*m.m[15] +
		m.m[5]*m.m[3]*m.m[14] +
		m.m[13]*m.m[2]*m.m[7] -
		m.m[13]*m.m[3]*m.m[6];

	tmp.m[6] = -m.m[0]*m.m[6]*m.m[15] +
		m.m[0]*m.m[7]*m.m[14] +
		m.m[4]*m.m[2]*m.m[15] -
		m.m[4]*m.m[3]*m.m[14] -
		m.m[12]*m.m[2]*m.m[7] +
		m.m[12]*m.m[3]*m.m[6];

	tmp.m[10] = m.m[0]*m.m[5]*m.m[15] -
		m.m[0]*m.m[7]*m.m[13] -
		m.m[4]*m.m[1]*m.m[15] +
		m.m[4]*m.m[3]*m.m[13] +
		m.m[12]*m.m[1]*m.m[7] -
		m.m[12]*m.m[3]*m.m[5];

	tmp.m[14] = -m.m[0]*m.m[5]*m.m[14] +
		m.m[0]*m.m[6]*m.m[13] +
		m.m[4]*m.m[1]*m.m[14] -
		m.m[4]*m.m[2]*m.m[13] -
		m.m[12]*m.m[1]*m.m[6] +
		m.m[12]*m.m[2]*m.m[5];

	tmp.m[3] = -m.m[1]*m.m[6]*m.m[11] +
		m.m[1]*m.m[7]*m.m[10] +
		m.m[5]*m.m[2]*m.m[11] -
		m.m[5]*m.m[3]*m.m[10] -
		m.m[9]*m.m[2]*m.m[7] +
		m.m[9]*m.m[3]*m.m[6];

	tmp.m[7] = m.m[0]*m.m[6]*m.m[11] -
		m.m[0]*m.m[7]*m.m[10] -
		m.m[4]*m.m[2]*m.m[11] +
		m.m[4]*m.m[3]*m.m[10] +
		m.m[8]*m.m[2]*m.m[7] -
		m.m[8]*m.m[3]*m.m[6];

	tmp.m[11] = -m.m[0]*m.m[5]*m.m[11] +
		m.m[0]*m.m[7]*m.m[9] +
		m.m[4]*m.m[1]*m.m[11] -
		m.m[4]*m.m[3]*m.m[9] -
		m.m[8]*m.m[1]*m.m[7] +
		m.m[8]*m.m[3]*m.m[5];

	tmp.m[15] = m.m[0]*m.m[5]*m.m[10] -
		m.m[0]*m.m[6]*m.m[9] -
		m.m[4]*m.m[1]*m.m[10] +
		m.m[4]*m.m[2]*m.m[9] +
		m.m[8]*m.m[1]*m.m[6] -
		m.m[8]*m.m[2]*m.m[5];

	float32 det = m.m[0]*tmp.m[0] + m.m[1]*tmp.m[4] + m.m[2]*tmp.m[8] + m.m[3]*tmp.m[12];

	if (success != NULL) {
		*success = (det == 0);
	}

	if (det == 0) {
    return M4Identity;
	}

	for (int i = 0; i < 16; i++) {
		tmp.m[i] = tmp.m[i] * 1.0 / det;
	}
	return tmp;
}

M4 M4Transpose(M4 m) {
	return (M4){
		m.m[0], m.m[4], m.m[8], m.m[12],
		m.m[1], m.m[5], m.m[9], m.m[13],
		m.m[2], m.m[6], m.m[10], m.m[14],
		m.m[3], m.m[7], m.m[11], m.m[15],
	};
}

M4 M4InverseTranspose(M4 m, bool* s) {
  bool success;
  M4 inverse = M4Inverse(m, &success);
	if (s != NULL) {
  	*s = false;
	}
	if (!success) {
		return M4Identity;
	}
	return M4Transpose(inverse);
}

M4 M4OrthographicProjection(float32 left, float32 right, float32 bottom, float32 top, float32 near, float32 far) {
  M4 m = M4Scale(M4Identity, (V3){2 / (right - left), 2 / (top - bottom), -2 / (far - near)});
  return M4Translate(m, (V3){-(right + left) / 2, -(top + bottom) / 2, (far + near) / 2});
}

M4 M4PerspectiveProjection(float32 fovRadians, float32 aspect, float32 near, float32 far) {
  float32 scale = 1.0 / tanf(fovRadians/2.0);
	return (M4){
		scale / aspect, 0, 0, 0,
		0, scale, 0, 0,
		0, 0, -(far + near) / (far - near), -1,
		0, 0, -2 * near * far / (far - near), 0,
	};
}

M4 M4Multiply(M4 ml, M4 mr) {
	return (M4){
		mr.m[0]*ml.m[0] + mr.m[1]*ml.m[4] + mr.m[2]*ml.m[8] + mr.m[3]*ml.m[12],
		mr.m[0]*ml.m[1] + mr.m[1]*ml.m[5] + mr.m[2]*ml.m[9] + mr.m[3]*ml.m[13],
		mr.m[0]*ml.m[2] + mr.m[1]*ml.m[6] + mr.m[2]*ml.m[10] + mr.m[3]*ml.m[14],
		mr.m[0]*ml.m[3] + mr.m[1]*ml.m[7] + mr.m[2]*ml.m[11] + mr.m[3]*ml.m[15],
		mr.m[4]*ml.m[0] + mr.m[5]*ml.m[4] + mr.m[6]*ml.m[8] + mr.m[7]*ml.m[12],
		mr.m[4]*ml.m[1] + mr.m[5]*ml.m[5] + mr.m[6]*ml.m[9] + mr.m[7]*ml.m[13],
		mr.m[4]*ml.m[2] + mr.m[5]*ml.m[6] + mr.m[6]*ml.m[10] + mr.m[7]*ml.m[14],
		mr.m[4]*ml.m[3] + mr.m[5]*ml.m[7] + mr.m[6]*ml.m[11] + mr.m[7]*ml.m[15],
		mr.m[8]*ml.m[0] + mr.m[9]*ml.m[4] + mr.m[10]*ml.m[8] + mr.m[11]*ml.m[12],
		mr.m[8]*ml.m[1] + mr.m[9]*ml.m[5] + mr.m[10]*ml.m[9] + mr.m[11]*ml.m[13],
		mr.m[8]*ml.m[2] + mr.m[9]*ml.m[6] + mr.m[10]*ml.m[10] + mr.m[11]*ml.m[14],
		mr.m[8]*ml.m[3] + mr.m[9]*ml.m[7] + mr.m[10]*ml.m[11] + mr.m[11]*ml.m[15],
		mr.m[12]*ml.m[0] + mr.m[13]*ml.m[4] + mr.m[14]*ml.m[8] + mr.m[15]*ml.m[12],
		mr.m[12]*ml.m[1] + mr.m[13]*ml.m[5] + mr.m[14]*ml.m[9] + mr.m[15]*ml.m[13],
		mr.m[12]*ml.m[2] + mr.m[13]*ml.m[6] + mr.m[14]*ml.m[10] + mr.m[15]*ml.m[14],
		mr.m[12]*ml.m[3] + mr.m[13]*ml.m[7] + mr.m[14]*ml.m[11] + mr.m[15]*ml.m[15],
	};
}

M4 M4Translate(M4 m, V3 v) {
	return M4Multiply(m, (M4){
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
    v.x, v.y, v.z, 1,
	});
}

M4 M4Rotate(M4 m, float32 radians, V3 axis) {
	axis = V3Normalize(axis);
	float32 x = axis.x;
	float32 y = axis.y;
	float32 z = axis.z;

  float32 c = cosf(radians);
  float32 s = sinf(radians);

	return M4Multiply(m, (M4){
    c + x*x*(1-c),
		 y*x*(1-c) + z*s,
		 z*x*(1-c) - y*s,
		 0,
    x*y*(1-c) - z*s,
		 c + y*y*(1-c),
		 z*y*(1-c) + x*s,
		 0,
    x*z*(1-c) + y*s,
		 y*z*(1-c) - x*s,
		 c + z*z*(1-c),
		 0,
		0,
		0,
		0,
		 1,
	});
}

M4 M4RotateX(M4 m, float32 radians) {
	return M4Rotate(m, radians, (V3){1, 0, 0});
}

M4 M4RotateY(M4 m, float32 radians) {
	return M4Rotate(m, radians, (V3){0, 1, 0});
}

M4 M4RotateZ(M4 m, float32 radians) {
	return M4Rotate(m, radians, (V3){0, 0, 1});
}

M4 M4Scale(M4 m, V3 v) {
	return M4Multiply(m,
		(M4){
			v.x, 0, 0, 0,
			0, v.y, 0, 0,
			0, 0, v.z, 0,
			0, 0, 0, 1,
		});
}

M4 M4Add(M4 ml, M4 mr) {
	return (M4){
		ml.m[0] + mr.m[0],
		ml.m[1] + mr.m[1],
		ml.m[2] + mr.m[2],
		ml.m[3] + mr.m[3],
		ml.m[4] + mr.m[4],
		ml.m[5] + mr.m[5],
		ml.m[6] + mr.m[6],
		ml.m[7] + mr.m[7],
		ml.m[8] + mr.m[8],
		ml.m[9] + mr.m[9],
		ml.m[10] + mr.m[10],
		ml.m[11] + mr.m[11],
		ml.m[12] + mr.m[12],
		ml.m[13] + mr.m[13],
		ml.m[14] + mr.m[14],
		ml.m[15] + mr.m[15],
	};
}

M4 M4Subtract(M4 ml, M4 mr) {
	return (M4){
		ml.m[0] - mr.m[0],
		ml.m[1] - mr.m[1],
		ml.m[2] - mr.m[2],
		ml.m[3] - mr.m[3],
		ml.m[4] - mr.m[4],
		ml.m[5] - mr.m[5],
		ml.m[6] - mr.m[6],
		ml.m[7] - mr.m[7],
		ml.m[8] - mr.m[8],
		ml.m[9] - mr.m[9],
		ml.m[10] - mr.m[10],
		ml.m[11] - mr.m[11],
		ml.m[12] - mr.m[12],
		ml.m[13] - mr.m[13],
		ml.m[14] - mr.m[14],
		ml.m[15] - mr.m[15],
	};
}

V3 M4MultiplyV3(M4 m, V3 v) {
	return (V3){
		v.x*m.m[0] + v.y*m.m[4] + v.z*m.m[8] + 1*m.m[12],
		v.x*m.m[1] + v.y*m.m[5] + v.z*m.m[9] + 1*m.m[13],
		v.x*m.m[2] + v.y*m.m[6] + v.z*m.m[10] + 1*m.m[14],
	};
}

V4 M4MultiplyV4(M4 m, V4 v) {
	return (V4){
		v.x*m.m[0] + v.y*m.m[4] + v.z*m.m[8] + v.w*m.m[12],
		v.x*m.m[1] + v.y*m.m[5] + v.z*m.m[9] + v.w*m.m[13],
		v.x*m.m[2] + v.y*m.m[6] + v.z*m.m[10] + v.w*m.m[14],
		v.x*m.m[3] + v.y*m.m[7] + v.z*m.m[11] + v.w*m.m[15],
	};
}

Q4 Q4Identity = {0, 0, 0, 1};

Q4 Q4FromAngleAxis(float32 radians, V3 axis) {
	axis = V3Normalize(axis);
	float32 halfAngle = radians / 2.0;
	float32 scale = sinf(halfAngle);
	return (Q4){
		axis.x * scale,
		axis.y * scale,
		axis.z * scale,
    cosf(halfAngle),
	};
}

float32 Q4Length(Q4 q) {
  return sqrtf(q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w);
}

Q4 Q4Normalize(Q4 q) {
  float32 scale = 1.0 / sqrtf(q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w);
	return (Q4){
		q.x * scale,
		q.y * scale,
		q.z * scale,
		q.w * scale,
	};
}

Q4 Q4Conjugate(Q4 q) {
	return (Q4){
		-q.x,
		-q.y,
		-q.z,
		q.w,
	};
}

Q4 Q4Inverse(Q4 q) {
	float32 scale = 1.0 / (q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w);
	return (Q4){
		-q.x * scale,
		-q.y * scale,
		-q.z * scale,
		q.w * scale,
	};
}

float32 Q4Angle(Q4 q) {
  return acosf(q.w * 2.0);
}

V3 Q4Axis(Q4 q) {
	float32 halfAngle = acosf(q.w);
  float32 scale = 1.0 / sinf(halfAngle);
	return (V3){
		q.x * scale,
		q.y * scale,
		q.z * scale,
	};
}

Q4 Q4Multiply(Q4 ql, Q4 qr) {
	return (Q4){
		ql.w*qr.x + ql.x*qr.w + ql.y*qr.z - ql.z*qr.y,
		ql.w*qr.y - ql.x*qr.z + ql.y*qr.w + ql.z*qr.x,
		ql.w*qr.z + ql.x*qr.y - ql.y*qr.x + ql.z*qr.w,
		ql.w*qr.w - ql.x*qr.x - ql.y*qr.y - ql.z*qr.z,
	};
}

Q4 Q4Add(Q4 ql, Q4 qr) {
	return (Q4){
		ql.x + qr.x,
		ql.y + qr.y,
		ql.z + qr.z,
		ql.w + qr.w,
	};
}

Q4 Q4Subtract(Q4 ql, Q4 qr) {
	return (Q4){
		ql.x - qr.x,
		ql.y - qr.y,
		ql.z - qr.z,
		ql.w - qr.w,
	};
}

V4 Q4RotateV4(Q4 q, V4 v) {
	return (V4){
		(1-2*q.y*q.y-2*q.z*q.z)*v.x + (2*q.x*q.y-2*q.w*q.z)*v.y + (2*q.x*q.z+2*q.w*q.y)*v.z,
		(2*q.x*q.y+2*q.w*q.z)*v.x + (1-2*q.x*q.x-2*q.z*q.z)*v.y + (2*q.y*q.z-2*q.w*q.x)*v.z,
		(2*q.x*q.z-2*q.w*q.y)*v.x + (2*q.y*q.z+2*q.w*q.x)*v.y + (1-2*q.x*q.x-2*q.y*q.y)*v.z,
		v.w,
	};
}

Q4 Q4Lerp(Q4 start, Q4 end, float32 t) {
	return (Q4){
		start.x*(1.0-t) + end.x*t,
		start.y*(1.0-t) + end.y*t,
		start.z*(1.0-t) + end.z*t,
		start.w*(1.0-t) + end.w*t,
	};
}

Q4 Q4Nlerp(Q4 start, Q4 end, float32 t) {
  return Q4Normalize(Q4Lerp(start, end, t));
}

Q4 Q4Slerp(Q4 start, Q4 end, float32 t) {
  float32 angle = acosf(start.x*end.x + start.y*end.y + start.z*end.z + start.w*end.w);
  float32 startScale = sinf(angle*(1.0-t)) / sinf(angle);
  float32 endScale = sinf(angle*t) / sinf(angle);
	return (Q4){
		start.x*startScale + end.x*endScale,
		start.y*startScale + end.y*endScale,
		start.z*startScale + end.z*endScale,
		start.w*startScale + end.w*endScale,
	};
}
`)
}
