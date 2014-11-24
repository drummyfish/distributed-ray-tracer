/**
 Simple distributed ray-tracer.

 The axes are: +x left, +y forward, +z up.

 @date 2014
 @author Miloslav Číž
 */

#include <iostream>
#include <math.h>
#include <vector>
#include <string>
#include <fstream>
#include "raytracer.hpp"

using namespace std;

int main(void)
  {
    t_color_buffer buffer,texture;
    scene_3D scene;
    mesh_3D mesh, mesh2;
    light_3D light, light2;

    light.set_position(-6,-4,0);
    light.set_intensity(0.5);

    light2.set_position(6,-3,3);
    light2.set_intensity(0.6);

    mesh.load_obj("cup.obj");

    mesh2.load_obj("plane.obj");
    color_buffer_load_from_png(&texture,"teacup.png");

    mesh.mat.surface_color.red = 255;
    mesh.mat.surface_color.green = 0;
    mesh.mat.surface_color.blue = 100;
  //  mesh.set_texture(&texture);
  //  mesh.print();

     mesh.scale(0.5,0.5,0.5);
     mesh.rotate(0.65,AROUND_X);
   // mesh.rotate(0.1,AROUND_Y);

    mesh.translate(0,4,0);
    mesh2.translate(0,8,2);

  //  mesh.print();

    scene.add_mesh(&mesh);
    scene.add_mesh(&mesh2);
    scene.add_light(&light2);
    scene.add_light(&light);

    scene.render(&buffer);

    color_buffer_save_to_png(&buffer,"picture.png");

    return 0;
  }
