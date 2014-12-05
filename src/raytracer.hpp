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
#include <stdlib.h>

#define ERROR_OFFSET 0.01

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
    double texture_coords[3];
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
    double glitter;
    color surface_color;
  } material;

class light_3D                      /**< light in 3D */
  {
    protected:
      point_3D position;
      double intensity;
      color light_color;

    public:
      double distance_factor;       /**< says how the distance affects the light intensity (distance where the intensity fades to zero) */

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

class texture_3D                    /**< 3D texture */
  {
    protected:
      double wrap_coordinate(double coord);

    public:
      virtual color get_color(double x, double y, double z) = 0;

      /**<
       Gets a sample of the texture at given coordinates.

       @param x x coordination of the sample
       @param y y coordination of the sample
       @param z z coordination of the sample
       @return color at given position in the texture, the texture space
               fits in a unit cube, i.e. x, y and z are in <0,1>,
               however for x, y and z outside this range wrapping will
               be used so any numerical value can be used
       */
  };

class texture_3D_checkers: public texture_3D
  {
    protected:
      color color1;
      color color2;
      unsigned int repeat;
      bool use_x;
      bool use_y;
      bool use_z;

    public:
      texture_3D_checkers(color color1, color color2, unsigned int repeat, bool use_x, bool use_y, bool use_z);

      /**<
       Class constructor, initialises new object.

       @param color1 color one of the checkers pattern
       @param color2 color two of the checkers pattern
       @param repeat how many times the pattern should be repeated in
              the interval <0,1>
       @param use_x whether x (u) coordinate will affect the pattern
       @param use_y whether y (v) coordinate will affect the pattern
       @param use_z whether z (w) coordinate will affect the pattern
       */

      virtual color get_color(double x, double y, double z);
  };

class mesh_3D                       /**< 3D object made of triangles */
  {
    protected:
      t_color_buffer *texture;
      texture_3D *tex_3D;

    public:
      material mat;
      bool use_3D_texture;          /**< says if 3D or 2D texture should be used */
      point_3D bounding_sphere_center;
      double bounding_sphere_radius;

      vector<vertex_3D> vertices;
      vector<unsigned int> triangle_indices;

      mesh_3D();
      material get_material();
      void update_bounding_sphere();
      void set_texture(t_color_buffer *texture);
      t_color_buffer *get_texture();
      void set_texture_3D(texture_3D *texture);
      texture_3D *get_texture_3D();
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

      point_3D get_vector_to_origin();

        /**<
          Gets a vector that's parallel with the line and points towards
          it's origin.

          @return normalized vector that's parallel with the line and
                  points towards it's origin
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
      unsigned int shadow_rays;
      unsigned int recursion_depth;
      unsigned int reflection_rays;
      unsigned int refraction_rays;
      double reflection_range;
      double refraction_range;
      double shadow_range;
      unsigned int depth_of_field_rays;
      double lens_width;
      double focus_distance;
      double focal_distance;
      color background_color;
      unsigned int resolution[2];   /**< final picture resolution */

      bool cast_shadow_ray(point_3D position, light_3D light, double threshold, double range);

      /**<
       Cast a shadow ray to given light and checks if the point the
       ray was casted from is vidible (lit) by the light.

       @param position position to cast the ray from
       @param light light to be checked
       @param threshold distance to which the intersections don't count
              so that the triangles don't cast shadows on themselves due
              to numerical errors
       @param range how much the ray should be altered (for distributed
              shadow computation)
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

      color cast_ray(line_3D line, double threshold, unsigned int recursion_depth);

      /**<
       Casts a ray and gets the color it hits (it is recursively
       computed by casting secondary rays)

       @param line line representing the ray
       @param threshold distance to which intersections don't count so
              that numerical errors don't cause for example reflection
              rays hit the surface they were cast from
       @param recursion depth depth of recursion, 0 means no secondary
              ray will be cast
       @return computed color
       */

    public:
      scene_3D(unsigned int width, unsigned int height);

      void set_recursion_depth(unsigned int depth);

      void set_distribution_parameters(unsigned int shadow_rays, double shadow_range,
        unsigned int reflection_rays, double reflection_range, unsigned int depth_of_field_rays,
        double lens_width, double focus_distance, unsigned int refraction_rays, double refraction_range);

      /**<
       Sets the parameters for distributed ray-casting.

       @param shadow_rays number of shadow rays casted from each point,
              casting multiple shadow rays in slightly different
              directions and averaging them makes smooth shadows
       @param shadow_range if multiple shadow rays are being casted
              from each point, this parameter says how much they should
              differ
       @param reflection_rays number of rays casted from the surface of
              materials with reflection
       @param reflection_range sets the range within which the
              reflection rays will be generated
       @param depth_of_field_rays number of rays casted for each pixel
              to achieve dept of field effect, 1 means no depth of
              field
       @param lens_width lens width that is used when depth of field
              is enabled
       @param focus_distance distance at which objects will be sharp
              when depth of field is enabled
       @param refraction_rays how many refraction rays will be cast for
              transparent materials
       @param refraction_range reflection_range sets the range within
              which the refraction rays will be generated
       */

      void render(t_color_buffer *buffer, void (* progress_callback)(int));

      /**<
       Renders the set up scene into given color buffer.

       @param buffer buffer to render the scene to, it must not be
              initialised
       @param progress_callback function that will be called at the
              beginning of processing of each line, the parameter is
              the line number, this parameter can be NULL
       */

      void add_mesh(mesh_3D *mesh);
      void set_resolution(unsigned int width, unsigned int height);
      void add_light(light_3D *light);
      void set_focal_distance(float distance);
      void set_background_color(unsigned char r, unsigned char g, unsigned char b);
      void camera_translate(double x, double y, double z);
      void camera_rotate(double angle, rotation_type type);
  };

void substract_vectors(point_3D vector1, point_3D vector2, point_3D &final_vector);
double point_distance(point_3D a, point_3D b);
int saturate_int(int value, int min, int max);
void print_point(point_3D point);
double vector_length(point_3D vector);
void normalize(point_3D &vector);
double dot_product(point_3D vector1, point_3D vector2);
void rotate_point(point_3D &point, double angle, rotation_type type);
void rotate_point_axis(point_3D &point, double angle, point_3D axis);
double string_to_double(string what, size_t *end_position);
  /**<
   Implementation of strtod because it can't be used because
   of a MinGW bug.
   */
void parse_obj_line(string line,float data[4][3]);
void multiply_quaternions(double q1[4], double q2[4], double dest[4]);
color multiply_colors(color color1, color color2);
void cross_product(point_3D vector1, point_3D vector2, point_3D &final_vector);
double vectors_angle(point_3D vector1, point_3D vector2);
double triangle_area(triangle_3D triangle);
point_3D make_reflection_vector(point_3D normal, point_3D incoming_vector_reverse);
point_3D make_refraction_vector(point_3D normal, point_3D incoming_vector_reverse, double refraction_index);
color add_colors(color color1, color color2);
color interpolate_colors(color color1, color color2, double ratio);
void multiply_color(color &c, double a);
void alter_vector(point_3D &what, double range);
  /**<
   Randomly alters given vector.

   @param what vector to be altered, it will be also normalized, it
          should also be normalized before this function is called
   @param range range that affects how much the vector will be altered
   */

double random_double();
  /**<
   Returns random double in range <0,1>

   @return random double in range <0,1>
   */

#endif
