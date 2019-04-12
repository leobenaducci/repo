// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

#ifndef PCH_H
#define PCH_H

#include <Windows.h>
#include <stdio.h>

#include <string>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <algorithm>

#include <typeinfo>
#include <functional>

#include <chrono>
#include <thread>

#define GLEW_STATIC 1
#include "glew-1.10.0/include/GL/glew.h"
#include "glew-1.10.0/include/GL/wglew.h"

#define FREEGLUT_LIB_PRAGMAS 0
#include "freeglut_bin/include/GL/freeglut.h"

#define GLM_FORCE_RADIANS 1
#include "glm/glm.hpp"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"

#define SHADER_EXTENSION ".glsl"

#include "../Common/shader.h"
#include "../Common/buffer.h"
#include "../Common/scene.h"
#include "../Common/framebuffer.h"
#include "../Common/camera.h"
#include "../Common/mesh.h"
#include "../Common/texture.h"
#include "../Common/raytrace.h"

static const std::string& GetResourcesPath()
{
	static std::string ret("../../resources/HybridRender/");
	return ret;
}
#endif //PCH_H
