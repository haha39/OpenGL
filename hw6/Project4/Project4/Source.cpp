/***************************
4108056041 陳璽文 第6-VI次作業12/22
***************************/

#include <Windows.h>
#include <GL\glew.h>
#include <GL\freeglut.h>
#include <iostream>

using namespace std;

/*
	  glm.c
	  Nate Robins, 1997
	  ndr@pobox.com, http://www.pobox.com/~ndr/

	  Wavefront OBJ model file format reader/writer/manipulator.

	  Includes routines for generating smooth normals with
	  preservation of edges, welding redundant vertices & texture
	  coordinate generation (spheremap and planar projections) + more.

 */


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "glm.h"


#pragma warning(disable:4996)
#define T(x) (model->triangles[(x)])


 /* _GLMnode: general purpose node
  */
typedef struct _GLMnode {
	GLuint           index;
	GLboolean        averaged;
	struct _GLMnode* next;
} GLMnode;


/* glmMax: returns the maximum of two floats */
static GLfloat
glmMax(GLfloat a, GLfloat b)
{
	if (b > a)
		return b;
	return a;
}

/* glmAbs: returns the absolute value of a float */
static GLfloat
glmAbs(GLfloat f)
{
	if (f < 0)
		return -f;
	return f;
}

/* glmDot: compute the dot product of two vectors
 *
 * u - array of 3 GLfloats (GLfloat u[3])
 * v - array of 3 GLfloats (GLfloat v[3])
 */
static GLfloat
glmDot(GLfloat* u, GLfloat* v)
{
	assert(u); assert(v);

	return u[0] * v[0] + u[1] * v[1] + u[2] * v[2];
}

/* glmCross: compute the cross product of two vectors
 *
 * u - array of 3 GLfloats (GLfloat u[3])
 * v - array of 3 GLfloats (GLfloat v[3])
 * n - array of 3 GLfloats (GLfloat n[3]) to return the cross product in
 */
static GLvoid
glmCross(GLfloat* u, GLfloat* v, GLfloat* n)
{
	assert(u); assert(v); assert(n);

	n[0] = u[1] * v[2] - u[2] * v[1];
	n[1] = u[2] * v[0] - u[0] * v[2];
	n[2] = u[0] * v[1] - u[1] * v[0];
}

/* glmNormalize: normalize a vector
 *
 * v - array of 3 GLfloats (GLfloat v[3]) to be normalized
 */
static GLvoid
glmNormalize(GLfloat* v)
{
	GLfloat l;

	assert(v);

	l = (GLfloat)sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	v[0] /= l;
	v[1] /= l;
	v[2] /= l;
}

/* glmEqual: compares two vectors and returns GL_TRUE if they are
 * equal (within a certain threshold) or GL_FALSE if not. An epsilon
 * that works fairly well is 0.000001.
 *
 * u - array of 3 GLfloats (GLfloat u[3])
 * v - array of 3 GLfloats (GLfloat v[3])
 */
static GLboolean
glmEqual(GLfloat* u, GLfloat* v, GLfloat epsilon)
{
	if (glmAbs(u[0] - v[0]) < epsilon &&
		glmAbs(u[1] - v[1]) < epsilon &&
		glmAbs(u[2] - v[2]) < epsilon)
	{
		return GL_TRUE;
	}
	return GL_FALSE;
}

/* glmWeldVectors: eliminate (weld) vectors that are within an
 * epsilon of each other.
 *
 * vectors    - array of GLfloat[3]'s to be welded
 * numvectors - number of GLfloat[3]'s in vectors
 * epsilon    - maximum difference between vectors
 *
 */
GLfloat*
glmWeldVectors(GLfloat* vectors, GLuint* numvectors, GLfloat epsilon)
{
	GLfloat* copies;
	GLuint   copied;
	GLuint   i, j;

	copies = (GLfloat*)malloc(sizeof(GLfloat) * 3 * (*numvectors + 1));
	memcpy(copies, vectors, (sizeof(GLfloat) * 3 * (*numvectors + 1)));

	copied = 1;
	for (i = 1; i <= *numvectors; i++) {
		for (j = 1; j <= copied; j++) {
			if (glmEqual(&vectors[3 * i], &copies[3 * j], epsilon)) {
				goto duplicate;
			}
		}

		/* must not be any duplicates -- add to the copies array */
		copies[3 * copied + 0] = vectors[3 * i + 0];
		copies[3 * copied + 1] = vectors[3 * i + 1];
		copies[3 * copied + 2] = vectors[3 * i + 2];
		j = copied;				/* pass this along for below */
		copied++;

	duplicate:
		/* set the first component of this vector to point at the correct
		   index into the new copies array */
		vectors[3 * i + 0] = (GLfloat)j;
	}

	*numvectors = copied - 1;
	return copies;
}

/* glmFindGroup: Find a group in the model
 */
GLMgroup*
glmFindGroup(GLMmodel* model, char* name)
{
	GLMgroup* group;

	assert(model);

	group = model->groups;
	while (group) {
		if (!strcmp(name, group->name))
			break;
		group = group->next;
	}

	return group;
}

/* glmAddGroup: Add a group to the model
 */
GLMgroup*
glmAddGroup(GLMmodel* model, char* name)
{
	GLMgroup* group;

	group = glmFindGroup(model, name);
	if (!group) {
		group = (GLMgroup*)malloc(sizeof(GLMgroup));
		group->name = strdup(name);
		group->material = 0;
		group->numtriangles = 0;
		group->triangles = NULL;
		group->next = model->groups;
		model->groups = group;
		model->numgroups++;
	}

	return group;
}

/* glmFindGroup: Find a material in the model
 */
GLuint
glmFindMaterial(GLMmodel* model, char* name)
{
	GLuint i;

	/* XXX doing a linear search on a string key'd list is pretty lame,
	   but it works and is fast enough for now. */
	for (i = 0; i < model->nummaterials; i++) {
		if (!strcmp(model->materials[i].name, name))
			goto found;
	}

	/* didn't find the name, so print a warning and return the default
	   material (0). */
	printf("glmFindMaterial():  can't find material \"%s\".\n", name);
	i = 0;

found:
	return i;
}


/* glmDirName: return the directory given a path
 *
 * path - filesystem path
 *
 * NOTE: the return value should be free'd.
 */
static char*
glmDirName(char* path)
{
	char* dir;
	char* s;

	dir = strdup(path);

	s = strrchr(dir, '/');
	if (s)
		s[1] = '\0';
	else
		dir[0] = '\0';

	return dir;
}


/* glmReadMTL: read a wavefront material library file
 *
 * model - properly initialized GLMmodel structure
 * name  - name of the material library
 */
static GLvoid
glmReadMTL(GLMmodel* model, char* name)
{
	FILE* file;
	char* dir;
	char* filename;
	char  buf[128];
	GLuint nummaterials, i;

	dir = glmDirName(model->pathname);
	filename = (char*)malloc(sizeof(char) * (strlen(dir) + strlen(name) + 1));
	strcpy(filename, dir);
	strcat(filename, name);
	free(dir);

	file = fopen(filename, "r");
	if (!file) {
		fprintf(stderr, "glmReadMTL() failed: can't open material file \"%s\".\n",
			filename);
		exit(1);
	}
	free(filename);

	/* count the number of materials in the file */
	nummaterials = 1;
	while (fscanf(file, "%s", buf) != EOF) {
		switch (buf[0]) {
		case '#':				/* comment */
		  /* eat up rest of line */
			fgets(buf, sizeof(buf), file);
			break;
		case 'n':				/* newmtl */
			fgets(buf, sizeof(buf), file);
			nummaterials++;
			sscanf(buf, "%s %s", buf, buf);
			break;
		default:
			/* eat up rest of line */
			fgets(buf, sizeof(buf), file);
			break;
		}
	}

	rewind(file);

	model->materials = (GLMmaterial*)malloc(sizeof(GLMmaterial) * nummaterials);
	model->nummaterials = nummaterials;

	/* set the default material */
	for (i = 0; i < nummaterials; i++) {
		model->materials[i].name = NULL;
		model->materials[i].shininess = 65.0;
		model->materials[i].diffuse[0] = 0.8;
		model->materials[i].diffuse[1] = 0.8;
		model->materials[i].diffuse[2] = 0.8;
		model->materials[i].diffuse[3] = 1.0;
		model->materials[i].ambient[0] = 0.2;
		model->materials[i].ambient[1] = 0.2;
		model->materials[i].ambient[2] = 0.2;
		model->materials[i].ambient[3] = 1.0;
		model->materials[i].specular[0] = 0.0;
		model->materials[i].specular[1] = 0.0;
		model->materials[i].specular[2] = 0.0;
		model->materials[i].specular[3] = 1.0;
	}
	model->materials[0].name = strdup("default");

	/* now, read in the data */
	nummaterials = 0;
	while (fscanf(file, "%s", buf) != EOF) {
		switch (buf[0]) {
		case '#':				/* comment */
		  /* eat up rest of line */
			fgets(buf, sizeof(buf), file);
			break;
		case 'n':				/* newmtl */
			fgets(buf, sizeof(buf), file);
			sscanf(buf, "%s %s", buf, buf);
			nummaterials++;
			model->materials[nummaterials].name = strdup(buf);
			break;
		case 'N':
			fscanf(file, "%f", &model->materials[nummaterials].shininess);
			/* wavefront shininess is from [0, 1000], so scale for OpenGL */
			model->materials[nummaterials].shininess /= 1000.0;
			model->materials[nummaterials].shininess *= 128.0;
			break;
		case 'K':
			switch (buf[1]) {
			case 'd':
				fscanf(file, "%f %f %f",
					&model->materials[nummaterials].diffuse[0],
					&model->materials[nummaterials].diffuse[1],
					&model->materials[nummaterials].diffuse[2]);
				break;
			case 's':
				fscanf(file, "%f %f %f",
					&model->materials[nummaterials].specular[0],
					&model->materials[nummaterials].specular[1],
					&model->materials[nummaterials].specular[2]);
				break;
			case 'a':
				fscanf(file, "%f %f %f",
					&model->materials[nummaterials].ambient[0],
					&model->materials[nummaterials].ambient[1],
					&model->materials[nummaterials].ambient[2]);
				break;
			default:
				/* eat up rest of line */
				fgets(buf, sizeof(buf), file);
				break;
			}
			break;
		default:
			/* eat up rest of line */
			fgets(buf, sizeof(buf), file);
			break;
		}
	}
}

/* glmWriteMTL: write a wavefront material library file
 *
 * model      - properly initialized GLMmodel structure
 * modelpath  - pathname of the model being written
 * mtllibname - name of the material library to be written
 */
static GLvoid
glmWriteMTL(GLMmodel* model, char* modelpath, char* mtllibname)
{
	FILE* file;
	char* dir;
	char* filename;
	GLMmaterial* material;
	GLuint i;

	dir = glmDirName(modelpath);
	filename = (char*)malloc(sizeof(char) * (strlen(dir) + strlen(mtllibname)));
	strcpy(filename, dir);
	strcat(filename, mtllibname);
	free(dir);

	/* open the file */
	file = fopen(filename, "w");
	if (!file) {
		fprintf(stderr, "glmWriteMTL() failed: can't open file \"%s\".\n",
			filename);
		exit(1);
	}
	free(filename);

	/* spit out a header */
	fprintf(file, "#  \n");
	fprintf(file, "#  Wavefront MTL generated by GLM library\n");
	fprintf(file, "#  \n");
	fprintf(file, "#  GLM library\n");
	fprintf(file, "#  Nate Robins\n");
	fprintf(file, "#  ndr@pobox.com\n");
	fprintf(file, "#  http://www.pobox.com/~ndr\n");
	fprintf(file, "#  \n\n");

	for (i = 0; i < model->nummaterials; i++) {
		material = &model->materials[i];
		fprintf(file, "newmtl %s\n", material->name);
		fprintf(file, "Ka %f %f %f\n",
			material->ambient[0], material->ambient[1], material->ambient[2]);
		fprintf(file, "Kd %f %f %f\n",
			material->diffuse[0], material->diffuse[1], material->diffuse[2]);
		fprintf(file, "Ks %f %f %f\n",
			material->specular[0], material->specular[1], material->specular[2]);
		fprintf(file, "Ns %f\n", material->shininess / 128.0 * 1000.0);
		fprintf(file, "\n");
	}
}


/* glmFirstPass: first pass at a Wavefront OBJ file that gets all the
 * statistics of the model (such as #vertices, #normals, etc)
 *
 * model - properly initialized GLMmodel structure
 * file  - (fopen'd) file descriptor
 */
static GLvoid
glmFirstPass(GLMmodel* model, FILE* file)
{
	GLuint    numvertices;		/* number of vertices in model */
	GLuint    numnormals;			/* number of normals in model */
	GLuint    numtexcoords;		/* number of texcoords in model */
	GLuint    numtriangles;		/* number of triangles in model */
	GLMgroup* group;			/* current group */
	unsigned  v, n, t;
	char      buf[128];

	/* make a default group */
	char def[] = "default";
	group = glmAddGroup(model, def);

	numvertices = numnormals = numtexcoords = numtriangles = 0;
	while (fscanf(file, "%s", buf) != EOF) {
		switch (buf[0]) {
		case '#':				/* comment */
		  /* eat up rest of line */
			fgets(buf, sizeof(buf), file);
			break;
		case 'v':				/* v, vn, vt */
			switch (buf[1]) {
			case '\0':			/* vertex */
		  /* eat up rest of line */
				fgets(buf, sizeof(buf), file);
				numvertices++;
				break;
			case 'n':				/* normal */
		  /* eat up rest of line */
				fgets(buf, sizeof(buf), file);
				numnormals++;
				break;
			case 't':				/* texcoord */
		  /* eat up rest of line */
				fgets(buf, sizeof(buf), file);
				numtexcoords++;
				break;
			default:
				printf("glmFirstPass(): Unknown token \"%s\".\n", buf);
				exit(1);
				break;
			}
			break;
		case 'm':
			fgets(buf, sizeof(buf), file);
			sscanf(buf, "%s %s", buf, buf);
			model->mtllibname = strdup(buf);
			glmReadMTL(model, buf);
			break;
		case 'u':
			/* eat up rest of line */
			fgets(buf, sizeof(buf), file);
			break;
		case 'g':				/* group */
		  /* eat up rest of line */
			fgets(buf, sizeof(buf), file);
#if SINGLE_STRING_GROUP_NAMES
			sscanf(buf, "%s", buf);
#else
			buf[strlen(buf) - 1] = '\0';	/* nuke '\n' */
#endif
			group = glmAddGroup(model, buf);
			break;
		case 'f':				/* face */
			v = n = t = 0;
			fscanf(file, "%s", buf);
			/* can be one of %d, %d//%d, %d/%d, %d/%d/%d %d//%d */
			if (strstr(buf, "//")) {
				/* v//n */
				sscanf(buf, "%d//%d", &v, &n);
				fscanf(file, "%d//%d", &v, &n);
				fscanf(file, "%d//%d", &v, &n);
				numtriangles++;
				group->numtriangles++;
				while (fscanf(file, "%d//%d", &v, &n) > 0) {
					numtriangles++;
					group->numtriangles++;
				}
			}
			else if (sscanf(buf, "%d/%d/%d", &v, &t, &n) == 3) {
				/* v/t/n */
				fscanf(file, "%d/%d/%d", &v, &t, &n);
				fscanf(file, "%d/%d/%d", &v, &t, &n);
				numtriangles++;
				group->numtriangles++;
				while (fscanf(file, "%d/%d/%d", &v, &t, &n) > 0) {
					numtriangles++;
					group->numtriangles++;
				}
			}
			else if (sscanf(buf, "%d/%d", &v, &t) == 2) {
				/* v/t */
				fscanf(file, "%d/%d", &v, &t);
				fscanf(file, "%d/%d", &v, &t);
				numtriangles++;
				group->numtriangles++;
				while (fscanf(file, "%d/%d", &v, &t) > 0) {
					numtriangles++;
					group->numtriangles++;
				}
			}
			else {
				/* v */
				fscanf(file, "%d", &v);
				fscanf(file, "%d", &v);
				numtriangles++;
				group->numtriangles++;
				while (fscanf(file, "%d", &v) > 0) {
					numtriangles++;
					group->numtriangles++;
				}
			}
			break;

		default:
			/* eat up rest of line */
			fgets(buf, sizeof(buf), file);
			break;
		}
	}

	/* set the stats in the model structure */
	model->numvertices = numvertices;
	model->numnormals = numnormals;
	model->numtexcoords = numtexcoords;
	model->numtriangles = numtriangles;

	/* allocate memory for the triangles in each group */
	group = model->groups;
	while (group) {
		group->triangles = (GLuint*)malloc(sizeof(GLuint) * group->numtriangles);
		group->numtriangles = 0;
		group = group->next;
	}
}

