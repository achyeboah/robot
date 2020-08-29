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

	robotGL::robotGL(bool do_fullscreen){
		prog_finished = false;
		keys_speed = 3.0f; // 3 units per second;
		mouse_wh_speed = 0.0f; //
		numVertices = 0;
		numIndices = 0;
		set_wireframe(false);
		if(this->init(do_fullscreen) != 0)
			prog_finished = true;
	}

	int robotGL::init(bool do_fullscreen ){
		int width, height;
		GLFWmonitor* monitor = NULL;

		// provide an error callback
		glfwSetErrorCallback(glfw_error_callback);	
		
		// Initialise GLFW
		if( !glfwInit() )
		{
			fprintf( stderr, "Failed to initialize GLFW\n" );
			getchar();
			return -1;
		}

		// process starting in fullscreen
		if(do_fullscreen == true)
			monitor = glfwGetPrimaryMonitor();
		if(monitor){
			const GLFWvidmode* mode = glfwGetVideoMode(monitor);

			glfwWindowHint(GLFW_RED_BITS, mode->redBits);
			glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
			glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
			glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

			width  = mode->width;
			height = mode->height;
		}else{
			width  = SCR_WIDTH; height = SCR_HEIGHT;
		}

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		// Open a window and create its OpenGL context
		this->window = glfwCreateWindow(width, height, "Robot Window", monitor, NULL);
		if( window == NULL ){
			fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
			getchar();
			glfwTerminate();
			return -1;
		}
		// disable mouse for fullscreen
		if(monitor) glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		// use the context
		glfwMakeContextCurrent(this->window);
		glfwSetFramebufferSizeCallback(window, this->glfw_resize_callback);
		glfwSwapInterval(1);

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
		// Set the mouse at the center of the screen
		glfwPollEvents();
		glfwSetCursorPos(window, 800/2, 600/2);

		// Dark blue background by default
		glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

		// Enable depth test
		glEnable(GL_DEPTH_TEST);
		// Accept fragment if it closer to the camera than the former one
		glDepthFunc(GL_LESS); 

		// Cull triangles which normal is not towards the camera
	//	glEnable(GL_CULL_FACE);

		int gl_success;
		// Create and compile our GLSL program from the shaders
		this->vertex_shader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(this->vertex_shader, 1, &vertex_shader_text, NULL);
		glCompileShader(this->vertex_shader);
		glGetShaderiv(this->vertex_shader, GL_COMPILE_STATUS, &gl_success);
		if(!gl_success){fprintf(stderr, "RobotGL:INIT(). Compile VertShader program failed\n");}

		this->fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(this->fragment_shader, 1, &fragment_shader_text, NULL);
		glCompileShader(this->fragment_shader);
		glGetShaderiv(this->fragment_shader, GL_COMPILE_STATUS, &gl_success);
		if(!gl_success){fprintf(stderr, "RobotGL:INIT(). Compile FragShader program failed\n");}

		this->programID = glCreateProgram();
		glAttachShader(this->programID, this->vertex_shader);
		glAttachShader(this->programID, this->fragment_shader);
		glLinkProgram(this->programID);
		// check for link errors
		glGetProgramiv(this->programID, GL_LINK_STATUS, &gl_success);
		if(!gl_success){fprintf(stderr, "RobotGL:INIT(). Link program failed\n");}

		glDeleteShader(this->vertex_shader);
		glDeleteShader(this->fragment_shader);

		// lets fill our matrices with some prelim data
		delete this->vertex_data;
		delete this->index_data;

		this->vertex_data = new GLfloat[12]{
			// position	
			0.5f,  0.5f, 0.0f, // top right
			0.5f, -0.5f, 0.0f, // bottom right
			-0.5f, -0.5f, 0.0f, // bottom left
			-0.5f,  0.5f, 0.0f, // top left
		};
		this->index_data = new unsigned int[6]{  // note that we start from 0!
			0, 1, 3,  // first Triangle
			1, 2, 3   // second Triangle
		};
		seg[1].colour = glm::vec3(1,0,0);
		this->numVertices = 4;
		this->numIndices = 6;

		// bind our objects
		glGenVertexArrays(1, &(this->VAO));
		glGenBuffers(1, &(this->VBO));
		glGenBuffers(1, &(this->EBO));
		
		updateBuffers();

		// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
		glBindBuffer(GL_ARRAY_BUFFER, 0); 
		// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
		// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		// Get a handle for our "MVP" uniform
		reset_view(); // this generates the model, view and proj matrices
		this->matrixID = glGetUniformLocation(programID, "MVP");

		// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
		// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
		// glBindVertexArray(0); 
		
		set_bg(1.0f, 0.3f, 0.3f, 1.0f);

		return 0;
	}	

	void robotGL::set_mat(const float* vdata, const unsigned int* idata, const int numVertices, const int numIndices){
		// vertices. Three consecutive floats give a 3D vertex; Three consecutive vertices give a triangle.
		// A cube has 6 faces with 2 triangles each, so this makes 6*2=12 triangles, and 12*3=36 vertices, 3*36=108 floats
		// however we'll use indexing!

		delete this->vertex_data;
		delete this->index_data;
		this->vertex_data = new GLfloat[3*numVertices];
		this->index_data = new unsigned int[numIndices];

		if((this->vertex_data == NULL) || (this->index_data == NULL)){
			fprintf(stderr, "set_mat:: not enough memory\n");
			this->numVertices = 0;
			this->numIndices = 0;
			return;
		}
		int i = 0;
		for(i = 0; i < 3*numVertices; i++){
			this->vertex_data[i] = (GLfloat)vdata[i];
		}
		this->numVertices = numVertices;

		for(i = 0; i < numIndices; i++)
			this->index_data[i] = idata[i];
		this->numIndices = numIndices;

		// update buffers with new data
		updateBuffers();


	}

	// this function actually calls glDraw
	void robotGL::update(void){
		// first process inputs
		// ----------------------
		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs();
		this->MVP = this->proj * this->view * this->model;

		// Send our transformation to the currently bound shader
		// in the "MVP" uniform
		glUniformMatrix4fv(this->matrixID, 1, GL_FALSE, &(this->MVP[0][0]));
		//
		// now draw the screen
		// -------------------

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(this->programID);

		// draw the elements
        	glBindVertexArray(this->VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
        	glDrawElements(GL_TRIANGLES, this->numIndices, GL_UNSIGNED_INT, 0);  

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	robotGL::~robotGL(){
		// cleanup vertex and index data
		delete this->vertex_data;
		delete this->index_data;

		// release the vertex array
		glBindVertexArray(0); 

		// Cleanup VBO and shader
		glDeleteVertexArrays(1, &(this->VAO));
		glDeleteBuffers(1, &(this->VBO));
		glDeleteBuffers(1, &(this->EBO));
		glDeleteProgram(programID);

		// Close OpenGL window and terminate GLFW
		glfwTerminate();
	}

	void robotGL::stop(void){
		prog_finished = true;
	}

	void robotGL::set_bg(const float r, const float g, const float b, const float a){
		glClearColor(r, g, b, a);
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
		return vertex_data;
	}

	unsigned int* robotGL::get_ind(int& size) const{
		size = this->numIndices;
		return index_data;
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

		// lets process inputs here - just because..

		if (glfwGetKey(this->window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
			glfwSetWindowShouldClose(window, true);
			prog_finished = true;
		}
		if (glfwGetKey(this->window, GLFW_KEY_Q ) == GLFW_PRESS){
			prog_finished = true;
		}
		if(glfwGetKey(this->window, GLFW_KEY_R) == GLFW_PRESS){
			reset_view();
		}
		if(glfwGetKey(this->window, GLFW_KEY_Z) == GLFW_PRESS){
			toggle_wireframe();
		}

		// below is for camera/view control
		// move up
		if (glfwGetKey(this->window, GLFW_KEY_PAGE_UP ) == GLFW_PRESS){
			position += direction * deltaTime * keys_speed;
		}
		// Move down 
		if (glfwGetKey( this->window, GLFW_KEY_PAGE_DOWN ) == GLFW_PRESS){
			position -= direction * deltaTime * keys_speed;
		}

		// Move forward
		if (glfwGetKey(this->window, GLFW_KEY_UP ) == GLFW_PRESS){
			position += direction * deltaTime * keys_speed; // correct this later
		}
		// Move backward
		if (glfwGetKey( this->window, GLFW_KEY_DOWN ) == GLFW_PRESS){
			position -= direction * deltaTime * keys_speed; // correct this later
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
		if (deltaTime > 0.000001f)
			this->fps = 1.0f/deltaTime;
		else
			this->fps = 1000.0f;
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

	}

	void robotGL::create_cuboid(const robotSeg segment){
		// create a cuboid for the segment passed - using indexing
		// each cuboid has 6 faces, 2 triangles per face, 3 vertices per triangle (no indexing), 3 floats per vertex
		float x,y,z,cr,cg,cb,cx,cy,cz;
		segment.get_dimensions(x, y, z);
		segment.get_colors(cr, cg, cb);
		segment.get_centre(cx, cy, cz);
		
		// lets define our cuboid faces use end quads bounded by vertices abcdefg
		// a(l,0,0), b(l,w,0), c(l,0,h), d(l,w,h)
		// h(0,0,0). g(0,w,0), e(0,0,h), f(0,w,h);
		struct myvec3 {float x; float y; float z;};
		myvec3 a{.x=x,.y=0,.z=0}, b{.x=x,.y=y,.z=0}, c{.x=x,.y=0,.z=z}, d{.x=x,.y=y,.z=z};
		myvec3 h{.x=0,.y=0,.z=0}, g{.x=0,.y=y,.z=0}, e{.x=0,.y=0,.z=z}, f{.x=0,.y=y,.z=z};

		// use an initializer list to fill values quickly, and presume successful;
		const float* vertices = new float[3*8]{
			// vertices in near face
			a.x, a.y, a.z, b.x, b.y, b.z, c.x, c.y, c.z, d.x, d.y, d.z, 
			// vertices in far face
			e.x, e.y, e.z, f.x, f.y, f.z, g.x, g.y, g.z, h.x, h.y, h.z 
		};
		const unsigned int* indices = new unsigned int[36]{
			0, 1, 2, 1, 2, 3, // near face
			2, 3, 4, 3, 4, 5, // top face
			4, 5, 7, 5, 6, 7, // far face
			6, 7, 1, 7, 1, 0, // bottom face
			1, 3, 5, 1, 5, 6, // near side face
			0, 2, 7, 2, 7, 5
		};

		if((vertices == NULL) || (indices ==NULL)){
			fprintf(stderr,"robotGL::create_cuboid - could not allocate memory!\n");
			return;
		}

		set_mat(vertices, indices, 8, 36);
		set_segProps(segment.getID(), glm::vec3(cr, cg, cb), glm::vec3(cx, cy, cz), glm::vec3(cr, cg, cb));
	}

	void robotGL::glfw_resize_callback(GLFWwindow* window, int width, int height){
		glViewport(0, 0, width, height);
	}


	void robotGL::process_inputs(void){
		// do nothing for now;
	}

	void robotGL::glfw_error_callback(int error, const char* desc){
		fprintf(stderr, "Error from robotGL: %s\n", desc);
	}


	void robotGL::toggle_wireframe(void){
		set_wireframe(!get_wireframe());
	}

	void robotGL::set_wireframe(const bool mode){
		this->modeWireframe = mode;
		if(this->modeWireframe == true)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	void robotGL::updateBuffers(void){
		// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
		glBindVertexArray(this->VAO);
		glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
		glBufferData(GL_ARRAY_BUFFER, 6*sizeof(float)*this->numVertices, this->vertex_data, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * numIndices, index_data, GL_STATIC_DRAW);
		glVertexAttribPointer(
				0, // index of the attribute
				3, // size: number of components per attribute (1 to 4 permitted)
				GL_FLOAT, // type
				GL_FALSE, // normalized/clamped to [-1 1]
				3 * sizeof(float), // stride: byte offset
				(void*)0 // pointer to first element in array
				);
		glEnableVertexAttribArray(0); // enable the attribute array

		int colorLoc = glGetUniformLocation(this->programID, "inCol");
		glUseProgram(this->programID);
		for(unsigned int i = 0; i < MAX_NUM_SEGMENTS; i++){
			glUniform3f(colorLoc, seg[1].colour.x, seg[1].colour.y, seg[1].colour.z);
		}
	}

	void robotGL::set_segProps(const unsigned int id, const glm::vec3 col, const glm::vec3 centre, const glm::vec3 orient){

		if(id > MAX_NUM_SEGMENTS) return;

		seg[id].id = id; // should use a more clever unique id, really - this is redundant
		seg[id].inUse = true;
		seg[id].colour = col;
		seg[id].centre = centre;
		seg[id].orient = orient;
	}

	void robotGL::unset_segProps(const unsigned int id){

		if(id > MAX_NUM_SEGMENTS) return;

		seg[id].id = 0; // should use a more clever unique id, really - this is redundant
		seg[id].inUse = false;
		seg[id].colour = glm::vec3(0,0,0);
		seg[id].centre = glm::vec3(0,0,0);
		seg[id].orient = glm::vec3(0,0,0);
	}

	unsigned int robotGL::getNumValidSegs(void){
		int x = 0;
		for (int i = 0; i < MAX_NUM_SEGMENTS; i++){
			if(seg[i].inUse == true)
				x++;
		}
		return x;
	}
}// namespace
