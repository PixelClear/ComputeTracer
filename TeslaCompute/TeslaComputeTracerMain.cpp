#include "TeslaComputeTracerMain.h"
#include <iostream>


GLuint render_vbo;
GLuint render_vao;
GLuint render_obj_vs;
GLuint render_obj_fs;
GLuint render_prog;
GLuint tex_buffer;

GLuint IBO;
GLint texture_location;

bool key_status = false;

 GLfloat vertexData[] = {
        1.0f,  -1.0f,  0.0f,
        1.0f, 1.0f, 0.0f ,
		-1.0f, -1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f
    }; // 4 ve

 GLfloat texData[] = {
        1.0f,  0.0f,
        1.0f, 1.0f,
		0.0f, 0.0f,
		0.0f, 1.0f
    }; // 

 static const GLushort vertex_indices[] =
{
    0,1,2,2,3,1
};

 const char* vertex_source[] ={
	    "#version 330\n"
        "layout (location = 0) in vec3 vposition;\n"
		"layout (location = 1) in vec2 TexCoord;\n"
		"out vec2 TexCoord0;\n"
		 "void main() {\n"
		 "TexCoord0     = TexCoord;\n"
		 "gl_Position = vec4(vposition,1.0);\n"
		"}\n"};
        
   const char* fragment_source[] ={
	   "#version 330\n"
	   "in vec2 TexCoord0;\n"
	   "uniform sampler2D tex;\n"
        "void main() {\n"
		"   gl_FragColor =texture(tex,TexCoord0);\n"
		"}\n"};

static int dispatch_once = 0;

void PrepareComputeTextures()
{   
	
	glGenTextures(1, &out_put_image);
	glBindTexture(GL_TEXTURE_2D, out_put_image);
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA32F, 800, 400, 0,GL_RGBA, GL_FLOAT, 0);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);   
	
	glBindTexture(GL_TEXTURE_2D, 0);
}

bool ReadFile(const char* pFileName, string& outFile)
{
    ifstream f(pFileName);
    
    bool ret = false;
    
    if (f.is_open()) {
        string line;
        while (getline(f, line)) {
            outFile.append(line);
            outFile.append("\n");
        }
        
        f.close();
        
        ret = true;
    }
    else {
       cout << "Error Loading file";
    }
    
    return ret;
}

string resolve_compute_shader_source;

