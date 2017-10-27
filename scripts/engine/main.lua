

engine.input = function()
   print("input")
end

engine.update = function()
   print("update")
end

engine.render = function(frameratio)
   print("render "..frameratio)
end

engine.start = function()
   print("onstart")
end

engine.stop = function()
   print("onstop")
end

engine.pause = function()
   print("onpause")
end

engine.initialize = function()
   gfx.initialize()
end

