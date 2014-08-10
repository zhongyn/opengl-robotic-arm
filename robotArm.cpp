#include <stdio.h>
#include <stdlib.h>
#include <math.h>
/* Use glew.h instead of gl.h to get all the GL prototypes declared */
#include <GL/glew.h>
/* Using the GLUT library for the base windowing setup */
#include <GL/freeglut.h>
/* Use GLM mathematics library*/
//#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
/* Use shader_utils.h for create and link shaders */
#include "shader_utils.h"
#include <iostream>
using namespace std;
/* Use the SOIL library for loading image */
#include <SOIL/SOIL.h>


// Global variables
int screen_width=600, screen_height=600;
int last_mx = 0, last_my = 0, cur_mx = 0, cur_my = 0;
int trackball_on = false;
int inputModeOn = false;
float ratio = 10.0;
GLuint spere_vbo, cylinder_vbo, rect_vbo;
GLuint sphere_vao, cylinder_vao, rect_vao;

GLuint program;
GLuint mytexture_id;
GLint attribute_v_coord;
GLint uniform_mytexture;
GLint uniform_m, uniform_v, uniform_p;

glm::mat4 model = glm::mat4(1.0f);
glm::mat4 view = glm::lookAt(glm::vec3(0.0, 0.0, 5.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
//glm::mat4 project = glm::perspective(45.0f, 1.0f*screen_width/screen_height, 0.1f, 25.0f);
glm::mat4 project = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,-15.0f,15.0f);
glm::mat4 instance;
float angle[2] = {0.0, 0.0};
float newAng = 0;
const glm::vec3 axisZ = glm::vec3(0.0, 0.0, 1.0);
float armLength[2] = {3.0,3.0};
glm::mat4 moveTorso = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f));

struct filename {;
    const char* texture_filename;
    const char* vshader_filename;
    const char* fshader_filename;
};
//struct filename files = {"Earthmap.jpg","sphere.v.glsl","sphere.f.glsl"};
//struct filename files = {"BasketballColor.jpg","sphere.v.glsl","sphere.f.glsl"};
//struct filename files = {"SoftballColor.jpg","sphere.v.glsl","sphere.f.glsl"};
//struct filename files = {"SoftballBump.jpg","sphere.v.glsl","sphere.f.glsl"};
//struct filename files = {"NewTennisBallColor.jpg","sphere.v.glsl","sphere.f.glsl"};
struct filename files = {"Pool Ball Skins/Ball13.jpg","robotArm.v.glsl","robotArm.f.glsl"};

int spSlices = 30;
int spStacks = 30;
float spRadius = 1.0;

int cySlices = 30;
float cyHeight = 1.0;
float cyRadius = 1.0;

// Create a 3D sphere.
GLuint sphere(float radius, int slices, int stacks) {
  GLuint vbo;
  int n = 2 * (slices + 1) * stacks;
  int i = 0;
  glm::vec3 points[n];
  float stepsk = M_PI/stacks;
  float stepsl = 2*M_PI/slices;

  for (float theta = 0.0; theta < M_PI - 0.0001; theta += stepsk) {
    for (float phi = 0.0; phi <= 2*M_PI + 0.0001; phi += stepsl) {
      points[i++] = glm::vec3(sin(theta) * sin(phi), cos(theta), sin(theta) * cos(phi))*radius;
      points[i++] = glm::vec3(sin(theta + stepsk) * sin(phi), cos(theta + stepsk), sin(theta + stepsk) * cos(phi))*radius;
    }
  }


  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof points, points, GL_STATIC_DRAW);
  
  return vbo;
}

// Create a cylinder.
GLuint cylinder(float radius, float height, int slices) {
  GLuint vbo;
  int n = 6*slices;
  int i = 0;
  glm::vec3 points[n];
  float step = 2*M_PI/slices;

  for (float theta = 0.0; theta < 2*M_PI-0.0001; theta += step){
    points[i++] = glm::vec3(0.0f)*radius;
    points[i++] = glm::vec3(cos(theta),0.0,sin(theta))*radius;
    points[i++] = glm::vec3(cos(theta+step),0.0,sin(theta+step))*radius;
    points[i++] = glm::vec3(cos(theta),height,sin(theta))*radius;
    points[i++] = glm::vec3(cos(theta+step),height,sin(theta+step))*radius;
    points[i++] = glm::vec3(0.0,height,0.0)*radius;
  }
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof points, points, GL_STATIC_DRAW);
  
  return vbo;
}

