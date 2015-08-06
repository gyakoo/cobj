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

#define SCREENWIDTH 1600
#define SCREENHEIGHT 1200
#define ENLARGEORTHO 1.50f
#define UNUSED

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
void _drawObj(cobj* o )
{
  unsigned int i;

  glEnableClientState(GL_VERTEX_ARRAY);  
  glVertexPointer(3,GL_FLOAT,0,(GLvoid*)o->xyz);
  if ( o->n )
  {
    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, 0, (GLvoid*)o->n);
  }

  for (i=0;i<o->g_c;++i)
  {
    glDrawElements( GL_TRIANGLES, o->g[i].ndx_c, GL_UNSIGNED_INT, (GLvoid*)o->g[i].v );
  }

  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
  glFlush();
}

void drawObj(cobj* o)
{
  unsigned int i,j,f,k,c;
  cobjGr* gr;
  
  for (i=0;i<o->g_c;++i)
  {
    gr=o->g+i;
    c=gr->ndx_c/3;
    glBegin(GL_TRIANGLES);

    for (j=0; j<c; ++j)
    {
      for (k=0;k<3;++k)
      {
        if ( gr->n )
        {
          f=gr->n[j*3+k];
          glNormal3fv((const GLfloat*)(o->n + f));
        }

        f=gr->v[j*3+k];        
        glVertex3fv((const GLfloat*)(o->xyz + f));
      }
    }
    glEnd();
  }
  
}
void frame(GLFWwindow* window, int w, int h)
{
	int width = 0, height = 0;
  static float a=0.0f;

  UNUSED(w); UNUSED(h);

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


  a+=3.14159f*0.5f;// * 0.016f;
  glRotatef(a,0,1,0);
//  glTranslatef(tx,ty,0);

  if ( glfwGetKey(window,GLFW_KEY_1) == GLFW_PRESS )
    _drawObj(&g_obj);
  else
    drawObj(&g_obj);

	glfwSwapBuffers(window);
}

void loadNextObj(cobj* obj)
{
#define MAXFILES 9
  static const char* names[MAXFILES]={
    "dragon.obj", "buddha.obj", "sponza/sponza.obj", 
    "torusbox.obj", "teapotball.obj", "cessna.obj", 
    "head.obj", "hand.obj", "Shoe2.obj" };
  static int curObj=0;
  unsigned int ntris=0;
  unsigned int i;
  double t0;

  cobj_release(obj,COBJ_FLAG_MATERIALS);
  t0=glfwGetTime();
  if ( cobj_load_from_filename(names[curObj], obj, COBJ_FLAG_MATERIALS|COBJ_FLAG_COMPUTENORMALS) )
  {
    adjustOrthoBounds(obj);
    // some info
    printf( "%s\n", names[curObj]);
    t0=glfwGetTime()-t0;
    printf( "%.3f seconds\n", t0);
    printf( "%d bytes\n", obj->allocatedSize);
    printf( "%d groups\n", obj->g_c );
    for (i=0;i<obj->g_c;++i) ntris += obj->g[i].ndx_c/3;
    printf( "\t%d triangles\n", ntris );
    printf( "\t%d vertices, %d textured, %d normals\n", obj->xyz_c, obj->uv_c, obj->n_c);
    if ( obj->matlib.m_c )
    {
      printf( "%d materials\n", obj->matlib.m_c );
      for (i=0;i<obj->matlib.m_c;++i)
        if ( obj->matlib.m[i].name )
          printf( "\t%s\n", obj->matlib.m[i].name );
    }
    printf("\n");
  }

  curObj = (curObj+1)%MAXFILES;
}

void keycallback(GLFWwindow* w, int key, int scancode, int action, int mods)
{
  UNUSED(mods); UNUSED(scancode);
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

int main()
{
  char timestr[32];
  GLFWwindow* window;
	const GLFWvidmode* mode;
  double t0,t1,acum=0.0;
  GLint i=1;
  GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
  GLfloat mat_shininess[] = { 50.0 };
  GLfloat light_position[] = { -10.0, 10.0, -1.0, 0.0 };

#ifdef _DEBUG
  _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
  if (!glfwInit())
		return -1;

  loadNextObj(&g_obj);

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
  glfwSwapInterval(1);

	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_LINE_SMOOTH);  
  glClearColor (0.0, 0.0, 0.0, 0.0);
  glShadeModel (GL_SMOOTH);

  glLoadIdentity();
  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
  glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);
  glLightModeliv(GL_LIGHT_MODEL_TWO_SIDE, &i ); // two-sided lighting

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_NORMALIZE);

  while (!glfwWindowShouldClose(window))
  {
    t0=glfwGetTime();
    frame(window,0,0);
    glfwPollEvents();
    t1=(glfwGetTime() - t0);
    acum += t1;
    if ( acum >= 1.0 )
    {
      sprintf_s(timestr,sizeof(timestr),"%.2g", 1.0/t1);
      glfwSetWindowTitle(window,timestr);
      acum-=1.0;
    }
  }

	glfwTerminate();
  cobj_release(&g_obj, COBJ_FLAG_MATERIALS);
	return 0;
}
