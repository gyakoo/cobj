/*
The MIT License (MIT)

Copyright (c) 2015 Manu Marin / @gyakoo

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */

/*
USAGE:
  - define COBJ_IMPLEMENTATION in *one* your source (C/C++) file to expand the implementation
  - define cobj_allocate/cobj_deallocate/cobj_allocatez for custom allocator functions (default: malloc/free/calloc)
  - define COBJ_INDEXBITS with the bits used for indices. (default: 32)
*/

/*
TODO:
  - Material info
  - Allocation functions
  - Option to Expand vertices+uv+normals for glDrawElements/glDrawArrays format
  - Option to compute normals when no present
  - Binary version serialization. defines to include parsing/binary code.
  - Export to DX/GL .h binary data to embed into data segment
  - Smoothing groups and other OBJ elements not supported so far
*/

#ifndef COBJ_H_
#define COBJ_H_

#ifdef _DEBUG
#include <intrin.h>
#endif

#ifndef COBJ_INDEXBITS      // put this to 16 bits to save memory for indices, just if you're sure the objects to load have <2^16 addressable vertices
#define COBJ_INDEXBITS 32
#endif

#ifndef COBJ_MAX_IBUFF      // no of elements in the temporary buffers for the indices
#define COBJ_MAX_IBUFF 48 
#endif

#define COBJ_FLAG_MTL (1<<0)
#define COBJ_FLAG_COMPUTENORMALS (1<<1)

// https://en.wikipedia.org/wiki/Wavefront_.obj_file#Material_template_library
// Illumination modes (cobjMtl->illum) are:
/*
0. Color on and Ambient off
1. Color on and Ambient on
2. Highlight on
3. Reflection on and Ray trace on
4. Transparency: Glass on, Reflection: Ray trace on
5. Reflection: Fresnel on and Ray trace on
6. Transparency: Refraction on, Reflection: Fresnel off and Ray trace on
7. Transparency: Refraction on, Reflection: Fresnel on and Ray trace on
8. Reflection on and Ray trace off
9. Transparency: Glass on, Reflection: Ray trace off
10. Casts shadows onto invisible surfaces
*/

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

  // Group
  typedef struct cobjGr
  { 
    char* name;         // group name
    itype ndx_c;        // # of indices
    itype *v, *uv, *n;  // indices of : positions, text coords (optional/nullable), normals (optional/nullable)
  }cobjGr;

  // http://paulbourke.net/dataformats/mtl/
  typedef struct cobjMtl
  {
    char* name;     // mtl name
    char* map_ka;   // map ambient tex
    char* map_ks;   // map specular tex
    char* map_kd;   // map diffuse tex
    char* map_d;    // map alpha texture
    char* map_ns;   // map specular hightlight component
    char* map_bump; // map bump mapping
    char* map_disp; // map displacement mapping
    char* map_decal;// map stencil decal
    cobjXYZ ka;     // ambient color
    cobjXYZ kd;     // diffuse color
    cobjXYZ ks;     // specular color
    float   tr;     // transparancy
    float   ns;     // specular hightlight 
    float   sharpness; // sharpness of the reflection
    float   ni;     // index of refraction
    char    illum;  // illumination mode (see notes above) 0..10
  }cobjMtl;

  // Material library
  typedef struct cobjMatlib
  {
    cobjMtl* m;
    unsigned int m_c;
  }cobjMatlib;

  // Object in memory
  typedef struct cobj
  {
    cobjXYZ minext; // min extents of the pos vertices 
    cobjXYZ maxext; // max extents of the pos vertices
    cobjXYZ center; // centroid or geometric center 
    cobjXYZ* xyz;   // position vertices
    cobjUV* uv;     // texture coordinates vertices or NULL
    cobjXYZ* n;     // normal vertices or NULL
    cobjGr* g;      // groups. if read was ok, it should have at least 1    
    cobjMatlib matlib;

    int flags;
    unsigned int allocatedSize; // bytes allocated
    unsigned int xyz_c; // # position vertices
    unsigned int uv_c;  // # texture coords vertices
    unsigned int n_c;   // # normals vertices
    unsigned int g_c;   // # groups
  }cobj;

  // Reads an OBJ (Wavefront) file into memory. 
  // Returns 1 if succeeded, 0 otherwise
  int cobj_load_from_filename(const char* filename, cobj* obj, int flags);

  // reads a MTL (wavefront) material library into memory
  int cobj_load_matlib_from_filename(const char* filename, cobjMatlib* matlib);

  // Releases the memory associated to the obj file
  void cobj_release(cobj* obj, int flags);
  void cobj_release_matlib(cobjMatlib* matlib);
  