// Create a rectangle
GLuint rect()
{
  GLuint vbo;
  glm::vec3 points[4] = {
    glm::vec3(-1.0,-1.0,0.0),
    glm::vec3(-1.0,1.0,0.0),
    glm::vec3(1.0,-1.0,0.0),
    glm::vec3(1.0,1.0,0.0)    
  };

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof points, points, GL_STATIC_DRAW);
  
  return vbo;
}


// Initial VBO, shader program, attributes, uniforms, and load texture.
int init_resources()
{
	printf("init_resources: %s, %s, %s\n", files.texture_filename, files.vshader_filename, files.fshader_filename);

	// Create spere VBO//
	spere_vbo = sphere(spRadius, spSlices, spStacks);
  cylinder_vbo = cylinder(cyRadius, cyHeight, cySlices);
  rect_vbo = rect();

	// Load texture image using SOIL//
	mytexture_id = SOIL_load_OGL_texture
    (
     files.texture_filename,
     SOIL_LOAD_AUTO,
     SOIL_CREATE_NEW_ID,
     SOIL_FLAG_INVERT_Y | SOIL_FLAG_TEXTURE_REPEATS
     );
  	if (mytexture_id == 0)
    	cerr << "SOIL loading error: '" << SOIL_last_result() << "' (" << files.texture_filename << ")" << endl;
    
    // Create and link shader//
    GLint link_ok = GL_FALSE;
    GLuint vs, fs;
    if ((vs = create_shader(files.vshader_filename, GL_VERTEX_SHADER))   == 0) return 0;
    if ((fs = create_shader(files.fshader_filename, GL_FRAGMENT_SHADER)) == 0) return 0;

    program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
    if (!link_ok) {
      fprintf(stderr, "glLinkProgram:");
      print_log(program);
      return 0;
    }

    const char* attribute_name;
    attribute_name = "v_coord";
    attribute_v_coord = glGetAttribLocation(program, attribute_name);
    if (attribute_v_coord == -1) {
      fprintf(stderr, "Could not bind attribute %s\n", attribute_name);
      return 0;
    }
    
    const char* uniform_name;
    uniform_name = "mytexture";
    uniform_mytexture = glGetUniformLocation(program, uniform_name);
    if (uniform_mytexture == -1) {
      fprintf(stderr, "Could not bind uniform %s\n", uniform_name);
      return 0;
    }

    uniform_name = "m";
    uniform_m = glGetUniformLocation(program, uniform_name);
    if (uniform_m == -1) {
    	fprintf(stderr, "Could not bind uniform %s\n", uniform_name);
    	return 0;
    }

    uniform_name = "v";
    uniform_v = glGetUniformLocation(program, uniform_name);
    if (uniform_v == -1) {
    	fprintf(stderr, "Could not bind uniform %s\n", uniform_name);
    	return 0;
    }

    uniform_name = "p";
    uniform_p = glGetUniformLocation(program, uniform_name);
    if (uniform_p == -1) {
    	fprintf(stderr, "Could not bind uniform %s\n", uniform_name);
    	return 0;
    }
    return 1;
}


