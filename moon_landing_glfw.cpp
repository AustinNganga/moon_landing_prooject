// ============================================================================
//  MOON LANDING  -  "MISSION HARAMBEE-1"            (single-file edition)
//  A 5:00 procedural short film in C++ / OpenGL 3.3 core (GLFW only)
// ----------------------------------------------------------------------------
//  ONE continuous camera, no cuts.  Everything is generated in code.
//
//  STORY (seconds)
//     0 -  15   Malindi (Kenya) launch site at dawn, countdown, engines ignite
//    15 -  48   Liftoff, ascent through the sky, hot staging, booster falls away
//    48 -  90   Torch-drive transit to the Moon (flip-and-burn at the midpoint)
//    90 - 118   Powered descent, landing legs, dust storm, touchdown
//   118 - 201   Elevator EVA: Kenyan flag planted, low-gravity hops ...
//               and a sneaky alien stowaway rides the elevator back up
//   201 - 250   Lunar liftoff and the trip home (the stowaway waves from a window)
//   250 - 279   Re-entry over the Indian Ocean: plasma, belly-flop fall
//   279 - 300   Flip, landing burn, CAUGHT by the launch-tower "chopstick" arms,
//               the alien says hello, credits
//
//  Real scale: Earth 6371 km, Moon 1737 km, 384 400 km apart.  The ship is a
//  (fictional) fusion-torch vehicle pulling ~100 g, which is how it can cross
//  that distance in 40 seconds: accelerate half way, flip, decelerate.
//
//  RENDERER: camera-relative double-precision world, logarithmic depth,
//  ray-traced Earth + Moon with physically based atmospheric scattering
//  (Rayleigh + Mie), clouds, ocean glint, procedural Africa, lunar maria,
//  4x MSAA HDR, sun shadow map with PCF, bloom, ACES tone mapping.
//
//  CONTROLS  (YouTube style)
//    SPACE / K / ENTER    pause / play  (at the end: replay)
//    LEFT / RIGHT         -5 s / +5 s         J / L   -10 s / +10 s
//    UP / DOWN            playback speed 0.25x .. 4x
//    , / .                step one frame (paused)
//    HOME / END           start / credits        1..9  jump to chapter
//    MOUSE                click / drag the red progress bar
//    T timer  H help  F fullscreen  C captions on/off  R restart  ESC quit
//
//  BUILD
//    Windows / Visual Studio: add this ONE .cpp to an empty C++ project,
//      add the GLFW include + lib folders, link:  glfw3.lib opengl32.lib
//      (+ gdi32.lib user32.lib shell32.lib if your GLFW is the static build).
//      No GLAD / GLEW needed - the few GL functions are loaded below.
//      Build in **Release x64** for full speed.
//    Linux:  g++ -O2 -std=c++14 moon_landing_glfw.cpp -o moon -lglfw -ldl
//    macOS:  clang++ -O2 -std=c++14 moon_landing_glfw.cpp -o moon -lglfw
//
//  COMMAND LINE (optional)
//    --size 1920x1080          window size
//    --shadow 2048             shadow-map resolution (lower = faster)
//    --shot <sec> out.ppm      render one frame and exit
//    --pipe [fps]              write raw RGB frames to stdout for ffmpeg:
//       moon --size 1920x1080 --pipe 30 | ffmpeg -f rawvideo -pix_fmt rgb24
//            -s 1920x1080 -r 30 -i - -c:v libx264 -pix_fmt yuv420p film.mp4
// ============================================================================

#define _CRT_SECURE_NO_WARNINGS     // Visual Studio: allow fopen / sscanf
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>
#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

// ============================================================================
//  1. MINIMAL OPENGL 3.3 LOADER (via glfwGetProcAddress)
// ============================================================================
#ifdef _WIN32
#define GLCALL __stdcall
#else
#define GLCALL
#endif
typedef unsigned int GLenum, GLuint, GLbitfield;
typedef int GLint, GLsizei;
typedef float GLfloat;
typedef unsigned char GLboolean, GLubyte;
typedef char GLchar;
typedef ptrdiff_t GLsizeiptr, GLintptr;

#define GL_FALSE 0
#define GL_TRUE 1
#define GL_NONE 0
#define GL_ZERO 0
#define GL_ONE 1
#define GL_COLOR_BUFFER_BIT 0x4000
#define GL_DEPTH_BUFFER_BIT 0x100
#define GL_DEPTH_TEST 0xB71
#define GL_BLEND 0xBE2
#define GL_CULL_FACE 0xB44
#define GL_POLYGON_OFFSET_FILL 0x8037
#define GL_PROGRAM_POINT_SIZE 0x8642
#define GL_MULTISAMPLE 0x809D
#define GL_BACK 0x405
#define GL_FRONT 0x404
#define GL_SRC_ALPHA 0x302
#define GL_ONE_MINUS_SRC_ALPHA 0x303
#define GL_LEQUAL 0x203
#define GL_LESS 0x201
#define GL_ALWAYS 0x207
#define GL_POINTS 0
#define GL_TRIANGLES 4
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_STATIC_DRAW 0x88E4
#define GL_STREAM_DRAW 0x88E0
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_FLOAT 0x1406
#define GL_UNSIGNED_INT 0x1405
#define GL_UNSIGNED_BYTE 0x1401
#define GL_TEXTURE_2D 0xDE1
#define GL_TEXTURE0 0x84C0
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_LINEAR 0x2601
#define GL_NEAREST 0x2600
#define GL_LINEAR_MIPMAP_LINEAR 0x2703
#define GL_REPEAT 0x2901
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_CLAMP_TO_BORDER 0x812D
#define GL_TEXTURE_BORDER_COLOR 0x1004
#define GL_TEXTURE_COMPARE_MODE 0x884C
#define GL_TEXTURE_COMPARE_FUNC 0x884D
#define GL_COMPARE_REF_TO_TEXTURE 0x884E
#define GL_RGBA8 0x8058
#define GL_RGBA 0x1908
#define GL_RGB 0x1907
#define GL_SRGB8_ALPHA8 0x8C43
#define GL_RGBA16F 0x881A
#define GL_DEPTH_COMPONENT 0x1902
#define GL_DEPTH_COMPONENT24 0x81A6
#define GL_FRAMEBUFFER 0x8D40
#define GL_READ_FRAMEBUFFER 0x8CA8
#define GL_DRAW_FRAMEBUFFER 0x8CA9
#define GL_RENDERBUFFER 0x8D41
#define GL_COLOR_ATTACHMENT0 0x8CE0
#define GL_DEPTH_ATTACHMENT 0x8D00
#define GL_FRAMEBUFFER_COMPLETE 0x8CD5
#define GL_VERTEX_SHADER 0x8B31
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_INFO_LOG_LENGTH 0x8B84
#define GL_VERSION 0x1F02
#define GL_RENDERER 0x1F01
#define GL_UNPACK_ALIGNMENT 0xCF5
#define GL_PACK_ALIGNMENT 0xD05
#define GL_TEXTURE_MAX_ANISOTROPY 0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY 0x84FF

#define GL_FUNCS(X) \
 X(void, glClear, (GLbitfield)) \
 X(void, glClearColor, (GLfloat, GLfloat, GLfloat, GLfloat)) \
 X(void, glViewport, (GLint, GLint, GLsizei, GLsizei)) \
 X(void, glEnable, (GLenum)) \
 X(void, glDisable, (GLenum)) \
 X(void, glBlendFunc, (GLenum, GLenum)) \
 X(void, glDepthMask, (GLboolean)) \
 X(void, glDepthFunc, (GLenum)) \
 X(void, glCullFace, (GLenum)) \
 X(void, glPolygonOffset, (GLfloat, GLfloat)) \
 X(void, glReadPixels, (GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, void*)) \
 X(void, glPixelStorei, (GLenum, GLint)) \
 X(void, glGetFloatv, (GLenum, GLfloat*)) \
 X(const GLubyte*, glGetString, (GLenum)) \
 X(void, glGenTextures, (GLsizei, GLuint*)) \
 X(void, glBindTexture, (GLenum, GLuint)) \
 X(void, glTexImage2D, (GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const void*)) \
 X(void, glTexParameteri, (GLenum, GLenum, GLint)) \
 X(void, glTexParameterf, (GLenum, GLenum, GLfloat)) \
 X(void, glTexParameterfv, (GLenum, GLenum, const GLfloat*)) \
 X(void, glGenerateMipmap, (GLenum)) \
 X(void, glActiveTexture, (GLenum)) \
 X(void, glGenBuffers, (GLsizei, GLuint*)) \
 X(void, glBindBuffer, (GLenum, GLuint)) \
 X(void, glBufferData, (GLenum, GLsizeiptr, const void*, GLenum)) \
 X(void, glBufferSubData, (GLenum, GLintptr, GLsizeiptr, const void*)) \
 X(void, glGenVertexArrays, (GLsizei, GLuint*)) \
 X(void, glBindVertexArray, (GLuint)) \
 X(void, glEnableVertexAttribArray, (GLuint)) \
 X(void, glVertexAttribPointer, (GLuint, GLint, GLenum, GLboolean, GLsizei, const void*)) \
 X(void, glDrawElements, (GLenum, GLsizei, GLenum, const void*)) \
 X(void, glDrawArrays, (GLenum, GLint, GLsizei)) \
 X(GLuint, glCreateShader, (GLenum)) \
 X(void, glShaderSource, (GLuint, GLsizei, const GLchar* const*, const GLint*)) \
 X(void, glCompileShader, (GLuint)) \
 X(void, glGetShaderiv, (GLuint, GLenum, GLint*)) \
 X(void, glGetShaderInfoLog, (GLuint, GLsizei, GLsizei*, GLchar*)) \
 X(GLuint, glCreateProgram, (void)) \
 X(void, glAttachShader, (GLuint, GLuint)) \
 X(void, glLinkProgram, (GLuint)) \
 X(void, glGetProgramiv, (GLuint, GLenum, GLint*)) \
 X(void, glGetProgramInfoLog, (GLuint, GLsizei, GLsizei*, GLchar*)) \
 X(void, glUseProgram, (GLuint)) \
 X(void, glDeleteShader, (GLuint)) \
 X(GLint, glGetUniformLocation, (GLuint, const GLchar*)) \
 X(void, glUniform1i, (GLint, GLint)) \
 X(void, glUniform1f, (GLint, GLfloat)) \
 X(void, glUniform2f, (GLint, GLfloat, GLfloat)) \
 X(void, glUniform3f, (GLint, GLfloat, GLfloat, GLfloat)) \
 X(void, glUniform4f, (GLint, GLfloat, GLfloat, GLfloat, GLfloat)) \
 X(void, glUniformMatrix3fv, (GLint, GLsizei, GLboolean, const GLfloat*)) \
 X(void, glUniformMatrix4fv, (GLint, GLsizei, GLboolean, const GLfloat*)) \
 X(void, glGenFramebuffers, (GLsizei, GLuint*)) \
 X(void, glBindFramebuffer, (GLenum, GLuint)) \
 X(void, glFramebufferTexture2D, (GLenum, GLenum, GLenum, GLuint, GLint)) \
 X(void, glFramebufferRenderbuffer, (GLenum, GLenum, GLenum, GLuint)) \
 X(void, glGenRenderbuffers, (GLsizei, GLuint*)) \
 X(void, glBindRenderbuffer, (GLenum, GLuint)) \
 X(void, glRenderbufferStorageMultisample, (GLenum, GLsizei, GLenum, GLsizei, GLsizei)) \
 X(void, glBlitFramebuffer, (GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum)) \
 X(GLenum, glCheckFramebufferStatus, (GLenum)) \
 X(void, glDrawBuffer, (GLenum)) \
 X(void, glReadBuffer, (GLenum)) \
 X(void, glDeleteTextures, (GLsizei, const GLuint*)) \
 X(void, glDeleteFramebuffers, (GLsizei, const GLuint*)) \
 X(void, glDeleteRenderbuffers, (GLsizei, const GLuint*)) \
 X(void, glFinish, (void))

#define X(ret, name, args) typedef ret (GLCALL *PFN_##name) args; static PFN_##name name = nullptr;
GL_FUNCS(X)
#undef X

static bool loadGL() {
    bool ok = true;
#define X(ret, fname, args) \
    fname = (PFN_##fname)glfwGetProcAddress(#fname); \
    if (!fname) { std::fprintf(stderr, "missing GL function: %s\n", #fname); ok = false; }
    GL_FUNCS(X)
#undef X
        return ok;
}

// ============================================================================
//  2. MATH  (float vectors for rendering, double vectors for the solar system)
// ============================================================================
static const float PI = 3.14159265358979f;
static inline float clampf(float x, float a = 0.f, float b = 1.f) { return x < a ? a : (x > b ? b : x); }
static inline float lerpf(float a, float b, float t) { return a + (b - a) * t; }
static inline float sstep(float a, float b, float x) { float t = clampf((x - a) / (b - a)); return t * t * (3.f - 2.f * t); }
static inline float fractf(float x) { return x - std::floor(x); }

struct vec3 {
    float x, y, z;
    vec3() : x(0), y(0), z(0) {}
    vec3(float a, float b, float c) : x(a), y(b), z(c) {}
};
static inline vec3 operator+(vec3 a, vec3 b) { return vec3(a.x + b.x, a.y + b.y, a.z + b.z); }
static inline vec3 operator-(vec3 a, vec3 b) { return vec3(a.x - b.x, a.y - b.y, a.z - b.z); }
static inline vec3 operator*(vec3 a, float s) { return vec3(a.x * s, a.y * s, a.z * s); }
static inline vec3 operator*(float s, vec3 a) { return a * s; }
static inline vec3 operator/(vec3 a, float s) { return a * (1.f / s); }
static inline vec3 operator-(vec3 a) { return vec3(-a.x, -a.y, -a.z); }
static inline float dot(vec3 a, vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
static inline vec3 cross(vec3 a, vec3 b) { return vec3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x); }
static inline float length(vec3 a) { return std::sqrt(dot(a, a)); }
static inline vec3 normalize(vec3 a) { float l = length(a); return l > 1e-12f ? a / l : vec3(0, 1, 0); }
static inline vec3 mix(vec3 a, vec3 b, float t) { return a + (b - a) * t; }

struct dvec3 {
    double x, y, z;
    dvec3() : x(0), y(0), z(0) {}
    dvec3(double a, double b, double c) : x(a), y(b), z(c) {}
    explicit dvec3(vec3 v) : x(v.x), y(v.y), z(v.z) {}
};
static inline dvec3 operator+(dvec3 a, dvec3 b) { return dvec3(a.x + b.x, a.y + b.y, a.z + b.z); }
static inline dvec3 operator-(dvec3 a, dvec3 b) { return dvec3(a.x - b.x, a.y - b.y, a.z - b.z); }
static inline dvec3 operator*(dvec3 a, double s) { return dvec3(a.x * s, a.y * s, a.z * s); }
static inline dvec3 operator*(double s, dvec3 a) { return a * s; }
static inline dvec3 operator+(dvec3 a, vec3 b) { return dvec3(a.x + b.x, a.y + b.y, a.z + b.z); }
static inline double ddot(dvec3 a, dvec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
static inline double dlength(dvec3 a) { return std::sqrt(ddot(a, a)); }
static inline dvec3 dnormalize(dvec3 a) { double l = dlength(a); return l > 1e-300 ? a * (1.0 / l) : dvec3(0, 1, 0); }
static inline dvec3 dmix(dvec3 a, dvec3 b, double t) { return a + (b - a) * t; }
static inline vec3 tof(dvec3 a) { return vec3((float)a.x, (float)a.y, (float)a.z); }

// orthonormal frame (columns x, y, z) - used for every local coordinate system
struct Basis {
    vec3 x, y, z;
    Basis() : x(1, 0, 0), y(0, 1, 0), z(0, 0, 1) {}
    Basis(vec3 a, vec3 b, vec3 c) : x(a), y(b), z(c) {}
    vec3 operator*(vec3 v) const { return x * v.x + y * v.y + z * v.z; }        // local -> world
    vec3 toLocal(vec3 v) const { return vec3(dot(v, x), dot(v, y), dot(v, z)); }
};
// frame whose y axis is 'up' and whose z axis is as close as possible to 'zref'
static Basis basisYZ(vec3 up, vec3 zref) {
    vec3 y = normalize(up);
    vec3 z = zref - y * dot(zref, y);
    if (length(z) < 1e-4f) z = std::fabs(y.x) < 0.9f ? cross(vec3(1, 0, 0), y) : cross(vec3(0, 0, 1), y);
    z = normalize(z);
    return Basis(cross(y, z), y, z);
}
static inline dvec3 local2world(dvec3 origin, const Basis& B, dvec3 l) {
    return origin + dvec3(B.x) * l.x + dvec3(B.y) * l.y + dvec3(B.z) * l.z;
}

struct quat { float x, y, z, w; };
static quat qFromBasis(const Basis& b) {
    float m00 = b.x.x, m11 = b.y.y, m22 = b.z.z, tr = m00 + m11 + m22; quat q;
    if (tr > 0) { float s = std::sqrt(tr + 1.f) * 2; q.w = 0.25f * s; q.x = (b.y.z - b.z.y) / s; q.y = (b.z.x - b.x.z) / s; q.z = (b.x.y - b.y.x) / s; }
    else if (m00 > m11 && m00 > m22) { float s = std::sqrt(1.f + m00 - m11 - m22) * 2; q.w = (b.y.z - b.z.y) / s; q.x = 0.25f * s; q.y = (b.y.x + b.x.y) / s; q.z = (b.z.x + b.x.z) / s; }
    else if (m11 > m22) { float s = std::sqrt(1.f + m11 - m00 - m22) * 2; q.w = (b.z.x - b.x.z) / s; q.x = (b.y.x + b.x.y) / s; q.y = 0.25f * s; q.z = (b.z.y + b.y.z) / s; }
    else { float s = std::sqrt(1.f + m22 - m00 - m11) * 2; q.w = (b.x.y - b.y.x) / s; q.x = (b.z.x + b.x.z) / s; q.y = (b.z.y + b.y.z) / s; q.z = 0.25f * s; }
    return q;
}
static Basis qToBasis(quat q) {
    float n = std::sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w); q.x /= n; q.y /= n; q.z /= n; q.w /= n;
    float x = q.x, y = q.y, z = q.z, w = q.w;
    return Basis(vec3(1 - 2 * (y * y + z * z), 2 * (x * y + z * w), 2 * (x * z - y * w)),
        vec3(2 * (x * y - z * w), 1 - 2 * (x * x + z * z), 2 * (y * z + x * w)),
        vec3(2 * (x * z + y * w), 2 * (y * z - x * w), 1 - 2 * (x * x + y * y)));
}
static quat qSlerp(quat a, quat b, float t) {
    float d = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    if (d < 0) { b.x = -b.x; b.y = -b.y; b.z = -b.z; b.w = -b.w; d = -d; }
    if (d > 0.9995f) { quat r = { lerpf(a.x, b.x, t), lerpf(a.y, b.y, t), lerpf(a.z, b.z, t), lerpf(a.w, b.w, t) }; return r; }
    float th = std::acos(d), s = std::sin(th), wa = std::sin((1 - t) * th) / s, wb = std::sin(t * th) / s;
    quat r = { a.x * wa + b.x * wb, a.y * wa + b.y * wb, a.z * wa + b.z * wb, a.w * wa + b.w * wb }; return r;
}
static Basis slerpB(const Basis& a, const Basis& b, float t) { return qToBasis(qSlerp(qFromBasis(a), qFromBasis(b), t)); }
static Basis rotAxis(const Basis& b, vec3 axis, float ang) {          // rotate every axis of b about 'axis'
    axis = normalize(axis); float c = std::cos(ang), s = std::sin(ang);
    auto R = [&](vec3 v) { return v * c + cross(axis, v) * s + axis * dot(axis, v) * (1 - c); };
    return Basis(R(b.x), R(b.y), R(b.z));
}

struct mat4 { float m[16]; };   // column major
static mat4 identity() { mat4 r; std::memset(r.m, 0, sizeof r.m); r.m[0] = r.m[5] = r.m[10] = r.m[15] = 1.f; return r; }
static mat4 operator*(const mat4& a, const mat4& b) {
    mat4 r;
    for (int c = 0; c < 4; c++) for (int row = 0; row < 4; row++) {
        float s = 0; for (int k = 0; k < 4; k++) s += a.m[k * 4 + row] * b.m[c * 4 + k];
        r.m[c * 4 + row] = s;
    }
    return r;
}
static mat4 translate(vec3 t) { mat4 r = identity(); r.m[12] = t.x; r.m[13] = t.y; r.m[14] = t.z; return r; }
static mat4 scale(vec3 s) { mat4 r = identity(); r.m[0] = s.x; r.m[5] = s.y; r.m[10] = s.z; return r; }
static mat4 scale(float s) { return scale(vec3(s, s, s)); }
static mat4 rotateX(float a) { mat4 r = identity(); float c = std::cos(a), s = std::sin(a); r.m[5] = c; r.m[6] = s; r.m[9] = -s; r.m[10] = c; return r; }
static mat4 rotateY(float a) { mat4 r = identity(); float c = std::cos(a), s = std::sin(a); r.m[0] = c; r.m[2] = -s; r.m[8] = s; r.m[10] = c; return r; }
static mat4 rotateZ(float a) { mat4 r = identity(); float c = std::cos(a), s = std::sin(a); r.m[0] = c; r.m[1] = s; r.m[4] = -s; r.m[5] = c; return r; }
static mat4 fromBasis(const Basis& b, vec3 t) {
    mat4 r = identity();
    r.m[0] = b.x.x; r.m[1] = b.x.y; r.m[2] = b.x.z;
    r.m[4] = b.y.x; r.m[5] = b.y.y; r.m[6] = b.y.z;
    r.m[8] = b.z.x; r.m[9] = b.z.y; r.m[10] = b.z.z;
    r.m[12] = t.x; r.m[13] = t.y; r.m[14] = t.z; return r;
}
static mat4 perspective(float fovy, float aspect, float n, float f) {
    mat4 r; std::memset(r.m, 0, sizeof r.m); float t = 1.f / std::tan(fovy * 0.5f);
    r.m[0] = t / aspect; r.m[5] = t; r.m[10] = (f + n) / (n - f); r.m[11] = -1.f; r.m[14] = 2.f * f * n / (n - f);
    return r;
}
static mat4 ortho(float l, float r_, float b, float t, float n, float f) {
    mat4 r = identity();
    r.m[0] = 2.f / (r_ - l); r.m[5] = 2.f / (t - b); r.m[10] = -2.f / (f - n);
    r.m[12] = -(r_ + l) / (r_ - l); r.m[13] = -(t + b) / (t - b); r.m[14] = -(f + n) / (f - n);
    return r;
}
static mat4 lookDir(vec3 eye, vec3 fwd, vec3 up) {          // view matrix
    vec3 f = normalize(fwd), s = normalize(cross(f, up)), u = cross(s, f);
    mat4 r = identity();
    r.m[0] = s.x; r.m[4] = s.y; r.m[8] = s.z;
    r.m[1] = u.x; r.m[5] = u.y; r.m[9] = u.z;
    r.m[2] = -f.x; r.m[6] = -f.y; r.m[10] = -f.z;
    r.m[12] = -dot(s, eye); r.m[13] = -dot(u, eye); r.m[14] = dot(f, eye);
    return r;
}
static mat4 alignY(vec3 dir) {                                // Y axis along dir (struts, poles)
    vec3 y = normalize(dir);
    vec3 h = std::fabs(y.y) > 0.99f ? vec3(1, 0, 0) : vec3(0, 1, 0);
    vec3 x = normalize(cross(h, y)), z = cross(x, y);
    return fromBasis(Basis(x, y, z), vec3());
}
static mat4 inverse(const mat4& M) {                           // general 4x4 inverse
    const float* m = M.m; float inv[16];
    inv[0] = m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - m[9] * m[6] * m[15] + m[9] * m[7] * m[14] + m[13] * m[6] * m[11] - m[13] * m[7] * m[10];
    inv[4] = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] + m[8] * m[6] * m[15] - m[8] * m[7] * m[14] - m[12] * m[6] * m[11] + m[12] * m[7] * m[10];
    inv[8] = m[4] * m[9] * m[15] - m[4] * m[11] * m[13] - m[8] * m[5] * m[15] + m[8] * m[7] * m[13] + m[12] * m[5] * m[11] - m[12] * m[7] * m[9];
    inv[12] = -m[4] * m[9] * m[14] + m[4] * m[10] * m[13] + m[8] * m[5] * m[14] - m[8] * m[6] * m[13] - m[12] * m[5] * m[10] + m[12] * m[6] * m[9];
    inv[1] = -m[1] * m[10] * m[15] + m[1] * m[11] * m[14] + m[9] * m[2] * m[15] - m[9] * m[3] * m[14] - m[13] * m[2] * m[11] + m[13] * m[3] * m[10];
    inv[5] = m[0] * m[10] * m[15] - m[0] * m[11] * m[14] - m[8] * m[2] * m[15] + m[8] * m[3] * m[14] + m[12] * m[2] * m[11] - m[12] * m[3] * m[10];
    inv[9] = -m[0] * m[9] * m[15] + m[0] * m[11] * m[13] + m[8] * m[1] * m[15] - m[8] * m[3] * m[13] - m[12] * m[1] * m[11] + m[12] * m[3] * m[9];
    inv[13] = m[0] * m[9] * m[14] - m[0] * m[10] * m[13] - m[8] * m[1] * m[14] + m[8] * m[2] * m[13] + m[12] * m[1] * m[10] - m[12] * m[2] * m[9];
    inv[2] = m[1] * m[6] * m[15] - m[1] * m[7] * m[14] - m[5] * m[2] * m[15] + m[5] * m[3] * m[14] + m[13] * m[2] * m[7] - m[13] * m[3] * m[6];
    inv[6] = -m[0] * m[6] * m[15] + m[0] * m[7] * m[14] + m[4] * m[2] * m[15] - m[4] * m[3] * m[14] - m[12] * m[2] * m[7] + m[12] * m[3] * m[6];
    inv[10] = m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - m[4] * m[1] * m[15] + m[4] * m[3] * m[13] + m[12] * m[1] * m[7] - m[12] * m[3] * m[5];
    inv[14] = -m[0] * m[5] * m[14] + m[0] * m[6] * m[13] + m[4] * m[1] * m[14] - m[4] * m[2] * m[13] - m[12] * m[1] * m[6] + m[12] * m[2] * m[5];
    inv[3] = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11] - m[5] * m[3] * m[10] - m[9] * m[2] * m[7] + m[9] * m[3] * m[6];
    inv[7] = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11] + m[4] * m[3] * m[10] + m[8] * m[2] * m[7] - m[8] * m[3] * m[6];
    inv[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + m[4] * m[1] * m[11] - m[4] * m[3] * m[9] - m[8] * m[1] * m[7] + m[8] * m[3] * m[5];
    inv[15] = m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[4] * m[1] * m[10] + m[4] * m[2] * m[9] + m[8] * m[1] * m[6] - m[8] * m[2] * m[5];
    float det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
    mat4 r; det = std::fabs(det) > 1e-30f ? 1.f / det : 0.f;
    for (int i = 0; i < 16; i++) r.m[i] = inv[i] * det;
    return r;
}

// cubic Hermite for smooth trajectories (p0,p1 positions, m0,m1 tangents scaled by duration)
static dvec3 hermite(dvec3 p0, dvec3 m0, dvec3 p1, dvec3 m1, double u) {
    double u2 = u * u, u3 = u2 * u;
    return p0 * (2 * u3 - 3 * u2 + 1) + m0 * (u3 - 2 * u2 + u) + p1 * (-2 * u3 + 3 * u2) + m1 * (u3 - u2);
}

// ============================================================================
//  3. NOISE  (deterministic value noise / fBm, optionally periodic)
// ============================================================================
static inline float hash2i(int x, int y, int seed) {
    uint32_t h = (uint32_t)x * 374761393u + (uint32_t)y * 668265263u + (uint32_t)seed * 2147483647u;
    h = (h ^ (h >> 13)) * 1274126177u; h ^= h >> 16;
    return (h & 0xffffffu) / 16777215.0f;
}
static inline int wrapi(int a, int p) { a %= p; return a < 0 ? a + p : a; }
static float vnoise(float x, float y, int seed, int P = 0) {
    int xi = (int)std::floor(x), yi = (int)std::floor(y);
    float xf = x - xi, yf = y - yi, u = xf * xf * (3 - 2 * xf), v = yf * yf * (3 - 2 * yf);
    int x0 = xi, x1 = xi + 1, y0 = yi, y1 = yi + 1;
    if (P > 0) { x0 = wrapi(x0, P); x1 = wrapi(x1, P); y0 = wrapi(y0, P); y1 = wrapi(y1, P); }
    float a = hash2i(x0, y0, seed), b = hash2i(x1, y0, seed), c = hash2i(x0, y1, seed), d = hash2i(x1, y1, seed);
    return a + (b - a) * u + (c - a) * v + (a - b - c + d) * u * v;
}
static float fbm(float x, float y, int oct, int seed, int P = 0) {
    float s = 0, a = 0.5f, n = 0;
    for (int i = 0; i < oct; i++) { s += a * vnoise(x, y, seed + i * 17, P); n += a; x *= 2; y *= 2; if (P > 0) P *= 2; a *= 0.5f; }
    return s / n;
}
struct Rng { uint32_t s; explicit Rng(uint32_t seed) : s(seed) {} float f() { s = s * 1664525u + 1013904223u; return (s >> 8) / 16777216.0f; } float r(float a, float b) { return a + (b - a) * f(); } };

// ============================================================================
//  4. MESHES  (indexed triangles in VBO/IBO; procedural primitive builders)
// ============================================================================
struct Vertex { float p[3], n[3], uv[2]; };
struct Mesh {
    GLuint vao = 0, vbo = 0, ibo = 0; int count = 0;
    void upload(const std::vector<Vertex>& v, const std::vector<unsigned>& idx) {
        glGenVertexArrays(1, &vao); glBindVertexArray(vao);
        glGenBuffers(1, &vbo); glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(Vertex), v.data(), GL_STATIC_DRAW);
        glGenBuffers(1, &ibo); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(unsigned), idx.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(1); glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)12);
        glEnableVertexAttribArray(2); glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)24);
        glBindVertexArray(0); count = (int)idx.size();
    }
    void draw() const { glBindVertexArray(vao); glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr); }
};
static Vertex V(float x, float y, float z, float nx, float ny, float nz, float u, float v) { Vertex r = { {x, y, z}, {nx, ny, nz}, {u, v} }; return r; }

static Mesh makeSphere(int seg, int rings) {
    std::vector<Vertex> vs; std::vector<unsigned> is;
    for (int j = 0; j <= rings; j++) for (int i = 0; i <= seg; i++) {
        float u = (float)i / seg, v = (float)j / rings, th = u * 2 * PI, ph = v * PI;
        float x = std::sin(ph) * std::cos(th), y = std::cos(ph), z = std::sin(ph) * std::sin(th);
        vs.push_back(V(x, y, z, x, y, z, u, 1.f - v));
    }
    int row = seg + 1;
    for (int j = 0; j < rings; j++) for (int i = 0; i < seg; i++) {
        unsigned A = j * row + i, B = A + 1, C = A + row, D = C + 1;
        is.insert(is.end(), { A, B, C, B, D, C });
    }
    Mesh m; m.upload(vs, is); return m;
}
// surface of revolution around +Y. prof = (radius, y) pairs from bottom to top. uv.x = angle/2pi, uv.y = v of profile
static Mesh makeLathe(const std::vector<float>& prof, int seg, float v0 = 0.f, float v1 = 1.f, bool smooth = true) {
    std::vector<Vertex> vs; std::vector<unsigned> is;
    int n = (int)prof.size() / 2;
    for (int k = 0; k < n; k++) {
        float r = prof[2 * k], y = prof[2 * k + 1];
        int ka = std::max(0, k - 1), kb = std::min(n - 1, k + 1);
        float dr = prof[2 * kb] - prof[2 * ka], dy = prof[2 * kb + 1] - prof[2 * ka + 1];
        float nr = dy, ny = -dr, l = std::sqrt(nr * nr + ny * ny); if (l < 1e-6f) { nr = 0; ny = 1; l = 1; }
        nr /= l; ny /= l;
        for (int i = 0; i <= seg; i++) {
            float th = 2 * PI * i / seg, c = std::cos(th), s = std::sin(th);
            vs.push_back(V(r * c, y, r * s, nr * c, ny, nr * s, (float)i / seg, lerpf(v0, v1, (float)k / (n - 1))));
        }
    }
    int row = seg + 1;
    for (int k = 0; k < n - 1; k++) for (int i = 0; i < seg; i++) {
        unsigned A = k * row + i, B = A + 1, C = A + row, D = C + 1;
        is.insert(is.end(), { A, C, B, B, C, D });
    }
    (void)smooth;
    Mesh m; m.upload(vs, is); return m;
}
static Mesh makeCylinder(float r0, float r1, float h, int seg, bool smooth, float phase = 0.f) {
    std::vector<Vertex> vs; std::vector<unsigned> is;
    float slope = (r0 - r1) / h;
    auto sideN = [&](float th, float& nx, float& ny, float& nz) {
        nx = std::cos(th); nz = std::sin(th); ny = slope;
        float l = std::sqrt(nx * nx + ny * ny + nz * nz); nx /= l; ny /= l; nz /= l;
        };
    for (int i = 0; i < seg; i++) {
        float t0 = phase + 2 * PI * i / seg, t1 = phase + 2 * PI * (i + 1) / seg;
        float n0x, n0y, n0z, n1x, n1y, n1z;
        if (smooth) { sideN(t0, n0x, n0y, n0z); sideN(t1, n1x, n1y, n1z); }
        else { sideN(0.5f * (t0 + t1), n0x, n0y, n0z); n1x = n0x; n1y = n0y; n1z = n0z; }
        unsigned b = (unsigned)vs.size();
        float u0 = (float)i / seg, u1 = (float)(i + 1) / seg;
        vs.push_back(V(r0 * std::cos(t0), 0, r0 * std::sin(t0), n0x, n0y, n0z, u0, 0));
        vs.push_back(V(r0 * std::cos(t1), 0, r0 * std::sin(t1), n1x, n1y, n1z, u1, 0));
        vs.push_back(V(r1 * std::cos(t0), h, r1 * std::sin(t0), n0x, n0y, n0z, u0, 1));
        vs.push_back(V(r1 * std::cos(t1), h, r1 * std::sin(t1), n1x, n1y, n1z, u1, 1));
        is.insert(is.end(), { b, b + 3, b + 1, b, b + 2, b + 3 });
    }
    auto cap = [&](float r, float y, float ny) {
        if (r <= 1e-5f) return;
        unsigned c = (unsigned)vs.size();
        vs.push_back(V(0, y, 0, 0, ny, 0, 0.5f, 0.5f));
        for (int i = 0; i <= seg; i++) {
            float t = phase + 2 * PI * i / seg;
            vs.push_back(V(r * std::cos(t), y, r * std::sin(t), 0, ny, 0, 0.5f + 0.5f * std::cos(t), 0.5f + 0.5f * std::sin(t)));
        }
        for (int i = 0; i < seg; i++) {
            if (ny > 0) is.insert(is.end(), { c, c + 2 + (unsigned)i, c + 1 + (unsigned)i });
            else        is.insert(is.end(), { c, c + 1 + (unsigned)i, c + 2 + (unsigned)i });
        }
        };
    cap(r1, h, 1); cap(r0, 0, -1);
    Mesh m; m.upload(vs, is); return m;
}
static Mesh makeBox() {
    struct F { vec3 n, u, v; };
    const F faces[6] = {
        {vec3(1,0,0),vec3(0,1,0),vec3(0,0,1)}, {vec3(-1,0,0),vec3(0,0,1),vec3(0,1,0)},
        {vec3(0,1,0),vec3(0,0,1),vec3(1,0,0)}, {vec3(0,-1,0),vec3(1,0,0),vec3(0,0,1)},
        {vec3(0,0,1),vec3(1,0,0),vec3(0,1,0)}, {vec3(0,0,-1),vec3(0,1,0),vec3(1,0,0)} };
    std::vector<Vertex> vs; std::vector<unsigned> is;
    for (int k = 0; k < 6; k++) {
        const F& f = faces[k];
        vec3 c = f.n * 0.5f; unsigned b = (unsigned)vs.size();
        vec3 p[4] = { c - f.u * 0.5f - f.v * 0.5f, c + f.u * 0.5f - f.v * 0.5f, c + f.u * 0.5f + f.v * 0.5f, c - f.u * 0.5f + f.v * 0.5f };
        float uv[4][2] = { {0,0},{1,0},{1,1},{0,1} };
        for (int i = 0; i < 4; i++) vs.push_back(V(p[i].x, p[i].y, p[i].z, f.n.x, f.n.y, f.n.z, uv[i][0], uv[i][1]));
        is.insert(is.end(), { b, b + 1, b + 2, b, b + 2, b + 3 });
    }
    Mesh m; m.upload(vs, is); return m;
}
static Mesh makeQuad() {
    std::vector<Vertex> vs = { V(-.5f,-.5f,0, 0,0,1, 0,0), V(.5f,-.5f,0, 0,0,1, 1,0), V(.5f,.5f,0, 0,0,1, 1,1), V(-.5f,.5f,0, 0,0,1, 0,1) };
    std::vector<unsigned> is = { 0, 1, 2, 0, 2, 3 };
    Mesh m; m.upload(vs, is); return m;
}
static Mesh makeCloth(float w, float h, int segs, float amp, float freq) {     // vacuum flag: frozen folds
    std::vector<Vertex> vs; std::vector<unsigned> is;
    for (int i = 0; i <= segs; i++) {
        float u = (float)i / segs, x = u * w;
        float z = amp * std::sin(u * freq) * (0.25f + 0.75f * u);
        float dz = amp * (std::cos(u * freq) * freq * (0.25f + 0.75f * u) + std::sin(u * freq) * 0.75f) / w;
        vec3 n = normalize(vec3(-dz, 0, 1));
        vs.push_back(V(x, 0, z, n.x, n.y, n.z, u, 1));
        vs.push_back(V(x, -h, z, n.x, n.y, n.z, u, 0));
    }
    for (int i = 0; i < segs; i++) {
        unsigned t0 = 2 * i, b0 = t0 + 1, t1 = t0 + 2, b1 = t0 + 3;
        is.insert(is.end(), { b0, b1, t1, b0, t1, t0 });
    }
    Mesh m; m.upload(vs, is); return m;
}
// open tube used for rocket plumes: radius grows from r0 (y=0) to r1 (y=-1); uv.y = 0..1 along the plume
static Mesh makePlume(int seg, int rings) {
    std::vector<Vertex> vs; std::vector<unsigned> is;
    for (int k = 0; k <= rings; k++) {
        float v = (float)k / rings, r = 0.25f + 0.75f * std::sqrt(v), y = -v;
        for (int i = 0; i <= seg; i++) {
            float th = 2 * PI * i / seg, c = std::cos(th), s = std::sin(th);
            vs.push_back(V(r * c, y, r * s, c, 0, s, (float)i / seg, v));
        }
    }
    int row = seg + 1;
    for (int k = 0; k < rings; k++) for (int i = 0; i < seg; i++) {
        unsigned A = k * row + i, B = A + 1, C = A + row, D = C + 1;
        is.insert(is.end(), { A, B, C, B, D, C });
    }
    Mesh m; m.upload(vs, is); return m;
}

