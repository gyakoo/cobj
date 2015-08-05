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

#ifndef COBJ_MAX_IBUFF
#define COBJ_MAX_IBUFF 48
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
  typedef struct cobjGr{ char* name;  itype f_c; itype *v, *uv, *n; }cobjGr;
  
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
#define cobj_isdigit(c) (c>='0' && c<='9')

void cobj_count_faces(char* line, cobjGr* gr)
{
  int n=0;
  char last=0;
  int slashfound=0;

  gr->v=(itype*)1; // always xyz vertex referenced
  while ( *line!='\r' && *line!='\n' && n < COBJ_MAX_IBUFF)
  {
    switch (*line)
    {
      case ' ': ++n; break;
      case '/': if ( n>0 ) break;
        if ( slashfound )
        {
          if (last=='/') { gr->n=(itype*)1; }
          else if (cobj_isdigit(last)){ gr->uv=(itype*)1; gr->n=(itype*)1; }
        }        
        slashfound++;
        break;
    }

    last = *line;
    ++line;
  }
  if ( last != ' ' )
    ++n;

  if ( n==4 ) 
    n=6; // or a quad that needs to be triangulated

  if ( n%3!=0)
    n=n;
  n-=n%3;
  gr->f_c += n;
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
            cobj_count_faces(line+2, obj->g+g);
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

// assumes line starts correctly with first index
int cobj_parse_faces(char* line, cobjGr* gr, unsigned int f, itype* vbuff, itype* uvbuff, itype* nbuff)
{
  unsigned int i;
  unsigned int n=0;

  while ( line && n<COBJ_MAX_IBUFF)
  {
    line=cobj_parse_nextind(line,vbuff+n);
    if ( gr->uv )   line= cobj_parse_nextind(line, uvbuff+n);
    else if (gr->n) while (*line=='/') ++line;

    if ( gr->n )
      line = cobj_parse_nextind(line,nbuff+n);

    while (*line==' ' || *line=='\r' || *line=='\n' ) ++line;
    if ( !*line) line=NULL;
    ++n;
  }

  // triangulate if n==4
  if ( n==4 )
  {
    vbuff[4]=vbuff[0]; uvbuff[4]=uvbuff[0]; nbuff[4]=nbuff[0];
    vbuff[5]=vbuff[2]; uvbuff[5]=uvbuff[2]; nbuff[5]=nbuff[2];
    n=6;
  }
  n-=n%3;  
  for (i=0;i<n;++i,++f)
  {
    gr->v[f] = vbuff[i];
    if ( gr->uv ) gr->uv[f] = uvbuff[i];
    if ( gr->n ) gr->n[f] = nbuff[i];
  }

  return n;
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
  itype vbuf[COBJ_MAX_IBUFF], uvbuff[COBJ_MAX_IBUFF], nbuff[COBJ_MAX_IBUFF];
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
    if (!gr->f_c ) continue;
    leni=sizeof(itype)*gr->f_c;
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
      f += cobj_parse_faces(line, gr, f, vbuf, uvbuff, nbuff);
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
