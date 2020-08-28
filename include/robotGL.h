#ifndef ROBOTGL_H
#define ROBOTGL_H

#define ROBOTGL_INIT_POS glm::vec3( 0, 0, 10 ); 
#define ROBOTGL_INIT_FOV 45.0f
#define ROBOTGL_INIT_HANG 3.14f
#define ROBOTGL_INIT_VANG 0.0f

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
		"void main()\n"
		"{\n"
		"    gl_Position = vec4(vPos, 1.0);\n"
		"}\n\0";

	static const char* fragment_shader_text =
		"#version 330 core\n"
		"out vec4 color;\n"
		"void main()\n"
		"{\n"
		"	color = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
		"}\n\0";

	class robotGL{
		private:
			GLFWwindow* window;
			GLuint programID;
			GLuint fragment_shader;
			GLuint vertex_shader;
			GLuint VAO, VBO, EBO; // buffer data	

			// create the projection and view matrices
			GLuint matrixID;
			glm::mat4 proj;
			glm::mat4 view;
			glm::mat4 model;
			glm::mat4 MVP;

			GLfloat* vertex_data;
			unsigned int* index_data; // actual float data
			int numVertices, numIndices; // number of vertices and indices (not always equal!)

			bool prog_finished;
			bool modeWireframe;
			float mouse_wh_speed, keys_speed;
			float fps; // track how fast we're updating screen

			// use these to compute user interaction
			// Initial position : on +Z
			glm::vec3 position = ROBOTGL_INIT_POS; 
			// Initial horizontal angle : toward -Z
			float horizontalAngle = ROBOTGL_INIT_HANG;
			// Initial vertical angle : none
			float verticalAngle = ROBOTGL_INIT_VANG;
			// Initial Field of View
			float initialFoV = ROBOTGL_INIT_FOV;


		public:
			robotGL();
			~robotGL();

			GLFWwindow* getWindow (void) const;
			int init(void);
			void stop(void);
			void update(void);

			void set_mat(const float* , const unsigned int*, const int , const int);
			void set_view(const glm::vec3 cam_pos, const glm::vec3 look_at_dir);
			void reset_view(void);
			void set_proj(const float fov, const float asp_ratio);
			void set_bg(const float r, const float g, const float b, const float a);

			GLfloat* get_mat(int &size) const;
			unsigned int* get_ind(int &size) const;

			inline float get_keys_speed(void) const {return this->keys_speed;}
			inline float get_mouse_wh_speed(void) const { return this->mouse_wh_speed;}
			inline bool get_wireframe(void) const {return modeWireframe;}
			void set_wireframe(const bool mode);
			void toggle_wireframe(void);

			glm::mat4 get_view() const;
			inline float get_fps(void){return this->fps;}
			inline bool get_progFinished(void){return this->prog_finished;}

			// from tutorial 06
			void computeMatricesFromInputs(void);
			void create_cuboid(const robotSeg segment);

			// callbacks
			static void glfw_resize_callback(GLFWwindow* window, int width, int height);
			static void glfw_error_callback(int error, const char* desc);
			void process_inputs();
	};



}

#endif
