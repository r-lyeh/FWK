#ifndef DELAUNAY_H
#define DELAUNAY_H

/*
**  delaunay.c : compute 2D delaunay triangulation in the plane.
**  Copyright (C) 2005  Wael El Oraiby <wael.eloraiby@gmail.com>
**
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU Affero General Public License as
**  published by the Free Software Foundation, either version 3 of the
**  License, or (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU Affero General Public License for more details.
**
**  You should have received a copy of the GNU Affero General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/




#ifdef __cplusplus
extern "C" {
#endif

typedef double real;

typedef struct {
	real	x, y;
} del_point2d_t;

typedef struct {
	/** input points count */
	unsigned int	num_points;

	/** the input points */
	del_point2d_t*	points;

	/** number of returned faces */
	unsigned int	num_faces;

	/** the faces are given as a sequence: num verts, verts indices, num verts, verts indices...
	 * the first face is the external face */
	unsigned int*	faces;
} delaunay2d_t;

/*
 * build the 2D Delaunay triangulation given a set of points of at least 3 points
 *
 * @points: point set given as a sequence of tuple x0, y0, x1, y1, ....
 * @num_points: number of given point
 * @preds: the incircle predicate
 * @faces: the triangles given as a sequence: num verts, verts indices, num verts, verts indices.
 *	Note that the first face is the external face
 * @return: the created topology
 */
delaunay2d_t*			delaunay2d_from(del_point2d_t *points, unsigned int num_points);

/*
 * release a delaunay2d object
 */
void				delaunay2d_release(delaunay2d_t* del);


typedef struct {
	/** input points count */
	unsigned int	num_points;

	/** input points */
	del_point2d_t*	points;

	/** number of triangles */
	unsigned int	num_triangles;

	/** the triangles indices v0,v1,v2, v0,v1,v2 .... */
	unsigned int*	tris;
} tri_delaunay2d_t;

/**
 * build a tri_delaunay2d_t out of a delaunay2d_t object
 */
tri_delaunay2d_t*		tri_delaunay2d_from(delaunay2d_t* del);

/**
 * release a tri_delaunay2d_t object
 */
void				tri_delaunay2d_release(tri_delaunay2d_t* tdel);

#ifdef __cplusplus
}
#endif

#endif // DELAUNAY_H

#ifdef DELAUNAY_C

/*
**  delaunay.c : compute 2D delaunay triangulation in the plane.
**  Copyright (C) 2005  Wael El Oraiby <wael.eloraiby@gmail.com>
**
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU Affero General Public License as
**  published by the Free Software Foundation, either version 3 of the
**  License, or (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU Affero General Public License for more details.
**
**  You should have received a copy of the GNU Affero General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>

#define ON_RIGHT	1
#define ON_SEG		0
#define ON_LEFT		-1

#define OUTSIDE		-1
#define	ON_CIRCLE	0
#define INSIDE		1

struct	point2d_s;
struct	face_s;
struct	halfedge_s;
struct	delaunay_s;


#define REAL_ZERO	0.0l
#define REAL_ONE	1.0l
#define REAL_TWO	2.0l
#define REAL_FOUR	4.0l


typedef struct point2d_s	point2d_t;
typedef struct face_s		face_t;
typedef struct halfedge_s	halfedge_t;
typedef struct delaunay_s	delaunay_t;
typedef struct working_set_s	working_set_t;

typedef long double lreal;
typedef lreal mat3_t[3][3];

struct point2d_s {
	real			x, y;			/* point coordinates */
	halfedge_t*		he;			/* point halfedge */
	unsigned int		idx;			/* point index in input buffer */
};

struct face_s {
	halfedge_t*		he;			/* a pointing half edge */
	unsigned int		num_verts;		/* number of vertices on this face */
};

struct halfedge_s {
	point2d_t*		vertex;			/* vertex */
	halfedge_t*		pair;			/* pair */
	halfedge_t*		next;			/* next */
	halfedge_t*		prev;			/* next^-1 */
	face_t*			face;			/* halfedge face */
};

