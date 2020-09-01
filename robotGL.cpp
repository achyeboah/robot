#include "defs.h"
#include "robotGL.h"
// lets use Sean Barrett's texture loading code
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Include standard headers

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <iostream>

namespace samsRobot{

	robotGL::robotGL(bool do_fullscreen){
		prog_finished = false;
		set_wireframe(false);
		fps = 0;

		reset_view();
		// init segments
		for(int i = 0; i < MAX_NUM_SEGMENTS; i++){
			seg[i].vertex_data = NULL;
			seg[i].index_data = NULL;
			unset_segProps(i);
		}

		if(this->init(do_fullscreen) != 0)
			stop();
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
    		glfwWindowHint(GLFW_DEPTH_BITS, 16);
   		glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

		// Open a window and create its OpenGL context
		this->window = glfwCreateWindow(width, height, "Robot Window", monitor, NULL);
		if( window == NULL ){
			fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
			getchar();
			glfwTerminate();
			return -1;
		}
		// disable mouse for fullscreen
		if(monitor) glfwSetInputMode(this->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		// use the context
		glfwMakeContextCurrent(this->window);
		glfwSetFramebufferSizeCallback(this->window, this->glfw_resize_callback);
//		glfwSetKeyCallback(this->window, this->glfw_key_callback);

		// not convinced this is necessary?
		glfwGetFramebufferSize(this->window, &width, &height);
		glfw_resize_callback(this->window, width, height);

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
		glfwSetCursorPos(window, SCR_WIDTH/2, SCR_HEIGHT/2);

		// Dark blue background by default
		glClearColor(0.0f, 0.0f, 0.1f, 0.2f);

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
		if(!gl_success){
			fprintf(stderr, "RobotGL:INIT(). Compile VertShader program failed\n");
			char infoLog[512];
			glGetShaderInfoLog(vertex_shader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;	
		}

		this->fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(this->fragment_shader, 1, &fragment_shader_text, NULL);
		glCompileShader(this->fragment_shader);
		glGetShaderiv(this->fragment_shader, GL_COMPILE_STATUS, &gl_success);
		if(!gl_success){
			fprintf(stderr, "RobotGL:INIT(). Compile FragShader program failed\n");
			char infoLog[512];
		        glGetShaderInfoLog(fragment_shader, 512, NULL, infoLog);
        		std::cout << "ERROR::SHADER::FRAG::COMPILATION_FAILED\n" << infoLog << std::endl;	
		}
		this->programID = glCreateProgram();
		glAttachShader(this->programID, this->vertex_shader);
		glAttachShader(this->programID, this->fragment_shader);
		glLinkProgram(this->programID);
		// check for link errors
		glGetProgramiv(this->programID, GL_LINK_STATUS, &gl_success);
		if(!gl_success){
			fprintf(stderr, "RobotGL:INIT(). Link program failed\n");
			char infoLog[512];
		        glGetShaderInfoLog(programID, 512, NULL, infoLog);
        		std::cout << "ERROR::LINK::FRAG::COMPILATION_FAILED\n" << infoLog << std::endl;	
		}

		glDeleteShader(this->vertex_shader);
		glDeleteShader(this->fragment_shader);

		// bind our objects
		glGenVertexArrays(1, &(this->VAO));
		glGenBuffers(1, &(this->VBO));
		glGenBuffers(1, &(this->EBO));
		
		// load the texture
		glGenTextures(1, &this->texture);
		glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// load image, create texture and generate mipmaps
		int nrChannels;
		// stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
		unsigned char *data = stbi_load("resources/1.png", &width, &height, &nrChannels, 0);
		if (data){
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}else
			fprintf(stderr, "robotGL::init(): Failed to load texture!\n");
		stbi_image_free(data);

		int loc = glGetUniformLocation(this->programID, "theTexture");
		glUniform1i(loc, 0);

		// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
		// glBindBuffer(GL_ARRAY_BUFFER, 0); 
		// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
		// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
		// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
		// glBindVertexArray(0); 
		
		glfwSetTime(0.0f);

		return 0;
	}	

	void robotGL::set_mat(const unsigned int id, const float* vdata, const unsigned int* idata, const int numVertices, const int numIndices){
		// vertices. Three consecutive floats give a 3D vertex; Three consecutive vertices give a triangle.
		// A cube has 6 faces with 2 triangles each, so this makes 6*2=12 triangles, and 12*3=36 vertices, 3*36=108 floats
		// however we'll use indexing!

		unset_segProps(id);
		seg[id].vertex_data = new GLfloat[8*numVertices]; // 3 for position, 3 for colour, 2 for texture
		seg[id].index_data = new unsigned int[numIndices];

		if((seg[id].vertex_data == NULL) || (seg[id].index_data == NULL)){
			fprintf(stderr, "set_mat:: not enough memory\n");
			seg[id].numVertices = 0;
			seg[id].numIndices = 0;
			return;
		}
		int i = 0;
		for(i = 0; i < 8*numVertices; i++){
			seg[id].vertex_data[i] = (GLfloat)vdata[i];
		}
		seg[id].numVertices = numVertices;

		for(i = 0; i < numIndices; i++)
			seg[id].index_data[i] = idata[i];
		seg[id].numIndices = numIndices;

		seg[id].inUse = true;

	}

	// this function actually calls glDraw
	void robotGL::updateScreen(void){
		process_inputs();

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object

		// Use our shader
		glUseProgram(this->programID);

		// draw the elements
		glBindVertexArray(this->VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized

		// create MVP transformations
		glm::mat4 iprojection    = glm::mat4(1.0f);
		iprojection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		int loc = glGetUniformLocation(this->programID, "projection");
		glUniformMatrix4fv(loc, 1, GL_FALSE, &(iprojection[0][0]));

	        glm::mat4 view = glm::lookAt(this->cameraPos, this->cameraPos + this->cameraFront, this->cameraUp);
		loc = glGetUniformLocation(this->programID, "view");
		glUniformMatrix4fv(loc, 1, GL_FALSE, &(view[0][0]));

		// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
		glBindVertexArray(this->VAO);

		// load each object into VBO, colour and texture it, then draw it below
		// for each object, update shaders with its model matrix (ie transform it as appropriate)

		for (int i = 0; i < MAX_NUM_SEGMENTS; i++){
			if (this->seg[i].inUse == true){
				// lets make sure we're putting the data in the right place - ie before we load it into VBO
				glm::mat4 model = glm::mat4(1.0f);

				// just for jokes
				float rot_x_ax = sin(glfwGetTime());
				float rot_y_ax = cos(glfwGetTime());
				// model = glm::rotate(model, glm::radians(rot_x_ax), glm::vec3(1.0f, 0.0f, 0.0f)); // based on IMU info
				// model = glm::rotate(model, glm::radians(rot_y_ax), glm::vec3(0.0f, 1.0f, 0.0f)); // based on IMU info
				// model = glm::rotate(model, glm::radians(rot_z_ax), glm::vec3(0.0f, 0.0f, 1.0f)); // based on IMU info
				
				// capture the existing record of trans/rots in model
				// model = seg[i].model * model;
				// seg[i].model = model;

				// check if this has a parent (ie a non-zero parent id)
				// and translate it such that it is centred (for rotation) around the endpoint of the parent, else use the given centre
				// check its valid and check that parent has been loaded for use
				if((seg[i].parentid != 0) && (seg[seg[i].parentid].inUse == true)){
					glm::vec3 temp = seg[seg[i].parentid].endpoint + seg[seg[i].parentid].beginpoint;
					seg[i].beginpoint = temp;
					model = glm::translate(model, temp);
				}

				loc = glGetUniformLocation(this->programID, "model");
				// glUniformMatrix4fv(loc, 1, GL_FALSE, &(seg[i].model[0][0]));
				glUniformMatrix4fv(loc, 1, GL_FALSE, &model[0][0]);

				// load the data into the VBO

				glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
				glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float)*this->seg[i].numVertices, seg[i].vertex_data, GL_DYNAMIC_DRAW);

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * this->seg[i].numIndices, seg[i].index_data, GL_DYNAMIC_DRAW);
				glVertexAttribPointer(
						0, // index of the attribute
						3, // size: number of components per attribute (1 to 4 permitted)
						GL_FLOAT, // type
						GL_FALSE, // normalized/clamped to [-1 1]
						8 * sizeof(float), // stride: byte offset
						(void*)0 // pointer to first element in array
						);
				glEnableVertexAttribArray(0); // enable the attribute array
				// lets do colour
				glVertexAttribPointer(
						1, // index of the attribute
						3, // size: number of components per attribute (1 to 4 permitted)
						GL_FLOAT, // type
						GL_FALSE, // normalized/clamped to [-1 1]
						8 * sizeof(float), // stride: byte offset
						(void*)(3*sizeof(float)) // pointer to first element in array
						);
				glEnableVertexAttribArray(1); // enable the attribute array
				// lets do texture
				glVertexAttribPointer(
						2, // index of the attribute
						2, // size: number of components per attribute (1 to 4 permitted)
						GL_FLOAT, // type
						GL_FALSE, // normalized/clamped to [-1 1]
						8 * sizeof(float), // stride: byte offset
						(void*)(6*sizeof(float)) // pointer to first element in array
						);
				glEnableVertexAttribArray(2); // enable the attribute array

				glDrawElements(GL_TRIANGLES, this->seg[i].numIndices, GL_UNSIGNED_INT, 0);  
			}
		}

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	robotGL::~robotGL(){
		// cleanup vertex and index data
		// delete should automatically be called when segments go out of existence but we'll be explicit
		for (int i = 0; i < MAX_NUM_SEGMENTS; i++)
			unset_segProps(i);

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
		glfwSetWindowShouldClose(this->window, true);
		this->prog_finished = true;
	}

	void robotGL::set_bg(const float r, const float g, const float b, const float a){
		glClearColor(r, g, b, a);
	}

	GLfloat* robotGL::get_mat(const unsigned int id, int& size) const{
		size = seg[id].numVertices;
		return seg[id].vertex_data;
	}

	unsigned int* robotGL::get_ind(const unsigned int id, int& size) const{
		if((id < 0) || (id >= MAX_NUM_SEGMENTS))
		       return NULL;
		size = seg[id].numIndices;
		return seg[id].index_data;
	}

	void robotGL::reset_view(void){
		cameraPos   = glm::vec3(10.0f, -10.0f, 10.0f);
		cameraFront = glm::normalize(glm::vec3(0.0f,-1.0f,0.0f));
		cameraFront = glm::normalize(-cameraPos);
		cameraUp    = glm::vec3(0.0f, 0.0f, 1.0f);
		// glfw_resize_callback(this->window, SCR_WIDTH, SCR_HEIGHT);
	}

	void robotGL::create_cuboid(const robotSeg segment){
		// create a cuboid for the segment passed - using indexing
		// each cuboid has 6 faces, 2 triangles per face, 3 vertices per triangle (no indexing), 3 floats per vertex
		float x,y,z, cr,cg,cb;
		segment.get_dimensions(x, y, z);
		segment.get_colors(cr, cg, cb);

		/*
		 * 	   z
		 * 	   | 
		 *         e----f
		 *	  /    /|
		 *	 /    / |
		 *	c----d  g --> y
		 *	|    | /
		 *	|    |/
		 *	a----b 
		 *     /
		 *    x
		 *
		 */


		// lets define our cuboid faces use end quads bounded by vertices abcdefg
		// a(l,0,0), b(l,w,0), c(l,0,h), d(l,w,h)
		// h(0,0,0). g(0,w,0), e(0,0,h), f(0,w,h);
		glm::vec3 a, b, c, d, e, f, g, h;
		if(segment.get_axis() != 0){ 
			// this is not an axis, dont centre it
			a = glm::vec3(x/2,-y/2,-z/2); b = glm::vec3(x/2,y/2,-z/2); c = glm::vec3(x/2,-y/2,z/2); d = glm::vec3(x/2,y/2,z/2);
			h = glm::vec3(-x/2,-y/2,-z/2); g = glm::vec3(-x/2,y/2,-z/2); e = glm::vec3(-x/2,-y/2,z/2); f = glm::vec3(-x/2,y/2,z/2);
		}else{
			a = glm::vec3(x,0,0); b = glm::vec3(x,y,0); c = glm::vec3(x,0,z); d = glm::vec3(x,y,z);
			h = glm::vec3(0,0,0); g = glm::vec3(0,y,0); e = glm::vec3(0,0,z); f = glm::vec3(0,y,z);
		}
		

		// use an initializer list to fill values quickly, and presume memory allocated successfully;
		const float* vertices = new float[8*8]{
			// position		//color		// texture
			// vertices in near face
			a.x, a.y, a.z, 		cr, cg, cb, 	0.0f, 0.0f,
			b.x, b.y, b.z, 		cr, cg, cb,	1.0f, 0.0f,
			c.x, c.y, c.z, 		cr, cg, cb,	0.0f, 1.0f,
			d.x, d.y, d.z, 		cr, cg, cb,	1.0f, 1.0f, 
			// vertices in far face
			e.x, e.y, e.z,		cr, cg, cb,	1.0f, 1.0f,
			f.x, f.y, f.z,		cr, cg, cb,	0.0f, 1.0f,
			g.x, g.y, g.z, 		cr, cg, cb,	0.0f, 0.0f,
			h.x, h.y, h.z,		cr, cg, cb,	1.0f, 0.0f
		};
		const unsigned int* indices = new unsigned int[36]{
			0, 1, 2, 1, 3, 2, // near face
			2, 3, 4, 3, 5, 4, // top face
			6, 7, 5, 7, 4, 5, // far face
			0, 1, 7, 1, 6, 7, // bottom face
			1, 6, 3, 6, 5, 3, // near side face
			7, 0, 4, 0, 2, 4 // far side face
		};

		if((vertices == NULL) || (indices ==NULL)){
			fprintf(stderr,"robotGL::create_cuboid - could not allocate memory!\n");
			return;
		}

		set_mat(segment.getID(), vertices, indices, 8, 36);
		// set_segProps(segment.getID(), glm::vec3(cr, cg, cb), glm::vec3(x, y, z), glm::vec3(cr, cg, cb), segment.getParentID(), segment.get_axis());
		set_segProps(segment.getID(), glm::vec3(cr, cg, cb), glm::vec3(x, 0, 0), glm::vec3(cr, cg, cb), segment.getParentID(), segment.get_axis());

		// clean up
		delete vertices; vertices = NULL;
		delete indices; indices = NULL;
	}

	void robotGL::glfw_resize_callback(GLFWwindow* window, int width, int height){
		glViewport(0, 0, width, height);
		GLfloat h = (GLfloat) height / (GLfloat) width;
		GLfloat xmax, znear, zfar;

		znear = 5.0f;
		zfar  = 30.0f;
		xmax  = znear * 0.5f;

		glViewport( 0, 0, (GLint) width, (GLint) height );
		glMatrixMode( GL_PROJECTION );
		glLoadIdentity();
		glFrustum( -xmax, xmax, -xmax*h, xmax*h, znear, zfar );
		glMatrixMode( GL_MODELVIEW );
		glLoadIdentity();
		glTranslatef( 0.0, 0.0, -20.0 );

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

	void robotGL::set_segProps(const unsigned int id, const glm::vec3 col, const glm::vec3 endpoint, const glm::vec3 orient, const unsigned int parentID, const unsigned int axis){

		if((id < 0) || (id >= MAX_NUM_SEGMENTS)) return;

		seg[id].id = id; // should use a more clever unique id, really - this is redundant
		seg[id].inUse = true;
		seg[id].colour = col;
		seg[id].endpoint = endpoint;
		seg[id].orient = orient;
		seg[id].parentid = parentID;
		seg[id].axis = axis;
		seg[id].model = glm::mat4(1.0f);
	}

	void robotGL::unset_segProps(const unsigned int id){

		if((id < 0) || (id >= MAX_NUM_SEGMENTS)) return;

		seg[id].inUse = false;
		if (seg[id].vertex_data != NULL){
			delete seg[id].vertex_data;
			seg[id].vertex_data = NULL;
		}
		if (seg[id].index_data != NULL){
			delete seg[id].index_data;
			seg[id].index_data = NULL;
		}

		glm::vec3 temp(0.0f,0.0f,0.0f);
		set_segProps(id, temp, temp, temp, 0, 0);
		seg[id].model = glm::mat4(1.0f);
		seg[id].beginpoint = glm::vec3(0);
	}

	// void robotGL::glfw_key_callback(GLFWwindow* window, int key, int scanmode, int action, int modifier){
	void robotGL::process_inputs(void){

		// track time
		float currtime = glfwGetTime();
		float deltatime = currtime - this->prevtime;
		this->prevtime = currtime;
		if (deltatime < 0.0001)
			deltatime = 0.001;
		fps = 1.0f/deltatime;

		float cameraSpeed = 2.0f * deltatime;

		if (glfwGetKey(this->window, GLFW_KEY_Z) == GLFW_PRESS)
			toggle_wireframe();
		if((glfwGetKey(this->window, GLFW_KEY_ESCAPE) == GLFW_PRESS) || 
			glfwGetKey(this->window, GLFW_KEY_Q) == GLFW_PRESS)
			stop();
		if(glfwGetKey(this->window, GLFW_KEY_R) == GLFW_PRESS)
			reset_view();

		// keys below move the camera position, mouse affects lookat and zoom
		if (glfwGetKey(this->window, GLFW_KEY_UP) == GLFW_PRESS)
			this->cameraPos -= (this->cameraUp)*cameraSpeed;
		if (glfwGetKey(this->window, GLFW_KEY_DOWN) == GLFW_PRESS)
			this->cameraPos += (this->cameraUp)*cameraSpeed;
		if (glfwGetKey(this->window, GLFW_KEY_LEFT) == GLFW_PRESS)
			this->cameraPos += glm::normalize(glm::cross(this->cameraFront, this->cameraUp)) * cameraSpeed;
		if (glfwGetKey(this->window, GLFW_KEY_RIGHT) == GLFW_PRESS)
			this->cameraPos -= glm::normalize(glm::cross(this->cameraFront, this->cameraUp)) * cameraSpeed;
		if (glfwGetKey(this->window, GLFW_KEY_PAGE_UP) == GLFW_PRESS)
			this->cameraPos += cameraSpeed * cameraFront;
		if (glfwGetKey(this->window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS)
			this->cameraPos -= cameraSpeed * cameraFront;
	}
}// namespace