void initVAO()
{
  glGenVertexArrays(1, &sphere_vao);
  glBindVertexArray(sphere_vao);
  glEnableVertexAttribArray(attribute_v_coord);
  glBindBuffer(GL_ARRAY_BUFFER, spere_vbo);
  glVertexAttribPointer(attribute_v_coord, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glBindVertexArray(0);

  glGenVertexArrays(1, &cylinder_vao);
  glBindVertexArray(cylinder_vao);  
  glEnableVertexAttribArray(attribute_v_coord);
  glBindBuffer(GL_ARRAY_BUFFER, cylinder_vbo);
  glVertexAttribPointer(attribute_v_coord, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glBindVertexArray(0);

  glGenVertexArrays(1, &rect_vao);
  glBindVertexArray(rect_vao);  
  glEnableVertexAttribArray(attribute_v_coord);
  glBindBuffer(GL_ARRAY_BUFFER, rect_vbo);
  glVertexAttribPointer(attribute_v_coord, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glBindVertexArray(0);

}

void root()
{
  glBindVertexArray(sphere_vao);
  instance = glm::rotate(glm::mat4(1.0f), -90.0f, axisZ) * glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
  glUniformMatrix4fv(uniform_m, 1, GL_FALSE, glm::value_ptr(model*instance));
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 2 * (spSlices+1) * spStacks);
}

void lowerArm()
{
  glBindVertexArray(cylinder_vao);
  instance = glm::rotate(glm::mat4(1.0f), -90.0f, axisZ) * glm::scale(glm::mat4(1.0f), glm::vec3(1.0, armLength[0], 1.0));
  glUniformMatrix4fv(uniform_m, 1, GL_FALSE, glm::value_ptr(model*instance));
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 6*cySlices);
}

void node()
{
  glBindVertexArray(sphere_vao);
  instance = glm::rotate(glm::mat4(1.0f), -90.0f, axisZ) * glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
  glUniformMatrix4fv(uniform_m, 1, GL_FALSE, glm::value_ptr(model*instance));
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 2 * (spSlices+1) * spStacks);
}

void upperArm()
{
  glBindVertexArray(cylinder_vao);
  instance = glm::rotate(glm::mat4(1.0f), -90.0f, axisZ) * glm::scale(glm::mat4(1.0f), glm::vec3(1.0, armLength[1], 1.0));
  glUniformMatrix4fv(uniform_m, 1, GL_FALSE, glm::value_ptr(model*instance));
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 6*cySlices);
}

void leaf()
{
  glBindVertexArray(sphere_vao);
  instance = glm::rotate(glm::mat4(1.0f), -90.0f, axisZ) * glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
  glUniformMatrix4fv(uniform_m, 1, GL_FALSE, glm::value_ptr(model*instance));
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 2 * (spSlices+1) * spStacks);
}

void slider1()
{
  glBindVertexArray(cylinder_vao);
  instance = glm::translate(glm::mat4(1.0f), glm::vec3(armLength[0]+armLength[1],-armLength[0]/2.0,0.0)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.2, armLength[0], 0.2));
  glUniformMatrix4fv(uniform_m, 1, GL_FALSE, glm::value_ptr(model*instance));
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 6*cySlices);
}

void slider2()
{
  glBindVertexArray(cylinder_vao);
  instance = glm::translate(glm::mat4(1.0f), glm::vec3(-(armLength[0]+armLength[1]),-armLength[0]/2.0,0.0)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.2, armLength[0], 0.2));
  glUniformMatrix4fv(uniform_m, 1, GL_FALSE, glm::value_ptr(model*instance));
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 6*cySlices);
}

void slider3()
{
  glBindVertexArray(cylinder_vao);
  instance = glm::translate(glm::mat4(1.0f), glm::vec3(armLength[0]/2.0,armLength[0]+armLength[1],0.0)) * glm::rotate(glm::mat4(1.0f), 90.0f, axisZ) * glm::scale(glm::mat4(1.0f), glm::vec3(0.2, armLength[0], 0.2));
  glUniformMatrix4fv(uniform_m, 1, GL_FALSE, glm::value_ptr(model*instance));
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 6*cySlices);
}

void slider4()
{
  glBindVertexArray(cylinder_vao);
  instance = glm::translate(glm::mat4(1.0f), glm::vec3(armLength[0]/2.0,-(armLength[0]+armLength[1]),0.0)) * glm::rotate(glm::mat4(1.0f), 90.0f, axisZ) * glm::scale(glm::mat4(1.0f), glm::vec3(0.2, armLength[0], 0.2));
  glUniformMatrix4fv(uniform_m, 1, GL_FALSE, glm::value_ptr(model*instance));
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 6*cySlices);
}

