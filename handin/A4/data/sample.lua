
mat1 = gr.material({0.7, 1.0, 0.7}, {0.8, 0.8, 0.8}, 30)
--whitemat = gr.material({1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}, 10)

scene_root = gr.node('root')

s1 = gr.nh_sphere('s1', {0, 0, -80}, 6)
scene_root:add_child(s1)
s1:set_material(mat1)

s2 = gr.nh_sphere('s2', {5, 5, -100}, 6)
scene_root:add_child(s2)
s2:set_material(mat1)

b1 = gr.nh_box('b1', {-4, 3, -60}, 3)
scene_root:add_child(b1)
b1:set_material(mat1)

--white_light = gr.light({-100.0, 150.0, 400.0}, {0.5, 0.5, 0.5}, {1, 0, 0})
white_light = gr.light({-10, 0, -2}, {0.5, 0.5, 0.5}, {1, 0.0000000001, 0.0000001})

gr.render(scene_root, 'sample.png', 1024, 768,
  {-15, 10, 0}, {0.2, -0.08, -1}, {0, 1, 0}, 50,
  {0.1, 0.1, 0.1}, {white_light})