struct delaunay_s {
	halfedge_t*		rightmost_he;		/* right most halfedge */
	halfedge_t*		leftmost_he;		/* left most halfedge */
	point2d_t*		points;			/* pointer to points */
	face_t*			faces;			/* faces of delaunay */
	unsigned int		num_faces;		/* face count */
	unsigned int		start_point;		/* start point index */
	unsigned int		end_point;		/* end point index */
};

struct working_set_s {
	halfedge_t*		edges;			/* all the edges (allocated in one shot) */
	face_t*			faces;			/* all the faces (allocated in one shot) */

	unsigned int		max_edge;		/* maximum edge count: 2 * 3 * n where n is point count */
	unsigned int		max_face;		/* maximum face count: 2 * n where n is point count */

	unsigned int		num_edges;		/* number of allocated edges */
	unsigned int		num_faces;		/* number of allocated faces */

	halfedge_t*		free_edge;		/* pointer to the first free edge */
	face_t*			free_face;		/* pointer to the first free face */
};

/*
* 3x3 matrix determinant
*/
static lreal det3(mat3_t m)
{
	lreal	res	= m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
			- m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
			+ m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);

	return res;
}

/*
* allocate a halfedge
*/
static halfedge_t* halfedge_alloc()
{
	halfedge_t*		d;

	d	= (halfedge_t*)malloc(sizeof(halfedge_t));
	assert( NULL != d );
	memset(d, 0, sizeof(halfedge_t));

	return d;
}

/*
* free a halfedge
*/
static void halfedge_free( halfedge_t* d )
{
	assert( d != NULL );
	memset(d, 0, sizeof(halfedge_t));
	free(d);
}

/*
* free all delaunay halfedges
*/
void del_free_halfedges( delaunay_t *del )
{
	unsigned int		i;
	halfedge_t		*d, *sig;

	/* if there is nothing to do */
	if( del->points == NULL )
		return;

	for( i = 0; i <= (del->end_point - del->start_point); i++ )
	{
		/* free all the halfedges around the point */
		d	= del->points[i].he;
		if( d != NULL )
		{
			do {
				sig	= d->next;
				halfedge_free( d );
				d	= sig;
			} while( d != del->points[i].he );
			del->points[i].he	= NULL;
		}
	}
}

/*
* compare 2 points when sorting
*/
static int cmp_points( const void *_pt0, const void *_pt1 )
{
	point2d_t		*pt0, *pt1;

	pt0	= (point2d_t*)(_pt0);
	pt1	= (point2d_t*)(_pt1);

	if( pt0->x < pt1->x )
		return -1;
	else if( pt0->x > pt1->x )
		return 1;
	else if( pt0->y < pt1->y )
		return -1;
	else if( pt0->y > pt1->y )
		return 1;
	printf("2 or more points share the same exact coordinate: (%f,%f)(%f,%f)\n", pt0->x,pt0->y,pt1->x,pt1->y);
	assert(0 && "2 or more points share the same exact coordinate");
	return 0; /* Should not be given! */
}

/*
* classify a point relative to a segment
*/
static int classify_point_seg( point2d_t *s, point2d_t *e, point2d_t *pt )
{
	lreal		se_x, se_y, spt_x, spt_y;
	lreal		res;

	se_x	= e->x - s->x;
	se_y	= e->y - s->y;

	spt_x	= pt->x - s->x;
	spt_y	= pt->y - s->y;

	res	= (( se_x * spt_y ) - ( se_y * spt_x ));
	if( res < REAL_ZERO )
		return ON_RIGHT;
	else if( res > REAL_ZERO )
		return ON_LEFT;

	return ON_SEG;
}

/*
* classify a point relative to a halfedge, -1 is left, 0 is on, 1 is right
*/
static int del_classify_point( halfedge_t *d, point2d_t *pt )
{
	point2d_t		*s, *e;

	s		= d->vertex;
	e		= d->pair->vertex;

	return classify_point_seg(s, e, pt);
}

