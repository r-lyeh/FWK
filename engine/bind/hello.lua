-- this will run on vanilla luajit.exe, provided that fwk.dll and this file are all present in same folder

local fwk=require('fwk')

-- specify location of cookbook
fwk.cook_config("../../tools/cook.ini");

-- create 75% sized + MSAAx2 anti-aliased window
fwk.window_create(75.0, fwk.WINDOW_MSAA2)

-- set window title
fwk.window_title("hello luajit")

-- config girl
local girl = fwk.model('kgirl/kgirls01.fbx', 0)
local girl_frame = 0
local girl_pivot = fwk.mat44()
fwk.rotationq44(girl_pivot, fwk.eulerq(fwk.vec3(0,0,0)))
fwk.scale44(girl_pivot, 2,2,2)

-- config & play music
local music = fwk.audio_stream('larry.mid') -- 'wrath_of_the_djinn.xm'
fwk.audio_play(music, 0);

-- config camera
local cam = fwk.camera()

-- main loop
while fwk.window_swap() == 1 do
   -- fps camera
   local grabbed = fwk.input(fwk.MOUSE_L) == 1 or fwk.input(fwk.MOUSE_R) == 1
   fwk.window_cursor( fwk.ui_active() == 1 or fwk.ui_hover() == 1 and 1 or (not grabbed) )
   if( fwk.window_has_cursor() ~= 1 ) then
      local wasdec3 = fwk.vec3(fwk.input(fwk.KEY_D)-fwk.input(fwk.KEY_A),fwk.input(fwk.KEY_E)-(fwk.input(fwk.KEY_C)),fwk.input(fwk.KEY_W)-fwk.input(fwk.KEY_S))
      local look2 = fwk.scale2(fwk.vec2(fwk.input_diff(fwk.MOUSE_X), -fwk.input_diff(fwk.MOUSE_Y)), 0.2)
      local move3 = fwk.scale3(wasdec3, cam.speed)
      fwk.camera_move(cam, wasdec3.x,wasdec3.y,wasdec3.z)
      fwk.camera_fps(cam, look2.x,look2.y)
   end

   -- draw grid/axis
   fwk.ddraw_grid(0)
   fwk.ddraw_flush()

   -- animate girl
   local delta = fwk.window_delta() * 30 -- 30fps anim
   girl_frame = fwk.model_animate(girl, girl_frame + delta)

   -- draw girl
   fwk.model_render(girl, cam.proj, cam.view, girl_pivot, 0)

   -- showcase ui
   if fwk.ui_panel("luajit", 0) == 1 then
      fwk.ui_panel_end()
   end
end
