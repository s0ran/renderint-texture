// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <thread>
#include <time.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <tuple>

using namespace glm;
using namespace std;

#include <common/shader.hpp>
#include <common/texture.hpp>
//#include <common/controls.hpp>
//#include <common/objloader.hpp>
//#include <common/vboindexer.hpp>

int WIDTH=1024;
int HEIGHT=768;
float scaleValue=0.01;
float clipRange[]={0.1f,20.0f};
vec3 lookFrom=vec3(0,-1,10);
vec3 lookTo=vec3(0,-1,0);




string to_binary_string(unsigned char byte)
{
    std::bitset<sizeof(unsigned char) * CHAR_BIT> bs(byte);
    return bs.to_string();
}

bool loadPLY(const string filePath, vector< glm::vec3 >& vertices, vector< glm::vec2 >& vuvs, vector< glm::vec3 >& normals, vector< glm::vec2 >& fuvs, vector< glm::vec4 >& colors) {
	ifstream ifs(filePath, ios::in | ios::binary );
	vector< glm::vec3 > tmp_vertices;
	vector< glm::vec2 > tmp_uvs;
	string str;
	if (ifs.fail()) {
		cerr << "Failed to open file." << endl;
		return false;
	}
	cout << "Loading...\n";
	cout << "======== PLY HEADER ========\n";
	while (getline(ifs, str)) {
		cout << str << endl;
		if (str == "end_header") break;
	}
	cout << "============================\n";
	int count=0;
	while (!ifs.eof()) {
		float x,y,z,u,v;
		if (count>=10295){
			break;
		}
		count++;
        ifs.read( ( char * ) &x, sizeof( float ) );
        ifs.read( ( char * ) &y, sizeof( float ) );
        ifs.read( ( char * ) &z, sizeof( float ) );
        ifs.read( ( char * ) &u, sizeof( float ) );
        ifs.read( ( char * ) &v, sizeof( float ) );
        tmp_vertices.push_back(vec3(x,y,z));
        tmp_uvs.push_back(vec2(u,v));
	}
	count=0;
	while (!ifs.eof()) {
		unsigned char n;
		ifs.read( ( char * ) &n, sizeof( unsigned char ) );
		//cout << "n: " << to_binary_string(n) << ": " << (int) n << endl;
		vector<vec3> triangles;
		for (int i=0;i<n;i++){
			int vertexindex;
			ifs.read( ( char * ) &vertexindex, sizeof( int ) );
			vertices.push_back(tmp_vertices[vertexindex]);
			vuvs.push_back(tmp_uvs[vertexindex]);
			triangles.push_back(tmp_vertices[vertexindex]);
			//cout<<"Vi"<<vertexindex<<endl;
		}
		normals.push_back(cross(triangles[1]-triangles[0],triangles[2]-triangles[0]));
		normals.push_back(cross(triangles[1]-triangles[0],triangles[2]-triangles[0]));
		normals.push_back(cross(triangles[1]-triangles[0],triangles[2]-triangles[0]));
		ifs.read( ( char * ) &n, sizeof( unsigned char ) );
		//cout << "un"<<(int) n << endl;
		float u,v;
		for (int i=0;i<n;i++){
			if (i%2==0) ifs.read( ( char * ) &u, sizeof( float ) );
			else {
				ifs.read( ( char * ) &v, sizeof( float ) );
				fuvs.push_back(vec2(u,v));
			}
			//cout<<"u: "<<u<<"v: "<<v<<endl;
			//vuvs.push_back(tmp_uvs[uvindex]);
		}
		int r,g,b,a;
		ifs.read( ( char * ) &r, sizeof( unsigned char ) );
		ifs.read( ( char * ) &g, sizeof( unsigned char ) );
		ifs.read( ( char * ) &b, sizeof( unsigned char ) );
		ifs.read( ( char * ) &a, sizeof( unsigned char ) );
		colors.push_back(vec4(r/255,g/255,b/255,a/255));
		count++;
	}
	cout << "Completed.\n";
	ifs.close();
	return true;
}

bool initializeWindow(){
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		getchar();
		return false;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow( WIDTH, HEIGHT, "Rendering", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		getchar();
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return false;
	}

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwPollEvents();
	//glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); 
	glEnable(GL_CULL_FACE);
	return true;
}

void process_MVP(GLuint MatrixID,GLuint ViewMatrixID,GLuint ModelMatrixID){
	mat4 ProjectionMatrix = perspective(radians(45.0f), (float) WIDTH / HEIGHT, clipRange[0], clipRange[1]);
	mat4 ModelMatrix = scale(mat4(1), vec3( scaleValue, scaleValue, scaleValue ));
	mat4 ViewMatrix = lookAt(lookFrom, lookTo, vec3(0,1,0));
	mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
	glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
}

