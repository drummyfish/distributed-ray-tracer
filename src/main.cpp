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

class object_3D
  {
     protected:
       point_3D position;           /**< translation */

     public:
       object_3D();
  };

class light_3D: public object_3D    /**< light in 3D */
  {
    double intensity;
    color light_color;
  };

class mesh_3D: public object_3D     /**< 3D object made of triangles */
  {
    protected:
      t_color_buffer *texture;

    public:
      vector<vertex_3D> vertices;
      vector<unsigned int> triangle_indices;

      mesh_3D();
      void set_texture(t_color_buffer *texture);
      t_color_buffer *get_texture();
      bool load_obj(string filename);
      void translate(double x, double y, double z);
      void rotate(double around_x, double around_y, double around_z);
      void scale(double x, double y, double z);
      void print();
  };

class scene_3D         /**< 3D scene with 3D objects, lights and rendering info */
  {
    protected:
      vector<mesh_3D *> meshes;
      vector<light_3D *> lights;
      point_3D camera_position;
      double focal_distance;
      unsigned int resolution[2];   /**< final picture resolution */

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

      bool intersects_triangle(triangle_3D triangle, double &a, double &b, double &c);

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
          @return true if the triangle is intersected by the line
         */
  };

mesh_3D::mesh_3D()
  {
    this->texture = 0;
  }

void mesh_3D::translate(double x, double y, double z)
  {
    unsigned int i;

    for (i = 0; i < this->vertices.size(); i++)
      {
        this->vertices[i].position.x += x;
        this->vertices[i].position.y += y;
        this->vertices[i].position.z += z;
      }
  }

void mesh_3D::rotate(double around_x, double around_y, double around_z)
  {
    unsigned int i;
    double x,y,z;

    for (i = 0; i < this->vertices.size(); i++)
      {
        x = this->vertices[i].position.x;
        y = this->vertices[i].position.y;
        z = this->vertices[i].position.z;

        x = x * cos(around_z) - y * sin(around_z);
        y = x * sin(around_z) + y * cos(around_z);

        y = y * cos(around_x) - z * sin(around_x);
        z = y * sin(around_x) + z * cos(around_x);

        x = x * cos(around_y) - z * sin(around_y);
        z = x * sin(around_y) + z * cos(around_y);

        this->vertices[i].position.x = x;
        this->vertices[i].position.y = y;
        this->vertices[i].position.z = z;
      }
  }

void mesh_3D::scale(double x, double y, double z)
  {
    unsigned int i;

    for (i = 0; i < this->vertices.size(); i++)
      {
        this->vertices[i].position.x *= x;
        this->vertices[i].position.y *= y;
        this->vertices[i].position.z *= z;
      }
  }

void mesh_3D::set_texture(t_color_buffer *texture)
  {
    this->texture = texture;
  }

t_color_buffer *mesh_3D::get_texture()
  {
    return this->texture;
  }

scene_3D::scene_3D()
  {
    this->resolution[0] = 640;
    this->resolution[1] = 480;
    this->camera_position.x = 0;
    this->camera_position.y = 0;
    this->camera_position.z = 0;
    this->focal_distance = 0.5;
  }

void print_point(point_3D point)
  {
    cout << "(" << point.x << ", " << point.y << ", " << point.z << ")" << endl;
  }

double string_to_double(string what, size_t *end_position)
  /**<
   Implementation of strtod because it can't be used because
   of a MinGW bug.
   */

  {
    *end_position = 0;
    double result = 0.0;
    int digit;
    double helper;
    bool fraction_part = false;
    unsigned int i, fraction_position;
    bool negative = false;

    fraction_position = 1;

    while (*end_position < what.size() && what[*end_position] == ' ')    // spaces at the beginnig
      *end_position = *end_position + 1;

    if (*end_position < what.size() && what[*end_position] == '-')
      {
        negative = true;
        *end_position = *end_position + 1;
      }

    while (true)
      {
        if (*end_position >= what.size())
          return negative ? -1 * result : result;

        if (what[*end_position] == '.')
          {
            if (fraction_part)
              return negative ? -1 * result : result;

            fraction_part = true;
            *end_position = *end_position + 1;
            continue;
          }

        digit = what[*end_position] - '0';

        if (digit < 0 || digit > 9)
          break;

        if (!fraction_part)
          {
            result *= 10;
            result += digit;
          }
        else
          {
            helper = digit;

            for (i = 0; i < fraction_position; i++)
              helper *= 0.1;

            result += helper;
            fraction_position++;
          }

        *end_position = *end_position + 1;
      }

    return negative ? -1 * result : result;
  }

