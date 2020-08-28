#include "defs.h"
#include "robotGL.h"

// Include standard headers

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

namespace samsRobot{

	robotGL::robotGL(){
		prog_finished = false;
		keys_speed = 3.0f; // 3 units per second;
		mouse_wh_speed = 0.0f; //
		numVertices = 0;
		numIndices = 0;
		this->init();
	}

	int robotGL::init( void ){
		// Initialise GLFW
		if( !glfwInit() )
		{
			fprintf( stderr, "Failed to initialize GLFW\n" );
			getchar();
			return -1;
		}


		glfwWindowHint(GLFW_SAMPLES, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		// Open a window and create its OpenGL context
		this->window = glfwCreateWindow( 800,600, "Robot Window", NULL, NULL);
		if( window == NULL ){
			fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
			getchar();
			glfwTerminate();
			return -1;
		}
		glfwMakeContextCurrent(this->window);
		glfwSetFramebufferSizeCallback(window, this->glfw_resize_callback);

		// Initialize GLEW
		glewExperimental = true; // Needed for core profile
		if (glewInit() != GLEW_OK) {
			fprintf(stderr, "Failed to initialize GLEW\n");
			getchar();
			glfwTerminate();
			return -1;
		}

		// Ensure we can capture the escape key being pressed below
		glfwSetInputMode(this->window, GLFW_STICKY_KEYS, GL_TRUE);
		// Hide the mouse and enable unlimited mouvement
		// glfwSetInputMode(this->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		// Set the mouse at the center of the screen
		glfwPollEvents();
		glfwSetCursorPos(window, 800/2, 600/2);

		// Dark blue background
		glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

		// Enable depth test
		glEnable(GL_DEPTH_TEST);
		// Accept fragment if it closer to the camera than the former one
		glDepthFunc(GL_LESS); 

		// Cull triangles which normal is not towards the camera
		glEnable(GL_CULL_FACE);

		glGenVertexArrays(1, &(this->vertexArrayID));
		glBindVertexArray(this->vertexArrayID);

		// Create and compile our GLSL program from the shaders
		this->vertex_shader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
		glCompileShader(vertex_shader);

		this->fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
		glCompileShader(fragment_shader);

		this->programID = glCreateProgram();
		glAttachShader(programID, vertex_shader);
		glAttachShader(programID, fragment_shader);
		glLinkProgram(programID);

		// Get a handle for our "MVP" uniform
		this->matrixID = glGetUniformLocation(programID, "MVP");

		// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
		this->proj = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
		// Camera matrix
		this->view       = glm::lookAt(
				glm::vec3(4,3,-3), // Camera is at (4,3,-3), in World Space
				glm::vec3(0,0,0), // and looks at the origin
				glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
				);
		// Model matrix : an identity matrix (model will be at the origin)
		this->model      = glm::mat4(1.0f);
		// Our ModelViewProjection : multiplication of our 3 matrices
		this->MVP        = (this->proj) * (this->view) * (this->model); // Remember, matrix multiplication is the other way around
		// if we're here all is good
		return 0;
	}	

	void robotGL::set_mat(const float* data, const int numVertices){
		// vertices. Three consecutive floats give a 3D vertex; Three consecutive vertices give a triangle.
		// A cube has 6 faces with 2 triangles each, so this makes 6*2=12 triangles, and 12*3=36 vertices, 3*36=108 floats
		delete this->g_vertex_buffer_data;
		this->g_vertex_buffer_data = new GLfloat[3*numVertices];
		if(this->g_vertex_buffer_data == NULL){
			fprintf(stderr, "set_mat:: not enough memory\n");
			return;
		}
		for(int i = 0; i < 3*numVertices; i++)
			g_vertex_buffer_data[i] = (GLfloat)data[i];
		this->numVertices = numVertices;

		glGenBuffers(1, &(this->vertexbuffer));
		glBindBuffer(GL_ARRAY_BUFFER, this->vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, 3*sizeof(GLfloat)*this->numVertices, this->g_vertex_buffer_data, GL_STATIC_DRAW);

		glGenBuffers(1, &(this->colorbuffer));
		glBindBuffer(GL_ARRAY_BUFFER, this->colorbuffer);
		glBufferData(GL_ARRAY_BUFFER, 3*sizeof(GLfloat)*this->numVertices, this->g_color_buffer_data, GL_STATIC_DRAW);
	}

	void robotGL::set_mat2(const float* vdata, const float* idata, const int numVertices){
		// vertices. Three consecutive floats give a 3D vertex; Three consecutive vertices give a triangle.
		// A cube has 6 faces with 2 triangles each, so this makes 6*2=12 triangles, and 12*3=36 vertices, 3*36=108 floats
		// however we'll use indexing!
		delete this->g_vertex_buffer_data;
		delete this->g_index_buffer_data;
		this->g_vertex_buffer_data = new GLfloat[3*numVertices];
		this->g_index_buffer_data = new GLfloat[numIndices];

		if((this->g_vertex_buffer_data == NULL) || (this->g_index_buffer_data == NULL)){
			fprintf(stderr, "set_mat2:: not enough memory\n");
			this->numVertices = 0;
			this->numIndices = 0;
			return;
		}
		for(int i = 0; i < 3*numVertices; i++)
			g_vertex_buffer_data[i] = (GLfloat)vdata[i];
		this->numVertices = numVertices;

		for(i = 0; i < numIndices; i++)
			this->g_index_buffer_data[i] = (GLfloat)vdata[i];
		this->numIndices = numIndices;

		glGenBuffers(1, &(this->vertexbuffer));
		glBindBuffer(GL_ARRAY_BUFFER, this->vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, 3*sizeof(GLfloat)*this->numVertices, this->g_vertex_buffer_data, GL_STATIC_DRAW);

		glGenBuffers(1, &(this->colorbuffer));
		glBindBuffer(GL_ARRAY_BUFFER, this->colorbuffer);
		glBufferData(GL_ARRAY_BUFFER, 3*sizeof(GLfloat)*this->numVertices, this->g_color_buffer_data, GL_STATIC_DRAW);
	}


	void robotGL::set_col(const float* data, const int numVertices){
		// vertices. Three consecutive floats give a 3D vertex; Three consecutive vertices give a triangle.
		// A cube has 6 faces with 2 triangles each, so this makes 6*2=12 triangles, and 12*3=36 vertices, 3*36=108 floats
		// One color for each vertex. They were generated randomly.
		delete this->g_color_buffer_data;
		this->g_color_buffer_data = new GLfloat[3*numVertices];
		if(this->g_color_buffer_data == NULL){
			fprintf(stderr, "set_col:: not enough memory\n");
			return;
		}
		for(int i = 0; i < 3*numVertices; i++)
			this->g_color_buffer_data[i] = (GLfloat)data[i];
		this->numVertices = numVertices;

		glGenBuffers(1, &(this->vertexbuffer));
		glBindBuffer(GL_ARRAY_BUFFER, this->vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, 3*sizeof(GLfloat)*this->numVertices, this->g_vertex_buffer_data, GL_STATIC_DRAW);

		glGenBuffers(1, &(this->colorbuffer));
		glBindBuffer(GL_ARRAY_BUFFER, this->colorbuffer);
		glBufferData(GL_ARRAY_BUFFER, 3*sizeof(GLfloat)*this->numVertices, this->g_color_buffer_data, GL_STATIC_DRAW);
	}

	void robotGL::update(void){

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(this->programID);

		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs();
		this->MVP = this->proj * this->view * this->model;

		// Send our transformation to the currently bound shader
		// in the "MVP" uniform
		glUniformMatrix4fv(this->matrixID, 1, GL_FALSE, &(this->MVP[0][0]));

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
				0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
				3,                  // size
				GL_FLOAT,           // type
				GL_FALSE,           // normalized?
				0,                  // stride
				(void*)0            // array buffer offset
				);

		// 2nd attribute buffer : colors
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
		glVertexAttribPointer(
				1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
				3,                                // size
				GL_FLOAT,                         // type
				GL_FALSE,                         // normalized?
				0,                                // stride
				(void*)0                          // array buffer offset
				);

		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0,3*this->numVertices); // 12*3 indices starting at 0 -> 12 triangles

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

