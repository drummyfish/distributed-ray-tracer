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

void render_scene_1()
  {
    t_color_buffer buffer,cube_texture,floor_texture;
    scene_3D scene(1024,768);
    mesh_3D cube, floor, cup, sphere;
    light_3D light, light2;

    color_buffer_load_from_png(&cube_texture,"compcube.png");
    color_buffer_load_from_png(&floor_texture,"floor.png");

    light.set_position(-6,-4,3);
    light.set_intensity(0.7);
    light.distance_factor = 100;

    light2.set_position(6,3,3);
    light2.set_intensity(0.4);
    light2.distance_factor = 50;

    color c1, c2;
    c1.red = 255;
    c1.green = 0;
    c1.blue = 0;
    c2.red = 0;
    c2.green = 255;
    c2.blue = 0;

    texture_3D_checkers checkers(c1,c2,1,true,true,false);

    sphere.load_obj("sphere.obj");
    sphere.scale(0.35,0.35,0.35);
    sphere.translate(8.5,17,11.5);
    sphere.mat.reflection = 0.5;

    cup.load_obj("cup.obj");
    cup.rotate(- PI / 2.0,AROUND_X);
    cup.scale(1.5,1.5,1.5);
    cup.translate(15,14.5,5);
    cup.use_3D_texture = true;
    cup.set_texture_3D(&checkers);
    cup.mat.ambient_intensity = 0.3;
    cup.mat.diffuse_intensity = 0.8;
    cup.mat.specular_intensity = 0.9;
    cup.mat.specular_exponent = 100;

    cube.load_obj("compcube.obj");
    cube.mat.ambient_intensity = 0.4;
    cube.mat.diffuse_intensity = 0.6;
    cube.mat.specular_intensity = 0.5;
    cube.mat.specular_exponent = 30;
    cube.scale(0.8,0.8,0.8);
    cube.set_texture(&cube_texture);

    cube.rotate(PI,AROUND_X);
    cube.rotate(PI + PI / 2.0,AROUND_Z);
    cube.translate(3,13,5);

    floor.load_obj("plane.obj");
    floor.scale(10,10,10);
    floor.rotate(-PI / 2.0,AROUND_X);
    floor.translate(0,0,-5);
    floor.translate(-1,19,2);
    floor.set_texture(&floor_texture);
    floor.mat.ambient_intensity = 0.2;

    scene.add_mesh(&cube);
    scene.add_mesh(&sphere);
    scene.add_mesh(&cup);
    scene.add_mesh(&floor);
    scene.add_light(&light2);
    scene.add_light(&light);

    scene.camera_translate(11,3,15);
    scene.camera_rotate(-0.1,AROUND_Z);
    scene.camera_rotate(0.5,AROUND_X);
    scene.set_background_color(255,200,100);

    scene.set_focal_distance(0.4);

/*
    scene.set_recursion_depth(5);

    scene.set_distribution_parameters(
      1,    // shadow rays
      0.5,   // shadow range
      1,    // reflection rays
      0.01,     // reflection range
      1,     // DOF rays
      0,     // lens width
      0,      // focus distance
      3,     // refraction rays
      0.1    // refraction range
      );
*/
    scene.render(&buffer,print_progress);
    color_buffer_save_to_png(&buffer,"scene1.png");
  }

int main(void)
  {
    render_scene_1();
    return 0;
  }