/* glmSecondPass: second pass at a Wavefront OBJ file that gets all
 * the data.
 *
 * model - properly initialized GLMmodel structure
 * file  - (fopen'd) file descriptor
 */
static GLvoid
glmSecondPass(GLMmodel* model, FILE* file)
{
	GLuint    numvertices;		/* number of vertices in model */
	GLuint    numnormals;			/* number of normals in model */
	GLuint    numtexcoords;		/* number of texcoords in model */
	GLuint    numtriangles;		/* number of triangles in model */
	GLfloat* vertices;			/* array of vertices  */
	GLfloat* normals;			/* array of normals */
	GLfloat* texcoords;			/* array of texture coordinates */
	GLMgroup* group;			/* current group pointer */
	GLuint    material;			/* current material */
	GLuint    v, n, t;
	char      buf[128];

	/* set the pointer shortcuts */
	vertices = model->vertices;
	normals = model->normals;
	texcoords = model->texcoords;
	group = model->groups;

	/* on the second pass through the file, read all the data into the
	   allocated arrays */
	numvertices = numnormals = numtexcoords = 1;
	numtriangles = 0;
	material = 0;
	while (fscanf(file, "%s", buf) != EOF) {
		switch (buf[0]) {
		case '#':				/* comment */
		  /* eat up rest of line */
			fgets(buf, sizeof(buf), file);
			break;
		case 'v':				/* v, vn, vt */
			switch (buf[1]) {
			case '\0':			/* vertex */
				fscanf(file, "%f %f %f",
					&vertices[3 * numvertices + 0],
					&vertices[3 * numvertices + 1],
					&vertices[3 * numvertices + 2]);
				numvertices++;
				break;
			case 'n':				/* normal */
				fscanf(file, "%f %f %f",
					&normals[3 * numnormals + 0],
					&normals[3 * numnormals + 1],
					&normals[3 * numnormals + 2]);
				numnormals++;
				break;
			case 't':				/* texcoord */
				fscanf(file, "%f %f",
					&texcoords[2 * numtexcoords + 0],
					&texcoords[2 * numtexcoords + 1]);
				numtexcoords++;
				break;
			}
			break;
		case 'u':
			fgets(buf, sizeof(buf), file);
			sscanf(buf, "%s %s", buf, buf);
			group->material = material = glmFindMaterial(model, buf);
			break;
		case 'g':				/* group */
		  /* eat up rest of line */
			fgets(buf, sizeof(buf), file);
#if SINGLE_STRING_GROUP_NAMES
			sscanf(buf, "%s", buf);
#else
			buf[strlen(buf) - 1] = '\0';	/* nuke '\n' */
#endif
			group = glmFindGroup(model, buf);
			group->material = material;
			break;
		case 'f':				/* face */
			v = n = t = 0;
			fscanf(file, "%s", buf);
			/* can be one of %d, %d//%d, %d/%d, %d/%d/%d %d//%d */
			if (strstr(buf, "//")) {
				/* v//n */
				sscanf(buf, "%d//%d", &v, &n);
				T(numtriangles).vindices[0] = v;
				T(numtriangles).nindices[0] = n;
				fscanf(file, "%d//%d", &v, &n);
				T(numtriangles).vindices[1] = v;
				T(numtriangles).nindices[1] = n;
				fscanf(file, "%d//%d", &v, &n);
				T(numtriangles).vindices[2] = v;
				T(numtriangles).nindices[2] = n;
				group->triangles[group->numtriangles++] = numtriangles;
				numtriangles++;
				while (fscanf(file, "%d//%d", &v, &n) > 0) {
					T(numtriangles).vindices[0] = T(numtriangles - 1).vindices[0];
					T(numtriangles).nindices[0] = T(numtriangles - 1).nindices[0];
					T(numtriangles).vindices[1] = T(numtriangles - 1).vindices[2];
					T(numtriangles).nindices[1] = T(numtriangles - 1).nindices[2];
					T(numtriangles).vindices[2] = v;
					T(numtriangles).nindices[2] = n;
					group->triangles[group->numtriangles++] = numtriangles;
					numtriangles++;
				}
			}
			else if (sscanf(buf, "%d/%d/%d", &v, &t, &n) == 3) {
				/* v/t/n */
				T(numtriangles).vindices[0] = v;
				T(numtriangles).tindices[0] = t;
				T(numtriangles).nindices[0] = n;
				fscanf(file, "%d/%d/%d", &v, &t, &n);
				T(numtriangles).vindices[1] = v;
				T(numtriangles).tindices[1] = t;
				T(numtriangles).nindices[1] = n;
				fscanf(file, "%d/%d/%d", &v, &t, &n);
				T(numtriangles).vindices[2] = v;
				T(numtriangles).tindices[2] = t;
				T(numtriangles).nindices[2] = n;
				group->triangles[group->numtriangles++] = numtriangles;
				numtriangles++;
				while (fscanf(file, "%d/%d/%d", &v, &t, &n) > 0) {
					T(numtriangles).vindices[0] = T(numtriangles - 1).vindices[0];
					T(numtriangles).tindices[0] = T(numtriangles - 1).tindices[0];
					T(numtriangles).nindices[0] = T(numtriangles - 1).nindices[0];
					T(numtriangles).vindices[1] = T(numtriangles - 1).vindices[2];
					T(numtriangles).tindices[1] = T(numtriangles - 1).tindices[2];
					T(numtriangles).nindices[1] = T(numtriangles - 1).nindices[2];
					T(numtriangles).vindices[2] = v;
					T(numtriangles).tindices[2] = t;
					T(numtriangles).nindices[2] = n;
					group->triangles[group->numtriangles++] = numtriangles;
					numtriangles++;
				}
			}
			else if (sscanf(buf, "%d/%d", &v, &t) == 2) {
				/* v/t */
				T(numtriangles).vindices[0] = v;
				T(numtriangles).tindices[0] = t;
				fscanf(file, "%d/%d", &v, &t);
				T(numtriangles).vindices[1] = v;
				T(numtriangles).tindices[1] = t;
				fscanf(file, "%d/%d", &v, &t);
				T(numtriangles).vindices[2] = v;
				T(numtriangles).tindices[2] = t;
				group->triangles[group->numtriangles++] = numtriangles;
				numtriangles++;
				while (fscanf(file, "%d/%d", &v, &t) > 0) {
					T(numtriangles).vindices[0] = T(numtriangles - 1).vindices[0];
					T(numtriangles).tindices[0] = T(numtriangles - 1).tindices[0];
					T(numtriangles).vindices[1] = T(numtriangles - 1).vindices[2];
					T(numtriangles).tindices[1] = T(numtriangles - 1).tindices[2];
					T(numtriangles).vindices[2] = v;
					T(numtriangles).tindices[2] = t;
					group->triangles[group->numtriangles++] = numtriangles;
					numtriangles++;
				}
			}
			else {
				/* v */
				sscanf(buf, "%d", &v);
				T(numtriangles).vindices[0] = v;
				fscanf(file, "%d", &v);
				T(numtriangles).vindices[1] = v;
				fscanf(file, "%d", &v);
				T(numtriangles).vindices[2] = v;
				group->triangles[group->numtriangles++] = numtriangles;
				numtriangles++;
				while (fscanf(file, "%d", &v) > 0) {
					T(numtriangles).vindices[0] = T(numtriangles - 1).vindices[0];
					T(numtriangles).vindices[1] = T(numtriangles - 1).vindices[2];
					T(numtriangles).vindices[2] = v;
					group->triangles[group->numtriangles++] = numtriangles;
					numtriangles++;
				}
			}
			break;

		default:
			/* eat up rest of line */
			fgets(buf, sizeof(buf), file);
			break;
		}
	}

#if 0
	/* announce the memory requirements */
	printf(" Memory: %d bytes\n",
		numvertices * 3 * sizeof(GLfloat) +
		numnormals * 3 * sizeof(GLfloat) * (numnormals ? 1 : 0) +
		numtexcoords * 3 * sizeof(GLfloat) * (numtexcoords ? 1 : 0) +
		numtriangles * sizeof(GLMtriangle));
#endif
}


/* public functions */


/* glmUnitize: "unitize" a model by translating it to the origin and
 * scaling it to fit in a unit cube around the origin.  Returns the
 * scalefactor used.
 *
 * model - properly initialized GLMmodel structure
 */
GLfloat
glmUnitize(GLMmodel* model)
{
	GLuint  i;
	GLfloat maxx, minx, maxy, miny, maxz, minz;
	GLfloat cx, cy, cz, w, h, d;
	GLfloat scale;

	assert(model);
	assert(model->vertices);

	/* get the max/mins */
	maxx = minx = model->vertices[3 + 0];
	maxy = miny = model->vertices[3 + 1];
	maxz = minz = model->vertices[3 + 2];
	for (i = 1; i <= model->numvertices; i++) {
		if (maxx < model->vertices[3 * i + 0])
			maxx = model->vertices[3 * i + 0];
		if (minx > model->vertices[3 * i + 0])
			minx = model->vertices[3 * i + 0];

		if (maxy < model->vertices[3 * i + 1])
			maxy = model->vertices[3 * i + 1];
		if (miny > model->vertices[3 * i + 1])
			miny = model->vertices[3 * i + 1];

		if (maxz < model->vertices[3 * i + 2])
			maxz = model->vertices[3 * i + 2];
		if (minz > model->vertices[3 * i + 2])
			minz = model->vertices[3 * i + 2];
	}

	/* calculate model width, height, and depth */
	w = glmAbs(maxx) + glmAbs(minx);
	h = glmAbs(maxy) + glmAbs(miny);
	d = glmAbs(maxz) + glmAbs(minz);

	/* calculate center of the model */
	cx = (maxx + minx) / 2.0;
	cy = (maxy + miny) / 2.0;
	cz = (maxz + minz) / 2.0;

	/* calculate unitizing scale factor */
	scale = 2.0 / glmMax(glmMax(w, h), d);

	/* translate around center then scale */
	for (i = 1; i <= model->numvertices; i++) {
		model->vertices[3 * i + 0] -= cx;
		model->vertices[3 * i + 1] -= cy;
		model->vertices[3 * i + 2] -= cz;
		model->vertices[3 * i + 0] *= scale;
		model->vertices[3 * i + 1] *= scale;
		model->vertices[3 * i + 2] *= scale;
	}

	return scale;
}

/* glmDimensions: Calculates the dimensions (width, height, depth) of
 * a model.
 *
 * model      - initialized GLMmodel structure
 * dimensions - array of 3 GLfloats (GLfloat dimensions[3])
 */
GLvoid
glmDimensions(GLMmodel* model, GLfloat* dimensions)
{
	GLuint i;
	GLfloat maxx, minx, maxy, miny, maxz, minz;

	assert(model);
	assert(model->vertices);
	assert(dimensions);

	/* get the max/mins */
	maxx = minx = model->vertices[3 + 0];
	maxy = miny = model->vertices[3 + 1];
	maxz = minz = model->vertices[3 + 2];
	for (i = 1; i <= model->numvertices; i++) {
		if (maxx < model->vertices[3 * i + 0])
			maxx = model->vertices[3 * i + 0];
		if (minx > model->vertices[3 * i + 0])
			minx = model->vertices[3 * i + 0];

		if (maxy < model->vertices[3 * i + 1])
			maxy = model->vertices[3 * i + 1];
		if (miny > model->vertices[3 * i + 1])
			miny = model->vertices[3 * i + 1];

		if (maxz < model->vertices[3 * i + 2])
			maxz = model->vertices[3 * i + 2];
		if (minz > model->vertices[3 * i + 2])
			minz = model->vertices[3 * i + 2];
	}

	/* calculate model width, height, and depth */
	dimensions[0] = glmAbs(maxx) + glmAbs(minx);
	dimensions[1] = glmAbs(maxy) + glmAbs(miny);
	dimensions[2] = glmAbs(maxz) + glmAbs(minz);
}

/* glmScale: Scales a model by a given amount.
 *
 * model - properly initialized GLMmodel structure
 * scale - scalefactor (0.5 = half as large, 2.0 = twice as large)
 */
GLvoid
glmScale(GLMmodel* model, GLfloat scale)
{
	GLuint i;

	for (i = 1; i <= model->numvertices; i++) {
		model->vertices[3 * i + 0] *= scale;
		model->vertices[3 * i + 1] *= scale;
		model->vertices[3 * i + 2] *= scale;
	}
}

/* glmReverseWinding: Reverse the polygon winding for all polygons in
 * this model.  Default winding is counter-clockwise.  Also changes
 * the direction of the normals.
 *
 * model - properly initialized GLMmodel structure
 */
