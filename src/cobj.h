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

  int cobj_load_from_file();
  
#ifdef __cplusplus
};
#endif

#endif // COBJ

#ifdef COBJ_IMPLEMENTATION

int cobj_load_from_file();

#ifdef _MSC_VER
# pragma warning (disable: 4996) // Switch off security warnings
# pragma warning (disable: 4100) // Switch off unreferenced formal parameter warnings
# pragma warning (disable: 4204) // Switch off nonstandard extension used : non-constant aggregate initializer
#endif

#endif
