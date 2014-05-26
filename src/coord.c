/*
  Copyright Simon Mitternacht 2013-2014.

  This file is part of FreeSASA.
  
  FreeSASA is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  FreeSASA is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with FreeSASA.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "freesasa.h"
#include "coord.h"

struct freesasa_coord_ {
    double *xyz;
    size_t n;
    int is_const;
};

freesasa_coord_t* freesasa_coord_new() 
{
    freesasa_coord_t* c = (freesasa_coord_t*) malloc(sizeof(freesasa_coord_t));
    c->xyz = NULL;
    c->n = 0;
    c->is_const = 0;
    return c;
}

void freesasa_coord_free(freesasa_coord_t *c) 
{
    assert(c != NULL && "NULL-pointer passed to freesasa_coord_free(1)");
    if (c->xyz && !c->is_const) free(c->xyz);
    free(c);
}
static int coord_clear(freesasa_coord_t *c) 
{
    if (c->is_const || c == NULL) return FREESASA_FAIL;
    if (c->xyz != NULL) {
	free(c->xyz);
	c->xyz = NULL;
    }
    c->n = 0;
    return FREESASA_SUCCESS;
}


freesasa_coord_t* freesasa_coord_copy(const freesasa_coord_t *src) 
{
    assert(src != NULL);
    freesasa_coord_t *c = freesasa_coord_new();
    freesasa_coord_set_all(c,src->xyz,src->n);
    return c;
}

freesasa_coord_t* freesasa_coord_new_linked(const double *xyz, size_t n)
{
    assert(xyz != NULL);
    freesasa_coord_t *c = freesasa_coord_new();
    c->xyz = (double*)xyz; 
    c->n = n;
    c->is_const = 1;
    return c;
}

void freesasa_coord_append(freesasa_coord_t *c, const double *xyz, size_t n)
{
    assert(c   != NULL);
    assert(xyz != NULL);
    assert(!c->is_const);

    size_t n_old = c->n;
    c->n += n;
    c->xyz = (double*) realloc(c->xyz, sizeof(double)*3*c->n);

    double *dest = memcpy(&(c->xyz[3*n_old]), xyz, sizeof(double)*n*3);

    assert(dest != NULL);
}

void freesasa_coord_append_xyz(freesasa_coord_t *c, 
			      const double *x, const double *y, 
			      const double *z, size_t n)
{
    assert(c != NULL);
    assert(x != NULL);
    assert(y != NULL);
    assert(z != NULL);
    assert(!c->is_const);
    
    double *xyz = (double*)malloc(sizeof(double)*n*3);
    for (int i = 0; i < n; ++i) {
        xyz[i*3] = x[i];
        xyz[i*3+1] = y[i];
        xyz[i*3+2] = z[i];
    }
    freesasa_coord_append(c,xyz,n);
    free(xyz);
}

void freesasa_coord_set_i(freesasa_coord_t *c, int i, const double* xyz) 
{
    assert(c   != NULL);
    assert(xyz != NULL);
    assert(c->n > i);
    assert(i >= 0);
    assert(!c->is_const);
    
    memcpy(&c->xyz[i*3], xyz, 3*sizeof(double));
}

void freesasa_coord_set_i_xyz(freesasa_coord_t *c,int i,
			     double x,double y,double z)
{
    assert(c != NULL);
    assert(c->n > i);
    assert(i >= 0);
    assert(!c->is_const);

    double *v_i = &c->xyz[i*3];
    *(v_i++) = x;
    *(v_i++) = y;
    *v_i = z;
}

void freesasa_coord_set_all(freesasa_coord_t *c, const double* xyz, size_t n) 
{
    assert(coord_clear(c) == FREESASA_SUCCESS);
    freesasa_coord_append(c,xyz,n);    
}

void freesasa_coord_set_all_xyz(freesasa_coord_t *c,
			       const double* x, const double *y,
			       const double *z, size_t n)
{
    assert(coord_clear(c) == FREESASA_SUCCESS);
    freesasa_coord_append_xyz(c, x, y, z, n);
}

void freesasa_coord_set_length_i(freesasa_coord_t *c, int i, double l)
{
    assert(c != NULL);
    assert(c->xyz != NULL);
    assert(!c->is_const);
    
    double x = c->xyz[3*i], y = c->xyz[3*i+1], z = c->xyz[3*i+2];
    double r = sqrt(x*x + y*y + z*z);
    c->xyz[3*i]   *= l/r;
    c->xyz[3*i+1] *= l/r;
    c->xyz[3*i+2] *= l/r;
}

void freesasa_coord_set_length_all(freesasa_coord_t *c, double l)
{
    assert(c != NULL);
    assert(!c->is_const);
    for (int i = 0; i < c->n; ++i) freesasa_coord_set_length_i(c,i,l);
}

const double* freesasa_coord_i(const freesasa_coord_t *c, int i)
{
    assert(c != NULL);
    assert(i < c->n);
    assert(i >= 0);
    return &c->xyz[3*i];
}

double freesasa_coord_dist(const freesasa_coord_t *c, int i, int j)
{
    return sqrt(freesasa_coord_dist2(c,i,j));
}

static inline double dist2(const double *v1, const double *v2)
{
    double dx = v1[0]-v2[0], dy = v1[1]-v2[1], dz = v1[2]-v2[2];
    return dx*dx + dy*dy + dz*dz;    
}

double freesasa_coord_dist2(const freesasa_coord_t *c, int i, int j)
{
    double *v1 = &c->xyz[3*i];
    double *v2 = &c->xyz[3*j];
    return dist2(v1,v2);
}

double freesasa_coord_dist2_12(const freesasa_coord_t* c1, 
			      const freesasa_coord_t* c2, int i1, int i2)
{
    double *v1 = &c1->xyz[3*i1];
    double *v2 = &c2->xyz[3*i2];
    return dist2(v1,v2);
}

const double* freesasa_coord_all(const freesasa_coord_t *c)
{
    assert(c != NULL);
    return c->xyz;
}

size_t freesasa_coord_n(const freesasa_coord_t* c)
{
    assert(c != NULL);
    return c->n;
}

void freesasa_coord_translate(freesasa_coord_t *c, const double *xyz)
{
    assert(!c->is_const);
    assert(xyz != NULL);
    freesasa_coord_translate_xyz(c,xyz[0],xyz[1],xyz[2]);
}

void freesasa_coord_translate_xyz(freesasa_coord_t *c, 
				 double x, double y, double z)
{
    assert(c != NULL);
    assert(!c->is_const);
    
    for (int i = 0; i < c->n; ++i) {
        c->xyz[3*i]   += x; 
        c->xyz[3*i+1] += y; 
        c->xyz[3*i+2] += z; 
    }
}

void freesasa_coord_scale(freesasa_coord_t *c, double s)
{
    assert(c != NULL);
    assert(!c->is_const);
    for (int i = 0; i < c->n*3; ++i) {
        c->xyz[i] *= s;
    }
}