// ============================================================================
//  5. SHADER PROGRAM WRAPPER
// ============================================================================
struct Program {
    GLuint id = 0;
    std::unordered_map<std::string, GLint> cache;
    static GLuint compile(GLenum type, const std::string& src, const char* name) {
        GLuint s = glCreateShader(type); const char* p = src.c_str();
        glShaderSource(s, 1, &p, nullptr); glCompileShader(s);
        GLint ok = 0; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
        if (!ok) {
            GLint len = 0; glGetShaderiv(s, GL_INFO_LOG_LENGTH, &len);
            std::vector<char> log(len + 1); glGetShaderInfoLog(s, len, nullptr, log.data());
            std::fprintf(stderr, "[shader %s] %s compile error:\n%s\n", name, type == GL_VERTEX_SHADER ? "VS" : "FS", log.data());
            return 0;
        }
        return s;
    }
    bool build(const std::string& vs, const std::string& fs, const char* name) {
        GLuint v = compile(GL_VERTEX_SHADER, vs, name), f = compile(GL_FRAGMENT_SHADER, fs, name);
        if (!v || !f) return false;
        id = glCreateProgram(); glAttachShader(id, v); glAttachShader(id, f); glLinkProgram(id);
        glDeleteShader(v); glDeleteShader(f);
        GLint ok = 0; glGetProgramiv(id, GL_LINK_STATUS, &ok);
        if (!ok) {
            GLint len = 0; glGetProgramiv(id, GL_INFO_LOG_LENGTH, &len);
            std::vector<char> log(len + 1); glGetProgramInfoLog(id, len, nullptr, log.data());
            std::fprintf(stderr, "[shader %s] link error:\n%s\n", name, log.data()); return false;
        }
        return true;
    }
    void use() const { glUseProgram(id); }
    GLint loc(const char* n) {
        std::unordered_map<std::string, GLint>::iterator it = cache.find(n);
        if (it != cache.end()) return it->second;
        GLint l = glGetUniformLocation(id, n); cache[n] = l; return l;
    }
    void set(const char* n, int v) { glUniform1i(loc(n), v); }
    void set(const char* n, float v) { glUniform1f(loc(n), v); }
    void set(const char* n, float a, float b) { glUniform2f(loc(n), a, b); }
    void set(const char* n, vec3 v) { glUniform3f(loc(n), v.x, v.y, v.z); }
    void set(const char* n, const mat4& m) { glUniformMatrix4fv(loc(n), 1, GL_FALSE, m.m); }
    void set(const char* n, const Basis& b) {
        float m[9] = { b.x.x, b.x.y, b.x.z, b.y.x, b.y.y, b.y.z, b.z.x, b.z.y, b.z.z };
        glUniformMatrix3fv(loc(n), 1, GL_FALSE, m);
    }
};

// ============================================================================
//  6. GLSL 3.30 SHADERS
//     (kept in several small raw strings - MSVC limits one literal to 16 KB)
//     World space in the shaders is CAMERA-RELATIVE: the camera sits at 0,0,0.
//     Depth is logarithmic so 2 cm boots and 400 000 km planets share one buffer.
// ============================================================================
namespace sh {

    static const char* header = "#version 330 core\n";

    // ------------------------------------------------------------ objects: vertex shader (log depth)
    static const char* objVS = R"(
layout(location=0) in vec3 aPos; layout(location=1) in vec3 aNrm; layout(location=2) in vec2 aUV;
uniform mat4 uModel; uniform mat4 uViewProj; uniform mat4 uLightVP; uniform float uNormalOffset; uniform float uLogC;
out vec3 vWorld; out vec3 vNrm; out vec2 vUV; out vec4 vLightPos; out vec3 vLocal; out float vLogZ;
void main(){
  vec4 w = uModel*vec4(aPos,1.0);
  vec3 n = normalize(mat3(transpose(inverse(uModel)))*aNrm);
  vWorld = w.xyz; vNrm = n; vUV = aUV; vLocal = aPos;
  vLightPos = uLightVP*vec4(w.xyz + n*uNormalOffset, 1.0);
  gl_Position = uViewProj*w;
  vLogZ = 1.0 + gl_Position.w;
  gl_Position.z = (log2(max(1e-6, vLogZ))*uLogC - 1.0)*gl_Position.w;
}
)";



    // ------------------------------------------------------------ lighting helpers shared by object shaders
    static const char* litHelpers = R"(
vec3 ambientTerm(vec3 N, vec3 albedo){
  float k = dot(N, uUp);
  return albedo*(uAmbUp*(0.5+0.5*k) + uAmbDown*(0.5-0.5*k) + uEarthCol*max(dot(N,uEarthDir),0.0));
}
vec3 envColor(vec3 R){ float k = dot(R,uUp); return mix(uAmbDown, uAmbUp, smoothstep(-0.25,0.25,k)); }
vec3 applyFog(vec3 c, float d){ float f = 1.0-exp(-d*uFogDens); return mix(c, uFogCol, f); }
)";

    // ------------------------------------------------------------ generic lit object (Blinn-Phong + Fresnel env reflection)
    static const char* litFS = R"(
uniform vec3 uAlbedo; uniform vec3 uEmissive; uniform float uSpec; uniform float uShin; uniform float uMetal;
uniform int uHasTex; uniform int uTwoSided; uniform int uTexMode; uniform sampler2D uTex;
void main(){
  writeDepth();
  vec3 albedo = uAlbedo; float metal = uMetal, spec = uSpec, shin = uShin;
  if(uHasTex==1){
    vec4 t = texture(uTex, vUV); albedo *= t.rgb;
    if(uTexMode==1){ metal *= t.a; spec = mix(0.12, spec, t.a); shin = mix(14.0, shin, t.a); }
    if(uTexMode==2 && t.a < 0.5) discard;
  }
  vec3 N = normalize(vNrm);
  if(uTwoSided==1 && !gl_FrontFacing) N = -N;
  vec3 V = normalize(-vWorld), L = uSunDir, H = normalize(L+V);
  float nl = max(dot(N,L), 0.0);
  float sh = nl > 0.0 ? shadowFactor(vLightPos) : 0.0;
  vec3 F0 = mix(vec3(0.04), albedo, metal);
  vec3 F  = F0 + (1.0-F0)*pow(1.0-max(dot(H,V),0.0), 5.0);
  float norm = (shin+8.0)/25.13;
  vec3 specC = F*pow(max(dot(N,H),0.0), shin)*norm*spec*nl;
  vec3 diffuse = albedo*(1.0-metal)*nl;
  float nv = max(dot(N,V),0.0);
  vec3 Fe = F0 + (1.0-F0)*pow(1.0-nv,5.0)*0.5;
  vec3 env = envColor(reflect(-V,N))*Fe*min(spec,1.0)*(0.35+0.65*metal);
  vec3 col = (diffuse + specC)*uSunCol*sh + ambientTerm(N, albedo*(1.0-0.75*metal)) + env + uEmissive;
  outColor = vec4(applyFog(col, length(vWorld)), 1.0);
}
)";

    // ------------------------------------------------------------ additive glow: rocket plumes, re-entry plasma
    static const char* glowFS = R"(
in vec3 vWorld; in vec3 vNrm; in vec2 vUV; in vec4 vLightPos; in vec3 vLocal; in float vLogZ;
layout(location=0) out vec4 outColor;
uniform float uLogC; uniform vec3 uCoreCol; uniform vec3 uOuterCol; uniform float uIntensity;
uniform float uDiamond; uniform float uTime; uniform int uMode; uniform vec3 uVelDir; uniform float uSeed;
void main(){
  gl_FragDepth = log2(vLogZ)*uLogC*0.5;
  vec3 V = normalize(-vWorld), N = normalize(vNrm);
  float facing = abs(dot(N,V));
  float I; vec3 c;
  if(uMode==0){                                   // plume: open tube, uv.y = 0 at the nozzle
    float a = vUV.y;
    float thick = pow(facing, 1.3);
    float fade = (1.0-smoothstep(0.25, 1.0, a))*smoothstep(0.0, 0.02, a);
    float flick = 0.86 + 0.14*sin(uTime*71.0 + a*37.0 + uSeed) * sin(uTime*13.0 + vUV.x*31.4);
    float k = fract(a*6.5 - 0.15);
    float diam = exp(-pow((k-0.5)*6.0, 2.0))*(1.0-smoothstep(0.1,0.75,a))*uDiamond;
    c = mix(uCoreCol, uOuterCol, smoothstep(0.0, 0.45, a));
    I = thick*fade*flick + diam*thick*1.6;
  } else {                                        // plasma sheath on an ellipsoid around the hull
    float front = max(dot(N, -uVelDir), 0.0);
    float rim = pow(1.0-facing, 2.0);
    float flick = 0.8 + 0.2*sin(uTime*40.0 + vUV.x*60.0 + uSeed)*sin(uTime*23.0 + vUV.y*40.0);
    I = (pow(front, 3.0)*1.4 + rim*0.6*front + 0.08)*flick;
    c = mix(uOuterCol, uCoreCol, pow(front, 4.0));
  }
  outColor = vec4(c*I*uIntensity, 1.0);
}
)";

    // ------------------------------------------------------------ stars (direction only: no parallax)
    static const char* starVS = R"(
layout(location=0) in vec3 aDir; layout(location=1) in vec2 aMagTint;
uniform mat4 uViewProjRot; uniform float uGain; out vec3 vCol;
void main(){
  gl_Position = uViewProjRot*vec4(aDir*1000.0, 1.0); gl_Position.z = gl_Position.w*0.9999;
  float m = aMagTint.x;
  gl_PointSize = 1.0 + 2.0*m*m;
  vec3 tint = mix(vec3(1.0,0.82,0.65), vec3(0.72,0.82,1.0), aMagTint.y);
  vCol = tint*(0.10 + 2.4*m*m*m)*uGain;
}
)";
    static const char* starFS = R"(
in vec3 vCol; layout(location=0) out vec4 outColor;
void main(){ vec2 c = gl_PointCoord*2.0-1.0; float a = clamp(1.0-dot(c,c),0.0,1.0); outColor = vec4(vCol*a, 1.0); }
)";

    // ------------------------------------------------------------ particles (soft round sprites, log depth)
    static const char* partVS = R"(
layout(location=0) in vec4 aPosSize; layout(location=1) in vec4 aColor;
uniform mat4 uViewProj; uniform float uProjScale; uniform float uLogC;
out vec4 vColor; out float vLogZ;
void main(){
  gl_Position = uViewProj*vec4(aPosSize.xyz, 1.0);
  gl_PointSize = clamp(aPosSize.w*uProjScale/max(gl_Position.w,0.01), 1.0, 160.0);
  vColor = aColor; vColor.a *= smoothstep(1.0, 6.0, gl_Position.w);
  vLogZ = 1.0 + gl_Position.w;
  gl_Position.z = (log2(max(1e-6, vLogZ))*uLogC - 1.0)*gl_Position.w;
}
)";
    static const char* partFS = R"(
in vec4 vColor; in float vLogZ; layout(location=0) out vec4 outColor; uniform float uLogC;
void main(){
  vec2 c = gl_PointCoord*2.0-1.0; float r2 = dot(c,c);
  if(r2 > 1.0) discard;
  gl_FragDepth = log2(vLogZ)*uLogC*0.5;
  outColor = vec4(vColor.rgb, vColor.a*pow(1.0-r2, 1.5));
}
)";

    // ------------------------------------------------------------ uniforms + noise shared by planet / terrain / ground
    static const char* uniformsCommon = R"(
uniform vec3 uSunDir; uniform vec3 uSunCol; uniform vec3 uSunCol0; uniform vec3 uUp; uniform float uTime;
uniform vec3 uAmbUp; uniform vec3 uAmbDown; uniform vec3 uEarthDir; uniform vec3 uEarthCol;
uniform vec3 uFogCol; uniform float uFogDens; uniform float uLogC;
uniform sampler2DShadow uShadow; uniform float uShadowOn; uniform float uShadowTexel;
float hash12(vec2 p){ vec3 p3 = fract(vec3(p.xyx)*0.1031); p3 += dot(p3, p3.yzx+33.33); return fract((p3.x+p3.y)*p3.z); }
vec2 hash22(vec2 p){ vec3 p3 = fract(vec3(p.xyx)*vec3(0.1031,0.1030,0.0973)); p3 += dot(p3, p3.yzx+33.33); return fract((p3.xx+p3.yz)*p3.zy); }
float vnoise2(vec2 p){ vec2 i=floor(p), f=fract(p); vec2 u=f*f*(3.0-2.0*f);
  return mix(mix(hash12(i),hash12(i+vec2(1,0)),u.x), mix(hash12(i+vec2(0,1)),hash12(i+vec2(1,1)),u.x), u.y); }
float fbm2(vec2 p){ float s=0.0,a=0.5; for(int i=0;i<4;i++){ s+=a*vnoise2(p); p=p*2.03+vec2(17.1,9.2); a*=0.5; } return s/0.9375; }
)";

    // in-variables of the object vertex shader + depth / shadow helpers
    static const char* objIn = R"(
in vec3 vWorld; in vec3 vNrm; in vec2 vUV; in vec4 vLightPos; in vec3 vLocal; in float vLogZ;
layout(location=0) out vec4 outColor;
void writeDepth(){ gl_FragDepth = log2(vLogZ)*uLogC*0.5; }
float shadowFactor(vec4 lp){
  if(uShadowOn < 0.5) return 1.0;
  vec3 p = lp.xyz/lp.w*0.5+0.5;
  if(p.z >= 1.0 || p.x<0.0 || p.x>1.0 || p.y<0.0 || p.y>1.0) return 1.0;
  float s = 0.0;
  for(int y=-2; y<=2; y++) for(int x=-2; x<=2; x++)
    s += texture(uShadow, vec3(p.xy + vec2(x,y)*uShadowTexel, p.z - 0.0004));
  float edge = smoothstep(0.0,0.04,min(min(p.x,1.0-p.x),min(p.y,1.0-p.y)));
  return mix(1.0, s/25.0, edge);
}
)";

    // ------------------------------------------------------------ Earth atmosphere (single scattering, Rayleigh + Mie)
    static const char* atmoCommon = R"(
const float RE = 6371000.0, RA = 6471000.0, RC = 6373600.0;
const vec3  BR = vec3(5.8e-6, 13.5e-6, 33.1e-6);
const float BM = 21e-6, HR = 8000.0, HM = 1200.0;
uniform vec3 uEC; uniform vec3 uEUp; uniform float uEH; uniform vec3 uSunE; uniform vec3 uSkyAmb; uniform vec3 uSkyRefl;
// camera at the origin, sphere centre at -(R+h)*up.  Numerically stable even 2 m above a 6371 km planet.
vec2 camSphere(vec3 rd, vec3 up, float h, float R){
  float B = (R+h)*dot(rd, up), C = h*(2.0*R+h), d = B*B - C;
  if(d < 0.0) return vec2(-1.0);
  float s = sqrt(d);
  if(C > 0.0){ if(B >= 0.0) return vec2(-1.0); return vec2(C/(-B+s), -B+s); }
  return vec2(-1e-3, -B+s);
}
vec2 lightDepth(vec3 p){
  vec3 oc = p - uEC;
  float b = dot(oc, uSunDir), r2 = dot(oc,oc);
  float ce = r2 - RE*RE, de = b*b - ce;
  if(ce > 0.0 && de > 0.0 && b < 0.0) return vec2(1e9);          // in the Earth's shadow
  float tEnd = -b + sqrt(max(b*b - (r2 - RA*RA), 0.0));
  vec2 od = vec2(0.0); float dt = tEnd/4.0;
  for(int j=0;j<4;j++){ float h = length(oc + uSunDir*(dt*(float(j)+0.5))) - RE; od += exp(-vec2(h/HR, h/HM))*dt; }
  return od;
}
void atmo(vec3 rd, float t0, float t1, out vec3 S, out vec3 T){
  const int N = 12;
  float mu = dot(rd, uSunDir);
  float pr = 0.0596831*(1.0+mu*mu);
  const float g = 0.76;
  float pm = 0.1193662*((1.0-g*g)*(1.0+mu*mu))/((2.0+g*g)*pow(1.0+g*g-2.0*g*mu, 1.5));
  float dt = (t1-t0)/float(N); vec2 od = vec2(0.0); vec3 sR = vec3(0.0), sM = vec3(0.0);
  for(int i=0;i<N;i++){
    vec3 p = rd*(t0 + dt*(float(i)+0.5));
    float h = length(p - uEC) - RE;
    vec2 d = exp(-vec2(h/HR, h/HM))*dt;
    od += d*0.5;
    vec2 ld = lightDepth(p);
    vec3 att = exp(-(BR*(od.x+ld.x) + BM*1.1*(od.y+ld.y)));
    sR += d.x*att; sM += d.y*att;
    od += d*0.5;
  }
  S = uSunE*(sR*BR*pr + sM*BM*pm);
  T = exp(-(BR*od.x + BM*1.1*od.y));
}
)";

    // ------------------------------------------------------------ Earth surface material (shared by the globe and the pad mesh)
    static const char* earthCommon = R"(
uniform sampler2D uEarthTex; uniform sampler2D uCloudTex; uniform mat3 uPadRot; uniform vec3 uCamPad;
vec2 earthUV(vec3 d){ return vec2(atan(-d.z, d.x)*0.15915494+0.5, asin(clamp(d.y,-1.0,1.0))*0.31830989+0.5); }
float coastX(float z){ return 640.0 + 260.0*sin(z/1900.0+0.7) + 120.0*sin(z/610.0+2.0) + 35.0*sin(z/170.0) - 0.18*z; }
void earthMaterial(vec3 lp, vec3 dirC, float dist, out vec3 alb, out float water){
  vec4 g = texture(uEarthTex, earthUV(dirC));
  alb = g.rgb; water = g.a;
  float land = 1.0 - water;
  float fd = 1.0 - smoothstep(4.0e4, 4.0e5, dist);                  // mid-range texture detail on land
  alb *= 1.0 + land*fd*(0.45*(fbm2(lp.xz/3800.0)-0.5) + 0.25*(vnoise2(lp.xz/700.0)-0.5));
  float dPad = length(lp.xz);
  float wLoc = 1.0 - smoothstep(3.0e4, 9.0e4, dPad);
  if(wLoc > 0.0){
    float sd = lp.x - coastX(lp.z);                                   // > 0 : Indian Ocean
    float n1 = fbm2(lp.xz/900.0), n2 = vnoise2(lp.xz/140.0), n3 = vnoise2(lp.xz/23.0);
    vec3 l = mix(vec3(0.075,0.105,0.035), vec3(0.17,0.15,0.075), smoothstep(0.35,0.7,n1));
    l = mix(l, vec3(0.035,0.075,0.022), smoothstep(0.55,0.8,n2)*0.85);
    l *= 0.85 + 0.3*n3*(1.0-smoothstep(200.0,2000.0,dist));
    float beach = smoothstep(-110.0,-60.0,sd);
    l = mix(l, vec3(0.62,0.56,0.44), beach);
    float pad = 1.0 - smoothstep(66.0, 70.0, dPad);
    l = mix(l, vec3(0.40,0.395,0.38)*(0.92+0.08*n3), pad);
    float road = (1.0-smoothstep(5.0,6.5,abs(lp.z - 30.0*sin(lp.x/700.0))))*step(lp.x,-66.0)*step(-3200.0,lp.x);
    l = mix(l, vec3(0.20,0.20,0.19), road);
    float tank = 1.0 - smoothstep(48.0, 52.0, length(lp.xz - vec2(-180.0, 140.0)));
    l = mix(l, vec3(0.36,0.35,0.33), tank);
    float wl = smoothstep(-2.0, 3.0, sd);
    vec3 sea = mix(vec3(0.03,0.20,0.20), vec3(0.006,0.03,0.075), smoothstep(60.0, 2200.0, sd));
    float reef = exp(-pow((sd-380.0)/40.0,2.0))*(0.5+0.5*vnoise2(lp.xz/60.0));
    sea = mix(sea, vec3(0.30,0.36,0.34), reef*0.55);
    alb = mix(alb, mix(l, sea, wl), wLoc); water = mix(water, wl*(1.0-reef*0.5), wLoc);
  }
}
vec3 waterNormal(vec2 q, float dist, vec3 Ng){
  float f = 1.0 - smoothstep(200.0, 6000.0, dist);
  if(f <= 0.0) return Ng;
  vec2 p1 = q/24.0 + vec2(uTime*0.05, uTime*0.02), p2 = q/7.5 + vec2(-uTime*0.09, uTime*0.06);
  float h0 = vnoise2(p1)+0.45*vnoise2(p2);
  float hx = vnoise2(p1+vec2(0.0625,0.0))+0.45*vnoise2(p2+vec2(0.2,0.0));
  float hz = vnoise2(p1+vec2(0.0,0.0625))+0.45*vnoise2(p2+vec2(0.0,0.2));
  vec2 gr = vec2(hx-h0, hz-h0)*0.9*f;
  return normalize(Ng - uPadRot[0]*gr.x - uPadRot[2]*gr.y);
}
vec3 earthShade(vec3 p, vec3 Ng, vec3 V, float dist, float shadow){
  vec3 lp = transpose(uPadRot)*(p + uCamPad);
  vec3 alb; float water;
  earthMaterial(lp, normalize(p - uEC), dist, alb, water);
  vec2 ld = lightDepth(p);
  vec3 E = uSunE*exp(-(BR*ld.x + BM*1.1*ld.y))*shadow;
  vec3 N = water > 0.01 ? normalize(mix(Ng, waterNormal(lp.xz, dist, Ng), water)) : Ng;
  float nl = max(dot(N, uSunDir), 0.0);
  vec3 col = alb*E*nl*0.3183 + alb*uSkyAmb;
  if(water > 0.01){
    vec3 H = normalize(uSunDir + V);
    float nv = max(dot(N,V), 0.0);
    float F = 0.02 + 0.98*pow(1.0-nv, 5.0);
    float rough = mix(1400.0, 90.0, smoothstep(300.0, 2.0e5, dist));
    float spec = pow(max(dot(N,H),0.0), rough)*(rough+8.0)*0.04;
    col += water*(F*uSkyRefl*(1.0-0.4*nv) + E*spec*F*0.3183*step(0.0, dot(Ng, uSunDir)));
  }
  return col;
}
float cloudDensity(vec3 p, float dist){
  vec3 d = normalize(p - uEC);
  float cov = texture(uCloudTex, earthUV(d)).r;
  vec3 lp = transpose(uPadRot)*(p + uCamPad);
  float det = fbm2(lp.xz/5200.0)*0.6 + fbm2(lp.xz/1250.0)*0.4*(1.0-smoothstep(3.0e4, 1.5e5, dist));
  float dens = smoothstep(0.45, 0.75, cov + (det-0.5)*0.55);
  return dens*smoothstep(9000.0, 20000.0, length(lp.xz));            // clear launch weather over the pad
}
vec3 cloudShade(vec3 cp, vec3 rd){
  vec2 ld = lightDepth(cp);
  vec3 Ts = exp(-(BR*ld.x + BM*1.1*ld.y));
  float ph = 0.55 + 0.7*pow(max(dot(rd, uSunDir),0.0), 8.0);
  float below = step(uEH, 2600.0);
  return 0.85*(uSunE*Ts*0.3183*ph*mix(1.0, 0.35, below) + uSkyAmb*1.3);
}
)";

    // ------------------------------------------------------------ Moon regolith (shared by the globe and the landing-site mesh)
    static const char* moonCommon = R"(
uniform mat3 uSiteRot; uniform vec3 uCamSite; uniform sampler2D uTileA; uniform sampler2D uTileN;
vec3 craterLayer(vec2 p, float scale, float density, float seed){
  vec2 q = p/scale; vec2 i = floor(q), f = fract(q);
  vec3 acc = vec3(0.0);
  for(int y=-1;y<=1;y++) for(int x=-1;x<=1;x++){
    vec2 g = vec2(float(x),float(y)), cell = i+g;
    if(hash12(cell*1.37 + seed) > density) continue;
    vec2 c = g + 0.2 + 0.6*hash22(cell + seed*3.1);
    float rad = 0.16 + 0.26*hash12(cell + seed*7.7);
    vec2 dv = f - c; float l = length(dv), dn = l/rad;
    if(dn > 2.0) continue;
    float e = exp(-(dn-1.0)*(dn-1.0)/0.05);
    float slope = (dn < 1.0 ? 2.0*dn : 0.0) - 0.35*2.0*(dn-1.0)/0.05*e;
    acc.xy += dv/max(l,1e-5)*slope*0.22;
    acc.z += e;
  }
  return acc;
}
// ls: position in landing-site coordinates, Ng: geometric normal (world), V: to camera
vec3 regolith(vec3 ls, vec3 Ng, vec3 V, float dist, vec3 albBase, float shadow, float zone, float farW){
  vec3 T = uSiteRot[0], B = uSiteRot[2];
  vec2 xz = ls.xz;
  float l1 = 1.0-smoothstep(40.0, 260.0, dist), l2 = 1.0-smoothstep(350.0, 2600.0, dist);
  vec3 a1 = texture(uTileA, xz/12.0).rgb, a2 = texture(uTileA, xz/113.0 + vec2(0.31,0.57)).rgb;
  float macro = clamp(dot(a2, vec3(0.3333))/0.14, 0.55, 1.6);
  vec3 alb = albBase*mix(vec3(1.0), a1/0.14, l1)*mix(1.0, macro, 0.7*l2);
  vec3 n1 = texture(uTileN, xz/12.0).xyz*2.0-1.0, n2 = texture(uTileN, xz/113.0 + vec2(0.31,0.57)).xyz*2.0-1.0;
  vec2 d = n1.xy*(1.0-smoothstep(30.0,200.0,dist)) + n2.xy*(1.0-smoothstep(300.0,1800.0,dist))*0.8;
  vec3 cr = vec3(0.0);
  cr += craterLayer(xz, 9000.0, 0.45, 0.5)*farW*(1.0-smoothstep(2.0e5, 9.0e5, dist));
  cr += craterLayer(xz, 2200.0, 0.50, 1.5)*farW*(1.0-smoothstep(4.0e4, 2.5e5, dist));
  cr += craterLayer(xz, 520.0, 0.55, 2.5)*zone*(1.0-smoothstep(1.2e4, 6.0e4, dist));
  cr += craterLayer(xz, 130.0, 0.60, 3.5)*zone*(1.0-smoothstep(3000.0, 1.5e4, dist));
  d -= cr.xy;
  alb *= 0.82*(1.0 + 0.10*min(cr.z, 1.0));
  vec3 N = normalize(Ng + T*d.x + B*d.y);
  vec3 L = uSunDir;
  float nl = clamp((dot(N,L)+0.10)/1.10, 0.0, 1.0), nv = max(dot(N,V),0.0);
  float ls_ = 2.0*nl/(nl+nv+0.05);
  float diff = mix(nl, ls_, 0.45);
  float surge = 1.0 + 0.55*pow(max(dot(V,L),0.0), 14.0);
  return alb*uSunCol0*diff*surge*shadow*step(0.0, dot(Ng,L)+0.15)
       + alb*(uSunCol0*0.05 + uEarthCol*max(dot(N,uEarthDir),0.0));
}
)";

    // ------------------------------------------------------------ full-screen planet pass: Earth + atmosphere + clouds + Moon + Sun
    static const char* fsTriVS = R"(
out vec2 vUV;
void main(){ vec2 p = vec2((gl_VertexID<<1)&2, gl_VertexID&2); vUV = p; gl_Position = vec4(p*2.0-1.0, 0.0, 1.0); }
)";
    static const char* planetFS = R"(
in vec2 vUV; layout(location=0) out vec4 outColor;
uniform mat4 uInvVP;
uniform vec3 uMC; uniform vec3 uMUp; uniform float uMH; uniform mat3 uMoonRot;
uniform sampler2D uMoonTex; uniform sampler2D uMoonNrm;
const float RM = 1737400.0;
vec3 moonShade(vec3 p, vec3 V, float t){
  vec3 Ng = normalize(p - uMC);
  vec3 dl = transpose(uMoonRot)*Ng;
  vec2 uv = vec2(atan(dl.z, dl.x)*0.15915494+0.5, asin(clamp(dl.y,-1.0,1.0))*0.31830989+0.5);
  vec3 alb = texture(uMoonTex, uv).rgb;
  vec3 Nt = normalize(uMoonRot*(texture(uMoonNrm, uv).xyz*2.0-1.0));
  vec3 ls = transpose(uSiteRot)*(p + uCamSite);
  float ds = length(ls.xz);
  vec3 N0 = normalize(mix(Ng, Nt, smoothstep(12000.0, 60000.0, ds)));
  float farW = (1.0-smoothstep(1.5e5, 4.2e5, ds))*(1.0-smoothstep(3.0e4, 6.0e4, -ls.y));
  return regolith(ls, N0, V, t, alb, 1.0, smoothstep(250.0, 600.0, ds)*farW, farW);
}
void main(){
  vec4 w = uInvVP*vec4(vUV*2.0-1.0, 1.0, 1.0);
  vec3 rd = normalize(w.xyz/w.w), V = -rd;
  vec2 tE = camSphere(rd, uEUp, uEH, RE);
  vec2 tM = camSphere(rd, uMUp, uMH, RM);
  float tEnd = 1e30; int hit = 0;
  if(tE.x > 0.0){ tEnd = tE.x; hit = 1; }
  if(tM.x > 0.0 && tM.x < tEnd){ tEnd = tM.x; hit = 2; }
  vec3 Lbg;
  if(hit == 1){ vec3 p = rd*tEnd; Lbg = earthShade(p, normalize(p - uEC), V, tEnd, 1.0); }
  else if(hit == 2) Lbg = moonShade(rd*tEnd, V, tEnd);
  else {
    float sd = dot(rd, uSunDir);
    Lbg = vec3(1.0,0.96,0.9)*(4000.0*smoothstep(0.999962, 0.999978, sd) + 6.0*pow(max(sd,0.0), 3000.0) + 0.25*pow(max(sd,0.0), 200.0));
  }
  vec3 col = Lbg; float trans = 1.0;
  vec2 tA = camSphere(rd, uEUp, uEH - 100000.0, RA);
  if(tA.y > 0.0){
    float t0 = max(tA.x, 0.0), t1 = min(tA.y, tEnd);
    if(t1 > t0){
      vec2 tC = camSphere(rd, uEUp, uEH - 2600.0, RC);
      float tc = (uEH > 2600.0) ? tC.x : tC.y;
      float dens = 0.0;
      if(tc > t0 && tc < t1) dens = cloudDensity(rd*tc, tc);
      vec3 S, T;
      if(dens > 0.004){
        vec3 S2, T2; atmo(rd, t0, tc, S, T); atmo(rd, tc, t1, S2, T2);
        float a = dens*0.96;
        col = S + T*(cloudShade(rd*tc, rd)*a + (1.0-a)*(S2 + T2*Lbg));
        trans = (1.0-a)*dot(T*T2, vec3(0.3333));
      } else {
        atmo(rd, t0, t1, S, T);
        col = S + T*Lbg; trans = dot(T, vec3(0.3333));
      }
    }
  }
  outColor = vec4(col, hit > 0 ? 0.0 : trans);
}
)";

    // ------------------------------------------------------------ landing-site terrain mesh (Moon)
    static const char* terrainMain = R"(