/*
* test if a point is inside a circle given by 3 points, 1 if inside, 0 if outside
*/
static int in_circle( point2d_t *pt0, point2d_t *pt1, point2d_t *pt2, point2d_t *p )
{
	// reduce the computational complexity by substracting the last row of the matrix
	// ref: https://www.cs.cmu.edu/~quake/robust.html
	lreal	p0p_x, p0p_y, p1p_x, p1p_y, p2p_x, p2p_y, p0p, p1p, p2p, res;
	mat3_t	m;

	p0p_x	= pt0->x - p->x;
	p0p_y	= pt0->y - p->y;

	p1p_x	= pt1->x - p->x;
	p1p_y	= pt1->y - p->y;

	p2p_x	= pt2->x - p->x;
	p2p_y	= pt2->y - p->y;

	p0p	= p0p_x * p0p_x + p0p_y * p0p_y;
	p1p	= p1p_x * p1p_x + p1p_y * p1p_y;
	p2p	= p2p_x * p2p_x + p2p_y * p2p_y;

	m[0][0]	= p0p_x;
	m[0][1]	= p0p_y;
	m[0][2] = p0p;

	m[1][0]	= p1p_x;
	m[1][1]	= p1p_y;
	m[1][2] = p1p;

	m[2][0]	= p2p_x;
	m[2][1]	= p2p_y;
	m[2][2] = p2p;

	res	= -det3(m);

	if( res < REAL_ZERO )
		return INSIDE;
	else if( res > REAL_ZERO )
		return OUTSIDE;

	return ON_CIRCLE;
}

/*
* initialize delaunay segment
*/
static int del_init_seg( delaunay_t *del, int start )
{
	halfedge_t		*d0, *d1;
	point2d_t		*pt0, *pt1;

	/* init delaunay */
	del->start_point	= start;
	del->end_point		= start + 1;

	/* setup pt0 and pt1 */
	pt0			= &(del->points[start]);
	pt1			= &(del->points[start + 1]);

	/* allocate the halfedges and setup them */
	d0	= halfedge_alloc();
	d1	= halfedge_alloc();

	d0->vertex	= pt0;
	d1->vertex	= pt1;

	d0->next	= d0->prev	= d0;
	d1->next	= d1->prev	= d1;

	d0->pair	= d1;
	d1->pair	= d0;

	pt0->he	= d0;
	pt1->he	= d1;

	del->rightmost_he	= d1;
	del->leftmost_he	= d0;


	return 0;
}

/*
* initialize delaunay triangle
*/
static int del_init_tri( delaunay_t *del, int start )
{
	halfedge_t		*d0, *d1, *d2, *d3, *d4, *d5;
	point2d_t		*pt0, *pt1, *pt2;

	/* initiate delaunay */
	del->start_point	= start;
	del->end_point		= start + 2;

	/* setup the points */
	pt0					= &(del->points[start]);
	pt1					= &(del->points[start + 1]);
	pt2					= &(del->points[start + 2]);

	/* allocate the 6 halfedges */
	d0	= halfedge_alloc();
	d1	= halfedge_alloc();
	d2	= halfedge_alloc();
	d3	= halfedge_alloc();
	d4	= halfedge_alloc();
	d5	= halfedge_alloc();

	if( classify_point_seg(pt0, pt2, pt1) == ON_LEFT )	/* first case */
	{
		/* set halfedges points */
		d0->vertex	= pt0;
		d1->vertex	= pt2;
		d2->vertex	= pt1;

		d3->vertex	= pt2;
		d4->vertex	= pt1;
		d5->vertex	= pt0;

		/* set points halfedges */
		pt0->he	= d0;
		pt1->he	= d2;
		pt2->he	= d1;

		/* next and next -1 setup */
		d0->next	= d5;
		d0->prev	= d5;

		d1->next	= d3;
		d1->prev	= d3;

		d2->next	= d4;
		d2->prev	= d4;

		d3->next	= d1;
		d3->prev	= d1;

		d4->next	= d2;
		d4->prev	= d2;

		d5->next	= d0;
		d5->prev	= d0;

		/* set halfedges pair */
		d0->pair	= d3;
		d3->pair	= d0;

		d1->pair	= d4;
		d4->pair	= d1;

		d2->pair	= d5;
		d5->pair	= d2;

		del->rightmost_he	= d1;
		del->leftmost_he		= d0;

	} else /* 2nd case */
	{
		/* set halfedges points */
		d0->vertex	= pt0;
		d1->vertex	= pt1;
		d2->vertex	= pt2;

		d3->vertex	= pt1;
		d4->vertex	= pt2;
		d5->vertex	= pt0;

		/* set points halfedges */
		pt0->he	= d0;
		pt1->he	= d1;
		pt2->he	= d2;

		/* next and next -1 setup */
		d0->next	= d5;
		d0->prev	= d5;

		d1->next	= d3;
		d1->prev	= d3;

		d2->next	= d4;
		d2->prev	= d4;

		d3->next	= d1;
		d3->prev	= d1;

		d4->next	= d2;
		d4->prev	= d2;

		d5->next	= d0;
		d5->prev	= d0;

		/* set halfedges pair */
		d0->pair	= d3;
		d3->pair	= d0;

		d1->pair	= d4;
		d4->pair	= d1;

		d2->pair	= d5;
		d5->pair	= d2;

		del->rightmost_he	= d2;
		del->leftmost_he		= d0;
	}

	return 0;
}

