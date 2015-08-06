# cobj
Single-Header fast Alias Wavefront OBJ loader written in C 

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
* can read/parse fast and efficiently +100Mb obj files (http://graphics.cs.williams.edu/data/meshes.xml)

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
<img src="data/buddha.png" width="50%" />
<img src="data/dragon.png" width="50%" />