uniform vec3 uSiteAlb;
void main(){
  writeDepth();
  vec3 V = normalize(-vWorld); float dist = length(vWorld);
  vec3 Ng = normalize(vNrm);
  float sh = dot(Ng, uSunDir) > -0.1 ? shadowFactor(vLightPos) : 0.0;
  float r = length(vLocal.xz);
  outColor = vec4(regolith(vLocal, Ng, V, dist, uSiteAlb, sh, smoothstep(250.0, 600.0, r), 1.0), 1.0);
}
)";
    // ------------------------------------------------------------ launch-site ground mesh (Earth): same material + aerial perspective
    static const char* groundMain = R"(
void main(){
  writeDepth();
  float dist = length(vWorld); vec3 rd = vWorld/dist;
  float sh = shadowFactor(vLightPos);
  vec3 col = earthShade(vWorld, normalize(vNrm), -rd, dist, sh);
  vec3 S, T; atmo(rd, 0.0, dist, S, T);
  outColor = vec4(col*T + S, 1.0);
}
)";

    // ------------------------------------------------------------ post processing (bloom + ACES)
    static const char* brightFS = R"(
in vec2 vUV; layout(location=0) out vec4 o; uniform sampler2D uSrc; uniform vec2 uTexel; uniform float uThresh;
void main(){
  vec3 c = vec3(0.0);
  c += texture(uSrc, vUV + uTexel*vec2(-1,-1)).rgb; c += texture(uSrc, vUV + uTexel*vec2( 1,-1)).rgb;
  c += texture(uSrc, vUV + uTexel*vec2(-1, 1)).rgb; c += texture(uSrc, vUV + uTexel*vec2( 1, 1)).rgb;
  c = min(c*0.25, vec3(400.0));
  float l = max(c.r, max(c.g, c.b));
  o = vec4(c*smoothstep(uThresh, uThresh*2.5, l), 1.0);
}
)";
    static const char* blurFS = R"(
in vec2 vUV; layout(location=0) out vec4 o; uniform sampler2D uSrc; uniform vec2 uDir;
void main(){
  const float w[5] = float[](0.2270270, 0.1945946, 0.1216216, 0.0540541, 0.0162162);
  vec3 c = texture(uSrc, vUV).rgb*w[0];
  for(int i=1;i<5;i++){ c += texture(uSrc, vUV+uDir*float(i)).rgb*w[i]; c += texture(uSrc, vUV-uDir*float(i)).rgb*w[i]; }
  o = vec4(c,1.0);
}
)";
    static const char* compositeFS = R"(
in vec2 vUV; layout(location=0) out vec4 o;
uniform sampler2D uHdr; uniform sampler2D uBloom;
uniform float uExposure; uniform float uBloomAmt; uniform float uFade; uniform float uTime; uniform vec2 uRes; uniform float uShake;
vec3 aces(vec3 x){ return clamp((x*(2.51*x+0.03))/(x*(2.43*x+0.59)+0.14), 0.0, 1.0); }
float hash(vec2 p){ return fract(sin(dot(p, vec2(12.9898,78.233)))*43758.5453); }
void main(){
  vec2 uv = vUV;
  vec3 c = texture(uHdr, uv).rgb + texture(uBloom, uv).rgb*uBloomAmt;
  c = aces(max(c, vec3(0.0))*uExposure);
  c = pow(c, vec3(1.0/2.2));
  vec2 q = (uv-0.5)*vec2(uRes.x/uRes.y, 1.0);
  c *= mix(0.66, 1.0, smoothstep(1.15, 0.30, length(q)));
  c += (hash(uv*uRes + uTime) - 0.5)*0.012;
  o = vec4(c*uFade, 1.0);
}
)";
    // ------------------------------------------------------------ 2D overlay (captions, telemetry, player UI)
    static const char* uiVS = R"(
layout(location=0) in vec2 aPos; layout(location=1) in vec4 aCol;
uniform vec2 uRes; out vec4 vCol;
void main(){ vCol = aCol; gl_Position = vec4(aPos/uRes*2.0-1.0, 0.0, 1.0); }
)";
    static const char* uiFS = R"(
in vec4 vCol; layout(location=0) out vec4 o; void main(){ o = vCol; }
)";

} // namespace sh

// ============================================================================
//  7. TINY VECTOR FONT  (strokes in a 4 x 6 cell, used for the HUD and decals)
// ============================================================================
static const char* glyph(char c) {
    if (c >= 'a' && c <= 'z') c = (char)(c - 32);
    switch (c) {
    case 'A': return "0,0 0,4 2,6 4,4 4,0;0,3 4,3";
    case 'B': return "0,0 0,6 3,6 4,5 4,4 3,3 0,3;3,3 4,2 4,1 3,0 0,0";
    case 'C': return "4,5 3,6 1,6 0,5 0,1 1,0 3,0 4,1";
    case 'D': return "0,0 0,6 2,6 4,4 4,2 2,0 0,0";
    case 'E': return "4,6 0,6 0,0 4,0;0,3 3,3";
    case 'F': return "4,6 0,6 0,0;0,3 3,3";
    case 'G': return "4,5 3,6 1,6 0,5 0,1 1,0 3,0 4,1 4,3 2,3";
    case 'H': return "0,0 0,6;4,0 4,6;0,3 4,3";
    case 'I': return "1,6 3,6;2,6 2,0;1,0 3,0";
    case 'J': return "4,6 4,1 3,0 1,0 0,1";
    case 'K': return "0,0 0,6;4,6 0,3 4,0";
    case 'L': return "0,6 0,0 4,0";
    case 'M': return "0,0 0,6 2,3 4,6 4,0";
    case 'N': return "0,0 0,6 4,0 4,6";
    case 'O': case '0': return "1,0 0,1 0,5 1,6 3,6 4,5 4,1 3,0 1,0";
    case 'P': return "0,0 0,6 3,6 4,5 4,4 3,3 0,3";
    case 'Q': return "1,0 0,1 0,5 1,6 3,6 4,5 4,1 3,0 1,0;2,2 4,0";
    case 'R': return "0,0 0,6 3,6 4,5 4,4 3,3 0,3;2,3 4,0";
    case 'S': return "4,5 3,6 1,6 0,5 0,4 1,3 3,3 4,2 4,1 3,0 1,0 0,1";
    case 'T': return "0,6 4,6;2,6 2,0";
    case 'U': return "0,6 0,1 1,0 3,0 4,1 4,6";
    case 'V': return "0,6 2,0 4,6";
    case 'W': return "0,6 1,0 2,3 3,0 4,6";
    case 'X': return "0,0 4,6;0,6 4,0";
    case 'Y': return "0,6 2,3 4,6;2,3 2,0";
    case 'Z': return "0,6 4,6 0,0 4,0";
    case '1': return "1,5 2,6 2,0;1,0 3,0";
    case '2': return "0,5 1,6 3,6 4,5 4,4 0,0 4,0";
    case '3': return "0,5 1,6 3,6 4,5 4,4 3,3 1,3;3,3 4,2 4,1 3,0 1,0 0,1";
    case '4': return "3,0 3,6 0,2 4,2";
    case '5': return "4,6 0,6 0,3 3,3 4,2 4,1 3,0 1,0 0,1";
    case '6': return "4,5 3,6 1,6 0,5 0,1 1,0 3,0 4,1 4,2 3,3 0,3";
    case '7': return "0,6 4,6 1,0";
    case '8': return "1,3 0,4 0,5 1,6 3,6 4,5 4,4 3,3 1,3 0,2 0,1 1,0 3,0 4,1 4,2 3,3";
    case '9': return "0,1 1,0 3,0 4,1 4,5 3,6 1,6 0,5 0,4 1,3 4,3";
    case '-': return "0,3 4,3";
    case '+': return "0,3 4,3;2,1 2,5";
    case '=': return "0,4 4,4;0,2 4,2";
    case '.': return "1.6,0 2.4,0 2.4,0.7 1.6,0.7 1.6,0";
    case ',': return "2,0.6 1.4,-1";
    case '!': return "2,6 2,2;1.6,0 2.4,0 2.4,0.7 1.6,0.7 1.6,0";
    case ':': return "2,4 2,4.6;2,1 2,1.6";
    case '\'': return "2,6 2,4.5";
    case '/': return "0,0 4,6";
    case '?': return "0,5 1,6 3,6 4,5 4,4 2,3 2,2;2,0 2,0.6";
    case '%': return "0,0 4,6;0.5,5 1,5.5 0.5,6 0,5.5 0.5,5;3.5,0 4,0.5 3.5,1 3,0.5 3.5,0";
    case '(': return "3,6 2,5 1.5,3 2,1 3,0";
    case ')': return "1,6 2,5 2.5,3 2,1 1,0";
    default:  return "";
    }
}
// calls f(x0,y0,x1,y1) for each stroke segment of a glyph (glyph units)
template <class F> static void forGlyphSegments(char ch, F f) {
    const char* p = glyph(ch); bool have = false; float px = 0, py = 0;
    while (*p) {
        if (*p == ';') { have = false; ++p; continue; }
        if (*p == ' ') { ++p; continue; }
        char* e; float x = std::strtof(p, &e); p = e; if (*p == ',') ++p;
        float y = std::strtof(p, &e); p = e;
        if (have) f(px, py, x, y);
        px = x; py = y; have = true;
    }
}

// ============================================================================
//  8. PROCEDURAL TEXTURES
// ============================================================================
static GLuint uploadTex(const uint8_t* px, int W, int H, bool srgb, bool repeatS, bool repeatT, bool mip = true) {
    GLuint t; glGenTextures(1, &t); glBindTexture(GL_TEXTURE_2D, t);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, srgb ? GL_SRGB8_ALPHA8 : GL_RGBA8, W, H, 0, GL_RGBA, GL_UNSIGNED_BYTE, px);
    if (mip) glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mip ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeatS ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeatT ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    float maxA = 0; glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxA);
    if (maxA >= 1.f) glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, std::min(16.f, maxA));
    return t;
}
static uint8_t enc(float lin) { return (uint8_t)(clampf(std::pow(clampf(lin), 1.f / 2.2f)) * 255.f + 0.5f); }   // linear -> sRGB byte
static uint8_t b8(float v) { return (uint8_t)(clampf(v) * 255.f + 0.5f); }

// ---------------------------------------------------------------- lunar regolith tile (seamless, albedo + normal)
static void makeMoonTile(int N, GLuint& albedoTex, GLuint& normalTex) {
    std::vector<float> h((size_t)N * N), rimMask((size_t)N * N, 0.f);
    for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
        float u = (float)x / N, v = (float)y / N;
        h[(size_t)y * N + x] = fbm(u * 5, v * 5, 5, 11, 5) * 26.f + fbm(u * 24, v * 24, 4, 23, 24) * 5.5f + fbm(u * 96, v * 96, 3, 37, 96) * 1.6f;
    }
    Rng rng(98765u);
    for (int c = 0; c < 190; c++) {
        float cx = rng.f() * N, cy = rng.f() * N;
        float R = 5.f + std::pow(rng.f(), 3.4f) * 120.f, A = R * (0.10f + 0.07f * rng.f());
        int ext = (int)(R * 2.6f);
        for (int dy = -ext; dy <= ext; dy++) for (int dx = -ext; dx <= ext; dx++) {
            float w = 1.f + 0.40f * (vnoise(dx / R * 2.2f + c * 7.1f, dy / R * 2.2f, 61) - 0.5f);
            float d = std::sqrt((float)(dx * dx + dy * dy)) / R * w;
            if (d > 2.6f) continue;
            float bowl = d < 1.f ? -(1.f - d * d) : 0.f;
            float rim = 0.20f * std::exp(-((d - 1.f) / 0.32f) * ((d - 1.f) / 0.32f));
            float ej = d > 1.f ? 0.07f * std::exp(-(d - 1.f) * 1.6f) : 0.f;
            size_t idx = (size_t)wrapi((int)cy + dy, N) * N + wrapi((int)cx + dx, N);
            h[idx] += A * (bowl + rim + ej);
            rimMask[idx] = std::max(rimMask[idx], rim * 2.6f);
        }
    }
    std::vector<uint8_t> alb((size_t)N * N * 4), nrm((size_t)N * N * 4);
    for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
        size_t i = (size_t)y * N + x;
        float dx = (h[(size_t)y * N + wrapi(x + 1, N)] - h[(size_t)y * N + wrapi(x - 1, N)]) * 0.5f;
        float dy = (h[(size_t)wrapi(y + 1, N) * N + x] - h[(size_t)wrapi(y - 1, N) * N + x]) * 0.5f;
        vec3 n = normalize(vec3(-dx * 0.7f, -dy * 0.7f, 1.f));
        nrm[i * 4 + 0] = b8(n.x * 0.5f + 0.5f); nrm[i * 4 + 1] = b8(n.y * 0.5f + 0.5f); nrm[i * 4 + 2] = b8(n.z * 0.5f + 0.5f); nrm[i * 4 + 3] = 255;
        float u = (float)x / N, v = (float)y / N;
        float a = 0.105f + 0.07f * fbm(u * 6, v * 6, 4, 83, 6) + 0.045f * (fbm(u * 64, v * 64, 3, 71, 64) - 0.5f);
        a += 0.03f * std::min(rimMask[i], 1.f);
        if (hash2i(x, y, 5) > 0.9985f) a += 0.20f;
        a = clampf(a, 0.04f, 0.45f);
        alb[i * 4 + 0] = enc(a); alb[i * 4 + 1] = enc(a * 0.965f); alb[i * 4 + 2] = enc(a * 0.92f); alb[i * 4 + 3] = 255;
    }
    albedoTex = uploadTex(alb.data(), N, N, true, true, true);
    normalTex = uploadTex(nrm.data(), N, N, false, true, true);
}

// ---------------------------------------------------------------- Earth: continents from rough real coastlines (lon, lat)
static const float AFRICA[] = { -17.1f,14.7f, -16.5f,19.5f, -17.0f,21.0f, -15.0f,24.5f, -13.2f,27.6f, -9.8f,30.0f, -9.6f,32.5f, -6.8f,34.0f, -5.9f,35.8f, -2.0f,35.1f, 1.0f,36.5f, 3.0f,36.8f, 8.0f,36.9f, 10.2f,37.2f, 11.1f,35.2f, 10.1f,34.2f, 11.2f,33.2f, 15.2f,32.3f, 19.0f,30.3f, 20.1f,31.0f, 20.0f,32.9f, 22.0f,32.9f, 25.2f,31.7f, 29.0f,30.9f, 32.3f,31.3f, 34.2f,31.2f, 34.6f,29.5f, 32.6f,29.9f, 33.6f,27.7f, 35.1f,24.2f, 36.9f,22.0f, 37.4f,18.9f, 38.5f,17.9f, 39.7f,15.1f, 41.6f,13.4f, 43.3f,12.5f, 44.6f,10.4f, 47.5f,11.1f, 51.2f,11.8f, 51.1f,10.4f, 50.5f,9.0f, 48.7f,5.6f, 47.0f,3.5f, 44.0f,1.5f, 42.0f,-0.8f, 40.5f,-2.3f, 40.13f,-3.25f, 39.6f,-4.6f, 38.8f,-6.5f, 39.4f,-8.3f, 40.5f,-10.6f, 40.6f,-14.5f, 39.5f,-16.6f, 36.8f,-18.0f, 35.2f,-21.0f, 35.5f,-24.0f, 32.8f,-25.9f, 32.4f,-28.6f, 30.5f,-31.0f, 27.5f,-33.5f, 25.6f,-34.0f, 22.0f,-34.2f, 20.0f,-34.8f, 18.4f,-34.1f, 17.9f,-31.5f, 16.5f,-28.6f, 15.2f,-26.7f, 14.4f,-22.9f, 13.2f,-20.0f, 11.8f,-17.3f, 12.3f,-13.4f, 13.6f,-10.9f, 13.0f,-8.4f, 12.2f,-6.0f, 11.1f,-3.9f, 9.3f,-0.8f, 9.6f,2.0f, 9.8f,3.8f, 8.6f,4.5f, 6.1f,4.3f, 4.4f,6.3f, 2.5f,6.3f, 0.5f,5.8f, -2.0f,4.7f, -4.0f,5.2f, -7.5f,4.4f, -9.3f,5.4f, -11.5f,6.9f, -13.2f,8.9f, -14.7f,10.7f, -16.6f,12.2f };
static const float MADAGASCAR[] = { 49.3f,-12.0f, 50.4f,-15.4f, 49.8f,-17.2f, 49.0f,-19.8f, 47.6f,-23.7f, 47.1f,-25.0f, 45.2f,-25.6f, 43.8f,-24.8f, 43.3f,-22.3f, 44.4f,-19.9f, 44.0f,-17.3f, 46.3f,-15.8f, 47.9f,-14.5f, 48.8f,-12.6f };
static const float ARABIA[] = { 32.6f,29.9f, 34.6f,29.5f, 35.0f,28.0f, 37.0f,25.0f, 39.0f,22.0f, 40.8f,19.0f, 42.7f,16.5f, 43.3f,13.0f, 45.0f,12.8f, 48.7f,14.0f, 52.2f,15.6f, 55.0f,17.0f, 57.7f,18.9f, 59.8f,22.5f, 58.6f,23.6f, 56.4f,26.4f, 56.0f,24.0f, 54.0f,24.1f, 51.6f,24.6f, 51.5f,26.0f, 50.1f,26.0f, 48.5f,28.4f, 47.9f,29.9f, 48.5f,30.5f, 45.0f,33.0f, 42.0f,36.0f, 38.0f,36.8f, 36.2f,36.6f, 35.9f,35.4f, 35.0f,33.0f, 34.2f,31.2f };
static const float EURASIA[] = { 26.0f,40.0f, 23.5f,40.2f, 22.9f,36.5f, 21.0f,38.5f, 19.4f,40.4f, 19.4f,42.0f, 15.5f,44.0f, 13.6f,45.7f, 12.3f,44.6f, 13.8f,42.6f, 16.1f,41.4f, 18.5f,40.1f, 16.6f,38.4f, 15.7f,38.0f, 15.8f,39.6f, 14.4f,40.7f, 12.2f,41.8f, 10.5f,42.9f, 8.8f,44.4f, 6.6f,43.1f, 4.0f,43.5f, 3.2f,42.0f, 0.6f,40.6f, -0.4f,39.3f, -2.0f,36.8f, -5.6f,36.0f, -6.4f,36.9f, -8.9f,37.0f, -9.5f,38.8f, -8.8f,42.0f, -9.3f,43.1f, -7.7f,43.7f, -1.8f,43.4f, -1.2f,46.0f, -2.5f,47.3f, -4.6f,48.0f, -1.6f,48.8f, 1.5f,50.0f, 3.4f,51.3f, 4.6f,52.9f, 6.9f,53.5f, 8.6f,53.9f, 8.6f,55.5f, 8.2f,56.9f, 10.5f,57.7f, 10.6f,56.2f, 10.0f,54.8f, 12.0f,54.2f, 14.2f,53.9f, 18.0f,54.8f, 21.1f,55.3f, 21.0f,56.8f, 24.1f,57.1f, 24.3f,58.4f, 23.4f,59.0f, 28.0f,59.5f, 30.2f,59.9f, 26.0f,60.4f, 22.7f,60.0f, 21.4f,61.0f, 21.4f,63.0f, 25.4f,65.0f, 22.2f,65.7f, 19.0f,63.5f, 17.2f,61.4f, 18.6f,60.0f, 16.5f,57.0f, 14.4f,55.6f, 12.7f,56.2f, 11.9f,57.7f, 11.0f,59.0f, 8.0f,58.1f, 5.6f,58.9f, 5.0f,61.5f, 7.0f,62.8f, 10.5f,64.4f, 13.5f,67.8f, 16.0f,69.0f, 20.0f,70.0f, 25.5f,71.0f, 31.0f,70.3f, 36.0f,69.0f, 41.0f,67.4f, 40.3f,65.2f, 44.0f,66.2f, 43.7f,68.4f, 53.0f,68.5f, 61.0f,69.8f, 66.0f,69.8f, 68.5f,68.2f, 73.0f,69.5f, 73.0f,72.5f, 80.0f,73.5f, 87.0f,74.0f, 98.0f,76.0f, 104.0f,77.7f, 113.0f,76.0f, 113.0f,73.6f, 127.0f,73.5f, 140.0f,72.5f, 152.0f,70.9f, 160.0f,70.0f, 170.0f,69.9f, 179.9f,69.0f, 179.9f,65.0f, 178.0f,64.5f, 170.0f,60.0f, 163.0f,59.9f, 162.0f,57.5f, 156.7f,51.0f, 156.0f,57.5f, 160.0f,61.5f, 156.0f,61.6f, 150.0f,59.5f, 142.0f,59.3f, 136.0f,54.5f, 141.0f,52.0f, 140.4f,48.0f, 135.0f,43.5f, 131.0f,42.7f, 129.4f,40.0f, 128.5f,38.4f, 129.4f,36.0f, 127.0f,34.7f, 126.3f,37.5f, 125.0f,39.6f, 121.5f,39.0f, 121.5f,40.8f, 118.0f,39.0f, 119.0f,37.2f, 122.5f,37.3f, 120.7f,36.1f, 119.2f,34.6f, 120.8f,32.4f, 121.9f,30.9f, 121.4f,28.5f, 119.6f,26.0f, 117.0f,23.6f, 113.6f,22.3f, 110.5f,21.2f, 109.0f,21.6f, 106.6f,20.4f, 105.7f,18.9f, 107.0f,16.8f, 109.2f,13.0f, 109.0f,11.5f, 106.5f,9.0f, 105.1f,8.6f, 104.8f,10.3f, 103.0f,11.0f, 101.0f,12.7f, 99.2f,10.0f, 100.4f,7.5f, 101.3f,6.7f, 103.4f,4.5f, 103.5f,1.3f, 101.0f,2.5f, 98.2f,8.0f, 98.7f,10.5f, 97.7f,16.5f, 94.2f,16.0f, 94.4f,19.4f, 92.2f,21.0f, 91.0f,22.6f, 88.5f,21.6f, 86.9f,21.2f, 85.0f,19.3f, 82.3f,17.0f, 80.3f,15.2f, 80.0f,10.3f, 78.0f,8.1f, 76.5f,8.9f, 75.5f,11.8f, 73.6f,15.8f, 72.8f,19.0f, 72.6f,21.1f, 70.0f,21.0f, 68.7f,22.8f, 67.0f,24.8f, 61.6f,25.2f, 57.3f,25.7f, 56.4f,27.1f, 54.0f,26.7f, 51.5f,27.9f, 50.2f,29.9f, 48.5f,30.5f, 45.0f,33.0f, 42.0f,36.0f, 38.0f,36.8f, 36.2f,36.8f, 34.5f,36.8f, 32.5f,36.1f, 30.6f,36.7f, 29.0f,36.6f, 27.4f,37.0f, 26.3f,38.3f, 26.2f,39.5f };
static const float AUSTRALIA[] = { 113.5f,-22.0f, 114.0f,-26.0f, 115.0f,-34.0f, 117.9f,-35.1f, 123.5f,-33.9f, 129.0f,-31.6f, 131.2f,-31.5f, 134.2f,-32.8f, 137.8f,-35.6f, 139.6f,-37.4f, 140.6f,-38.0f, 144.9f,-37.9f, 146.3f,-39.0f, 150.0f,-37.5f, 151.3f,-33.6f, 153.6f,-28.0f, 153.1f,-25.2f, 150.8f,-22.6f, 149.2f,-21.1f, 146.3f,-19.0f, 145.4f,-16.0f, 145.3f,-14.9f, 143.5f,-14.2f, 142.5f,-10.7f, 141.5f,-13.0f, 141.6f,-16.5f, 140.8f,-17.4f, 139.0f,-17.0f, 136.7f,-15.9f, 135.9f,-15.0f, 137.0f,-12.3f, 136.5f,-11.9f, 132.5f,-11.5f, 131.0f,-12.2f, 129.8f,-14.9f, 128.1f,-15.2f, 126.0f,-14.0f, 124.4f,-16.3f, 122.2f,-18.2f, 121.1f,-19.5f, 117.4f,-20.7f, 114.6f,-21.8f };
static const float NAMERICA[] = { -168.0f,65.6f, -162.0f,66.9f, -156.0f,71.3f, -140.0f,69.6f, -128.0f,70.0f, -115.0f,68.5f, -108.0f,68.3f, -95.0f,68.0f, -90.0f,69.0f, -85.0f,69.6f, -81.0f,68.5f, -82.0f,66.0f, -87.0f,64.0f, -92.5f,62.5f, -94.5f,59.0f, -92.0f,57.0f, -87.0f,55.5f, -82.0f,52.9f, -79.5f,51.5f, -79.0f,54.6f, -77.0f,59.5f, -78.5f,62.5f, -74.0f,62.3f, -70.0f,61.0f, -65.0f,60.3f, -61.0f,56.5f, -56.0f,53.5f, -58.0f,51.3f, -60.0f,50.2f, -66.0f,50.2f, -64.5f,49.0f, -64.0f,46.5f, -60.0f,45.5f, -66.0f,44.8f, -70.5f,43.0f, -70.0f,41.7f, -74.0f,40.5f, -75.5f,38.5f, -76.0f,36.5f, -75.5f,35.2f, -79.0f,33.5f, -81.0f,31.5f, -80.5f,28.5f, -80.0f,26.0f, -81.0f,25.2f, -82.6f,27.5f, -83.5f,29.8f, -86.0f,30.4f, -89.5f,30.2f, -89.2f,29.0f, -91.0f,29.4f, -94.0f,29.6f, -97.0f,27.8f, -97.5f,25.0f, -97.7f,21.6f, -96.0f,19.0f, -94.5f,18.2f, -91.0f,18.7f, -90.5f,21.0f, -87.0f,21.5f, -87.6f,18.0f, -88.3f,16.0f, -86.0f,15.9f, -83.4f,15.2f, -83.6f,11.0f, -81.5f,9.0f, -79.5f,9.6f, -77.4f,8.6f, -79.5f,7.2f, -81.0f,7.5f, -83.0f,8.4f, -85.7f,10.8f, -87.5f,13.0f, -91.5f,14.0f, -94.0f,16.0f, -96.5f,15.7f, -101.0f,17.3f, -105.5f,20.0f, -105.6f,22.7f, -109.0f,25.5f, -112.8f,31.5f, -117.0f,32.5f, -120.5f,34.5f, -122.5f,37.5f, -124.0f,40.5f, -124.5f,43.0f, -124.0f,46.2f, -124.7f,48.4f, -123.0f,49.0f, -127.8f,50.9f, -130.5f,54.5f, -133.0f,57.0f, -137.5f,58.5f, -140.0f,59.8f, -146.0f,60.7f, -151.0f,59.5f, -152.0f,61.0f, -154.0f,59.0f, -158.0f,58.5f, -162.0f,55.5f, -157.5f,57.8f, -162.0f,58.7f, -162.8f,60.5f, -165.5f,61.5f, -164.5f,63.2f, -161.0f,64.5f, -166.0f,64.6f };
static const float SAMERICA[] = { -77.4f,8.6f, -75.5f,10.7f, -72.0f,12.0f, -70.0f,12.0f, -67.0f,10.6f, -62.0f,10.7f, -60.0f,8.3f, -57.0f,6.0f, -53.0f,5.5f, -51.0f,4.0f, -50.0f,1.8f, -49.0f,-0.5f, -44.5f,-2.4f, -40.0f,-2.9f, -35.2f,-5.5f, -34.9f,-8.0f, -37.0f,-11.0f, -39.0f,-14.0f, -39.2f,-17.5f, -40.8f,-21.5f, -43.0f,-23.0f, -46.7f,-24.0f, -48.6f,-26.5f, -48.6f,-28.6f, -52.0f,-32.0f, -53.6f,-34.0f, -56.0f,-34.9f, -57.3f,-36.3f, -57.6f,-38.2f, -62.3f,-38.8f, -62.2f,-40.6f, -65.0f,-41.0f, -63.8f,-42.2f, -65.5f,-45.0f, -67.5f,-46.5f, -65.8f,-47.8f, -69.0f,-50.5f, -68.3f,-52.3f, -70.0f,-53.0f, -71.5f,-53.9f, -74.5f,-52.0f, -75.5f,-48.0f, -73.8f,-43.5f, -73.5f,-40.0f, -73.4f,-37.2f, -71.5f,-33.0f, -71.4f,-30.0f, -70.2f,-23.0f, -70.3f,-18.5f, -75.0f,-15.5f, -76.4f,-13.5f, -78.0f,-10.0f, -79.8f,-7.2f, -81.2f,-6.0f, -80.3f,-3.4f, -80.9f,-1.0f, -80.0f,0.9f, -78.9f,1.6f, -77.4f,4.2f, -77.3f,7.0f };
static const float GREENLAND[] = { -73.0f,78.0f, -65.0f,81.5f, -40.0f,83.5f, -20.0f,82.0f, -18.0f,76.5f, -21.0f,72.0f, -22.0f,70.0f, -31.0f,68.0f, -38.0f,65.5f, -43.0f,60.0f, -48.0f,61.0f, -51.0f,64.0f, -53.5f,67.5f, -54.0f,70.5f, -58.0f,75.5f, -66.0f,76.5f };
static const float SMALLISLANDS[] = { 0 };
static const float BORNEO[] = { 109.0f,1.5f, 111.0f,2.5f, 113.0f,3.0f, 116.0f,6.5f, 119.0f,5.0f, 118.0f,1.0f, 117.0f,-1.0f, 116.0f,-3.8f, 113.0f,-3.3f, 110.0f,-2.8f };
static const float SUMATRA[] = { 95.3f,5.5f, 97.5f,5.2f, 100.3f,2.2f, 104.0f,-1.0f, 106.0f,-3.0f, 106.0f,-5.8f, 104.5f,-5.9f, 101.0f,-3.0f, 98.6f,0.0f, 96.0f,3.0f };
static const float JAVA[] = { 105.2f,-6.8f, 106.5f,-6.0f, 108.5f,-6.6f, 111.0f,-6.5f, 114.5f,-7.8f, 114.4f,-8.7f, 111.0f,-8.2f, 108.0f,-7.8f, 106.0f,-7.4f };
static const float NGUINEA[] = { 131.0f,-1.4f, 135.0f,-3.4f, 138.0f,-1.6f, 141.0f,-2.6f, 145.7f,-4.8f, 147.6f,-6.1f, 150.6f,-10.4f, 147.0f,-10.1f, 143.5f,-9.0f, 141.0f,-9.1f, 138.5f,-8.4f, 137.8f,-5.5f, 134.3f,-4.0f, 132.0f,-2.8f };
static const float JAPAN[] = { 130.0f,31.0f, 131.5f,33.5f, 135.0f,34.0f, 140.0f,35.0f, 141.0f,38.0f, 141.5f,41.5f, 140.0f,40.0f, 139.0f,37.0f, 136.0f,36.5f, 133.0f,35.5f, 130.5f,34.0f };
static const float UK[] = { -5.7f,50.0f, 1.4f,51.0f, 1.7f,52.7f, 0.0f,53.5f, -1.6f,55.5f, -2.0f,57.5f, -3.0f,58.6f, -5.0f,58.6f, -6.2f,56.6f, -5.6f,55.3f, -4.6f,54.7f, -3.0f,53.4f, -4.6f,53.2f, -4.2f,51.6f, -5.3f,51.7f };
static const float SRILANKA[] = { 79.8f,6.0f, 81.8f,7.5f, 80.2f,9.8f, 79.7f,8.0f };
static const float NZ_N[] = { 172.7f,-34.4f, 174.5f,-36.0f, 178.5f,-37.7f, 177.0f,-39.3f, 175.3f,-41.6f, 174.6f,-41.3f, 173.9f,-39.3f, 174.6f,-37.0f };
static const float NZ_S[] = { 172.7f,-40.5f, 174.3f,-41.7f, 173.2f,-43.0f, 171.2f,-44.4f, 170.6f,-45.9f, 169.0f,-46.7f, 166.5f,-46.0f, 167.8f,-44.5f, 170.8f,-42.8f };
static const float CASPIAN[] = { 47.0f,44.5f, 50.0f,46.5f, 53.0f,47.0f, 54.0f,45.0f, 51.0f,42.0f, 53.0f,40.0f, 54.0f,37.0f, 51.0f,36.7f, 49.0f,38.0f, 49.5f,40.5f };
static const float BLACKSEA[] = { 28.0f,43.0f, 28.0f,45.0f, 30.0f,46.5f, 33.0f,46.0f, 35.0f,45.0f, 36.5f,45.3f, 38.0f,47.0f, 39.5f,47.0f, 38.0f,45.0f, 41.5f,41.5f, 38.0f,41.0f, 35.0f,42.0f, 31.0f,41.2f, 29.0f,41.2f };
struct Poly { const float* p; int n; };

