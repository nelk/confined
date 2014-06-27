
dodec_mat = gr.material({0.9, 0.6, 0.3}, {0.5, 0.5, 0.5}, 15, 0.1)
ground = gr.material({0.4, 0.4, 0.4}, {0.8, 0.8, 0.8}, 25, 0.3)
mirror = gr.material({1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}, 40, 0.5)

--whitemat = gr.material({1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}, 10)

scene = gr.node('root')

--require('mickey')
--mickey:set_material(mat1)
--m1 = gr.node('m1')
--scene:add_child(m1)
--m1:add_child(mickey)
--m1:translate(-0.5, -0.5, -5)
--m1:rotate('Y', -90)
--m1:rotate('X', -90)
--m1:scale(2, 2, 2)

--require('cylinder')
--cyl:set_material(mirror)

--cyl_number = 1
--for _, x in pairs({-2, -1, 0, 1, 2}) do
  --cyl_instance = gr.node('cyl' .. tostring(c_number))
  --scene:add_child(cyl_instance)
  --cyl_instance:add_child(cyl)
  --cyl_instance:translate(x, -1, -6)
  --cyl_instance:scale(0.3, 3.0, 0.3)
  --cyl_number = cyl_number + 1
--end

--mirror_sphere = gr.sphere('mirror_sphere')
--mirror_sphere:set_material(mirror)

--s_number = 1
--for _, x in pairs({-2.5, -1.5, -0.5, 0.5, 1.5, 2.5}) do
  --s_instance = gr.node('s' .. tostring(s_number))
  --scene:add_child(s_instance)
  --s_instance:add_child(mirror_sphere)
  --s_instance:translate(x, -1, -6)
  --s_instance:scale(0.3, 0.3, 0.3)
  --s_number = s_number + 1
--end

require('smstdodeca')
steldodec:set_material(dodec_mat)
steldodec:translate(150, -150, -100)
--steldodec:scale(0.4, 0.4, 0.4)

d1 = gr.node('dodec1')
d1:add_child(steldodec)
scene:add_child(d1)
d1:translate(-150, 150, -400)

d2 = gr.node('dodec2')
d2:add_child(steldodec)
scene:add_child(d2)
d2:translate(150, 150, -400)


s_node = gr.node('the_spheres')
s_node:translate(0, 0, -10)
s_node:rotate('X', 30)
s_node:rotate('Y', 45)
scene:add_child(s_node)

mirror_sphere = gr.sphere('mirror_sphere')
mirror_sphere:set_material(mirror)

s_number = 1
for x=-2,2,1 do
  for y=-2,2,1 do
    for z=-2,2,1 do
      s_instance = gr.node('s' .. tostring(s_number))
      s_node:add_child(s_instance)
      s_instance:add_child(mirror_sphere)
      s_instance:translate(x, y, z)
      s_instance:scale(0.4, 0.4, 0.4)
      s_number = s_number + 1
    end
  end
end

platform = gr.cube('platform')
platform:set_material(ground)
scene:add_child(platform)
platform:translate(-10, -4.5, -20)
platform:scale(20, 1, 20)

white_light = gr.light({-100.0, 150.0, 400.0}, {0.5, 0.5, 0.5}, {1, 0, 0})
red_light = gr.light({-5, -1, -5}, {1.0, 0.0, 0.0}, {1, 0, 0})
green_light = gr.light({5, -1, -5}, {0.0, 1.0, 0.0}, {1, 0, 0})
--blue_light = gr.light({0, 10, -5}, {0.0, 0.0, 1.0}, {1, 0, 0})
blue_light = gr.light({0, 2, -5}, {0.0, 0.0, 1.0}, {1, 0, 0})

gr.render(scene, 'sample.png', 256, 256, --1920, 1080,
  {0, 0, 6}, {0, 0, -1}, {0, 1, 0}, 50,
  {0.3, 0.3, 0.3}, {red_light, green_light, blue_light}
)

