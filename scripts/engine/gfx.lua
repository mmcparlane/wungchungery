

gfx.initialize = function()

   gfx.create_context({alpha = true})

   gfx.clear_color(0.0, 0.0, 0.0, 1.0)

   vertices = gfx.array.new(gfx.FLOAT,
			    0.0, 0.5, 0.0,
			   -0.5, -0.5, 0.0,
			    0.5, -0.5, 0.0)

   vertexPosObject = gfx.gen_buffers(1)
   gfx.bind_buffer(gfx.ARRAY_BUFFER, vertexPosObject)
   gfx.buffer_data(gfx.ARRAY_BUFFER, 9*4, vertices, gfx.STATIC_DRAW)
   
   vss = "attribute vec4 vPosition; void main() { gl_Position = vPosition; }"
   vs = gfx.create_shader(gfx.VERTEX_SHADER, vss)

   fss = "precision mediump float; void main() { gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0); }"
   fs = gfx.create_shader(gfx.FRAGMENT_SHADER, fss)

   program = gfx.create_program(vs, fs)

   w, h = gfx.canvas_size()
   gfx.viewport(0, 0, w, h)

   gfx.clear(gfx.COLOR_BUFFER_BIT)

   gfx.use_program(program)
   
   gfx.bind_buffer(gfx.ARRAY_BUFFER, vertexPosObject)
   gfx.vertex_attrib_pointer(0, 3, gfx.FLOAT, 0, 0, 0)
   gfx.enable_vertex_attrib_array(0)
   
   gfx.draw_arrays(gfx.TRIANGLES, 0, 3)

end
