/* date = May 18th 2020 6:43 pm */

union v2 {
    struct {
        float x, y;
    };
    struct {
        float u, v;
    };
    struct {
        float width, height;
    };
    float e[2];
};

union v3 {
    struct {
        float x, y, z;
    };
    struct {
        float u, v, __;
    };
    struct {
        float r, g, b;
    };
    float e[3];
};

union v4 {
    struct {
        union {
            v3 xyz;
            struct {
                float x, y, z;
            };
        };
        float w;
    };
    struct {
        union {
            v3 rgb;
            struct {
                float r, g, b;
            };
        };
        float a;
    };
    float e[4];
};

struct mat4 {
    union {
        real32 e[4*4];
        real32 rc[4][4];
        v4 rows[4];
    };
};

inline mat4
identity() {
    mat4 result = {};
    result.e[0 + 0 * 4] = 1.0f;
	result.e[1 + 1 * 4] = 1.0f;
	result.e[2 + 2 * 4] = 1.0f;
	result.e[3 + 3 * 4] = 1.0f;
    return result;
}

inline mat4
ortho(real32 l, real32 r, real32 t, real32 b, real32 f, real32 n) {
	mat4 result = identity();
    
	result.e[0 + 0 * 4] =  2.f / (r - l);
	result.e[1 + 1 * 4] =  2.f / (t - b);
	result.e[2 + 2 * 4] = -2.f / (f - n);
    
	result.e[0 + 3 * 4] = -((r + l) / (r - l));
	result.e[1 + 3 * 4] = -((t + b) / (t - b));
	result.e[2 + 3 * 4] = -((f + n) / (f - n));
    
	return result;
}


inline mat4
ortho(real32 size, real32 aspect_ratio, real32 f, real32 n) {
    real32 right = size * aspect_ratio;
    real32 left = -right;
    
    real32 top = size;
    real32 bottom = -top;
    
    return ortho(left, right, top, bottom, f, n);
}

inline mat4
translate(v2 pos) {
    mat4 result = identity();
    
    result.e[0 + 3 * 4] = pos.x;
	result.e[1 + 3 * 4] = pos.y;
    
    return result;
}

inline v2
make_v2(float x, float y) {
    v2 result;
    
    result.x = x;
    result.y = y;
    
    return result;
}

inline v3
make_v3(float x, float y, float z) {
    v3 result;
    
    result.x = x;
    result.y = y;
    result.z = z;
    
    return result;
}

inline v4
make_v4(float x, float y, float z, float w) {
    v4 result;
    
    result.x = x;
    result.y = y;
    result.z = z;
    result.w = w;
    
    return result;
}