static void fillPoly(std::vector<float>& mask, int W, int H, const float* p, int n, float value) {
    std::vector<float> xs;
    for (int j = 0; j < H; j++) {
        float lat = (j + 0.5f) / H * 180.f - 90.f;
        xs.clear();
        for (int k = 0; k < n; k++) {
            float x0 = p[2 * k], y0 = p[2 * k + 1], x1 = p[2 * ((k + 1) % n)], y1 = p[2 * ((k + 1) % n) + 1];
            if ((y0 <= lat && y1 > lat) || (y1 <= lat && y0 > lat)) xs.push_back(x0 + (lat - y0) / (y1 - y0) * (x1 - x0));
        }
        std::sort(xs.begin(), xs.end());
        for (size_t k = 0; k + 1 < xs.size(); k += 2) {
            int i0 = (int)std::ceil((xs[k] + 180.f) / 360.f * W - 0.5f), i1 = (int)std::floor((xs[k + 1] + 180.f) / 360.f * W - 0.5f);
            for (int i = std::max(0, i0); i <= std::min(W - 1, i1); i++) mask[(size_t)j * W + i] = value;
        }
    }
}
static void boxBlur(std::vector<float>& a, int W, int H, int r, bool wrapX) {
    std::vector<float> t(a.size());
    for (int pass = 0; pass < 2; pass++) {
        for (int j = 0; j < H; j++) for (int i = 0; i < W; i++) {
            float s = 0; for (int k = -r; k <= r; k++) { int ii = wrapX ? wrapi(i + k, W) : std::min(W - 1, std::max(0, i + k)); s += a[(size_t)j * W + ii]; }
            t[(size_t)j * W + i] = s / (2 * r + 1);
        }
        for (int j = 0; j < H; j++) for (int i = 0; i < W; i++) {
            float s = 0; for (int k = -r; k <= r; k++) { int jj = std::min(H - 1, std::max(0, j + k)); s += t[(size_t)jj * W + i]; }
            a[(size_t)j * W + i] = s / (2 * r + 1);
        }
    }
}
static float boxW(float lon, float lat, float l0, float l1, float b0, float b1, float soft) {
    return sstep(l0 - soft, l0 + soft, lon) * (1.f - sstep(l1 - soft, l1 + soft, lon)) * sstep(b0 - soft, b0 + soft, lat) * (1.f - sstep(b1 - soft, b1 + soft, lat));
}
static void makeEarthTextures(int W, int H, GLuint& colorTex, GLuint& cloudTex) {
    std::vector<float> m((size_t)W * H, 0.f);
    const Poly land[] = { {AFRICA, sizeof(AFRICA) / 8}, {MADAGASCAR, sizeof(MADAGASCAR) / 8}, {ARABIA, sizeof(ARABIA) / 8}, {EURASIA, sizeof(EURASIA) / 8},
        {AUSTRALIA, sizeof(AUSTRALIA) / 8}, {NAMERICA, sizeof(NAMERICA) / 8}, {SAMERICA, sizeof(SAMERICA) / 8}, {GREENLAND, sizeof(GREENLAND) / 8},
        {BORNEO, sizeof(BORNEO) / 8}, {SUMATRA, sizeof(SUMATRA) / 8}, {JAVA, sizeof(JAVA) / 8}, {NGUINEA, sizeof(NGUINEA) / 8}, {JAPAN, sizeof(JAPAN) / 8},
        {UK, sizeof(UK) / 8}, {SRILANKA, sizeof(SRILANKA) / 8}, {NZ_N, sizeof(NZ_N) / 8}, {NZ_S, sizeof(NZ_S) / 8} };
    (void)SMALLISLANDS;
    for (const Poly& p : land) fillPoly(m, W, H, p.p, p.n, 1.f);
    fillPoly(m, W, H, CASPIAN, sizeof(CASPIAN) / 8, 0.f);
    fillPoly(m, W, H, BLACKSEA, sizeof(BLACKSEA) / 8, 0.f);
    for (int j = 0; j < H; j++) {                                        // Antarctica
        float lat = (j + 0.5f) / H * 180.f - 90.f;
        for (int i = 0; i < W; i++) {
            float lon = (i + 0.5f) / W * 360.f - 180.f;
            float edge = -69.f + 3.f * std::sin(lon * 0.05f) + 2.f * std::sin(lon * 0.13f + 1.f);
            if (lon > -75.f && lon < -55.f) edge = std::max(edge, -63.f - 0.3f * (lon + 65.f) * (lon + 65.f) / 10.f);   // peninsula
            if (lat < edge) m[(size_t)j * W + i] = 1.f;
        }
    }
    boxBlur(m, W, H, 2, true);
    std::vector<uint8_t> px((size_t)W * H * 4), cl((size_t)W * H * 4);
    for (int j = 0; j < H; j++) for (int i = 0; i < W; i++) {
        float u = (float)i / W, v = (float)j / H;
        float lon = (i + 0.5f) / W * 360.f - 180.f, lat = (j + 0.5f) / H * 180.f - 90.f, alat = std::fabs(lat);
        float coastN = fbm(u * 64, v * 32, 4, 5, 64) - 0.5f;
        float landv = m[(size_t)j * W + i] + coastN * 0.55f;
        bool isLand = landv > 0.5f;
        float moist = fbm(u * 12, v * 6, 5, 13, 12);
        vec3 col;
        if (isLand) {
            float desert = std::max(std::max(boxW(lon, lat, -17, 33, 15, 31, 4) * 1.f, boxW(lon, lat, 34, 60, 14, 33, 3)),
                std::max(std::max(boxW(lon, lat, 120, 145, -31, -19, 4), boxW(lon, lat, 14, 26, -28, -18, 3) * 0.7f), std::max(boxW(lon, lat, 88, 115, 38, 47, 4) * 0.8f, boxW(lon, lat, 52, 72, 25, 40, 4) * 0.7f)));
            desert = clampf(desert * (0.75f + 0.5f * moist));
            vec3 forest(0.035f, 0.075f, 0.025f), savanna(0.12f, 0.12f, 0.055f), sand(0.50f, 0.38f, 0.22f), boreal(0.04f, 0.06f, 0.035f), tundra(0.14f, 0.13f, 0.11f);
            col = mix(savanna, forest, sstep(0.45f, 0.65f, moist) * (1.f - sstep(8.f, 16.f, alat)));
            col = mix(col, mix(savanna, forest, 0.5f), sstep(30.f, 40.f, alat));
            col = mix(col, boreal, sstep(50.f, 58.f, alat));
            col = mix(col, tundra, sstep(62.f, 70.f, alat));
            col = mix(col, sand * (0.8f + 0.4f * fbm(u * 40, v * 20, 3, 91, 40)), desert);
            float ice = sstep(-65.f, -66.f, lat * 1.f) + sstep(73.f, 76.f, lat) + boxW(lon, lat, -60, -18, 60, 84, 2);
            col = mix(col, vec3(0.72f, 0.74f, 0.78f), clampf(ice + (lat < -60.f ? 1.f : 0.f)));
        }
        else {
            float shelf = clampf((landv - 0.25f) * 4.f);
            col = mix(vec3(0.006f, 0.028f, 0.075f), vec3(0.015f, 0.09f, 0.12f), shelf * shelf);
            if (alat > 72.f) col = mix(col, vec3(0.6f, 0.62f, 0.66f), sstep(72.f, 78.f, alat));          // sea ice
        }
        size_t k = ((size_t)j * W + i) * 4;
        px[k] = enc(col.x); px[k + 1] = enc(col.y); px[k + 2] = enc(col.z); px[k + 3] = b8(isLand ? 0.f : (alat > 74.f ? 0.f : 1.f));
        // clouds: domain-warped fBm, ITCZ band, sub-tropical clear belts, mid-latitude storm tracks
        float wx = fbm(u * 6, v * 3, 4, 201, 6), wy = fbm(u * 6 + 3.3f, v * 3 + 1.7f, 4, 203, 6);
        float c = fbm(u * 10 + (wx - 0.5f) * 2.2f, v * 5 + (wy - 0.5f) * 1.6f, 6, 207, 10);
        float band = 0.10f * std::exp(-(lat - 6.f) * (lat - 6.f) / 60.f) - 0.10f * std::exp(-(alat - 24.f) * (alat - 24.f) / 70.f) + 0.08f * std::exp(-(alat - 52.f) * (alat - 52.f) / 120.f);
        float cov = clampf(c + band);
        cl[k] = b8(cov); cl[k + 1] = cl[k]; cl[k + 2] = cl[k]; cl[k + 3] = 255;
    }
    colorTex = uploadTex(px.data(), W, H, true, true, false);
    cloudTex = uploadTex(cl.data(), W, H, false, true, false);
}

// ---------------------------------------------------------------- the Moon: maria + named craters + random craters
struct Feature { float lon, lat, r; };
static std::vector<float> g_moonAlb; static int g_moonW = 0, g_moonH = 0;   // kept to look up the landing-site albedo
static void makeMoonGlobal(int W, int H, GLuint& albTex, GLuint& nrmTex) {
    static const Feature MARIA[] = { {-16,33,11}, {17,28,6.5f}, {31,8.5f,7.5f}, {59,17,5}, {51,-8,6.5f}, {35,-15,3.5f}, {-17,-21,6.5f}, {-39,-24,4},
        {-57,18,12}, {-50,5,10}, {-62,30,9}, {-31,7,5}, {-23,-10,3.5f}, {4,13,3}, {-20,56,4}, {0,57,4}, {20,56,3.5f}, {38,55,3}, {-40,55,3}, {-68,-5,1.6f}, {-9.3f,51.6f,0.9f} };
    std::vector<float> h((size_t)W * H, 0.f), alb((size_t)W * H, 0.f), bright((size_t)W * H, 0.f);
    auto dirOf = [](float lon, float lat) { float a = lon * PI / 180.f, b = lat * PI / 180.f; return vec3(std::cos(b) * std::cos(a), std::sin(b), std::cos(b) * std::sin(a)); };
    for (int j = 0; j < H; j++) for (int i = 0; i < W; i++) {
        float u = (float)i / W, v = (float)j / H;
        float lon = (i + 0.5f) / W * 360.f - 180.f, lat = (j + 0.5f) / H * 180.f - 90.f;
        vec3 d = dirOf(lon, lat);
        float mare = 0.f;
        for (const Feature& f : MARIA) {
            float ang = std::acos(clampf(dot(d, dirOf(f.lon, f.lat)), -1.f, 1.f)) * 180.f / PI;
            float edge = f.r * (0.85f + 0.3f * fbm(u * 40 + f.lon, v * 20 + f.lat, 3, 301, 0));
            mare = std::max(mare, 1.f - sstep(edge * 0.8f, edge * 1.1f, ang));
        }
        float hl = 0.13f + 0.05f * (fbm(u * 24, v * 12, 5, 311, 24) - 0.5f);
        float mr = 0.065f + 0.02f * (fbm(u * 30, v * 15, 4, 313, 30) - 0.5f);
        alb[(size_t)j * W + i] = lerpf(hl, mr, mare);
        h[(size_t)j * W + i] = (fbm(u * 16, v * 8, 5, 317, 16) - 0.5f) * 3.f - mare * 1.2f;
    }
    // craters (lon,lat,radius deg) : named ones + 1600 random, splatted in lon/lat space with cos(lat) correction
    std::vector<Feature> craters = { {-11.4f,-43.3f,0.85f}, {-20.f,9.6f,0.9f}, {-38.f,8.1f,0.33f}, {-47.5f,23.7f,0.35f}, {-14.f,-58.f,2.4f}, {-9.3f,51.6f,0.95f}, {-68.f,-5.f,1.8f}, {9.f,-25.f,1.6f}, {-3.f,-29.f,2.f}, {26.f,-11.f,1.1f} };
    Rng rng(4242u);
    for (int k = 0; k < 1600; k++) craters.push_back({ rng.r(-180, 180), std::asin(rng.r(-1, 1)) * 180.f / PI, 0.12f + 2.2f * std::pow(rng.f(), 4.f) });
    for (size_t c = 0; c < craters.size(); c++) {
        const Feature& f = craters[c];
        float cl = std::cos(f.lat * PI / 180.f);
        float R = f.r / 180.f * H;                        // pixels (latitude direction)
        int ext = (int)(R * 2.4f) + 1, cy = (int)((f.lat + 90.f) / 180.f * H), cx = (int)((f.lon + 180.f) / 360.f * W);
        int extX = std::min(W / 2, (int)(ext / std::max(cl, 0.05f)) + 1);
        float depth = f.r * 6.f;
        for (int dy = -ext; dy <= ext; dy++) {
            int jj = cy + dy; if (jj < 0 || jj >= H) continue;
            float clj = std::cos(((jj + 0.5f) / H * 180.f - 90.f) * PI / 180.f);
            for (int dx = -extX; dx <= extX; dx++) {
                int ii = wrapi(cx + dx, W);
                float d = std::sqrt((dx * clj) * (dx * clj) + (float)(dy * dy)) / R;
                if (d > 2.4f) continue;
                float bowl = d < 1.f ? -(1.f - d * d) : 0.f, rim = 0.25f * std::exp(-((d - 1.f) / 0.25f) * ((d - 1.f) / 0.25f));
                size_t idx = (size_t)jj * W + ii;
                h[idx] += depth * (bowl + rim);
                bright[idx] = std::max(bright[idx], (c < 4 || c > 9 ? 1.f : 0.3f) * rim * (f.r < 1.f ? 1.f : 0.3f));
            }
        }
    }
    // ray systems of Tycho and Copernicus
    const Feature rays[2] = { {-11.4f,-43.3f,0}, {-20.f,9.6f,0} };
    for (int j = 0; j < H; j++) for (int i = 0; i < W; i++) {
        float lon = (i + 0.5f) / W * 360.f - 180.f, lat = (j + 0.5f) / H * 180.f - 90.f;
        vec3 d = dirOf(lon, lat);
        for (int r = 0; r < 2; r++) {
            vec3 c = dirOf(rays[r].lon, rays[r].lat);
            float ang = std::acos(clampf(dot(d, c), -1.f, 1.f)) * 180.f / PI;
            if (ang > (r == 0 ? 40.f : 18.f) || ang < 0.5f) continue;
            vec3 t = normalize(d - c * dot(d, c)), e1 = normalize(cross(c, vec3(0, 1, 0))), e2 = cross(c, e1);
            float az = std::atan2(dot(t, e2), dot(t, e1));
            float spoke = std::pow(std::fabs(std::sin(az * (r == 0 ? 11.f : 9.f) + std::sin(az * 3.f) * 2.f)), 18.f);
            bright[(size_t)j * W + i] += spoke * 0.6f * (1.f - ang / (r == 0 ? 40.f : 18.f));
        }
    }
    std::vector<uint8_t> pa((size_t)W * H * 4), pn((size_t)W * H * 4);
    g_moonAlb.assign((size_t)W * H, 0.f); g_moonW = W; g_moonH = H;
    const float relief = 0.004f;
    for (int j = 0; j < H; j++) for (int i = 0; i < W; i++) {
        size_t idx = (size_t)j * W + i;
        float a = alb[idx] * (1.f + 0.6f * std::min(bright[idx], 1.2f));
        g_moonAlb[idx] = a;
        pa[idx * 4] = enc(a); pa[idx * 4 + 1] = enc(a * 0.97f); pa[idx * 4 + 2] = enc(a * 0.93f); pa[idx * 4 + 3] = 255;
        float lon = (i + 0.5f) / W * 2 * PI - PI, lat = (j + 0.5f) / H * PI - PI / 2;
        float cl = std::max(std::cos(lat), 0.02f);
        float dhx = (h[(size_t)j * W + wrapi(i + 1, W)] - h[(size_t)j * W + wrapi(i - 1, W)]) / (2.f * 2 * PI / W * cl) * relief;
        float dhy = (h[(size_t)std::min(H - 1, j + 1) * W + i] - h[(size_t)std::max(0, j - 1) * W + i]) / (2.f * PI / H) * relief;
        vec3 n0(std::cos(lat) * std::cos(lon), std::sin(lat), std::cos(lat) * std::sin(lon));
        vec3 east(-std::sin(lon), 0, std::cos(lon)), north(-std::sin(lat) * std::cos(lon), std::cos(lat), -std::sin(lat) * std::sin(lon));
        vec3 n = normalize(n0 - east * dhx - north * dhy);
        pn[idx * 4] = b8(n.x * 0.5f + 0.5f); pn[idx * 4 + 1] = b8(n.y * 0.5f + 0.5f); pn[idx * 4 + 2] = b8(n.z * 0.5f + 0.5f); pn[idx * 4 + 3] = 255;
    }
    albTex = uploadTex(pa.data(), W, H, true, true, false);
    nrmTex = uploadTex(pn.data(), W, H, false, true, false);
}

// ---------------------------------------------------------------- flag of Kenya
static vec3 kenyaColor(float u, float v) {        // u: left->right, v: 0 = top
    const vec3 black(0.01f, 0.01f, 0.01f), white(1, 1, 1), red(0.62f, 0.02f, 0.03f), green(0.0f, 0.30f, 0.12f);
    vec3 c;
    if (v < 0.30f) c = black; else if (v < 0.35f) c = white; else if (v < 0.65f) c = red; else if (v < 0.70f) c = white; else c = green;
    float px = (u - 0.5f) * 1.5f, py = (v - 0.5f);
    for (int s = -1; s <= 1; s += 2) {
        float ang = s * 1.08f, ca = std::cos(ang), sa = std::sin(ang);
        float along = px * ca + py * sa, across = std::fabs(-px * sa + py * ca);
        float taper = 0.010f + 0.020f * sstep(0.30f, 0.20f, std::fabs(along)) * (std::fabs(along) > 0.22f ? 1.f : 0.f);
        if (std::fabs(along) < 0.40f && across < taper) c = white;
    }
    float sx = px / 0.13f, sy = py / 0.235f, e = sx * sx + sy * sy;
    if (e < 1.f) {
        c = (e > 0.80f) ? black : red;
        if (e <= 0.80f) {
            if (std::fabs(sx) < 0.09f) c = white;
            if (std::fabs(sx) < 0.05f && std::fabs(sy) < 0.55f) c = black;
            if (std::fabs(sy) < 0.07f && e < 0.55f) c = white;
            if (std::fabs(sy) < 0.03f && e < 0.55f) c = black;
            float ee = (sx * sx) / 0.30f + (sy * sy) / 0.65f;
            if (ee < 1.f && ee > 0.82f && std::fabs(sx) > 0.12f) c = white;
        }
    }
    return c;
}
static GLuint makeKenyaFlag(int W, int H) {
    std::vector<uint8_t> px((size_t)W * H * 4);
    for (int y = 0; y < H; y++) for (int x = 0; x < W; x++) {
        vec3 acc;
        for (int sy = 0; sy < 3; sy++) for (int sx = 0; sx < 3; sx++)
            acc = acc + kenyaColor((x + (sx + .5f) / 3) / W, 1.f - (y + (sy + .5f) / 3) / H);
        acc = acc / 9.f;
        size_t i = ((size_t)y * W + x) * 4;
        px[i] = enc(acc.x); px[i + 1] = enc(acc.y); px[i + 2] = enc(acc.z); px[i + 3] = 255;
    }
    return uploadTex(px.data(), W, H, true, false, false);
}

// ---------------------------------------------------------------- stainless-steel hull skin (alpha = metal mask)
// u = angle around the hull (u 0.5..1 is the -Z "belly" with black heat-shield tiles), v = height
static void strokeText(std::vector<float>& mask, int W, int H, const char* s, float cx, float cy, float hpx, bool vertical) {
    float sc = hpx / 6.f, adv = 5.5f * sc;
    int n = (int)std::strlen(s);
    float total = n * adv - 1.5f * sc, r = sc * 0.55f;
    for (int k = 0; k < n; k++) {
        float ox = -total / 2 + k * adv;
        forGlyphSegments(s[k], [&](float x0, float y0, float x1, float y1) {
            float ax = ox + x0 * sc, ay = y0 * sc - hpx / 2, bx = ox + x1 * sc, by = y1 * sc - hpx / 2;
            if (vertical) { float t; t = ax; ax = ay; ay = -t; t = bx; bx = by; by = -t; }
            ax += cx; ay += cy; bx += cx; by += cy;
            int x0i = (int)(std::min(ax, bx) - r - 1), x1i = (int)(std::max(ax, bx) + r + 1), y0i = (int)(std::min(ay, by) - r - 1), y1i = (int)(std::max(ay, by) + r + 1);
            for (int y = std::max(0, y0i); y <= std::min(H - 1, y1i); y++) for (int x = std::max(0, x0i); x <= std::min(W - 1, x1i); x++) {
                float px = x + 0.5f - ax, py = y + 0.5f - ay, dx = bx - ax, dy = by - ay;
                float t = clampf((px * dx + py * dy) / std::max(dx * dx + dy * dy, 1e-6f));
                float d = std::sqrt((px - t * dx) * (px - t * dx) + (py - t * dy) * (py - t * dy));
                mask[(size_t)y * W + x] = std::max(mask[(size_t)y * W + x], clampf(r + 0.5f - d));
            }
            });
    }
}
static GLuint makeHullSkin(int W, int H, bool tiles, const char* text, float soot) {
    std::vector<float> txt((size_t)W * H, 0.f);
    if (text) strokeText(txt, W, H, text, 0.25f * W, 0.52f * H, 0.072f * W, true);
    std::vector<uint8_t> px((size_t)W * H * 4);
    for (int y = 0; y < H; y++) for (int x = 0; x < W; x++) {
        float u = (x + 0.5f) / W, v = (y + 0.5f) / H;
        float brushed = 0.9f + 0.1f * fbm(u * 6, v * 140, 3, 401, 0) + 0.05f * (fbm(u * 40, v * 10, 3, 403, 0) - 0.5f);
        float ring = std::fabs(fractf(v * 16.f) - 0.5f);
        float seam = std::fabs(fractf(u * 6.f) - 0.5f);
        float weld = 1.f - 0.35f * sstep(0.488f, 0.497f, ring) - 0.2f * sstep(0.494f, 0.499f, seam);
        vec3 col = vec3(0.58f, 0.58f, 0.60f) * brushed * weld;
        float metal = 1.f;
        if (soot > 0.f) {
            float s = soot * (1.f - sstep(0.0f, 0.35f, v + 0.1f * fbm(u * 18, v * 4, 3, 409, 0)));
            col = mix(col, vec3(0.05f, 0.045f, 0.04f), s); metal *= 1.f - 0.8f * s;
        }
        if (tiles) {
            float edge = 0.5f + 0.006f * std::sin(v * 140.f);
            float belly = sstep(edge - 0.003f, edge + 0.003f, u) * (1.f - sstep(0.997f, 1.0f, u));
            float tu = u * W / 7.f, tv = v * H / 6.f;                  // hex tiles
            float row = std::floor(tv), off = std::fmod(row, 2.f) * 0.5f;
            float cu = fractf(tu + off) - 0.5f, cv = fractf(tv) - 0.5f;
            float hexd = std::max(std::fabs(cu) * 0.87f + std::fabs(cv) * 0.5f, std::fabs(cv));
            float grout = sstep(0.40f, 0.47f, hexd);
            float tileShade = 0.03f + 0.012f * hash2i((int)std::floor(tu + off), (int)row, 77);
            vec3 tc = vec3(tileShade, tileShade, tileShade * 1.05f) + vec3(0.03f, 0.03f, 0.03f) * grout;
            col = mix(col, tc, belly); metal *= 1.f - belly;
        }
        float t = txt[(size_t)y * W + x];
        col = mix(col, vec3(0.02f, 0.02f, 0.022f), t); metal *= 1.f - t;
        size_t i = ((size_t)y * W + x) * 4;
        px[i] = enc(col.x); px[i + 1] = enc(col.y); px[i + 2] = enc(col.z); px[i + 3] = b8(metal);
    }
    return uploadTex(px.data(), W, H, true, true, false);
}
// ---------------------------------------------------------------- truss texture for the launch tower (alpha cut-out)
static GLuint makeTruss(int N) {
    std::vector<uint8_t> px((size_t)N * N * 4);
    for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
        float u = (x + 0.5f) / N, v = (y + 0.5f) / N, w = 0.055f;
        bool on = u < w || u > 1 - w || v < w || v > 1 - w || std::fabs(u - v) < w * 0.7f || std::fabs(u + v - 1) < w * 0.7f;
        float sh = 0.85f + 0.15f * hash2i(x / 8, y / 8, 9);
        size_t i = ((size_t)y * N + x) * 4;
        px[i] = enc(0.23f * sh); px[i + 1] = enc(0.23f * sh); px[i + 2] = enc(0.24f * sh); px[i + 3] = on ? 255 : 0;
    }
    return uploadTex(px.data(), N, N, true, true, true);
}

// ============================================================================
//  9. TERRAIN MESHES
// ============================================================================
static const double R_EARTH = 6371000.0, R_MOON = 1737400.0, EARTH_MOON = 384400000.0;

// --- Moon landing site: craters + rolling ground, flat pad in the middle, edges bend with the Moon's curvature
struct Crater { float x, z, r, depth; };
static std::vector<Crater> g_craters;
static void initCraters() {
    if (!g_craters.empty()) return;
    Rng rng(1234567u);
    for (int i = 0; i < 150; i++) {
        float ang = rng.f() * 2 * PI, rad = 30.f + std::pow(rng.f(), 0.8f) * 1300.f;
        float r = 3.f + std::pow(rng.f(), 3.0f) * 55.f;
        g_craters.push_back({ rad * std::cos(ang), rad * std::sin(ang), r, r * (0.12f + 0.08f * rng.f()) });
    }
}
static float moonTerrainHeight(float x, float z) {
    initCraters();
    float r = std::sqrt(x * x + z * z);
    float flat = sstep(16.f, 50.f, r);
    float h = (vnoise(x * 0.011f, z * 0.011f, 3) - 0.5f) * 5.0f + (vnoise(x * 0.045f, z * 0.045f, 7) - 0.5f) * 1.1f + (vnoise(x * 0.21f, z * 0.21f, 9) - 0.5f) * 0.14f;
    for (const Crater& c : g_craters) {
        float dx = x - c.x, dz = z - c.z, R = c.r * 2.8f, d2 = dx * dx + dz * dz;
        if (d2 > R * R) continue;
        float d = std::sqrt(d2) / c.r;
        float bowl = d < 1.f ? -(1.f - d * d) : 0.f, rim = 0.32f * std::exp(-((d - 1.f) / 0.2f) * ((d - 1.f) / 0.2f)), ej = d > 1.f ? 0.06f * std::exp(-(d - 1.f) * 1.8f) : 0.f;
        h += c.depth * (bowl + rim + ej);
    }
    float edge = 1.f - sstep(1800.f, 2600.f, r);
    return h * flat * edge - r * r / (2.f * (float)R_MOON);
}
// polar grid mesh: dense at the centre, sparse at the rim
template <class HF> static Mesh buildPolarTerrain(float R, int RINGS, int SEGS, float power, HF heightFn, float curvR) {
    std::vector<Vertex> vs; std::vector<unsigned> is;
    vs.reserve((size_t)(RINGS + 1) * (SEGS + 1));
    for (int i = 0; i <= RINGS; i++) {
        float r = R * std::pow((float)i / RINGS, power);
        for (int j = 0; j <= SEGS; j++) {
            float th = 2 * PI * j / SEGS, x = r * std::cos(th), z = r * std::sin(th);
            float e = std::max(0.35f, r * 0.0025f);
            float hx = heightFn(x - e, z) - heightFn(x + e, z), hz = heightFn(x, z - e) - heightFn(x, z + e);
            vec3 n = normalize(vec3(hx, 2 * e, hz));
            (void)curvR;
            Vertex v = { {x, heightFn(x, z), z}, {n.x, n.y, n.z}, {0, 0} };
            vs.push_back(v);
        }
    }
    int row = SEGS + 1;
    for (int i = 0; i < RINGS; i++) for (int j = 0; j < SEGS; j++) {
        unsigned A = i * row + j, B = A + 1, C = A + row, D = C + 1;
        is.insert(is.end(), { A, B, C, B, D, C });
    }
    Mesh m; m.upload(vs, is); return m;
}
// --- Earth launch site: savanna scrub, beach, the Indian Ocean (the shader decides which is which)
static float coastXcpu(float z) { return 640.f + 260.f * std::sin(z / 1900.f + 0.7f) + 120.f * std::sin(z / 610.f + 2.0f) + 35.f * std::sin(z / 170.f) - 0.18f * z; }
static float padGroundHeight(float x, float z) {
    float r = std::sqrt(x * x + z * z);
    float sd = x - coastXcpu(z);
    float land = (vnoise(x * 0.004f, z * 0.004f, 21) - 0.3f) * 9.f + (vnoise(x * 0.03f, z * 0.03f, 23) - 0.5f) * 1.2f;
    land = land * sstep(90.f, 400.f, r) + 1.2f;
    float shore = sstep(0.f, -120.f, sd);                    // 0 at the water line, 1 inland
    float h = std::max(land, 0.6f) * shore;
    h = sd > 0.f ? 0.f : h;
    float edge = 1.f - sstep(5500.f, 7000.f, r);
    return h * edge - r * r / (2.f * (float)R_EARTH);
}

// ============================================================================
//  10. TIMELINE  (seconds of film time)
// ============================================================================
static const float T_END = 300.f;
static const float T_IGN = 12.f, T_LIFT = 15.f, T_HOTSTAGE = 39.5f, T_STAGE = 40.f, T_TLI = 48.f, T_ARRIVE = 90.f, T_D1 = 112.f, T_TOUCH = 118.f;
static const float T_MLIFT = 201.f, T_LASC = 214.f, T_ENTRY = 248.f, T_FLOP = 270.f, T_LANDFLIP = 279.5f, T_GLIDE_END = 281.f, T_CATCH = 288.f;
static const float T_CREDITS = 289.5f;
// chapter marks for the number keys / progress bar
static const float CHAPTER[9] = { 0.f, 15.f, 48.f, 90.f, 118.f, 201.f, 248.f, 279.5f, 289.5f };
static const char* CHAPTER_NAME[9] = { "COUNTDOWN", "LIFTOFF", "TO THE MOON", "LUNAR DESCENT", "MOONWALK", "LIFTOFF FROM THE MOON", "RE-ENTRY", "TOWER CATCH", "CREDITS" };

// vehicle dimensions (metres, ship-local: y = 0 at the aft skirt, nose tip at y = 34)
static const float SHIP_R = 3.4f, SHIP_H = 34.f, BOOST_H = 44.f, MOUNT_H = 16.f, STACK_Y = 61.f, LEG_CLEAR = 2.2f;
static const float DOOR_Y = 21.f, PIN_Y = 26.5f, ARM_H = 82.5f, CATCH_Y = ARM_H - PIN_Y;
static const vec3 TOWER_POS(-17.f, 0.f, 0.f);

// ============================================================================
//  11. THE WORLD: Earth at the origin (Y = north pole), Malindi pad, the Moon, the landing site
// ============================================================================
struct World {
    dvec3 padO; Basis padB;     // pad frame:  x = east, y = up, z = south
    dvec3 moonC; Basis moonB;   // Moon frame: x points at the Earth (near side)
    dvec3 siteO; Basis siteB;   // landing-site frame: y = up
    vec3 sun;                   // unit vector towards the Sun
    vec3 siteAlb;               // Moon albedo at the landing site (so the terrain mesh matches the globe)
} W;

static dvec3 padP(dvec3 l) { return local2world(W.padO, W.padB, l); }
static dvec3 siteP(dvec3 l) { return local2world(W.siteO, W.siteB, l); }
static dvec3 dmul(const Basis& b, dvec3 v) { return dvec3(b.x) * v.x + dvec3(b.y) * v.y + dvec3(b.z) * v.z; }

static void setupWorld() {
    float lat = -3.25f * PI / 180.f, lon = 40.10f * PI / 180.f;                        // Malindi, Kenya
    vec3 up(std::cos(lat) * std::cos(lon), std::sin(lat), -std::cos(lat) * std::sin(lon));
    vec3 east = normalize(vec3(-std::sin(lon), 0.f, -std::cos(lon)));
    vec3 south = cross(east, up);
    W.padB = Basis(east, up, south);
    W.padO = dvec3(up) * R_EARTH;
    // afternoon sun from the south-west, 30 degrees high
    float el = 30.f * PI / 180.f, az = 145.f * PI / 180.f;
    W.sun = normalize(W.padB * vec3(std::cos(el) * std::cos(az), std::sin(el), std::cos(el) * std::sin(az)));
    // the Moon: 40 degrees high in the southern sky, 53 degrees away from the Sun (a daytime crescent)
    vec3 M; float best = 1e9f;
    for (int i = 0; i <= 3600; i++) {
        float a = (float)i / 3600.f * 2 * PI, e2 = 40.f * PI / 180.f;
        vec3 m = normalize(W.padB * vec3(std::cos(e2) * std::cos(a), std::sin(e2), std::cos(e2) * std::sin(a)));
        float err = std::fabs(std::acos(clampf(dot(m, W.sun), -1, 1)) - 53.f * PI / 180.f) + 0.02f * std::fabs(std::sin(a) - 1.f);
        if (err < best) { best = err; M = m; }
    }
    W.moonC = W.padO + dvec3(M) * EARTH_MOON;
    vec3 Em = normalize(tof(dvec3(0, 0, 0) - W.moonC));                                   // Earth as seen from the Moon
    W.moonB = basisYZ(Em, vec3(0, 1, 0));                                                 // temp: y = Earth dir
    W.moonB = Basis(Em, W.moonB.z, cross(Em, W.moonB.z));                                 // x = Earth, y ~ north
    // landing site: rotate the "nice" local lighting (sun behind-right of the camera, Earth over the lander) onto the real sky
    vec3 Ls = normalize(vec3(0.55f, 0.42f, 0.42f)), Le = normalize(vec3(-0.45f, 0.28f, -0.85f));
    vec3 a1 = Ls, a2 = normalize(Le - Ls * dot(Le, Ls)), a3 = cross(a1, a2);
    vec3 b1 = W.sun, b2 = normalize(Em - W.sun * dot(Em, W.sun)), b3 = cross(b1, b2);
    auto R = [&](vec3 v) { return b1 * dot(v, a1) + b2 * dot(v, a2) + b3 * dot(v, a3); };
    W.siteB = Basis(normalize(R(vec3(1, 0, 0))), normalize(R(vec3(0, 1, 0))), normalize(R(vec3(0, 0, 1))));
    W.siteO = W.moonC + dvec3(W.siteB.y) * R_MOON;
}

// ============================================================================
//  12. FLIGHT PATH  (one continuous function of time, in double precision)
// ============================================================================
struct Trajectory {
    dvec3 pA, vA, pB, vB, pC, vC, pE, vE;
    vec3 L1, L2;
    dvec3 D0, v0, D1, v1, D2, A1, vA1, E0, F0, vF, G0, vG, C0;
} TJ;

static dvec3 ascentLocal(double tau) {
    tau = std::max(0.0, tau);
    double t2 = tau * tau, t3 = t2 * tau, t4 = t3 * tau;
    return dvec3(0.12 * t4, STACK_Y + 5 * t2 + 1.5 * t3 + 0.05 * t4, 0.0);
}
static dvec3 ascentVelLocal(double tau) {
    tau = std::max(0.0, tau);
    double t2 = tau * tau, t3 = t2 * tau;
    return dvec3(0.48 * t3, 10 * tau + 4.5 * t2 + 0.2 * t3, 0.0);
}
// re-entry: exponential slow-down from 200 km down-range / 90 km up to 4.5 km over the coast
static const double RE_T = T_FLOP - T_ENTRY, KX = 4.5, KY = 3.0;
static double reK(double s, double k) { return (1.0 - std::exp(-k * s)) / (1.0 - std::exp(-k)); }
static double reKd(double s, double k) { return k * std::exp(-k * s) / (1.0 - std::exp(-k)); }
static dvec3 reentryLocal(double s) {
    dvec3 a = TJ.E0, b = TJ.F0;
    return dvec3(a.x + (b.x - a.x) * reK(s, KX), a.y + (b.y - a.y) * reK(s, KY), a.z + (b.z - a.z) * reK(s, KX));
}
static dvec3 reentryVelLocal(double s) {
    dvec3 a = TJ.E0, b = TJ.F0;
    return dvec3((b.x - a.x) * reKd(s, KX), (b.y - a.y) * reKd(s, KY), (b.z - a.z) * reKd(s, KX)) * (1.0 / RE_T);
}

static void setupTrajectory() {
    TJ.D0 = dvec3(-4500, 12000, -2500); TJ.v0 = dvec3(180, -700, 100);                // lunar descent
    TJ.D1 = dvec3(0, 60 + LEG_CLEAR, 0); TJ.v1 = dvec3(0, -12, 0);
    TJ.D2 = dvec3(0, LEG_CLEAR, 0);
    TJ.A1 = dvec3(-1200, 4000, -2200); TJ.vA1 = dvec3(-260, 700, -470);                  // lunar ascent towards the Earth
    TJ.E0 = dvec3(200000, 90000, 3000); TJ.F0 = dvec3(4000, 4500, 600);                 // re-entry (pad frame)
    TJ.G0 = dvec3(40, 420, 0); TJ.vG = dvec3(0, -60, 0);                                // start of the landing burn
    TJ.C0 = dvec3(0, CATCH_Y, 0);                                                       // caught by the tower arms
    TJ.pA = padP(ascentLocal(T_TLI - T_LIFT)); TJ.vA = dmul(W.padB, ascentVelLocal(T_TLI - T_LIFT));
    TJ.pB = siteP(TJ.D0); TJ.vB = dmul(W.siteB, TJ.v0);
    TJ.pC = siteP(TJ.A1); TJ.vC = dmul(W.siteB, TJ.vA1);
    TJ.pE = padP(TJ.E0); TJ.vE = dmul(W.padB, reentryVelLocal(0.0));
    TJ.vF = reentryVelLocal(1.0);
    TJ.L1 = tof(dnormalize(TJ.pB - TJ.pA)); TJ.L2 = tof(dnormalize(TJ.pE - TJ.pC));
}

