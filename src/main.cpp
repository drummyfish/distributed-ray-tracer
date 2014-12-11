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

unsigned int width;
unsigned int height;

using namespace std;

void print_progress(int line)
  {
    if (line % 10 == 0)
      cout << ((int) (line / ((double) height - 1) * 100)) << " %" << endl;
  }

void render_scene_1(unsigned int n)
  /* shadow demonstration, n goes from 0 to 4:
     0: hard shadows
     1: soft shadows, few rays, small range
     2: soft shadows, many rays, small range
     3: soft shadows, few rays, high range
     4: soft shadows, many rays, high range
     */
  {
    t_color_buffer buffer,cube_texture,floor_texture;
    scene_3D scene(width,height);
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

    unsigned int rays;
    double range;
    string filename, info;

    switch (n)
      {
        case 0: rays = 1; range = 0; filename = "scene1_0.png"; info = "hard shadows"; break;
        case 1: rays = 3; range = 0.2; filename = "scene1_1.png"; info = "soft shadows, few lines, small range"; break;
        case 2: rays = 15; range = 0.2; filename = "scene1_2.png"; info = "soft shadows, many lines, small range"; break;
        case 3: rays = 3; range = 1.2; filename = "scene1_3.png"; info = "soft shadows, few lines, high range"; break;
        default: n = 4; rays = 15; range = 1.2; filename = "scene1_4.png"; info = "soft shadows, many lines, high range"; break;
      }

    scene.set_distribution_parameters(
      rays,  // shadow rays
      range, // shadow range
      1,     // reflection rays
      1,     // reflection range
      1,     // DOF rays
      1,     // lens width
      1,     // focus distance
      1,     // refraction rays
      0      // refraction range
      );

    scene.set_focal_distance(0.4);
    cout << "rendering scene 1, " << (n + 1) << " out of 5 (" << info << ")" << endl;
    scene.render(&buffer,print_progress);
    color_buffer_save_to_png(&buffer,(char *) filename.c_str());

    color_buffer_destroy(&buffer);
    color_buffer_destroy(&cube_texture);
    color_buffer_destroy(&floor_texture);
  }

void render_scene_2(unsigned int n)
  /* depth of field and reflection demonstration, n can be:
     0: non-distributed raytracing
     1: distributed reflection, small range
     2: distributed reflection, high range
     3: depth of field distance 1
     4: depth of field distance 2
     5: depth of field distance 3
     6: depth of field distance 4
     7: depth of field distance 2, lens wisth 1
     8: depth of field distance 2, lens wisth 2
   */
  {
    t_color_buffer buffer,floor_texture,wall_texture,pyramid_texture;
    scene_3D scene(width,height);
    mesh_3D floor, cup, wall, mirror, pyramid;
    light_3D light, light2;

    color_buffer_load_from_png(&floor_texture,"floor.png");
    color_buffer_load_from_png(&wall_texture,"wall.png");
    color_buffer_load_from_png(&pyramid_texture,"pyramid.png");

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

    pyramid.load_obj("pyramid.obj");
    pyramid.rotate(0.4,AROUND_Z);
    pyramid.translate(5,30,0);
    pyramid.set_texture(&pyramid_texture);
    pyramid.mat.diffuse_intensity = 0.9;

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
    scene.add_mesh(&pyramid);
    scene.add_light(&light2);
    scene.add_light(&light);

    scene.camera_translate(40,25,10);
    scene.camera_rotate(-1.6,AROUND_Z);
    scene.camera_rotate(0.2,AROUND_X);
    scene.set_background_color(50,10,10);

    scene.set_focal_distance(0.6);
    scene.set_recursion_depth(4);

    unsigned int reflection_rays, dof_rays;
    double distance, reflection_range;
    string filename, info;
    double lens_width;

    switch (n)
      {
        case 0: reflection_rays = 1; reflection_range = 0.7; dof_rays = 1; distance = 0; lens_width = 1.0; filename = "scene2_0.png"; info = "non-distributed raytracing"; break;
        case 1: reflection_rays = 30; reflection_range = 0.2; dof_rays = 1; distance = 0; lens_width = 1.0; filename = "scene2_1.png"; info = "distributed reflection, small range"; break;
        case 2: reflection_rays = 30; reflection_range = 0.05; dof_rays = 1; distance = 0; lens_width = 1.0; filename = "scene2_2.png"; info = "distributed reflection, high range"; break;
        case 3: reflection_rays = 1; reflection_range = 0; dof_rays = 30; distance = 20; lens_width = 1.5; filename = "scene2_3.png"; info = "depth of field distance 1"; break;
        case 4: reflection_rays = 1; reflection_range = 0; dof_rays = 30; distance = 30; lens_width = 1.5; filename = "scene2_4.png"; info = "depth of field distance 2"; break;
        case 5: reflection_rays = 1; reflection_range = 0; dof_rays = 30; distance = 40; lens_width = 1.5; filename = "scene2_5.png"; info = "depth of field distance 3"; break;
        case 6: reflection_rays = 1; reflection_range = 0; dof_rays = 30; distance = 50; lens_width = 1.5; filename = "scene2_6.png"; info = "depth of field distance 4"; break;
        case 7: reflection_rays = 1; reflection_range = 0; dof_rays = 30; distance = 30; lens_width = 2.3; filename = "scene2_7.png"; info = "depth of field distance 2, lens width 2"; break;
        default: n = 8; reflection_rays = 1; reflection_range = 0; dof_rays = 30; distance = 30; lens_width = 3.0; filename = "scene2_8.png"; info = "depth of field distance 3, lens width 3"; break;
      }

    scene.set_distribution_parameters(
      1,                 // shadow rays
      0,                 // shadow range
      reflection_rays,   // reflection rays
      reflection_range,  // reflection range
      dof_rays,          // DOF rays
      lens_width,        // lens width
      distance,          // focus distance
      1,                 // refraction rays
      0                  // refraction range
      );

    cout << "rendering scene 2, " << (n + 1) << " out of 9 (" << info << ")" << endl;

    scene.render(&buffer,print_progress);
    color_buffer_save_to_png(&buffer,(char *) filename.c_str());
    color_buffer_destroy(&buffer);
    color_buffer_destroy(&floor_texture);
    color_buffer_destroy(&wall_texture);
    color_buffer_destroy(&pyramid_texture);
  }

