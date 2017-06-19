#version 430 core
	
layout (local_size_x = 1,local_size_y = 1) in;
layout (rgba32f, binding = 1) uniform image2D output_image;

/*Defining viewport of 4*4 size with left corner as below*/
vec3 lower_left_corner = vec3(-2.0, -1.0, -1.0);
vec3 horizontal = vec3(4.0,0.0,0.0);
vec3 verticle = vec3(0.0,4.0,0.0);
vec3 origin =  vec3(0.0,0.0,0.0);

struct Ray
{
 vec3 A;
 vec3 B;
};

Ray init_ray(vec3 a, vec3 b)
{
  Ray r;
  r.A = a;
  r.B = b;
  return r;
}

vec3 ray_origin(Ray r)
{
 return r.A;
}

vec3 direction(Ray r)
{
 return r.B;
}

Ray get_ray(float u, float v)
{
  return init_ray(origin, lower_left_corner + u * horizontal + v * verticle);
}

vec3 point_at_parameter(Ray r, float t)
{
  return r.A + t * r.B;
}

float squared_length(vec3 p)
{
   return (p[0]*p[0] + p[1]*p[1]+ p[2]*p[2]);
}


float hit_sphere(vec3 centre, float radius, Ray r)
{
  vec3 oc = ray_origin(r) - centre;
  float a = dot(direction(r), direction(r));
  float b = dot(oc, direction(r));
  float c = dot( oc, oc) - radius * radius;

  float d = b * b - a * c;

  if(d < 0)
  {
   return -1;
  }
  else
  {
   return((-b-sqrt(d))/ a); 
  }
}


vec3 color_lerp(Ray r)
{
    
	vec3 unit_direction = normalize(direction(r));
	float t = 0.5 * (unit_direction[1] + 1.0);
	return (1.0- t) * vec3( 1.0, 1.0, 1.0) + t * vec3(0.0, 0.0, 0.4);
}

vec3 color_lerp_red(Ray r)
{
    
	vec3 unit_direction = normalize(direction(r));
	float t = 0.5 * (unit_direction[1] + 1.0);
	return (1.0- t) * vec3( 1.0, 1.0, 1.0) + t * vec3(1.0, 0.5, 0.5);
}

float rand(float x)
{
  return fract(sin(x));
}

vec3 color_sphere(Ray r)
{
  float t = hit_sphere(vec3(0.0,0.0,-3.0), 2.0, r);
  if( t > 0)
  {
     //return vec3(1.0,0.3,0.3);
     vec3 N = vec3(normalize(point_at_parameter(r, t) - vec3(0.0, 0.0, -3.0)));
	 return 0.5 * vec3(N[0]+1, N[1]+1, N[2]+1);
  }
  
  return color_lerp(r); 

}


void main(void)
{
	
  int s; 
  vec3 color = vec3(0.0,0.0,0.0);
  
  ivec2 pos = ivec2(gl_GlobalInvocationID.xy);

  //fraction to traverse vieport [0,1]
  float u = float((pos.x + rand(float(pos.x)))/800.0);
  float v = float((pos.y + rand(float(pos.y)))/800.0);
 
  //Generate ray to send through pixel
  Ray r = get_ray( u, v);
   
  color = color_sphere(r);
 
  //Gamma Correction
  color = vec3(sqrt(color[0]),sqrt(color[1]),sqrt(color[2]));	

  vec4 result = vec4(color,1.0);

  imageStore(output_image, pos, result);
  
}