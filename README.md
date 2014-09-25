OpenGL Robotic Arm
==================

Part 1 Construct a mechanical arm

In this part, a 3D robotic arm model is created with three parts: a torso, a upper arm, and a lower arm. I create the sphere and cylinder meshes by myself. And a user interface is available such that user can input the joint angles between the upper arm and the tosor, and between the upper arm and the lower arm.

Part 2 Reaching for a ball

In this part, we implement the interface and algorithm such that when clicking on the image plane, the program will compute the appropriate joint angles that allow the end of the lower arm to be at the clicked point. The torso is fixed  at the origin when the clicked point is inside the arm’s reaching area. And the system is enhanced such that it  allows the torso move in the plane when the clicked point is outside the reaching area. 
Two features are implemented:
1) when clicking a point, the arm will touch the point immediately; 
2) when clicking a point and keep moving the mouse, the arms keep moving too.