GLvoid
glmReverseWinding(GLMmodel* model)
{
	GLuint i, swap;

	assert(model);

	for (i = 0; i < model->numtriangles; i++) {
		swap = T(i).vindices[0];
		T(i).vindices[0] = T(i).vindices[2];
		T(i).vindices[2] = swap;

		if (model->numnormals) {
			swap = T(i).nindices[0];
			T(i).nindices[0] = T(i).nindices[2];
			T(i).nindices[2] = swap;
		}

		if (model->numtexcoords) {
			swap = T(i).tindices[0];
			T(i).tindices[0] = T(i).tindices[2];
			T(i).tindices[2] = swap;
		}
	}

	/* reverse facet normals */
	for (i = 1; i <= model->numfacetnorms; i++) {
		model->facetnorms[3 * i + 0] = -model->facetnorms[3 * i + 0];
		model->facetnorms[3 * i + 1] = -model->facetnorms[3 * i + 1];
		model->facetnorms[3 * i + 2] = -model->facetnorms[3 * i + 2];
	}

	/* reverse vertex normals */
	for (i = 1; i <= model->numnormals; i++) {
		model->normals[3 * i + 0] = -model->normals[3 * i + 0];
		model->normals[3 * i + 1] = -model->normals[3 * i + 1];
		model->normals[3 * i + 2] = -model->normals[3 * i + 2];
	}
}

/* glmFacetNormals: Generates facet normals for a model (by taking the
 * cross product of the two vectors derived from the sides of each
 * triangle).  Assumes a counter-clockwise winding.
 *
 * model - initialized GLMmodel structure
 */
GLvoid
glmFacetNormals(GLMmodel* model)
{
	GLuint  i;
	GLfloat u[3];
	GLfloat v[3];

	assert(model);
	assert(model->vertices);

	/* clobber any old facetnormals */
	if (model->facetnorms)
		free(model->facetnorms);

	/* allocate memory for the new facet normals */
	model->numfacetnorms = model->numtriangles;
	model->facetnorms = (GLfloat*)malloc(sizeof(GLfloat) *
		3 * (model->numfacetnorms + 1));

	for (i = 0; i < model->numtriangles; i++) {
		model->triangles[i].findex = i + 1;

		u[0] = model->vertices[3 * T(i).vindices[1] + 0] -
			model->vertices[3 * T(i).vindices[0] + 0];
		u[1] = model->vertices[3 * T(i).vindices[1] + 1] -
			model->vertices[3 * T(i).vindices[0] + 1];
		u[2] = model->vertices[3 * T(i).vindices[1] + 2] -
			model->vertices[3 * T(i).vindices[0] + 2];

		v[0] = model->vertices[3 * T(i).vindices[2] + 0] -
			model->vertices[3 * T(i).vindices[0] + 0];
		v[1] = model->vertices[3 * T(i).vindices[2] + 1] -
			model->vertices[3 * T(i).vindices[0] + 1];
		v[2] = model->vertices[3 * T(i).vindices[2] + 2] -
			model->vertices[3 * T(i).vindices[0] + 2];

		glmCross(u, v, &model->facetnorms[3 * (i + 1)]);
		glmNormalize(&model->facetnorms[3 * (i + 1)]);
	}
}

/* glmVertexNormals: Generates smooth vertex normals for a model.
 * First builds a list of all the triangles each vertex is in.  Then
 * loops through each vertex in the the list averaging all the facet
 * normals of the triangles each vertex is in.  Finally, sets the
 * normal index in the triangle for the vertex to the generated smooth
 * normal.  If the dot product of a facet normal and the facet normal
 * associated with the first triangle in the list of triangles the
 * current vertex is in is greater than the cosine of the angle
 * parameter to the function, that facet normal is not added into the
 * average normal calculation and the corresponding vertex is given
 * the facet normal.  This tends to preserve hard edges.  The angle to
 * use depends on the model, but 90 degrees is usually a good start.
 *
 * model - initialized GLMmodel structure
 * angle - maximum angle (in degrees) to smooth across
 */
GLvoid
glmVertexNormals(GLMmodel* model, GLfloat angle)
{
	GLMnode* node;
	GLMnode* tail;
	GLMnode** members;
	GLfloat* normals;
	GLuint    numnormals;
	GLfloat   average[3];
	GLfloat   dot, cos_angle;
	GLuint    i, avg;

	assert(model);
	assert(model->facetnorms);

	/* calculate the cosine of the angle (in degrees) */
	cos_angle = cos(angle * M_PI / 180.0);

	/* nuke any previous normals */
	if (model->normals)
		free(model->normals);

	/* allocate space for new normals */
	model->numnormals = model->numtriangles * 3; /* 3 normals per triangle */
	model->normals = (GLfloat*)malloc(sizeof(GLfloat) * 3 * (model->numnormals + 1));

	/* allocate a structure that will hold a linked list of triangle
	   indices for each vertex */
	members = (GLMnode**)malloc(sizeof(GLMnode*) * (model->numvertices + 1));
	for (i = 1; i <= model->numvertices; i++)
		members[i] = NULL;

	/* for every triangle, create a node for each vertex in it */
	for (i = 0; i < model->numtriangles; i++) {
		node = (GLMnode*)malloc(sizeof(GLMnode));
		node->index = i;
		node->next = members[T(i).vindices[0]];
		members[T(i).vindices[0]] = node;

		node = (GLMnode*)malloc(sizeof(GLMnode));
		node->index = i;
		node->next = members[T(i).vindices[1]];
		members[T(i).vindices[1]] = node;

		node = (GLMnode*)malloc(sizeof(GLMnode));
		node->index = i;
		node->next = members[T(i).vindices[2]];
		members[T(i).vindices[2]] = node;
	}

	/* calculate the average normal for each vertex */
	numnormals = 1;
	for (i = 1; i <= model->numvertices; i++) {
		/* calculate an average normal for this vertex by averaging the
		   facet normal of every triangle this vertex is in */
		node = members[i];
		if (!node)
			fprintf(stderr, "glmVertexNormals(): vertex w/o a triangle\n");
		average[0] = 0.0; average[1] = 0.0; average[2] = 0.0;
		avg = 0;
		while (node) {
			/* only average if the dot product of the angle between the two
			   facet normals is greater than the cosine of the threshold
			   angle -- or, said another way, the angle between the two
			   facet normals is less than (or equal to) the threshold angle */
			dot = glmDot(&model->facetnorms[3 * T(node->index).findex],
				&model->facetnorms[3 * T(members[i]->index).findex]);
			if (dot > cos_angle) {
				node->averaged = GL_TRUE;
				average[0] += model->facetnorms[3 * T(node->index).findex + 0];
				average[1] += model->facetnorms[3 * T(node->index).findex + 1];
				average[2] += model->facetnorms[3 * T(node->index).findex + 2];
				avg = 1;			/* we averaged at least one normal! */
			}
			else {
				node->averaged = GL_FALSE;
			}
			node = node->next;
		}

		if (avg) {
			/* normalize the averaged normal */
			glmNormalize(average);

			/* add the normal to the vertex normals list */
			model->normals[3 * numnormals + 0] = average[0];
			model->normals[3 * numnormals + 1] = average[1];
			model->normals[3 * numnormals + 2] = average[2];
			avg = numnormals;
			numnormals++;
		}

		/* set the normal of this vertex in each triangle it is in */
		node = members[i];
		while (node) {
			if (node->averaged) {
				/* if this node was averaged, use the average normal */
				if (T(node->index).vindices[0] == i)
					T(node->index).nindices[0] = avg;
				else if (T(node->index).vindices[1] == i)
					T(node->index).nindices[1] = avg;
				else if (T(node->index).vindices[2] == i)
					T(node->index).nindices[2] = avg;
			}
			else {
				/* if this node wasn't averaged, use the facet normal */
				model->normals[3 * numnormals + 0] =
					model->facetnorms[3 * T(node->index).findex + 0];
				model->normals[3 * numnormals + 1] =
					model->facetnorms[3 * T(node->index).findex + 1];
				model->normals[3 * numnormals + 2] =
					model->facetnorms[3 * T(node->index).findex + 2];
				if (T(node->index).vindices[0] == i)
					T(node->index).nindices[0] = numnormals;
				else if (T(node->index).vindices[1] == i)
					T(node->index).nindices[1] = numnormals;
				else if (T(node->index).vindices[2] == i)
					T(node->index).nindices[2] = numnormals;
				numnormals++;
			}
			node = node->next;
		}
	}

	model->numnormals = numnormals - 1;

	/* free the member information */
	for (i = 1; i <= model->numvertices; i++) {
		node = members[i];
		while (node) {
			tail = node;
			node = node->next;
			free(tail);
		}
	}
	free(members);

	/* pack the normals array (we previously allocated the maximum
	   number of normals that could possibly be created (numtriangles *
	   3), so get rid of some of them (usually alot unless none of the
	   facet normals were averaged)) */
	normals = model->normals;
	model->normals = (GLfloat*)malloc(sizeof(GLfloat) * 3 * (model->numnormals + 1));
	for (i = 1; i <= model->numnormals; i++) {
		model->normals[3 * i + 0] = normals[3 * i + 0];
		model->normals[3 * i + 1] = normals[3 * i + 1];
		model->normals[3 * i + 2] = normals[3 * i + 2];
	}
	free(normals);
}


/* glmLinearTexture: Generates texture coordinates according to a
 * linear projection of the texture map.  It generates these by
 * linearly mapping the vertices onto a square.
 *
 * model - pointer to initialized GLMmodel structure
 */
GLvoid
glmLinearTexture(GLMmodel* model)
{
	GLMgroup* group;
	GLfloat dimensions[3];
	GLfloat x, y, scalefactor;
	GLuint i;

	assert(model);

	if (model->texcoords)
		free(model->texcoords);
	model->numtexcoords = model->numvertices;
	model->texcoords = (GLfloat*)malloc(sizeof(GLfloat) * 2 * (model->numtexcoords + 1));

	glmDimensions(model, dimensions);
	scalefactor = 2.0 /
		glmAbs(glmMax(glmMax(dimensions[0], dimensions[1]), dimensions[2]));

	/* do the calculations */
	for (i = 1; i <= model->numvertices; i++) {
		x = model->vertices[3 * i + 0] * scalefactor;
		y = model->vertices[3 * i + 2] * scalefactor;
		model->texcoords[2 * i + 0] = (x + 1.0) / 2.0;
		model->texcoords[2 * i + 1] = (y + 1.0) / 2.0;
	}

	/* go through and put texture coordinate indices in all the triangles */
	group = model->groups;
	while (group) {
		for (i = 0; i < group->numtriangles; i++) {
			T(group->triangles[i]).tindices[0] = T(group->triangles[i]).vindices[0];
			T(group->triangles[i]).tindices[1] = T(group->triangles[i]).vindices[1];
			T(group->triangles[i]).tindices[2] = T(group->triangles[i]).vindices[2];
		}
		group = group->next;
	}

#if 0
	printf("glmLinearTexture(): generated %d linear texture coordinates\n",
		model->numtexcoords);
#endif
}

/* glmSpheremapTexture: Generates texture coordinates according to a
 * spherical projection of the texture map.  Sometimes referred to as
 * spheremap, or reflection map texture coordinates.  It generates
 * these by using the normal to calculate where that vertex would map
 * onto a sphere.  Since it is impossible to map something flat
 * perfectly onto something spherical, there is distortion at the
 * poles.  This particular implementation causes the poles along the X
 * axis to be distorted.
 *
 * model - pointer to initialized GLMmodel structure
 */
GLvoid
glmSpheremapTexture(GLMmodel* model)
{
	GLMgroup* group;
	GLfloat theta, phi, rho, x, y, z, r;
	GLuint i;

	assert(model);
	assert(model->normals);

	if (model->texcoords)
		free(model->texcoords);
	model->numtexcoords = model->numnormals;
	model->texcoords = (GLfloat*)malloc(sizeof(GLfloat) * 2 * (model->numtexcoords + 1));

	for (i = 1; i <= model->numnormals; i++) {
		z = model->normals[3 * i + 0];	/* re-arrange for pole distortion */
		y = model->normals[3 * i + 1];
		x = model->normals[3 * i + 2];
		r = sqrt((x * x) + (y * y));
		rho = sqrt((r * r) + (z * z));

		if (r == 0.0) {
			theta = 0.0;
			phi = 0.0;
		}
		else {
			if (z == 0.0)
				phi = 3.14159265 / 2.0;
			else
				phi = acos(z / rho);

			if (y == 0.0)
				theta = 3.141592365 / 2.0;
			else
				theta = asin(y / r) + (3.14159265 / 2.0);
		}

		model->texcoords[2 * i + 0] = theta / 3.14159265;
		model->texcoords[2 * i + 1] = phi / 3.14159265;
	}

	/* go through and put texcoord indices in all the triangles */
	group = model->groups;
	while (group) {
		for (i = 0; i < group->numtriangles; i++) {
			T(group->triangles[i]).tindices[0] = T(group->triangles[i]).nindices[0];
			T(group->triangles[i]).tindices[1] = T(group->triangles[i]).nindices[1];
			T(group->triangles[i]).tindices[2] = T(group->triangles[i]).nindices[2];
		}
		group = group->next;
	}
}

/* glmDelete: Deletes a GLMmodel structure.
 *
 * model - initialized GLMmodel structure
 */
GLvoid
glmDelete(GLMmodel* model)
{
	GLMgroup* group;
	GLuint i;

	assert(model);

	if (model->pathname)   free(model->pathname);
	if (model->mtllibname) free(model->mtllibname);
	if (model->vertices)   free(model->vertices);
	if (model->normals)    free(model->normals);
	if (model->texcoords)  free(model->texcoords);
	if (model->facetnorms) free(model->facetnorms);
	if (model->triangles)  free(model->triangles);
	if (model->materials) {
		for (i = 0; i < model->nummaterials; i++)
			free(model->materials[i].name);
	}
	free(model->materials);
	while (model->groups) {
		group = model->groups;
		model->groups = model->groups->next;
		free(group->name);
		free(group->triangles);
		free(group);
	}

	free(model);
}

/* glmReadOBJ: Reads a model description from a Wavefront .OBJ file.
 * Returns a pointer to the created object which should be free'd with
 * glmDelete().
 *
 * filename - name of the file containing the Wavefront .OBJ format data.
 */
