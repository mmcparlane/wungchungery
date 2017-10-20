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


// Platform-Specific

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

		lua_getglobal(L, "gfx_initialize");
		if (lua_isfunction(L, -1)) lua_call(L, 0, 0);

		/*
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
		*/
	}

	return 0;
}

static int gfx_canvas_size(lua_State* L) {
	int w, h;
	emscripten_get_canvas_element_size(0, &w, &h);
	lua_pushinteger(L, w);
	lua_pushinteger(L, h);
	return 2;
}

#else
static int gfx_initialize(lua_State* L) {
	exit(EXIT_FAILURE);
	return 0;
}

static int gfx_canvas_size(lua_State* L) {
	exit(EXIT_FAILURE);
	return 0;
}
#endif


// Platform-Agnostic

typedef struct GLArray GLArray;

struct GLArray {
	GLsizei len;
	GLenum type;
	union {
		GLbyte byte_[1];
		GLubyte ubyte_[1];
		GLshort short_[1];
		GLushort ushort_[1];
		GLfloat float_[1];
	};
};

static int gfx_array_new(lua_State* L) {
	int argc, i; size_t elsize; GLenum type; GLArray *a = NULL;
	argc = lua_gettop(L);
	type = (GLenum) luaL_checkinteger(L, 1);
	if (argc < 2) luaL_error(L, "Missing array data after type info.");
	for (i = 2; i <= argc; ++i) luaL_checknumber(L, i);

	switch(type) {
	case GL_BYTE:
		elsize = sizeof(GLbyte);
		break;
		
	case GL_UNSIGNED_BYTE:
		elsize = sizeof(GLubyte);
		break;
		
	case GL_SHORT:
		elsize = sizeof(GLshort);
		break;
		
	case GL_UNSIGNED_SHORT:
		elsize = sizeof(GLushort);
		break;
		
	case GL_FLOAT:
		elsize = sizeof(GLfloat);
		break;
		
	default:
		return luaL_error(L, "Invalid array type '%d' specified", type);
		break;
	}

	a = lua_newuserdata(
		L,
		sizeof(GLArray) + ((argc == 2) ?
				   0 : (elsize*(argc-2))-(sizeof(GLfloat)-elsize)));

	a->type = type;
	a->len = argc-1;
	for (i = 2; i <= argc; ++i) {
		switch(type) {
		case GL_BYTE:
			a->byte_[i-2] = (GLbyte)lua_tointeger(L, i);
			break;
			
		case GL_UNSIGNED_BYTE:
			a->ubyte_[i-2] = (GLubyte)lua_tointeger(L, i);
			break;
			
		case GL_SHORT:
			a->short_[i-2] = (GLshort)lua_tointeger(L, i);
			break;
			
		case GL_UNSIGNED_SHORT:
			a->ushort_[i-2] = (GLushort)lua_tointeger(L, i);
			break;
			
		case GL_FLOAT:
			a->float_[i-2] = (GLfloat)lua_tonumber(L, i);
			break;

		default:
			return luaL_error(L, "Invalid array type '%d' specified", type);
			break;
		}
	}
	
	return 1;
}

static int gfx_array_get(lua_State* L) {
	GLArray* a = NULL; int i;
	a = lua_touserdata(L, 1);
	i = luaL_checkinteger(L, 2) - 1;

	luaL_argcheck(L, a != NULL, 1, "'array' expected");
	luaL_argcheck(L, 0 < i && i <= (a->len), 2, "index out of range");

	switch(a->type) {
	case GL_BYTE:
		lua_pushinteger(L, a->byte_[i]);
		break;
			
	case GL_UNSIGNED_BYTE:
		lua_pushinteger(L, a->ubyte_[i]);
		break;
			
	case GL_SHORT:
		lua_pushinteger(L, a->short_[i]);
		break;
			
	case GL_UNSIGNED_SHORT:
		lua_pushinteger(L, a->ushort_[i]);
		break;
			
	case GL_FLOAT:
		lua_pushnumber(L, a->float_[i]);
		break;
	}
	
	return 1;
}

static int gfx_array_set(lua_State* L) {
	GLArray* a = NULL; int i; lua_Number n;
	a = lua_touserdata(L, 1);
	i = luaL_checkinteger(L, 2) - 1;
	n = luaL_checknumber(L, 3);

	luaL_argcheck(L, a != NULL, 1, "'array' expected");
	luaL_argcheck(L, 0 <= i && i < (a->len), 2, "index out of range");

	switch(a->type) {
	case GL_BYTE:
		a->byte_[i] = (GLbyte)n;
		break;
			
	case GL_UNSIGNED_BYTE:
		a->ubyte_[i] = (GLubyte)n;
		break;
			
	case GL_SHORT:
		a->short_[i] = (GLshort)n;
		break;
			
	case GL_UNSIGNED_SHORT:
		a->ushort_[i] = (GLushort)n;
		break;
			
	case GL_FLOAT:
		a->float_[i] = (GLfloat)n;
		break;
	}
	
	return 0;
}

static int gfx_clear(lua_State* L) {
	GLbitfield mask = luaL_checkinteger(L, 1);
	glClear(mask);
	return 0;
}