static dvec3 shipPos(double t) {
    if (t < T_TLI) return padP(ascentLocal(t - T_LIFT));
    if (t < T_ARRIVE) { double d = T_ARRIVE - T_TLI; return hermite(TJ.pA, TJ.vA * d, TJ.pB, TJ.vB * d, (t - T_TLI) / d); }
    if (t < T_D1) { double d = T_D1 - T_ARRIVE; return siteP(hermite(TJ.D0, TJ.v0 * d, TJ.D1, TJ.v1 * d, (t - T_ARRIVE) / d)); }
    if (t < T_TOUCH) { double d = T_TOUCH - T_D1; return siteP(hermite(TJ.D1, TJ.v1 * d, TJ.D2, dvec3(), (t - T_D1) / d)); }
    if (t < T_MLIFT) return siteP(TJ.D2);
    if (t < T_LASC) { double d = T_LASC - T_MLIFT; return siteP(hermite(TJ.D2, dvec3(), TJ.A1, TJ.vA1 * d, (t - T_MLIFT) / d)); }
    if (t < T_ENTRY) { double d = T_ENTRY - T_LASC; return hermite(TJ.pC, TJ.vC * d, TJ.pE, TJ.vE * d, (t - T_LASC) / d); }
    if (t < T_FLOP) return padP(reentryLocal((t - T_ENTRY) / RE_T));
    if (t < T_GLIDE_END) { double d = T_GLIDE_END - T_FLOP; return padP(hermite(TJ.F0, TJ.vF * d, TJ.G0, TJ.vG * d, (t - T_FLOP) / d)); }
    if (t < T_CATCH) { double d = T_CATCH - T_GLIDE_END; return padP(hermite(TJ.G0, TJ.vG * d, TJ.C0, dvec3(), (t - T_GLIDE_END) / d)); }
    double k = t - T_CATCH;
    double settle = -0.4 * (1.0 - std::exp(-k * 3.0)) - 0.12 * std::exp(-k * 1.6) * std::sin(k * 7.0);
    return padP(TJ.C0 + dvec3(0, settle, 0));
}
static dvec3 shipVel(double t) { const double h = 0.02; return (shipPos(t + h) - shipPos(t - h)) * (0.5 / h); }

// distance below the ship the engines are pointing at the ground (for dust) etc.
static Basis shipBasis(float t) {
    vec3 U = W.padB.y, E = W.padB.x;
    Basis Bs = basisYZ(W.siteB.y, W.siteB.z);
    Basis Bt1 = basisYZ(TJ.L1, W.padB.z), Bt1r = basisYZ(-TJ.L1, W.padB.z);
    Basis Bt2 = basisYZ(TJ.L2, W.siteB.z), Bt2r = basisYZ(-TJ.L2, W.siteB.z);
    float a20 = 0.30f;
    Basis Bflop = basisYZ(normalize(-E * std::cos(a20) + U * std::sin(a20)), U);
    Basis Bcatch = basisYZ(U, E);
    auto asc = [&](float tt) { vec3 v = W.padB * tof(ascentVelLocal(tt - T_LIFT)); return basisYZ(normalize(v + U * 15.f), W.padB.z); };
    if (t < 47.f) return asc(t);
    if (t < 53.f) return slerpB(asc(t), Bt1, sstep(47.f, 53.f, t));
    if (t < 66.5f) return Bt1;
    if (t < 71.5f) return slerpB(Bt1, Bt1r, sstep(66.5f, 71.5f, t));
    if (t < 84.f) return Bt1r;
    if (t < 100.f) return slerpB(Bt1r, Bs, sstep(84.f, 100.f, t));
    if (t < 203.f) {
        float w = (t > 100.f && t < T_TOUCH) ? (1.f - sstep(108.f, T_TOUCH, t)) * 0.04f : 0.f;     // small hover corrections
        return w > 0.f ? rotAxis(Bs, Bs.x, w * std::sin(t * 1.3f)) : Bs;
    }
    if (t < 214.f) return slerpB(Bs, Bt2, sstep(203.f, 214.f, t));
    if (t < 229.5f) return Bt2;
    if (t < 234.5f) return slerpB(Bt2, Bt2r, sstep(229.5f, 234.5f, t));
    if (t < 244.f) return Bt2r;
    if (t < 252.f) return slerpB(Bt2r, Bflop, sstep(244.f, 252.f, t));
    if (t < T_LANDFLIP) return rotAxis(Bflop, Bflop.y, 0.03f * std::sin(t * 0.9f));
    if (t < 282.f) return slerpB(Bflop, Bcatch, sstep(T_LANDFLIP, 282.f, t));
    return Bcatch;
}

// engine throttle 0..1 for the ship and for the booster; legs, flaps, door, elevator
struct ShipState {
    float ship = 0, booster = 0, legs = 0, flapF = 0, flapA = 0, door = 0, boom = 0, platY = 0, hotStage = 0;
    bool stacked = false, vacuum = true;
};
static float ramp(float t, float a, float b, float c, float d) { return sstep(a, b, t) * (1.f - sstep(c, d, t)); }
static float elevatorY(float t) {
    const float TOP = LEG_CLEAR + DOOR_Y - 1.25f, GND = 0.05f;
    if (t < 130.f) return TOP;
    if (t < 140.f) return lerpf(TOP, GND, sstep(130.f, 140.f, t));
    if (t < 186.f) return GND;
    if (t < 196.f) return lerpf(GND, TOP, sstep(186.f, 196.f, t));
    return TOP;
}
static float elevatorZ(float t) { return SHIP_R + lerpf(-1.2f, 1.45f, sstep(126.f, 128.f, t) - sstep(197.f, 198.5f, t)); }
static ShipState shipState(float t) {
    ShipState s;
    s.stacked = t < T_STAGE;
    float flick = 0.92f + 0.08f * std::sin(t * 37.f) * std::sin(t * 11.f);
    s.booster = (t < T_STAGE) ? sstep(T_IGN, T_LIFT - 0.5f, t) * flick : 0.f;
    s.ship = ramp(t, T_HOTSTAGE, T_HOTSTAGE + 0.4f, 66.5f, 67.2f) + ramp(t, 71.2f, 71.8f, T_TOUCH - 0.3f, T_TOUCH)
        + ramp(t, T_MLIFT - 1.2f, T_MLIFT, 229.f, 229.7f) + ramp(t, 234.2f, 234.9f, 244.f, 245.f) + ramp(t, 279.6f, 280.f, T_CATCH - 0.6f, T_CATCH + 0.2f);
    if (t > 90.f && t < T_TOUCH) s.ship *= lerpf(0.55f, 1.f, sstep(T_D1 - 2.f, T_D1 + 3.f, t)) * (1.f - 0.45f * sstep(T_D1 + 3.f, T_TOUCH, t));
    s.ship *= flick;
    s.hotStage = ramp(t, T_HOTSTAGE, T_HOTSTAGE + 0.2f, T_STAGE + 0.3f, T_STAGE + 1.5f);
    s.legs = sstep(100.f, 106.f, t) - sstep(206.f, 212.f, t);
    s.flapF = s.flapA = 0.85f;                                                     // folded (radians towards the belly)
    float flop = ramp(t, 246.f, 250.f, T_LANDFLIP + 0.5f, 283.f);
    s.flapF = lerpf(0.85f, 0.12f + 0.10f * std::sin(t * 2.1f), flop);
    s.flapA = lerpf(0.85f, 0.05f + 0.08f * std::sin(t * 1.7f + 1.f), flop);
    s.door = ramp(t, 124.f, 126.5f, 197.5f, 200.f) + sstep(290.5f, 292.5f, t);
    s.boom = sstep(126.f, 128.f, t) - sstep(197.f, 198.5f, t);
    s.platY = elevatorY(t);
    vec3 shipP = tof(shipPos(t) - W.padO);
    s.vacuum = length(shipP) > 60000.f || (t > 60.f && t < 260.f);
    return s;
}

// booster after hot staging: drops behind, flips for its boost-back burn
struct BoosterState { bool vis; dvec3 pos; Basis B; float plume; };
static BoosterState boosterState(float t) {
    BoosterState b; b.vis = t < T_STAGE + 14.f; b.plume = 0;
    dvec3 base = shipPos(t);
    if (t < T_STAGE) { b.B = shipBasis(t); b.pos = base - dvec3(b.B.y) * (double)(BOOST_H + 1.f); b.plume = shipState(t).booster; return b; }
    float d = t - T_STAGE;
    double sep = 0.8 * d * d + 0.6 * d * d * d;
    Basis B = shipBasis(t);
    b.B = rotAxis(B, B.z, -sstep(1.5f, 9.f, d) * PI * 0.92f);
    b.pos = base - dvec3(B.y) * (BOOST_H + 1.0 + sep) + dvec3(B.x) * (double)(d * d * 1.2f);
    b.plume = ramp(d, 6.5f, 7.5f, 30.f, 31.f) * 0.7f;
    return b;
}

// ============================================================================
//  13. MOONWALK CHOREOGRAPHY  (all positions in landing-site coordinates)
// ============================================================================
struct AstroPose {
    bool visible = false; vec3 pos; float heading = 0, walk = 0, phase = 0, lean = 0, hop = 0, crouch = 0;
    float raiseL = 0, raiseR = 0, elbowL = 0, elbowR = 0, spread = 0;
    vec3 poleBase, poleTop; float unfurl = 0; bool poleVisible = true;
};
static const vec3 G1(0.f, 0.f, 7.4f), PL(9.5f, 0.f, 14.5f), BOULDER(17.f, 0.f, 6.f), ALIEN_HIDE(17.8f, 0.f, 4.4f), LEG_HIDE(4.0f, 0.f, 3.7f);
static const vec3 CAM_FLAG(12.6f, 1.9f, 24.6f);
static const float POLE_LEN = 3.5f, HOP_T = 2.53f, HOP_H = 1.3f;
static vec3 rotYv(vec3 v, float a) { float c = std::cos(a), s = std::sin(a); return vec3(c * v.x + s * v.z, v.y, -s * v.x + c * v.z); }
static const vec3 CARRY_LOCAL(0.31f, 0.22f, 0.49f);
static float heroHeading() { return std::atan2(CAM_FLAG.x - PL.x, CAM_FLAG.z - PL.z); }
static float angLerp(float a, float b, float t) { float d = std::fmod(b - a + 3.f * PI, 2.f * PI) - PI; return a + d * t; }

static AstroPose astroAt(float t) {
    AstroPose a;
    const float H = heroHeading(), Hwalk = std::atan2(PL.x - G1.x, PL.z - G1.z);
    float zp = elevatorZ(t), py = elevatorY(t);
    const vec3 PEL(0.f, 0.f, zp + 1.0f);
    const float Hback = std::atan2(PEL.x - PL.x, PEL.z - PL.z), Hboulder = std::atan2(BOULDER.x - PL.x, BOULDER.z - PL.z);
    vec3 stowBase(0.72f, py + 0.12f, zp + 0.55f), stowTop = stowBase + vec3(0.f, POLE_LEN, 0.f);
    vec3 flagBase = PL + rotYv(CARRY_LOCAL, H); flagBase.y = -0.35f;
    a.poleBase = stowBase; a.poleTop = stowTop;
    if (t < 127.5f || t > 197.8f) {
        a.visible = false;
        if (t > 197.8f) { a.poleBase = flagBase; a.poleTop = flagBase + vec3(0, POLE_LEN, 0); a.unfurl = 1.f; a.poleVisible = true; }
        else a.poleVisible = t >= 126.f;
        return a;
    }
    a.visible = true;
    if (t < 130.f) {                                                        // steps out of the airlock onto the platform
        a.pos = vec3(0.f, py, lerpf(SHIP_R - 0.7f, zp, sstep(127.5f, 129.6f, t))); a.walk = 0.4f * (1.f - sstep(129.f, 130.f, t)); a.phase = t * 3.f;
        a.raiseR = 0.5f;
    }
    else if (t < 140.f) {                                                 // rides the elevator down, holding the rail
        a.pos = vec3(0.f, py, zp); a.raiseR = 0.55f; a.raiseL = 0.15f + 0.1f * std::sin(t * 0.8f);
        if (t > 138.5f) { a.raiseR = 0.55f + 0.8f * sstep(138.5f, 139.5f, t); a.elbowR = 0.2f; }
    }
    else if (t < 142.f) {                                                 // steps off onto the Moon
        float s = sstep(140.f, 142.f, t);
        a.pos = mix(vec3(0.f, 0.f, zp), G1, s); a.pos.y = 0.f; a.walk = 0.5f; a.phase = t * 3.2f; a.raiseR = 1.0f;
    }
    else if (t < 143.5f) {
        a.pos = G1; a.heading = angLerp(0.f, Hwalk, sstep(142.f, 143.5f, t)); a.raiseR = 1.0f;
    }
    else if (t < 152.f) {                                                 // the walk to the flag site
        float s = (t - 143.5f) / 8.5f, len = length(PL - G1);
        a.pos = mix(G1, PL, s); a.heading = Hwalk; a.walk = 1.f; a.phase = s * len / 0.9f * PI; a.lean = 0.10f; a.raiseR = 1.0f;
    }
    else if (t < 153.5f) {
        a.pos = PL; a.heading = angLerp(Hwalk, H, sstep(152.f, 153.5f, t)); a.raiseR = 1.0f;
    }
    else if (t < 171.f) {
        a.pos = PL; a.heading = H;
        if (t < 159.f) {                                                    // hammer the pole into the regolith
            float p = (t - 153.5f) / 5.5f, bob = std::fabs(std::sin(p * 5.f * PI));
            a.raiseR = 1.0f + 0.55f * bob * (1.f - p * 0.5f); a.raiseL = 0.9f; a.spread = -0.12f; a.lean = 0.14f * bob;
        }
        else if (t < 162.f) {
            a.raiseR = lerpf(1.0f, 0.3f, sstep(159.f, 160.f, t)); a.raiseL = 0.9f * (1.f - sstep(159.f, 160.f, t));
        }
        else {
            float ht = t - 162.f, tot = 3 * HOP_T;
            if (ht < tot) {                                                 // cheer: ballistic hops in 1/6 g
                float f = fractf(ht / HOP_T);
                a.hop = HOP_H * 4.f * f * (1.f - f); a.crouch = 0.55f * (1.f - sstep(0.f, 0.12f, f));
                a.raiseL = a.raiseR = 2.7f; a.spread = 0.5f; a.elbowL = a.elbowR = 0.2f;
            }
            else { float k = 1.f - sstep(tot, tot + 1.2f, ht); a.raiseL = a.raiseR = 2.7f * k; a.spread = 0.5f * k; }
        }
    }
    else if (t < 174.5f) {                                                // "...did something move?"
        a.pos = PL;
        a.heading = t < 172.5f ? angLerp(H, Hboulder, sstep(171.f, 172.f, t)) : angLerp(Hboulder, Hback, sstep(173.5f, 174.5f, t));
        float shrug = ramp(t, 172.5f, 172.9f, 173.4f, 173.8f);
        a.raiseL = a.raiseR = shrug * 0.9f; a.spread = shrug * 0.6f; a.elbowL = a.elbowR = shrug * 1.2f;
    }
    else if (t < 183.f) {                                                 // walks back (and never sees the alien)
        float s = (t - 174.5f) / 8.5f, len = length(PEL - PL);
        a.pos = mix(PL, PEL, s); a.heading = Hback; a.walk = 1.f; a.phase = s * len / 0.9f * PI; a.lean = 0.08f;
    }
    else if (t < 186.f) {                                                 // steps on, turns around
        a.pos = mix(PEL, vec3(0.f, 0.f, zp), sstep(183.f, 184.2f, t)); a.pos.y = py;
        a.heading = angLerp(Hback, 0.f, sstep(184.f, 185.8f, t)); a.walk = 0.4f * (1.f - sstep(184.f, 185.f, t)); a.phase = t * 3.f;
        a.raiseR = 0.55f * sstep(185.f, 185.8f, t);
    }
    else if (t < 196.f) {
        a.pos = vec3(0.f, py, zp); a.raiseR = 0.55f; a.raiseL = 0.3f + 1.6f * ramp(t, 190.f, 190.6f, 192.5f, 193.2f);   // waves goodbye
        a.elbowL = 0.4f * ramp(t, 190.f, 190.6f, 192.5f, 193.2f) * (1.f + std::sin(t * 9.f));
    }
    else {                                                                // back into the airlock
        a.pos = vec3(0.f, py, lerpf(zp, SHIP_R - 0.8f, sstep(196.f, 197.8f, t))); a.heading = PI; a.walk = 0.5f; a.phase = t * 3.f;
    }
    // ---- the flag pole: stowed on the platform rail, carried, then planted
    vec3 carryBase = a.pos + rotYv(CARRY_LOCAL, a.heading);
    if (t < 139.5f) { a.poleBase = stowBase; a.poleTop = stowTop; }
    else if (t < 140.2f) {
        float s = sstep(139.5f, 140.2f, t);
        vec3 b = mix(stowBase, carryBase, s); a.poleBase = b; a.poleTop = b + vec3(0, POLE_LEN, 0);
    }
    else if (t < 153.5f) { a.poleBase = carryBase; a.poleTop = carryBase + vec3(0, POLE_LEN, 0); }
    else if (t < 159.f) {
        float p = (t - 153.5f) / 5.5f, bob = std::fabs(std::sin(p * 5.f * PI)), depth = sstep(0.08f, 0.95f, p);
        vec3 b = carryBase; b.y = lerpf(0.22f, -0.35f, depth) + 0.26f * bob * (1.f - depth);
        a.poleBase = b; a.poleTop = b + vec3(0, POLE_LEN, 0);
    }
    else { a.poleBase = flagBase; a.poleTop = flagBase + vec3(0, POLE_LEN, 0); }
    a.unfurl = sstep(159.f, 162.f, t);
    return a;
}

// pivot = true: 'pos' is where the hands grip, and the body hangs from that point (rotated by 'lean')
struct AlienPose { bool vis = false; vec3 pos; float heading = 0, armUp = 0, armUp2 = 0, armBack = 0, lean = 0; bool moving = false, hanging = false, flag = false, pivot = false; };
// centre point between the alien's two hands, in its own coordinates (feet at 0,0,0)
static vec3 alienHands(float armUp, float armBack) {
    float ar = (20.f + armUp * 150.f) * PI / 180.f, c = std::cos(armBack);
    return vec3(0.f, 0.95f - std::cos(ar) * 0.6f * c, -0.6f * std::sin(armBack));
}
static const float HANG_UP = 0.87f, HANG_BACK = 0.6f;    // arms in a "V", reaching back over the platform lip
static AlienPose alienAt(float t) {
    AlienPose a;
    if (t < 165.f || t > 197.6f) return a;
    a.vis = true;
    float zp = elevatorZ(t), py = elevatorY(t);
    float hToCam = std::atan2(CAM_FLAG.x - ALIEN_HIDE.x, CAM_FLAG.z - ALIEN_HIDE.z);
    a.heading = hToCam;
    if (t < 167.f) a.pos = vec3(ALIEN_HIDE.x, -1.6f * (1.f - sstep(165.f, 167.f, t)), ALIEN_HIDE.z);
    else if (t < 175.f) {
        float y = (t < 171.2f) ? 0.05f * std::sin((t - 167.f) * 4.f) : -1.6f * ramp(t, 171.2f, 171.9f, 174.2f, 175.f);
        a.pos = vec3(ALIEN_HIDE.x, y, ALIEN_HIDE.z);
        a.armUp = 0.85f * ramp(t, 167.4f, 168.2f, 170.4f, 171.2f);
    }
    else if (t < 181.5f) {                                                // sneaky dash to a landing leg
        float s = sstep(175.6f, 181.5f, t);
        a.pos = mix(ALIEN_HIDE, LEG_HIDE, s);
        a.pos.y = (t < 175.6f ? -1.6f * (1.f - sstep(175.f, 175.6f, t)) : 0.f) + 0.35f * std::fabs(std::sin((t - 175.f) * 7.f)) * (1.f - sstep(181.f, 181.5f, t));
        a.heading = std::atan2(LEG_HIDE.x - ALIEN_HIDE.x, LEG_HIDE.z - ALIEN_HIDE.z); a.moving = true;
    }
    else if (t < 185.6f) {                                                // crouches behind the leg
        a.pos = LEG_HIDE; a.pos.y = -0.25f; a.heading = std::atan2(0.f - LEG_HIDE.x, zp - LEG_HIDE.z) + 0.6f * std::sin(t * 2.f);
    }
    else if (t < 186.4f) {                                                // leaps for the lip of the platform!
        float s = sstep(185.6f, 186.4f, t);
        vec3 h0 = alienHands(0.f, 0.f);
        vec3 from(LEG_HIDE.x, -0.25f + h0.y, LEG_HIDE.z), grip(0.f, py + 0.10f, zp + 0.93f);
        a.pivot = true;
        a.pos = mix(from, grip, s); a.pos.y += 1.0f * 4.f * s * (1.f - s);
        a.armUp = a.armUp2 = HANG_UP * s; a.armBack = HANG_BACK * s;
        a.heading = angLerp(std::atan2(0.f - LEG_HIDE.x, zp - LEG_HIDE.z), 0.f, s);
    }
    else if (t < 196.2f) {
        // hands hooked over the open front edge of the platform, body hanging OUTSIDE it, pressed against the edge.
        // It swings gently like a pendulum in the Moon's weak gravity (period ~5 s) and the swing dies away.
        float k = t - 186.4f;
        a.pivot = true; a.hanging = true;
        a.pos = vec3(0.f, py + 0.10f, zp + 0.93f);
        a.armUp = a.armUp2 = HANG_UP; a.armBack = HANG_BACK;
        a.lean = 0.12f * std::exp(-k / 3.5f) * std::sin(k * 2.f * PI / 4.9f);
        a.heading = 0.05f * std::sin(t * 1.1f);
    }
    else {                                                                // ...climbs up and slips into the airlock behind the astronaut
        float s = sstep(196.2f, 197.6f, t);
        a.pivot = true;
        a.armUp = a.armUp2 = HANG_UP * (1.f - s); a.armBack = HANG_BACK * (1.f - s);
        vec3 h = alienHands(a.armUp, a.armBack);
        a.pos = mix(vec3(0.f, py + 0.10f, zp + 0.93f), vec3(0.f, py + h.y, SHIP_R - 0.5f), s);
        a.heading = angLerp(0.f, PI, s);
    }
    return a;
}

// ============================================================================
//  14. CAMERA - one continuous path. Shots blend into each other (no cuts).
// ============================================================================
struct Cam { dvec3 eye, target; vec3 up; float fov; };
static Cam camShip(float t, vec3 offWorld, vec3 tgtWorld, vec3 up, float fov, float centreY = 17.f) {
    Cam c; dvec3 p = shipPos(t); Basis B = shipBasis(t);
    dvec3 ctr = p + dvec3(B.y) * (double)centreY;
    c.eye = ctr + dvec3(offWorld); c.target = ctr + dvec3(tgtWorld); c.up = up; c.fov = fov; return c;
}
static Cam camPadFixed(vec3 eyeL, dvec3 targetW, float fov) {
    Cam c; c.eye = padP(dvec3(eyeL)); c.target = targetW; c.up = W.padB.y; c.fov = fov; return c;
}
static Cam camSiteFixed(vec3 eyeL, vec3 tgtL, float fov) {
    Cam c; c.eye = siteP(dvec3(eyeL)); c.target = siteP(dvec3(tgtL)); c.up = W.siteB.y; c.fov = fov; return c;
}
static vec3 shake(float t, float amp) {
    return vec3(std::sin(t * 41.f) + 0.5f * std::sin(t * 97.f), std::sin(t * 53.f + 1.f) + 0.5f * std::sin(t * 83.f), std::sin(t * 61.f + 2.f)) * amp;
}
static Cam shotAt(int k, float t) {
    const Basis& P = W.padB; const Basis& S = W.siteB;
    switch (k) {
    case 0: {                                                    // the launch site, slowly circling the stack
        float a = lerpf(115.f, 92.f, sstep(0.f, 16.f, t)) * PI / 180.f, r = lerpf(230.f, 200.f, sstep(0.f, 16.f, t));
        vec3 e(r * std::cos(a), lerpf(5.f, 9.f, sstep(0.f, 16.f, t)), r * std::sin(a));
        float sh = (t > T_IGN) ? 0.12f * sstep(T_IGN, T_LIFT, t) * (1.f - sstep(T_LIFT + 6.f, T_LIFT + 14.f, t)) : 0.f;
        dvec3 tgt = padP(dvec3(0, lerpf(48.f, 62.f, sstep(5.f, 14.f, t)), 0));
        dvec3 shipC = shipPos(t) + dvec3(shipBasis(t).y) * 10.0;
        tgt = dmix(tgt, shipC, sstep(T_LIFT + 0.5f, T_LIFT + 5.f, t));
        Cam c = camPadFixed(e + shake(t, sh), tgt, lerpf(40.f, 34.f, sstep(T_LIFT, T_LIFT + 8.f, t))); return c;
    }
    case 1: {                                                    // drone chase, below and beside the climbing ship
        vec3 off = P * mix(vec3(-55.f, -18.f, 125.f), vec3(-80.f, 30.f, 95.f), sstep(30.f, 44.f, t));
        return camShip(t, off + shake(t, 0.05f), P * vec3(0, -6.f, 0), P.y, 40.f, 0.f);
    }
    case 2: {                                                    // trans-lunar coast: ahead of the ship, Earth behind it
        Basis T1 = basisYZ(TJ.L1, P.z);
        return camShip(t, T1 * vec3(14.f, 75.f, 26.f), T1 * vec3(0, -30.f, 0), T1.z, 46.f);
    }
    case 3: {                                                    // side-on for the flip
        Basis T1 = basisYZ(TJ.L1, P.z);
        return camShip(t, T1 * vec3(72.f, 6.f, 24.f), T1 * vec3(0, 0, 0), T1.z, 40.f);
    }
    case 4: {                                                    // behind the ship, the Moon swelling ahead
        Basis T1 = basisYZ(TJ.L1, P.z);
        return camShip(t, T1 * vec3(16.f, -62.f, 22.f), T1 * vec3(0, 45.f, 0), T1.z, 46.f);
    }
    case 5: {                                                    // descent: chase from the side, high
        return camShip(t, S * vec3(55.f, 18.f, 80.f), S * vec3(0, -4.f, 0), S.y, 42.f);
    }
    case 6: {                                                    // ...settling on the surface as the ship touches down
        Cam c = camSiteFixed(vec3(38.f, 7.f, 52.f), vec3(0, 14.f, 0), 44.f);
        c.target = dmix(shipPos(t) + dvec3(S.y) * 12.0, c.target, sstep(112.f, T_TOUCH + 2.f, t));
        return c;
    }
    case 7: return camSiteFixed(vec3(7.f, 6.f, 30.f), vec3(0.f, 18.f, 4.f), 42.f);           // up at the airlock
    case 8: {                                                    // follow the elevator down
        float py = elevatorY(t), zp = elevatorZ(t);
        return camSiteFixed(vec3(13.f, 3.5f + py * 0.32f, 25.f), vec3(0.f, py + 1.3f, zp), 40.f);
    }
    case 9: {                                                    // tracking the walk to the flag site
        AstroPose a = astroAt(t);
        vec3 d = normalize(PL - G1), perp(-d.z, 0, d.x);
        vec3 tg = (a.visible ? a.pos : G1) + vec3(0, 1.1f, 0) + d * 0.8f;
        return camSiteFixed(tg + perp * 8.5f + vec3(0, 0.45f, 0) - d * 1.2f, tg, 42.f);
    }
    case 10: {                                                   // flag planting close-up (the boulder is at the right edge...)
        float u = sstep(152.f, 171.f, t);
        return camSiteFixed(mix(vec3(13.4f, 1.8f, 25.8f), CAM_FLAG, u), vec3(9.4f, 1.75f, 14.3f), 44.f);
    }
    case 11: return camSiteFixed(vec3(23.f, 4.6f, 30.f), vec3(6.f, 2.0f, 7.f), 46.f);         // wide: the alien sneaks
    case 12: {                                                   // tilt up with the elevator
        float py = elevatorY(t);
        return camSiteFixed(vec3(21.f, 5.f, 33.f), vec3(0.f, py * 0.85f + 1.5f, 4.5f), 44.f);
    }
    case 13: {                                                   // liftoff from the Moon, from the ground
        Cam c = camSiteFixed(vec3(36.f, 4.f, 50.f), vec3(0, 16.f, 0), 44.f);
        c.target = shipPos(t) + dvec3(S.y) * 15.0;
        return c;
    }
    case 14: {                                                   // chase climbing away, Earth in the black sky
        return camShip(t, S * vec3(42.f, -6.f, 64.f), S * vec3(-6.f, 2.f, -10.f), S.y, 42.f);
    }
    case 15: {                                                   // close to the crew windows... who is that?
        Basis B = shipBasis(t);
        Cam c; dvec3 p = shipPos(t);
        c.eye = p + dvec3(B * vec3(1.6f, 23.9f, 9.5f)); c.target = p + dvec3(B * vec3(0.f, 23.4f, SHIP_R)); c.up = B.y; c.fov = 36.f; return c;
    }
    case 16: {                                                   // the flip on the way home
        Basis T2 = basisYZ(TJ.L2, S.z);
        return camShip(t, T2 * vec3(70.f, 4.f, 26.f), vec3(), T2.z, 40.f);
    }
    case 17: {                                                   // behind, the Earth growing ahead
        Basis T2 = basisYZ(TJ.L2, S.z);
        return camShip(t, T2 * vec3(14.f, -60.f, 24.f), T2 * vec3(0, 50.f, 0), T2.z, 46.f);
    }
    case 18: {                                                   // re-entry: chase above the plasma, ocean below
        return camShip(t, P * vec3(62.f, 20.f, 48.f), P * vec3(-30.f, -14.f, 0.f), P.y, 46.f, 0.f);
    }
    case 19: {                                                   // closer during the belly-flop fall
        return camShip(t, P * vec3(45.f, 16.f, 62.f), P * vec3(-6.f, -8.f, 0.f), P.y, 44.f, 0.f);
    }
    case 20: {                                                   // drone hovering near the tower for the catch
        Cam c = camPadFixed(vec3(150.f, 66.f, 190.f), padP(dvec3(-4, 62, 0)), 40.f);
        dvec3 sc = shipPos(t) + dvec3(shipBasis(t).y) * 17.0;
        c.target = dmix(sc, c.target, sstep(286.f, 291.f, t));
        return c;
    }
    case 21: {                                                   // push in: the hatch opens...
        return camPadFixed(vec3(34.f, CATCH_Y + DOOR_Y + 2.f, 26.f), padP(dvec3(SHIP_R + 0.3f, CATCH_Y + DOOR_Y + 0.6f, 0.4f)), 40.f);
    }
    default: {                                                   // final pull-back for the credits
        float u = sstep(296.f, T_END, t);
        return camPadFixed(mix(vec3(120.f, 85.f, 150.f), vec3(170.f, 95.f, 210.f), u), padP(dvec3(-6, 64 + 6 * u, 0)), 42.f);
    }
    }
}
// shot k becomes active over [start, end] (blending from the previous one); windows never overlap
struct ShotWin { float a, b; };
static const ShotWin SHOTS[] = {
    {0, 0}, {16.f, 27.f}, {46.f, 54.f}, {56.f, 63.f}, {73.f, 80.f}, {84.f, 92.f}, {104.f, 114.f}, {121.f, 126.f}, {127.f, 131.f},
    {139.f, 142.5f}, {150.f, 153.5f}, {171.f, 174.5f}, {184.f, 187.f}, {197.f, 200.f}, {204.f, 209.f}, {213.f, 217.f}, {224.f, 228.f},
    {235.f, 240.f}, {244.f, 251.f}, {264.f, 269.f}, {274.f, 283.f}, {289.f, 291.5f}, {294.5f, 298.5f} };
static const int N_SHOTS = sizeof(SHOTS) / sizeof(SHOTS[0]);
static Cam cameraAt(float t) {
    int k = 0;
    for (int i = 1; i < N_SHOTS; i++) if (t >= SHOTS[i].a) k = i;
    Cam c = shotAt(k, t);
    if (k > 0 && t < SHOTS[k].b) {
        Cam p = shotAt(k - 1, t);
        float w = sstep(SHOTS[k].a, SHOTS[k].b, t);
        Cam r;
        r.eye = dmix(p.eye, c.eye, w); r.target = dmix(p.target, c.target, w);
        r.up = normalize(mix(p.up, c.up, w)); r.fov = lerpf(p.fov, c.fov, w);
        return r;
    }
    return c;
}

// ============================================================================
//  15. GPU ASSETS
// ============================================================================
struct Assets {
    GLuint tileA = 0, tileN = 0, earthTex = 0, cloudTex = 0, moonTex = 0, moonNrm = 0, flagTex = 0, shipSkin = 0, boosterSkin = 0, truss = 0;
    Mesh moonTerrain, padGround, sphere, box, quad, cyl, cyl8, cone, cloth, plume, nose, nozzle, skirt;
    GLuint starVAO = 0, starVBO = 0; int starCount = 0;
    void init() {
        std::printf("generating textures (a few seconds)...\n");
        makeMoonTile(1024, tileA, tileN);
        makeEarthTextures(2048, 1024, earthTex, cloudTex);
        makeMoonGlobal(2048, 1024, moonTex, moonNrm);
        flagTex = makeKenyaFlag(600, 400);
        shipSkin = makeHullSkin(1024, 1024, true, "HARAMBEE-1", 0.f);
        boosterSkin = makeHullSkin(512, 1024, false, nullptr, 0.9f);
        truss = makeTruss(256);
        std::printf("building terrain meshes...\n");
        moonTerrain = buildPolarTerrain(2600.f, 240, 360, 2.4f, moonTerrainHeight, (float)R_MOON);
        padGround = buildPolarTerrain(7000.f, 200, 320, 2.2f, padGroundHeight, (float)R_EARTH);
        sphere = makeSphere(64, 40); box = makeBox(); quad = makeQuad();
        cyl = makeCylinder(1.f, 1.f, 1.f, 48, true);
        cyl8 = makeCylinder(1.f, 1.f, 1.f, 8, false, PI / 8.f);
        cone = makeCylinder(1.f, 0.f, 1.f, 32, true);
        cloth = makeCloth(1.5f, 1.0f, 28, 0.07f, 9.f);
        plume = makePlume(32, 24);
        std::vector<float> np;                                   // ogive nose, v continues the hull skin
        for (int i = 0; i <= 24; i++) { float s = i / 24.f; np.push_back(SHIP_R * std::pow(std::max(0.f, 1.f - std::pow(s, 1.7f)), 0.55f) + (i == 24 ? 0.f : 0.f)); np.push_back(24.f + 10.f * s); }
        nose = makeLathe(np, 48, 1.0f, 1.42f);
        std::vector<float> zp = { 1.0f,-1.0f, 0.86f,-0.75f, 0.68f,-0.5f, 0.47f,-0.25f, 0.3f,-0.05f, 0.34f,0.12f };
        nozzle = makeLathe(zp, 28);
        std::vector<float> sk = { SHIP_R * 0.92f, 0.f, SHIP_R, 0.6f };
        skirt = makeLathe(sk, 48);
        // star field: 9000 stars, 40 % in a Milky-Way band
        struct S { float d[3], mag, tint; };
        std::vector<S> st; Rng r(424242u);
        vec3 bandN = normalize(vec3(0.35f, 0.8f, 0.45f));
        for (int i = 0; i < 9000; i++) {
            vec3 d;
            do { d = vec3(r.f() * 2 - 1, r.f() * 2 - 1, r.f() * 2 - 1); } while (dot(d, d) > 1.f || dot(d, d) < 0.01f);
            d = normalize(d);
            if (i % 10 < 4) d = normalize(d - bandN * (dot(d, bandN) * (0.85f + 0.1f * r.f())));
            S s = { {d.x, d.y, d.z}, std::pow(r.f(), 5.f), r.f() }; st.push_back(s);
        }
        starCount = (int)st.size();
        glGenVertexArrays(1, &starVAO); glBindVertexArray(starVAO);
        glGenBuffers(1, &starVBO); glBindBuffer(GL_ARRAY_BUFFER, starVBO);
        glBufferData(GL_ARRAY_BUFFER, st.size() * sizeof(S), st.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(S), (void*)0);
        glEnableVertexAttribArray(1); glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(S), (void*)12);
        glBindVertexArray(0);
    }
};

