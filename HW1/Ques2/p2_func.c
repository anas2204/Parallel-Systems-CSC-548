/*
Group info:
hkhetaw Harsh Khetawat
asiddiq Anas Siddiqui
rkrish11 Rahul Krishna
*/
#include <math.h>

/* floating point precision type definitions */
typedef   double   FP_PREC;

//returns the function y(x) = fn
FP_PREC fn(FP_PREC x)
{
  return sqrt(x);
}

//returns the derivative d(fn)/dx = dy/dx
FP_PREC dfn(FP_PREC x)
{
  return 0.5*(1.0/sqrt(x));
}

//returns the integral from a to b of y(x) = fn
FP_PREC ifn(FP_PREC a, FP_PREC b)
{
  return (2./3.) * (pow(sqrt(b), 3) - pow(sqrt(a),3));
}