void parse_obj_line(string line,float data[4][3])

  /**<
    Parses the data contained in one line of obj file format
    (e.g. "v 1.5 3 4.2" or "f 1/2 3/5 4/6 1/20").
    @param data in this variable the parsed data will be returned, the
           first index represents the element number (i.e. x, y, z for
           a vertex or one of the triangle indices) and the second index
           represents one of up to 3 shashed values (if the values are
           in format a/b/3), if any of the values is not present, -1.0
           is inserted.
   */

  {
    line = line.substr(line.find_first_of(' '));  // get rid of the first characters

    unsigned int i,j;
    size_t position;
    bool do_break;

    for (i = 0; i < 4; i++)
      for (j = 0; j < 3; j++)
        data[i][j] = -1.0;

    for (i = 0; i < 4; i++)
      {
        for (j = 0; j < 3; j++)
          {
            do_break = false;

            try
              {
                if (line.length() >= 1)
                  data[i][j] = string_to_double(line,&position);

                if (line[position] != '/')
                  do_break = true;

                if (position + 1 <= line.length())
                  line = line.substr(position + 1);
                else
                  return;

                if (do_break)
                  break;
              }
            catch (exception& e)
              {
              }
          }
      }
}

bool mesh_3D::load_obj(string filename)
  {
    ifstream obj_file(filename.c_str());
    string line;
    float obj_line_data[4][3];
    point_3D helper_point;

    vector<point_3D> normals;
    vector<point_3D> texture_vertices;

    if (!obj_file.is_open())
      return false;

    this->vertices.clear();
    this->triangle_indices.clear();

    while (getline(obj_file,line))
      {
        switch (line[0])
          {
            case 'v':
              if (line[1] == 'n')        // normal vertex
                {
                  parse_obj_line(line,obj_line_data);

                  helper_point.x = obj_line_data[0][0];
                  helper_point.y = obj_line_data[1][0];
                  helper_point.z = obj_line_data[2][0];

                  normals.push_back(helper_point);
                  break;
                }
              else if (line[1] == 't')   // texture vertex
                {
                  parse_obj_line(line,obj_line_data);

                  helper_point.x = obj_line_data[0][0];
                  helper_point.y = obj_line_data[1][0];
                  helper_point.z = 0;

                  texture_vertices.push_back(helper_point);
                  break;
                }
              else                       // position vertex
                {
                  parse_obj_line(line,obj_line_data);

                  vertex_3D vertex;

                  vertex.position.x = obj_line_data[0][0];
                  vertex.position.y = obj_line_data[1][0];
                  vertex.position.z = obj_line_data[2][0];

                  this->vertices.push_back(vertex);

                  break;
                }

            case 'f':
              unsigned int indices[4],i,faces;

              parse_obj_line(line,obj_line_data);

              for (i = 0; i < 4; i++)     // triangle indices
                indices[i] = floor(obj_line_data[i][0]) - 1;

              if (obj_line_data[3][0] < 0.0)
                {
                  this->triangle_indices.push_back(indices[0]);
                  this->triangle_indices.push_back(indices[1]);
                  this->triangle_indices.push_back(indices[2]);

                  faces = 3;     // 3 vertex face
                }
              else
                {
                  this->triangle_indices.push_back(indices[0]);
                  this->triangle_indices.push_back(indices[1]);
                  this->triangle_indices.push_back(indices[2]);

                  this->triangle_indices.push_back(indices[0]);
                  this->triangle_indices.push_back(indices[2]);
                  this->triangle_indices.push_back(indices[3]);

                  faces = 4;     // 4 vertex face
                }

              int vt_index, vn_index;

              for (i = 0; i < faces; i++)    // texture coordinates and normals
                {
                  vt_index = floor(obj_line_data[i][1]) - 1;
                  vn_index = floor(obj_line_data[i][2]) - 1;

                  if (indices[i] >= this->vertices.size() || vt_index >= texture_vertices.size())
                    continue;

                  this->vertices[indices[i]].texture_coords[0] = texture_vertices[vt_index].x;
                  this->vertices[indices[i]].texture_coords[1] = texture_vertices[vt_index].y;

                  if (vn_index >= (int) normals.size() || vn_index < 0)
                    continue;

                  this->vertices[indices[i]].normal.x = normals[vn_index].x;
                  this->vertices[indices[i]].normal.y = normals[vn_index].y;
                  this->vertices[indices[i]].normal.z = normals[vn_index].z;
                }

              break;

            default:
              break;
          }
      }

    obj_file.close();
    return true;
  }