// ============================================================================
//  16. PARTICLES  (smoke, steam, lunar dust, plasma)  - stored in a local frame
// ============================================================================
enum { FR_PAD = 0, FR_SITE = 1, FR_SHIP = 2 };
struct Particle {
    vec3 p, v; float life, maxLife, size, grow, grav, drag; vec3 col; float alpha; int frame; int collide; bool settled, additive;
};
struct Particles {
    std::vector<Particle> ps; GLuint vao = 0, vbo = 0; Rng rng = Rng(1u);
    static const size_t MAXP = 24000;
    void init() {
        glGenVertexArrays(1, &vao); glBindVertexArray(vao);
        glGenBuffers(1, &vbo); glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, MAXP * 8 * sizeof(float), nullptr, GL_STREAM_DRAW);
        glEnableVertexAttribArray(0); glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 32, (void*)0);
        glEnableVertexAttribArray(1); glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 32, (void*)16);
        glBindVertexArray(0);
    }
    void clear() { ps.clear(); rng = Rng(1u); }
    float f() { return rng.f(); }
    float r(float a, float b) { return rng.r(a, b); }
    void add(const Particle& q) { if (ps.size() < MAXP) ps.push_back(q); }
    // radial ballistic dust on the Moon (no air: no drag, 1.62 m/s^2)
    void dust(vec3 pos, int n, float smin, float smax, float upMin, float upMax, float size, float life, float alpha) {
        for (int i = 0; i < n; i++) {
            float a = f() * 2 * PI, sp = lerpf(smin, smax, f());
            Particle q; q.p = pos + vec3(std::cos(a), 0, std::sin(a)) * (0.2f + f() * 0.8f);
            q.v = vec3(std::cos(a) * sp, lerpf(upMin, upMax, f()), std::sin(a) * sp);
            q.maxLife = q.life = life * (0.6f + 0.8f * f()); q.size = size * (0.6f + 0.9f * f()); q.grow = 0;
            q.grav = -1.62f; q.drag = 0; float g = 0.80f + 0.15f * f(); q.col = vec3(g, g * 0.96f, g * 0.9f); q.alpha = alpha;
            q.frame = FR_SITE; q.collide = 1; q.settled = false; q.additive = false; add(q);
        }
    }
    void step(float dt) {
        for (size_t i = 0; i < ps.size(); i++) {
            Particle& q = ps[i];
            if (!q.settled) {
                q.v.y += q.grav * dt;
                if (q.drag > 0) q.v = q.v * std::max(0.f, 1.f - q.drag * dt);
                q.p = q.p + q.v * dt;
                if (q.collide == 1) {
                    float h = moonTerrainHeight(q.p.x, q.p.z) + 0.03f;
                    if (q.p.y < h) { q.p.y = h; q.v = vec3(); q.settled = true; q.life = std::min(q.life, 1.8f); }
                }
                else if (q.collide == 2) {
                    if (q.p.y < 0.5f) { q.p.y = 0.5f; q.v.y = std::fabs(q.v.y) * 0.2f; }
                }
            }
            q.size += q.grow * dt; q.life -= dt;
        }
        size_t w = 0;
        for (size_t i = 0; i < ps.size(); i++) if (ps[i].life > 0.f) ps[w++] = ps[i];
        ps.resize(w);
    }
};

// ============================================================================
//  17. RENDERER  (shadow map -> MSAA HDR -> planets -> objects -> glow -> particles -> bloom -> ACES)
// ============================================================================
struct Material {
    vec3 albedo; float spec, shin, metal; vec3 emissive; GLuint tex; int texMode; bool twoSided;
    Material() : albedo(0.8f, 0.8f, 0.8f), spec(0.2f), shin(32.f), metal(0.f), emissive(), tex(0), texMode(0), twoSided(false) {}
};
static Material mat(vec3 c, float spec, float shin, float metal, vec3 em = vec3()) { Material m; m.albedo = c; m.spec = spec; m.shin = shin; m.metal = metal; m.emissive = em; return m; }

struct Env {                       // lighting environment at the camera, recomputed every frame
    vec3 sunCol, sunCol0, up, ambUp, ambDown, earthDir, earthCol, fogCol; float fogDens, starGain, exposure;
};
static const vec3 SUN_COL0(6.2f, 5.9f, 5.4f);
static const vec3 BETA_R(5.8e-6f, 13.5e-6f, 33.1e-6f);
static vec3 sunTransmittance(dvec3 pRelEarth) {             // CPU copy of the shader's optical depth
    vec3 oc = tof(pRelEarth); double r = dlength(pRelEarth);
    if (r > R_EARTH + 100000.0) return vec3(1, 1, 1);
    float b = dot(oc, W.sun), r2 = (float)(r * r);
    if (b < 0.f && b * b - (r2 - (float)(R_EARTH * R_EARTH)) > 0.f) return vec3(0, 0, 0);
    float RA = (float)(R_EARTH + 100000.0), tEnd = -b + std::sqrt(std::max(b * b - (r2 - RA * RA), 0.f));
    float odR = 0, odM = 0, dt = tEnd / 16.f;
    for (int j = 0; j < 16; j++) { float h = length(oc + W.sun * (dt * (j + 0.5f))) - (float)R_EARTH; odR += std::exp(-h / 8000.f) * dt; odM += std::exp(-h / 1200.f) * dt; }
    return vec3(std::exp(-(BETA_R.x * odR + 21e-6f * 1.1f * odM)), std::exp(-(BETA_R.y * odR + 21e-6f * 1.1f * odM)), std::exp(-(BETA_R.z * odR + 21e-6f * 1.1f * odM)));
}
static Env computeEnv(dvec3 cam) {
    Env e;
    double hE = dlength(cam) - R_EARTH, hM = dlength(cam - W.moonC) - R_MOON;
    vec3 T = sunTransmittance(cam);
    e.sunCol0 = SUN_COL0;
    e.sunCol = vec3(SUN_COL0.x * T.x, SUN_COL0.y * T.y, SUN_COL0.z * T.z);
    float air = (float)std::exp(-std::max(hE, 0.0) / 8000.0);
    bool nearMoon = hM < hE;
    e.up = nearMoon ? tof(dnormalize(cam - W.moonC)) : tof(dnormalize(cam));
    float wE = 1.f - sstep(5.0e4f, 2.0e6f, (float)hE), wM = 1.f - sstep(2.0e4f, 1.5e6f, (float)hM);
    e.ambUp = vec3(0.32f, 0.46f, 0.78f) * air;
    vec3 ground = vec3(0.10f, 0.11f, 0.075f) * (0.4f * std::max(dot(W.padB.y, W.sun), 0.f)) * 6.f;
    e.ambDown = ground * air;
    float sunElevSite = std::max(dot(W.siteB.y, W.sun), 0.05f);
    vec3 regolith = vec3(0.13f, 0.125f, 0.115f) * SUN_COL0.x * sunElevSite * 0.5f;
    e.ambDown = e.ambDown + regolith * wM;
    float k = (float)((R_EARTH / std::max(dlength(cam), R_EARTH)) * (R_EARTH / std::max(dlength(cam), R_EARTH)));
    e.earthDir = tof(dnormalize(dvec3(0, 0, 0) - cam));
    e.earthCol = vec3(0.30f, 0.45f, 0.9f) * 0.05f + vec3(0.20f, 0.27f, 0.40f) * (2.2f * k * (1.f - air) * wE);
    e.fogCol = vec3(0.75f, 0.92f, 1.25f) * air;
    e.fogDens = air / 70000.f;
    e.starGain = nearMoon ? 0.55f : sstep(25000.f, 90000.f, (float)hE);
    e.exposure = 0.46f;
    return e;
}

struct Frame {                     // per-frame camera data
    dvec3 cam; mat4 view, proj, viewProj; float fov, aspect; Env env; float t;
    vec3 rel(dvec3 p) const { return tof(p - cam); }
};

struct Renderer {
    Assets* A = nullptr; Particles* P = nullptr;
    Program pShadow, pLit, pTerrain, pGround, pPlanet, pStars, pPart, pGlow, pBright, pBlur, pComposite, pUI;
    int shadowRes = 2048; GLuint shadowFBO = 0, shadowTex = 0, emptyVAO = 0;
    GLuint msaaFBO = 0, msaaColor = 0, msaaDepth = 0, resolveFBO = 0, resolveTex = 0;
    GLuint bloomFBO[2] = { 0, 0 }, bloomTex[2] = { 0, 0 }; int bw = 0, bh = 0;
    int outW = 0, outH = 0;
    bool inShadow = false; mat4 lightVP; float shadowE = 50.f;
    const Frame* F = nullptr;
    float logC = 0.f;

    bool init(Assets* a, Particles* p, int shadowRes_) {
        A = a; P = p; shadowRes = shadowRes_;
        using std::string;
        string H = sh::header, U = sh::uniformsCommon, O = sh::objIn;
        bool ok = true;
        ok &= pShadow.build(H + R"(
layout(location=0) in vec3 aPos; layout(location=2) in vec2 aUV; uniform mat4 uModel; uniform mat4 uLightVP; out vec2 vUV;
void main(){ vUV = aUV; gl_Position = uLightVP*uModel*vec4(aPos,1.0); })",
H + "in vec2 vUV; uniform int uCut; uniform sampler2D uTex; void main(){ if(uCut==1 && texture(uTex,vUV).a < 0.5) discard; }\n", "shadow");
        ok &= pLit.build(H + sh::objVS, H + U + O + sh::litHelpers + sh::litFS, "lit");
        ok &= pTerrain.build(H + sh::objVS, H + U + O + sh::moonCommon + sh::terrainMain, "terrain");
        ok &= pGround.build(H + sh::objVS, H + U + O + sh::atmoCommon + sh::earthCommon + sh::groundMain, "ground");
        ok &= pPlanet.build(H + sh::fsTriVS, H + U + sh::atmoCommon + sh::earthCommon + sh::moonCommon + sh::planetFS, "planet");
        ok &= pStars.build(H + sh::starVS, H + sh::starFS, "stars");
        ok &= pPart.build(H + sh::partVS, H + sh::partFS, "particles");
        ok &= pGlow.build(H + sh::objVS, H + sh::glowFS, "glow");
        ok &= pBright.build(H + sh::fsTriVS, H + sh::brightFS, "bright");
        ok &= pBlur.build(H + sh::fsTriVS, H + sh::blurFS, "blur");
        ok &= pComposite.build(H + sh::fsTriVS, H + sh::compositeFS, "composite");
        ok &= pUI.build(H + sh::uiVS, H + sh::uiFS, "ui");
        if (!ok) return false;
        glGenVertexArrays(1, &emptyVAO);
        glGenTextures(1, &shadowTex); glBindTexture(GL_TEXTURE_2D, shadowTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, shadowRes, shadowRes, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        float border[4] = { 1, 1, 1, 1 }; glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        glGenFramebuffers(1, &shadowFBO); glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTex, 0);
        glDrawBuffer(GL_NONE); glReadBuffer(GL_NONE);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) { std::fprintf(stderr, "shadow FBO incomplete\n"); return false; }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        logC = 2.f / std::log2(1.0e9f + 1.f);
        return true;
    }
    static GLuint makeTarget(int w, int h, GLuint& fbo) {
        GLuint t; glGenTextures(1, &t); glBindTexture(GL_TEXTURE_2D, t);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glGenFramebuffers(1, &fbo); glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, t, 0);
        return t;
    }
    void resize(int w, int h) {
        if (w < 16 || h < 16) return;
        if (msaaFBO) {
            glDeleteFramebuffers(1, &msaaFBO); glDeleteFramebuffers(1, &resolveFBO); glDeleteFramebuffers(2, bloomFBO);
            glDeleteRenderbuffers(1, &msaaColor); glDeleteRenderbuffers(1, &msaaDepth);
            glDeleteTextures(1, &resolveTex); glDeleteTextures(2, bloomTex);
        }
        outW = w; outH = h;
        glGenFramebuffers(1, &msaaFBO); glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
        glGenRenderbuffers(1, &msaaColor); glBindRenderbuffer(GL_RENDERBUFFER, msaaColor);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA16F, w, h);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, msaaColor);
        glGenRenderbuffers(1, &msaaDepth); glBindRenderbuffer(GL_RENDERBUFFER, msaaDepth);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT24, w, h);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, msaaDepth);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) std::fprintf(stderr, "MSAA FBO incomplete\n");
        resolveTex = makeTarget(w, h, resolveFBO);
        bw = std::max(1, w / 2); bh = std::max(1, h / 2);
        for (int i = 0; i < 2; i++) bloomTex[i] = makeTarget(bw, bh, bloomFBO[i]);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    // uniforms shared by every lit program
    void bindCommon(Program& p) {
        const Env& e = F->env;
        p.set("uViewProj", F->viewProj); p.set("uLightVP", lightVP); p.set("uLogC", logC);
        p.set("uSunDir", W.sun); p.set("uSunCol", e.sunCol); p.set("uSunCol0", e.sunCol0); p.set("uUp", e.up); p.set("uTime", F->t);
        p.set("uAmbUp", e.ambUp); p.set("uAmbDown", e.ambDown); p.set("uEarthDir", e.earthDir); p.set("uEarthCol", e.earthCol);
        p.set("uFogCol", e.fogCol); p.set("uFogDens", e.fogDens);
        p.set("uShadowOn", 1.f); p.set("uShadowTexel", 1.f / shadowRes); p.set("uNormalOffset", 2.6f * (2.f * shadowE / shadowRes));
        glActiveTexture(GL_TEXTURE0 + 5); glBindTexture(GL_TEXTURE_2D, shadowTex); p.set("uShadow", 5);
        glActiveTexture(GL_TEXTURE0);
    }
    void bindPlanetUniforms(Program& p) {
        const Frame& f = *F;
        dvec3 ec = dvec3(0, 0, 0) - f.cam;
        p.set("uEC", tof(ec)); p.set("uEUp", tof(dnormalize(f.cam))); p.set("uEH", (float)(dlength(f.cam) - R_EARTH));
        p.set("uSunE", SUN_COL0 * PI); p.set("uSkyAmb", vec3(0.30f, 0.42f, 0.66f)); p.set("uSkyRefl", vec3(0.42f, 0.58f, 0.86f));
        p.set("uPadRot", W.padB); p.set("uCamPad", tof(f.cam - W.padO));
        p.set("uSiteRot", W.siteB); p.set("uCamSite", tof(f.cam - W.siteO));
        glActiveTexture(GL_TEXTURE0 + 1); glBindTexture(GL_TEXTURE_2D, A->earthTex); p.set("uEarthTex", 1);
        glActiveTexture(GL_TEXTURE0 + 2); glBindTexture(GL_TEXTURE_2D, A->cloudTex); p.set("uCloudTex", 2);
        glActiveTexture(GL_TEXTURE0 + 3); glBindTexture(GL_TEXTURE_2D, A->tileA); p.set("uTileA", 3);
        glActiveTexture(GL_TEXTURE0 + 4); glBindTexture(GL_TEXTURE_2D, A->tileN); p.set("uTileN", 4);
        glActiveTexture(GL_TEXTURE0 + 6); glBindTexture(GL_TEXTURE_2D, A->moonTex); p.set("uMoonTex", 6);
        glActiveTexture(GL_TEXTURE0 + 7); glBindTexture(GL_TEXTURE_2D, A->moonNrm); p.set("uMoonNrm", 7);
        glActiveTexture(GL_TEXTURE0);
    }
    void draw(const Mesh& m, const mat4& model, const Material& mt) {
        if (inShadow) {
            pShadow.set("uModel", model);
            int cut = (mt.texMode == 2 && mt.tex) ? 1 : 0;
            pShadow.set("uCut", cut);
            if (cut) { glBindTexture(GL_TEXTURE_2D, mt.tex); pShadow.set("uTex", 0); }
            if (mt.twoSided || cut) glDisable(GL_CULL_FACE);
            m.draw();
            if (mt.twoSided || cut) glEnable(GL_CULL_FACE);
            return;
        }
        pLit.set("uModel", model);
        pLit.set("uAlbedo", mt.albedo); pLit.set("uEmissive", mt.emissive);
        pLit.set("uSpec", mt.spec); pLit.set("uShin", mt.shin); pLit.set("uMetal", mt.metal);
        pLit.set("uHasTex", mt.tex ? 1 : 0); pLit.set("uTwoSided", mt.twoSided ? 1 : 0); pLit.set("uTexMode", mt.texMode);
        if (mt.tex) { glBindTexture(GL_TEXTURE_2D, mt.tex); pLit.set("uTex", 0); }
        bool noCull = mt.twoSided || mt.texMode == 2;
        if (noCull) glDisable(GL_CULL_FACE);
        m.draw();
        if (noCull) glEnable(GL_CULL_FACE);
    }
    void glow(const Mesh& m, const mat4& model, vec3 core, vec3 outer, float intensity, float diamond, int mode, vec3 velDir, float seed) {
        if (intensity <= 0.001f) return;
        pGlow.set("uModel", model); pGlow.set("uCoreCol", core); pGlow.set("uOuterCol", outer); pGlow.set("uIntensity", intensity);
        pGlow.set("uDiamond", diamond); pGlow.set("uMode", mode); pGlow.set("uVelDir", velDir); pGlow.set("uSeed", seed);
        m.draw();
    }
    void fullscreen() { glBindVertexArray(emptyVAO); glDrawArrays(GL_TRIANGLES, 0, 3); }
};

// ============================================================================
//  18. THE FILM: models, special effects, particles
// ============================================================================
struct Palm { float x, z, h, lean, rot; };
struct Rock { float x, z, s, sy, g, rot; };

struct Film {
    Assets* A = nullptr; Particles* P = nullptr;
    float t = 0; float acc[16]; int hopsDone = 0; bool touchBurst = false, liftBurst = false;
    std::vector<Palm> palms; std::vector<Rock> rocks;

    void init(Assets* a, Particles* p) {
        A = a; P = p;
        Rng r(777u);
        for (int i = 0; i < 70; i++) {                                         // palms: scattered + a row along the beach
            Palm pm;
            if (i < 40) {
                float ang = r.r(0, 2 * PI), rad = r.r(140.f, 900.f);
                pm.x = rad * std::cos(ang); pm.z = rad * std::sin(ang);
                if (pm.x < -60.f && std::fabs(pm.z) < 25.f) pm.z += 40.f;
                if (pm.x > coastXcpu(pm.z) - 90.f) pm.x = coastXcpu(pm.z) - 120.f - r.r(0, 200);
                if (length(vec3(pm.x + 180.f, 0, pm.z - 140.f)) < 60.f) pm.x -= 80.f;
            }
            else { pm.z = -650.f + (i - 40) * 45.f + r.r(-15, 15); pm.x = coastXcpu(pm.z) - r.r(75.f, 105.f); }
            pm.h = r.r(9.f, 15.f); pm.lean = r.r(-0.18f, 0.18f); pm.rot = r.r(0, 2 * PI);
            palms.push_back(pm);
        }
        Rng q(31337u);
        for (int i = 0; i < 46; i++) {                                         // lunar rocks, keeping the walking area clear
            Rock k; float rr, a;
            do { rr = 8.f + 70.f * std::pow(q.f(), 1.5f); a = q.r(0, 2 * PI); k.x = rr * std::cos(a); k.z = rr * std::sin(a); } while ((k.x > -4.f && k.x < 21.f && k.z > -1.f && k.z < 19.f) || rr < 9.f);
            k.s = q.r(0.15f, 0.8f) * (1.f + rr / 50.f); k.sy = q.r(0.4f, 0.9f); k.g = q.r(0.8f, 1.2f); k.rot = q.r(0, 2 * PI);
            rocks.push_back(k);
        }
        reset();
    }
    void reset() { t = 0; for (int i = 0; i < 16; i++) acc[i] = 0; hopsDone = 0; touchBurst = liftBurst = false; P->clear(); }
    void seek(float target) {
        P->clear(); for (int i = 0; i < 16; i++) acc[i] = 0;
        target = clampf(target, 0.f, T_END);
        t = std::max(0.f, target - 12.f);
        touchBurst = t >= T_TOUCH; liftBurst = t >= T_MLIFT;
        hopsDone = 0; while (hopsDone < 3 && t >= 162.f + HOP_T * (hopsDone + 1)) hopsDone++;
        const float dt = 1.f / 60.f;
        while (t < target - 1e-4f) step(std::min(dt, target - t));
    }

    // ------------------------------------------------------------------ simulation: particle emitters
    int emitCount(int k, float rate, float dt) { acc[k] += rate * dt; int n = (int)acc[k]; acc[k] -= n; return n; }
    void smoke(vec3 pos, vec3 vel, float size, float grow, float life, vec3 col, float alpha, float grav, float drag, int frame, bool additive) {
        Particle q; q.p = pos; q.v = vel; q.size = size; q.grow = grow; q.life = q.maxLife = life; q.col = col; q.alpha = alpha;
        q.grav = grav; q.drag = drag; q.frame = frame; q.collide = frame == FR_PAD ? 2 : 0; q.settled = false; q.additive = additive; P->add(q);
    }
    void step(float dt) {
        t += dt; const float T = t;
        float f;
        // ---- launch pad: venting, ignition, the smoke storm of liftoff
        if (T < T_IGN) {
            int n = emitCount(0, 14.f, dt);
            for (int i = 0; i < n; i++) {
                float a = P->f() * 2 * PI, y = P->f() < 0.5f ? 28.f + P->f() * 6.f : 66.f + P->f() * 10.f;
                vec3 d(std::cos(a), 0, std::sin(a));
                smoke(d * (SHIP_R + 0.2f) + vec3(0, y, 0), d * P->r(2.f, 5.f) + vec3(1.2f, -0.6f, 0), P->r(0.8f, 1.6f), 1.6f, P->r(2.f, 3.5f), vec3(2.6f, 2.6f, 2.65f), 0.32f, -0.4f, 0.4f, FR_PAD, false);
            }
        }
        float alt = (float)ascentLocal(T - T_LIFT).y - STACK_Y;
        if (T > T_IGN - 0.3f && T < T_LIFT + 16.f) {
            float k = sstep(T_IGN - 0.3f, T_LIFT, T) * (1.f - sstep(T_LIFT + 5.f, T_LIFT + 15.f, T));
            int n = emitCount(1, 300.f * k, dt);
            for (int i = 0; i < n; i++) {
                float a = P->f() * 2 * PI, sp = P->r(16.f, 52.f);
                vec3 d(std::cos(a), 0, std::sin(a));
                float g = P->r(0.75f, 1.1f);
                smoke(d * 9.f + vec3(0, P->r(2.f, 6.f), 0), d * sp + vec3(0, P->r(0.f, 7.f), 0), P->r(6.f, 12.f), P->r(2.5f, 4.5f), P->r(8.f, 14.f),
                    vec3(2.4f, 2.32f, 2.2f) * g, 0.55f, 0.8f, 0.32f, FR_PAD, false);
            }
            if (T > T_IGN) {                                                    // fire glow in the flame trench
                int m = emitCount(2, 120.f * (1.f - sstep(T_LIFT + 2.f, T_LIFT + 6.f, T)), dt);
                for (int i = 0; i < m; i++) {
                    float a = P->f() * 2 * PI; vec3 d(std::cos(a), 0, std::sin(a));
                    smoke(d * P->r(2.f, 9.f) + vec3(0, P->r(1.f, 8.f), 0), d * P->r(10.f, 35.f) + vec3(0, P->r(0.f, 6.f), 0), P->r(3.f, 7.f), 5.f, P->r(0.6f, 1.4f),
                        vec3(9.f, 4.f, 1.4f) * P->r(0.6f, 1.2f), 0.6f, 0.f, 0.5f, FR_PAD, true);
                }
            }
            if (T > T_LIFT && alt < 900.f) {                                     // exhaust column under the climbing booster
                int m = emitCount(3, 160.f * (1.f - alt / 900.f), dt);
                for (int i = 0; i < m; i++) {
                    float a = P->f() * 2 * PI, r = P->f() * 3.f;
                    smoke(vec3(std::cos(a) * r, MOUNT_H + alt - P->r(5.f, 40.f), std::sin(a) * r), vec3(std::cos(a) * 6.f, -P->r(20.f, 45.f), std::sin(a) * 6.f),
                        P->r(5.f, 9.f), 3.5f, P->r(6.f, 10.f), vec3(2.3f, 2.25f, 2.15f) * P->r(0.8f, 1.05f), 0.45f, 0.6f, 0.5f, FR_PAD, false);
                }
            }
        }
        // ---- the Moon: descent dust, touchdown, footsteps, the flag, hops, liftoff
        if ((T > 100.f && T < T_TOUCH + 0.2f) || (T > T_MLIFT - 1.2f && T < T_MLIFT + 6.f)) {
            dvec3 lp = shipPos(T) - W.siteO;
            float h = (float)ddot(lp, dvec3(W.siteB.y)) - LEG_CLEAR + 2.f;
            float thr = shipState(T).ship;
            if (h < 48.f && thr > 0.05f) {
                float k = 1.f - h / 48.f;
                int n = emitCount(4, 2400.f * std::pow(k, 1.2f) * thr, dt);
                P->dust(vec3((float)ddot(lp, dvec3(W.siteB.x)), moonTerrainHeight(0, 0), (float)ddot(lp, dvec3(W.siteB.z))), n, 5.f, 12.f + 26.f * k, 0.15f, 2.2f, 0.65f, 6.5f, 0.55f);
            }
        }
        if (!touchBurst && T >= T_TOUCH) { touchBurst = true; P->dust(vec3(0, 0.02f, 0), 600, 2.f, 22.f, 0.15f, 1.8f, 0.6f, 6.f, 0.55f); }
        if (!liftBurst && T >= T_MLIFT) { liftBurst = true; P->dust(vec3(0, 0.02f, 0), 700, 4.f, 30.f, 0.2f, 2.5f, 0.6f, 6.f, 0.55f); }
        AstroPose a = astroAt(T);
        if (a.visible && a.walk > 0.25f && a.pos.y < 0.05f) {
            acc[5] += a.walk * dt;
            if (acc[5] > 0.9f) { acc[5] = 0; P->dust(a.pos + rotYv(vec3(0, 0, 0.1f), a.heading), 9, 0.3f, 1.3f, 0.3f, 1.4f, 0.13f, 3.f, 0.45f); }
        }
        if (T > 153.8f && T < 159.f) {
            acc[6] += dt;
            if (acc[6] > 0.55f) { acc[6] = 0; P->dust(vec3(a.poleBase.x, 0.f, a.poleBase.z), 35, 0.6f, 2.8f, 0.4f, 1.8f, 0.14f, 3.5f, 0.5f); }
        }
        while (hopsDone < 3 && T >= 162.f + HOP_T * (hopsDone + 1)) { hopsDone++; P->dust(vec3(PL.x, 0.02f, PL.z), 60, 0.6f, 3.2f, 0.3f, 1.6f, 0.17f, 4.f, 0.5f); }
        AlienPose al = alienAt(T);
        if (al.vis && al.moving && al.pos.y < 0.4f) {
            acc[7] += dt;
            if (acc[7] > 0.22f) { acc[7] = 0; P->dust(al.pos, 5, 0.2f, 0.9f, 0.2f, 1.0f, 0.1f, 2.5f, 0.45f); }
        }
        // ---- re-entry plasma streaming off the hull (in the ship's frame)
        f = ramp(T, 249.f, 253.f, 262.f, 269.f);
        if (f > 0.f) {
            Basis B = shipBasis(T); vec3 vd = tof(dnormalize(shipVel(T)));
            int n = emitCount(8, 700.f * f, dt);
            for (int i = 0; i < n; i++) {
                vec3 o = B * vec3(P->r(-SHIP_R, SHIP_R) * (P->f() < 0.3f ? 1.6f : 1.f), P->r(0.f, SHIP_H), -SHIP_R * P->r(0.6f, 1.1f));
                smoke(o, -vd * P->r(90.f, 260.f) + vec3(P->r(-8, 8), P->r(-8, 8), P->r(-8, 8)), P->r(0.6f, 1.6f), 2.f, P->r(0.3f, 0.9f),
                    vec3(7.f, 2.6f, 1.6f) * P->r(0.5f, 1.3f) * f, 0.7f, 0.f, 0.f, FR_SHIP, true);
            }
        }
        // ---- landing burn: exhaust splashing off the launch mount
        f = shipState(T).ship * sstep(283.f, 285.f, T) * (1.f - sstep(T_CATCH, T_CATCH + 1.f, T));
        if (f > 0.f) {
            int n = emitCount(9, 160.f * f, dt);
            for (int i = 0; i < n; i++) {
                float a2 = P->f() * 2 * PI; vec3 d(std::cos(a2), 0, std::sin(a2));
                smoke(d * 4.f + vec3(0, MOUNT_H + 1.f, 0), d * P->r(12.f, 30.f) + vec3(0, P->r(-2.f, 4.f), 0), P->r(3.f, 6.f), 3.f, P->r(4.f, 7.f),
                    vec3(2.5f, 2.45f, 2.4f) * P->r(0.8f, 1.05f), 0.4f, 0.5f, 0.45f, FR_PAD, false);
            }
        }
        P->step(dt);
    }

    // ------------------------------------------------------------------ models
    static void strut(Renderer& R, const Assets& A, const mat4& B, vec3 a, vec3 b, float r, const Material& m) {
        R.draw(A.cyl, B * translate(a) * alignY(b - a) * scale(vec3(r, length(b - a), r)), m);
    }
    void drawAlienHead(Renderer& R, const mat4& M) const {
        Material skin = mat(vec3(0.30f, 0.80f, 0.32f), 0.35f, 30.f, 0.f), eye = mat(vec3(0.02f, 0.02f, 0.04f), 1.0f, 120.f, 0.f);
        Material shine = mat(vec3(1, 1, 1), 0, 1, 0, vec3(3.f, 3.f, 3.f)), tip = mat(vec3(1.f, 0.9f, 0.3f), 0.5f, 30.f, 0.f, vec3(4.f, 3.4f, 0.8f));
        R.draw(A->sphere, M * scale(vec3(0.41f, 0.36f, 0.36f)), skin);
        for (int e = -1; e <= 1; e += 2) {
            R.draw(A->sphere, M * translate(vec3(0.17f * e, 0.05f, 0.28f)) * scale(vec3(0.088f, 0.132f, 0.055f)), eye);
            R.draw(A->sphere, M * translate(vec3(0.17f * e + 0.03f, 0.10f, 0.325f)) * scale(0.028f), shine);
            strut(R, *A, M, vec3(0.12f * e, 0.30f, 0), vec3(0.24f * e, 0.68f, 0), 0.02f, skin);
            R.draw(A->sphere, M * translate(vec3(0.24f * e, 0.71f, 0)) * scale(0.06f), tip);
        }
    }
    void drawMiniFlag(Renderer& R, const mat4& M) const {
        Material pole = mat(vec3(0.85f, 0.85f, 0.88f), 0.5f, 60.f, 0.7f);
        R.draw(A->cyl, M * scale(vec3(0.016f, 0.9f, 0.016f)), pole);
        Material cl = mat(vec3(1, 1, 1), 0.04f, 10.f, 0.f); cl.tex = A->flagTex; cl.twoSided = true;
        R.draw(A->cloth, M * translate(vec3(0.02f, 0.88f, 0)) * scale(vec3(0.3f, 0.3f, 0.5f)), cl);
    }
    void drawAlien(Renderer& R, const mat4& M, float walk, float armR, float armL, bool moving, bool hanging, bool flag, float armBack = 0.f) const {
        Material skin = mat(vec3(0.30f, 0.80f, 0.32f), 0.35f, 30.f, 0.f);
        float sw = moving ? std::sin(walk) * 0.5f : 0.f;
        for (int s = -1; s <= 1; s += 2) {
            float legA = hanging ? 0.25f * std::sin(walk * 0.7f + s) : s * sw;
            R.draw(A->cyl, M * translate(vec3(0.12f * s, 0.38f, 0)) * rotateX(legA) * rotateX(PI) * scale(vec3(0.07f, 0.38f, 0.07f)), skin);
        }
        R.draw(A->sphere, M * translate(vec3(0, 0.68f, 0)) * scale(vec3(0.27f, 0.34f, 0.24f)), skin);
        drawAlienHead(R, M * translate(vec3(0, 1.22f, 0)));
        for (int s = -1; s <= 1; s += 2) {
            vec3 shp(0.26f * s, 0.95f, 0.f);
            float up = (s > 0) ? armR : armL;
            float deg = 20.f + up * 150.f - (s > 0 ? 0.f : sw * 20.f);
            float ar = deg * PI / 180.f;
            float cb = std::cos(armBack);
            vec3 hand(shp.x + s * std::sin(ar) * 0.6f * cb, shp.y - std::cos(ar) * 0.6f * cb, -0.6f * std::sin(armBack));
            strut(R, *A, M, shp, hand, 0.045f, skin);
            R.draw(A->sphere, M * translate(hand) * scale(0.07f), skin);
            if (s > 0 && flag) drawMiniFlag(R, M * translate(hand));
        }
    }
    void drawAstronaut(Renderer& R, const mat4& base, const AstroPose& a) const {
        Material suit = mat(vec3(0.82f, 0.82f, 0.84f), 0.25f, 24.f, 0.f), joint = mat(vec3(0.35f, 0.35f, 0.38f), 0.3f, 24.f, 0.f),
            visor = mat(vec3(0.95f, 0.70f, 0.20f), 1.0f, 220.f, 1.0f), boot = mat(vec3(0.22f, 0.22f, 0.24f), 0.2f, 20.f, 0.f),
            pack = mat(vec3(0.72f, 0.72f, 0.75f), 0.3f, 30.f, 0.2f);
        Material patch = mat(vec3(1, 1, 1), 0.1f, 20.f, 0.f); patch.tex = A->flagTex;
        mat4 B = base * translate(a.pos + vec3(0, a.hop + 0.05f * std::fabs(std::cos(a.phase)) * a.walk - a.crouch * 0.18f, 0)) * rotateY(a.heading) * rotateX(a.lean);
        R.draw(A->box, B * translate(vec3(0, 1.17f, 0)) * scale(vec3(0.46f, 0.58f, 0.30f)), suit);
        R.draw(A->box, B * translate(vec3(0, 0.92f, 0)) * scale(vec3(0.44f, 0.12f, 0.29f)), joint);
        R.draw(A->box, B * translate(vec3(0, 1.24f, -0.26f)) * scale(vec3(0.42f, 0.54f, 0.22f)), pack);
        R.draw(A->box, B * translate(vec3(0.06f, 1.18f, 0.16f)) * scale(vec3(0.24f, 0.16f, 0.06f)), joint);
        R.draw(A->quad, B * translate(vec3(-0.13f, 1.33f, 0.152f)) * scale(vec3(0.16f, 0.107f, 1.f)), patch);
        R.draw(A->cyl, B * translate(vec3(0, 1.44f, 0)) * scale(vec3(0.10f, 0.07f, 0.10f)), joint);
        R.draw(A->sphere, B * translate(vec3(0, 1.69f, 0.01f)) * scale(0.215f), suit);
        R.draw(A->sphere, B * translate(vec3(0, 1.68f, 0.10f)) * scale(vec3(0.175f, 0.14f, 0.15f)), visor);
        for (int sd = -1; sd <= 1; sd += 2) {
            float s = (float)sd, sw = std::sin(a.phase);
            float thigh = -0.55f * a.walk * s * sw + a.crouch * 0.5f;
            float knee = std::max(0.f, s * std::cos(a.phase)) * 0.9f * a.walk + a.crouch * 0.9f + (a.hop > 0.01f ? 0.35f : 0.f);
            mat4 Hm = B * translate(vec3(0.115f * s, 0.90f, 0)) * rotateX(thigh);
            R.draw(A->cyl, Hm * rotateX(PI) * scale(vec3(0.095f, 0.44f, 0.095f)), suit);
            mat4 K = Hm * translate(vec3(0, -0.44f, 0)) * rotateX(knee);
            R.draw(A->sphere, K * scale(0.09f), joint);
            R.draw(A->cyl, K * rotateX(PI) * scale(vec3(0.085f, 0.40f, 0.085f)), suit);
            R.draw(A->box, K * translate(vec3(0, -0.45f, 0.07f)) * scale(vec3(0.15f, 0.11f, 0.31f)), boot);
            float raise = (s > 0 ? a.raiseR : a.raiseL), el = (s > 0 ? a.elbowR : a.elbowL);
            float swing = -0.45f * a.walk * s * sw;
            mat4 S = B * translate(vec3(0.30f * s, 1.40f, 0)) * rotateZ(s * (raise > 1.57f ? -1.f : 1.f) * (0.06f + a.spread)) * rotateX(-(raise + swing));
            R.draw(A->sphere, S * scale(0.085f), joint);
            R.draw(A->cyl, S * rotateX(PI) * scale(vec3(0.075f, 0.30f, 0.075f)), suit);
            mat4 E = S * translate(vec3(0, -0.30f, 0)) * rotateX(-el);
            R.draw(A->cyl, E * rotateX(PI) * scale(vec3(0.068f, 0.28f, 0.068f)), suit);
            R.draw(A->sphere, E * translate(vec3(0, -0.30f, 0)) * scale(0.075f), joint);
        }
    }
    void drawFlagPole(Renderer& R, const mat4& base, const AstroPose& a) const {
        Material poleM = mat(vec3(0.85f, 0.85f, 0.88f), 0.5f, 60.f, 0.7f), gold = mat(vec3(0.85f, 0.65f, 0.2f), 1.f, 100.f, 1.f);
        vec3 d = a.poleTop - a.poleBase;
        R.draw(A->cyl, base * translate(a.poleBase) * alignY(d) * scale(vec3(0.022f, length(d), 0.022f)), poleM);
        R.draw(A->sphere, base * translate(a.poleTop) * scale(0.04f), gold);
        if (a.unfurl > 0.001f) {
            mat4 M = base * translate(a.poleTop - vec3(0, 0.10f, 0)) * rotateY(heroHeading());
            R.draw(A->cyl, M * rotateZ(-PI / 2) * scale(vec3(0.012f, 1.55f * a.unfurl, 0.012f)), poleM);
            Material cloth = mat(vec3(1, 1, 1), 0.04f, 10.f, 0.f); cloth.tex = A->flagTex; cloth.twoSided = true;
            R.draw(A->cloth, M * translate(vec3(0.02f, -0.02f, 0)) * scale(vec3(a.unfurl, 1.f, 1.f)), cloth);
        }
        else {
            static const vec3 C[3] = { vec3(0.0f, 0.30f, 0.12f), vec3(0.62f, 0.02f, 0.03f), vec3(0.01f, 0.01f, 0.01f) };
            for (int b = 0; b < 3; b++)
                R.draw(A->cyl, base * translate(a.poleBase) * alignY(d) * translate(vec3(0, POLE_LEN - 1.3f + 0.4f * b, 0)) * scale(vec3(0.06f, 0.4f, 0.06f)), mat(C[b], 0.1f, 10.f, 0.f));
        }
    }

