#ifndef ROBOTGL_H
#define ROBOTGL_H

#define ROBOTGL_INIT_POS glm::vec3( 0, 0, 10 ); 
#define ROBOTGL_INIT_FOV 45.0f
#define ROBOTGL_INIT_HANG 3.14f
#define ROBOTGL_INIT_VANG 0.0f

#define SCR_WIDTH 640
#define SCR_HEIGHT 480

#define MAX_NUM_SEGMENTS 10

// Include GLFW
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "robotSeg.h"

namespace samsRobot{

	//////////////////////////////////////////////////
	static const char* vertex_shader_text =
		"#version 330 core\n"
		"layout (location=0) in vec3 vPos;\n"
		"layout (location=1) in vec3 vCol;\n"
		"layout (location=2) in vec2 vtex;\n"
		"uniform mat4 model;\n"
		"uniform mat4 view;\n"
		"uniform mat4 projection;\n"
		"out vec3 outCol; // going to the fragment shader\n"
		"out vec2 outTex; \n"
		"void main()\n"
		"{\n"
		"    gl_Position = projection * view * model * vec4(vPos, 1.0);\n"
		"    outCol = vCol;\n"
		"    outTex = vtex;\n"
		"}\n\0";

	static const char* fragment_shader_text =
		"#version 330 core\n"
		"in vec3 outCol;\n"
		"in vec2 outTex;\n"
		"out vec4 color;\n"
		"uniform sampler2D theTexture;\n"
		"void main()\n"
		"{\n"
		"	color = vec4(outCol, 1.0f);\n"
		"	color = texture(theTexture, outTex) * vec4(outCol, 1.0f);\n"
		"}\n\0";

	struct segProps{
		unsigned int id; // just so we can make sure we're talking to right segment, taken from seg.id
		unsigned int parentid; // allows us to place this in position relative to its parent end point
		unsigned int axis; // is this an axis (non-zero), dont texture it
		bool inUse; // is this segment in use?
		glm::vec3 colour; // contains the colours from the segment
		glm::vec3 beginpoint, endpoint; // contains the (updated! by shader?) endpoint (for translation of any child)
		glm::vec3 orient; // contains the orientation data (for rotations) of the segment
		glm::mat4 model; // records any translations/rotations applied to this matrix
		GLfloat* vertex_data; // vertex data (position, color and texture)
		unsigned int* index_data; // actual float data
		int numVertices, numIndices; // number of vertices and indices (not always equal!)

		float pitch, yaw, roll; // prob should be elsewhere
	};

	static float fov; // camera field of view

	class robotGL{
		private:
			GLFWwindow* window;
			GLuint programID;
			GLuint fragment_shader;
			GLuint vertex_shader;
			GLuint VAO, VBO, EBO; // handles for array, buffer and index data	
			unsigned int texture; // handle for texture data

			glm::vec3 cameraPos, cameraFront, cameraUp;
			// use these to compute user interaction (accessed by a static callback)
			float view_rotx, view_roty, view_rotz;
			bool firstMouse = true;
			float yaw, pitch, lastX, lastY;

			bool prog_finished;
			bool modeWireframe;
			float prevtime;
			float fps; // track how fast we're updating screen

			// each seg contains a unique robot segment
			segProps seg[MAX_NUM_SEGMENTS]; // allocate space for some segments

		public:
			robotGL(bool fullscreen = false);
			~robotGL();

			GLFWwindow* getWindow (void) const;
			int init(bool full = false);

			void updateScreen(void);
			void reset_view(void);
			void set_bg(const float r, const float g, const float b, const float a);
			inline bool get_wireframe(void) const {return modeWireframe;}
			void set_wireframe(const bool mode);
			void toggle_wireframe(void);

			glm::mat4 get_view() const;
			inline float get_fps(void){return this->fps;}
			inline bool get_progFinished(void){return this->prog_finished;}

			void create_cuboid(const robotSeg segment);
			void set_mat(const unsigned int id, const float* ,const unsigned int*, const int , const int);
			void set_segProps(const unsigned int id, const glm::vec3 col, const glm::vec3 endpoint, const glm::vec3 orient, const unsigned int parentID, const unsigned int axis);
			GLfloat* get_mat(const unsigned int id, int &size) const;
			unsigned int* get_ind(const unsigned int id, int &size) const;
			void unset_segProps(const unsigned int id);
			void set_segAngles(const unsigned int id, const float pitch, const float yaw, const float roll);
			void get_segAngles(const unsigned int id, float& pitch, float& yaw, float &roll) const;

			// callbacks
			static void glfw_resize_callback(GLFWwindow* window, int width, int height);
			static void glfw_error_callback(int error, const char* desc);
			void mouse_callback(GLFWwindow* window);
			static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

			void stop(void);
			// static void glfw_key_callback(GLFWwindow* window, int key, int scanmode, int action, int modifier);

			void process_inputs(void); 
	};

}

#endif