void mesh_3D::print()
  {
    unsigned int i, counter;

    cout << "vertices: " << endl;

    for (i = 0; i < this->vertices.size(); i++)
      cout << "  " << i << ": (" << this->vertices[i].position.x << ", "
        << this->vertices[i].position.y << ", " << this->vertices[i].position.z << ") ("
        << this->vertices[i].texture_coords[0] << ", " << this->vertices[i].texture_coords[1]
        << ") (" << this->vertices[i].normal.x << ", " << this->vertices[i].normal.y << ", "
        << this->vertices[i].normal.z << ")" << endl;

    counter = 0;

    cout << "triangles: " << endl;

    for (i = 0; i < this->triangle_indices.size(); i++)
      {
        counter++;

        cout << "  " << this->triangle_indices[i] << " ";

        if (counter >= 3)
          {
            cout << endl;
            counter = 0;
          }
      }
  }

void scene_3D::render(t_color_buffer *buffer)
  {
    color_buffer_init(buffer,this->resolution[0],this->resolution[1]);

    unsigned int i,j,k,l;
    point_3D point1, point2;
    triangle_3D triangle;
    double barycentric_a, barycentric_b, barycentric_c;
    double *texture_coords_a, *texture_coords_b, *texture_coords_c;
    double aspect_ratio;

    aspect_ratio = this->resolution[1] / ((double) this->resolution[0]);

    for (j = 0; j < this->resolution[1]; j++)
      for (i = 0; i < this->resolution[0]; i++)
        {
          point1.x = 0;
          point1.y = -this->focal_distance;
          point1.z = 0;

          point2.x = i / ((double) this->resolution[0]) - 0.5;
          point2.y = 0;
          point2.z = -1 * aspect_ratio * (j / ((double) this->resolution[1]) - 0.5);

          line_3D line(point1,point2);

          for (k = 0; k < this->meshes.size(); k++)
            {
              for (l = 0; l < this->meshes[k]->triangle_indices.size(); l += 3)
                {
                  triangle.a = this->meshes[k]->vertices[this->meshes[k]->triangle_indices[l]].position;
                  triangle.b = this->meshes[k]->vertices[this->meshes[k]->triangle_indices[l + 1]].position;
                  triangle.c = this->meshes[k]->vertices[this->meshes[k]->triangle_indices[l + 2]].position;

                  texture_coords_a = this->meshes[k]->vertices[this->meshes[k]->triangle_indices[l]].texture_coords;
                  texture_coords_b = this->meshes[k]->vertices[this->meshes[k]->triangle_indices[l + 1]].texture_coords;
                  texture_coords_c = this->meshes[k]->vertices[this->meshes[k]->triangle_indices[l + 2]].texture_coords;

                  if (line.intersects_triangle(triangle,barycentric_a,barycentric_b,barycentric_c))
                    if (this->meshes[k]->get_texture() != 0)
                      {
                        unsigned char r,g,b;
                        double u,v;

                        u = barycentric_a * texture_coords_a[0] + barycentric_b * texture_coords_b[0] + barycentric_c * texture_coords_c[0];
                        v = barycentric_a * texture_coords_a[1] + barycentric_b * texture_coords_b[1] + barycentric_c * texture_coords_c[1];

                        color_buffer_get_pixel(this->meshes[k]->get_texture(),u * this->meshes[k]->get_texture()->width,v * this->meshes[k]->get_texture()->height,&r,&g,&b);

                        //color_buffer_set_pixel(buffer,i,j,barycentric_a * 255,barycentric_b * 255,barycentric_c * 255);

                        color_buffer_set_pixel(buffer,i,j,r,g,b);
                      }
                    else
                      color_buffer_set_pixel(buffer,i,j,255,0,0);
                }
            }
        }
  }

