
mirror = gr.material({0, 0, 0}, {0.8, 0.8, 0.8}, 50, 1.0)
mat1 = gr.material({1.0, 0.6, 0.1}, {0.5, 0.7, 0.5}, 25, 0.5)

scene = gr.node('root')

s = gr.sphere('s')
s:translate(4, 4, -80)
s:scale(6, 6, 6)
scene:add_child(s)
s:set_material(mat1)

wall = gr.mesh('wall', {
  { -1, -1, 0 },
  {  1, -1, 0 },
  {  1, 1, 0 },
  { -1, 1, 0 }
}, {
  {3, 2, 1, 0}
})
scene:add_child(wall)
wall:set_material(mirror)
wall:translate(-30, 0, -100)
wall:scale(30, 30, 30)
wall:rotate('Y', 15)

white_light = gr.light({-50, 100, 200}, {0.5, 0.5, 0.5}, {1, 0, 0})

gr.render(scene, 'mirror.png', 1024, 768,
  {0, 0, -30}, {0, 0, -1}, {0, 1, 0}, 50,
  {0.3, 0.3, 0.3}, {white_light}
)