/*
* remove an edge given a halfedge
*/
static void del_remove_edge( halfedge_t *d )
{
	halfedge_t	*next, *prev, *pair, *orig_pair;

	orig_pair	= d->pair;

	next	= d->next;
	prev	= d->prev;
	pair	= d->pair;

	assert(next != NULL);
	assert(prev != NULL);

	next->prev	= prev;
	prev->next	= next;


	/* check to see if we have already removed pair */
	if( pair )
		pair->pair	= NULL;

	/* check to see if the vertex points to this halfedge */
	if( d->vertex->he == d )
		d->vertex->he	= next;

	d->vertex	= NULL;
	d->next		= NULL;
	d->prev		= NULL;
	d->pair		= NULL;

	next	= orig_pair->next;
	prev	= orig_pair->prev;
	pair	= orig_pair->pair;

	assert(next != NULL);
	assert(prev != NULL);

	next->prev	= prev;
	prev->next	= next;


	/* check to see if we have already removed pair */
	if( pair )
		pair->pair	= NULL;

	/* check to see if the vertex points to this halfedge */
	if( orig_pair->vertex->he == orig_pair )
		orig_pair->vertex->he	= next;

	orig_pair->vertex	= NULL;
	orig_pair->next		= NULL;
	orig_pair->prev		= NULL;
	orig_pair->pair		= NULL;


	/* finally free the halfedges */
	halfedge_free(d);
	halfedge_free(orig_pair);
}

/*
* pass through all the halfedges on the left side and validate them
*/
static halfedge_t* del_valid_left( halfedge_t* b )
{
	point2d_t		*g, *d, *u, *v;
	halfedge_t		*c, *du, *dg;

	g	= b->vertex;				/* base halfedge point */
	dg	= b;

	d	= b->pair->vertex;			/* pair(halfedge) point */
	b	= b->next;

	u	= b->pair->vertex;			/* next(pair(halfedge)) point */
	du	= b->pair;

	v	= b->next->pair->vertex;	/* pair(next(next(halfedge)) point */

	if( classify_point_seg(g, d, u) == ON_LEFT )
	{
		/* 3 points aren't colinear */
		/* as long as the 4 points belong to the same circle, do the cleaning */
		assert( v != u && "1: floating point precision error");
		while( v != d && v != g && in_circle(g, d, u, v) == INSIDE )
		{
			c	= b->next;
			du	= b->next->pair;
			del_remove_edge(b);
			b	= c;
			u	= du->vertex;
			v	= b->next->pair->vertex;
		}

		assert( v != u && "2: floating point precision error");
		if( v != d && v != g && in_circle(g, d, u, v) == ON_CIRCLE )
		{
			du	= du->prev;
			del_remove_edge(b);
		}
	} else	/* treat the case where the 3 points are colinear */
		du		= dg;

	assert(du->pair);
	return du;
}

