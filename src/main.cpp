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
    mesh_3D cup, floor, mirror;
    light_3D light, light2;

    color_buffer_load_from_png(&texture,"floor.png");

    light.set_position(-6,-4,3);
    light.set_intensity(0.7);
    light.distance_factor = 50;

    light2.set_position(6,3,3);
    light2.set_intensity(0.4);
    light2.distance_factor = 50;

    cup.load_obj("cup2.obj");
    cup.mat.surface_color.red = 255;
    cup.mat.surface_color.green = 0;
    cup.mat.surface_color.blue = 100;
    cup.mat.specular_intensity = 0.9;
    cup.mat.specular_exponent = 50;
    cup.scale(1.25,1.25,1.25);
    cup.rotate(-1.4,AROUND_X);
    cup.rotate(0.1,AROUND_Y);
    cup.translate(2,18,3);

    floor.load_obj("plane.obj");
    floor.scale(3,3,3);
    floor.rotate(-PI / 2.0,AROUND_X);
    floor.translate(0,0,-5);
    floor.translate(-1,19,2);
    floor.set_texture(&texture);

    mirror.load_obj("plane.obj");
    mirror.scale(3,1,1);
    mirror.translate(-1,27,5);
    mirror.mat.reflection = 0.4;

    scene.add_mesh(&cup);
    scene.add_mesh(&floor);
    scene.add_mesh(&mirror);
    scene.add_light(&light2);
    scene.add_light(&light);

    scene.camera_translate(8,4,13);
    scene.camera_rotate(-0.4,AROUND_Z);
    scene.camera_rotate(0.5,AROUND_X);
    scene.set_background_color(255,200,100);
    scene.set_resolution(640,480);

    scene.set_distribution_parameters(4,0.5);

    scene.render(&buffer,print_progress);
    color_buffer_save_to_png(&buffer,"picture.png");

    return 0;
  }
