//
// Created by admin on 2022/2/16.
//

#ifndef MIDILIB_GPU_HPP
#define MIDILIB_GPU_HPP
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl32.h>
#include <stdio.h>
#include <stdlib.h>
//注意：GPU计算只能在主线程中使用，否则可能导致未知错误
namespace sinrivUtils {
    namespace gpu {

#define CHECK_GPU                                                          \
    {                                                                      \
        GLenum err = glGetError();                                         \
        if (err != GL_NO_ERROR) {                                          \
        ::__android_log_print(ANDROID_LOG_ERROR,"GPU",__FILE__ ":%d glGetError returns %d\n", __LINE__, err);\
        }                                                                  \
    }

        class GPUContext {
        private:
            EGLContext context;
            EGLDisplay dpy;

        public:
            inline GPUContext() {
                ::__android_log_print(ANDROID_LOG_INFO, "GPU", "create opengl device");
                dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
                if (dpy == EGL_NO_DISPLAY) {
                    ::__android_log_print(ANDROID_LOG_ERROR, "GPU",
                                          "eglGetDisplay returned EGL_NO_DISPLAY.");
                    return;
                }

                EGLint majorVersion;
                EGLint minorVersion;
                ::__android_log_print(ANDROID_LOG_INFO, "GPU", "eglInitialize");
                EGLBoolean returnValue = eglInitialize(dpy, &majorVersion, &minorVersion);
                if (returnValue != EGL_TRUE) {
                    ::__android_log_print(ANDROID_LOG_ERROR, "GPU", "eglInitialize failed");
                    return;
                }

                EGLConfig cfg;
                EGLint count;
                EGLint s_configAttribs[] = {
                        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
                        EGL_NONE};
                ::__android_log_print(ANDROID_LOG_INFO, "GPU", "eglChooseConfig");
                if (eglChooseConfig(dpy, s_configAttribs, &cfg, 1, &count) == EGL_FALSE) {
                    ::__android_log_print(ANDROID_LOG_ERROR, "GPU", "eglChooseConfig failed");
                    return;
                }

                EGLint context_attribs[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
                ::__android_log_print(ANDROID_LOG_INFO, "GPU", "eglCreateContext");
                context = eglCreateContext(dpy, cfg, EGL_NO_CONTEXT, context_attribs);
                if (context == EGL_NO_CONTEXT) {
                    ::__android_log_print(ANDROID_LOG_ERROR, "GPU", "eglCreateContext failed");
                    return;
                }

                ::__android_log_print(ANDROID_LOG_INFO, "GPU", "eglMakeCurrent");
                returnValue = eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, context);
                if (returnValue != EGL_TRUE) {
                    ::__android_log_print(ANDROID_LOG_ERROR, "GPU",
                                          "eglMakeCurrent failed returned %d\n", returnValue);
                    return;
                }
                ::__android_log_print(ANDROID_LOG_INFO, "GPU", "create opengl device success");
            }

            inline static GLuint loadShader(GLenum shaderType, const char *pSource) {
                GLuint shader = glCreateShader(shaderType);
                if (shader) {
                    glShaderSource(shader, 1, &pSource, NULL);
                    glCompileShader(shader);
                    GLint compiled = 0;
                    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
                    if (!compiled) {
                        GLint infoLen = 0;
                        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
                        if (infoLen) {
                            char *buf = (char *) malloc(infoLen);
                            if (buf) {
                                glGetShaderInfoLog(shader, infoLen, NULL, buf);
                                ::__android_log_print(ANDROID_LOG_ERROR, "GPU",
                                                      "Could not compile shader %d:\n%s\n",
                                                      shaderType, buf);
                                free(buf);
                            }
                            glDeleteShader(shader);
                            shader = 0;
                        }
                    }
                }
                return shader;
            }

            inline static GLuint createComputeProgram(const char *pComputeSource) {
                GLuint computeShader = loadShader(GL_COMPUTE_SHADER, pComputeSource);
                if (!computeShader) {
                    return 0;
                }

                GLuint program = glCreateProgram();
                if (program) {
                    glAttachShader(program, computeShader);
                    glLinkProgram(program);
                    GLint linkStatus = GL_FALSE;
                    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
                    if (linkStatus != GL_TRUE) {
                        GLint bufLength = 0;
                        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
                        if (bufLength) {
                            char *buf = (char *) malloc(bufLength);
                            if (buf) {
                                glGetProgramInfoLog(program, bufLength, NULL, buf);
                                ::__android_log_print(ANDROID_LOG_ERROR, "GPU",
                                                      "Could not link program:\n%s\n", buf);
                                free(buf);
                            }
                        }
                        glDeleteProgram(program);
                        program = 0;
                    }
                }
                return program;
            }