/*
* pass through all the halfedges on the right side and validate them
*/
static halfedge_t* del_valid_right( halfedge_t *b )
{
	point2d_t		*rv, *lv, *u, *v;
	halfedge_t		*c, *dd, *du;

	b	= b->pair;
	rv	= b->vertex;
	dd	= b;
	lv	= b->pair->vertex;
	b	= b->prev;
	u	= b->pair->vertex;
	du	= b->pair;

	v	= b->prev->pair->vertex;

	if( classify_point_seg(lv, rv, u) == ON_LEFT )
	{
		assert( v != u && "1: floating point precision error");
		while( v != lv && v != rv && in_circle(lv, rv, u, v) == INSIDE )
		{
			c	= b->prev;
			du	= c->pair;
			del_remove_edge(b);
			b	= c;
			u	= du->vertex;
			v	= b->prev->pair->vertex;
		}

		assert( v != u && "1: floating point precision error");
		if( v != lv && v != rv && in_circle(lv, rv, u, v) == ON_CIRCLE )
		{
			du	= du->next;
			del_remove_edge(b);
		}
	} else
		du	= dd;

	assert(du->pair);
	return du;
}


/*
* validate a link
*/
static halfedge_t* del_valid_link( halfedge_t *b )
{
	point2d_t	*g, *g_p, *d, *d_p;
	halfedge_t	*gd, *dd, *new_gd, *new_dd;
	int		a;

	g	= b->vertex;
	gd	= del_valid_left(b);
	g_p	= gd->vertex;

	assert(b->pair);
	d	= b->pair->vertex;
	dd	= del_valid_right(b);
	d_p	= dd->vertex;
	assert(b->pair);

	if( g != g_p && d != d_p ) {
		a	= in_circle(g, d, g_p, d_p);

		if( a != ON_CIRCLE ) {
			if( a == INSIDE ) {
				g_p	= g;
				gd	= b;
			} else {
				d_p = d;
				dd	= b->pair;
			}
		}
	}

	/* create the 2 halfedges */
	new_gd	= halfedge_alloc();
	new_dd	= halfedge_alloc();

	/* setup new_gd and new_dd */

	new_gd->vertex	= gd->vertex;
	new_gd->pair	= new_dd;
	new_gd->prev	= gd;
	new_gd->next	= gd->next;
	gd->next->prev	= new_gd;
	gd->next		= new_gd;

	new_dd->vertex	= dd->vertex;
	new_dd->pair	= new_gd;
	new_dd->prev	= dd->prev;
	dd->prev->next	= new_dd;
	new_dd->next	= dd;
	dd->prev		= new_dd;

	return new_gd;
}

/*
* find the lower tangent between the two delaunay, going from left to right (returns the left half edge)
*/
static halfedge_t* del_get_lower_tangent( delaunay_t *left, delaunay_t *right )
{
	point2d_t	*pl, *pr;
	halfedge_t	*right_d, *left_d, *new_ld, *new_rd;
	int		sl, sr;

	left_d	= left->rightmost_he;
	right_d	= right->leftmost_he;

	do {
		pl		= left_d->prev->pair->vertex;
		pr		= right_d->pair->vertex;

		if( (sl = classify_point_seg(left_d->vertex, right_d->vertex, pl)) == ON_RIGHT ) {
			left_d	= left_d->prev->pair;
		}

		if( (sr = classify_point_seg(left_d->vertex, right_d->vertex, pr)) == ON_RIGHT ) {
			right_d	= right_d->pair->next;
		}

	} while( sl == ON_RIGHT || sr == ON_RIGHT );

	/* create the 2 halfedges */
	new_ld	= halfedge_alloc();
	new_rd	= halfedge_alloc();

	/* setup new_gd and new_dd */
	new_ld->vertex	= left_d->vertex;
	new_ld->pair	= new_rd;
	new_ld->prev	= left_d->prev;
	left_d->prev->next	= new_ld;
	new_ld->next	= left_d;
	left_d->prev	= new_ld;

	new_rd->vertex	= right_d->vertex;
	new_rd->pair	= new_ld;
	new_rd->prev	= right_d->prev;
	right_d->prev->next	= new_rd;
	new_rd->next	= right_d;
	right_d->prev	= new_rd;

	return new_ld;
}

