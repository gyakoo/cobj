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
  unsigned int i,j,f;
  cobjXYZ* xyz;
  cobjGr* gr;

  for (i=0;i<o->g_c;++i)
  {
    gr=o->g+i;

    glBegin(gr->ipf==3?GL_TRIANGLES:GL_QUADS);
    for (f=0,j=0;j<gr->f_c;++j)
    {
      glVertex3fv((const GLfloat*)(o->xyz + gr->v[f++]));
      glVertex3fv((const GLfloat*)(o->xyz + gr->v[f++]));
      glVertex3fv((const GLfloat*)(o->xyz + gr->v[f++]));
      if ( gr->ipf==4 )
        glVertex3fv((const GLfloat*)(o->xyz + gr->v[f++]));
    }
    glEnd();
  }
}
float a=0.0f;
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
	glDisable(GL_DEPTH_TEST);
	glColor4ub(255,255,255,255);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  a+=0.01f;
  glTranslatef(0, -g_obj.center.y, 0);
  glRotatef(a,0,1,0);
  drawObj(&g_obj);

	glfwSwapBuffers(window);
}

void keycallback(GLFWwindow* w, int key, int scancode, int action, int mods)
{
  if ( key==GLFW_KEY_ESCAPE && action == GLFW_PRESS )
    glfwSetWindowShouldClose(w, GL_TRUE);
}


int main()
{
	GLFWwindow* window;
	const GLFWvidmode* mode;

#ifdef _DEBUG
  _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
  cobj_load_from_filename("shoe2.obj", &g_obj);

  adjustOrthoBounds(&g_obj);

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

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_LINE_SMOOTH);

	while (!glfwWindowShouldClose(window))
	{
		frame(window,0,0);
		glfwPollEvents();
	}

	glfwTerminate();
  cobj_release(&g_obj);
	return 0;
}