void scene_3D::add_mesh(mesh_3D *mesh)

  {
    this->meshes.push_back(mesh);
  }

void scene_3D::add_light(light_3D *light)

  {
    this->lights.push_back(light);
  }

void cross_product(point_3D vector1, point_3D vector2, point_3D &final_vector)
  {
    final_vector.x = vector1.y * vector2.z - vector1.z * vector2.y;
    final_vector.y = vector1.z * vector2.x - vector1.x * vector2.z;
    final_vector.z = vector1.x * vector2.y - vector1.y * vector2.x;
  }

void substract_vectors(point_3D vector1, point_3D vector2, point_3D &final_vector)
  {
    final_vector.x = vector2.x - vector1.x;
    final_vector.y = vector2.y - vector1.y;
    final_vector.z = vector2.z - vector1.z;
  }

double point_distance(point_3D a, point_3D b)
  {
    point_3D difference;

    substract_vectors(a,b,difference);

    return sqrt(difference.x * difference.x + difference.y * difference.y + difference.z * difference.z);
  }

double vector_length(point_3D vector)
  {
    return sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
  }

double dot_product(point_3D vector1, point_3D vector2)
  {
    return vector1.x * vector2.x + vector1.y * vector2.y + vector1.z * vector2.z;
  }

void normalize(point_3D &vector)
  {
    double length = vector_length(vector);

    vector.x /= length;
    vector.y /= length;
    vector.z /= length;
  }

object_3D::object_3D()
  {
    this->position.x = 0;
    this->position.y = 0;
    this->position.z = 0;
  }

double vectors_angle(point_3D vector1, point_3D vector2)
  {
    normalize(vector1);
    normalize(vector2);
    return acos(dot_product(vector1,vector2));
  }

line_3D::line_3D(point_3D point1, point_3D point2)
  {
    this->c0 = point1.x;
    this->q0 = point2.x - point1.x;
    this->c1 = point1.y;
    this->q1 = point2.y - point1.y;
    this->c2 = point1.z;
    this->q2 = point2.z - point1.z;
  }

void revert_vector(point_3D &vector)
  {
    vector.x *= -1;
    vector.y *= -1;
    vector.z *= -1;
  }

void line_3D::get_point(double t, point_3D &point)
  {
    point.x = this->c0 + this->q0 * t;
    point.y = this->c1 + this->q1 * t;
    point.z = this->c2 + this->q2 * t;
  }

double triangle_area(triangle_3D triangle)
  {
    double a_length, b_length, gamma;
    point_3D a_vector, b_vector;

    a_length = point_distance(triangle.a,triangle.b);
    b_length = point_distance(triangle.a,triangle.c);

    substract_vectors(triangle.b,triangle.a,a_vector);
    substract_vectors(triangle.c,triangle.a,b_vector);

    gamma = vectors_angle(a_vector,b_vector);

    return 1/2.0 * a_length * b_length * sin(gamma);
  }

