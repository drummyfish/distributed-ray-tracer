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

#define WIDTH 640
#define HEIGHT 480

using namespace std;

void print_progress(int line)
  {
    cout << line << endl;
  }

void render_scene_1()
  {
    t_color_buffer buffer,cube_texture,floor_texture;
    scene_3D scene(WIDTH,HEIGHT);
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
    sphere.scale(0.9,0.9,0.9);
    sphere.translate(8.5,24,8);
    sphere.mat.reflection = 0.5;
    sphere.mat.specular_exponent = 50;
    sphere.mat.specular_intensity = 1.0;

    cup.load_obj("cup.obj");
    cup.rotate(- PI / 2.0,AROUND_X);
    cup.scale(1.5,1.5,1.5);
    cup.translate(15,14.5,5);
    cup.use_3D_texture = true;
    cup.set_texture_3D(&checkers);
    cup.mat.ambient_intensity = 0.3;
    cup.mat.diffuse_intensity = 0.8;
    cup.mat.specular_intensity = 0.9;
    cup.mat.specular_exponent = 1;

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
    scene.render(&buffer,print_progress);
    color_buffer_save_to_png(&buffer,"scene1.png");

    color_buffer_destroy(&buffer);
    color_buffer_destroy(&cube_texture);
    color_buffer_destroy(&floor_texture);
  }

void render_scene_2()
  {
    t_color_buffer buffer,floor_texture,wall_texture;
    scene_3D scene(WIDTH,HEIGHT);
    mesh_3D floor, cup, wall, mirror;
    light_3D light, light2;

    color_buffer_load_from_png(&floor_texture,"floor.png");
    color_buffer_load_from_png(&wall_texture,"wall.png");

    light.set_position(-50,-10,5);
    light.set_intensity(1.0);
    light.distance_factor = 200;

    light2.set_position(-50,20,3);
    light2.set_intensity(1.0);
    light2.distance_factor = 150;

    cup.load_obj("cup.obj");
    cup.rotate(- PI / 2.0,AROUND_X);
    cup.rotate(-0.6,AROUND_Y);
    cup.rotate(-0.3,AROUND_Z);
    cup.scale(1.5,1.5,1.5);
    cup.translate(20,14.5,5);
    cup.mat.surface_color.red = 100;
    cup.mat.surface_color.green = 100;
    cup.mat.surface_color.blue = 0;
    cup.mat.diffuse_intensity = 0.8;
    cup.mat.specular_intensity = 0.7;
    cup.mat.specular_exponent = 5;

    floor.load_obj("plane.obj");
    floor.scale(10,10,10);
    floor.rotate(-PI / 2.0,AROUND_X);
    floor.translate(0,0,-5);
    floor.translate(-1,19,2);
    floor.set_texture(&floor_texture);
    floor.mat.ambient_intensity = 0.2;

    wall.load_obj("plane.obj");
    wall.scale(10,10,10);
    wall.translate(0,20,-5);
    wall.translate(-1,19,2);
    wall.set_texture(&wall_texture);
    wall.mat.ambient_intensity = 0.2;

    mirror.load_obj("plane.obj");
    mirror.scale(2,2,1);
    mirror.mat.reflection = 0.7;
    mirror.rotate(-PI / 2.0,AROUND_Z);
    mirror.translate(-10,19,5);
    mirror.mat.ambient_intensity = 0.2;

    scene.add_mesh(&cup);
    scene.add_mesh(&floor);
    scene.add_mesh(&wall);
    scene.add_mesh(&mirror);
    scene.add_light(&light2);
    scene.add_light(&light);

    scene.camera_translate(40,25,10);
    scene.camera_rotate(-1.6,AROUND_Z);
    scene.camera_rotate(0.2,AROUND_X);
    scene.set_background_color(50,10,10);

    scene.set_focal_distance(0.6);

    scene.set_recursion_depth(4);
/*
    scene.set_distribution_parameters(
      1,     // shadow rays
      0.5,   // shadow range
      1,     // reflection rays
      0.01,  // reflection range
      5,     // DOF rays
      1,     // lens width
      30,    // focus distance
      3,     // refraction rays
      0.1    // refraction range
      );
*/
    scene.render(&buffer,print_progress);
    color_buffer_save_to_png(&buffer,"scene2.png");
    color_buffer_destroy(&buffer);
    color_buffer_destroy(&floor_texture);
    color_buffer_destroy(&wall_texture);
  }

void render_scene_3()
  {
    t_color_buffer buffer,floor_texture;
    scene_3D scene(WIDTH,HEIGHT);
    mesh_3D cube, floor, cup, sphere;
    light_3D light, light2;

    color_buffer_load_from_png(&floor_texture,"floor.png");

    light.set_position(-3,2,30);
    light.set_intensity(0.8);
    light.distance_factor = 100;

    light2.set_position(6,3,3);
    light2.set_intensity(0.6);
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
    sphere.scale(1.0,1.0,1.0);
    sphere.translate(12,24,11);
    sphere.mat.surface_color.red = 0;
    sphere.mat.surface_color.green = 230;
    sphere.mat.surface_color.blue = 255;
    sphere.mat.transparency = 0.8;
    sphere.mat.specular_exponent = 50;
    sphere.mat.specular_intensity = 1.0;

    cup.load_obj("cup.obj");
    cup.rotate(- PI / 2.0,AROUND_X);
    cup.scale(5,5,5);
    cup.translate(-1,40,15);
    cup.mat.ambient_intensity = 0.3;
    cup.mat.diffuse_intensity = 0.8;
    cup.mat.specular_intensity = 0.9;
    cup.mat.surface_color.red = 255;
    cup.mat.surface_color.green = 0;
    cup.mat.surface_color.blue = 0;
    cup.mat.specular_exponent = 100;

    cube.load_obj("cube.obj");
    cube.mat.ambient_intensity = 0.4;
    cube.mat.diffuse_intensity = 0.6;
    cube.mat.specular_intensity = 0.5;
    cube.mat.specular_exponent = 30;
    cube.mat.transparency = 0.9;
    cube.mat.surface_color.red = 255;
    cube.mat.surface_color.green = 255;
    cube.mat.surface_color.blue = 0;
    cube.rotate(PI,AROUND_X);
    cube.scale(0.2,1.0,1.2);
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
    scene.set_recursion_depth(4);



    scene.set_distribution_parameters(
      1,     // shadow rays
      0.5,   // shadow range
      1,     // reflection rays
      0.01,  // reflection range
      1,     // DOF rays
      1,     // lens width
      30,    // focus distance
      10,     // refraction rays
      0.25   // refraction range
      );


    scene.render(&buffer,print_progress);
    color_buffer_save_to_png(&buffer,"scene3.png");

    color_buffer_destroy(&buffer);
    color_buffer_destroy(&floor_texture);
  }

int main(void)
  {
    render_scene_3();
    return 0;
  }
