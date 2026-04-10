#pragma once
// Minimal subset of GLM used by the engine when system GLM isn't available.
// This is a lightweight shim (vec2, vec3, quat, mat4, lookAt, angleAxis, normalize, value_ptr).

#include <cmath>

namespace glm {

struct vec2 {
    float x, y;
    vec2() : x(0), y(0) {}
    vec2(float V) : x(V), y(V) {}
    vec2(float X, float Y) : x(X), y(Y) {}
    vec2 operator+(const vec2& o) const { return vec2(x+o.x, y+o.y); }
    vec2 operator-(const vec2& o) const { return vec2(x-o.x, y-o.y); }
    vec2 operator*(float s) const { return vec2(x*s, y*s); }
    vec2 operator/(float s) const { return vec2(x/s, y/s); }
    vec2& operator+=(const vec2& o) { x += o.x; y += o.y; return *this; }
    vec2& operator-=(const vec2& o) { x -= o.x; y -= o.y; return *this; }
};

inline vec2 operator*(float s, const vec2& v) { return vec2(v.x*s, v.y*s); }
inline float dot(const vec2& a, const vec2& b) { return a.x*b.x + a.y*b.y; }
inline float length(const vec2& v) { return std::sqrt(dot(v,v)); }
inline vec2 normalize(const vec2& v) { float l = length(v); return (l==0.0f) ? v : v / l; }

struct vec3 {
    float x, y, z;
    vec3() : x(0), y(0), z(0) {}
    vec3(float V) : x(V), y(V), z(V) {}
    vec3(float X, float Y, float Z) : x(X), y(Y), z(Z) {}
    vec3 operator+(const vec3& o) const { return vec3(x+o.x, y+o.y, z+o.z); }
    vec3 operator-(const vec3& o) const { return vec3(x-o.x, y-o.y, z-o.z); }
    vec3 operator*(float s) const { return vec3(x*s, y*s, z*s); }
    vec3 operator/(float s) const { return vec3(x/s, y/s, z/s); }
    vec3& operator+=(const vec3& o) { x += o.x; y += o.y; z += o.z; return *this; }
    vec3& operator-=(const vec3& o) { x -= o.x; y -= o.y; z -= o.z; return *this; }
};

inline vec3 operator*(float s, const vec3& v) { return vec3(v.x*s, v.y*s, v.z*s); }

inline float dot(const vec3& a, const vec3& b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
inline vec3 cross(const vec3& a, const vec3& b) { return vec3(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x); }
inline float length(const vec3& v) { return std::sqrt(dot(v,v)); }
inline vec3 normalize(const vec3& v) { float l = length(v); return (l==0.0f) ? v : v / l; }

struct quat {
    float w, x, y, z;
    quat() : w(1), x(0), y(0), z(0) {}
    quat(float W, float X, float Y, float Z) : w(W), x(X), y(Y), z(Z) {}
};

inline quat normalize(const quat& q) {
    float m = std::sqrt(q.w*q.w + q.x*q.x + q.y*q.y + q.z*q.z);
    if (m == 0.0f) return q;
    return quat(q.w/m, q.x/m, q.y/m, q.z/m);
}

inline quat angleAxis(float angle, const vec3& axis) {
    float half = angle * 0.5f;
    float s = std::sin(half);
    vec3 n = normalize(axis);
    return quat(std::cos(half), n.x * s, n.y * s, n.z * s);
}

inline quat operator*(const quat& a, const quat& b) {
    return quat(
        a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z,
        a.w*b.x + a.x*b.w + a.y*b.z - a.z*b.y,
        a.w*b.y - a.x*b.z + a.y*b.w + a.z*b.x,
        a.w*b.z + a.x*b.y - a.y*b.x + a.z*b.w
    );
}

inline quat conj(const quat& q) { return quat(q.w, -q.x, -q.y, -q.z); }

inline vec3 operator*(const quat& q, const vec3& v) {
    // v' = q * (0, v) * q^-1
    quat qv(0.0f, v.x, v.y, v.z);
    quat r = q * qv * conj(q);
    return vec3(r.x, r.y, r.z);
}

struct mat4 { 
    float data[16]; 
    mat4() { for (int i=0;i<16;++i) data[i]=0.0f; data[0]=data[5]=data[10]=data[15]=1.0f; } 
    // Identity matrix scaled by s
    mat4(float s) { for (int i=0;i<16;++i) data[i]=0.0f; data[0]=data[5]=data[10]=s; data[15]=1.0f; }
};

inline mat4 lookAt(const vec3& eye, const vec3& center, const vec3& up) {
    vec3 f = normalize(vec3(center.x - eye.x, center.y - eye.y, center.z - eye.z));
    vec3 s = normalize(cross(f, up));
    vec3 u = cross(s, f);
    mat4 M;
    // column-major
    M.data[0] = s.x; M.data[1] = u.x; M.data[2] = -f.x; M.data[3] = 0.0f;
    M.data[4] = s.y; M.data[5] = u.y; M.data[6] = -f.y; M.data[7] = 0.0f;
    M.data[8] = s.z; M.data[9] = u.z; M.data[10] = -f.z; M.data[11] = 0.0f;
    M.data[12] = -dot(s, eye); M.data[13] = -dot(u, eye); M.data[14] = dot(f, eye); M.data[15] = 1.0f;
    return M;
}

inline const float* value_ptr(const mat4& m) { return m.data; }

// Matrix multiplication: M1 * M2 (column-major)
inline mat4 operator*(const mat4& a, const mat4& b) {
    mat4 result;
    for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 4; ++row) {
            result.data[col*4 + row] = 
                a.data[row] * b.data[col*4] +
                a.data[row+4] * b.data[col*4+1] +
                a.data[row+8] * b.data[col*4+2] +
                a.data[row+12] * b.data[col*4+3];
        }
    }
    return result;
}