GLMmodel*
glmReadOBJ(char* filename)
{
	GLMmodel* model;
	FILE* file;

	/* open the file */
	file = fopen(filename, "r");
	if (!file) {
		fprintf(stderr, "glmReadOBJ() failed: can't open data file \"%s\".\n",
			filename);
		exit(1);
	}

	/* allocate a new model */
	model = (GLMmodel*)malloc(sizeof(GLMmodel));
	model->pathname = strdup(filename);
	model->mtllibname = NULL;
	model->numvertices = 0;
	model->vertices = NULL;
	model->numnormals = 0;
	model->normals = NULL;
	model->numtexcoords = 0;
	model->texcoords = NULL;
	model->numfacetnorms = 0;
	model->facetnorms = NULL;
	model->numtriangles = 0;
	model->triangles = NULL;
	model->nummaterials = 0;
	model->materials = NULL;
	model->numgroups = 0;
	model->groups = NULL;
	model->position[0] = 0.0;
	model->position[1] = 0.0;
	model->position[2] = 0.0;

	/* make a first pass through the file to get a count of the number
	   of vertices, normals, texcoords & triangles */
	glmFirstPass(model, file);

	/* allocate memory */
	model->vertices = (GLfloat*)malloc(sizeof(GLfloat) *
		3 * (model->numvertices + 1));
	model->triangles = (GLMtriangle*)malloc(sizeof(GLMtriangle) *
		model->numtriangles);
	if (model->numnormals) {
		model->normals = (GLfloat*)malloc(sizeof(GLfloat) *
			3 * (model->numnormals + 1));
	}
	if (model->numtexcoords) {
		model->texcoords = (GLfloat*)malloc(sizeof(GLfloat) *
			2 * (model->numtexcoords + 1));
	}

	/* rewind to beginning of file and read in the data this pass */
	rewind(file);

	glmSecondPass(model, file);

	/* close the file */
	fclose(file);

	return model;
}

/* glmWriteOBJ: Writes a model description in Wavefront .OBJ format to
 * a file.
 *
 * model    - initialized GLMmodel structure
 * filename - name of the file to write the Wavefront .OBJ format data to
 * mode     - a bitwise or of values describing what is written to the file
 *            GLM_NONE     -  render with only vertices
 *            GLM_FLAT     -  render with facet normals
 *            GLM_SMOOTH   -  render with vertex normals
 *            GLM_TEXTURE  -  render with texture coords
 *            GLM_COLOR    -  render with colors (color material)
 *            GLM_MATERIAL -  render with materials
 *            GLM_COLOR and GLM_MATERIAL should not both be specified.
 *            GLM_FLAT and GLM_SMOOTH should not both be specified.
 */
GLvoid
glmWriteOBJ(GLMmodel* model, char* filename, GLuint mode)
{
	GLuint    i;
	FILE* file;
	GLMgroup* group;

	assert(model);

	/* do a bit of warning */
	if (mode & GLM_FLAT && !model->facetnorms) {
		printf("glmWriteOBJ() warning: flat normal output requested "
			"with no facet normals defined.\n");
		mode &= ~GLM_FLAT;
	}
	if (mode & GLM_SMOOTH && !model->normals) {
		printf("glmWriteOBJ() warning: smooth normal output requested "
			"with no normals defined.\n");
		mode &= ~GLM_SMOOTH;
	}
	if (mode & GLM_TEXTURE && !model->texcoords) {
		printf("glmWriteOBJ() warning: texture coordinate output requested "
			"with no texture coordinates defined.\n");
		mode &= ~GLM_TEXTURE;
	}
	if (mode & GLM_FLAT && mode & GLM_SMOOTH) {
		printf("glmWriteOBJ() warning: flat normal output requested "
			"and smooth normal output requested (using smooth).\n");
		mode &= ~GLM_FLAT;
	}
	if (mode & GLM_COLOR && !model->materials) {
		printf("glmWriteOBJ() warning: color output requested "
			"with no colors (materials) defined.\n");
		mode &= ~GLM_COLOR;
	}
	if (mode & GLM_MATERIAL && !model->materials) {
		printf("glmWriteOBJ() warning: material output requested "
			"with no materials defined.\n");
		mode &= ~GLM_MATERIAL;
	}
	if (mode & GLM_COLOR && mode & GLM_MATERIAL) {
		printf("glmWriteOBJ() warning: color and material output requested "
			"outputting only materials.\n");
		mode &= ~GLM_COLOR;
	}


	/* open the file */
	file = fopen(filename, "w");
	if (!file) {
		fprintf(stderr, "glmWriteOBJ() failed: can't open file \"%s\" to write.\n",
			filename);
		exit(1);
	}

	/* spit out a header */
	fprintf(file, "#  \n");
	fprintf(file, "#  Wavefront OBJ generated by GLM library\n");
	fprintf(file, "#  \n");
	fprintf(file, "#  GLM library\n");
	fprintf(file, "#  Nate Robins\n");
	fprintf(file, "#  ndr@pobox.com\n");
	fprintf(file, "#  http://www.pobox.com/~ndr\n");
	fprintf(file, "#  \n");

	if (mode & GLM_MATERIAL && model->mtllibname) {
		fprintf(file, "\nmtllib %s\n\n", model->mtllibname);
		glmWriteMTL(model, filename, model->mtllibname);
	}

	/* spit out the vertices */
	fprintf(file, "\n");
	fprintf(file, "# %d vertices\n", model->numvertices);
	for (i = 1; i <= model->numvertices; i++) {
		fprintf(file, "v %f %f %f\n",
			model->vertices[3 * i + 0],
			model->vertices[3 * i + 1],
			model->vertices[3 * i + 2]);
	}

	/* spit out the smooth/flat normals */
	if (mode & GLM_SMOOTH) {
		fprintf(file, "\n");
		fprintf(file, "# %d normals\n", model->numnormals);
		for (i = 1; i <= model->numnormals; i++) {
			fprintf(file, "vn %f %f %f\n",
				model->normals[3 * i + 0],
				model->normals[3 * i + 1],
				model->normals[3 * i + 2]);
		}
	}
	else if (mode & GLM_FLAT) {
		fprintf(file, "\n");
		fprintf(file, "# %d normals\n", model->numfacetnorms);
		for (i = 1; i <= model->numnormals; i++) {
			fprintf(file, "vn %f %f %f\n",
				model->facetnorms[3 * i + 0],
				model->facetnorms[3 * i + 1],
				model->facetnorms[3 * i + 2]);
		}
	}

	/* spit out the texture coordinates */
	if (mode & GLM_TEXTURE) {
		fprintf(file, "\n");
		fprintf(file, "# %d texcoords\n", model->texcoords);
		for (i = 1; i <= model->numtexcoords; i++) {
			fprintf(file, "vt %f %f\n",
				model->texcoords[2 * i + 0],
				model->texcoords[2 * i + 1]);
		}
	}

	fprintf(file, "\n");
	fprintf(file, "# %d groups\n", model->numgroups);
	fprintf(file, "# %d faces (triangles)\n", model->numtriangles);
	fprintf(file, "\n");

	group = model->groups;
	while (group) {
		fprintf(file, "g %s\n", group->name);
		if (mode & GLM_MATERIAL)
			fprintf(file, "usemtl %s\n", model->materials[group->material].name);
		for (i = 0; i < group->numtriangles; i++) {
			if (mode & GLM_SMOOTH && mode & GLM_TEXTURE) {
				fprintf(file, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
					T(group->triangles[i]).vindices[0],
					T(group->triangles[i]).nindices[0],
					T(group->triangles[i]).tindices[0],
					T(group->triangles[i]).vindices[1],
					T(group->triangles[i]).nindices[1],
					T(group->triangles[i]).tindices[1],
					T(group->triangles[i]).vindices[2],
					T(group->triangles[i]).nindices[2],
					T(group->triangles[i]).tindices[2]);
			}
			else if (mode & GLM_FLAT && mode & GLM_TEXTURE) {
				fprintf(file, "f %d/%d %d/%d %d/%d\n",
					T(group->triangles[i]).vindices[0],
					T(group->triangles[i]).findex,
					T(group->triangles[i]).vindices[1],
					T(group->triangles[i]).findex,
					T(group->triangles[i]).vindices[2],
					T(group->triangles[i]).findex);
			}
			else if (mode & GLM_TEXTURE) {
				fprintf(file, "f %d/%d %d/%d %d/%d\n",
					T(group->triangles[i]).vindices[0],
					T(group->triangles[i]).tindices[0],
					T(group->triangles[i]).vindices[1],
					T(group->triangles[i]).tindices[1],
					T(group->triangles[i]).vindices[2],
					T(group->triangles[i]).tindices[2]);
			}
			else if (mode & GLM_SMOOTH) {
				fprintf(file, "f %d//%d %d//%d %d//%d\n",
					T(group->triangles[i]).vindices[0],
					T(group->triangles[i]).nindices[0],
					T(group->triangles[i]).vindices[1],
					T(group->triangles[i]).nindices[1],
					T(group->triangles[i]).vindices[2],
					T(group->triangles[i]).nindices[2]);
			}
			else if (mode & GLM_FLAT) {
				fprintf(file, "f %d//%d %d//%d %d//%d\n",
					T(group->triangles[i]).vindices[0],
					T(group->triangles[i]).findex,
					T(group->triangles[i]).vindices[1],
					T(group->triangles[i]).findex,
					T(group->triangles[i]).vindices[2],
					T(group->triangles[i]).findex);
			}
			else {
				fprintf(file, "f %d %d %d\n",
					T(group->triangles[i]).vindices[0],
					T(group->triangles[i]).vindices[1],
					T(group->triangles[i]).vindices[2]);
			}
		}
		fprintf(file, "\n");
		group = group->next;
	}

	fclose(file);
}

/* glmDraw: Renders the model to the current OpenGL context using the
 * mode specified.
 *
 * model    - initialized GLMmodel structure
 * mode     - a bitwise OR of values describing what is to be rendered.
 *            GLM_NONE     -  render with only vertices
 *            GLM_FLAT     -  render with facet normals
 *            GLM_SMOOTH   -  render with vertex normals
 *            GLM_TEXTURE  -  render with texture coords
 *            GLM_COLOR    -  render with colors (color material)
 *            GLM_MATERIAL -  render with materials
 *            GLM_COLOR and GLM_MATERIAL should not both be specified.
 *            GLM_FLAT and GLM_SMOOTH should not both be specified.
 */
GLvoid
glmDraw(GLMmodel* model, GLuint mode)
{
	static GLuint i;
	static GLMgroup* group;
	static GLMtriangle* triangle;
	static GLMmaterial* material;

	assert(model);
	assert(model->vertices);

	/* do a bit of warning */
	if (mode & GLM_FLAT && !model->facetnorms) {
		printf("glmDraw() warning: flat render mode requested "
			"with no facet normals defined.\n");
		mode &= ~GLM_FLAT;
	}
	if (mode & GLM_SMOOTH && !model->normals) {
		printf("glmDraw() warning: smooth render mode requested "
			"with no normals defined.\n");
		mode &= ~GLM_SMOOTH;
	}
	if (mode & GLM_TEXTURE && !model->texcoords) {
		printf("glmDraw() warning: texture render mode requested "
			"with no texture coordinates defined.\n");
		mode &= ~GLM_TEXTURE;
	}
	if (mode & GLM_FLAT && mode & GLM_SMOOTH) {
		printf("glmDraw() warning: flat render mode requested "
			"and smooth render mode requested (using smooth).\n");
		mode &= ~GLM_FLAT;
	}
	if (mode & GLM_COLOR && !model->materials) {
		printf("glmDraw() warning: color render mode requested "
			"with no materials defined.\n");
		mode &= ~GLM_COLOR;
	}
	if (mode & GLM_MATERIAL && !model->materials) {
		printf("glmDraw() warning: material render mode requested "
			"with no materials defined.\n");
		mode &= ~GLM_MATERIAL;
	}
	if (mode & GLM_COLOR && mode & GLM_MATERIAL) {
		printf("glmDraw() warning: color and material render mode requested "
			"using only material mode.\n");
		mode &= ~GLM_COLOR;
	}
	if (mode & GLM_COLOR)
		glEnable(GL_COLOR_MATERIAL);
	else if (mode & GLM_MATERIAL)
		glDisable(GL_COLOR_MATERIAL);

	/* perhaps this loop should be unrolled into material, color, flat,
	   smooth, etc. loops?  since most cpu's have good branch prediction
	   schemes (and these branches will always go one way), probably
	   wouldn't gain too much?  */

	group = model->groups;
	while (group) {
		if (mode & GLM_MATERIAL) {
			material = &model->materials[group->material];
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material->ambient);
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material->diffuse);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material->specular);
			glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material->shininess);
		}

		if (mode & GLM_COLOR) {
			glColor3fv(material->diffuse);
		}

		glBegin(GL_TRIANGLES);
		for (i = 0; i < group->numtriangles; i++) {
			triangle = &T(group->triangles[i]);

			if (mode & GLM_FLAT)
				glNormal3fv(&model->facetnorms[3 * triangle->findex]);

			if (mode & GLM_SMOOTH)
				glNormal3fv(&model->normals[3 * triangle->nindices[0]]);
			if (mode & GLM_TEXTURE)
				glTexCoord2fv(&model->texcoords[2 * triangle->tindices[0]]);
			glVertex3fv(&model->vertices[3 * triangle->vindices[0]]);

			if (mode & GLM_SMOOTH)
				glNormal3fv(&model->normals[3 * triangle->nindices[1]]);
			if (mode & GLM_TEXTURE)
				glTexCoord2fv(&model->texcoords[2 * triangle->tindices[1]]);
			glVertex3fv(&model->vertices[3 * triangle->vindices[1]]);

			if (mode & GLM_SMOOTH)
				glNormal3fv(&model->normals[3 * triangle->nindices[2]]);
			if (mode & GLM_TEXTURE)
				glTexCoord2fv(&model->texcoords[2 * triangle->tindices[2]]);
			glVertex3fv(&model->vertices[3 * triangle->vindices[2]]);

		}
		glEnd();

		group = group->next;
	}
}

/* glmList: Generates and returns a display list for the model using
 * the mode specified.
 *
 * model    - initialized GLMmodel structure
 * mode     - a bitwise OR of values describing what is to be rendered.
 *            GLM_NONE     -  render with only vertices
 *            GLM_FLAT     -  render with facet normals
 *            GLM_SMOOTH   -  render with vertex normals
 *            GLM_TEXTURE  -  render with texture coords
 *            GLM_COLOR    -  render with colors (color material)
 *            GLM_MATERIAL -  render with materials
 *            GLM_COLOR and GLM_MATERIAL should not both be specified.
 * GLM_FLAT and GLM_SMOOTH should not both be specified.  */
GLuint
glmList(GLMmodel* model, GLuint mode)
{
	GLuint list;

	list = glGenLists(1);
	glNewList(list, GL_COMPILE);
	glmDraw(model, mode);
	glEndList();

	return list;
}

/* glmWeld: eliminate (weld) vectors that are within an epsilon of
 * each other.
 *
 * model      - initialized GLMmodel structure
 * epsilon    - maximum difference between vertices
 *              ( 0.00001 is a good start for a unitized model)
 *
 */