/*
* link the 2 delaunay together
*/
static void del_link( delaunay_t *result, delaunay_t *left, delaunay_t *right )
{
	point2d_t		*u, *v, *ml, *mr;
	halfedge_t		*base;

	assert( left->points == right->points );

	/* save the most right point and the most left point */
	ml		= left->leftmost_he->vertex;
	mr		= right->rightmost_he->vertex;

	base		= del_get_lower_tangent(left, right);

	u		= base->next->pair->vertex;
	v		= base->pair->prev->pair->vertex;

	while( del_classify_point(base, u) == ON_LEFT ||
		   del_classify_point(base, v) == ON_LEFT )
	{
		base	= del_valid_link(base);
		u	= base->next->pair->vertex;
		v	= base->pair->prev->pair->vertex;
	}

	right->rightmost_he	= mr->he;
	left->leftmost_he	= ml->he;

	/* TODO: this part is not needed, and can be optimized */
	while( del_classify_point( right->rightmost_he, right->rightmost_he->prev->pair->vertex ) == ON_RIGHT )
		   right->rightmost_he	= right->rightmost_he->prev;

	while( del_classify_point( left->leftmost_he, left->leftmost_he->prev->pair->vertex ) == ON_RIGHT )
		   left->leftmost_he	= left->leftmost_he->prev;

	result->leftmost_he		= left->leftmost_he;
	result->rightmost_he		= right->rightmost_he;
	result->points			= left->points;
	result->start_point		= left->start_point;
	result->end_point		= right->end_point;
}

/*
* divide and conquer delaunay
*/
void del_divide_and_conquer( delaunay_t *del, int start, int end )
{
	delaunay_t	left, right;
	int			i, n;

	n		= (end - start + 1);

	if( n > 3 ) {
		i		= (n / 2) + (n & 1);
		left.points		= del->points;
		right.points	= del->points;
		del_divide_and_conquer( &left, start, start + i - 1 );
		del_divide_and_conquer( &right, start + i, end );
		del_link( del, &left, &right );
	} else {
		if( n == 3 ) {
			del_init_tri( del, start );
		} else {
			if( n == 2 ) {
				del_init_seg( del, start );
			}
		}
	}
}

static void build_halfedge_face( delaunay_t *del, halfedge_t *d )
{
	halfedge_t	*curr;

	/* test if the halfedge has already a pointing face */
	if( d->face != NULL )
		return;

	/* TODO: optimize this */
	del->faces = (face_t*)realloc(del->faces, (del->num_faces + 1) * sizeof(face_t));
	assert( NULL != del->faces );

	face_t	*f	= &(del->faces[del->num_faces]);
	curr	= d;
	f->he	= d;
	f->num_verts	= 0;
	do {
		curr->face	= f;
		(f->num_verts)++;
		curr	= curr->pair->prev;
	} while( curr != d );

	(del->num_faces)++;
}

/*
* build the faces for all the halfedge
*/
void del_build_faces( delaunay_t *del )
{
	unsigned int	i;
	halfedge_t	*curr;

	del->num_faces	= 0;
	del->faces		= NULL;

	/* build external face first */
	build_halfedge_face(del, del->rightmost_he->pair);

	for( i = del->start_point; i <= del->end_point; i++ )
	{
		curr	= del->points[i].he;

		do {
			build_halfedge_face( del, curr );
			curr	= curr->next;
		} while( curr != del->points[i].he );
	}
}