// Translation matrix
inline mat4 translate(const mat4& m, const vec3& v) {
    mat4 result = m;
    result.data[12] += v.x;
    result.data[13] += v.y;
    result.data[14] += v.z;
    return result;
}

// Rotation matrix around axis by angle (radians)
inline mat4 rotate(const mat4& m, float angle, const vec3& axis) {
    vec3 ax = normalize(axis);
    float c = std::cos(angle);
    float s = std::sin(angle);
    float t = 1.0f - c;
    
    mat4 rot;
    rot.data[0] = ax.x*ax.x*t + c;       rot.data[4] = ax.x*ax.y*t - ax.z*s;   rot.data[8] = ax.x*ax.z*t + ax.y*s;   rot.data[12] = 0.0f;
    rot.data[1] = ax.x*ax.y*t + ax.z*s;  rot.data[5] = ax.y*ax.y*t + c;        rot.data[9] = ax.y*ax.z*t - ax.x*s;   rot.data[13] = 0.0f;
    rot.data[2] = ax.x*ax.z*t - ax.y*s;  rot.data[6] = ax.y*ax.z*t + ax.x*s;   rot.data[10] = ax.z*ax.z*t + c;       rot.data[14] = 0.0f;
    rot.data[3] = 0.0f;                   rot.data[7] = 0.0f;                    rot.data[11] = 0.0f;                   rot.data[15] = 1.0f;
    
    return m * rot;
}

// Scale matrix
inline mat4 scale(const mat4& m, const vec3& v) {
    mat4 result = m;
    result.data[0] *= v.x;
    result.data[5] *= v.y;
    result.data[10] *= v.z;
    return result;
}

// Perspective projection matrix
inline mat4 perspective(float fovy, float aspect, float near, float far) {
    float f = 1.0f / std::tan(fovy * 0.5f);
    mat4 result;
    result.data[0] = f / aspect;
    result.data[5] = f;
    result.data[10] = (far + near) / (near - far);
    result.data[11] = -1.0f;
    result.data[14] = (2.0f * far * near) / (near - far);
    result.data[15] = 0.0f;
    return result;
}

// Convert degrees to radians
inline float radians(float degrees) {
    return degrees * (3.141592653589793f / 180.0f);
}

} // namespace glm