GLvoid
glmWeld(GLMmodel* model, GLfloat epsilon)
{
	GLfloat* vectors;
	GLfloat* copies;
	GLuint   numvectors;
	GLuint   i;

	/* vertices */
	numvectors = model->numvertices;
	vectors = model->vertices;
	copies = glmWeldVectors(vectors, &numvectors, epsilon);

#if 0
	printf("glmWeld(): %d redundant vertices.\n",
		model->numvertices - numvectors - 1);
#endif

	for (i = 0; i < model->numtriangles; i++) {
		T(i).vindices[0] = (GLuint)vectors[3 * T(i).vindices[0] + 0];
		T(i).vindices[1] = (GLuint)vectors[3 * T(i).vindices[1] + 0];
		T(i).vindices[2] = (GLuint)vectors[3 * T(i).vindices[2] + 0];
	}

	/* free space for old vertices */
	free(vectors);

	/* allocate space for the new vertices */
	model->numvertices = numvectors;
	model->vertices = (GLfloat*)malloc(sizeof(GLfloat) *
		3 * (model->numvertices + 1));

	/* copy the optimized vertices into the actual vertex list */
	for (i = 1; i <= model->numvertices; i++) {
		model->vertices[3 * i + 0] = copies[3 * i + 0];
		model->vertices[3 * i + 1] = copies[3 * i + 1];
		model->vertices[3 * i + 2] = copies[3 * i + 2];
	}

	free(copies);
}


#if 0
/* normals */
if (model->numnormals) {
	numvectors = model->numnormals;
	vectors = model->normals;
	copies = glmOptimizeVectors(vectors, &numvectors);

	printf("glmOptimize(): %d redundant normals.\n",
		model->numnormals - numvectors);

	for (i = 0; i < model->numtriangles; i++) {
		T(i).nindices[0] = (GLuint)vectors[3 * T(i).nindices[0] + 0];
		T(i).nindices[1] = (GLuint)vectors[3 * T(i).nindices[1] + 0];
		T(i).nindices[2] = (GLuint)vectors[3 * T(i).nindices[2] + 0];
	}

	/* free space for old normals */
	free(vectors);

	/* allocate space for the new normals */
	model->numnormals = numvectors;
	model->normals = (GLfloat*)malloc(sizeof(GLfloat) *
		3 * (model->numnormals + 1));

	/* copy the optimized vertices into the actual vertex list */
	for (i = 1; i <= model->numnormals; i++) {
		model->normals[3 * i + 0] = copies[3 * i + 0];
		model->normals[3 * i + 1] = copies[3 * i + 1];
		model->normals[3 * i + 2] = copies[3 * i + 2];
	}

	free(copies);
}

/* texcoords */
if (model->numtexcoords) {
	numvectors = model->numtexcoords;
	vectors = model->texcoords;
	copies = glmOptimizeVectors(vectors, &numvectors);

	printf("glmOptimize(): %d redundant texcoords.\n",
		model->numtexcoords - numvectors);

	for (i = 0; i < model->numtriangles; i++) {
		for (j = 0; j < 3; j++) {
			T(i).tindices[j] = (GLuint)vectors[3 * T(i).tindices[j] + 0];
		}
	}

	/* free space for old texcoords */
	free(vectors);

	/* allocate space for the new texcoords */
	model->numtexcoords = numvectors;
	model->texcoords = (GLfloat*)malloc(sizeof(GLfloat) *
		2 * (model->numtexcoords + 1));

	/* copy the optimized vertices into the actual vertex list */
	for (i = 1; i <= model->numtexcoords; i++) {
		model->texcoords[2 * i + 0] = copies[2 * i + 0];
		model->texcoords[2 * i + 1] = copies[2 * i + 1];
	}

	free(copies);
}
#endif

#if 0
/* look for unused vertices */
/* look for unused normals */
/* look for unused texcoords */
for (i = 1; i <= model->numvertices; i++) {
	for (j = 0; j < model->numtriangles; i++) {
		if (T(j).vindices[0] == i ||
			T(j).vindices[1] == i ||
			T(j).vindices[1] == i)
			break;
	}
}
#endif



#define width 1540
#define height 440
#define halfWidth 770.0f
#define halfHeight 220.0f


static GLfloat spin = 0.0;
static GLint mouseClick = 0;
static GLint keyboardpress = 0;
static GLfloat rotation = 0;
static GLfloat  whiteline[3] = { 0.99f, 0.99f, 0.99f };
static GLfloat  redcircleone[3] = { 0.34f, 0.0f, 0.0f };
static GLfloat  bluething[3] = { 0.19f, 0.39f, 0.59f };
static GLfloat day = 0;
static GLfloat moveX = 0;
static GLfloat moveZ = 0;
static GLfloat jump = 0.0;
static GLfloat fire = 0.0;
static GLfloat monsterMove = -3.0;
static GLint monsterHP1 = 1;
static GLint monsterHP2 = 2;
static GLint monsterHP3 = 3;
static GLint fogState = 0;
static GLint fogMode;
static GLint snipeState = 0;
static GLint snipeMode;
static GLdouble snipeMove = 0.0;
static 	GLdouble seeX = 0.0;
static 	GLdouble seeY = 0.0;
static 	GLdouble seeZ = 4.0;
static 	GLint antiAaliasing = 0;
static 	GLint offsetPolygon = 0; 
static 	GLint bulletState = 0;
static 	GLdouble bulletMove = 0.0;



class Poly
{
protected:
	float startX, startY;
	void drawC(int(*coordinate)[2], int len)
	{
		/*if (keyboardpress == 1) {		//keyboard press 1, should use glDrawArrays() to draw polygons
			float* buffer = (float*)malloc(2 * len * sizeof(float));
			for (int i = 0; i < len; i++) {
				*(buffer + 2 * i) = ((coordinate[i][0] - startX) / startX);
				*(buffer + 2 * i + 1) = ((startY - coordinate[i][1]) / startY);
			}
			glEnableClientState(GL_VERTEX_ARRAY);

			glVertexPointer(2, GL_FLOAT, 0, buffer);
			glDrawArrays(GL_POLYGON, 0, len);
			glDisableClientState(GL_VERTEX_ARRAY);
			free(buffer);
		}
		else if (keyboardpress == 2) {	//keyboard press 2, should use glDrawElements() to draw polygons
			float* buffer = (float*)malloc(2 * len * sizeof(float));
			for (int i = 0; i < len; i++) {
				*(buffer + 2 * i) = ((coordinate[i][0] - startX) / startX);
				*(buffer + 2 * i + 1) = ((startY - coordinate[i][1]) / startY);
			}
			unsigned int* offset = (unsigned int*)malloc(len * sizeof(unsigned int));
			for (int i = 0; i < len; i++) {
				*(offset + i) = i;
			}
			glEnableClientState(GL_VERTEX_ARRAY);
			glVertexPointer(2, GL_FLOAT, 0, buffer);
			glDrawElements(GL_POLYGON, len, GL_UNSIGNED_INT, offset);
			glDisableClientState(GL_VERTEX_ARRAY);
			free(offset);
			free(buffer);
		}
		else if (keyboardpress == 3) {	//keyboard press 3, should use glMultiDrawArrays() to draw polygons

		}
		else if (keyboardpress == 4) {	//keyboard press 4, should use glMultiDrawElements() to draw polygons

		}
		else {*/
			glBegin(GL_POLYGON);
			for (int i = 0; i < len; i++)
			{
				glVertex2f(((coordinate[i][0] - startX) / startX), ((startY - coordinate[i][1]) / startY));
			}
			glEnd();
		//}
	}
public:
	Poly(float w, float h) {
		startX = w / 2;
		startY = h / 2;
	}
};

class Tri
{
protected:
	float startX, startY;
	void drawE(int(*coordinate)[2], int move)
	{
		if (move == 0)
		{
			glBegin(GL_TRIANGLES);
			for (int i = 0; i < 3; i++)
			{
				glVertex2f(((coordinate[i][0] - startX) / startX), ((startY - coordinate[i][1]) / startY));
			}
			glEnd();
		}
		else
		{
			glBegin(GL_TRIANGLES);
			for (int i = 0; i < 3; i++)
			{
				glVertex2f(((coordinate[i][0] + move - startX) / startX), ((startY - coordinate[i][1]) / startY));
			}
			glEnd();
		}
	}
public:
	Tri(float w, float h) {
		startX = w / 2;
		startY = h / 2;
	}
};

/////////////////////////////國//////////////////////////////////////
class A :protected Poly
{
private:
	int a1[5][2] = { {7,63},{2,190},{6,196},{17,195},{17,60} };
	int a2[5][2] = { {17,60},{17,77},{139,48},{148,38},{134,32} };
	int a3[4][2] = { {139,48},{139,190},{156,201},{148,38} };
	int a4[5][2] = { {139,190},{129,193},{138,216},{146,216},{156,201} };
	int a5[5][2] = { {17,77},{17,117},{40,104},{40,89},{28,70} };
	int a6[5][2] = { {40,89},{40,104},{102,78},{17,117},{28,77} };
	int a7[5][2] = { {40,89},{40,104},{104,77},{98,67},{81,73} };
	int a8[4][2] = { {81,50},{78,78},{98,67},{93,50} };
	int a9[4][2] = { {98,67},{104,77},{131,77},{126,64} };
	int a10[4][2] = { {100,52},{106,67},{126,64},{115,48} };
	int a11[4][2] = { {17,170},{17,195},{129,180},{130,169} };
	int a12[5][2] = { {55,97},{22,129},{23,137},{43,128},{57,109} };
	int a13[7][2] = { {43,128},{26,146},{28,160},{75,122},{103,93},{87,89},{57,109} };
	int a14[5][2] = { {26,146},{28,160},{96,134},{98,120},{75,122} };
	int a15[6][2] = { {81,73},{87,89},{130,161},{137,147},{103,93},{102,75} };
	int a16[4][2] = { {115,88},{60,172},{94,169},{125,100} };
	/*int a1[][2] = { {,},{,},{,},{,},{,} };*/

public:
	A(float w, float h) :Poly(width, height) {}
	void displayC()
	{
		drawC(a1, sizeof(a1) / sizeof(a1[0]));
		drawC(a2, sizeof(a2) / sizeof(a2[0]));
		drawC(a3, sizeof(a3) / sizeof(a3[0]));
		drawC(a4, sizeof(a4) / sizeof(a4[0]));
		drawC(a5, sizeof(a5) / sizeof(a5[0]));
		drawC(a6, sizeof(a6) / sizeof(a6[0]));
		drawC(a7, sizeof(a7) / sizeof(a7[0]));
		drawC(a8, sizeof(a8) / sizeof(a8[0]));
		drawC(a9, sizeof(a9) / sizeof(a9[0]));
		drawC(a10, sizeof(a10) / sizeof(a10[0]));
		drawC(a11, sizeof(a11) / sizeof(a11[0]));
		drawC(a12, sizeof(a12) / sizeof(a12[0]));
		drawC(a13, sizeof(a13) / sizeof(a13[0]));
		drawC(a14, sizeof(a14) / sizeof(a14[0]));
		drawC(a15, sizeof(a15) / sizeof(a15[0]));
		drawC(a16, sizeof(a16) / sizeof(a16[0]));

	}
};
/////////////////////////////立//////////////////////////////////////
class B :protected Poly
{
private:
	int b1[5][2] = { {270,40},{269,59},{285,78},{311,73},{312,66} };
	int b2[6][2] = { {227,90},{222,107},{232,113},{310,89},{311,73},{285,78} };
	int b3[6][2] = { {310,89},{334,91},{340,86},{340,77},{332,73},{311,73} };
	int b4[6][2] = { {291,94},{251,126},{249,143},{260,146},{287,112},{310,89} };
	int b5[4][2] = { {287,112},{262,143},{313,111},{323,104} };
	int b6[5][2] = { {323,104},{313,110},{325,111},{343,112},{339,106} };
	int b7[4][2] = { {325,111},{275,165},{296,163},{343,112} };
	int b8[7][2] = { {224,174},{240,189},{369,180},{369,170},{359,162},{296,163},{275,165} };
	/*int b[][2] = { {,},{,},{,},{,},{,} };*/

public:
	B(float w, float h) :Poly(width, height) {}
	void displayC()
	{
		drawC(b1, sizeof(b1) / sizeof(b1[0]));
		drawC(b2, sizeof(b2) / sizeof(b2[0]));
		drawC(b3, sizeof(b3) / sizeof(b3[0]));
		drawC(b4, sizeof(b4) / sizeof(b4[0]));
		drawC(b5, sizeof(b5) / sizeof(b5[0]));
		drawC(b6, sizeof(b6) / sizeof(b6[0]));
		drawC(b7, sizeof(b7) / sizeof(b7[0]));
		drawC(b8, sizeof(b8) / sizeof(b8[0]));
		//drawC(b, sizeof(b) / sizeof(b[0]));
	}
};
/////////////////////////////中//////////////////////////////////////
class C :protected Poly
{
private:
	int c1[4][2] = { {403,70},{400,100},{421,135}/*,{409,124},{427,135},{427,131},{423,135},{420,86},{416,82}*/,{418,70} };
	int c2[4][2] = { {418,70},{418,77},{490,68},{484,56} };
	int c3[4][2] = { {484,38},{484,56},{500,56},{500,34} };
	int c4[4][2] = { {484,56},{490,68},{550,68},{541,56} };
	int c5[4][2] = { {426,70},{435,87},{444,87},{479,69} };
	int c6[4][2] = { {490,68},{481,261},{500,253},{500,34} };
	int c7[4][2] = { {490,94},{456,114},{451,131},{489,123} };
	int c8[4][2] = { {541,56},{490,94},{500,99},{550,68} };
	int c9[4][2] = { {500,108},{500,122},{519,120},{523,109} };

