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

#ifndef COBJ_MAX_GROUPS
#define COBJ_MAX_GROUPS 256
#endif

#ifndef COBJ_INDEXBITS
#define COBJ_INDEXBITS 16
#endif

#if COBJ_INDEXBITS==16
typedef unsigned short itype;
#else
typedef unsigned int itype;
#endif

#ifdef __cplusplus
extern "C" {
#endif

  typedef struct cobjXYZ{ float x,y,z; }cobjXYZ;
  typedef struct cobjUV{ float u,v; }cobjUV;
  typedef struct cobjGr{ char* name;  itype f_c, ipf; itype *v, *uv, *n; }cobjGr;
  
  typedef struct cobj
  {
    cobjXYZ* xyz;
    cobjUV* uv;
    cobjXYZ* n;
    cobjGr* g;
    unsigned int xyz_c, uv_c, n_c, g_c;
  }cobj;

  int cobj_count_from_file(FILE* file, cobj* obj);
  int cobj_load_from_filename(const char* filename, cobj* obj);
  void cobj_release(cobj* obj);
  
#ifdef __cplusplus
};
#endif

#endif // COBJ

#ifdef COBJ_IMPLEMENTATION
#ifdef _MSC_VER
# pragma warning (disable: 4996 4100 4204)
#endif
#define COBJ_COUNTPASS_ALL 0
#define COBJ_COUNTPASS_FACES 1

// counts number of indices per face and face components (only v, v and vt, v and vn, all)
void cobj_count_ipf(const char* line, cobjGr* g)
{
#define COBJ_P_SPACES 0
#define COBJ_P_V 1
#define COBJ_P_VT 2
#define COBJ_P_VN 3
  int digic=0;
  int state=COBJ_P_SPACES;
  int doneComp=0;
  
  line+=2;
  while (*line && *line!='\r' && *line!='\n')
  {
    switch (*line)
    {
    case '\t':
    case ' ' : 
      state = COBJ_P_SPACES; 
      if (digic) g->n=(itype*)1;
      doneComp=1;
      break;
    default:
      if ( state==COBJ_P_SPACES )
      {
        ++g->ipf;
        state = COBJ_P_V;
        digic=0;
      }
      if ( !doneComp )
      {
        if (*line=='/')
        {
          if (digic)
          {
            switch(state)
            {
            case COBJ_P_V: g->v=(itype*)1; break;
            case COBJ_P_VT:g->uv=(itype*)1; break;
            }
          }
          ++state;
          digic=0;
        }
        else
        {
          ++digic;
        }
      }
    }
    ++line;
  }
}

int cobj_count_from_file(FILE* file, cobj* obj)
{
  char line[512];
  long groupOffs=0;
  int pass,g=-1;

  obj->xyz_c = obj->uv_c = obj->n_c = obj->g_c = 0;  
  for (pass=0;pass<2;++pass)
  {
    while (fgets(line,sizeof(line),file))
    {
      if ( !*line || *line=='#' || *line=='\r' || *line=='\n' ) continue;
      switch (pass)
      {
      case COBJ_COUNTPASS_ALL:
          switch ( *line )
          {
          case 'v':
            switch ( *(line+1) )
            {
            case ' ': ++obj->xyz_c; break;
            case 't': ++obj->uv_c; break;
            case 'n': ++obj->n_c; break;
            }break;
          case 'g':
            if ( !groupOffs ) groupOffs = ftell(file)-sizeof(line);
            ++obj->g_c; 
          break;
          }
      break;
      case COBJ_COUNTPASS_FACES:
          switch(*line)
          {
          case 'g': ++g; break;
          case 'f': ++obj->g[g].f_c; 
            if ( !obj->g[g].ipf ) cobj_count_ipf(line,obj->g+g);
            break;
          }
      break;
      }
    }
    if ( pass==COBJ_COUNTPASS_ALL )
    {
      if ( groupOffs<0 ) groupOffs = 0;
      fseek(file,groupOffs,SEEK_SET);
      obj->g = (cobjGr*)calloc( obj->g_c, sizeof(cobjGr) );
    }
  }
}

void* cobj_allocate(unsigned int size_bytes)
{
  return malloc(size_bytes);
}

void cobj_deallocate(void** ptr)
{
  if (*ptr) free(*ptr);
  *ptr = NULL;
}
#define cobj_parse2(nam) { fptr=(float*)&obj->nam[nam++]; sscanf(line, "%f %f", fptr, fptr+1); }
#define cobj_parse3(nam) { fptr=(float*)&obj->nam[nam++]; sscanf(line, "%f %f %f", fptr, fptr+1, fptr+2); }
int cobj_load_from_filename(const char* filename, cobj* obj)
{
  char _line[512];
  char* line;
  unsigned int xyz=0,uv=0,n=0,g=-1,f=0,i=0,leni;
  float* fptr;
  cobjGr* gr;

  FILE* file = fopen(filename, "rt");

  if ( !file ) return 0;

  // preparsing, counting
  cobj_count_from_file(file,obj);

  // memory allocation
  obj->xyz = ( obj->xyz_c ) ? (cobjXYZ*)cobj_allocate( sizeof(cobjXYZ)*obj->xyz_c ) : NULL;
  obj->uv = (obj->uv_c) ? (cobjUV*)cobj_allocate(sizeof(cobjUV)*obj->uv_c) : NULL;
  obj->n = (obj->n_c) ? (cobjXYZ*)cobj_allocate(sizeof(cobjXYZ)*obj->n_c) : NULL; 
  for (i=0;i<obj->g_c;++i)
  {
    gr = obj->g+i;
    if (!gr->f_c || !gr->ipf ) continue;
    leni=sizeof(itype)*gr->ipf*gr->f_c;
    if (gr->v) gr->v=(itype*)cobj_allocate(leni);
    if (gr->uv) gr->uv=(itype*)cobj_allocate(leni);
    if (gr->n) gr->n=(itype*)cobj_allocate(leni);
  }

  // actual parsing
  fseek(file,0,SEEK_SET);
  while (fgets(_line,sizeof(_line),file))
  {
    line = _line;
    switch ( *line )
    {
    case 'v':
      line+=2;
      switch ( *(line-1) )
      {
      case ' ': cobj_parse3(xyz); break;
      case 't': cobj_parse2(uv);  break;
      case 'n': cobj_parse3(n); break;
      }break;
    case 'g': ++g;
      obj->g[g].name = strdup(line+2);
      break;
    case 'f':
      break;
    }
  }

  // finishing up
  fclose(file);
  return 1;
}

void cobj_release(cobj* obj)
{
  int i;

  cobj_deallocate((void**)&obj->xyz);
  cobj_deallocate((void**)&obj->uv);
  cobj_deallocate((void**)&obj->n);
  for (i=0;i<obj->g_c;++i)
  {
    cobj_deallocate((void**)&obj->g[i].name);
    cobj_deallocate((void**)&obj->g[i].v);
    cobj_deallocate((void**)&obj->g[i].uv);
    cobj_deallocate((void**)&obj->g[i].n);
  }
  cobj_deallocate((void**)&obj->g);
  obj->xyz_c = obj->uv_c = obj->n_c = obj->g_c = 0;
}

#ifdef _MSC_VER
# pragma warning (default: 4996 4100 4204)
#endif

#endif
