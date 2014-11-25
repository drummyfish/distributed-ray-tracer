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

void print_progress(int line)
  {
    cout << line << endl;
  }

int main(void)
  {
    t_color_buffer buffer,texture;
    scene_3D scene(640,480);
    mesh_3D mesh, mesh2, mesh3;
    light_3D light, light2;

    light.set_position(-6,-4,3);
    light.set_intensity(0.2);

    light2.set_position(6,3,3);
    light2.set_intensity(0.2);

    mesh.load_obj("cup.obj");
    mesh3.load_obj("cube.obj");

    mesh2.load_obj("plane.obj");
    color_buffer_load_from_png(&texture,"water.png");


    mesh.mat.surface_color.red = 255;
    mesh.mat.surface_color.green = 0;
    mesh.mat.surface_color.blue = 100;
    mesh3.mat.surface_color.red = 0;
    mesh3.mat.surface_color.green = 255;
    mesh3.mat.surface_color.blue = 100;
   // mesh2.set_texture(&texture);
  //  mesh.print();

    mesh.scale(0.5,0.5,0.5);
  //  mesh3.scale(0.5,0.5,0.5);
  //  mesh.rotate(0.8,AROUND_X);
  //  mesh.rotate(0.1,AROUND_Y);


    mesh2.rotate(0.4,AROUND_Z);

    mesh.translate(2,4,2);
    mesh2.translate(-1,9,2);
  //  mesh3.translate(3,2,0);

  //  mesh.print();

    scene.add_mesh(&mesh);
    scene.add_mesh(&mesh2);
 //   scene.add_mesh(&mesh3);
    scene.add_light(&light2);
    scene.add_light(&light);

mesh2.mat.reflection = 0.2;

//mesh.mat.reflection = 0.1;


mesh2.print();

    scene.render(&buffer,NULL);


print_point(mesh.bounding_sphere_center);
cout << mesh.bounding_sphere_radius << endl;

    color_buffer_save_to_png(&buffer,"picture.png");

    return 0;
  }