bool line_3D::intersects_triangle(triangle_3D triangle, double &a, double &b, double &c)
  {
    point_3D vector1,vector2,vector3,normal;
    point_3D center;
    double bounding_sphere_radius;
    double distance_ca, distance_cb, distance_cc;

    // compute the triangle bounding sphere:

    center.x = (triangle.a.x + triangle.b.x + triangle.c.x) / 3.0;
    center.y = (triangle.a.y + triangle.b.y + triangle.c.y) / 3.0;
    center.z = (triangle.a.z + triangle.b.z + triangle.c.z) / 3.0;

    distance_ca = point_distance(center,triangle.a);
    distance_cb = point_distance(center,triangle.b);
    distance_cc = point_distance(center,triangle.c);

    bounding_sphere_radius = distance_ca;

    if (distance_cb > bounding_sphere_radius)
      bounding_sphere_radius = distance_cb;

    if (distance_cc > bounding_sphere_radius)
      bounding_sphere_radius = distance_cc;

    a = 0.0;
    b = 0.0;
    c = 0.0;

    substract_vectors(triangle.a,triangle.b,vector1);
    substract_vectors(triangle.a,triangle.c,vector2);

    cross_product(vector1,vector2,normal);

    /*
     Compute general plane equation in form
     qa * x + qb * y + qc * z + d = 0:
     */

    double qa = normal.x;
    double qb = normal.y;
    double qc = normal.z;
    double d = -1 * (qa * triangle.a.x + qb * triangle.a.y + qc * triangle.a.z);

    /* Solve for t: */

    double denominator = (qa * this->q0 + qb * this->q1 + qc * this->q2);

    if (denominator == 0)
      return false;

    double t = (-qa * this->c0 - qb * this->c1 - qc * this->c2 - d) / denominator;

    /* t now contains parameter value for the intersection */

    if (t < 0.0)
      return false;

    point_3D intersection;

    this->get_point(t,intersection);  // intersection in 3D space

    if (point_distance(intersection,center) > bounding_sphere_radius)
      return false;

    // vectors from the intersection to each triangle vertex:

    substract_vectors(triangle.a,intersection,vector1);
    substract_vectors(triangle.b,intersection,vector2);
    substract_vectors(triangle.c,intersection,vector3);

    point_3D normal1, normal2, normal3;

    // now multiply the vectors to get their normals:

    cross_product(vector1,vector2,normal1);
    cross_product(vector2,vector3,normal2);
    cross_product(vector3,vector1,normal3);

    // if one of the vectors points in other direction than the others, the point is not inside the triangle:

    if (dot_product(normal1,normal2) <= 0 || dot_product(normal2,normal3) <= 0)
      return false;

    // now compute the barycentric coordinates:

    triangle_3D helper_triangle;
    double total_area;

    total_area = triangle_area(triangle);

    helper_triangle.a = intersection;
    helper_triangle.b = triangle.b;
    helper_triangle.c = triangle.c;
    a = triangle_area(helper_triangle) / total_area;

    helper_triangle.a = triangle.a;
    helper_triangle.b = intersection;
    helper_triangle.c = triangle.c;
    b = triangle_area(helper_triangle) / total_area;

    helper_triangle.a = triangle.a;
    helper_triangle.b = triangle.b;
    helper_triangle.c = intersection;
    c = triangle_area(helper_triangle) / total_area;

    return true;
  }

int main(void)
  {
    t_color_buffer buffer,texture;
    scene_3D scene;
    mesh_3D mesh;

    mesh.load_obj("cube.obj");
    color_buffer_load_from_png(&texture,"texture.png");

    mesh.set_texture(&texture);
    mesh.print();
/*
    vertex_3D v;

    color_buffer_load_from_png(&texture,"texture.png");

    mesh.set_texture(&texture);

    v.position.x = -0.5;
    v.position.y = 1.5;
    v.position.z = 0;
    v.texture_coords[0] = 0.0;
    v.texture_coords[1] = 0.0;
    mesh.vertices.push_back(v);

    v.position.x = 0.5;
    v.position.y = 1.5;
    v.position.z = 0;
    v.texture_coords[0] = 1.0;
    v.texture_coords[1] = 1.0;
    mesh.vertices.push_back(v);

    v.position.x = 0;
    v.position.y = 1.5;
    v.position.z = 0.5;
    v.texture_coords[0] = 0.0;
    v.texture_coords[1] = 1.0;
    mesh.vertices.push_back(v);

    v.position.x = 0;
    v.position.y = 1.5;
    v.position.z = -0.5;
    v.texture_coords[0] = 1.0;
    v.texture_coords[1] = 0.0;
    mesh.vertices.push_back(v);

    mesh.triangle_indices.push_back(0);
    mesh.triangle_indices.push_back(1);
    mesh.triangle_indices.push_back(2);
    mesh.triangle_indices.push_back(0);
    mesh.triangle_indices.push_back(1);
    mesh.triangle_indices.push_back(3);
*/

    mesh.rotate(PI / 5.0,0,0);
    mesh.translate(0,4,0);
    scene.add_mesh(&mesh);

    scene.render(&buffer);

    color_buffer_save_to_png(&buffer,"picture.png");

    return 0;
  }
