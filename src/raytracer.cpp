#include "raytracer.hpp"

void light_3D::set_position(double x, double y, double z)
  {
    this->position.x = x;
    this->position.y = y;
    this->position.z = z;
  }

bool line_3D::intersects_sphere(point_3D center, double radius)
  {
    double a,b,c;

    a = this->q0 * this->q0 + this->q1 * this->q1 + this->q2 * this->q2;
    b = 2 * this->c0 * this->q0 - 2 * center.x * this->q0 +
        2 * this->c1 * this->q1 - 2 * center.y * this->q1 +
        2 * this->c2 * this->q2 - 2 * center.z * this->q2;
    c = this->c0 * this->c0 + this->c1 * this->c1 + this->c2 * this->c2 -
        2 * center.x * this->c0 - 2 * center.y * this->c1 - 2 * center.z * this->c2 +
        center.x * center.x + center.y * center.y + center.z * center.z -
        radius * radius;

    return b * b - 4 * a * c >= 0;
  }

mesh_3D::mesh_3D()
  {
    this->texture = 0;

    this->mat.surface_color.red = 200;
    this->mat.surface_color.green = 200;
    this->mat.surface_color.blue = 200;
    this->mat.surface_color.alpha = 255;
    this->mat.ambient_intensity = 0.3;
    this->mat.diffuse_intensity = 0.9;
    this->mat.specular_intensity = 0.6;
    this->mat.specular_exponent = 100.0;
    this->mat.reflection = 0;
    this->mat.refractive_index = 1.2;
    this->mat.transparency = 0;
    this->mat.glitter = 0;
    this->bounding_sphere_center.x = 0;
    this->bounding_sphere_center.y = 0;
    this->bounding_sphere_center.z = 0;
  }