#ifdef __cplusplus
};
#endif

#endif // COBJ

#ifdef COBJ_IMPLEMENTATION
#ifdef _MSC_VER
# pragma warning (disable: 4996 4100 4204)
# define COBJ_DEBUGBREAK() __debugbreak()
#else
# define COBJ_DEBUGBREAK() raise(SIGTRAP)
#endif
#define COBJ_COUNTPASS_ALL 0
#define COBJ_COUNTPASS_FACES 1
#define COBJ_P_SPACES 0
#define COBJ_P_V 1
#define COBJ_P_VT 2
#define COBJ_P_VN 3
#define cobj_isdigit(c) (c>='0' && c<='9')
#define cobj_parse2(nam) { fptr=(float*)&obj->nam[nam++]; sscanf(line, "%f %f", fptr, fptr+1); }
#define cobj_parse3(nam) { fptr=(float*)&obj->nam[nam++]; sscanf(line, "%f %f %f", fptr, fptr+1, fptr+2); }
#define cobj_checkmin_(o,f) { if (*(fptr+o)<obj->minext.f) obj->minext.f=*(fptr+o); }
#define cobj_checkmin()  { cobj_checkmin_(0,x); cobj_checkmin_(1,y); cobj_checkmin_(2,z); }
#define cobj_checkmax_(o,f) { if (*(fptr+o)>obj->maxext.f) obj->maxext.f=*(fptr+o); }
#define cobj_checkmax()  { cobj_checkmax_(0,x); cobj_checkmax_(1,y); cobj_checkmax_(2,z); }
#define cobj_checkcom_(o,f) {obj->center.f += *(fptr+o);}
#define cobj_checkcom() {cobj_checkcom_(0,x); cobj_checkcom_(1,y); cobj_checkcom_(1,z); }
#define cobj_face_addv(f,l) { gr->v[f]=vbuff[l]; if(gr->uv)gr->uv[f]=uvbuff[l]; if(gr->n)gr->n[f]=nbuff[l]; }

#ifndef cobj_allocate
#define cobj_allocate cobj_allocate_cr
#endif

#ifndef cobj_deallocate
#define cobj_deallocate cobj_deallocate_cr
#endif

#ifndef cobj_allocatez
#define cobj_allocatez cobj_allocatez_cr
#endif

void* cobj_allocate_cr(unsigned int size_bytes)
{
  void* ptr= malloc(size_bytes);
#ifdef _DEBUG
  if (!ptr)
    COBJ_DEBUGBREAK();
#endif
  return ptr;
}

void cobj_deallocate_cr(void* ptr)
{
  if (ptr) 
    free(ptr);
}

void* cobj_allocatez_cr(unsigned int c, unsigned int size)
{ 
  void* ptr = calloc(c,size);
#ifdef _DEBUG
  if (!ptr)
    COBJ_DEBUGBREAK();
#endif
  return ptr;
}

// count number of faces in the line
// also mark with 1 if the position/tc/normal is present
void cobj_count_faces(char* line, cobjGr* gr)
{
  int n=0;
  char last=0;
  int slashfound=0;

  gr->v=(itype*)1; // always xyz vertex present
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

  if ( n>=3 )
    gr->ndx_c += (n-2)*3; // triangulate
}

// count total number of elements in the file
// used as pre step before allocating
void cobj_count_from_file(FILE* file, cobj* obj)
{
  char line[512];
  long groupOffs=0;
  int pass,g=-1;
  int facefound=0;

  obj->xyz_c = obj->uv_c = obj->n_c = obj->g_c = 0;
  obj->allocatedSize=0;
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
          case 'm':
            if ( !(obj->flags&COBJ_FLAG_MTL) ) break;
            if ( strncmp(line+1,"tllib",5)==0 )
            {
              if ( !obj->matlib.m )
                cobj_load_matlib_from_filename(line+7,&obj->matlib);
            }
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
      if (obj->g_c)
      {
        obj->g = (cobjGr*)cobj_allocatez( obj->g_c, sizeof(cobjGr) );
        obj->allocatedSize += obj->g_c*sizeof(cobjGr);
      }
      else obj->g=0;
    }
  }
}