void render_scene_3(unsigned int n)
  /* refraction demonstration, n can be:
     0: perfect refraction
     1: distributed refraction, few rays, small range
     2: distributed refraction, many rays, small range
     3: distributed refraction, few rays, high range
     4: distributed refraction, many rays, high range
     */
  {
    t_color_buffer buffer,floor_texture;
    scene_3D scene(width,height);
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

    unsigned int rays;
    double range;
    string filename, info;

    switch (n)
      {
        case 0: rays = 1; range = 0; filename = "scene3_0.png"; info = "perfect refraction"; break;
        case 1: rays = 3; range = 0.02; filename = "scene3_1.png"; info = "distributed refraction, few rays, small range"; break;
        case 2: rays = 7; range = 0.08; filename = "scene3_2.png"; info = "distributed refraction, many rays, small range"; break;
        case 3: rays = 3; range = 0.02; filename = "scene3_3.png"; info = "distributed refraction, few rays, high range"; break;
        default: n = 4; rays = 7; range = 0.08; filename = "scene1_4.png"; info = "distributed refraction, many rays, high range"; break;
      }

    scene.set_distribution_parameters(
      1,     // shadow rays
      0,     // shadow range
      1,     // reflection rays
      1,     // reflection range
      1,     // DOF rays
      1,     // lens width
      1,     // focus distance
      rays,  // refraction rays
      range  // refraction range
      );

    scene.set_focal_distance(0.4);
    cout << "rendering scene 3, " << (n + 1) << " out of 5 (" << info << ")" << endl;

    scene.render(&buffer,print_progress);
    color_buffer_save_to_png(&buffer,(char *) filename.c_str());

    color_buffer_destroy(&buffer);
    color_buffer_destroy(&floor_texture);
  }

int main(int argc, char **argv)
  {
    int i, scene_number;
    string helper;

    if (argc > 3)
      {
        cerr << "error: bad number of arguments" << endl;
        return 1;
      }

    width = 640;        // default values
    height = 480;
    scene_number = 0;

    for (i = 1; i < argc; i++)
      {
        helper = argv[i];

        if (helper.compare("-h") == 0)
          {
            cout << "distributed raytracer demo, usage:" << endl;
            cout << "demo [[-s|-l] [X] | -h] " << endl << endl;
            cout << "-s sets small resolution (fast). " << endl;
            cout << "-s sets large resolution (slow). " << endl;
            cout << "X is the scene number (0, 1 or 2). " << endl;
            cout << "-h prints help. " << endl << endl;
            return 0;
          }
        else if (helper.compare("-s") == 0)
          {
            width = 320;
            height = 240;
          }
        else if (helper.compare("-l") == 0)
          {
            width = 1024;
            height = 768;
          }
        else
          {
            scene_number = atoi(helper.c_str());
          }
      }

    switch (scene_number)
      {
        case 0: for (i = 0; i < 5; i++) render_scene_1(i); break;
        case 1: for (i = 0; i < 9; i++) render_scene_2(i); break;
        case 2: for (i = 0; i < 5; i++) render_scene_3(i); break;
        default: break;
      }

    return 0;
  }
