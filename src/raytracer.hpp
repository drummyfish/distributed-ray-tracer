#ifndef RAYTRACER_H
#define RAYTRACER_H

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

using namespace std;

#define PI 3.1415926535897932384626

extern "C"
{
#include "colorbuffer.h"
}

typedef enum
  {
    AROUND_X,
    AROUND_Y,
    AROUND_Z
  } rotation_type;

typedef struct          /**< point, also a vector */
  {
    double x;
    double y;
    double z;
  } point_3D;

typedef struct
  {
    point_3D a;
    point_3D b;
    point_3D c;
  } triangle_3D;

typedef struct         /**< vertex used by 3D object */
  {
    point_3D position;
    double texture_coords[2];
    point_3D normal;
  } vertex_3D;

typedef struct
  {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char alpha;
  } color;

typedef struct
  {
    double ambient_intensity;
    double diffuse_intensity;
    double specular_intensity;
    double specular_exponent;
    double reflection;
    double transparency;
    double refractive_index;
    color surface_color;
  } material;

class light_3D                      /**< light in 3D */
  {
    protected:
      point_3D position;
      double intensity;
      color light_color;

    public:
      light_3D();
      point_3D get_position();
      color get_color();
      void set_intensity(double intensity);
      /**<
       Sets the light intensity.

       @param intensity new intensity in range <0,1>
       */

      double get_intensity();
      void set_color(unsigned char r, unsigned char g, unsigned char b);
      void set_position(double x, double y, double z);
  };

class mesh_3D                       /**< 3D object made of triangles */
  {
    protected:
      t_color_buffer *texture;

    public:
      material mat;
      point_3D bounding_sphere_center;
      double bounding_sphere_radius;

      vector<vertex_3D> vertices;
      vector<unsigned int> triangle_indices;

      mesh_3D();
      material get_material();
      void update_bounding_sphere();
      void set_texture(t_color_buffer *texture);
      t_color_buffer *get_texture();
      bool load_obj(string filename);
      void translate(double x, double y, double z);
      void rotate(double angle, rotation_type type);
      void scale(double x, double y, double z);
      void print();
  };

class line_3D
  {
    protected:
      /* parametric line equation:
         x(t) = c0 + q0 * t;
         y(t) = c1 + q1 * t;
         z(t) = c2 + q2 * t; */

      double c0;
      double q0;
      double c1;
      double q1;
      double c2;
      double q2;

    public:
      line_3D(point_3D point1, point_3D point2);

        /**<
          Class constructor, makes a line by two given points.

          @param point1 first point, the value t = 0 in parametric
                 equation will give this point
          @param point2 second point, the value t = 1 in parametric
                 equation will give this point
          */

      void get_point(double t, point_3D &point);

        /**<
          Gets a point of this line by given parameter value.

          @param t parameter value
          @param point in this variable the line point will be returned
         */

      bool intersects_triangle(triangle_3D triangle, double &a, double &b, double &c, double &t);

        /**<
          Checks whether the line intersects given triangle plus
          computes the barycentric coordination od the intersection in
          the triangle.

          @param a in this variable the first coordination of the
                 barycentric coordinations of the intersection will
                 be returned
          @param b in this variable the second coordination of the
                 barycentric coordinations of the intersection will
                 be returned
          @param c in this variable the third coordination of the
                 barycentric coordinations of the intersection will
                 be returned
          @param t in this variable a parameter value of the
                 intersection will be returned;
          @return true if the triangle is intersected by the line
         */

      bool intersects_sphere(point_3D center, double radius);
  };

class scene_3D         /**< 3D scene with 3D objects, lights and rendering info */
  {
    protected:
      vector<mesh_3D *> meshes;
      vector<light_3D *> lights;
      point_3D camera_position;
      double focal_distance;
      unsigned int resolution[2];   /**< final picture resolution */

      bool cast_shadow_ray(line_3D line, light_3D light, double threshold);

      /**<
       Cast a shadow ray to given light and checks if the point the
       ray was casted from is vidible (lit) by the light.

       @param line line representing the ray
       @param light light to be checked
       @param threshold distance to which the intersections don't count
              so that the triangles don't cast shadows on themselves due
              to numerical errors
       @return true if the ray hits the light without hitting any
               other object in the scene, false otherwise
       */

      color compute_lighting(point_3D position, material surface_material, point_3D surface_normal);

      /**<
       Computes the lighting for given point and material in the scene
       taken all lights in the scene into account, the shadow rays are
       also casted.

       @param position position in the scene where to compute the
              lighting
       @param surface_material material to compute the lighting for
       @param surface_normal surface normal
       @return the computed color
       */

      color cast_ray(line_3D line, unsigned int recursion_depth);

      /**<
       Casts a ray and gets the color it hits (it is recursively
       computed by casting secondary rays)

       @param line line representing the ray
       @param recursion depth depth of recursion, 0 means no secondary
              ray will be cast
       @return computed color
       */

    public:
      scene_3D();

      void render(t_color_buffer *buffer);

      /**<
       Renders the set up scene into given color buffer.

       @param buffer buffer to render the scene to, it must not be
              initialised
       */

      void add_mesh(mesh_3D *mesh);
      void add_light(light_3D *light);
  };

void substract_vectors(point_3D vector1, point_3D vector2, point_3D &final_vector);
double point_distance(point_3D a, point_3D b);
int saturate_int(int value, int min, int max);
void print_point(point_3D point);
double vector_length(point_3D vector);
void normalize(point_3D &vector);
double dot_product(point_3D vector1, point_3D vector2);
void rotate_point(point_3D &point, double angle, rotation_type type);
double string_to_double(string what, size_t *end_position);
  /**<
   Implementation of strtod because it can't be used because
   of a MinGW bug.
   */
void parse_obj_line(string line,float data[4][3]);
color multiply_colors(color color1, color color2);
void cross_product(point_3D vector1, point_3D vector2, point_3D &final_vector);
double vectors_angle(point_3D vector1, point_3D vector2);
double triangle_area(triangle_3D triangle);

#endif
