#pragma once

#include <GL/glew.h>

namespace CSCI441 {

	class BezierSurfaceUtils {
	public:
		static GLuint readSurfaceFile( const char *filename );
		static int getNumSurfaces( const char *filename );
	private:
		BezierSurfaceUtils();
	};

}
