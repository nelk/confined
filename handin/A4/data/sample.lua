
mat1 = gr.material({0.7, 1.0, 0.7}, {0.8, 0.8, 0.8}, 25, 0.5)
--mat1 = gr.material({0.7, 1.0, 0.7}, {0.8, 0.8, 0.8}, 30, 0.5)
mat2 = gr.material({1.0, 0.0, 0.0}, {0.8, 0.8, 0.8}, 25, 0.0)

--whitemat = gr.material({1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}, 10)

scene_root = gr.node('root')

--s = gr.sphere('s')
--s:translate(10, -10, -20)
--s:scale(6, 6, 6)
--s:rotate('Y', 180)
--scene_root:add_child(s)
--s:set_material(mat2)

--s1 = gr.nh_sphere('s1', {0, 0, -80}, 6, 0.5)
--scene_root:add_child(s1)
--s1:set_material(mat1)

--s2 = gr.nh_sphere('s2', {5, 5, -100}, 6)
--scene_root:add_child(s2)
--s2:set_material(mat1)

--s3 = gr.nh_sphere('s3', {3, 5, -45}, 2)
--scene_root:add_child(s3)
--s3:set_material(mat1)

--b1 = gr.nh_box('b1', {-4, 3, -60}, 5)
--scene_root:add_child(b1)
--b1:set_material(mat1)

--b1 = gr.cube('b1')
--scene_root:add_child(b1)
--b1:translate(0, 0, -60)
--b1:rotate('Y', 20)
--b1:rotate('X', 10)
--b1:scale(50, 20, 20)
--b1:set_material(mat1)

require('smstdodeca')
steldodec:set_material(mat1)
steldodec:translate(100, 10, 10)
scene_root:add_child(steldodec)

--face = gr.mesh('face', {
  --{ 0, 0, -15},
  --{ 0, 1, -15},
  --{ 1, 1, -10},
  --{ 1, 0, -10}
--}, {
  --{ 3, 2, 1, 0}
--})
--face:set_material(mat1)
--face:translate(0, 0, 8)
--scene_root:add_child(face)

--thing = gr.mesh('thing', {
  --{ -158.333350, 225.647350, -64.699450},
  --{ -207.117500, 241.498300, -64.699450},
  --{ -176.967250, 200.000000, -64.699450}
--}, {
  --{ 0, 1, 2}
--})
--thing:set_material(mat1)
--scene_root:add_child(thing)

-- Cow test ----------------------------
--cow = gr.node('the_cow')
--hide = gr.material({0.84, 0.6, 0.53}, {0.3, 0.3, 0.3}, 20)
--hide = gr.material({0.84, 0.6, 0.53}, {0.6, 0.6, 0.6}, 100)
--for _, spec in pairs({
      --{'body', {0, 0, 0}, 1.0},
      --{'head', {.9, .3, 0}, 0.6},
      --{'tail', {-.94, .34, 0}, 0.2},
      --{'lfleg', {.7, -.7, -.7}, 0.3},
      --{'lrleg', {-.7, -.7, -.7}, 0.3},
      --{'rfleg', {.7, -.7, .7}, 0.3},
      --{'rrleg', {-.7, -.7, .7}, 0.3}
         --}) do
   --part = gr.nh_sphere(unpack(spec))
   --part:set_material(hide)
   --cow:add_child(part)
--end

--cow_instance = gr.node('cow')
--scene_root:add_child(cow_instance)
--cow_instance:add_child(cow)
--cow_instance:translate(0, 0, -20)
--cow_instance:rotate('Y', 180)
--cow_instance:scale(1.4, 1.4, 1.4)
-- End Cow test ----------------------------


-- Arch test --------------------------
--stone = gr.material({0.8, 0.7, 0.7}, {0.0, 0.0, 0.0}, 0)
--arc = gr.node('arc')
--arc:translate(0, -2, -10)

--p1 = gr.nh_box('p1', {0, 0, 0}, 1)
--arc:add_child(p1)
--p1:set_material(stone)
--p1:translate(-2.4, 0, -0.4)
--p1:scale(0.8, 4, 0.8)

--p2 = gr.nh_box('p2', {0, 0, 0}, 1)
--arc:add_child(p2)
--p2:set_material(stone)
--p2:translate(1.6, 0, -0.4)
--p2:scale(0.8, 4, 0.8)

--s = gr.nh_sphere('s', {0, 0, 0}, 1)
--arc:add_child(s)
--s:set_material(stone)
--s:translate(0, 4, 0)
--s:scale(4, 0.6, 0.6)

--for i = 1, 6 do
   --an_arc = gr.node('arc' .. tostring(i))
   --an_arc:rotate('Y', (i-1) * 60)
   --scene_root:add_child(an_arc)
   --an_arc:add_child(arc)
--end
-- End arch test ---------------------

--white_light = gr.light({-100.0, 150.0, 400.0}, {0.5, 0.5, 0.5}, {1, 0, 0})
white_light = gr.light({0.0, 0.0, 300.0}, {0.8, 0.8, 0.8}, {1, 0, 0})

-- Straight-on raw dodec
gr.render(scene_root, 'sample.png', 256, 256, --512, 512,
  {-50, 200, 200}, {0, 0, -1}, {0, 1, 0}, 50,
  {0.1, 0.1, 0.1}, {white_light})

--gr.render(scene_root, 'sample.png', 256, 256, --1024, 768,
  --{0, 0, 0}, {0, 0, -1}, {0, 1, 0}, 50,
  --{0.3, 0.3, 0.3}, {white_light}
--)

