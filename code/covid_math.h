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

inline v4
make_v4(float x, float y, float z, float w) {
    v4 result;
    
    result.x = x;
    result.y = y;
    result.z = z;
    result.w = w;
    
    return result;
}
