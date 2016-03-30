/* This file contains utilities for the Numerical Recipes in C algorithms. */

#include <malloc.h>
#include <stdio.h>

void nrerror (char error_text[])
{
	void exit();

	fprintf (stderr,"Numerical Recipes run-time error...\n");
	fprintf (stderr,"%s\n",error_text);
	fprintf (stderr,"...now exiting to system...\n");
	exit (1);
}

float *vector (int nl, int nh)
{
	float *v;

	v=(float *) malloc ((unsigned) (nh+1)*sizeof (float));
	if (!v) nrerror ("allocation failure in vector()");
	return v;
}

int *ivector (int nl, int nh)
{
	int *v;

	v=(int *)malloc ((unsigned) (nh+1)*sizeof (int));
	if (!v) nrerror ("allocation failure in ivector()");
	return v;
}

float **matrix (int nrl, int nrh, int ncl, int nch)
{
	int i;
	float **m;

	m=(float **) malloc ((unsigned) (nrh+1)*sizeof (float*));
	if (!m) nrerror ("allocation failure 1 in matrix()");

	for (i = nrl; i <= nrh; i++) 
	{
		m[i]=(float *) malloc ((unsigned) (nch+1)*sizeof (float));
		if (!m[i]) nrerror ("allocation failure 2 in matrix()");
	}
	return m;
}

void free_vector (float *v, int nl, int nh)
{
	free ((char*) (v));
}

void free_ivector (int *v, int nl, int nh)
{
	free ((char*) (v));
}

void free_matrix (float **m, int nrl, int nrh, int ncl, int nch)
{
	int i;

	for (i = nrh; i >= nrl; i--) 
		free ((char*) (m[i]));
	free ((char*) (m));
}
