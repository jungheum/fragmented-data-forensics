#pragma once
#include <math.h>

/***************************************************************************** */
/*                      Def valuables                                          */
/***************************************************************************** */

#define ITMAX 30000
#define EPS 3.0e-7
#define FPMIN 1.0e-30
#define sqrt2 1.414213562373095048801688724209698078569672
#define PI 3.14159265358979323846

#define ALPHA 0.01

#define MAX(x,y)             ((x) <  (y)  ? (y)  : (x))
#define MIN(x,y)             ((x) >  (y)  ? (y)  : (x))

/***************************************************************************** */
/*                            evaluation Function                              */
/***************************************************************************** */

double gammp(double a, double x);
double gammq(double a, double x);
double gammln(double xx);
void gser(double *gamser, double a, double x, double *gln);
void gcf(double *gammcf, double a, double x, double *gln);
void nrerror(char error_text[]);
double erff(double x);
double erffc(double x);
double erfcc(double x);
double normal2(double a);
double Pr(int u, double eta);

