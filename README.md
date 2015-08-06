# cobj
Single-header OBJ (Alias Wavefront) loader written in C 

<a href="src/cobj.h">cobj.h</a> 

# features
* no dependencies
* custom memory allocator functions
* three modes of allocation: 
 * exact amount (2 passes reader) 
 * double-the-size array type
 * callback (no allocation)
* material library (optional and shareable), see struct cobjMtl
* negative indices
* polygons > 3 vertices
* 16/32 bits for indices
* can read/parse fast and efficiently +100Mb obj files

# todo
* make three allocation modes available by flag
* Option to Expand vertices+uv+normals for glDrawElements/glDrawArrays format 
* Option to compute normals when no presents
* Binary version serialization. defines to include parsing/binary code.
* Export to DX/GL .h binary data to embed into data segment
* Smoothing groups and other OBJ elements not supported so far
* Support for half float?
* Stripize?

# some screenshots
Models are from http://graphics.cs.williams.edu/data/meshes.xml<br/>
### buddha.obj
File size: 90Mb <br/>
Mem footprint: 39Mb (2 pass mode)<br/>
Triangles: 1,087,474 <br/>
Vertices: 549,333 <br/>
Load time: 2.3s (1 pass mode)<br/>
<img src="data/buddha.png" width="50%" /> <br/>

### dragon.obj
File size: 70Mb <br/>
Mem footprint: 31Mb (2 pass mode)<br/>
Triangles: 871,306<br/>
Vertices: 438,929 <br/>
Load time: 1.9s (1 pass mode)<br/>
<img src="data/dragon.png" width="50%" /> 

### rungholt.obj
File size: 262Mb <br/>
Mem footprint: 270Mb! (2 pass mode) <br/>
Triangles: 6,704,264 <br/>
Vertices: 12,308,528 <br/>
Load time: 5.2s (1 pass mode)<br/>