void process_RT(GLuint RTID,int sec){
	mat4 T=translate(mat4(), vec3((float) (sec*10%200)-100, 0.0f, 0.0f));
	mat4 RT=rotate(T,(float) (sec%6),vec3(0,1,0));
	glUniformMatrix4fv(RTID,1,GL_FALSE, &RT[0][0]);
}

void genBuffers(vector< vec3 >& vertices, vector< vec2 >& vuvs, vector<vec3>& normals, vector< vec2 >& fuvs, vector< vec4 >& colors,
	GLuint& vertexbuffer, GLuint& vuvbuffer, GLuint& normalbuffer, GLuint& fuvbuffer,  GLuint& colorbuffer){
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &vuvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vuvbuffer);
	glBufferData(GL_ARRAY_BUFFER, vuvs.size() * sizeof(glm::vec2), &vuvs[0], GL_STATIC_DRAW);

	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

	glGenBuffers(1, &fuvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, fuvbuffer);
	glBufferData(GL_ARRAY_BUFFER, fuvs.size() * sizeof(glm::vec2), &fuvs[0], GL_STATIC_DRAW);
	
	glGenBuffers(1, &colorbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec4), &colors[0], GL_STATIC_DRAW);
	
}

void draw(vector<vec3> vertices, GLuint& vertexbuffer, GLuint& vuvbuffer, GLuint& normalbuffer, GLuint& fuvbuffer, GLuint& colorbuffer){
	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(
		0,                  // attribute
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);
	// 2nd attribute buffer : vUVs
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, vuvbuffer);
	glVertexAttribPointer(
		1,                                // attribute
		2,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);
	
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glVertexAttribPointer(
		2,                                // attribute
		3,                                // size
		GL_FLOAT,                        // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);
	glEnableVertexAttribArray(3);
	glBindBuffer(GL_ARRAY_BUFFER, fuvbuffer);
	glVertexAttribPointer(
		3,                                // attribute
		2,                                // size
		GL_FLOAT,                        // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);
	
	glEnableVertexAttribArray(4);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	glVertexAttribPointer(
		4,                                // attribute
		4,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);
	
	// Draw the triangles !
	glDrawArrays(GL_TRIANGLES, 0, vertices.size() );
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
	glDisableVertexAttribArray(4);
}

int main( void )
{
	if (!initializeWindow()) return -1;


	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);
	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders( "StandardShading.vshader", "StandardShading.fshader" );
	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");
	GLuint RTID = glGetUniformLocation(programID, "RT");
	GLuint Texture = loadDDS("uvmap.DDS");
	GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");


	vector<vec3> vertices;
	vector<vec2> vuvs;
	vector<vec3> normals;
	vector<vec2> fuvs;
	vector<vec4> colors;
	bool res = loadPLY("object.ply", vertices, vuvs, normals, fuvs, colors);
	// Load it into a VBO
	GLuint vertexbuffer, vuvbuffer, normalbuffer,fuvbuffer, colorbuffer;
	genBuffers(vertices, vuvs, normals, fuvs, colors, vertexbuffer, vuvbuffer,normalbuffer, fuvbuffer, colorbuffer);


	// Get a handle for our "LightPosition" uniform
	glUseProgram(programID);
	GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
	process_MVP(MatrixID, ViewMatrixID, ModelMatrixID);
	glm::vec3 lightPos = glm::vec3(100,-100,300);
	glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, Texture);
	// Set our "myTextureSampler" sampler to use Texture Unit 0
	glUniform1i(TextureID, 0);
	do{
		// Clear the screen
		time_t now = time(NULL);
     	struct tm *pnow = localtime(&now);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glUseProgram(programID);
		process_RT(RTID,pnow->tm_sec);
		draw(vertices,vertexbuffer, vuvbuffer, normalbuffer, fuvbuffer, colorbuffer);
		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

		// opencv用
		// #include <opencv2/opencv.hpp>
		// #include <opencv2/highgui.hpp>
		// using namespace cv;
		//glReadBuffer(GL_FRONT);
		//Mat object = Mat(Size(WIDTH, HEIGHT), CV_8UC4);
		//glReadPixels(0, 0, WIDTH, HEIGHT, GL_BGRA_EXT, GL_UNSIGNED_BYTE, object.data);
		//flip(object, object, 0);
		//sendimageやらなんやら

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS && glfwWindowShouldClose(window) == 0 );

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &vuvbuffer);
	glDeleteBuffers(1, &normalbuffer);
	glDeleteBuffers(1, &fuvbuffer);
	glDeleteBuffers(1, &colorbuffer);

	glDeleteProgram(programID);
	glDeleteTextures(1, &Texture);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}