    // the ship: stainless steel, black heat-shield tiles on the belly (-Z), flaps, legs, airlock on +Z
    void drawShip(Renderer& R, const Frame& fr, dvec3 pos, const Basis& Bs, const ShipState& s, float tt, float alienWin) const {
        mat4 M = fromBasis(Bs, fr.rel(pos));
        Material steel = mat(vec3(1, 1, 1), 0.9f, 90.f, 0.85f); steel.tex = A->shipSkin; steel.texMode = 1;
        Material tile = mat(vec3(0.035f, 0.035f, 0.038f), 0.25f, 20.f, 0.f), dark = mat(vec3(0.05f, 0.05f, 0.055f), 0.5f, 40.f, 0.3f);
        Material bell = mat(vec3(0.30f, 0.22f, 0.17f), 0.8f, 60.f, 0.8f), sm = mat(vec3(0.72f, 0.72f, 0.74f), 0.6f, 60.f, 0.8f);
        bell.twoSided = true;
        R.draw(A->cyl, M * scale(vec3(SHIP_R, 24.f, SHIP_R)), steel);
        R.draw(A->nose, M, steel);
        R.draw(A->cyl, M * translate(vec3(0, -0.02f, 0)) * scale(vec3(SHIP_R * 1.003f, 0.55f, SHIP_R * 1.003f)), dark);
        // engines: 3 sea-level (centre), 3 vacuum bells (outer)
        float glowI = s.ship;
        for (int i = 0; i < 6; i++) {
            bool vac = i >= 3; float a = (vac ? 60.f + 120.f * (i - 3) : 120.f * i) * PI / 180.f, r = vac ? 2.25f : 1.05f;
            float sc = vac ? 1.0f : 0.62f, len = vac ? 2.0f : 1.5f;
            Material b = bell; b.emissive = vec3(3.5f, 1.4f, 0.5f) * (glowI * 0.35f);
            R.draw(A->nozzle, M * translate(vec3(r * std::cos(a), 0.45f, r * std::sin(a))) * scale(vec3(sc, len, sc)), b);
        }
        // flaps: hinge along the hull, folded towards the belly when angle grows
        for (int sd = -1; sd <= 1; sd += 2) {
            float s1 = (float)sd;
            for (int k = 0; k < 2; k++) {
                bool fwd = k == 0; float ang = fwd ? s.flapF : s.flapA;
                float hr = fwd ? 3.0f : SHIP_R, y = fwd ? 28.2f : 4.6f, L = fwd ? 2.6f : 3.4f, Hh = fwd ? 3.8f : 6.4f;
                vec3 hinge(s1 * hr * std::cos(0.30f), y, -hr * std::sin(0.30f));
                vec3 d(s1 * std::cos(ang), 0, -std::sin(ang));
                float phi = std::atan2(std::sin(ang), s1 * std::cos(ang));
                R.draw(A->box, M * translate(hinge + d * (L * 0.5f)) * rotateY(phi) * scale(vec3(L, Hh, 0.22f)), tile);
            }
            R.draw(A->box, M * translate(vec3(s1 * (SHIP_R + 0.42f), PIN_Y, 0.9f)) * scale(vec3(1.1f, 0.45f, 0.6f)), sm);   // catch pins
        }
        // airlock door (slides around the hull), crew windows
        R.draw(A->box, M * translate(vec3(0, DOOR_Y, SHIP_R - 0.16f)) * scale(vec3(1.8f, 2.5f, 0.42f)), mat(vec3(0.02f, 0.02f, 0.022f), 0.1f, 10.f, 0.f));
        if (s.door > 0.f || true) {
            float al = -s.door * 0.62f, rr = SHIP_R + 0.07f + 0.12f * sstep(0.f, 0.2f, s.door);
            R.draw(A->box, M * rotateY(al) * translate(vec3(0, DOOR_Y, rr)) * scale(vec3(1.78f, 2.46f, 0.1f)), sm);
        }
        Material glass = mat(vec3(0.015f, 0.02f, 0.03f), 1.f, 200.f, 0.f);
        for (int w = -1; w <= 1; w++) {
            float al = w * 0.24f;
            R.draw(A->box, M * rotateY(al) * translate(vec3(0, 23.4f, SHIP_R - 0.02f)) * scale(vec3(0.55f, 0.48f, 0.12f)), glass);
        }
        if (alienWin > 0.01f) {
            mat4 H = M * translate(vec3(0.f, 23.32f, SHIP_R + 0.02f + 0.12f * alienWin)) * scale(0.62f * alienWin) * rotateY(0.15f * std::sin(tt * 2.f));
            drawAlienHead(R, H);
            Material skin = mat(vec3(0.30f, 0.80f, 0.32f), 0.35f, 30.f, 0.f);           // a little green hand, waving
            float wv = 0.35f * std::sin(tt * 9.f);
            R.draw(A->sphere, M * translate(vec3(0.28f + 0.05f * wv, 23.36f + 0.06f * wv, SHIP_R + 0.07f)) * scale(vec3(0.06f, 0.08f, 0.04f) * alienWin), skin);
        }
        // Kenyan flag decal
        Material decal = mat(vec3(1, 1, 1), 0.15f, 30.f, 0.f); decal.tex = A->flagTex;
        R.draw(A->quad, M * rotateY(0.55f) * translate(vec3(0, 9.f, SHIP_R + 0.06f)) * scale(vec3(2.1f, 1.4f, 1.f)), decal);
        // landing legs (deployed on the Moon)
        if (s.legs > 0.001f) {
            Material leg = mat(vec3(0.62f, 0.63f, 0.65f), 0.7f, 70.f, 0.7f), pad = mat(vec3(0.5f, 0.5f, 0.52f), 0.3f, 40.f, 0.3f);
            for (int k = 0; k < 4; k++) {
                float a = (45.f + 90.f * k) * PI / 180.f; vec3 d(std::cos(a), 0, std::sin(a));
                vec3 top = d * (SHIP_R - 0.05f) + vec3(0, 6.5f, 0);
                vec3 foot = mix(d * (SHIP_R + 0.12f) + vec3(0, 0.6f, 0), d * 6.6f + vec3(0, -LEG_CLEAR + 0.12f, 0), s.legs);
                strut(R, *A, M, top, foot, 0.16f, leg);
                strut(R, *A, M, d * (SHIP_R - 0.05f) + vec3(0, 1.0f, 0), mix(top, foot, 0.6f), 0.10f, leg);
                R.draw(A->cyl, M * translate(foot - vec3(0, 0.12f, 0)) * scale(vec3(0.7f, 0.14f, 0.7f)), pad);
            }
        }
        // elevator: boom, cables, platform with railings (only on the Moon)
        if (s.boom > 0.001f || (tt > 125.f && tt < 199.f)) {
            float zTip = SHIP_R - 0.6f + 0.8f + s.boom * 2.45f, yb = DOOR_Y + 1.45f;
            R.draw(A->box, M * translate(vec3(0, yb, (SHIP_R - 0.6f + zTip) * 0.5f)) * scale(vec3(0.28f, 0.28f, zTip - SHIP_R + 0.6f)), sm);
            float py = s.platY - LEG_CLEAR, zp = elevatorZ(tt);
            Material rail = mat(vec3(0.78f, 0.78f, 0.8f), 0.6f, 60.f, 0.7f), cab = mat(vec3(0.1f, 0.1f, 0.1f), 0.5f, 60.f, 0.f);
            R.draw(A->box, M * translate(vec3(0, py, zp)) * scale(vec3(1.9f, 0.12f, 1.9f)), mat(vec3(0.55f, 0.55f, 0.57f), 0.4f, 40.f, 0.5f));
            for (int sx = -1; sx <= 1; sx += 2) {
                R.draw(A->box, M * translate(vec3(sx * 0.92f, py + 0.55f, zp)) * scale(vec3(0.05f, 1.05f, 1.9f)), rail);
                R.draw(A->box, M * translate(vec3(sx * 0.92f, py + 1.08f, zp)) * scale(vec3(0.07f, 0.07f, 1.9f)), rail);
                strut(R, *A, M, vec3(sx * 0.9f, py + 1.08f, zp), vec3(sx * 0.12f, yb, zTip), 0.018f, cab);
            }
            R.draw(A->box, M * translate(vec3(0, py + 1.08f, zp - 0.92f)) * scale(vec3(1.9f, 0.07f, 0.07f)), rail);
        }
    }
    void drawBooster(Renderer& R, const Frame& fr, dvec3 pos, const Basis& Bb, float hot) const {
        mat4 M = fromBasis(Bb, fr.rel(pos));
        Material steel = mat(vec3(1, 1, 1), 0.85f, 80.f, 0.8f); steel.tex = A->boosterSkin; steel.texMode = 1;
        Material dark = mat(vec3(0.06f, 0.06f, 0.065f), 0.5f, 40.f, 0.3f), fin = mat(vec3(0.25f, 0.25f, 0.27f), 0.6f, 50.f, 0.6f);
        Material bell = mat(vec3(0.30f, 0.22f, 0.17f), 0.8f, 60.f, 0.8f); bell.twoSided = true;
        R.draw(A->cyl, M * scale(vec3(SHIP_R, BOOST_H, SHIP_R)), steel);
        Material ring = dark; ring.emissive = vec3(8.f, 3.2f, 0.9f) * hot;
        R.draw(A->cyl8, M * translate(vec3(0, BOOST_H, 0)) * scale(vec3(SHIP_R * 1.01f, 1.0f, SHIP_R * 1.01f)), ring);
        for (int k = 0; k < 4; k++) {
            float a = (45.f + 90.f * k) * PI / 180.f;
            R.draw(A->box, M * rotateY(-a) * translate(vec3(SHIP_R + 1.1f, BOOST_H - 2.6f, 0)) * scale(vec3(2.2f, 0.18f, 2.4f)), fin);
        }
        for (int sx = -1; sx <= 1; sx += 2) R.draw(A->box, M * translate(vec3(sx * (SHIP_R + 0.15f), 20.f, 0)) * scale(vec3(0.3f, 30.f, 0.5f)), steel);
        R.draw(A->cyl, M * translate(vec3(0, -0.02f, 0)) * scale(vec3(SHIP_R * 1.003f, 0.8f, SHIP_R * 1.003f)), dark);
        for (int i = 0; i < 13; i++) {
            float a = i < 3 ? i * 2 * PI / 3 : (i - 3) * 2 * PI / 10 + 0.3f, r = i < 3 ? 0.9f : 2.55f;
            R.draw(A->nozzle, M * translate(vec3(r * std::cos(a), 0.4f, r * std::sin(a))) * scale(vec3(0.5f, 1.2f, 0.5f)), bell);
        }
    }
    void drawPad(Renderer& R, const Frame& fr, float tt) const {
        mat4 M = fromBasis(W.padB, fr.rel(W.padO));
        Material conc = mat(vec3(0.42f, 0.41f, 0.39f), 0.1f, 10.f, 0.f), steel = mat(vec3(0.55f, 0.56f, 0.58f), 0.5f, 50.f, 0.6f);
        Material truss = mat(vec3(1, 1, 1), 0.3f, 30.f, 0.3f); truss.tex = A->truss; truss.texMode = 2;
        Material white = mat(vec3(0.78f, 0.78f, 0.77f), 0.4f, 40.f, 0.2f), dark = mat(vec3(0.1f, 0.1f, 0.11f), 0.4f, 40.f, 0.3f);
        // launch mount
        R.draw(A->box, M * translate(vec3(0, 0.3f, 0)) * scale(vec3(16.f, 0.6f, 16.f)), dark);
        R.draw(A->cyl, M * translate(vec3(0, MOUNT_H - 2.2f, 0)) * scale(vec3(6.6f, 2.2f, 6.6f)), steel);
        for (int k = 0; k < 6; k++) { float a = k * PI / 3 + 0.3f; R.draw(A->box, M * translate(vec3(7.f * std::cos(a), (MOUNT_H - 2.2f) * 0.5f, 7.f * std::sin(a))) * scale(vec3(1.8f, MOUNT_H - 2.2f, 1.8f)), conc); }
        // the tower: 12 truss sections, crown, lightning rod
        vec3 tp = TOWER_POS;
        for (int k = 0; k < 12; k++) R.draw(A->box, M * translate(tp + vec3(0, 4.5f + 9.f * k, 0)) * scale(9.f), truss);
        R.draw(A->box, M * translate(tp + vec3(0, 108.8f, 0)) * scale(vec3(10.f, 1.6f, 10.f)), steel);
        R.draw(A->cyl, M * translate(tp + vec3(0, 109.6f, 0)) * scale(vec3(0.25f, 14.f, 0.25f)), steel);
        Material flagM = mat(vec3(1, 1, 1), 0.05f, 10.f, 0.f); flagM.tex = A->flagTex; flagM.twoSided = true;
        float wave = 0.05f * std::sin(tt * 3.f);
        R.draw(A->cloth, M * translate(tp + vec3(0.3f, 123.f, 0)) * rotateY(0.6f + wave) * scale(vec3(3.6f, 3.6f, 3.0f)), flagM);
        // chopstick arms + carriage
        R.draw(A->box, M * translate(tp + vec3(0, ARM_H + 0.6f, 0)) * scale(vec3(10.6f, 3.6f, 10.6f)), steel);
        float open = (tt < 285.5f) ? 0.62f : 0.62f * (1.f - sstep(285.5f, 287.8f, tt));
        for (int sd = -1; sd <= 1; sd += 2) {
            float s = (float)sd;
            mat4 Arm = M * translate(tp + vec3(4.6f, ARM_H - 0.4f, s * (SHIP_R + 1.0f))) * rotateY(-s * open);
            for (int k = 0; k < 5; k++) R.draw(A->box, Arm * translate(vec3(2.f + 4.f * k, 0, 0)) * scale(vec3(4.f, 1.6f, 1.0f)), truss);
            R.draw(A->box, Arm * translate(vec3(10.f, 0.82f, 0)) * scale(vec3(20.f, 0.12f, 1.0f)), steel);   // rail the pins land on
        }
        // ship quick-disconnect arm swings away before launch
        float qd = sstep(8.f, 11.f, tt) * 1.35f;
        mat4 Q = M * translate(tp + vec3(4.6f, 75.f, -2.f)) * rotateY(qd);
        R.draw(A->box, Q * translate(vec3(4.f, 0, 0)) * scale(vec3(8.f, 2.f, 2.f)), truss);
        // tank farm, lightning masts, buildings, a flag pole
        for (int k = 0; k < 6; k++) R.draw(A->cyl, M * translate(vec3(-200.f + (k % 3) * 11.f, 0, 125.f + (k / 3) * 12.f)) * scale(vec3(4.f, 22.f, 4.f)), white);
        for (int k = 0; k < 2; k++) R.draw(A->sphere, M * translate(vec3(-160.f, 7.f, 125.f + k * 16.f)) * scale(6.5f), white);
        for (int k = 0; k < 4; k++) { float a = k * PI / 2 + PI / 4; R.draw(A->cyl, M * translate(vec3(70.f * std::cos(a), 0, 70.f * std::sin(a))) * scale(vec3(0.5f, 115.f, 0.5f)), steel); }
        R.draw(A->box, M * translate(vec3(-120.f, 5.f, -60.f)) * scale(vec3(40.f, 10.f, 22.f)), white);
        R.draw(A->box, M * translate(vec3(-90.f, 3.f, 90.f)) * scale(vec3(18.f, 6.f, 12.f)), white);
        R.draw(A->cyl, M * translate(vec3(48.f, 0, 72.f)) * scale(vec3(0.15f, 16.f, 0.15f)), steel);
        R.draw(A->cloth, M * translate(vec3(48.1f, 15.8f, 72.f)) * rotateY(-0.5f + wave * 2.f) * scale(vec3(2.2f, 2.2f, 2.f)), flagM);
        // palms
        Material trunk = mat(vec3(0.30f, 0.21f, 0.12f), 0.1f, 10.f, 0.f), frond = mat(vec3(0.07f, 0.20f, 0.05f), 0.15f, 20.f, 0.f);
        for (size_t i = 0; i < palms.size(); i++) {
            const Palm& p = palms[i];
            float gy = padGroundHeight(p.x, p.z);
            mat4 T = M * translate(vec3(p.x, gy - 0.3f, p.z)) * rotateY(p.rot) * rotateZ(p.lean);
            R.draw(A->cyl, T * scale(vec3(0.28f, p.h, 0.28f)), trunk);
            mat4 C = T * translate(vec3(0, p.h, 0));
            for (int k = 0; k < 8; k++) R.draw(A->sphere, C * rotateY(k * 0.785f + 0.3f * std::sin(tt * 1.3f + k + p.x) * 0.1f) * rotateZ(-0.35f) * translate(vec3(2.6f, 0, 0)) * scale(vec3(2.8f, 0.12f, 0.55f)), frond);
        }
    }
    void drawSite(Renderer& R, const Frame& fr, float tt) const {
        mat4 M = fromBasis(W.siteB, fr.rel(W.siteO));
        R.draw(A->sphere, M * translate(vec3(BOULDER.x, 0.32f, BOULDER.z)) * scale(vec3(1.7f, 0.9f, 1.4f)), mat(W.siteAlb * 1.6f, 0.05f, 8.f, 0.f));
        for (size_t i = 0; i < rocks.size(); i++) {
            const Rock& k = rocks[i];
            R.draw(A->sphere, M * translate(vec3(k.x, moonTerrainHeight(k.x, k.z) + k.s * k.sy * 0.35f, k.z)) * rotateY(k.rot) * scale(vec3(k.s, k.s * k.sy, k.s * 0.8f)), mat(W.siteAlb * (1.5f * k.g), 0.05f, 8.f, 0.f));
        }
        AstroPose a = astroAt(tt);
        if (a.visible) drawAstronaut(R, M, a);
        if (a.poleVisible) drawFlagPole(R, M, a);
        AlienPose al = alienAt(tt);
        if (al.vis) {
            mat4 Ma = M * translate(al.pos) * rotateY(al.heading);
            if (al.pivot) Ma = Ma * rotateX(al.lean) * translate(-alienHands(al.armUp, al.armBack));   // hang from the hands
            drawAlien(R, Ma, tt * 7.f, al.armUp, al.armUp2, al.moving, al.hanging, false, al.armBack);
        }
    }
    // everything opaque - used for the shadow pass AND the lit pass
    void drawObjects(Renderer& R, const Frame& fr) const {
        float tt = fr.t;
        double dPad = dlength(fr.cam - W.padO), dSite = dlength(fr.cam - W.siteO);
        if (dPad < 4.0e5) drawPad(R, fr, tt);
        if (dSite < 2.0e5 && tt > 95.f && tt < 230.f) drawSite(R, fr, tt);
        ShipState s = shipState(tt);
        float win = ramp(tt, 216.5f, 218.f, 225.f, 226.5f);
        drawShip(R, fr, shipPos(tt), shipBasis(tt), s, tt, win);
        BoosterState b = boosterState(tt);
        if (b.vis) drawBooster(R, fr, b.pos, b.B, s.hotStage);
        if (tt > 290.5f) {                                      // home: the stowaway says hello from the airlock
            Basis B = shipBasis(tt); dvec3 sp = shipPos(tt);
            mat4 M = fromBasis(B, fr.rel(sp));
            float out = sstep(291.5f, 293.f, tt);
            R.draw(A->box, M * translate(vec3(0, DOOR_Y - 1.32f, SHIP_R + 0.25f * out)) * scale(vec3(1.7f, 0.12f, 0.6f * out + 0.05f)), mat(vec3(0.55f, 0.55f, 0.57f), 0.4f, 40.f, 0.5f));
            if (out > 0.02f) {
                float wave = 0.75f + 0.25f * std::sin(tt * 8.f);
                drawAlien(R, M * translate(vec3(0.35f, DOOR_Y - 1.26f, SHIP_R + 0.1f + 0.35f * out)) * rotateY(0.25f), tt * 6.f, wave, 0.f, false, false, true);
                AstroPose a; a.visible = true; a.pos = vec3(-0.45f, DOOR_Y - 1.26f, SHIP_R - 0.2f + 0.3f * sstep(293.f, 294.5f, tt)); a.heading = -0.15f;
                a.raiseL = 2.6f * sstep(293.5f, 294.2f, tt); a.spread = 0.4f; a.elbowL = 0.3f * (1.f + std::sin(tt * 7.f));
                if (tt > 293.f) drawAstronaut(R, M, a);
            }
        }
    }
    // additive effects: rocket plumes, hot-staging flash, re-entry plasma
    void drawGlow(Renderer& R, const Frame& fr) const {
        float tt = fr.t;
        ShipState s = shipState(tt);
        dvec3 sp = shipPos(tt); Basis B = shipBasis(tt);
        mat4 M = fromBasis(B, fr.rel(sp));
        float hE = (float)(dlength(sp) - R_EARTH);
        float air = std::exp(-std::max(hE, 0.f) / 8000.f);
        if (s.ship > 0.01f) {
            bool seaLevel = air > 0.05f;
            if (seaLevel) {
                for (int i = 0; i < 3; i++) {
                    float a = 120.f * i * PI / 180.f;
                    mat4 N = M * translate(vec3(1.05f * std::cos(a), -1.05f, 1.05f * std::sin(a)));
                    R.glow(A->plume, N * scale(vec3(0.75f, 14.f + 5.f * s.ship, 0.75f)), vec3(5.f, 4.2f, 3.6f), vec3(3.4f, 1.3f, 0.55f), 1.6f * s.ship, 1.f, 0, vec3(), (float)i);
                    R.glow(A->plume, N * scale(vec3(0.42f, 4.f, 0.42f)), vec3(7.f, 6.f, 5.5f), vec3(5.f, 3.f, 1.6f), 2.0f * s.ship, 0.f, 0, vec3(), 3.f + i);
                }
            }
            else {                                             // vacuum: wide, translucent torch plume
                for (int i = 0; i < 3; i++) {
                    float a = (60.f + 120.f * i) * PI / 180.f;
                    mat4 N = M * translate(vec3(2.25f * std::cos(a), -1.6f, 2.25f * std::sin(a)));
                    R.glow(A->plume, N * scale(vec3(1.2f, 7.f, 1.2f)), vec3(6.f, 6.f, 7.5f), vec3(1.6f, 1.4f, 3.2f), 1.4f * s.ship, 0.f, 0, vec3(), (float)i);
                }
                R.glow(A->plume, M * translate(vec3(0, -2.5f, 0)) * scale(vec3(9.f, 70.f, 9.f)), vec3(1.6f, 1.6f, 2.6f), vec3(0.35f, 0.3f, 0.9f), 0.9f * s.ship, 0.f, 0, vec3(), 7.f);
            }
        }
        BoosterState b = boosterState(tt);
        if (b.vis && b.plume > 0.01f) {
            mat4 Mb = fromBasis(b.B, fr.rel(b.pos));
            float hb = (float)(dlength(b.pos) - R_EARTH), airB = std::exp(-std::max(hb, 0.f) / 8000.f);
            float spread = 1.f + 5.f * (1.f - airB);
            R.glow(A->plume, Mb * translate(vec3(0, -0.8f, 0)) * scale(vec3(3.9f * spread, 38.f + 50.f * (1.f - airB), 3.9f * spread)), vec3(6.f, 4.8f, 3.6f), vec3(3.6f, 1.4f, 0.5f), 1.5f * b.plume * (0.5f + 0.5f * airB), airB, 0, vec3(), 11.f);
            R.glow(A->plume, Mb * translate(vec3(0, -0.8f, 0)) * scale(vec3(3.1f, 9.f, 3.1f)), vec3(9.f, 7.f, 5.f), vec3(6.f, 3.f, 1.f), 1.8f * b.plume, 0.f, 0, vec3(), 12.f);
        }
        float hot = s.hotStage;
        if (hot > 0.01f) R.glow(A->sphere, M * translate(vec3(0, -0.5f, 0)) * scale(vec3(5.5f, 1.6f, 5.5f)), vec3(9.f, 5.f, 2.f), vec3(6.f, 2.f, 0.6f), hot * 1.4f, 0.f, 1, -B.y, 13.f);
        float heat = ramp(tt, 249.f, 253.f, 262.f, 269.f);
        if (heat > 0.01f) {
            vec3 vd = tof(dnormalize(shipVel(tt)));
            mat4 E = M * translate(vec3(0, 17.f, -1.2f)) * scale(vec3(SHIP_R * 2.3f, 21.f, SHIP_R * 2.0f));
            R.glow(A->sphere, E, vec3(6.f, 3.2f, 2.f), vec3(3.5f, 0.9f, 1.2f), heat * 1.3f, 0.f, 1, vd, 17.f);
            mat4 Wk = fromBasis(basisYZ(vd, B.z), fr.rel(sp + dvec3(B.y) * 17.0));
            R.glow(A->plume, Wk * translate(vec3(0, -6.f, 0)) * scale(vec3(16.f, 170.f, 16.f)), vec3(3.f, 1.2f, 0.8f), vec3(1.4f, 0.35f, 0.5f), heat * 0.55f, 0.f, 0, vec3(), 19.f);
        }
    }
};

// ============================================================================
//  19. 2D OVERLAY: captions, telemetry, title, credits, YouTube-style player
// ============================================================================
static const char* CREDITS[] = {
    "MOON LANDING",
    "Mission HARAMBEE-1",
    "A Computer Graphics Project  -  C++ and OpenGL",
    "",
    "Group members",
    "Maina Jeremy Gachanga  -  190280",
    "Ng'ang'a Austin Thayu  -  181807",
    "Gitonga Collins Kinoti  -  192560",
    "Muiya Adrian Kioko  -  178619",
    "",
    "Asante sana for watching!",
};
static const int N_CREDITS = sizeof(CREDITS) / sizeof(CREDITS[0]);

struct Caption { float a, b; const char* t; };
static const Caption CAPS[] = {
    {   2.0f,   6.5f, "Malindi, Kenya - the HARAMBEE-1 stack is fuelled and ready" },
    {   7.0f,  11.8f, "Terminal count: the crew access arm swings clear" },
    {  12.0f,  15.0f, "Engine ignition" },
    {  15.5f,  24.0f, "Liftoff! 33 engines at full thrust" },
    {  24.5f,  33.5f, "Climbing through the atmosphere - the sky fades to black" },
    {  34.0f,  39.3f, "Fusion torch drive armed" },
    {  39.5f,  46.5f, "Hot staging! The booster falls away" },
    {  48.5f,  58.0f, "Trans-lunar burn: 100 g, straight for the Moon" },
    {  58.5f,  66.0f, "Halfway there - engines off" },
    {  66.5f,  72.0f, "Flip manoeuvre: 180 degrees, engines first" },
    {  72.5f,  84.0f, "Braking burn - the Moon swells ahead" },
    {  84.5f,  96.0f, "Pitching over for powered descent" },
    {  96.5f, 105.0f, "Landing legs deployed" },
    { 105.5f, 113.0f, "Engine blast scours the regolith" },
    { 113.5f, 118.5f, "Contact light... touchdown!" },
    { 119.0f, 126.0f, "HARAMBEE-1 has landed on the Moon" },
    { 126.5f, 133.0f, "The airlock opens" },
    { 133.5f, 140.0f, "The crew elevator lowers to the surface" },
    { 140.5f, 146.0f, "One small step for Kenya..." },
    { 146.5f, 153.0f, "Carrying the flag of Kenya across the Moon" },
    { 153.5f, 161.0f, "Planting the flag - the first Kenyan flag on the Moon!" },
    { 161.5f, 166.5f, "Harambee! Pulling together, even on the Moon" },
    { 167.0f, 171.2f, "Low gravity: one-sixth of Earth's" },
    { 171.5f, 175.0f, "...wait. Did something move?" },
    { 175.5f, 185.5f, "Time to head home" },
    { 186.0f, 196.0f, "Going up. (Is that... a passenger?)" },
    { 197.0f, 201.0f, "Airlock sealed - engines arming" },
    { 201.0f, 209.5f, "Liftoff from the Moon" },
    { 210.0f, 216.0f, "Pitching over towards the Earth" },
    { 216.5f, 225.5f, "We may have picked up a stowaway..." },
    { 226.0f, 235.0f, "Trans-Earth burn - flip at the midpoint" },
    { 235.5f, 247.5f, "Braking for Earth - home is getting bigger" },
    { 248.0f, 258.0f, "Re-entry! 1,600 degrees on the heat shield" },
    { 258.5f, 268.0f, "Communications blackout over the Indian Ocean" },
    { 268.5f, 279.0f, "Belly-flop: falling sideways, steering with the flaps" },
    { 279.5f, 285.0f, "Flip and landing burn" },
    { 285.5f, 290.0f, "Catch! The tower arms grab the ship" },
    { 290.5f, 297.0f, "Welcome home... and karibu Kenya, little friend!" },
};

struct UI {
    std::vector<float> v; GLuint vao = 0, vbo = 0; size_t cap = 0; float W = 1280, H = 720;
    void init() {
        glGenVertexArrays(1, &vao); glBindVertexArray(vao);
        glGenBuffers(1, &vbo); glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glEnableVertexAttribArray(0); glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 24, (void*)0);
        glEnableVertexAttribArray(1); glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 24, (void*)8);
        glBindVertexArray(0);
    }
    float s() const { return H / 720.f; }
    void vert(float x, float y, float r, float g, float b, float a) { float q[6] = { x, y, r, g, b, a }; v.insert(v.end(), q, q + 6); }
    void quad(float x0, float y0, float x1, float y1, float r, float g, float b, float a) {
        vert(x0, y0, r, g, b, a); vert(x1, y0, r, g, b, a); vert(x1, y1, r, g, b, a);
        vert(x0, y0, r, g, b, a); vert(x1, y1, r, g, b, a); vert(x0, y1, r, g, b, a);
    }
    void seg(float ax, float ay, float bx, float by, float w, float r, float g, float b, float a) {
        float dx = bx - ax, dy = by - ay, l = std::sqrt(dx * dx + dy * dy);
        if (l < 1e-4f) { dx = 1; dy = 0; l = 1; }
        dx /= l; dy /= l; float nx = -dy * w * 0.5f, ny = dx * w * 0.5f;
        ax -= dx * w * 0.5f; ay -= dy * w * 0.5f; bx += dx * w * 0.5f; by += dy * w * 0.5f;
        vert(ax + nx, ay + ny, r, g, b, a); vert(bx + nx, by + ny, r, g, b, a); vert(bx - nx, by - ny, r, g, b, a);
        vert(ax + nx, ay + ny, r, g, b, a); vert(bx - nx, by - ny, r, g, b, a); vert(ax - nx, ay - ny, r, g, b, a);
    }
    static float width(const char* t, float h) { int n = (int)std::strlen(t); return n > 0 ? (n * 5.5f - 1.5f) * h / 6.f : 0.f; }
    // align: 0 left, 1 centre, 2 right.  y = baseline
    void text(const char* t, float x, float y, float h, float lw, float r, float g, float b, float a, int align = 0, bool shadow = true) {
        float sc = h / 6.f, w = width(t, h); lw = std::max(lw, 1.3f);
        float x0 = align == 1 ? x - w / 2 : (align == 2 ? x - w : x);
        for (int pass = shadow ? 0 : 1; pass < 2; pass++) {
            float ox = pass == 0 ? lw * 0.6f : 0.f, oy = pass == 0 ? -lw * 0.6f : 0.f;
            float cr = pass == 0 ? 0.f : r, cg = pass == 0 ? 0.f : g, cb = pass == 0 ? 0.f : b, ca = pass == 0 ? a * 0.55f : a;
            for (int k = 0; t[k]; k++) {
                float gx = x0 + k * 5.5f * sc + ox;
                forGlyphSegments(t[k], [&](float ax, float ay, float bx, float by) { seg(gx + ax * sc, y + ay * sc + oy, gx + bx * sc, y + by * sc + oy, lw, cr, cg, cb, ca); });
            }
        }
    }
    void flush(Program& p) {
        if (v.empty()) return;
        glBindVertexArray(vao); glBindBuffer(GL_ARRAY_BUFFER, vbo);
        if (v.size() > cap) { cap = v.size() * 2; glBufferData(GL_ARRAY_BUFFER, cap * sizeof(float), nullptr, GL_DYNAMIC_DRAW); }
        glBufferSubData(GL_ARRAY_BUFFER, 0, v.size() * sizeof(float), v.data());
        p.use(); p.set("uRes", W, H);
        glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); glDisable(GL_DEPTH_TEST); glDisable(GL_CULL_FACE);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(v.size() / 6));
        glDisable(GL_BLEND);
        v.clear();
    }
};