	/*int c[][2] = { {,},{,},{,},{,},{,} };*/

public:
	C(float w, float h) :Poly(width, height) {}
	void displayC()
	{
		drawC(c1, sizeof(c1) / sizeof(c1[0]));
		drawC(c2, sizeof(c2) / sizeof(c2[0]));
		drawC(c3, sizeof(c3) / sizeof(c3[0]));
		drawC(c4, sizeof(c4) / sizeof(c4[0]));
		drawC(c5, sizeof(c5) / sizeof(c5[0]));
		drawC(c6, sizeof(c6) / sizeof(c6[0]));
		drawC(c7, sizeof(c7) / sizeof(c7[0]));
		drawC(c8, sizeof(c8) / sizeof(c8[0]));
		drawC(c9, sizeof(c9) / sizeof(c9[0]));
		//drawC(c1, sizeof(c1) / sizeof(c1[0]));
	}
};
/////////////////////////////興//////////////////////////////////////
class D :protected Poly
{
private:
	int d1[7][2] = { {617,51},{613,58},{617,60},{617,130},{631,129},{631,57},{627,51} };
	int d2[6][2] = {/* {627,51},*/{639,56},/*{639,127}*/{639,136},{645,144},{649,124},{649,56},{644,47} };
	int d2_5[4][2] = { {627,51},{635,51},{639,56},{644,47} };
	int d3[5][2] = { {655,50},{660,52},{660,137},{669,131},{670,43} };
	int d4[6][2] = { {670,43},{669,50},{705,48},{705,35},{701,31},{691,31} };
	int d5[4][2] = { {669,64},{669,81},{684,82},{684,62} };
	int d6[5][2] = { {677,82},{669,91},{669,114},{679,118},{680,82} };
	int d7[4][2] = { {684,45},{684,144},{696,136},{696,45} };
	int d8[6][2] = { {700,43},{701,53},{733,53},{741,33},{730,33},{716,43} };
	int d9[4][2] = { {696,45},{696,76},{701,67},{701,45} };
	int d10[4][2] = { {701,67},{696,76},{724,98},{731,90} };
	int d11[4][2] = { {680,95},{680,102},{684,102},{684,94} };
	int d12[3][2] = { {696,90},{731,90},{707,86} };
	int d13[4][2] = { {696,90},{706,111},{713,111},{702,94} };
	int d14[4][2] = { {718,53},{708,136},{718,122},{733,53} };
	int d15[9][2] = { {554,145},{554,161},{565,169},{679,120},{768,121},{771,113},{763,104},{753,110},{617,130} };
	int d16[6][2] = { {594,163},{599,180},{615,196},{616,169} };
	int d17[4][2] = { {616,169},{615,196},{648,177},{656,162} };
	int d18[3][2] = { {662,160},{662,163},{686,153} };
	int d19[4][2] = { {720,146},{760,200},{765,190},{768,175} };

	/*int d[][2] = { {,},{,},{,},{,},{,} };*/

public:
	D(float w, float h) :Poly(width, height) {}
	void displayC()
	{
		drawC(d1, sizeof(d1) / sizeof(d1[0]));
		drawC(d2, sizeof(d2) / sizeof(d2[0]));
		drawC(d2_5, sizeof(d2_5) / sizeof(d2_5[0]));
		drawC(d3, sizeof(d3) / sizeof(d3[0]));
		drawC(d4, sizeof(d4) / sizeof(d4[0]));
		drawC(d5, sizeof(d5) / sizeof(d5[0]));
		drawC(d6, sizeof(d6) / sizeof(d6[0]));
		drawC(d7, sizeof(d7) / sizeof(d7[0]));
		drawC(d8, sizeof(d8) / sizeof(d8[0]));
		drawC(d9, sizeof(d9) / sizeof(d9[0]));
		drawC(d10, sizeof(d10) / sizeof(d10[0]));
		drawC(d11, sizeof(d11) / sizeof(d11[0]));
		drawC(d12, sizeof(d12) / sizeof(d12[0]));
		drawC(d13, sizeof(d13) / sizeof(d13[0]));
		drawC(d14, sizeof(d14) / sizeof(d14[0]));
		drawC(d15, sizeof(d15) / sizeof(d15[0]));
		drawC(d16, sizeof(d16) / sizeof(d16[0]));
		drawC(d17, sizeof(d17) / sizeof(d17[0]));
		drawC(d18, sizeof(d18) / sizeof(d18[0]));
		drawC(d19, sizeof(d19) / sizeof(d19[0]));

		/*drawC(d, sizeof(d) / sizeof(d[0]));*/
	}
};
/////////////////////////////大//////////////////////////////////////
class E :protected Poly
{
private:
	int e1[4][2] = { {909,36},{893,90},{902,90},{914,36} };
	int e2[4][2] = { {909,36},{907,90},{911,90},{914,36} };
	int e3[8][2] = { {820,108},{823,124},{830,132},{847,125},{859,113},{928,96},{933,89},{931,78} };
	int e4[8][2] = { {886,105},{878,128},{836,167},{844,163},{847,168},{868,164},{886,140},{897,99} };
	int e5[3][2] = { {907,130},{953,175},{963,152} };
	int e6[3][2] = { {938,152},{915,166},{948,166} };

	/*int e[][2] = { {,},{,},{,},{,},{,} };*/

public:
	E(float w, float h) :Poly(width, height) {}
	void displayC()
	{
		drawC(e1, sizeof(e1) / sizeof(e1[0]));
		drawC(e2, sizeof(e2) / sizeof(e2[0]));
		drawC(e3, sizeof(e3) / sizeof(e3[0]));
		drawC(e4, sizeof(e4) / sizeof(e4[0]));
		drawC(e5, sizeof(e5) / sizeof(e5[0]));
		drawC(e6, sizeof(e6) / sizeof(e6[0]));

		/*drawC(e, sizeof(e) / sizeof(e[0]));*/
	}
};
/////////////////////////////學//////////////////////////////////////
class F :protected Poly
{
private:
	int f1[7][2] = { {1005,121},{988,140},{984,146},{984,162},{988,168},{995,168},{1005,147} };
	int f2[6][2] = { {1005,126},{1136,90},{1128,104},{1118,102},{1023,131},{1005,147} };
	int f3[6][2] = { {1136,90},{1128,104},{1109,124},{1127,113},{1148,110},{1151,102} };
	int f4[4][2] = { {1038,26},{1015,55},{1013,69},{1046,39} };
	int f5[4][2] = { {1018,40},{1018,125},{1030,120},{1027,45} };
	int f6[4][2] = { {1027,53},{1027,74},{1040,66},{1040,52} };
	int f7[4][2] = { {1074,26},{1027,90},{1027,103},{1082,37} };
	int f8[4][2] = { {1042,54},{1045,61},{1085,61},{1086,51} };
	int f9[4][2] = { {1073,61},{1050,115},{1068,111},{1079,71} };
	int f10[4][2] = { {1045,76},{1045,84},{1076,95},{1077,90} };
	int f11[5][2] = { {1079,44},{1088,50},{1118,50},{1124,32},{1115,30} };
	int f12[8][2] = { {1106,50},{1103,56},{1079,58},{1079,90},{1093,90},{1087,108},{1106,103},{1118,50} };
	int f13[7][2] = { {1083,115},{1060,131},{1065,137},{1078,138},{1087,138},{1094,126},{1092,119} };
	int f14[8][2] = { {1078,138},{1072,144},{1071,149},{1038,153},{1034,159},{1040,162},{1081,152},{1087,138} };
	int f15[7][2] = { {1087,138},{1081,152},{1124,155},{1128,150},{1125,145},{1095,144},{1087,138} };
	int f16[4][2] = { {1081,152},{1079,203}/*,{1025,241},{982,241},{1025,254}*/,{1092,215},{1094,150} };
	int f17[4][2] = { {1079,203},{1025,241}/*,{982,241}*/,{1025,254},{1092,215} };
	int f18[3][2] = { {1025,241},{982,241},{1025,254} };

	/*int f[][2] = { {1,},{1,},{1,},{1,},{1,} };*/

public:
	F(float w, float h) :Poly(width, height) {}
	void displayC()
	{
		drawC(f1, sizeof(f1) / sizeof(f1[0]));
		drawC(f2, sizeof(f2) / sizeof(f2[0]));
		drawC(f3, sizeof(f3) / sizeof(f3[0]));
		drawC(f4, sizeof(f4) / sizeof(f4[0]));
		drawC(f5, sizeof(f5) / sizeof(f5[0]));
		drawC(f6, sizeof(f6) / sizeof(f6[0]));
		drawC(f7, sizeof(f7) / sizeof(f7[0]));
		drawC(f8, sizeof(f8) / sizeof(f8[0]));
		drawC(f9, sizeof(f9) / sizeof(f9[0]));
		drawC(f10, sizeof(f10) / sizeof(f10[0]));
		drawC(f11, sizeof(f11) / sizeof(f11[0]));
		drawC(f12, sizeof(f12) / sizeof(f12[0]));
		drawC(f13, sizeof(f13) / sizeof(f13[0]));
		drawC(f14, sizeof(f14) / sizeof(f14[0]));
		drawC(f15, sizeof(f15) / sizeof(f15[0]));
		drawC(f16, sizeof(f16) / sizeof(f16[0]));
		drawC(f17, sizeof(f17) / sizeof(f17[0]));
		drawC(f18, sizeof(f18) / sizeof(f18[0]));

		/*drawC(f, sizeof(f) / sizeof(f[0]));*/
	}
};
/////////////////////////////logo 白色線線///////////////////////////
class G :protected Poly
{
private:
	int g1[4][2] = { {1286,13},{1290,24},{1366,63},{1377,59} };
	int g2[4][2] = { {1366,63},{1433,140},{1437,131},{1377,59} };
	int g3[4][2] = { {1433,140},{1457,190},{1456,178},{1437,131} };

	/*int g[][2] = { {1,},{1,},{1,},{1,},{1,} };*/

public:
	G(float w, float h) :Poly(width, height) {}
	void displayC()
	{
		drawC(g1, sizeof(g1) / sizeof(g1[0]));
		drawC(g2, sizeof(g2) / sizeof(g2[0]));
		drawC(g3, sizeof(g3) / sizeof(g3[0]));

		/*drawC(g, sizeof(g) / sizeof(g[0]));*/
	}
};
/////////////////////////////logo 紅色圈圈///////////////////////////
class H :protected Poly
{
private:
	int h1[5][2] = { {1366,63},{1361,91},{1373,104},{1389,110},{1402,104} };
	int h2[4][2] = { {1377,59},{1414,96},{1413,64},{1403,60} };
	/*int h[][2] = { {1,},{1,},{1,},{1,},{1,} };*/

public:
	H(float w, float h) :Poly(width, height) {}
	void displayC()
	{
		drawC(h1, sizeof(h1) / sizeof(h1[0]));
		drawC(h2, sizeof(h2) / sizeof(h2[0]));

		/*drawC(h, sizeof(h) / sizeof(h[0]));*/
	}
};

class I :protected Poly
{
private:
	int i1[6][2] = { {1366,63},{1349,81},{1344,107},{1373,133},{1402,132},{1420,124} };
	int i2[4][2] = { {1377,59},{1429,116},{1433,76},{1403,60} };

	/*int i[][2] = { {1,},{1,},{1,},{1,},{1,} };*/

public:
	I(float w, float h) :Poly(width, height) {}
	void displayC()
	{
		drawC(i1, sizeof(i1) / sizeof(i1[0]));
		drawC(i2, sizeof(i2) / sizeof(i2[0]));

		/*drawC(i, sizeof(i) / sizeof(i[0]));*/
	}
};

class J :protected Poly
{
private:
	int j1[7][2] = { {1366,63},{1349,80},{1346,107},{1352,135},{1372,150},{1406,154},{1433,137} };
	int j2[6][2] = { {1377,59},{1437,131},{1447,114},{1433,76},{1413,64},{1403,60} };

	/*int h[][2] = { {1,},{1,},{1,},{1,},{1,} };*/

public:
	J(float w, float h) :Poly(width, height) {}
	void displayC()
	{
		drawC(j1, sizeof(j1) / sizeof(j1[0]));
		drawC(j2, sizeof(j2) / sizeof(j2[0]));

		/*drawC(h, sizeof(h) / sizeof(h[0]));*/
	}
};
/////////////////////////////logo 藍色咚咚///////////////////////////
class K :protected Poly
{
private:
	int k1[5][2] = { {1290,24},{1298,115},{1346,107},{1349,80},{1366,63} };
	int k2[9][2] = { {1298,115},{1291,155},{1266,225},{1333,200},{1395,195},{1395,157},{1372,150},{1352,135},{1346,107} };
	int k3[8][2] = { {1281,3},{1286,13},{1377,59},{1403,60},{1413,64},{1433,76},{1507,3},{1396,18} };
	int k4[4][2] = { {1507,3},{1433,76},{1447,114},{1487,59} };
	int k5[6][2] = { {1487,59},{1447,114},{1437,131},{1456,178},{1457,190},{1488,114}, };
	int k6[5][2] = { {1488,114},{1456,178},{1457,190},{1499,202},{1494,159} };
	int k7[3][2] = { {1494,159},{1499,202},{1523,230} };
	int k8[6][2] = { {1433,140},{1406,154},{1395,155},{1395,195},{1499,202},{1457,190} };

	/*int k[][2] = { {1,},{1,},{1,},{1,},{1,} };*/

public:
	K(float w, float h) :Poly(width, height) {}
	void displayC()
	{
		drawC(k1, sizeof(k1) / sizeof(k1[0]));
		drawC(k2, sizeof(k2) / sizeof(k2[0]));
		drawC(k3, sizeof(k3) / sizeof(k3[0]));
		drawC(k4, sizeof(k4) / sizeof(k4[0]));
		drawC(k5, sizeof(k5) / sizeof(k5[0]));
		drawC(k6, sizeof(k6) / sizeof(k6[0]));
		drawC(k7, sizeof(k7) / sizeof(k7[0]));
		drawC(k8, sizeof(k8) / sizeof(k8[0]));

		/*drawC(k, sizeof(k) / sizeof(k[0]));*/
	}
};
/////////////////////////////N//////////////////////////////////////
class L :protected Tri
{
private:
	int l1[3][2] = { {14,353},{14,419},{25,419} };
	int l2[3][2] = { {14,353},{25,353},{25,419} };
	int l3[3][2] = { {25,353},{25,367},{58,419} };
	int l4[3][2] = { {58,419},{58,405},{25,353} };
	int l5[3][2] = { {58,353},{58,419},{69,419} };
	int l6[3][2] = { {58,353},{69,353},{69,419} };

	/*int l[3][2] = { {,},{,},{,} };*/

public:
	L(float w, float h) :Tri(width, height) {}
	void displayC()
	{
		//N
		drawE(l1, 0);
		drawE(l2, 0);
		drawE(l3, 0);
		drawE(l4, 0);
		drawE(l5, 0);
		drawE(l6, 0);
		//l
		drawE(l1, 375);
		drawE(l2, 375);
		//h
		drawE(l1, 498);
		drawE(l2, 498);
		//H
		drawE(l1, 759);
		drawE(l2, 759);
		//H
		drawE(l1, 804);
		drawE(l2, 804);
		/*drawC(f);*/
	}
};
/////////////////////////////a//////////////////////////////////////
class M :protected Tri
{
private:
	int m1[3][2] = { {88,384},{96,385},{98,372} };
	int m2[3][2] = { {98,372},{95,378},{122,372} };
	int m3[3][2] = { {95,378},{122,372},{130,378} };
	int m4[3][2] = { {122,372},{122,418},{130,378} };
	int m5[3][2] = { {130,378},{122,418},{130,418} };
	int m6[3][2] = { {122,390},{122,396},{107,396} };
	int m7[3][2] = { {122,390},{99,393},{99,401} };
	int m8[3][2] = { {87,411},{99,393},{99,401} };
	int m9[3][2] = { {87,411},{99,420},{99,401} };
	int m10[3][2] = { {113,414},{99,420},{99,414} };
	int m11[3][2] = { {122,406},{122,412},{99,420} };