// gets the next integer index in the line
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
// parses all indices in the line
// the passed buffers are for temporary storage. after parsing the indices
// are triangulated and stored in 'gr'
// return number of indices
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
  if ( n<3 ) return 0;

  for (i=2;i<n;++i)
  {
    { gr->v[f]=vbuff[0]; if(gr->uv)gr->uv[f]=uvbuff[0]; if(gr->n)gr->n[f]=nbuff[0]; }
    /*cobj_face_addv(f,0);*/ ++f;
    cobj_face_addv(f,i-1); ++f;
    cobj_face_addv(f,i); ++f;    
  }
  return (n-2)*3;
}

// load an obj from file
// it will allocate heap memory for the elements
int cobj_load_from_filename(const char* filename, cobj* obj, int flags)
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

  obj->flags = flags;
  // pre-parsing, counting
  cobj_count_from_file(file,obj);
  obj->minext.x=obj->minext.y=obj->minext.z=FLT_MAX;
  obj->maxext.x=obj->maxext.y=obj->maxext.z=-FLT_MAX;
  obj->center.x=obj->center.y=obj->center.z=0.0f;

  // memory allocation
  obj->xyz = ( obj->xyz_c ) ? (cobjXYZ*)cobj_allocate( sizeof(cobjXYZ)*obj->xyz_c ) : NULL;
  obj->uv = (obj->uv_c) ? (cobjUV*)cobj_allocate(sizeof(cobjUV)*obj->uv_c) : NULL;
  obj->n = (obj->n_c) ? (cobjXYZ*)cobj_allocate(sizeof(cobjXYZ)*obj->n_c) : NULL; 
  obj->allocatedSize += (obj->n_c+obj->xyz_c)*sizeof(cobjXYZ) + obj->uv_c*sizeof(cobjUV);
  for (i=0;i<obj->g_c;++i)
  {
    gr = obj->g+i;
    if (!gr->ndx_c ) continue;
    leni=sizeof(itype)*gr->ndx_c;
    if (gr->v) { gr->v=(itype*)cobj_allocate(leni); obj->allocatedSize+=leni; }
    if (gr->uv){ gr->uv=(itype*)cobj_allocate(leni); obj->allocatedSize+=leni; }
    if (gr->n) { gr->n=(itype*)cobj_allocate(leni); obj->allocatedSize+=leni; }
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
      obj->allocatedSize += strlen(gr->name);
      f=0;
      break;
    case 'f':
      if ( g==-1 ) g=0;
      line+=2;
      f += cobj_parse_faces(line, gr, f, vbuf, uvbuff, nbuff);
      break;
    }
  }

  // geometric center
  invc = 1.0f/obj->xyz_c;
  obj->center.x *= invc; 
  obj->center.y *= invc; 
  obj->center.z *= invc;

  // finishing up
  fclose(file);

  return obj->g_c;
}

int cobj_load_matlib_from_filename(const char* filename, cobjMatlib* matlib)
{
  return matlib->m_c;
}

void cobj_release_matlib(cobjMatlib* matlib)
{
  unsigned int i;
  cobjMtl* m;

  for (i=0;i<matlib->m_c;++i)
  {
    m=matlib->m+i;
    cobj_deallocate(m->name);
    cobj_deallocate(m->map_ka);
    cobj_deallocate(m->map_ks);
    cobj_deallocate(m->map_kd);
    cobj_deallocate(m->map_d);
    cobj_deallocate(m->map_ns);
    cobj_deallocate(m->map_bump);
    cobj_deallocate(m->map_disp);
    cobj_deallocate(m->map_decal);
  }
  cobj_deallocate(matlib->m);
  matlib->m = 0;
  matlib->m_c= 0;
}

// release allocated memory in the cobj object
void cobj_release(cobj* obj, int flags)
{
  unsigned int i;

  cobj_deallocate(obj->xyz);
  cobj_deallocate(obj->uv);
  cobj_deallocate(obj->n);
  for (i=0;i<obj->g_c;++i)
  {
    cobj_deallocate(obj->g[i].name);
    cobj_deallocate(obj->g[i].v);
    cobj_deallocate(obj->g[i].uv);
    cobj_deallocate(obj->g[i].n);
  }
  cobj_deallocate(obj->g);

  if ( (flags&COBJ_FLAG_MTL)!= 0 )
    cobj_release_matlib(&obj->matlib);  

  obj->xyz=0; obj->uv=0; obj->n=0; obj->g=0;
  obj->xyz_c = obj->uv_c = obj->n_c = obj->g_c = 0;
  obj->allocatedSize=0;  
}

#ifdef _MSC_VER
# pragma warning (default: 4996 4100 4204)
#endif

#endif
