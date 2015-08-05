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
#define COBJ_INDEXBITS 32
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
    cobjXYZ minext, maxext, center;
    unsigned int xyz_c, uv_c, n_c, g_c;
  }cobj;

  void cobj_count_from_file(FILE* file, cobj* obj);
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
#define COBJ_P_SPACES 0
#define COBJ_P_V 1
#define COBJ_P_VT 2
#define COBJ_P_VN 3

// counts number of indices per face and face components (only v, v and vt, v and vn, all)
void cobj_count_ipf(const char* line, cobjGr* g)
{
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
      if (digic)
      {
        if ( !g->v ) g->v=(itype*)1;
        else         g->n=(itype*)1;
      }
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
  if ( g->ipf > 4 ) g->ipf=4;
}

void cobj_count_from_file(FILE* file, cobj* obj)
{
  char line[512];
  long groupOffs=0;
  int pass,g=-1;
  int facefound=0;

  obj->xyz_c = obj->uv_c = obj->n_c = obj->g_c = 0;
  for (pass=0;pass<2;++pass)
  {
    while (fgets(line,sizeof(line),file))
    {
      if ( !*line || *line=='#' || *line=='\r' || *line=='\n' || (*line=='g' && (*(line+1)=='\r' || *(line+1)=='\n')) ) continue;
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
          case 'f': facefound=1;
            if (obj->g_c) break;
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
          case 'f': 
            if ( !facefound ) continue;
            if ( g==-1 ) g=0;
            ++obj->g[g].f_c; 
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
      if (!obj->g_c && facefound) 
        obj->g_c=1;
      if (obj->g_c) obj->g = (cobjGr*)calloc( obj->g_c, sizeof(cobjGr) );
      else obj->g=0;
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

char* cobj_parse_nextind(char* line, itype* i)
{
  register char* startptr=line;
  if ( !line || !*line ) 
    return NULL;
  while ( *line && *line!='/' && *line!=' ' && *line!='\r' && *line!='\n' )
    ++line;
  *line=0;
  if (startptr!=line)
    *i = (itype)(atoi(startptr)-1);
  else if ( i )
    return NULL;  
  return ++line;
}

// assumes starts correctly with first index
void cobj_parse_face(char* line, cobjGr* gr, unsigned int f)
{
  unsigned int i,flags=0;
  unsigned int maxipf=gr->ipf;
 
  if ( maxipf>4 ) maxipf=4;
  if (gr->v ) { flags |= 1<<0; }
  if (gr->uv) { flags |= 1<<1; }
  if (gr->n ) { flags |= 1<<2; }
  
  for (i=0;i<maxipf;++i)
  {
    switch(flags)
    {
    case 1:
      line=cobj_parse_nextind(line,gr->v+f);
    break;
    case 3:
      line=cobj_parse_nextind(line,gr->v+f);
      line=cobj_parse_nextind(line,gr->uv+f);
    break;
    case 5:
      line=cobj_parse_nextind(line,gr->v+f);
      line=cobj_parse_nextind(line,NULL);
      line=cobj_parse_nextind(line,gr->n+f);
    break;
    case 7:
      line=cobj_parse_nextind(line,gr->v+f);
      line=cobj_parse_nextind(line,gr->uv+f);
      line=cobj_parse_nextind(line,gr->n+f);
    break;    
    }

    if (!line && i <= (gr->ipf-1))
    {
      if ( i<4 )
      {
        if (gr->v)  *(gr->v+f) = *(gr->v+f-1);
        if (gr->uv) *(gr->uv+f) = *(gr->uv+f-1);
        if (gr->n)  *(gr->n+f) = *(gr->n+f-1);
      }
      break;
    }
    ++f;
  }
}

#define cobj_parse2(nam) { fptr=(float*)&obj->nam[nam++]; sscanf(line, "%f %f", fptr, fptr+1); }
#define cobj_parse3(nam) { fptr=(float*)&obj->nam[nam++]; sscanf(line, "%f %f %f", fptr, fptr+1, fptr+2); }
#define cobj_checkmin_(o,f) { if (*(fptr+o)<obj->minext.f) obj->minext.f=*(fptr+o); }
#define cobj_checkmin()  { cobj_checkmin_(0,x); cobj_checkmin_(1,y); cobj_checkmin_(2,z); }
#define cobj_checkmax_(o,f) { if (*(fptr+o)>obj->maxext.f) obj->maxext.f=*(fptr+o); }
#define cobj_checkmax()  { cobj_checkmax_(0,x); cobj_checkmax_(1,y); cobj_checkmax_(2,z); }
#define cobj_checkcom_(o,f) {obj->center.f += *(fptr+o);}
#define cobj_checkcom() {cobj_checkcom_(0,x); cobj_checkcom_(1,y); cobj_checkcom_(1,z); }

int cobj_load_from_filename(const char* filename, cobj* obj)
{
  char _line[512];
  char* line;
  unsigned int xyz=0,uv=0,n=0,f=0,i=0,leni;
  int g=-1;
  float* fptr, invc;
  cobjGr* gr;

  FILE* file = fopen(filename, "rt");

  if ( !file ) return 0;

  // preparsing, counting
  cobj_count_from_file(file,obj);
  obj->minext.x=obj->minext.y=obj->minext.z=FLT_MAX;
  obj->maxext.x=obj->maxext.y=obj->maxext.z=-FLT_MAX;
  obj->center.x=obj->center.y=obj->center.z=0.0f;

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
    if ( !*line || *line=='#' || *line=='\r' || *line=='\n' || (*line=='g' && (*(line+1)=='\r' || *(line+1)=='\n')) ) continue;
    switch ( *line )
    {
    case 'v':
      line+=2;
      switch ( *(line-1) )
      {
      case ' ': cobj_parse3(xyz); cobj_checkmin(); cobj_checkmax(); cobj_checkcom(); break;
      case 't': cobj_parse2(uv);  break;
      case 'n': cobj_parse3(n); break;
      }break;
    case 'g': 
      ++g;
      gr = obj->g+g;
      gr->name = strdup(line+2);
      f=0;
      break;
    case 'f':
      if ( g==-1 ) g=0;
      line+=2;
      cobj_parse_face(line,gr,f);      
      f+=gr->ipf;
      break;
    }
  }

  // center of mass
  invc = 1.0f/obj->xyz_c;
  obj->center.x *= invc; obj->center.y *= invc; obj->center.z *= invc;

  // finishing up
  fclose(file);
  return obj->g_c!=0;
}

void cobj_release(cobj* obj)
{
  unsigned int i;

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