void mesh_3D::update_bounding_sphere()
  {
    unsigned int i;
    double distance;

    this->bounding_sphere_center.x = 0;
    this->bounding_sphere_center.y = 0;
    this->bounding_sphere_center.z = 0;

    for (i = 0; i < this->vertices.size(); i++)
      {
        bounding_sphere_center.x += this->vertices[i].position.x;
        bounding_sphere_center.y += this->vertices[i].position.y;
        bounding_sphere_center.z += this->vertices[i].position.z;
      }

    bounding_sphere_center.x /= this->vertices.size();
    bounding_sphere_center.y /= this->vertices.size();
    bounding_sphere_center.z /= this->vertices.size();

    this->bounding_sphere_radius = 0;

    for (i = 0; i < this->vertices.size(); i++)
      {
        distance = point_distance(this->vertices[i].position,this->bounding_sphere_center);

        if (distance > this->bounding_sphere_radius)
          this->bounding_sphere_radius = distance;
      }
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

material mesh_3D::get_material()
  {
    return this->mat;
  }

int saturate_int(int value, int min, int max)
  {
    if (value > max)
      return max;

    if (value < min)
      return min;

    return value;
  }

void print_point(point_3D point)
  {
    cout << "(" << point.x << ", " << point.y << ", " << point.z << ")" << endl;
  }

double vector_length(point_3D vector)
  {
    return sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
  }

void normalize(point_3D &vector)
  {
    double length = vector_length(vector);

    vector.x /= length;
    vector.y /= length;
    vector.z /= length;
  }

double dot_product(point_3D vector1, point_3D vector2)
  {
    return vector1.x * vector2.x + vector1.y * vector2.y + vector1.z * vector2.z;
  }

point_3D make_reflection_vector(point_3D normal, point_3D vector_to_light)
  {
    double helper;
    point_3D result;

    normalize(normal);
    normalize(vector_to_light);

    helper = 2 * dot_product(vector_to_light,normal);
    normal.x = helper * normal.x;
    normal.y = helper * normal.y;
    normal.z = helper * normal.z;
    substract_vectors(normal,vector_to_light,result);
    normalize(result);

    return result;
  }

void scene_3D::set_distribution_parameters(unsigned int shadow_rays, double shadow_range, unsigned int depth_of_field_rays, double lens_width, double focus_distance)
  {
    this->shadow_rays = shadow_rays;
    this->shadow_range = shadow_range;
    this->depth_of_field_rays = depth_of_field_rays;
    this->lens_width = lens_width;
    this->focus_distance = focus_distance;
  }

double random_double()
  {
    return ((rand() % 1000) / 1000.0);
  }

color scene_3D::compute_lighting(point_3D position, material surface_material, point_3D surface_normal)
  {
    unsigned int i, j;
    point_3D vector_to_light, vector_to_camera, reflection_vector;
    color final_color, light_color;
    double helper, intensity, distance_penalty;
    int helper_color[3];

    helper_color[0] = surface_material.ambient_intensity * surface_material.surface_color.red;
    helper_color[1] = surface_material.ambient_intensity * surface_material.surface_color.green;
    helper_color[2] = surface_material.ambient_intensity * surface_material.surface_color.blue;

    vector_to_camera.x = -1 * position.x;
    vector_to_camera.y = -1 * position.y;
    vector_to_camera.z = -1 * position.z;

    normalize(vector_to_camera);

    for (i = 0; i < this->lights.size(); i++)
      {
        unsigned int sum;

        sum = this->cast_shadow_ray(position,*this->lights[i],ERROR_OFFSET,0.0) ? 1 : 0; // main shadow ray

        for (j = 1; j < this->shadow_rays; j++) // additional shadow rays
          sum += this->cast_shadow_ray(position,*this->lights[i],ERROR_OFFSET,this->shadow_range) ? 1 : 0;

        if (sum != 0)  // at least one shadow ray hit the light
          {
            double shadow_ratio;    // how much shadow the point is in, 0.0 = full shadow, 1.0 = no shadow
            shadow_ratio = sum / ((double) this->shadow_rays);

            distance_penalty = 1.0 - point_distance(position,this->lights[i]->get_position()) / this->lights[i]->distance_factor;
            distance_penalty = distance_penalty < 0 ? 0 : pow(distance_penalty,0.25);
            intensity = this->lights[i]->get_intensity() * distance_penalty * shadow_ratio;

            substract_vectors(this->lights[i]->get_position(),position,vector_to_light);
            normalize(vector_to_light);

            helper = -1 * dot_product(vector_to_light,surface_normal);
            helper = helper < 0 ? 0 : helper;

            // add diffuse part:
            helper_color[0] += intensity * surface_material.surface_color.red * surface_material.diffuse_intensity * helper;
            helper_color[1] += intensity * surface_material.surface_color.green * surface_material.diffuse_intensity * helper;
            helper_color[2] += intensity * surface_material.surface_color.blue * surface_material.diffuse_intensity * helper;

            // add specular part:
            reflection_vector = make_reflection_vector(surface_normal,vector_to_light);
            helper = pow(dot_product(reflection_vector,vector_to_camera),surface_material.specular_exponent);
            helper = helper < 0 ? 0 : helper;
            light_color = this->lights[i]->get_color();

            helper_color[0] += intensity * surface_material.specular_intensity * helper * light_color.red;
            helper_color[1] += intensity * surface_material.specular_intensity * helper * light_color.green;
            helper_color[2] += intensity * surface_material.specular_intensity * helper * light_color.blue;
          }
      }

    final_color.red = saturate_int(helper_color[0],0,255);
    final_color.green = saturate_int(helper_color[1],0,255);
    final_color.blue = saturate_int(helper_color[2],0,255);

    return final_color;
  }

void scene_3D::set_resolution(unsigned int width, unsigned int height)
  {
    this->resolution[0] = width;
    this->resolution[1] = height;
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

    this->update_bounding_sphere();
  }

point_3D light_3D::get_position()
  {
    return this->position;
  }

void rotate_point(point_3D &point, double angle, rotation_type type)
  {
    double x,y,z;
    double x2,y2,z2;

    while (angle < 0)
      angle += 2 * PI;

    x = point.x;
    y = point.y;
    z = point.z;

    switch (type)
          {
            case AROUND_Z:
              x2 = x * cos(angle) - y * sin(angle);
              y2 = x * sin(angle) + y * cos(angle);
              z2 = z;
              break;

            case AROUND_X:
              y2 = y * cos(angle) - z * sin(angle);
              z2 = y * sin(angle) + z * cos(angle);
              x2 = x;
              break;

            case AROUND_Y:
              x2 = x * cos(angle) - z * sin(angle);
              z2 = x * sin(angle) + z * cos(angle);
              y2 = y;
              break;

            default:
              x2 = x;
              y2 = y;
              z2 = z;
              break;
          }

    point.x = x2;
    point.y = y2;
    point.z = z2;
  }

void mesh_3D::rotate(double angle, rotation_type type)
  {
    unsigned int i;

    for (i = 0; i < this->vertices.size(); i++)
      {
        rotate_point(this->vertices[i].position,angle,type);
        rotate_point(this->vertices[i].normal,angle,type);
      }

    this->update_bounding_sphere();
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

    this->update_bounding_sphere();
  }

void mesh_3D::set_texture(t_color_buffer *texture)
  {
    this->texture = texture;
  }

t_color_buffer *mesh_3D::get_texture()
  {
    return this->texture;
  }

scene_3D::scene_3D(unsigned int width, unsigned int height)
  {
    this->resolution[0] = width;
    this->resolution[1] = height;
    this->camera_position.x = 0;
    this->camera_position.y = 0;
    this->camera_position.z = 0;
    this->focal_distance = 0.5;
    this->background_color.red = 255;
    this->background_color.green = 255;
    this->background_color.blue = 255;
    this->shadow_rays = 1;
    this->shadow_range = 0.1;
    this->depth_of_field_rays = 1;
    this->focus_distance = 10;
    this->lens_width = 2.0;
  }

void scene_3D::set_background_color(unsigned char r, unsigned char g, unsigned char b)
  {
    this->background_color.red = r;
    this->background_color.green = g;
    this->background_color.blue = b;
  }

void scene_3D::camera_translate(double x, double y, double z)
  {
    unsigned int i;

    for (i = 0; i < this->meshes.size(); i++)
      this->meshes[i]->translate(-x, -y, -z);
  }

void scene_3D::camera_rotate(double angle, rotation_type type)
  {
    unsigned int i;

    for (i = 0; i < this->meshes.size(); i++)
      this->meshes[i]->rotate(angle,type);
  }

void scene_3D::set_focal_distance(float distance)
  {
    this->focal_distance = distance;
  }

double string_to_double(string what, size_t *end_position)
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

color light_3D::get_color()
  {
    return this->light_color;
  }

void light_3D::set_color(unsigned char r, unsigned char g, unsigned char b)
  {
    this->light_color.red = r;
    this->light_color.green = g;
    this->light_color.blue = b;
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

                  if (indices[i] >= (int) this->vertices.size() || vt_index >= (int) texture_vertices.size())
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
    this->update_bounding_sphere();
    return true;
  }

void light_3D::set_intensity(double intensity)
  {
    this->intensity = intensity;
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

color multiply_colors(color color1, color color2)
  {
    color final_color;

    final_color.red = (color1.red / 255.0 * color2.red / 255.0) * 255;
    final_color.green = (color1.green / 255.0 * color2.green / 255.0) * 255;
    final_color.blue = (color1.blue / 255.0 * color2.blue / 255.0) * 255;

    return final_color;
  }

bool scene_3D::cast_shadow_ray(point_3D position, light_3D light, double threshold, double range)
  {
    unsigned int i, j;
    triangle_3D triangle;
    double a,b,c,t,distance;
    point_3D intersection;
    point_3D light_position;
    light_position = light.get_position();

    light_position.x += random_double() * range;
    light_position.y += random_double() * range;
    light_position.z += random_double() * range;

    line_3D line(position,light_position);

    for (i = 0; i < this->meshes.size(); i++)
      {
        if (!line.intersects_sphere(this->meshes[i]->bounding_sphere_center,this->meshes[i]->bounding_sphere_radius))
          continue;

        for (j = 0; j < this->meshes[i]->triangle_indices.size(); j += 3)
          {
            triangle.a = this->meshes[i]->vertices[this->meshes[i]->triangle_indices[j]].position;
            triangle.b = this->meshes[i]->vertices[this->meshes[i]->triangle_indices[j + 1]].position;
            triangle.c = this->meshes[i]->vertices[this->meshes[i]->triangle_indices[j + 2]].position;

            if (line.intersects_triangle(triangle,a,b,c,t))
              {
                line.get_point(t,intersection);
                distance = point_distance(position,intersection);

                if (distance > threshold)
                  {
                    return false;
                  }
              }
          }
       }

    return true;
  }

point_3D line_3D::get_vector_to_origin()
  {
    point_3D a,b,result;

    this->get_point(0,a);
    this->get_point(1,b);
    substract_vectors(b,a,result);

    normalize(result);
    return result;
  }

color scene_3D::cast_ray(line_3D line, double threshold, unsigned int recursion_depth)
  {
    unsigned int k, l;
    triangle_3D triangle;
    color final_color, helper_color, add_color;
    double depth, t;
    double *texture_coords_a, *texture_coords_b, *texture_coords_c;
    double barycentric_a, barycentric_b, barycentric_c;
    point_3D starting_point;
    point_3D normal,normal_a,normal_b,normal_c;
    point_3D reflection_vector, incoming_vector_reverse;
    material mat;

    line.get_point(0,starting_point);

    depth = 99999999;

    final_color.red = this->background_color.red;
    final_color.green = this->background_color.green;
    final_color.blue = this->background_color.blue;

    for (k = 0; k < this->meshes.size(); k++)
      {
        if (!line.intersects_sphere(this->meshes[k]->bounding_sphere_center,this->meshes[k]->bounding_sphere_radius))
          continue;

        for (l = 0; l < this->meshes[k]->triangle_indices.size(); l += 3)
          {
            triangle.a = this->meshes[k]->vertices[this->meshes[k]->triangle_indices[l]].position;
            triangle.b = this->meshes[k]->vertices[this->meshes[k]->triangle_indices[l + 1]].position;
            triangle.c = this->meshes[k]->vertices[this->meshes[k]->triangle_indices[l + 2]].position;

            texture_coords_a = this->meshes[k]->vertices[this->meshes[k]->triangle_indices[l]].texture_coords;
            texture_coords_b = this->meshes[k]->vertices[this->meshes[k]->triangle_indices[l + 1]].texture_coords;
            texture_coords_c = this->meshes[k]->vertices[this->meshes[k]->triangle_indices[l + 2]].texture_coords;

            if (line.intersects_triangle(triangle,barycentric_a,barycentric_b,barycentric_c,t))
              {
                point_3D intersection;
                line.get_point(t,intersection);
                double distance = point_distance(starting_point,intersection);

                mat = this->meshes[k]->get_material();

                if (distance < depth && distance > threshold)  // depth test
                  {
                    depth = distance;

                    normal_a = this->meshes[k]->vertices[this->meshes[k]->triangle_indices[l]].normal;
                    normal_b = this->meshes[k]->vertices[this->meshes[k]->triangle_indices[l + 1]].normal;
                    normal_c = this->meshes[k]->vertices[this->meshes[k]->triangle_indices[l + 2]].normal;

                    normal.x = 0;
                    normal.y = 0;
                    normal.z = 0;

                    normal.x = barycentric_a * normal_a.x + barycentric_b * normal_b.x + barycentric_c * normal_c.x;
                    normal.y = barycentric_a * normal_a.y + barycentric_b * normal_b.y + barycentric_c * normal_c.y;
                    normal.z = barycentric_a * normal_a.z + barycentric_b * normal_b.z + barycentric_c * normal_c.z;
                    normalize(normal);  // interpolation breaks normalization

                    if (this->meshes[k]->get_texture() != 0)
                      {
                        double u,v;

                        u = barycentric_a * texture_coords_a[0] + barycentric_b * texture_coords_b[0] + barycentric_c * texture_coords_c[0];
                        v = barycentric_a * texture_coords_a[1] + barycentric_b * texture_coords_b[1] + barycentric_c * texture_coords_c[1];

                        color_buffer_get_pixel(this->meshes[k]->get_texture(),u * this->meshes[k]->get_texture()->width,v * this->meshes[k]->get_texture()->height,&final_color.red,&final_color.green,&final_color.blue);
                      }
                    else
                      {
                        final_color.red = 255;
                        final_color.green = 255;
                        final_color.blue = 255;
                      }

                    helper_color = compute_lighting(intersection,mat,normal);

                    final_color = multiply_colors(helper_color,final_color);

                    if (recursion_depth != 0)
                      {
                        incoming_vector_reverse = line.get_vector_to_origin();

                        if (mat.reflection > 0)
                          {
                            point_3D helper_point;
                            reflection_vector = make_reflection_vector(normal,incoming_vector_reverse);

                            reflection_vector.x *= -1;
                            reflection_vector.y *= -1;
                            reflection_vector.z *= -1;

                            helper_point.x = intersection.x + reflection_vector.x;
                            helper_point.y = intersection.y + reflection_vector.y;
                            helper_point.z = intersection.z + reflection_vector.z;

                            line_3D reflection_line(intersection,helper_point);


                            add_color = cast_ray(reflection_line,ERROR_OFFSET,recursion_depth - 1);

                            multiply_color(add_color,mat.reflection);
                            final_color = add_colors(final_color,add_color);
                          }

                        if (mat.transparency > 0)
                          {
                            point_3D helper_point;
                            reflection_vector = make_reflection_vector(normal,incoming_vector_reverse);

                            reflection_vector.x *= -1;
                            reflection_vector.y *= -1;
                            reflection_vector.z *= -1;

                            helper_point.x = intersection.x + reflection_vector.x;
                            helper_point.y = intersection.y + reflection_vector.y;
                            helper_point.z = intersection.z + reflection_vector.z;

                            line_3D reflection_line(intersection,helper_point);


                            add_color = cast_ray(reflection_line,ERROR_OFFSET,recursion_depth - 1);

                            multiply_color(add_color,mat.reflection);
                            final_color = add_colors(final_color,add_color);
                          }
                      }
                  }
                }
              }
            }

    return final_color;
  }

color add_colors(color color1, color color2)
  {
    color result;

    result.red = saturate_int(color1.red + color2.red,0,255);
    result.green = saturate_int(color1.green + color2.green,0,255);
    result.blue = saturate_int(color1.blue + color2.blue,0,255);

    return result;
  }

void multiply_color(color &c, double a)
  {
    c.red = saturate_int(c.red * a,0,255);
    c.green = saturate_int(c.green * a,0,255);
    c.blue = saturate_int(c.blue * a,0,255);
  }

void scene_3D::render(t_color_buffer *buffer, void (* progress_callback)(int))
  {
    color_buffer_init(buffer,this->resolution[0],this->resolution[1]);

    unsigned int i, j, k;
    point_3D point1, point2;
    double aspect_ratio, angle, distance;
    color ray_color, helper_color;
    unsigned int color_sum[3];

    aspect_ratio = this->resolution[1] / ((double) this->resolution[0]);

    for (j = 0; j < this->resolution[1]; j++)
      {
        if (progress_callback != NULL)
          progress_callback(j);

        for (i = 0; i < this->resolution[0]; i++)
          {
            point1.x = 0;
            point1.y = -this->focal_distance;
            point1.z = 0;

            point2.x = i / ((double) this->resolution[0]) - 0.5;
            point2.y = 0;
            point2.z = -1 * aspect_ratio * (j / ((double) this->resolution[1]) - 0.5);

            line_3D line(point1,point2);
            ray_color = this->cast_ray(line,ERROR_OFFSET,1); // main ray

            if (this->depth_of_field_rays != 1)
              {
                color_sum[0] = ray_color.red;
                color_sum[1] = ray_color.green;
                color_sum[2] = ray_color.blue;

                point2.x = point2.x * this->focus_distance;
                point2.y = (point2.y + this->focal_distance) * this->focus_distance - this->focal_distance; // the vector must be shifted to (0,0,0) before multiplication, then shifted back
                point2.z = point2.z * this->focus_distance;

                for (k = 1; k < this->depth_of_field_rays; k++)   // additional rays (for dept of field)
                  {
                    angle = random_double() * 2 * PI;   // random position in polar coordinates
                    distance = random_double() * this->lens_width / 2.0;

                    point1.x = distance * cos(angle);
                    point1.z = distance * sin(angle);

                    line_3D line2(point1,point2);

                    helper_color = this->cast_ray(line2,ERROR_OFFSET,1);

                    color_sum[0] += helper_color.red;
                    color_sum[1] += helper_color.green;
                    color_sum[2] += helper_color.blue;
                  }

                color_sum[0] /= this->depth_of_field_rays;
                color_sum[1] /= this->depth_of_field_rays;
                color_sum[2] /= this->depth_of_field_rays;

                ray_color.red = color_sum[0];
                ray_color.green = color_sum[1];
                ray_color.blue = color_sum[2];
              }

            color_buffer_set_pixel(buffer,i,j,ray_color.red,ray_color.green,ray_color.blue);
          }
      }
  }


double light_3D::get_intensity()
  {
    return this->intensity;
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

bool line_3D::intersects_triangle(triangle_3D triangle, double &a, double &b, double &c, double &t)
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

    t = (-qa * this->c0 - qb * this->c1 - qc * this->c2 - d) / denominator;

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

light_3D::light_3D()
  {
    this->position.x = 0.0;
    this->position.y = 0.0;
    this->position.z = 0.0;
    this->intensity = 1.0;
    this->light_color.red = 255;
    this->light_color.green = 255;
    this->light_color.blue = 255;
    this->light_color.alpha = 255;
    this->distance_factor = 20;
  }


