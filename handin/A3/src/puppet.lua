
-- A humanoid puppet.

-- Create all nodes.
root = gr.node('root')

torso = gr.node('torso')
shoulders = gr.node('shoulders')
neck = gr.node('neck')
head = gr.node('head')

leftUpperArm = gr.joint('leftUpperArm', {0, -45, -180}, {0, 0, 0})
rightUpperArm = gr.node('rightUpperArm')
leftForearm = gr.node('leftForearm')
rightForearm = gr.node('rightForearm')
leftHand = gr.node('leftHand')
rightHand = gr.node('rightHand')

hips = gr.node('hips')
leftThigh = gr.node('leftThigh')
rightThigh = gr.node('rightThigh')
leftCalf = gr.node('leftCalf')
rightCalf = gr.node('rightCalf')
leftFoot = gr.node('leftFoot')
rightFoot = gr.node('rightFoot')

-- Geometry for each node
torso_g = gr.sphere('torso')
shoulders_g = gr.sphere('shoulders')
neck_g = gr.sphere('neck')
head_g = gr.sphere('head')

leftUpperArm_g = gr.sphere('leftUpperArm')
rightUpperArm_g = gr.sphere('rightUpperArm')
leftForearm_g = gr.sphere('leftForearm')
rightForearm_g = gr.sphere('rightForearm')
leftHand_g = gr.sphere('leftHand')
rightHand_g = gr.sphere('rightHand')

hips_g = gr.sphere('hips')
leftThigh_g = gr.sphere('leftThigh')
rightThigh_g = gr.sphere('rightThigh')
leftCalf_g = gr.sphere('leftCalf')
rightCalf_g = gr.sphere('rightCalf')
leftFoot_g = gr.sphere('leftFoot')
rightFoot_g = gr.sphere('rightFoot')

-- Add geometries to joints.
torso:add_child(torso_g)
shoulders:add_child(shoulders_g)
neck:add_child(neck_g)
head:add_child(head_g)

leftUpperArm:add_child(leftUpperArm_g)
rightUpperArm:add_child(rightUpperArm_g)
leftForearm:add_child(leftForearm_g)
rightForearm:add_child(rightForearm_g)
leftHand:add_child(leftHand_g)
rightHand:add_child(rightHand_g)

hips:add_child(hips_g)
leftThigh:add_child(leftThigh_g)
rightThigh:add_child(rightThigh_g)
leftCalf:add_child(leftCalf_g)
rightCalf:add_child(rightCalf_g)
leftFoot:add_child(leftFoot_g)
rightFoot:add_child(rightFoot_g)

-- Build hierarchy.
root:add_child(torso)
  torso:add_child(shoulders)
    shoulders:add_child(leftUpperArm)
      leftUpperArm:add_child(leftForearm)
        leftForearm:add_child(leftHand)
    shoulders:add_child(neck)
      neck:add_child(head)
    shoulders:add_child(rightUpperArm)
      rightUpperArm:add_child(rightForearm)
        rightForearm:add_child(rightHand)
  torso:add_child(hips)
    hips:add_child(leftThigh)
      leftThigh:add_child(leftCalf)
        leftCalf:add_child(leftFoot)
    hips:add_child(rightThigh)
      rightThigh:add_child(rightCalf)
        rightCalf:add_child(rightFoot)

-- Apply transformations.
root:translate(0.0, 1.0, -5.0)

torso_g:scale(0.5, 1.5, 0.4)
hips:translate(0.0, -1.5, 0.0)
hips_g:scale(0.7, 0.25, 0.3)
shoulders:translate(0.0, 1.3, 0.0)
shoulders_g:scale(1.2, 0.22, 0.2)
neck:translate(0.0, 0.15, 0.0)
neck_g:scale(0.1, 0.3, 0.1)
head:translate(0.0, 0.8, 0.0)
head_g:scale(0.5, 0.7, 0.4)

leftUpperArm:translate(-1.0, -0.5, 0.0)
leftUpperArm_g:scale(0.16, 0.7, 0.16)
leftForearm:translate(0.0, -1.15, 0.0)
leftForearm_g:scale(0.16, 0.6, 0.16)
leftHand:translate(0.0, -0.6, 0.0)
leftHand_g:scale(0.15, 0.18, 0.07)

rightUpperArm:translate(1.0, -0.5, 0.0)
rightUpperArm_g:scale(0.16, 0.7, 0.16)
rightForearm:translate(0.0, -1.15, 0.0)
rightForearm_g:scale(0.16, 0.6, 0.16)
rightHand:translate(0.0, -0.6, 0.0)
rightHand_g:scale(0.15, 0.18, 0.07)

leftThigh:translate(-0.5, -1.0, 0.0)
leftThigh_g:scale(0.22, 1.0, 0.22)
leftCalf:translate(0.0, -1.7, 0.0)
leftCalf_g:scale(0.18, 0.8, 0.18)
leftFoot:translate(0.0, -0.8, 0.2)
leftFoot_g:scale(0.1, 0.08, 0.4)

rightThigh:translate(0.5, -1.0, 0.0)
rightThigh_g:scale(0.22, 1.0, 0.22)
rightCalf:translate(0.0, -1.7, 0.0)
rightCalf_g:scale(0.18, 0.8, 0.18)
rightFoot:translate(0.0, -0.8, 0.2)
rightFoot_g:scale(0.1, 0.08, 0.4)


-- All materials.
red = gr.material({1.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)
blue = gr.material({0.0, 0.0, 1.0}, {0.1, 0.1, 0.1}, 10)
green = gr.material({0.0, 1.0, 0.0}, {0.1, 0.1, 0.1}, 10)
white = gr.material({1.0, 1.0, 1.0}, {0.1, 0.1, 0.1}, 10)
c1 = gr.material({1.0, 1.0, 0.0}, {0.1, 0.1, 0.1}, 10)
c2 = gr.material({0.0, 1.0, 1.0}, {0.1, 0.1, 0.1}, 10)
c3 = gr.material({1.0, 0.0, 1.0}, {0.1, 0.1, 0.1}, 10)
c4 = gr.material({0.8, 0.4, 0.2}, {0.1, 0.1, 0.1}, 10)

-- Apply materials.
torso_g:set_material(white)
head_g:set_material(red)
shoulders_g:set_material(blue)
leftUpperArm_g:set_material(c1)
leftForearm_g:set_material(c1)
leftHand_g:set_material(c1)
rightUpperArm_g:set_material(c2)
rightForearm_g:set_material(c2)
rightHand_g:set_material(c2)
leftThigh_g:set_material(c3)
leftCalf_g:set_material(c3)
leftFoot_g:set_material(c3)
rightThigh_g:set_material(c4)
rightCalf_g:set_material(c4)
rightFoot_g:set_material(c4)

return root
