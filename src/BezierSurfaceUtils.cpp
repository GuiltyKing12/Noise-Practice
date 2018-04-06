#include "../include/BezierSurfaceUtils.h"

#include <stdio.h>

#include <vector>

GLuint CSCI441::BezierSurfaceUtils::readSurfaceFile( const char *filename ) {
	int nPatches, nVertices;

	FILE *fp;

	if (!(fp = fopen(filename,"r"))) {
		fprintf(stderr,"[ERROR]: CSCI441::BezierSurfaceUtils::readSurfaceFile(): Can't open %s\n",filename);
		return 0;
	} else {
		printf( "\n[INFO]: Reading surface file: \"%s\"\n", filename );
	}

	fscanf( fp,"%i\n", &nPatches );

	printf( "[INFO]: There are %d patches to be read\n", nPatches );

	int index;
	std::vector<int> indices;

	for (int ii = 0; ii < nPatches; ii++) {
		for( int jj = 0; jj < 15; jj++ ) {
			fscanf( fp, "%i, ", &index );
			indices.push_back( index-1 );
		}
		fscanf( fp, "%i\n", &index );
		indices.push_back( index-1 );
	}

	fscanf(fp,"%i\n", &nVertices);

	printf( "[INFO]: There are %d vertices to be read\n", nVertices );

	float x,y,z;
	std::vector<float> pointList;

	for( int ii = 1; ii <= nVertices; ii++ ) {
		fscanf(fp,"%f, %f, %f\n",&x,&y,&z);
		pointList.push_back( x );
		pointList.push_back( y );
		pointList.push_back( z );
	}

	GLuint vaod;

	glGenVertexArrays( 1, &vaod );
	glBindVertexArray( vaod );

	GLuint vbods[2];
	glGenBuffers(2, vbods);
	glBindBuffer(GL_ARRAY_BUFFER, vbods[0]);
	glBufferData(GL_ARRAY_BUFFER, pointList.size() * sizeof(float), &pointList[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbods[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), &indices[0], GL_STATIC_DRAW);


	return vaod;
}

int CSCI441::BezierSurfaceUtils::getNumSurfaces( const char *filename ) {
	int patches;

	FILE *fp;

	if (!(fp = fopen(filename,"r"))) {
		fprintf(stderr,"[ERROR]: CSCI441::BezierSurfaceUtils::readSurfaceFile(): Can't open %s\n",filename);
		return 0;
	}

	fscanf( fp,"%i\n", &patches );

	return patches;
}

CSCI441::BezierSurfaceUtils::BezierSurfaceUtils() {}