		if (glfwGetKey(this->window, GLFW_KEY_Q ) == GLFW_PRESS){
			prog_finished = true;
		}
		if(glfwGetKey(this->window, GLFW_KEY_R) == GLFW_PRESS){
			reset_view();
		}
	}

	robotGL::~robotGL(){
		// cleanup vertex and color data
		delete g_vertex_buffer_data;
		delete g_color_buffer_data;

		// Cleanup VBO and shader
		glDeleteBuffers(1, &vertexbuffer);
		glDeleteBuffers(1, &colorbuffer);
		glDeleteProgram(programID);
		glDeleteVertexArrays(1, &(this->vertexArrayID));

		// Close OpenGL window and terminate GLFW
		glfwTerminate();
	}

	void robotGL::stop(void){
		prog_finished = true;
	}

	void robotGL::set_bg(const float r, const float g, const float b, const float a){
		glClearColor(r, g, b, a);
		fprintf(stderr, "r = %f\n", r);
	}

	void robotGL::set_view(const glm::vec3 cam_pos, const glm::vec3 look_at_dir){
		this->view       = glm::lookAt(cam_pos, look_at_dir, glm::vec3(0,1,0));
		// Our ModelViewProjection : multiplication of our 3 matrices
		this->MVP        = (this->proj) * (this->view) * (this->model); // Remember, matrix multiplication is the other way around
	}

	void robotGL::set_proj(const float fov, const float asp_ratio){
		this->proj = glm::perspective(glm::radians(fov), asp_ratio, 0.1f, 100.0f);
		this->MVP        = (this->proj) * (this->view) * (this->model); // Remember, matrix multiplication is the other way around
	}

	GLfloat* robotGL::get_mat(int& size) const{
		size = this->numVertices;
		return g_vertex_buffer_data;
	}

	GLfloat* robotGL::get_col(int& size) const{
		size = this->numVertices;
		return g_color_buffer_data;
	}

	glm::mat4 robotGL::get_view() const {
		return this->view;
	}
	

	void robotGL::computeMatricesFromInputs(){

		// glfwGetTime is called only once, the first time this function is called
		static double lastTime = glfwGetTime();

		// Compute time difference between current and last frame
		double currentTime = glfwGetTime();
		float deltaTime = float(currentTime - lastTime);

		// Get mouse position
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		// Reset mouse position for next frame
		glfwSetCursorPos(window, 800/2, 600/2);

		// Compute new orientation
		horizontalAngle += mouse_wh_speed * float(800/2 - xpos );
		verticalAngle   += mouse_wh_speed * float( 600/2 - ypos );

		// Direction : Spherical coordinates to Cartesian coordinates conversion
		glm::vec3 direction(
				cos(verticalAngle) * sin(horizontalAngle), 
				sin(verticalAngle),
				cos(verticalAngle) * cos(horizontalAngle)
				);

		// Right vector
		glm::vec3 right = glm::vec3(
				sin(horizontalAngle - 3.14f/2.0f), 
				0,
				cos(horizontalAngle - 3.14f/2.0f)
				);

		// Up vector
		glm::vec3 up = glm::cross( right, direction );

		// Move forward
		if (glfwGetKey(this->window, GLFW_KEY_UP ) == GLFW_PRESS){
			position += direction * deltaTime * keys_speed;
		}
		// Move backward
		if (glfwGetKey( this->window, GLFW_KEY_DOWN ) == GLFW_PRESS){position -= direction * deltaTime * keys_speed;
		}
		// Strafe right
		if (glfwGetKey(this->window, GLFW_KEY_RIGHT ) == GLFW_PRESS){
			position += right * deltaTime * keys_speed;
		}
		// Strafe left
		if (glfwGetKey(this->window, GLFW_KEY_LEFT ) == GLFW_PRESS){
			position -= right * deltaTime * keys_speed;
		}

		float FoV = initialFoV;// - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.

		// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
		this->proj = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, 0.1f, 100.0f);
		// Camera matrix
		this->view       = glm::lookAt(
				position,           // Camera is here
				position+direction, // and looks here : at the same position, plus "direction"
				up                  // Head is up (set to 0,-1,0 to look upside-down)
				);

		// For the next frame, the "last time" will be "now"
		this->fps = 1.0f/deltaTime;
		lastTime = currentTime;
	}

	void robotGL::reset_view(void){
		// Initial position : on +Z
		glm::vec3 position = ROBOTGL_INIT_POS; 
		// Initial horizontal angle : toward -Z
		float horizontalAngle = ROBOTGL_INIT_HANG;
		// Initial vertical angle : none
		float verticalAngle = ROBOTGL_INIT_VANG;
		// Initial Field of View
		float initialFoV = ROBOTGL_INIT_FOV;
	}

	void robotGL::create_cuboid(const robotSeg segment){
		// create a cuboid for the segment passed
		// each cuboid has 6 faces, 2 triangles per face, 3 vertices per triangle (no indexing), 3 floats per vertex
		float x,y,z,cx,cy,cz;
		segment.get_dimensions(x,y,z);
		segment.get_centre(cx, cy, cz);
		
		// lets define our cuboid faces use end quads bounded by vertices abcdefg
		// a(l,0,0), b(l,w,0), c(l,0,h), d(l,w,h)
		// h(0,0,0). g(0,w,0), e(0,0,h), f(0,w,h);
		struct myvec3 {float x; float y; float z;};
		myvec3 a{.x=x,.y=0,.z=0}, b{.x=x,.y=y,.z=0}, c{.x=x,.y=0,.z=z}, d{.x=x,.y=y,.z=z};
		myvec3 h{.x=0,.y=0,.z=0}, g{.x=0,.y=y,.z=0}, e{.x=0,.y=0,.z=z}, f{.x=0,.y=y,.z=z};

		// use an initializer list to fill values quickly, and presume successful;
		const float* cube  = new float[3*3*2*6]{ 
			a.x, a.y, a.z, b.x, b.y, b.z, c.x, c.y, c.z,
			b.x, b.y, b.z, c.x, c.y, c.z, d.x, d.y, d.z,

			c.x, c.y, c.z, d.x, d.y, d.z, e.x, e.y, e.z,
			d.x, d.y, d.z, e.x, e.y, e.z, f.x, f.y, f.z,

			e.x, e.y, e.z, f.x, f.y, f.z, h.x, h.y, h.z,
			f.x, f.y, f.z, g.x, g.y, g.z, h.x, h.y, h.z,

			g.x, g.y, g.z, h.x, h.y, h.z, a.x, a.y, a.z,
			g.x, g.y, g.z, a.x, a.y, a.z, b.x, b.y, b.z,

			d.x, d.y, d.z, f.x, f.y, f.z, g.x, g.y, g.z,
			d.x, d.y, d.z, g.x, g.y, g.z, b.x, b.y, b.z,

			c.x, c.y, c.z, e.x, e.y, e.z, h.x, h.y, h.z,
			c.x, c.y, c.z, h.x, h.y, h.z, a.x, a.y, a.z
		};
		const float* vertices = new float[3*8]{
			a.x, a.y, a.z, b.x, b.y, b.z, c.x, c.y, c.z, d.x, d.y, d.z, // vertices in near face
			e.x, e.y, e.z, f.x, f.y, f.z, g.x, g.y, g.z, h.x, h.y, h.z // vertices in far face
		};

		set_mat(cube, 3*2*6);
	}

	void robotGL::create_cuboid2(const robotSeg segment){
		// create a cuboid for the segment passed - using indexing
		// each cuboid has 6 faces, 2 triangles per face, 3 vertices per triangle (no indexing), 3 floats per vertex
		float x,y,z,cx,cy,cz;
		segment.get_dimensions(x,y,z);
		segment.get_centre(cx, cy, cz);
		
		// lets define our cuboid faces use end quads bounded by vertices abcdefg
		// a(l,0,0), b(l,w,0), c(l,0,h), d(l,w,h)
		// h(0,0,0). g(0,w,0), e(0,0,h), f(0,w,h);
		struct myvec3 {float x; float y; float z;};
		myvec3 a{.x=x,.y=0,.z=0}, b{.x=x,.y=y,.z=0}, c{.x=x,.y=0,.z=z}, d{.x=x,.y=y,.z=z};
		myvec3 h{.x=0,.y=0,.z=0}, g{.x=0,.y=y,.z=0}, e{.x=0,.y=0,.z=z}, f{.x=0,.y=y,.z=z};

		// use an initializer list to fill values quickly, and presume successful;
		const float* vertices = new float[3*8]{
			a.x, a.y, a.z, b.x, b.y, b.z, c.x, c.y, c.z, d.x, d.y, d.z, // vertices in near face
			e.x, e.y, e.z, f.x, f.y, f.z, g.x, g.y, g.z, h.x, h.y, h.z // vertices in far face
		};
		const float* indices = new float[]{
			0, 1, 2, 1, 2, 3, // near face
			2, 3, 4, 3, 4, 5, // top face
			4, 5, 7, 5, 6, 7, // far face
			6, 7, 1, 7, 1, 0, // bottom face
			1, 3, 5, 1, 5, 6, // near side face
			0, 2, 7, 2, 7, 5
		};

		if((vertices == NULL) || (indices ==NULL))
			return;

		set_mat2(vertices, indices, 8, 36);
	}

	void robotGL::glfw_resize_callback(GLFWwindow* window, int width, int height){
		glViewport(0, 0, width, height);
	}

}// namespace