void inverKinematic()
{
  float xx = (1.0*cur_mx/screen_width*2 -1.0);
  float yy = -(1.0*cur_my/screen_height*2 -1.0);
  float norm = xx*xx+yy*yy;
  float am1 = armLength[0]/ratio;
  float am2 = armLength[1]/ratio;
  float arm = am1+am2;
  newAng = glm::sign(yy)*acos(xx/sqrt(norm));

  if (sqrt(norm)<=arm)
  {
    float rad1 = acos((am1*am1+norm-am2*am2)/(2*am1*sqrt(norm)));
    float rad2 = acos(-(am1*am1-norm+am2*am2)/(2*am1*am2));
    angle[0] = (newAng-rad1)*180/M_PI;
    angle[1] = rad2*180/M_PI;
    moveTorso = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f));
  }
  else
  {
    angle[0] = newAng*180/M_PI;
    angle[1] = 0.0;
    moveTorso = glm::translate(glm::mat4(1.0f), glm::vec3(xx-arm*cos(newAng), yy-arm*sin(newAng), 0.0)*ratio);
  }
  glutPostRedisplay();  

}

void draw()
{
	glClearColor(0.0, 0.0, 0.0, 1.0);
	//glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glUseProgram(program);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mytexture_id);
	glUniform1i(uniform_mytexture, /*GL_TEXTURE*/0);

  glUniformMatrix4fv(uniform_v, 1, GL_FALSE, glm::value_ptr(view));
  glUniformMatrix4fv(uniform_p, 1, GL_FALSE, glm::value_ptr(project));

  initVAO();

  model = glm::mat4(1.0f);

  slider1();
  slider2();
  slider3();
  slider4();

  
  model = model * moveTorso * glm::rotate(glm::mat4(1.0f), angle[0], axisZ);
  root();
  lowerArm();

  model = model * glm::translate(glm::mat4(1.0f), glm::vec3(armLength[0],0.0,0.0)) * glm::rotate(glm::mat4(1.0f), angle[1], axisZ);
  node();
  upperArm();

  model = model * glm::translate(glm::mat4(1.0f), glm::vec3(armLength[1],0.0,0.0));
  leaf();

}


void onDisplay()
{
   if (!inputModeOn)
  {
    inverKinematic();
  }
  
	draw();
	glutSwapBuffers();
}


void onReshape(int width, int height)
{
	screen_width = width;
	screen_height = height;
	glViewport(0, 0, screen_width, screen_height);
}


void onMouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		last_mx = cur_mx = x;
		last_my = cur_my = y;
		trackball_on = true;
  
	}
	else trackball_on = false;
}


void onMotion(int x, int y)
{
	if (trackball_on)
	{
		cur_mx = x;
		cur_my = y;
	}
}


void keyfunc( unsigned char key, int x, int y)
{
  switch( key ) {
    case 27:  //escape
      exit ( 0 );
      break;
    case 'i':
      inputModeOn = true;
      cout << "Angle between torso and upper arm:\n";
      cin >> angle[0];
      cout << "Angle between upper and lower arms:\n";
      cin >> angle[1];
      break;
    case 'q':
      inputModeOn = false;
      break;
  }
    glutPostRedisplay();

}


// Free memory after closing windows//
void free_resources()
{
	glDeleteProgram(program);
	glDeleteTextures(1, &mytexture_id);
}


int main(int argc, char* argv[]) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA|GLUT_ALPHA|GLUT_DOUBLE|GLUT_DEPTH);
  glutInitWindowSize(screen_width, screen_height);
  glutCreateWindow("Robot Arm");


  GLenum glew_status = glewInit();
  if (glew_status != GLEW_OK) {
    fprintf(stderr, "Error: %s\n", glewGetErrorString(glew_status));
    return EXIT_FAILURE;
  }

  if (!GLEW_VERSION_2_0) {
    fprintf(stderr, "Error: your graphic card does not support OpenGL 2.0\n");
    return EXIT_FAILURE;
  }

  if (init_resources()) {
    glutDisplayFunc(onDisplay);
    glutReshapeFunc(onReshape);
 	  glutMouseFunc(onMouse);
 	  glutMotionFunc(onMotion);
    glutKeyboardFunc (keyfunc);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glutMainLoop();
  }

  free_resources();
  return EXIT_SUCCESS;
}