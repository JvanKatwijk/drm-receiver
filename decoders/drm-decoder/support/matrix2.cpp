//
//	source:
//	http://bbs.dartmouth.edu/~fangq/MATH/download/source/inverse_c.htm
// inverse.c
// Source Code for "GRPP, A Scientific Programming Language 
// Processor Designed for Lex and Yacc."
// Author: James Kent Blackburn
// Goddard Space Flight Center, Code 664.0, Greenbelt, MD. 20771
// Computers in Physics, Journal Section, Jan/Feb 1994

/* 
   Matrix Inversion using 
   LU Decomposition from
   Numerical Recipes in C
   Chapter 2
*/

#include	"matrix2.h"
#define   TINY 1.0e-20

void	inverse		(JAN**, int);
void	ludcmp		(JAN**, int, int*, JAN*);
void	lubksb		(JAN**, int, int*, JAN*);
JAN	**matrix	(int, int, int, int);
JAN	*vector		(int, int);
void	free_matrix	(JAN**, int, int, int, int);
void	free_vector	(JAN*, int, int);

void inverse (JAN **mat, int dim) {
int i, j, *indx;
JAN **y, d, *col;

	y = new JAN *[dim];
	for (i = 0; i < dim; i ++)
	   y [i] = new JAN [dim];
	indx = new int [dim];
	col = new JAN [dim];
	ludcmp (mat, dim, indx, &d);
	for (j = 0; j < dim; j++) {
	   for (i = 0; i < dim; i++)
	      col [i] = 0.0;
	   col [j] = 1.0;
	   lubksb (mat, dim, indx, col);
	   for (i = 0; i < dim; i++)
	      y [i][j] = col[i];
	}

	for (i = 0; i < dim; i++)
	   for (j = 0; j < dim; j++)
	      mat [i][j] = y[i][j];
	delete [] indx;
	delete [] col;
	for (i = 0; i < dim; i ++)
	   delete [] y [i];
	delete [] y;
}

void	ludcmp (JAN **a, int n, int *indx, JAN *d) {
int i, imax = 0, j, k;
JAN   big,dum,sum,temp;
JAN   *vv;

	vv = new JAN [n];
	*d = 1.0;
	for (i = 0; i < n; i++) {
	   big = 0.0;
	   for (j = 0; j < n; j++) {
	      if ((temp = fabs(a[i][j])) > big)
	         big = temp;
	   }

	   if (big == 0.0) {
	      fprintf(stderr,"Singular Matrix in Routine LUDCMP\n");
	      for (j = 0; j < n; j++)
	         printf(" %f ",a[i][j]);
	      printf("\n");
	      exit(1);
	   }

	   vv [i] = 1.0 / big;
	}

	for (j = 0; j < n; j++) {
	   for (i = 0; i < j; i++) {
	      sum = a[i][j];
	      for (k = 0; k < i; k++)
	         sum -= a[i][k] * a[k][j];
	      a [i][j] = sum;
	   }
	   big = 0.0;
	   for (i = j; i < n; i++) {
	      sum = a[i][j];
	      for (k = 0; k < j; k++)
	         sum -= a[i][k] * a[k][j];
	      a [i][j] = sum;
	      if ((dum = vv[i] * fabs(sum)) >= big) {
	         big = dum;
	         imax = i;
	      }
	   }
	   if (j != imax) {
	      for (k = 0; k < n; k++) {
	         dum = a [imax][k];
	         a [imax][k] = a[j][k];
	         a [j][k] = dum;
	      }

	      *d = -(*d);
	      vv [imax] = vv[j];
	   }
	   indx[j] = imax;
	   if (a[j][j] == 0.0)
	      a[j][j] = TINY;
	   if (j != n-1) {
	      dum = 1.0 / a[j][j];
	      for (i = j + 1; i < n; i++)
	         a[i][j] *= dum;
	   }
	}
	delete[] vv;
}

void lubksb (JAN **a, int n, int *indx, JAN *b) {
int i,ip,j,ii=-1;
JAN   sum;

	for (i = 0; i < n; i++) {
	   ip = indx [i];
	   sum = b [ip];
	   b [ip] = b [i];
	   if (ii >= 0)
	      for (j = ii; j < i; j++)
	         sum -= a[i][j] * b[j];
	   else
	   if (sum)
	      ii = i;
	   b[i] = sum;
	}
	for (i = n - 1; i >= 0; i--) {
	   sum = b[i];
	   for (j = i+1; j < n; j++)
	      sum -= a[i][j] * b[j];
	   b [i] = sum / a [i][i];
	}
}

