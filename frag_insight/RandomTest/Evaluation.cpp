#include "stdafx.h"
#include "Evaluation.h"


/***************************************************************************** */
/*                            gammp Functition                                 */
/***************************************************************************** */
double gammp(double a, double x)
{
	void gcf(double *gammcf, double a, double x, double *gln);
	void gser(double *gamser, double a, double x, double *gln);
	void nrerror(char error_text[]);
	double gamser,gammcf,gln;

	if (x < 0.0 || a <= 0.0) nrerror("Invalid arguments in routine gammp");
	if (x < (a+1.0))
	{
		gser(&gamser,a,x,&gln);
		return gamser;
	}
	else
	{
		gcf(&gammcf,a,x,&gln);
		return 1.0-gammcf;
	}
}

/***************************************************************************** */
/*                            gammq Functition                                 */
/***************************************************************************** */
double gammq(double a, double x)
{
	void gcf(double *gammcf, double a, double x, double *gln);
	void gser(double *gamser, double a, double x, double *gln);
	void nrerror(char error_text[]);
	double gamser,gammcf,gln;

	if (x < 0.0 || a <= 0.0) nrerror("Invalid arguments in routine gammq");
	if (x < (a+1.0))
	{
		gser(&gamser,a,x,&gln);
		return 1.0-gamser;
	}
	else
	{
		gcf(&gammcf,a,x,&gln);
		return gammcf;
	}
}

/***************************************************************************** */
/*                           gammln Functition                                 */
/***************************************************************************** */
double gammln(double xx)
{
	double x,y,temp,ser;
	static double cof[6] = {76.18009172947146, -86.50532032941677, 24.01409824083091, -1.231739572450155, 0.1208650973866179e-2, -0.5395239384953e-5};
	int i;
	
	y=x=xx;
	temp=x+5.5;
	temp-=(x+0.5)*log(temp);
	ser=1.000000000190015;
	for (i=0;i<6;i++)
		ser += cof[i]/++y;
	return -temp+log(2.5066282746310005*ser/x);
}

/***************************************************************************** */
/*                             gser Functition                                 */
/***************************************************************************** */
void gser(double *gamser, double a, double x, double *gln)
{
	void nrerror(char error_text[]);
	int n;
	double sum,del,ap;
	*gln=gammln(a);
	if (x <= 0.0)
	{
		if (x < 0.0) nrerror("x less than 0 in routine gser");
		*gamser=0.0;
		return;
	}
	else
	{
		ap=a;
		del=sum=1.0/a;
		for (n=1;n<=ITMAX;n++)
		{
			++ap;
			del *= x/ap;
			sum += del;
			if (fabs(del) < fabs(sum)*EPS)
			{
				*gamser=sum*exp(-x+a*log(x)-(*gln));
				return;
			}
		}
		nrerror("a too large, ITMAX too small in routine gser");
		return;
	}
}

/***************************************************************************** */
/*                              gcf Functition                                 */
/***************************************************************************** */
void gcf(double *gammcf, double a, double x, double *gln)
{
	void nrerror(char error_text[]);
	int i;
	double an,b,c,d,del,h;

	*gln=gammln(a);
	b=x+1.0-a;
	c=1.0/FPMIN;
	d=1.0/b;
	h=d;
	for (i=1;i<=ITMAX;i++)
	{
		an = -i*(i-a);
		b += 2.0;
		d=an*d+b;
		if (fabs(d) < FPMIN) d=FPMIN;
		c=b+an/c;
		if (fabs(c) < FPMIN) c=FPMIN;
		d=1.0/d;
		del=d*c;
		h *= del;
		if (fabs(del-1.0) < EPS) break;
	}
	if (i > ITMAX) nrerror("a too large, ITMAX too small in gcf");
	*gammcf=exp(-x+a*log(x)-(*gln))*h;
}

/***************************************************************************** */
/*                          nrerror Functition                                 */
/***************************************************************************** */
void nrerror(char error_text[])
{
	fprintf(stderr,"%s\n",error_text);
	exit(1);
}

/***************************************************************************** */
/*                             erff Functition                                 */
/***************************************************************************** */
double erff(double x)
{
	return x<0.0 ? -gammp(0.5,x*x) : gammp(0.5,x*x);
}

/***************************************************************************** */
/*                            erffc Functition                                 */
/***************************************************************************** */
double erffc(double x)
{
	return x<0.0 ? 1.0+gammp(0.5,x*x) : gammq(0.5,x*x);
}

/***************************************************************************** */
/*                            erfcc Functition                                 */
/***************************************************************************** */
double erfcc(double x)
{
	double t,z,ans;

	z=fabs(x);
	t=1.0/(1.0+0.5*z);
	ans=t*exp(-z*z-1.26551223+t*(1.00002368+t*(0.37409196+t*(0.09678418+t*(-0.18628806+t*(0.27886807+t*(-1.13520398+t*(1.48851587+t*(0.82215223+t*0.17087277)))))))));
	
	return x>=0.0 ? ans : 2.0-ans;
}

/***************************************************************************** */
/*                           normal Functition                                 */
/***************************************************************************** */
double normal(double x)
{
	double arg, result
		;

	if (x > 0)
	{
		arg = x/sqrt2;
		result = 0.5 * ( 1 + erff(arg) );
	}
	else
	{
		arg = -x/sqrt2;
		result = 0.5 * ( 1 - erff(arg) );
	}
	return( result);
}

double normal2(double a)
{
	return (1.0-0.5*erffc(a/sqrt(2.0)));
}