            inline ~GPUContext() {
                ::__android_log_print(ANDROID_LOG_INFO, "GPU", "opengl shutdown");
                eglDestroyContext(dpy, context);
                eglTerminate(dpy);
            }
        };

        namespace test {
            static const char gComputeShader[] =
                    "#version 320 es\n"
                    "layout(local_size_x = 8) in;\n"
                    "layout(binding = 0) readonly buffer Input0 {\n"
                    "    float data[];\n"
                    "} input0;\n"
                    "layout(binding = 1) readonly buffer Input1 {\n"
                    "    float data[];\n"
                    "} input1;\n"
                    "layout(binding = 2) writeonly buffer Output {\n"
                    "    float data[];\n"
                    "} output0;\n"
                    "void main()\n"
                    "{\n"
                    "    uint idx = gl_GlobalInvocationID.x;\n"
                    "    float f = input0.data[idx] + input1.data[idx];"
                    "    output0.data[idx] = f;\n"
                    "}\n";

            GLuint loadShader(GLenum shaderType, const char *pSource) {
                GLuint shader = glCreateShader(shaderType);
                if (shader) {
                    glShaderSource(shader, 1, &pSource, NULL);
                    glCompileShader(shader);
                    GLint compiled = 0;
                    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
                    if (!compiled) {
                        GLint infoLen = 0;
                        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
                        if (infoLen) {
                            char *buf = (char *) malloc(infoLen);
                            if (buf) {
                                glGetShaderInfoLog(shader, infoLen, NULL, buf);
                                ::__android_log_print(ANDROID_LOG_ERROR, "GPU",
                                                      "Could not compile shader %d:\n%s\n",
                                                      shaderType, buf);
                                free(buf);
                            }
                            glDeleteShader(shader);
                            shader = 0;
                        }
                    }
                }
                return shader;
            }

            GLuint createComputeProgram(const char *pComputeSource) {
                GLuint computeShader = loadShader(GL_COMPUTE_SHADER, pComputeSource);
                if (!computeShader) {
                    return 0;
                }

                GLuint program = glCreateProgram();
                if (program) {
                    glAttachShader(program, computeShader);
                    glLinkProgram(program);
                    GLint linkStatus = GL_FALSE;
                    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
                    if (linkStatus != GL_TRUE) {
                        GLint bufLength = 0;
                        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
                        if (bufLength) {
                            char *buf = (char *) malloc(bufLength);
                            if (buf) {
                                glGetProgramInfoLog(program, bufLength, NULL, buf);
                                ::__android_log_print(ANDROID_LOG_ERROR, "GPU",
                                                      "Could not link program:\n%s\n", buf);
                                free(buf);
                            }
                        }
                        glDeleteProgram(program);
                        program = 0;
                    }
                }
                return program;
            }

            void setupSSBufferObject(GLuint &ssbo, GLuint index, float *pIn, GLuint count) {
                glGenBuffers(1, &ssbo);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);

                glBufferData(GL_SHADER_STORAGE_BUFFER, count * sizeof(float), pIn, GL_STATIC_DRAW);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, ssbo);
            }

            void testComputeShader() {
                GLuint computeProgram;
                GLuint input0SSbo;
                GLuint input1SSbo;
                GLuint outputSSbo;

                CHECK_GPU;
                computeProgram = createComputeProgram(gComputeShader);
                CHECK_GPU;

                const GLuint arraySize = 8000;
                float f0[arraySize];
                float f1[arraySize];
                for (GLuint i = 0; i < arraySize; ++i) {
                    f0[i] = i;
                    f1[i] = i;
                }
                setupSSBufferObject(input0SSbo, 0, f0, arraySize);
                setupSSBufferObject(input1SSbo, 1, f1, arraySize);
                setupSSBufferObject(outputSSbo, 2, NULL, arraySize);
                CHECK_GPU;

                glUseProgram(computeProgram);
                glDispatchCompute(1000, 1, 1);   // arraySize/local_size_x
                glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
                CHECK_GPU;

                glBindBuffer(GL_SHADER_STORAGE_BUFFER, outputSSbo);
                float *pOut = (float *) glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0,
                                                         arraySize * sizeof(float),
                                                         GL_MAP_READ_BIT);
                for (GLuint i = 0; i < arraySize; ++i) {
                    if (fabs(pOut[i] - (f0[i] + f1[i])) > 0.0001) {
                        ::__android_log_print(ANDROID_LOG_ERROR, "GPU",
                                              "verification FAILED at array index %d, actual: %f, expected: %f\n",
                                              i, pOut[i], f0[i] + f1[i]);
                        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
                        return;
                    }
                }
                glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
                ::__android_log_print(ANDROID_LOG_INFO, "GPU", "verification PASSED\n");
                glDeleteProgram(computeProgram);
            }
        }
    }
}
#endif //MIDILIB_GPU_HPP
