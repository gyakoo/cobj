//
//

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <float.h>
#include <GLFW/glfw3.h>
#pragma comment(lib, "glfw3.lib")

#define COBJ_IMPLEMENTATION
#include "cobj.h"

#define SCREENWIDTH 1024
#define SCREENHEIGHT 768
#define ENLARGEORTHO 1.20f

// ortho proj
double viewbounds[]={-200, 200, -200, 200, -300, 300};
cobj g_obj={0};
int g_wf=0;

float maxf(float a, float b)  { return a>b?a:b;}
void swapf(float* a, float* b){ const float t=*a; *a=*b; *b=t; }

void adjustOrthoBounds(cobj* o)
{
  float invar, dim, maxd;
  float w = (o->maxext.x - o->minext.x)*ENLARGEORTHO;
  float h = (o->maxext.y - o->minext.y)*ENLARGEORTHO;
  float d = (o->maxext.z - o->minext.z)*ENLARGEORTHO;

  if ( w>h )
  {
    invar = (float)SCREENHEIGHT/SCREENWIDTH;
  }
  else
  {
    invar = (float)SCREENWIDTH/SCREENHEIGHT;
    swapf(&w,&h);
  }

  maxd = (w>d) ? maxf(w,h) : maxf(h,d);
  dim = w*0.5f;
  viewbounds[0]=-dim; viewbounds[1]=dim;
  dim*=invar;
  viewbounds[2]=-dim; viewbounds[3]=dim;
  dim=maxd*0.5f;
  viewbounds[4]=-dim; viewbounds[5]=dim;
}

void drawObj(cobj* o)
{
  static float a=0.0f;
  unsigned int i,j,f,k,c;
  float tx=0.0f,ty=0.0f;
  cobjXYZ* xyz;
  cobjGr* gr;
  
  a+=0.01f;
  if ( (o->maxext.x - o->minext.x) > (o->maxext.y - o->minext.y) ) ty = -o->center.y;
  else tx = -o->center.x;

  glRotatef(a,0,1,0);
  glTranslatef(tx,ty,0);

  for (i=0;i<o->g_c;++i)
  {
    gr=o->g+i;
    c=gr->f_c/3;
    glBegin(GL_TRIANGLES);

    for (j=0; j<c; ++j)
    {
      for (k=0;k<3;++k)
      {
        f=gr->v[j*3+k];
        if ( f>o->xyz_c )  
          continue;
        glVertex3fv((const GLfloat*)(o->xyz + f));
        if ( gr->n )
        {
          f=gr->n[j*3+k];
          glNormal3fv((const GLfloat*)(o->n+f));
        }
      }
    }
    glEnd();
  }
}
void frame(GLFWwindow* window, int w, int h)
{
	int width = 0, height = 0;
  
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);
	glClearColor(20.0f/255.0f, 20.0f/255.0f, 90.0f/255.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_TEXTURE_2D);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(viewbounds[0], viewbounds[1], viewbounds[2], viewbounds[3], viewbounds[4], viewbounds[5]);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//glDisable(GL_DEPTH_TEST);
	glColor4ub(240,240,240,255);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);  
  drawObj(&g_obj);

	glfwSwapBuffers(window);
}

void loadNextObj(cobj* obj)
{
  static const char* names[]={"teapotball.obj", "cessna.obj", "head.obj", "hand.obj", "Shoe2.obj" };
  static int curObj=0;

  cobj_release(obj);
  cobj_load_from_filename(names[curObj], obj);
  adjustOrthoBounds(obj);
  curObj = (curObj+1)%5;
}

void keycallback(GLFWwindow* w, int key, int scancode, int action, int mods)
{
  if ( key==GLFW_KEY_ESCAPE && action == GLFW_PRESS )
  {
    glfwSetWindowShouldClose(w, GL_TRUE);
  }
  if ( key==GLFW_KEY_SPACE && action == GLFW_PRESS )
  {
    loadNextObj(&g_obj);
  }
  if ( key==GLFW_KEY_W && action==GLFW_PRESS )
  {
    g_wf = 1-g_wf;
    glPolygonMode(GL_FRONT_AND_BACK, g_wf?GL_LINE:GL_FILL);
  }
}

void test()
{
  char* line="1/1/1 2/2/2 3/3/3 4/4/4\n";
  cobjGr gr={0};

  cobj_count_faces(line,&gr);

  gr.f_c=0; gr.n=gr.v=gr.uv=0;
  line="1//1 2//2 3//3\n";
  cobj_count_faces(line,&gr);

  gr.f_c=0; gr.n=gr.v=gr.uv=0;
  line="1 2 3 4 5 6\n";
  cobj_count_faces(line,&gr);
}


int main()
{
	GLFWwindow* window;
	const GLFWvidmode* mode;
  GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
  GLfloat mat_shininess[] = { 50.0 };
  GLfloat light_position[] = { 0.0, 50.0, -100.0, 0.0 };

#ifdef _DEBUG
  _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

  test();
  loadNextObj(&g_obj);
	if (!glfwInit())
		return -1;

	mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
  window = glfwCreateWindow(SCREENWIDTH, SCREENHEIGHT, "cobj example", NULL, NULL);
	if (!window)
	{
		printf("Could not open window\n");
		glfwTerminate();
		return -1;
	}

	glfwSetFramebufferSizeCallback(window, frame);
	glfwMakeContextCurrent(window);
  glfwSetKeyCallback(window, keycallback);
    
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_LINE_SMOOTH);  
  glClearColor (0.0, 0.0, 0.0, 0.0);
  glShadeModel (GL_SMOOTH);

  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
  glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);

  //glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_DEPTH_TEST);


	while (!glfwWindowShouldClose(window))
	{
		frame(window,0,0);
		glfwPollEvents();
	}

	glfwTerminate();
  cobj_release(&g_obj);
	return 0;
}