	/*int m[3][2] = { {,},{,},{,} };*/

public:
	M(float w, float h) :Tri(width, height) {}
	void displayC()
	{
		drawE(m1, 0);
		drawE(m2, 0);
		drawE(m3, 0);
		drawE(m4, 0);
		drawE(m5, 0);
		drawE(m6, 0);
		drawE(m7, 0);
		drawE(m8, 0);
		drawE(m9, 0);
		drawE(m10, 0);
		drawE(m11, 0);

		drawE(m1, 238);
		drawE(m2, 238);
		drawE(m3, 238);
		drawE(m4, 238);
		drawE(m5, 238);
		drawE(m6, 238);
		drawE(m7, 238);
		drawE(m8, 238);
		drawE(m9, 238);
		drawE(m10, 238);
		drawE(m11, 238);
		/*drawC(m);*/
	}
};
/////////////////////////////t//////////////////////////////////////
class N :protected Tri
{
private:
	int n1[3][2] = { {144,372},{144,379},{168,379} };
	int n2[3][2] = { {168,372},{144,372},{168,379} };
	int n3[3][2] = { {160,355},{150,360},{150,417} };
	int n4[3][2] = { {160,355},{160,420},{150,417} };
	int n5[3][2] = { {160,413},{160,420},{170,417} };
	/*int n6[3][2] = { {58,353},{69,353},{69,419} };

	int n[3][2] = { {,},{,},{,} };*/

public:
	N(float w, float h) :Tri(width, height) {}
	void displayC()
	{
		drawE(n1, 0);
		drawE(n2, 0);
		drawE(n3, 0);
		drawE(n4, 0);
		drawE(n5, 0);

		drawE(n1, 1316);
		drawE(n2, 1316);
		drawE(n3, 1316);
		drawE(n4, 1316);
		drawE(n5, 1316);

		/*drawC(n); */
	}
};
/////////////////////////////i//////////////////////////////////////
class O :protected Tri
{
private:
	int o1[3][2] = { {182,354},{182,364},{191,364} };
	int o2[3][2] = { {182,354},{191,354},{191,364} };
	int o3[3][2] = { {182,372},{182,420},{191,420} };
	int o4[3][2] = { {182,372},{191,372},{191,420} };

	/*int o[3][2] = { {,},{,},{,} };*/

public:
	O(float w, float h) :Tri(width, height) {}
	void displayC()
	{
		//i
		drawE(o1, 0);
		drawE(o2, 0);
		drawE(o3, 0);
		drawE(o4, 0);
		//n
		drawE(o3, 85);
		drawE(o4, 85);
		//u
		drawE(o3, 421);
		drawE(o4, 421);
		//n
		drawE(o3, 448);
		drawE(o4, 448);
		//g
		drawE(o3, 542);
		drawE(o4, 542);
		//i
		drawE(o1, 719);
		drawE(o2, 719);
		drawE(o3, 719);
		drawE(o4, 719);
		//n
		drawE(o3, 746);
		drawE(o4, 746);
		//g
		drawE(o3, 840);
		drawE(o4, 840);
		//n
		drawE(o3, 964);
		drawE(o4, 964);
		//i
		drawE(o1, 1021);
		drawE(o2, 1021);
		drawE(o3, 1021);
		drawE(o4, 1021);
		//r
		drawE(o3, 1164);
		drawE(o4, 1164);
		//i
		drawE(o1, 1256);
		drawE(o2, 1256);
		drawE(o3, 1256);
		drawE(o4, 1256);



		/*drawC(n); */
	}
};
/////////////////////////////o//////////////////////////////////////
class P :protected Tri
{
private:
	int p1[3][2] = { {229,370},{229,377},{219,372} };
	int p2[3][2] = { {219,372},{207,385},{229,377} };
	int p11[3][2] = { {229,370},{229,377},{240,372} };
	int p12[3][2] = { {240,372},{229,377},{251,385} };
	int p3[3][2] = { {207,385},{215,385},{229,370} };
	int p13[3][2] = { {243,385},{251,385},{229,370} };
	int p4[3][2] = { {207,407},{207,385},{215,385} };
	int p14[3][2] = { {251,407},{251,385},{243,385} };
	int p5[3][2] = { {207,407},{215,417},{215,385} };
	int p15[3][2] = { {251,407},{243,418},{243,385} };
	int p6[3][2] = { {207,407},{229,422},{229,414} };
	int p16[3][2] = { {251,407},{229,422},{229,414} };
	int p7[3][2] = { {207,407},{215,417},{229,422} };
	int p17[3][2] = { {251,407},{243,418},{229,422} };

	/*int p[3][2] = { {000,000},{000,000},{000,000} };*/

public:
	P(float w, float h) :Tri(width, height) {}
	void displayC()
	{
		//o
		drawE(p1, 0);
		drawE(p2, 0);
		drawE(p3, 0);
		drawE(p11, 0);
		drawE(p12, 0);
		drawE(p13, 0);
		drawE(p4, 0);
		drawE(p14, 0);
		drawE(p5, 0);
		drawE(p15, 0);
		drawE(p6, 0);
		drawE(p16, 0);
		drawE(p7, 0);
		drawE(p17, 0);
		//g
		drawE(p1, 482);
		drawE(p2, 482);
		drawE(p3, 482);
		drawE(p4, 482);
		drawE(p5, 482);
		drawE(p6, 482);
		drawE(p7, 482);
		//g
		drawE(p1, 780);
		drawE(p2, 780);
		drawE(p3, 780);
		drawE(p4, 780);
		drawE(p5, 780);
		drawE(p6, 780);
		drawE(p7, 780);
		//e
		drawE(p1, 1079);
		drawE(p2, 1079);
		drawE(p3, 1079);
		drawE(p4, 1079);
		drawE(p5, 1079);
		drawE(p6, 1079);
		drawE(p7, 1079);
		/*drawC(f);*/
	}
};
/////////////////////////////n//////////////////////////////////////
class Q :protected Tri
{
private:
	int q1[3][2] = { {276,378},{276,386},{292,372} };
	int q2[3][2] = { {292,372},{276,386},{292,378} };
	int q3[3][2] = { {292,372},{292,378},{307,377} };
	int q4[3][2] = { {307,377},{301,383},{301,418} };
	int q5[3][2] = { {301,418},{309,418},{307,377} };
	int q6[3][2] = { {292,378},{301,383},{307,377} };

	/*int [3][2] = { {000,000},{000,000},{000,000} };*/

public:
	Q(float w, float h) :Tri(width, height) {}
	void displayC()
	{
		//n
		drawE(q1, 0);
		drawE(q2, 0);
		drawE(q3, 0);
		drawE(q4, 0);
		drawE(q5, 0);
		drawE(q6, 0);
		//n
		drawE(q1, 363);
		drawE(q2, 363);
		drawE(q3, 363);
		drawE(q4, 363);
		drawE(q5, 363);
		drawE(q6, 363);
		//n
		drawE(q1, 661);
		drawE(q2, 661);
		drawE(q3, 661);
		drawE(q4, 661);
		drawE(q5, 661);
		drawE(q6, 661);
		//n
		drawE(q1, 878);
		drawE(q2, 878);
		drawE(q3, 878);
		drawE(q4, 878);
		drawE(q5, 878);
		drawE(q6, 878);

		/*drawC();*/
	}
};
/////////////////////////////C//////////////////////////////////////
class R :protected Tri
{
private:
	int r1[13][3][2] =
	{
		/**/{{496,402},{487,397},{482,418}},
		{ {487,397},{482,418},{469,414} },
		{ {482,418},{469,414},{454,419} },
		{ {469,414},{454,419},{450,406} },
		{ {454,419},{450,406},{435,396} },
		{ {450,406},{435,396},{436,377} },
		{ {435,396},{436,377},{445,376} },
		{ {436,377},{445,376},{455,354} },
		{ {445,376},{455,354},{458,362} },
		{ {455,354},{458,362},{478,354} },
		{ {458,362},{478,354},{477,362} },
		{ {478,354},{477,362},{496,372} },
		{ {477,362},{496,372},{487,375} }
	};
	/*int [3][2] = { {000,000},{000,000},{000,000} };*/

public:
	R(float w, float h) :Tri(width, height) {}
	void displayC()
	{
		//C
		for (int i = 0; i < 13; i++)
		{
			drawE(r1[i], 0);
		}

		/*drawC();*/
	}
};
/////////////////////////////h//////////////////////////////////////
class S :protected Tri
{
private:
	int s1[8][3][2] =
	{
		{{520,387},{520,377},{530,371} },
		{ {520,387},{530,371},{530,377} },

		{ {530,371},{530,377},{545,371} },
		{ {530,377},{540,377},{545,371} },

		{ {545,371},{540,377},{553,381} },
		{ {540,377},{553,381} ,{545,384} },

		{ {553,381} ,{545,384},{553,419} },
		{ {545,384},{553,419},{545,420} },

	};
	/*int [3][2] = { {000,000},{000,000},{000,000} };*/

public:
	S(float w, float h) :Tri(width, height) {}
	void displayC()
	{
		//C
		for (int i = 0; i < 8; i++)
		{
			drawE(s1[i], 0);
		}

		/*drawC();*/
	}
};
/////////////////////////////u//////////////////////////////////////
class TT :protected Tri
{
private:
	int t1[6][3][2] =
	{
		{{605,413},{604,406},{590,421} },
		{ {604,406},{590,421},{590,414} },

		{ {590,421},{590,414},{571,409} },
		{ {590,414},{571,409},{580,409} },

		{ {571,409},{580,409},{571,372} },
		{ {580,409},{571,372},{580,372} },

	};
	/*int [3][2] = { {000,000},{000,000},{000,000} };*/

public:
	TT(float w, float h) :Tri(width, height) {}
	void displayC()
	{
		//C
		for (int i = 0; i < 6; i++)
		{
			drawE(t1[i], 0);
		}

		/*drawC();*/
	}
};
/////////////////////////////g//////////////////////////////////////
class U :protected Tri
{
private:
	int u1[9][3][2] =
	{
		{{709,420},{709,413},{724,406} },
		{ {709,420},{724,414},{724,406} },

		{ {709,377},{709,370},{724,386} },
		{ {709,370},{724,386},{724,378} },

		{ {688,424},{698,424},{707,439} },
		{ {698,424},{707,439},{703,433} },

		{ {703,433},{707,439},{719,431} },
		{ {707,439},{719,431},{732,420} },

		{ {724,414},{719,431},{732,420} },
	};
	/*int [3][2] = { {000,000},{000,000},{000,000} };*/

public:
	U(float w, float h) :Tri(width, height) {}
	void displayC()
	{
		//C
		for (int i = 0; i < 9; i++)
		{
			drawE(u1[i], 0);
			drawE(u1[i], 298);

		}

		/*drawC();*/
	}
};
/////////////////////////////H//////////////////////////////////////
class V :protected Tri
{
private:
	int v1[2][3][2] =
	{
		{{783,380},{783,391},{820,380} },
		{ {783,391},{820,380},{820,391} }

	};
	/*int [3][2] = { {000,000},{000,000},{000,000} };*/

public:
	V(float w, float h) :Tri(width, height) {}
	void displayC()
	{
		//C
		for (int i = 0; i < 2; i++)
		{
			drawE(v1[i], 0);
		}

		/*drawC();*/
	}
};
/////////////////////////////s//////////////////////////////////////
class W :protected Tri
{
private:
	int w1[12][3][2] =
	{
		{ {844,405},{855,404},{858,421} },
		{ {855,404},{858,421},{859,413} },

		{ {858,421},{859,413},{874,421} },
		{ {859,413},{874,421},{884,413} },

		{ {874,421},{884,413},{877,404} },
		{ {884,413},{877,404},{882,396} },

		{ {877,404},{882,396},{851,394} },
		{ {882,396},{851,394},{846,381} },

		{ {846,381},{855,387},{854,373} },
		{ {854,373},{856,379},{875,372} },

		{ {872,378},{875,372},{877,385} },
		{ {875,372},{877,385},{885,383} },

		//{ {858,421},{874,421},{859,413} },
		//{ {875,372},{877,385},{885,383} },
	};
	/*int [3][2] = { {000,000},{000,000},{000,000} };*/

public:
	W(float w, float h) :Tri(width, height) {}
	void displayC()
	{
		//C
		for (int i = 0; i < 12; i++)
		{
			drawE(w1[i], 0);
			drawE(w1[i], 537);

		}

		/*drawC();*/
	}
};
/////////////////////////////U//////////////////////////////////////
class X :protected Tri
{
private:
	int x1[7][3][2] =
	{
		{ {1070,354},{1081,354},{1074,408} },
		{ {1081,354},{1074,408},{1081,416} },

		{ {1081,416},{1081,404},{1099,421} },
		{ {1118,416},{1118,404},{1099,421} },

		{ {1089,410},{1111,410},{1099,421} },

		{ {1127,354},{1116,354},{1125,408} },
		{ {1116,354},{1125,408},{1118,416} },

	};

public:
	X(float w, float h) :Tri(width, height) {}
	void displayC()
	{
		//C
		for (int i = 0; i < 7; i++)
		{
			drawE(x1[i], 0);
		}

		/*drawC();*/
	}
};
/////////////////////////////v//////////////////////////////////////
class Y :protected Tri
{
private:
	int y1[5][3][2] =
	{
		{ {1237,371},{1228,371},{1250,409} },
		{ {1228,371},{1250,409},{1246,419} },

		{ {1250,409},{1246,419},{1255,419} },

		{ {1265,371},{1275,371},{1250,409} },
		{ {1275,371},{1250,409},{1255,419} },

	};

public:
	Y(float w, float h) :Tri(width, height) {}
	void displayC()
	{
		//C
		for (int i = 0; i < 5; i++)
		{
			drawE(y1[i], 0);
			drawE(y1[i], 265);

		}

		/*drawC();*/
	}
};
/////////////////////////////v//////////////////////////////////////
class Z :protected Tri
{
private:
	int z1[15][3][2] =
	{
		//o
		{ {1292,398},{1291,392},{1332,397} },
		{ {1291,392},{1332,397},{1321,392} },

		{ {1332,397},{1321,392},{1328,381} },
		{ {1321,392},{1328,381},{1321,375} },

		{ {1308,378},{1308,371},{1321,375} },
		{ {1308,378},{1321,375},{1322,384} },

		{ {1308,414},{1308,422},{1322,405} },
		{ {1308,422},{1322,405},{1330,409} },
		//r
		{ {1354,378},{1355,386},{1364,372} },
		{ {1355,386},{1364,372},{1372,381} },
		{ {1364,372},{1372,381},{1374,374} },
		//y
		{ {1517,430},{1512,419},{1503,439} },
		{ {1503,439},{1508,428},{1496,438} },
		{ {1503,431},{1496,438},{1503,431} },

		{ {1512,419},{1522,419},{1517,430} },
	};

public:
	Z(float w, float h) :Tri(width, height) {}
	void displayC()
	{
		//C
		for (int i = 0; i < 15; i++)
		{
			drawE(z1[i], 0);
		}

		/*drawC();*/
	}
};