bool PrepareComputeShader()
{
	ReadFile("compu.shader",resolve_compute_shader_source);
	
	const char* p[1];
	p[0] = resolve_compute_shader_source.c_str();

	compute_shader_obj = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(compute_shader_obj,1,p,NULL);
	glCompileShader(compute_shader_obj);

	GLint success;
	glGetShaderiv(compute_shader_obj, GL_COMPILE_STATUS, &success);

	if (!success) {
		char InfoLog[1024];
		glGetShaderInfoLog(compute_shader_obj, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling : '%s'\n", InfoLog);
		return false;
	}

	compute_shader_prog = glCreateProgram();
	glAttachShader(compute_shader_prog,compute_shader_obj);
	glLinkProgram(compute_shader_prog);

	glGetShaderiv(compute_shader_prog, GL_LINK_STATUS, &success);

	if (!success) {
		char InfoLog[1024];
		glGetShaderInfoLog(compute_shader_prog, 1024, NULL, InfoLog);
		fprintf(stderr, "Error linking : '%s'\n", InfoLog);
		return false;
	}

	return true;
}

void PrepareVBO()
{
	glGenVertexArrays(1, &render_vao);
	glBindVertexArray(render_vao);
	
	glGenBuffers(1,&IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vertex_indices), vertex_indices,GL_STATIC_DRAW);

	glGenBuffers(1, &render_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, render_vbo);
	glBufferData(GL_ARRAY_BUFFER,sizeof(vertexData), vertexData, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	
	glGenBuffers(1,&tex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, tex_buffer);
	glBufferData(GL_ARRAY_BUFFER,sizeof(texData), texData, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1,2,GL_FLOAT, GL_FALSE, 0, 0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}



bool PrepareShaders()
{
	GLint success;
	
	render_obj_vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(render_obj_vs,1,vertex_source,NULL);
	glCompileShader(render_obj_vs);

	
	glGetShaderiv(render_obj_vs, GL_COMPILE_STATUS, &success);

	if (!success) {
		char InfoLog[1024];
		glGetShaderInfoLog(render_obj_vs, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling render vs: '%s'\n", InfoLog);
		return false;
	}

	render_obj_fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(render_obj_fs,1,fragment_source,NULL);
	glCompileShader(render_obj_fs);

	
	glGetShaderiv(render_obj_fs, GL_COMPILE_STATUS, &success);

	if (!success) {
		char InfoLog[1024];
		glGetShaderInfoLog(render_obj_fs, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling : '%s'\n", InfoLog);
		return false;
	}

	render_prog = glCreateProgram();
	glAttachShader(render_prog,render_obj_vs);
	glAttachShader(render_prog,render_obj_fs);
	glLinkProgram(render_prog);

	texture_location = glGetUniformLocation(render_prog, "tex");
	glUniform1i(texture_location, 1);

	glGetShaderiv(render_prog, GL_LINK_STATUS, &success);

	if (!success) {
		char InfoLog[1024];
		glGetShaderInfoLog(render_prog, 1024, NULL, InfoLog);
		fprintf(stderr, "Error linking : '%s'\n", InfoLog);
		return false;
	}

	glDetachShader(render_prog, render_obj_vs);
	glDetachShader(render_prog,render_obj_fs);

	return true;
}

void handleResize(int w, int h) {
	
}


void KeyBoard(unsigned char key, int x, int y)
{
	switch(key)
	{
	case 'e':
		   if(key_status == true)
		   {
			   key_status = false;
		   }
		   else
		   {
			   key_status = true;
			   dispatch_once = 0;
		   }
		break;
	}
}


void Clear()
{

	glDeleteBuffers(1, &render_vbo);
	glDeleteVertexArrays(1, &render_vao);
	glDeleteShader(render_obj_vs);
	glDeleteShader(render_obj_fs);
	glDeleteProgram(render_prog);
	glDeleteBuffers(1, &tex_buffer);
	glDeleteShader(compute_shader_obj);
	glDeleteProgram(compute_shader_prog);
	glDeleteTextures(1, &out_put_image);
}

void DisplayMainWindow(void)
{
   //Trying to avoid continous spawn of workgroups
   
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glEnable(GL_DEPTH_TEST);
   glDisable(GL_CULL_FACE);

   
   if(key_status == true && dispatch_once == 0)
   {
	   
	   glUseProgram(compute_shader_prog);
	   glBindImageTexture(1, out_put_image, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	   //We need to generate 800X400 image so thats our workgroup size in x and y direction
	   glDispatchCompute(800, 400, 1);

	   glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	   glUseProgram(0);

	   // Clear, select the rendering program and draw a full screen quad
	   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	   glUseProgram(render_prog);
	   glBindVertexArray(render_vao);
	   glActiveTexture(GL_TEXTURE0);
	   glBindTexture(GL_TEXTURE_2D, out_put_image);

	   glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

	   glutSwapBuffers();   
	   glFinish();
	   
	   glBindTexture(GL_TEXTURE_2D, 0);
	   glBindVertexArray(0);
	   glUseProgram(0);
	   dispatch_once = 1;
   }

   
   glutPostRedisplay();

}

int main(int argc, char *argv[])
{	
    int mainWinID;

    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 400);
    glutCreateWindow("TeslaComputeTracer 1.0");
    glutDisplayFunc(DisplayMainWindow);
	glutReshapeFunc(handleResize);
	glutKeyboardFunc(KeyBoard);

	glewInit();
	glClearColor(0.0,0.0,0.0,1.0);
	PrepareShaders();
	PrepareVBO();
	PrepareComputeTextures();
	PrepareComputeShader();

    glutMainLoop();

	Clear();
    return 0;
}