static void kenyaStripes(UI& u, float cx, float y, float w, float a) {
    float k = u.s(); float h[5] = { 6.f * k, 1.5f * k, 7.f * k, 1.5f * k, 6.f * k };
    float c[5][3] = { {0.03f,0.03f,0.03f}, {1,1,1}, {0.8f,0.07f,0.07f}, {1,1,1}, {0.f,0.46f,0.2f} };
    float yy = y;
    for (int i = 0; i < 5; i++) { u.quad(cx - w / 2, yy - h[i], cx + w / 2, yy, c[i][0], c[i][1], c[i][2], a); yy -= h[i]; }
}
static std::string thousands(long v) {
    char b[32]; std::snprintf(b, sizeof b, "%ld", v); std::string s(b), r; int n = (int)s.size();
    for (int i = 0; i < n; i++) { r += s[i]; if ((n - 1 - i) % 3 == 0 && i != n - 1) r += ' '; }
    return r;
}
static bool g_captions = true;
static void drawHUD(UI& u, float t) {
    float k = u.s(), cx = u.W / 2;
    if (t < 7.f) {                                                        // title card
        float a = sstep(1.0f, 2.5f, t) * (1.f - sstep(5.5f, 7.f, t));
        u.text("MOON LANDING", cx, u.H * 0.60f, u.H * 0.12f, 3.2f * k, 1, 1, 1, a, 1);
        kenyaStripes(u, cx, u.H * 0.565f, u.W * 0.30f, a);
        u.text("Mission HARAMBEE-1", cx, u.H * 0.47f, 22.f * k, 2.2f * k, 0.85f, 0.9f, 1.f, a, 1);
    }
    if (t >= 6.f && t < T_LIFT) {
        char b[32]; std::snprintf(b, sizeof b, "T - %d", (int)std::ceil(T_LIFT - t));
        float a = sstep(6.f, 6.5f, t);
        u.text(b, cx, u.H * 0.82f, u.H * 0.08f, 3.f * k, 1, 1, 1, 0.92f * a, 1);
    }
    if (t >= T_LIFT && t < T_LIFT + 3.5f) {
        float a = sstep(T_LIFT, T_LIFT + 0.3f, t) * (1.f - sstep(T_LIFT + 2.3f, T_LIFT + 3.5f, t));
        u.text("LIFTOFF!", cx, u.H * 0.82f, u.H * 0.08f, 3.f * k, 1.f, 0.82f, 0.35f, a, 1);
    }
    // telemetry (SpaceX-webcast style), only while flying
    float tel = ramp(t, T_LIFT + 1.f, T_LIFT + 2.f, T_TOUCH + 1.f, T_TOUCH + 2.5f) + ramp(t, T_MLIFT + 1.f, T_MLIFT + 2.f, T_CATCH + 1.f, T_CATCH + 2.5f);
    if (tel > 0.01f) {
        dvec3 p = shipPos(t);
        double hE = dlength(p) - R_EARTH, hM = dlength(p - W.moonC) - R_MOON;
        double alt = std::min(hE, hM), spd = dlength(shipVel(t)) * 3.6;
        if (hM < hE && t > 100.f && t < T_TOUCH + 3.f) alt = ddot(p - W.siteO, dvec3(W.siteB.y)) - LEG_CLEAR;
        if (t > 270.f) alt = ddot(p - W.padO, dvec3(W.padB.y));
        int mt = (int)std::max(0.f, t - T_LIFT);
        char b1[64], b2[64], b3[64];
        std::snprintf(b1, sizeof b1, "T+ 00:%02d:%02d", mt / 60, mt % 60);
        std::snprintf(b2, sizeof b2, "SPEED  %s KM/H", thousands((long)spd).c_str());
        if (alt > 2000.0) std::snprintf(b3, sizeof b3, "ALTITUDE  %s KM", thousands((long)(alt / 1000.0)).c_str());
        else std::snprintf(b3, sizeof b3, "ALTITUDE  %d M", (int)std::max(0.0, alt));
        float x0 = 24.f * k, y0 = u.H - 34.f * k;
        u.quad(x0 - 10 * k, y0 - 58 * k, x0 + 290 * k, y0 + 22 * k, 0, 0, 0, 0.35f * tel);
        u.text("HARAMBEE-1", x0, y0, 15.f * k, 1.9f * k, 1.f, 0.82f, 0.35f, tel);
        u.text(b2, x0, y0 - 24 * k, 13.f * k, 1.6f * k, 1, 1, 1, tel);
        u.text(b3, x0, y0 - 46 * k, 13.f * k, 1.6f * k, 1, 1, 1, tel);
        u.text(b1, u.W - 24.f * k, y0, 17.f * k, 2.0f * k, 1, 1, 1, tel, 2);
        if (t > T_TLI && t < T_ARRIVE + 2.f) {
            char b4[64]; std::snprintf(b4, sizeof b4, "MOON  %s KM", thousands((long)(hM / 1000.0)).c_str());
            u.text(b4, u.W - 24.f * k, y0 - 26 * k, 13.f * k, 1.6f * k, 0.8f, 0.85f, 1.f, tel, 2);
        }
        if (t > T_LASC && t < T_ENTRY + 2.f) {
            char b4[64]; std::snprintf(b4, sizeof b4, "EARTH  %s KM", thousands((long)(hE / 1000.0)).c_str());
            u.text(b4, u.W - 24.f * k, y0 - 26 * k, 13.f * k, 1.6f * k, 0.8f, 0.85f, 1.f, tel, 2);
        }
    }
    if (g_captions) for (size_t i = 0; i < sizeof(CAPS) / sizeof(CAPS[0]); i++) {     // lower-third captions
        const Caption& c = CAPS[i];
        if (t < c.a || t >= c.b) continue;
        float a = sstep(c.a, c.a + 0.5f, t) * (1.f - sstep(c.b - 0.5f, c.b, t));
        float y0 = u.H * 0.085f, w = UI::width(c.t, 17.f * k) + 48.f * k;
        u.quad(cx - w / 2, y0 - 2 * k, cx + w / 2, y0 + 46.f * k, 0, 0, 0, 0.42f * a);
        u.text(c.t, cx, y0 + 15.f * k, 17.f * k, 1.9f * k, 1, 1, 1, a, 1);
    }
    if (t > T_CREDITS) {                                                  // credits roll over the final shot
        float tc = t - T_CREDITS, sp = 34.f * k, y0 = u.H * 0.78f;
        float band = sstep(0.f, 1.f, tc);
        u.quad(cx - 330 * k, y0 - (N_CREDITS + 0.6f) * sp, cx + 330 * k, y0 + 70 * k, 0, 0, 0, 0.45f * band);
        for (int i = 0; i < N_CREDITS; i++) {
            if (!CREDITS[i][0]) continue;
            float a = sstep(0.4f + i * 0.45f, 1.2f + i * 0.45f, tc);
            float y = y0 - i * sp - (i >= 1 ? 16.f * k : 0.f);
            if (i == 0) { u.text(CREDITS[0], cx, y, u.H * 0.065f, 2.6f * k, 1, 1, 1, a, 1); kenyaStripes(u, cx, y - 10.f * k, u.W * 0.28f, a); }
            else if (i == 4) u.text(CREDITS[i], cx, y, 17.f * k, 1.9f * k, 1.f, 0.82f, 0.35f, a, 1);
            else u.text(CREDITS[i], cx, y, 16.f * k, 1.8f * k, 0.88f, 0.9f, 1.f, a, 1);
        }
    }
}

// ---- player state + controls
static bool g_paused = false, g_showTimer = false, g_showHelp = false, g_dragging = false;
static const float SPEEDS[5] = { 0.25f, 0.5f, 1.f, 2.f, 4.f };
static int g_speed = 2;
static double g_lastActivity = -100.0, g_flashUntil = 0.0;
static char g_flash[32] = ""; static float g_flashX = 0.5f;
static float g_seekTo = -1.f;                 // pending full seek
static float g_target = 0.f;                  // film time requested by the UI

static float barX0(const UI& u) { return 24.f * u.s(); }
static float barX1(const UI& u) { return u.W - 24.f * u.s(); }
static float barZone(const UI& u) { return 56.f * u.s(); }
static void drawPlayer(UI& u, float t, double now) {
    float k = u.s(), cx = u.W / 2, cy = u.H / 2;
    bool show = g_paused || g_dragging || (now - g_lastActivity < 3.0);
    if (show) {
        float x0 = barX0(u), x1 = barX1(u), by = 14.f * k, bh = 5.f * k, fr = clampf(t / T_END);
        u.quad(0, 0, u.W, barZone(u), 0, 0, 0, 0.55f);
        u.quad(x0, by, x1, by + bh, 1, 1, 1, 0.28f);
        u.quad(x0, by, x0 + (x1 - x0) * fr, by + bh, 0.92f, 0.1f, 0.1f, 1);
        for (int i = 1; i < 9; i++) { float xm = x0 + (x1 - x0) * CHAPTER[i] / T_END; u.quad(xm - k, by - 2 * k, xm + k, by + bh + 2 * k, 1, 1, 1, 0.9f); }
        float kx = x0 + (x1 - x0) * fr;
        u.quad(kx - 4 * k, by - 4 * k, kx + 4 * k, by + bh + 4 * k, 1, 1, 1, 1);
        char b[96]; int s = (int)t, tt = (int)T_END, ch = 0;
        for (int i = 0; i < 9; i++) if (t >= CHAPTER[i]) ch = i;
        std::snprintf(b, sizeof b, "%d:%02d / %d:%02d   %s", s / 60, s % 60, tt / 60, tt % 60, CHAPTER_NAME[ch]);
        u.text(b, x0, by + bh + 9 * k, 12.f * k, 1.5f * k, 1, 1, 1, 1, 0, false);
        std::snprintf(b, sizeof b, "%gx", SPEEDS[g_speed]);
        u.text(b, x1, by + bh + 9 * k, 12.f * k, 1.5f * k, 1, 1, 1, 0.9f, 2, false);
        u.text("H = controls", cx, by + bh + 9 * k, 12.f * k, 1.4f * k, 1, 1, 1, 0.7f, 1, false);
    }
    if (g_paused) {
        u.quad(cx - 60 * k, cy - 60 * k, cx + 60 * k, cy + 60 * k, 0, 0, 0, 0.35f);
        u.quad(cx - 26 * k, cy - 32 * k, cx - 8 * k, cy + 32 * k, 1, 1, 1, 0.9f);
        u.quad(cx + 8 * k, cy - 32 * k, cx + 26 * k, cy + 32 * k, 1, 1, 1, 0.9f);
    }
    if (now < g_flashUntil) {
        float a = clampf((float)(g_flashUntil - now) / 0.35f), fx = u.W * g_flashX;
        u.quad(fx - 70 * k, cy - 28 * k, fx + 70 * k, cy + 28 * k, 0, 0, 0, 0.45f * a);
        u.text(g_flash, fx, cy - 10 * k, 22.f * k, 2.4f * k, 1, 1, 1, a, 1, false);
    }
    if (t < 9.f && !g_showHelp && !g_paused) u.text("Press H for controls", cx, u.H - 30 * k, 13.f * k, 1.5f * k, 1, 1, 1, 0.6f * (1.f - sstep(6.f, 9.f, t)), 1, false);
    if (g_showTimer) { char b[64]; std::snprintf(b, sizeof b, "t = %.1f s", t); u.text(b, 10 * k, u.H - 60 * k, 16.f * k, 1.8f * k, 1, 1, 0, 1); }
    if (g_showHelp) {
        static const char* L[] = { "CONTROLS", "SPACE / K / ENTER  -  PAUSE / PLAY", "LEFT / RIGHT  -  BACK / FORWARD 5 S", "J / L  -  BACK / FORWARD 10 S",
            "UP / DOWN  -  PLAYBACK SPEED", ", / .  -  STEP ONE FRAME (WHEN PAUSED)", "HOME / END  -  START / CREDITS", "1 TO 9  -  JUMP TO CHAPTER",
            "CLICK OR DRAG THE PROGRESS BAR", "C CAPTIONS   T TIMER   F FULLSCREEN", "R RESTART   ESC QUIT", "PRESS H TO CLOSE" };
        const int n = sizeof(L) / sizeof(L[0]);
        float lh = 28.f * k, pw = 560.f * k, ph = (n + 1) * lh;
        u.quad(cx - pw / 2, cy - ph / 2, cx + pw / 2, cy + ph / 2, 0, 0, 0, 0.78f);
        for (int i = 0; i < n; i++) {
            float r = 1, g = 1, b = 1, a = i == n - 1 ? 0.6f : 0.95f;
            if (i == 0) { g = 0.82f; b = 0.35f; }
            u.text(L[i], cx, cy + ph / 2 - (i + 1.3f) * lh, (i == 0 ? 18.f : 13.f) * k, 1.7f * k, r, g, b, a, 1, false);
        }
    }
}

// ============================================================================
//  20. FRAME RENDERING
// ============================================================================
static float filmFade(float t) { return sstep(0.f, 2.5f, t) * (1.f - sstep(T_END - 2.5f, T_END, t)); }

static void renderFrame(Renderer& R, Film& film, float t, GLuint outFBO) {
    Frame f; f.t = t;
    Cam c = cameraAt(t);
    f.cam = c.eye; f.fov = c.fov * PI / 180.f; f.aspect = (float)R.outW / (float)R.outH;
    vec3 fwd = tof(dnormalize(c.target - c.eye));
    f.view = lookDir(vec3(), fwd, c.up);
    f.proj = perspective(f.fov, f.aspect, 0.1f, 1.0e9f);
    f.viewProj = f.proj * f.view;
    f.env = computeEnv(f.cam);
    R.F = &f;

    // ---- 1. sun shadow map around the action (texel-snapped in world space -> no shimmering)
    float wPad = clampf((1.f - sstep(18.f, 32.f, t)) + sstep(276.f, 284.f, t));
    float wSite = sstep(100.f, 110.f, t) * (1.f - sstep(204.f, 214.f, t));
    float wShip = std::max(0.f, 1.f - wPad - wSite);
    dvec3 focus = padP(dvec3(0, 55, 0)) * wPad + siteP(dvec3(2, 9, 4)) * wSite + (shipPos(t) + dvec3(shipBasis(t).y) * 17.0) * wShip;
    float E = 115.f * wPad + 58.f * wSite + 42.f * wShip;
    R.shadowE = E;
    {
        vec3 lf = -W.sun, upRef = std::fabs(W.sun.y) > 0.95f ? vec3(1, 0, 0) : vec3(0, 1, 0);
        vec3 right = normalize(cross(lf, upRef)), up = cross(right, lf);
        double texel = 2.0 * E / R.shadowRes;
        double cx = std::floor(ddot(focus, dvec3(right)) / texel + 0.5) * texel, cy = std::floor(ddot(focus, dvec3(up)) / texel + 0.5) * texel;
        dvec3 cW = dvec3(right) * cx + dvec3(up) * cy + dvec3(lf) * ddot(focus, dvec3(lf));
        vec3 cRel = tof(cW - f.cam);
        R.lightVP = ortho(-E, E, -E, E, 1.f, 1600.f) * lookDir(cRel + W.sun * 800.f, lf, up);
        glBindFramebuffer(GL_FRAMEBUFFER, R.shadowFBO);
        glViewport(0, 0, R.shadowRes, R.shadowRes);
        glClear(GL_DEPTH_BUFFER_BIT);
        R.inShadow = true; glEnable(GL_DEPTH_TEST); glDepthMask(GL_TRUE); glDepthFunc(GL_LEQUAL); glEnable(GL_CULL_FACE); glCullFace(GL_FRONT);
        glEnable(GL_POLYGON_OFFSET_FILL); glPolygonOffset(2.f, 4.f);
        R.pShadow.use(); R.pShadow.set("uLightVP", R.lightVP);
        film.drawObjects(R, f);
        glDisable(GL_POLYGON_OFFSET_FILL); glCullFace(GL_BACK);
        R.inShadow = false;
    }
    // ---- 2. HDR scene
    glBindFramebuffer(GL_FRAMEBUFFER, R.msaaFBO);
    glViewport(0, 0, R.outW, R.outH);
    glClearColor(0, 0, 0, 1); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_MULTISAMPLE); glEnable(GL_PROGRAM_POINT_SIZE);
    // stars (no parallax)
    glDisable(GL_DEPTH_TEST); glDepthMask(GL_FALSE);
    if (f.env.starGain > 0.01f) {
        R.pStars.use(); R.pStars.set("uViewProjRot", f.viewProj); R.pStars.set("uGain", f.env.starGain);
        glBindVertexArray(R.A->starVAO); glDrawArrays(GL_POINTS, 0, R.A->starCount);
    }
    // the Sun, the Earth with its atmosphere and clouds, the Moon - ray traced
    {
        R.pPlanet.use(); R.bindCommon(R.pPlanet); R.bindPlanetUniforms(R.pPlanet);
        mat4 projRay = perspective(f.fov, f.aspect, 1.f, 1000.f);
        R.pPlanet.set("uInvVP", inverse(projRay * f.view));
        dvec3 mc = W.moonC - f.cam;
        R.pPlanet.set("uMC", tof(mc)); R.pPlanet.set("uMUp", tof(dnormalize(f.cam - W.moonC))); R.pPlanet.set("uMH", (float)(dlength(f.cam - W.moonC) - R_MOON));
        R.pPlanet.set("uMoonRot", W.moonB);
        glEnable(GL_BLEND); glBlendFunc(GL_ONE, GL_SRC_ALPHA);
        R.fullscreen();
        glDisable(GL_BLEND);
    }
    glEnable(GL_DEPTH_TEST); glDepthMask(GL_TRUE); glDepthFunc(GL_LEQUAL); glEnable(GL_CULL_FACE); glCullFace(GL_BACK);
    // terrain patches that continue the globes seamlessly
    if (dlength(f.cam - W.padO) < 2.5e6) {
        R.pGround.use(); R.bindCommon(R.pGround); R.bindPlanetUniforms(R.pGround);
        R.pGround.set("uModel", fromBasis(W.padB, f.rel(W.padO)));
        glDisable(GL_CULL_FACE); R.A->padGround.draw(); glEnable(GL_CULL_FACE);
    }
    if (dlength(f.cam - W.siteO) < 1.5e6) {
        R.pTerrain.use(); R.bindCommon(R.pTerrain); R.bindPlanetUniforms(R.pTerrain);
        R.pTerrain.set("uModel", fromBasis(W.siteB, f.rel(W.siteO))); R.pTerrain.set("uSiteAlb", W.siteAlb);
        glDisable(GL_CULL_FACE); R.A->moonTerrain.draw(); glEnable(GL_CULL_FACE);
    }
    // objects
    R.pLit.use(); R.bindCommon(R.pLit);
    film.drawObjects(R, f);
    // additive glow (plumes, plasma)
    glDisable(GL_CULL_FACE); glDepthMask(GL_FALSE); glEnable(GL_BLEND); glBlendFunc(GL_ONE, GL_ONE);
    R.pGlow.use(); R.pGlow.set("uViewProj", f.viewProj); R.pGlow.set("uLogC", R.logC); R.pGlow.set("uTime", t);
    R.pGlow.set("uLightVP", R.lightVP); R.pGlow.set("uNormalOffset", 0.f);
    film.drawGlow(R, f);
    // particles
    {
        static std::vector<float> bufA, bufB; bufA.clear(); bufB.clear();
        vec3 org[3] = { f.rel(W.padO), f.rel(W.siteO), f.rel(shipPos(t)) };
        Basis bas[3] = { W.padB, W.siteB, Basis() };
        for (size_t i = 0; i < film.P->ps.size(); i++) {
            const Particle& q = film.P->ps[i];
            vec3 p = org[q.frame] + bas[q.frame] * q.p;
            float age = q.maxLife - q.life;
            float a = q.alpha * std::min(1.f, q.life * 1.2f) * std::min(1.f, age * 10.f);
            float tint = 1.f;
            if (!q.additive && q.frame != FR_SITE) tint = 1.f;
            std::vector<float>& b = q.additive ? bufB : bufA;
            float v8[8] = { p.x, p.y, p.z, q.size, q.col.x * tint, q.col.y * tint, q.col.z * tint, a };
            b.insert(b.end(), v8, v8 + 8);
        }
        R.pPart.use(); R.pPart.set("uViewProj", f.viewProj); R.pPart.set("uLogC", R.logC);
        R.pPart.set("uProjScale", R.outH / (2.f * std::tan(f.fov * 0.5f)));
        glBindVertexArray(film.P->vao); glBindBuffer(GL_ARRAY_BUFFER, film.P->vbo);
        if (!bufA.empty()) {
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glBufferSubData(GL_ARRAY_BUFFER, 0, bufA.size() * sizeof(float), bufA.data());
            glDrawArrays(GL_POINTS, 0, (GLsizei)(bufA.size() / 8));
        }
        if (!bufB.empty()) {
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            glBufferSubData(GL_ARRAY_BUFFER, 0, bufB.size() * sizeof(float), bufB.data());
            glDrawArrays(GL_POINTS, 0, (GLsizei)(bufB.size() / 8));
        }
    }
    glDisable(GL_BLEND); glDepthMask(GL_TRUE);
    // ---- 3. resolve, bloom, tone map
    glBindFramebuffer(GL_READ_FRAMEBUFFER, R.msaaFBO); glBindFramebuffer(GL_DRAW_FRAMEBUFFER, R.resolveFBO);
    glBlitFramebuffer(0, 0, R.outW, R.outH, 0, 0, R.outW, R.outH, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glDisable(GL_DEPTH_TEST); glDisable(GL_CULL_FACE);
    glBindFramebuffer(GL_FRAMEBUFFER, R.bloomFBO[0]); glViewport(0, 0, R.bw, R.bh);
    R.pBright.use(); glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, R.resolveTex);
    R.pBright.set("uSrc", 0); R.pBright.set("uTexel", 1.f / R.outW, 1.f / R.outH); R.pBright.set("uThresh", 3.0f);
    R.fullscreen();
    R.pBlur.use(); R.pBlur.set("uSrc", 0);
    for (int i = 0; i < 3; i++) {
        float kk = 1.f + i * 1.3f;
        glBindFramebuffer(GL_FRAMEBUFFER, R.bloomFBO[1]); glBindTexture(GL_TEXTURE_2D, R.bloomTex[0]); R.pBlur.set("uDir", kk / R.bw, 0.f); R.fullscreen();
        glBindFramebuffer(GL_FRAMEBUFFER, R.bloomFBO[0]); glBindTexture(GL_TEXTURE_2D, R.bloomTex[1]); R.pBlur.set("uDir", 0.f, kk / R.bh); R.fullscreen();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, outFBO); glViewport(0, 0, R.outW, R.outH);
    R.pComposite.use();
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, R.resolveTex); R.pComposite.set("uHdr", 0);
    glActiveTexture(GL_TEXTURE0 + 1); glBindTexture(GL_TEXTURE_2D, R.bloomTex[0]); R.pComposite.set("uBloom", 1);
    R.pComposite.set("uExposure", f.env.exposure); R.pComposite.set("uBloomAmt", 0.30f);
    R.pComposite.set("uFade", filmFade(t)); R.pComposite.set("uTime", std::fmod(t, 10.f)); R.pComposite.set("uRes", (float)R.outW, (float)R.outH);
    R.fullscreen();
    glActiveTexture(GL_TEXTURE0);
    R.F = nullptr;
}

// ============================================================================
//  21. WINDOW, INPUT, MAIN LOOP
// ============================================================================
static std::vector<float> g_shotList;
struct App { Film* film; UI* ui; GLFWwindow* win; bool fullscreen = false; int sx = 100, sy = 100, sw = 1280, sh = 720; };
static App* g_app = nullptr;
static void flash(const char* s, float x) { std::snprintf(g_flash, sizeof g_flash, "%s", s); g_flashX = x; g_flashUntil = glfwGetTime() + 0.8; }
static void requestSeek(float s) { g_target = clampf(s, 0.f, T_END); g_seekTo = g_target; }
static void onKey(GLFWwindow* w, int key, int, int action, int) {
    if (action == GLFW_RELEASE) return;
    bool press = action == GLFW_PRESS;
    g_lastActivity = glfwGetTime();
    float cur = g_target;
    switch (key) {
    case GLFW_KEY_ESCAPE: if (press) glfwSetWindowShouldClose(w, 1); break;
    case GLFW_KEY_ENTER: case GLFW_KEY_KP_ENTER: case GLFW_KEY_SPACE: case GLFW_KEY_K:
        if (press) { if (cur >= T_END) { requestSeek(0.f); g_paused = false; } else g_paused = !g_paused; } break;
    case GLFW_KEY_RIGHT: requestSeek(cur + 5.f); flash("+5 S", 0.78f); break;
    case GLFW_KEY_LEFT: requestSeek(cur - 5.f); flash("-5 S", 0.22f); break;
    case GLFW_KEY_L: requestSeek(cur + 10.f); flash("+10 S", 0.78f); break;
    case GLFW_KEY_J: requestSeek(cur - 10.f); flash("-10 S", 0.22f); break;
    case GLFW_KEY_UP: if (press && g_speed < 4) { g_speed++; char b[16]; std::snprintf(b, sizeof b, "%gX", SPEEDS[g_speed]); flash(b, 0.5f); } break;
    case GLFW_KEY_DOWN: if (press && g_speed > 0) { g_speed--; char b[16]; std::snprintf(b, sizeof b, "%gX", SPEEDS[g_speed]); flash(b, 0.5f); } break;
    case GLFW_KEY_PERIOD: if (g_paused) requestSeek(cur + 1.f / 30.f); break;
    case GLFW_KEY_COMMA: if (g_paused) requestSeek(cur - 1.f / 30.f); break;
    case GLFW_KEY_HOME: case GLFW_KEY_R: if (press) requestSeek(0.f); break;
    case GLFW_KEY_END: if (press) requestSeek(T_CREDITS); break;
    case GLFW_KEY_T: if (press) g_showTimer = !g_showTimer; break;
    case GLFW_KEY_H: if (press) g_showHelp = !g_showHelp; break;
    case GLFW_KEY_C: if (press) { g_captions = !g_captions; flash(g_captions ? "CAPTIONS ON" : "CAPTIONS OFF", 0.5f); } break;
    case GLFW_KEY_F:
        if (press) {
            App* a = g_app; a->fullscreen = !a->fullscreen;
            if (a->fullscreen) {
                glfwGetWindowPos(w, &a->sx, &a->sy); glfwGetWindowSize(w, &a->sw, &a->sh);
                GLFWmonitor* m = glfwGetPrimaryMonitor(); const GLFWvidmode* vm = glfwGetVideoMode(m);
                glfwSetWindowMonitor(w, m, 0, 0, vm->width, vm->height, vm->refreshRate);
            }
            else glfwSetWindowMonitor(w, nullptr, a->sx, a->sy, a->sw, a->sh, 0);
        }
        break;
    default:
        if (press && key >= GLFW_KEY_1 && key <= GLFW_KEY_9) requestSeek(CHAPTER[key - GLFW_KEY_1]);
    }
}
static float cursorFilmTime(GLFWwindow* w) {
    double cx, cy; int ww, wh; glfwGetCursorPos(w, &cx, &cy); glfwGetWindowSize(w, &ww, &wh);
    UI& u = *g_app->ui; float x = (float)cx * (ww > 0 ? u.W / ww : 1.f);
    return clampf((x - barX0(u)) / (barX1(u) - barX0(u))) * T_END;
}
static void onMouseButton(GLFWwindow* w, int button, int action, int) {
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;
    g_lastActivity = glfwGetTime();
    if (action == GLFW_PRESS) {
        double cx, cy; int ww, wh; glfwGetCursorPos(w, &cx, &cy); glfwGetWindowSize(w, &ww, &wh);
        UI& u = *g_app->ui; float y = u.H - (float)cy * (wh > 0 ? u.H / wh : 1.f);
        if (y < barZone(u)) { g_dragging = true; g_target = cursorFilmTime(w); }
    }
    else if (g_dragging) { g_dragging = false; requestSeek(cursorFilmTime(w)); }
}
static void onCursor(GLFWwindow*, double, double) { g_lastActivity = glfwGetTime(); }

int main(int argc, char** argv) {
    int W_ = 1280, H_ = 720, shadowRes = 2048, pipeFps = 0; float shotT = -1; std::string shotFile = "shot.ppm";
    for (int i = 1; i < argc; i++) {
        if (!std::strcmp(argv[i], "--shot") && i + 2 < argc) { shotT = (float)std::atof(argv[i + 1]); shotFile = argv[i + 2]; i += 2; }
        else if (!std::strcmp(argv[i], "--size") && i + 1 < argc) std::sscanf(argv[++i], "%dx%d", &W_, &H_);
        else if (!std::strcmp(argv[i], "--shadow") && i + 1 < argc) shadowRes = std::atoi(argv[++i]);
        else if (!std::strcmp(argv[i], "--shots") && i + 2 < argc) {        // --shots 10,20,30 prefix  (debug: several frames)
            std::string l = argv[i + 1]; size_t p = 0;
            while (p < l.size()) { size_t q = l.find(',', p); if (q == std::string::npos) q = l.size(); g_shotList.push_back((float)std::atof(l.substr(p, q - p).c_str())); p = q + 1; }
            shotFile = argv[i + 2]; i += 2; shotT = 0.f;
        }
        else if (!std::strcmp(argv[i], "--pipe")) { pipeFps = 30; if (i + 1 < argc && argv[i + 1][0] != '-') pipeFps = std::atoi(argv[++i]); }
    }
    if (!glfwInit()) { std::fprintf(stderr, "glfwInit failed\n"); return 1; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    if (shotT >= 0 || pipeFps > 0) glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* win = glfwCreateWindow(W_, H_, "Moon Landing - Mission HARAMBEE-1", nullptr, nullptr);
    if (!win) { std::fprintf(stderr, "cannot create an OpenGL 3.3 core context (update your graphics driver)\n"); return 1; }
    glfwMakeContextCurrent(win); glfwSwapInterval(1);
    if (!loadGL()) return 1;
    std::fprintf(stderr, "GL %s | %s\n", (const char*)glGetString(GL_VERSION), (const char*)glGetString(GL_RENDERER));

    setupWorld(); setupTrajectory();
    Assets assets; assets.init();
    {   // Moon albedo at the landing site, so the terrain patch matches the globe around it
        vec3 d = W.moonB.toLocal(W.siteB.y);
        float lon = std::atan2(d.z, d.x), lat = std::asin(clampf(d.y, -1, 1));
        int i = (int)((lon / (2 * PI) + 0.5f) * g_moonW), j = (int)((lat / PI + 0.5f) * g_moonH);
        float a = g_moonAlb[(size_t)std::min(g_moonH - 1, std::max(0, j)) * g_moonW + wrapi(i, g_moonW)];
        W.siteAlb = vec3(a, a * 0.97f, a * 0.93f);
    }
    Particles particles; particles.init();
    Renderer R; if (!R.init(&assets, &particles, shadowRes)) return 1;
    Film film; film.init(&assets, &particles);
    UI ui; ui.init();
    App app; app.film = &film; app.ui = &ui; app.win = win; g_app = &app;
    glfwSetKeyCallback(win, onKey); glfwSetMouseButtonCallback(win, onMouseButton); glfwSetCursorPosCallback(win, onCursor);
    int fw, fh; glfwGetFramebufferSize(win, &fw, &fh); R.resize(fw, fh);

    auto grab = [&](std::vector<unsigned char>& out) {
        std::vector<unsigned char> px((size_t)fw * fh * 3); out.resize(px.size());
        glPixelStorei(GL_PACK_ALIGNMENT, 1); glReadPixels(0, 0, fw, fh, GL_RGB, GL_UNSIGNED_BYTE, px.data());
        for (int y = 0; y < fh; y++) std::memcpy(&out[(size_t)y * fw * 3], &px[(size_t)(fh - 1 - y) * fw * 3], (size_t)fw * 3);
        };
    auto drawOverlay = [&](float t, bool player) {
        ui.W = (float)fw; ui.H = (float)fh;
        drawHUD(ui, t);
        if (player) drawPlayer(ui, t, glfwGetTime());
        ui.flush(R.pUI);
        };
    if (shotT >= 0) {                                       // single frame(s) -> PPM
        if (g_shotList.empty()) g_shotList.push_back(shotT);
        for (size_t k = 0; k < g_shotList.size(); k++) {
            float st = g_shotList[k];
            film.seek(st);
            renderFrame(R, film, film.t, 0); drawOverlay(film.t, false);
            std::vector<unsigned char> img; grab(img);
            std::string name = shotFile;
            if (g_shotList.size() > 1) { char b[32]; std::snprintf(b, sizeof b, "_%05.1f.ppm", st); name += b; }
            FILE* fo = std::fopen(name.c_str(), "wb");
            if (fo) { std::fprintf(fo, "P6\n%d %d\n255\n", fw, fh); std::fwrite(img.data(), 1, img.size(), fo); std::fclose(fo); }
            std::fprintf(stderr, "wrote %s (t=%.1fs)\n", name.c_str(), st);
        }
        return 0;
    }
    if (pipeFps > 0) {                                      // whole film as raw RGB frames on stdout
#ifdef _WIN32
        _setmode(_fileno(stdout), _O_BINARY);
#endif
        int n = (int)(T_END * pipeFps); std::vector<unsigned char> img;
        film.reset();
        for (int i = 0; i <= n; i++) {
            float tt = (float)i / pipeFps;
            while (film.t < tt - 1e-5f) film.step(std::min(1.f / 60.f, tt - film.t));
            renderFrame(R, film, tt, 0); drawOverlay(tt, false);
            grab(img); std::fwrite(img.data(), 1, img.size(), stdout);
            if (i % pipeFps == 0) std::fprintf(stderr, "\rrendered %d / %d s", i / pipeFps, (int)T_END);
        }
        std::fprintf(stderr, "\ndone\n"); return 0;
    }

    double prev = glfwGetTime();
    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents();
        double now = glfwGetTime(); float frame = (float)std::min(now - prev, 0.1); prev = now;
        if (g_dragging) { g_target = cursorFilmTime(win); film.t = g_target; film.P->clear(); }
        else if (g_seekTo >= 0.f) { film.seek(g_seekTo); g_seekTo = -1.f; g_target = film.t; }
        else if (!g_paused && film.t < T_END) {
            float target = std::min(T_END, film.t + frame * SPEEDS[g_speed]);
            while (film.t < target - 1e-5f) film.step(std::min(1.f / 60.f, target - film.t));
            g_target = film.t;
        }
        glfwGetFramebufferSize(win, &fw, &fh);
        if (fw != R.outW || fh != R.outH) R.resize(fw, fh);
        if (fw > 0 && fh > 0) { renderFrame(R, film, film.t, 0); drawOverlay(film.t, true); }
        glfwSwapBuffers(win);
    }
    glfwTerminate();
    return 0;
}