/////////////////////////////snipe//////////////////////////////////////
class Snipe :protected Poly
{
private:
	int ss1[4][2] = { {750,80},{790,80},{790,200},{750,200} };
	int ss2[4][2] = { {750,240},{790,240},{790,360},{750,360} };
	int ss3[4][2] = { {250,210},{750,210},{750,230},{250,230} };
	int ss4[4][2] = { {790,210},{1290,210},{1290,230},{790,230} };
	/*int ss5[3][2] = { {907,130},{953,175},{963,152} };
	int ss6[3][2] = { {938,152},{915,166},{948,166} };

	int e[][2] = { {,},{,},{,},{,},{,} };*/

public:
	Snipe(float w, float h) :Poly(width, height) {}
	void displayC()
	{
		drawC(ss1, sizeof(ss1) / sizeof(ss1[0]));
		drawC(ss2, sizeof(ss2) / sizeof(ss2[0]));
		drawC(ss3, sizeof(ss3) / sizeof(ss3[0]));
		drawC(ss4, sizeof(ss4) / sizeof(ss4[0]));
		/*drawC(e5, sizeof(e5) / sizeof(e5[0]));
		drawC(e6, sizeof(e6) / sizeof(e6[0]));

		drawC(e, sizeof(e) / sizeof(e[0]));*/
	}
};

GLMmodel* pmodel = NULL;

const GLfloat light_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
const GLfloat light_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_position[] = { 2.0f, 5.0f, 5.0f, 0.0f };

const GLfloat mat_ambient[] = { 0.7f, 0.7f, 0.7f, 1.0f };
const GLfloat mat_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
const GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat high_shininess[] = { 100.0f };


void My_Display()
{
	//fog
	if (fogState == 1) {
		glEnable(GL_FOG);
		{
			GLfloat fogColor[4] = { 0.6, 0.6, 0.7, 1.0 };

			fogMode = GL_EXP;
			glFogi(GL_FOG_MODE, fogMode);
			glFogfv(GL_FOG_COLOR, fogColor);
			glFogf(GL_FOG_DENSITY, 0.35);
			glHint(GL_FOG_HINT, GL_DONT_CARE);
			glFogf(GL_FOG_START, 1.0);
			glFogf(GL_FOG_END, 5.0);
		}
		glClearColor(0.5, 0.5, 0.5, 1.0);
	}
	else {
		glDisable(GL_FOG);
		glClearColor(0.109, 0.188, 0.078, 0.0);
	}

	//Trigger anti-aliasing
	if (antiAaliasing == 0) {
		//cout << antiAaliasing << "  3" << endl;
		glEnable(GL_MULTISAMPLE);
		glEnable(GL_POINT_SMOOTH);
		glEnable(GL_LINE_SMOOTH);
		//glEnable(GL_POLYGON_SMOOTH);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
	}
	else {
		//cout << antiAaliasing << "  2" << endl;
		glDisable(GL_MULTISAMPLE);
		glDisable(GL_POINT_SMOOTH);
		glDisable(GL_LINE_SMOOTH);
		//glDisable(GL_POLYGON_SMOOTH);
		glDisable(GL_BLEND);
	}
	if (offsetPolygon == 1) {
		//cout << "on" << endl;

		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(10.0f, 10.0f);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	else {
		glDisable(GL_POLYGON_OFFSET_FILL);

	}
	

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();

	gluLookAt(seeX + moveX, seeY, seeZ + moveZ, 0 + moveX, 0 + jump, 0 + moveZ, 0.0, 1.0, 0.0);

	/* Floor */
	glColor3f(0.8, 0.8, 0.8);
	glBegin(GL_QUADS);
	glVertex3f(-15, -1, -15);
	glVertex3f(-15, -1, 15);
	glVertex3f(15, -1, 15);
	glVertex3f(15, -1, -15);
	glEnd();
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHT0);
	glDisable(GL_NORMALIZE);
	glDisable(GL_COLOR_MATERIAL);
	glDisable(GL_LIGHTING);

	glPushMatrix();
		float chColor[3] = { 0.0, 0.40f, 0.50f };
		glColor3fv(chColor);
		/////////////////////////////國//////////////////////////////////////
		A a(width, height);
		a.displayC();
		/////////////////////////////立//////////////////////////////////////
		B b(width, height);
		b.displayC();
		/////////////////////////////中//////////////////////////////////////
		C c(width, height);
		c.displayC();
		/////////////////////////////興//////////////////////////////////////
		D d(width, height);
		d.displayC();
		/////////////////////////////大//////////////////////////////////////
		E e(width, height);
		e.displayC();
		/////////////////////////////學//////////////////////////////////////
		F f(width, height);
		f.displayC();

		//1400,100
		glPushMatrix();
			glTranslatef((600 / halfWidth), (50 / halfHeight), 0.0);
			glRotatef((GLfloat)rotation, 0.0, 1.0, 0.0);
			glTranslatef(-(600 / halfWidth), -(50 / halfHeight), 0.0);
			/////////////////////////////logo 白色線線//////////////////////////////////////
			glColor3fv(whiteline);
			//glColor3f(0.99f, 0.99f, 0.99f);
			G g(width, height);
			g.displayC();
			/////////////////////////////logo 紅色圈圈//////////////////////////////////////
			glColor3fv(redcircleone);
			//glColor3f(0.34f, 0.0f, 0.0f);
			J j(width, height);
			j.displayC();

			//glColor3fv(redcircletwo);
			//glColor3f(0.59f, 0.19f, 0.19f);
			I i(width, height);
			i.displayC();

			//glColor3fv(redcirclethree);
			//glColor3f(0.99f, 0.19f, 0.19f);
			H h(width, height);
			h.displayC();
			/////////////////////////////logo 藍色東東//////////////////////////////////////
			glColor3fv(bluething);
			//glColor3f(0.19f, 0.39f, 0.59f);
			K k(width, height);
			k.displayC();

		glPopMatrix();









		//snipe
		float snipeColor[3] = { 0.0, 0.0f, 0.0f };
		glPushMatrix();
			glColor3fv(snipeColor);
			glTranslatef(seeX + moveX, 0.0, seeZ + moveZ - 2.0);
			Snipe sn(width, height);
			sn.displayC();
		glPopMatrix();

		//bullet
		if (bulletState == 1) {
			float bulletColor[3] = { 0.0, 0.0f, 0.0f };
			glPushMatrix();
				glColor3fv(bulletColor);
				glTranslatef(seeX + moveX, 0.0, seeZ + moveZ - bulletMove - 2.0);
				glutSolidSphere(0.8, 16, 16);
			glPopMatrix();
		}











		glPushMatrix();
			/////////////////////////////N//////////////////////////////////////
			float enColor[3] = { 0.0, 0.40f, 0.50f };
			glColor3fv(enColor);
			//glColor3f(0.0, 0.40f, 0.50f);
			L l(width, height);
			l.displayC();
			/////////////////////////////a//////////////////////////////////////
			M m(width, height);
			m.displayC();
			/////////////////////////////t//////////////////////////////////////
			N n(width, height);
			n.displayC();
			/////////////////////////////i//////////////////////////////////////
			O o(width, height);
			o.displayC();
			/////////////////////////////o//////////////////////////////////////
			P p(width, height);
			p.displayC();
			/////////////////////////////n//////////////////////////////////////
			Q q(width, height);
			q.displayC();
			/////////////////////////////C//////////////////////////////////////
			R r(width, height);
			r.displayC();
			/////////////////////////////h//////////////////////////////////////
			S s(width, height);
			s.displayC();
			/////////////////////////////u//////////////////////////////////////
			TT t(width, height);
			t.displayC();
			/////////////////////////////g//////////////////////////////////////
			U u(width, height);
			u.displayC();
			/////////////////////////////H//////////////////////////////////////
			V v(width, height);
			v.displayC();
			/////////////////////////////s//////////////////////////////////////
			W w(width, height);
			w.displayC();
			/////////////////////////////U//////////////////////////////////////
			X x(width, height);
			x.displayC();
			/////////////////////////////v//////////////////////////////////////
			Y y(width, height);
			y.displayC();
			/////////////////////////////ery//////////////////////////////////////
			Z z(width, height);
			z.displayC();
		glPopMatrix();
			
		/*al*/
		float monsterColor[3] = { 0.20f, 0.80f, 0.10f };
		glColor3fv(monsterColor);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		//glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		glEnable(GL_LIGHT0);
		glEnable(GL_NORMALIZE);
		glEnable(GL_COLOR_MATERIAL);
		glEnable(GL_LIGHTING);

		glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
		glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
		glLightfv(GL_LIGHT0, GL_POSITION, light_position);

		glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
		glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
		glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);
		/*al-1*/
		glPushMatrix();
			glTranslatef(0.0, 0.0, monsterMove);
			if (!pmodel) {
				char filename[] = "al.obj";
				pmodel = glmReadOBJ(filename);
				if (!pmodel) exit(0);
				glmUnitize(pmodel);
				glmFacetNormals(pmodel);
				glmVertexNormals(pmodel, 90.0);
			}
			glmDraw(pmodel, GLM_SMOOTH);
		glPopMatrix();
		/*al-2*/
		glPushMatrix();
			glTranslatef(2.0, 0.0, monsterMove);
			if (!pmodel) {
				char filename[] = "al.obj";
				pmodel = glmReadOBJ(filename);
				if (!pmodel) exit(0);
				glmUnitize(pmodel);
				glmFacetNormals(pmodel);
				glmVertexNormals(pmodel, 90.0);
			}
			glmDraw(pmodel, GLM_SMOOTH);
		glPopMatrix();
		/*al-3*/
		glPushMatrix();
			glTranslatef(-2.0, 0.0, monsterMove);
			if (!pmodel) {
				char filename[] = "al.obj";
				pmodel = glmReadOBJ(filename);
				if (!pmodel) exit(0);
				glmUnitize(pmodel);
				glmFacetNormals(pmodel);
				glmVertexNormals(pmodel, 90.0);
			}
			glmDraw(pmodel, GLM_SMOOTH);
		glPopMatrix();


	glPopMatrix();

	glutSwapBuffers();
}

void spinDisplay(void)
{
	//logo
	rotation = rotation + 0.5;
	if (rotation > 360.0)
		rotation = rotation - 360.0;
	glutPostRedisplay();

	whiteline[0] += 0.3;
	whiteline[1] += 0.3;
	whiteline[2] += 0.3;
	if (whiteline[0] >= 1.0)
		whiteline[0] = 0.0;
	if (whiteline[1] >= 1.0)
		whiteline[1] = 0.0;
	if (whiteline[2] >= 1.0)
		whiteline[2] = 0.0;
	redcircleone[0] += 0.3;
	redcircleone[1] += 0.3;
	redcircleone[2] += 0.3;
	if (redcircleone[0] >= 1.0)
		redcircleone[0] = 0.0;
	if (redcircleone[1] >= 1.0)
		redcircleone[1] = 0.0;
	if (redcircleone[2] >= 1.0)
		redcircleone[2] = 0.0;
	bluething[0] += 0.3;
	bluething[1] += 0.3;
	bluething[2] += 0.3;
	if (bluething[0] >= 1.0)
		bluething[0] = 0.0;
	if (bluething[1] >= 1.0)
		bluething[1] = 0.0;
	if (bluething[2] >= 1.0)
		bluething[2] = 0.0;

	glutPostRedisplay();
}

int hitten = 0;

void move(void) {
	//monsterMove
	monsterMove = monsterMove + 0.0002;
	if (monsterMove > 4) {
		hitten = hitten + 1;
		glClearColor(0.2f, 0.0f, 0.0f, 1.0f);
		if (hitten > 50) {
			hitten = 0;
			monsterMove = -3;
			glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		}
	}

	//bulletMove
	if (bulletState == 1) {
		bulletMove += 0.5;
		if (bulletMove > 20) {
			//cout << bulletMove << endl;
			bulletMove = 0;
			bulletState = 0;
		}
	}


	glutPostRedisplay();
}

void init(void)
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glShadeModel(GL_FLAT);
}

void reshape(int w, int h)
{
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(65.0, (GLfloat)w / (GLfloat)h, 1.0, 20.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void keyboardUp(unsigned char key, int x, int y) {
	glutIgnoreKeyRepeat(1); //ignore key repeat
	switch (key) {
	case 'b':
		seeZ = 4.0;
		bulletState = 1;
		break;
	default:
		break;
	}
	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'w':
		moveZ -= 0.05;
		break;
	case 's':
		moveZ += 0.05;
		break;
	case 'a':
		moveX -= 0.05;
		break;
	case 'd':
		moveX += 0.05;
		break;
	case 'b':
		seeZ = 3.0;
		break;
	case 'c':
		//cout << antiAaliasing << "  3" << endl;
		if (antiAaliasing == 0) {
			//cout << antiAaliasing << "  2" << endl;
			antiAaliasing = 1;
		}
		else {
			//cout << antiAaliasing << "  1" << endl;
			antiAaliasing = 0;
		}
		break;
	case 'f':
		if (fogState == 0) {
			fogState = 1;
		}
		else {
			fogState = 0;
		}
		break;
	case 'p':
		if (offsetPolygon == 0) {
			offsetPolygon = 1;
		}
		else {
			offsetPolygon = 0;
		}
		break;
	default:
		break;
	}
}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);	// Initialize GLUT
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);	// Set up some memory buffers for our display
	glutInitWindowPosition(300, 300);
	glutInitWindowSize(width, height);	// Set the window size
	glutCreateWindow("HW6");	// Create the window with the title "HW2"
	init();
	//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	glutDisplayFunc(My_Display);
	glutReshapeFunc(reshape);
	glutIdleFunc(spinDisplay);
	glutIdleFunc(move);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardUp);

	// Very important!  This initializes the entry points in the OpenGL driver so we can 
	// call all the functions in the API.
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		fprintf(stderr, "GLEW error");
		return 1;
	}

	glutMainLoop();
	return 0;
}








