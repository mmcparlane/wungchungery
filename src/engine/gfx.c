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
	a.alpha = 0;
		
	EMSCRIPTEN_WEBGL_CONTEXT_HANDLE c = emscripten_webgl_create_context(0, &a);
	if (c < 0) {
		fprintf(stderr, "gfx_initialize: Error '%s' creating context.\n", gfx_em_result(c));
		
	} else {
		emscripten_webgl_make_context_current(c);

		lua_getglobal(L, "gfx_initialize");
		if (lua_isfunction(L, -1)) lua_call(L, 0, 0);
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

	luaL_getmetatable(L, "wch.gfx.array");
	lua_setmetatable(L, -2);
	return 1;
}

static int gfx_array_get(lua_State* L) {
	GLArray* a = NULL; int i;
	a = luaL_checkudata(L, 1, "wch.gfx.array");
	i = luaL_checkinteger(L, 2) - 1;

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
	a = luaL_checkudata(L, 1, "wch.gfx.array");
	i = luaL_checkinteger(L, 2) - 1;
	n = luaL_checknumber(L, 3);

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

static int gfx_array_len(lua_State* L) {
	GLArray* a = luaL_checkudata(L, 1, "wch.gfx.array");
	lua_pushinteger(L, a->len);
	return 1;
}

static const char* gfx_array_typetostr(GLenum t) {
	switch(t) {
	case GL_BYTE: return "BYTE";
	case GL_UNSIGNED_BYTE: return "UNSIGNED_BYTE";
	case GL_SHORT: return "SHORT";
	case GL_UNSIGNED_SHORT: return "UNSIGNED_SHORT";
	case GL_FLOAT: return "FLOAT";
	}
	return "UNKNOWN TYPE";
}

static int gfx_array_tostr(lua_State* L) {
	GLArray* a = luaL_checkudata(L, 1, "wch.gfx.array");
	lua_pushfstring(L,"gfx.array(len: %d, type: %s)", a->len, gfx_array_typetostr(a->type));
	return 1;
}

static int gfx_bind_buffer(lua_State* L) {
	GLenum target = (GLenum)luaL_checkinteger(L, 1);
	GLuint buffer = (GLuint)luaL_checkinteger(L, 2);
	glBindBuffer(target, buffer);
	return 0;
}

static int gfx_buffer_data(lua_State* L) {
        GLenum target = (GLenum)luaL_checkinteger(L, 1);
	GLsizeiptr size = (GLsizeiptr)luaL_checkinteger(L, 2);
	GLArray* data = luaL_checkudata(L, 3, "wch.gfx.array");
	GLenum usage = (GLenum)luaL_checkinteger(L, 4);
        glBufferData(target, size, &(data->byte_[0]), usage);
	return 0;
}

static int gfx_clear(lua_State* L) {
	GLbitfield mask = luaL_checkinteger(L, 1);
	glClear(mask);
	return 0;
}

static int gfx_clear_color(lua_State* L) {
	GLclampf r, g, b, a;
	r = luaL_checknumber(L, 1);
	g = luaL_checknumber(L, 2);
        b = luaL_checknumber(L, 3);
	a = luaL_checknumber(L, 4);

	glClearColor(r, g, b, a);
	return 0;
}

static int gfx_draw_arrays(lua_State* L) {
	GLenum mode = (GLenum)luaL_checkinteger(L, 1);
	GLint first = (GLint)luaL_checkinteger(L, 2);
	GLsizei count = (GLsizei)luaL_checkinteger(L, 3);
	glDrawArrays(mode, first, count);
	return 0;
}

static int gfx_enable_vertex_attrib_array(lua_State* L) {
	GLuint index = (GLuint)luaL_checkinteger(L, 1);
	glEnableVertexAttribArray(index);
	return 0;
}

static int gfx_gen_buffers(lua_State* L) {
	GLsizei n = (GLsizei)luaL_checkinteger(L, 1);
	luaL_argcheck(L, n > 0, 1, "must be greater than 0");
	GLuint r[n];
	glGenBuffers(n, r);
	for (int i = 0; i < n; ++i) lua_pushinteger(L, r[i]);
	return n;
}

static int gfx_use_program(lua_State* L) {
	GLuint program = luaL_checkinteger(L, 1);
	glUseProgram(program);
	return 0;
}

static int gfx_vertex_attrib_pointer(lua_State* L) {
	GLuint index = (GLuint)luaL_checkinteger(L, 1);
	GLint size = (GLint)luaL_checkinteger(L, 2);
	GLenum type = (GLenum)luaL_checkinteger(L, 3);	
	GLboolean normalized = (GLboolean)lua_toboolean(L, 4);
	GLsizei stride = (GLsizei)luaL_checkinteger(L, 5);
	GLvoid* ptr = NULL;
	if (lua_isuserdata(L, 6)) {
		GLArray* a = luaL_checkudata(L, 6, "wch.gfx.array");
		ptr = &(a->byte_[0]);

	} else if (lua_isinteger(L, 6)) {
		GLuint offset = (GLuint)lua_tointeger(L, 6);
		ptr = (GLvoid*)offset;
	}

	glVertexAttribPointer(index, size, type, normalized, stride, ptr);
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

	{"ARRAY_BUFFER", GL_ARRAY_BUFFER},
	{"ELEMENT_ARRAY_BUFFER", GL_ELEMENT_ARRAY_BUFFER},
	{"STREAM_DRAW", GL_STREAM_DRAW},
	{"STATIC_DRAW", GL_STATIC_DRAW},
	{"DYNAMIC_DRAW", GL_DYNAMIC_DRAW},
	
	{"BYTE", GL_BYTE},
	{"UNSIGNED_BYTE", GL_UNSIGNED_BYTE},
	{"SHORT", GL_SHORT},
	{"UNSIGNED_SHORT", GL_UNSIGNED_SHORT},
	{"FLOAT", GL_FLOAT},
	
	{"POINTS", GL_POINTS},
	{"LINE_STRIP", GL_LINE_STRIP},
	{"LINE_LOOP", GL_LINE_LOOP},
	{"LINES", GL_LINES},
	{"TRIANGLE_STRIP", GL_TRIANGLE_STRIP},
	{"TRIANGLE_FAN", GL_TRIANGLE_FAN},
	{"TRIANGLES", GL_TRIANGLES},
};

static const GLBitfieldInfo glbitfields[] = {
	{"COLOR_BUFFER_BIT", GL_COLOR_BUFFER_BIT},
	{"DEPTH_BUFFER_BIT", GL_DEPTH_BUFFER_BIT},
	{"STENCIL_BUFFER_BIT", GL_STENCIL_BUFFER_BIT},
};

static int luaopen_gfx_array(lua_State* L) {

	const luaL_Reg gfx_array_lib[] = {
		{"new", gfx_array_new},
		{NULL, NULL},
	};

	const luaL_Reg gfx_array_methods[] = {
		{"__index", gfx_array_get},
		{"__newindex", gfx_array_set},
		{"__len", gfx_array_len},
		{"__tostring", gfx_array_tostr},
		{NULL, NULL},
	};
	
	luaL_newmetatable(L, "wch.gfx.array");
	luaL_setfuncs(L, gfx_array_methods, 0);
	luaL_newlib(L, gfx_array_lib);
	
	return 1;
}

static const luaL_Reg gfx_lib[] = {
	{"bind_buffer", gfx_bind_buffer},
	{"buffer_data", gfx_buffer_data},
	{"canvas_size", gfx_canvas_size},
	{"create_shader", gfx_create_shader},
	{"create_program", gfx_create_program},
	{"clear", gfx_clear},
	{"clear_color", gfx_clear_color},
	{"draw_arrays", gfx_draw_arrays},
	{"enable_vertex_attrib_array", gfx_enable_vertex_attrib_array},
	{"gen_buffers", gfx_gen_buffers},
	{"initialize", gfx_initialize},
	{"use_program", gfx_use_program},
	{"vertex_attrib_pointer", gfx_vertex_attrib_pointer},
	{"viewport", gfx_viewport},
	{NULL, NULL},
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
	lua_pushcfunction(L, luaopen_gfx_array);
	lua_call(L, 0, 1);
	lua_setfield(L, -2, "array");
	
	return 1;
}