/*
*/
delaunay2d_t* delaunay2d_from(del_point2d_t *points, unsigned int num_points) {
	delaunay2d_t*	res	= NULL;
	delaunay_t	del;
	unsigned int	i, j, fbuff_size = 0;
	unsigned int*	faces	= NULL;

	/* allocate the points */
	del.points	= (point2d_t*)malloc(num_points * sizeof(point2d_t));
	assert( NULL != del.points );
	memset(del.points, 0, num_points * sizeof(point2d_t));

	/* copy the points */
	for( i = 0; i < num_points; i++ )
	{
		del.points[i].idx	= i;
		del.points[i].x	= points[i].x;
		del.points[i].y	= points[i].y;
	}

	qsort(del.points, num_points, sizeof(point2d_t), cmp_points);

	if( num_points >= 3 ) {
		del_divide_and_conquer( &del, 0, num_points - 1 );

		del_build_faces( &del );

		fbuff_size	= 0;
		for( i = 0; i < del.num_faces; i++ )
			fbuff_size	+= del.faces[i].num_verts + 1;

		faces = (unsigned int*)malloc(sizeof(unsigned int) * fbuff_size);
		assert( NULL != faces );

		j = 0;
		for( i = 0; i < del.num_faces; i++ )
		{
			halfedge_t	*curr;

			faces[j]	= del.faces[i].num_verts;
			j++;

			curr	= del.faces[i].he;
			do {
				faces[j]	= curr->vertex->idx;
				j++;
				curr	= curr->pair->prev;
			} while( curr != del.faces[i].he );
		}

		del_free_halfedges( &del );

		free(del.faces);
		free(del.points);
	}

	res		= (delaunay2d_t*)malloc(sizeof(delaunay2d_t));
	assert( NULL != res );
	res->num_points	= num_points;
	res->points	= (del_point2d_t*)malloc(sizeof(del_point2d_t) * num_points);
	assert( NULL != res->points );
	memcpy(res->points, points, sizeof(del_point2d_t) * num_points);
	res->num_faces	= del.num_faces;
	res->faces	= faces;

	return res;
}

void delaunay2d_release(delaunay2d_t *del) {
	free(del->faces);
	free(del->points);
	free(del);
}


tri_delaunay2d_t* tri_delaunay2d_from(delaunay2d_t* del) {
	unsigned int		v_offset	= del->faces[0] + 1;	/* ignore external face */
	unsigned int		dst_offset	= 0;
	unsigned int		i;

	tri_delaunay2d_t*	tdel = (tri_delaunay2d_t*)malloc(sizeof(tri_delaunay2d_t));
	assert( NULL != tdel );
	tdel->num_triangles	= 0;

	/* count the number of triangles */
	if( 1 == del->num_faces ) { /* degenerate case: only external face exists */
		unsigned int	nv	= del->faces[0];
		tdel->num_triangles	+= nv - 2;
	} else {
		for( i = 1; i < del->num_faces; ++i ) {
			unsigned int	nv	= del->faces[v_offset];
			tdel->num_triangles	+= nv - 2;
			v_offset		+= nv + 1;
		}
	}

	/* copy points */
	tdel->num_points	= del->num_points;
	tdel->points		= (del_point2d_t*)malloc(sizeof(del_point2d_t) * del->num_points);
	assert( NULL != tdel->points );
	memcpy(tdel->points, del->points, sizeof(del_point2d_t) * del->num_points);

	/* build the triangles */
	tdel->tris		= (unsigned int*)malloc(sizeof(unsigned int) * 3 * tdel->num_triangles);
	assert( NULL != tdel->tris );

	v_offset	= del->faces[0] + 1;	/* ignore external face */

	if( 1 == del->num_faces ) {
		/* handle the degenerated case where only the external face exists */
		unsigned int	nv	= del->faces[0];
		unsigned int	j	= 0;
		v_offset	= 1;
		for( ; j < nv - 2; ++j ) {
			tdel->tris[dst_offset]		= del->faces[v_offset + j];
			tdel->tris[dst_offset + 1]	= del->faces[(v_offset + j + 1) % nv];
			tdel->tris[dst_offset + 2]	= del->faces[v_offset + j];
			dst_offset	+= 3;
		}
	} else {
		for( i = 1; i < del->num_faces; ++i ) {
			unsigned int	nv	= del->faces[v_offset];
			unsigned int	j	= 0;
			unsigned int	first	= del->faces[v_offset + 1];


			for( ; j < nv - 2; ++j ) {
				tdel->tris[dst_offset]		= first;
				tdel->tris[dst_offset + 1]	= del->faces[v_offset + j + 2];
				tdel->tris[dst_offset + 2]	= del->faces[v_offset + j + 3];
				dst_offset	+= 3;
			}

			v_offset		+= nv + 1;
		}
	}

	return tdel;
}


void tri_delaunay2d_release(tri_delaunay2d_t* tdel) {
	free(tdel->tris);
	free(tdel->points);
	free(tdel);
}

#endif
