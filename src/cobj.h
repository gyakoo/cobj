/*
Disclaimer:
THIS IS WORK IN PROGRESS, SOME FUNCTIONS AREN'T HEAVILY TESTED FOR BEHAVIOR AND PERFORMANCE.

 */

#ifndef COBJ_H_
#define COBJ_H_

#include <math.h>
#ifdef _DEBUG
#include <intrin.h>
#endif
#ifdef __cplusplus
extern "C" {
#endif

  typedef struct cobjXYZ{ float x,y,z; }cobjXYZ;
  typedef struct cobjUV{ float u,v; }cobjUV;
  typedef struct cobjG32{ char* name;  int f_c; int *v, *uv, *n; }cobjG32;
  
  typedef struct cobj
  {
    cobjXYZ* xyz;
    cobjUV* uv;
    cobjXYZ* n;
    cobjG32* g;
    int xyz_c, uv_c, n_c, g_c;
    void* membuffer;
  }cobj;


  int cobj_load_from_filename(const char* filename, cobj* obj);
  void cobj_release(cobj* obj);
  
#ifdef __cplusplus
};
#endif

#endif // COBJ

#ifdef COBJ_IMPLEMENTATION

int cobj_load_from_filename(const char* filename, cobj* obj)
{
  char line[512];
  FILE* f = fopen(filename, "rt");

  if ( !f ) return 0;

  // preparse, (counting)
  obj->xyz = 0; obj->uv = 0; obj->n = 0; obj->g;
  obj->xyz_c = obj->uv_c = obj->n_c = obj->g_c = 0;
  obj->membuffer=0;
  while (fgets(line,sizeof(line),f))
  {
    if ( !*line || *line=='#' || *line=='\r' || *line=='\n' ) continue;
    switch ( *line )
    {
    case 'v':
      switch ( *(line+1) )
      {
      case ' ':
      case 'n':
      case 't':
      }
    case 'f':
    case 'g':

    }    
  }

  // memory allocation

  // actual parsing
  fseek(f,0,SEEK_SET);


  // finishing up
  fclose(f);
  return 1;
}

void cobj_release(cobj* obj)
{
  free(obj->membuffer);
}


#ifdef _MSC_VER
# pragma warning (disable: 4996 4100 4204)
#endif


#ifdef _MSC_VER
# pragma warning (default: 4996 4100 4204)
#endif

#endif
