//
// Copyright © Mason McParlane
//
#include <stdlib.h>
#include <stdio.h>
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include <GLES2/gl2.h>
#include <math.h>

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#include <emscripten/html5.h>

static const char* gfx_em_result(EMSCRIPTEN_RESULT r) {
	switch(r) {
	case EMSCRIPTEN_RESULT_SUCCESS: return "EMSCRIPTEN_RESULT_SUCCESS";
	case EMSCRIPTEN_RESULT_DEFERRED: return "EMSCRIPTEN_RESULT_DEFERRED";
	case EMSCRIPTEN_RESULT_NOT_SUPPORTED: return "EMSCRIPTEN_RESULT_NOT_SUPPORTED";
	case EMSCRIPTEN_RESULT_FAILED_NOT_DEFERRED: return "EMSCRIPTEN_RESULT_FAILED_NOT_DEFERRED";
	case EMSCRIPTEN_RESULT_INVALID_TARGET: return "EMSCRIPTEN_RESULT_INVALID_TARGET";
	case EMSCRIPTEN_RESULT_UNKNOWN_TARGET: return "EMSCRIPTEN_RESULT_UNKNOWN_TARGET";
	case EMSCRIPTEN_RESULT_INVALID_PARAM: return "EMSCRIPTEN_RESULT_INVALID_PARAM";
	case EMSCRIPTEN_RESULT_FAILED: return "EMSCRIPTEN_RESULT_FAILED";
	case EMSCRIPTEN_RESULT_NO_DATA: return "EMSCRIPTEN_RESULT_NO_DATA";
	case EMSCRIPTEN_RESULT_TIMED_OUT: return "EMSCRIPTEN_RESULT_TIMED_OUT";
	default: return "UNKNOWN EMSCRIPTEN RESULT";
	}
}

static int gfx_initialize(lua_State* L) {
	EmscriptenWebGLContextAttributes a = {0};
	emscripten_webgl_init_context_attributes(&a);
	
	EMSCRIPTEN_WEBGL_CONTEXT_HANDLE c = emscripten_webgl_create_context(0, &a);
	if (c < 0) {
		fprintf(stderr, "gfx_initialize: Error '%s' creating context.\n", gfx_em_result(c));
		
	} else {
		emscripten_webgl_make_context_current(c);

		// Temporary code to show something on the gl context.
		GLuint vs = glCreateShader(GL_VERTEX_SHADER);
		const char* vss = "attribute vec4 vPosition; uniform mat4 mat; void main(){ gl_Position = mat*vPosition;}";
		glShaderSource(vs, 1, &vss, 0);
		glCompileShader(vs);

		GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
		const char* fss = "precision lowp float; uniform vec3 color[3]; void main(){ gl_FragColor = vec4(1,0,0,1);}";
		glShaderSource(fs, 1, &fss, 0);
		glCompileShader(fs);

		GLuint program = glCreateProgram();
		glAttachShader(program, vs);
		glAttachShader(program, fs);
		glBindAttribLocation(program, 0, "vPosition");
		glLinkProgram(program);
		glUseProgram(program);

		float verts[] = {0.0, 0.5, 0.0, -0.5, -0.5, 0.0, 0.5, -0.5, 0.0};
		glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, 0, 0, 0);
		glEnableVertexAttribArray(0);

		int w, h;
		emscripten_get_canvas_element_size(0, &w, &h);
		float t = emscripten_get_now() / 1000.0f;
		float xs = (float)h / w;
		float ys = 1.0f;
		float mat[] = {cosf(t)*xs, sinf(t)*ys, 0, 0, -sinf(t)*xs, cosf(t)*ys, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
		glUniformMatrix4fv(glGetUniformLocation(program, "mat"), 1, 0, mat);
		glClearColor(0, 0, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}

	

	return 0;
}

#else
static int gfx_initialize(lua_State* L) {
	//Not implemented
	exit(EXIT_FAILURE);
	return 0;
}
#endif


static const luaL_Reg gfx_lib[] = {
	{"initialize", gfx_initialize},
	{NULL, NULL},
};

int luaopen_gfx(lua_State* L) {
	luaL_newlib(L, gfx_lib);
	return 1;
}
