# cobj
Single-Header fast Alias Wavefront OBJ loader written in C

# support
* custom memory allocator functions
* material library (optional and shareable), see struct cobjMtl
* negative indices
* polygons > 3 vertices
* 16/32 bits for indices


# todo
* Option to Expand vertices+uv+normals for glDrawElements/glDrawArrays format 
* Option to compute normals when no presents
* Binary version serialization. defines to include parsing/binary code.
* Export to DX/GL .h binary data to embed into data segment
* Smoothing groups and other OBJ elements not supported so far

# some numbers 
(i5 3.00GHz, 12GB, win8 64bits) 
Parses and load: 
* 90MB buddha.obj in ~3.2 s (39Mb footprint allocation)
* 72MB dragon.obj in ~2.7 s (31Mb footprint allocation)

# some screenshots
<img src="data/buddha.png" width="50%" />
<img src="data/dragon.png" width="50%" />