static int gfx_use_program(lua_State* L) {
	GLuint program = luaL_checkinteger(L, 1);
	glUseProgram(program);
	return 0;
}

static int gfx_viewport(lua_State* L) {
	GLint x, y; GLsizei w, h;
	x = luaL_checkinteger(L, 1);
	y = luaL_checkinteger(L, 2);
	w = luaL_checkinteger(L, 3);
	h = luaL_checkinteger(L, 4);
	glViewport(x, y, w, h);
	return 0;
}

static int gfx_create_program(lua_State* L) {
	int i; GLuint program; GLuint shader; GLint linked; GLint infolen; char *info = NULL;
	int argc = lua_gettop(L);
	
	for (i = 1; i <= argc; ++i) {
		luaL_argcheck(L,
			      glIsShader((GLuint)luaL_checkinteger(L, i)),
			      i,
			      "is not a valid shader");
	}
	
	program = glCreateProgram();
	if (0 == program) {
		lua_pushinteger(L, 0);
		return 1;
	}
	
	for (i = 1; i <= argc; ++i) {
		shader = (GLuint)lua_tointeger(L, i);		
		glAttachShader(program, shader);
	}

	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &linked);

	if (! linked) {
		infolen = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infolen);
		if (infolen > 1) {
			info = lua_newuserdata(L, sizeof(char)*infolen);
			glGetProgramInfoLog(program, infolen, NULL, info);
		        fprintf(stderr, "gfx_create_program: Error '%s' occurred during linking.", info);
		}
		glDeleteProgram(program);
		lua_pushinteger(L, 0);
		return 1;
	}

	lua_pushinteger(L, program);
	return 1;
}

static int gfx_create_shader(lua_State* L) {
	GLenum type = (GLenum)luaL_checkinteger(L, 1);
	const char* src = luaL_checkstring(L, 2);
	GLuint shader;
	GLint compiled;

	shader = glCreateShader(type);
	if (0 == shader) {
		lua_pushinteger(L, 0);
		return 1;
	}

	glShaderSource(shader, 1, &src, NULL);
	glCompileShader(shader);

	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

	if (! compiled) {
		GLint infolen = 0;

		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infolen);
		if (infolen > 1) {
			char* info = lua_newuserdata(L, sizeof(char)*infolen);

			glGetShaderInfoLog(shader, infolen, NULL, info);
			fprintf(stderr, "gfx_load_shader: Error '%s' occurred during compilation.", info);
		}
		glDeleteShader(shader);
		lua_pushinteger(L, 0);
		return 1;
	}
	
	lua_pushinteger(L, shader);
	return 1;
}

static const luaL_Reg gfx_lib[] = {
	{"initialize", gfx_initialize},
	{"canvas_size", gfx_canvas_size},
	{"create_shader", gfx_create_shader},
	{"create_program", gfx_create_program},
	{"clear", gfx_clear},
	{"use_program", gfx_use_program},
	{"viewport", gfx_viewport},
	{NULL, NULL},
};

static const luaL_Reg gfx_array_lib[] = {
	{"new", gfx_array_new},
	{"get", gfx_array_get},
	{"set", gfx_array_set},
	{NULL, NULL},
};

typedef struct GLEnumInfo GLEnumInfo;
typedef struct GLBitfieldInfo GLBitfieldInfo;

struct GLEnumInfo {
	const char* name;
	const GLenum value;
};

struct GLBitfieldInfo {
	const char* name;
	const GLbitfield value;
};

static const GLEnumInfo glenums[] = {
	{"VERTEX_SHADER", GL_VERTEX_SHADER},
	{"FRAGMENT_SHADER", GL_FRAGMENT_SHADER},
	{"BYTE", GL_BYTE},
	{"UNSIGNED_BYTE", GL_UNSIGNED_BYTE},
	{"SHORT", GL_SHORT},
	{"UNSIGNED_SHORT", GL_UNSIGNED_SHORT},
	{"FLOAT", GL_FLOAT},
};

static const GLBitfieldInfo glbitfields[] = {
	{"COLOR_BUFFER_BIT", GL_COLOR_BUFFER_BIT},
	{"DEPTH_BUFFER_BIT", GL_DEPTH_BUFFER_BIT},
	{"STENCIL_BUFFER_BIT", GL_STENCIL_BUFFER_BIT},
};

int luaopen_gfx(lua_State* L) {
	int i; int len;
	
	luaL_newlib(L, gfx_lib);

	// Populate enums
	for (i = 0, len = sizeof(glenums)/sizeof(glenums[0]); i < len; ++i) {
		lua_pushinteger(L, glenums[i].value);
		lua_setfield(L, -2, glenums[i].name);
	}

	// Populate bitfields
	for (i = 0, len = sizeof(glbitfields)/sizeof(glbitfields[0]); i < len; ++i) {
		lua_pushinteger(L, glbitfields[i].value);
		lua_setfield(L, -2, glbitfields[i].name);
	}

	// Add array
	luaL_newlib(L, gfx_array_lib);
	lua_setfield(L, -2, "array");
	
	return 1;
}
