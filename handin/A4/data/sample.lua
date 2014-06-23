
mat1 = gr.material({0.7, 1.0, 0.7}, {0.8, 0.8, 0.8}, 30)
--whitemat = gr.material({1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}, 10)

scene_root = gr.node('root')

s = gr.sphere('s')
s:translate(0, 0, -80)
s:scale(6, 6, 6)
scene_root:add_child(s)
s:set_material(mat1)

--s1 = gr.nh_sphere('s1', {0, 0, -80}, 6)
--scene_root:add_child(s1)
--s1:set_material(mat1)

--s2 = gr.nh_sphere('s2', {5, 5, -100}, 6)
--scene_root:add_child(s2)
--s2:set_material(mat1)

--b1 = gr.nh_box('b1', {-4, 3, -60}, 3)
--scene_root:add_child(b1)
--b1:set_material(mat1)

--require('smstdodeca')
--steldodec:set_material(mat1)
--steldodec:translate(100, 10, 10)
--scene_root:add_child(steldodec)

--triangle = gr.mesh('triangle', {
  --{ 0, 0, -15},
  --{ 0, 1, -15},
  --{ 1, 1, -10},
  --{ 1, 0, -10}
--}, {
  --{ 3, 2, 1, 0}
--})
--triangle:set_material(mat1)
--scene_root:add_child(triangle)

--thing = gr.mesh('thing', {
  --{ -158.333350, 225.647350, -64.699450},
  --{ -207.117500, 241.498300, -64.699450},
  --{ -176.967250, 200.000000, -64.699450}
--}, {
  --{ 0, 1, 2}
--})
--thing:set_material(mat1)
--scene_root:add_child(thing)

--white_light = gr.light({-100.0, 150.0, 400.0}, {0.5, 0.5, 0.5}, {1, 0, 0})
white_light = gr.light({-10, 0, -2}, {0.5, 0.5, 0.5}, {1, 0.0000000001, 0.0000001})

-- Straight-on raw dodec
--gr.render(scene_root, 'sample.png', 512, 512,
  --{-140, 200, 600}, {0, 0, -1}, {0, 1, 0}, 50,
  --{0.1, 0.1, 0.1}, {white_light})

-- Side view raw dodec
--gr.render(scene_root, 'sample.png', 256, 256,
  --{200, -200, 200}, {-1.1, 1.1, -1}, {0, 1, 0}, 50,
  --{0.1, 0.1, 0.1}, {white_light})

gr.render(scene_root, 'sample.png', 1024, 768,
  {0, 0, 0}, {0, 0, -1}, {0, 1, 0}, 50,
  {0.1, 0.1, 0.1}, {white_light})

