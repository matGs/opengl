/******************************************************************
*
* Interaction.c  
*
* Description: This file demonstrates the loading of external 
* triangle meshes provided in OBJ format. In addition, user
* interaction via mouse and keyboard is employed.
*
* The loaded triangle mesh is draw in wireframe mode and rotated
* around a reference axis. The user can control the rotation axis
* via the mouse and start/stop/reset animation via the keyboard.
*
* Computer Graphics Proseminar SS 2016
* 
* Interactive Graphics and Simulation Group
* Institute of Computer Science
* University of Innsbruck
*
* 
* Team: Mathias Gschwandtner, Artur Fedrigolli
* 
*******************************************************************/


/* Standard includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* OpenGL includes */
#include <GL/glew.h>
#include <GL/freeglut.h>

/* Local includes */
#include "LoadShader.h"    /* Loading function for shader code */
#include "Matrix.h"        /* Functions for matrix handling */
#include "OBJParser.h"     /* Loading function for triangle meshes in OBJ format */


/*----------------------------------------------------------------*/

/* Flag for starting/stopping animation */
GLboolean anim = GL_TRUE;

GLboolean anim_cam = GL_FALSE;						// Anim var for automatic camera mode

/* Define handles to two vertex buffer objects */
GLuint VBO[13];

/* Define handles to two index buffer objects */
GLuint IBO[13];

/* Indices to vertex attributes; in this case positon only */ 
enum DataID {vPosition = 0}; 

/* Strings for loading and storing shader code */
static const char* VertexShaderString;
static const char* FragmentShaderString;

GLuint ShaderProgram;


/* Matrices for uniform variables in vertex shader */
float ProjectionMatrix[16]; /* Perspective projection matrix */
float ViewMatrix[16];       /* Camera view matrix */ 
float ModelMatrix[13][16];      /* Model matrix for each .obj file */

/* Transformation matrices for model rotation */			// NEW: For every object Rotation Matrices
float RotationMatrixAnimX[16];
float RotationMatrixAnimY[16];
float RotationMatrixAnimZ[16];
float RotationMatrixAnim[16];

float RotationMatrixAnimY2[16];								
    
/* Variables for storing current rotation angles */			// NEW: fore every object Rotation angles
float angleY= 0.0f; 

float angleY2 = 0.0f;										
  

/* Identity Matrix: filler for Multiplication-gaps  */
float IdentityMatrixAnim[16];

/* Extra Translation Matrix to Translate from/to center */
float TranslationMatrixAnim[16];



/* Indices to active rotation axes */
enum {YaxisStop=0, Yaxis=1};
int axis = Yaxis;

/* Indices to active triangle mesh */
enum {Model1=0, Model2=1, Model3=3, Model4=4, Model5=5, Model6=6, Model7=7,	
      Model8=8, Model9=9, Model10=10, Model11=11, Model12=12, Model13=13
};
int model = Model1; 
  
/* Arrays for holding vertex data of the two models */				
GLfloat *vertex_buffer_data[13];


/* Arrays for holding indices of the two models */
GLushort *index_buffer_data[13];


/* Structures for loading of OBJ data */
obj_scene_data data[13];

/* Reference time for animation */
int oldTime = 0;


/* View values */						// New - reset camera (view) values
float camera_disp = -12.0;
float angle=0, xx=0;

/*----------------------------------------------------------------*/



/******************************************************************
*
* Display
*
* This function is called when the content of the window needs to be
* drawn/redrawn. It has been specified through 'glutDisplayFunc()';
* Enable vertex attributes, create binding between C program and 
* attribute name in shader, provide data for uniform variables
*
*******************************************************************/

void Display()								// NEW: all models center in point of origin?
{
  // draw fixed background structure: Cube as a room space 		// NEW: here fixed background structure?
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDisable(GL_DEPTH_TEST);
  gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
  glColor3f(0.5, 0.1, 0.2);
  glBegin(GL_QUADS);
  glVertex3f(-100.0f, 0.0f, -100.0f);
  glVertex3f(-100.0f, 0.0f,  100.0f);
  glVertex3f( 100.0f, 0.0f,  100.0f);
  glVertex3f( 100.0f, 0.0f, -100.0f);
  glEnd();

  /* Clear window; color specified in 'Initialize()' */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
  int i;
  for(i=0; i<13; ++i){
    glEnableVertexAttribArray(vPosition);

    /* Bind buffer with vertex data of currently active object */
    glBindBuffer(GL_ARRAY_BUFFER, VBO[i]);
    glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
     

    /* Bind buffer with index data of currently active object */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO[i]);
    
    
    GLint size; 
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);

    /* Associate program with uniform shader matrices */
    GLint projectionUniform = glGetUniformLocation(ShaderProgram, "ProjectionMatrix");
    if (projectionUniform == -1) 
    {
        fprintf(stderr, "Could not bind uniform ProjectionMatrix\n");
	exit(-1);
    }
    glUniformMatrix4fv(projectionUniform, 1, GL_TRUE, ProjectionMatrix);
    
    GLint ViewUniform = glGetUniformLocation(ShaderProgram, "ViewMatrix");
    if (ViewUniform == -1) 
    {
        fprintf(stderr, "Could not bind uniform ViewMatrix\n");
        exit(-1);
    }
    glUniformMatrix4fv(ViewUniform, 1, GL_TRUE, ViewMatrix);
   
    GLint RotationUniform = glGetUniformLocation(ShaderProgram, "ModelMatrix");
    if (RotationUniform == -1) 
    {
        fprintf(stderr, "Could not bind uniform ModelMatrix\n");
        exit(-1);
    }
    
    glUniformMatrix4fv(RotationUniform, 1, GL_TRUE, ModelMatrix[i]);  	// Rotation Matrix HERE? - Translation Matrices HERE?
    
    // NEW roation of single objects - DOES NOT WORK
/*    
    int newTime = glutGet(GLUT_ELAPSED_TIME);
    int delta = newTime - oldTime;
    oldTime = newTime;
    angleY = 0;
    angleY = fmod(angleY + delta/20.0, 360.0);
    SetRotationY(angleY, RotationMatrixAnimY);      
    SetIdentityMatrix(IdentityMatrixAnim);
    MultiplyMatrix(IdentityMatrixAnim, RotationMatrixAnimY, ModelMatrix[8]);
    angleY = 0;
*/

    
    
    /* Set state to only draw wireframe (no lighting used, yet) */
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); 

    /* Issue draw command, using indexed triangle list */
    glDrawElements(GL_TRIANGLES, size/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);

    /* Disable attributes */
    glDisableVertexAttribArray(vPosition);
    
  }  

  /* Swap between front and back buffer */ 
  glutSwapBuffers();
}


/******************************************************************
*
* Mouse
*
* Function is called on mouse button press; has been seta
* with glutMouseFunc(), x and y specify mouse coordinates,
* but are not used here.
*
*******************************************************************/

void Mouse(int button, int state, int x, int y) 
{
    if(state == GLUT_DOWN) 
    {
      /* Depending on button pressed, set rotation axis,
       * turn on animation */
        switch(button) 
	{
	    case GLUT_LEFT_BUTTON:    
	        axis = YaxisStop;					
		break;

	    case GLUT_MIDDLE_BUTTON:  
  	        axis = Yaxis;
	        break;
		
	    case GLUT_RIGHT_BUTTON: 					
		exit(0);    
		break;
	}
	anim = GL_TRUE;
    }
}


/******************************************************************		// NEW re-able keyboard for camera modes?
*
* Keyboard
* 
* Function to be called on key press in window; set by
* glutKeyboardFunc(); x and y specify mouse position on keypress;
* not used in this example 
*
*******************************************************************/


void Keyboard(unsigned char key, int x, int y){
  switch( key ) {				// NEW: use AWSD to control camera (VieMatrix: translation on x axis and z axis
    case 'w' :
	camera_disp -= 0.05f;
	break;
    case 's' :
	camera_disp += 0.05f;
	break;
    case 'd' :
	xx += 0.025f;
	break;
    case 'a' :
	xx -= 0.025f;
	break;
    case 'm' :	// set automatic camera mode 
	anim_cam = GL_TRUE;
	return;
	break;
    case 'n' :	// unset automatic camera mode
	anim_cam = GL_FALSE;
	return;
	break;
  }
  SetTranslation(xx, 0, camera_disp, ViewMatrix);
  glutPostRedisplay();
}


/******************************************************************
*
* OnIdle
*
* Function executed when no other events are processed; set by
* call to glutIdleFunc(); holds code for animation  
*
*******************************************************************/

void OnIdle()
{
    /* Determine delta time between two frames to ensure constant animation */
    int newTime = glutGet(GLUT_ELAPSED_TIME);
    int delta = newTime - oldTime;
    oldTime = newTime;

    /* if in automatic camera mode do following */				// NEW auto camera mode
    if(anim_cam){
	if(camera_disp > -18){
	    camera_disp -= 0.1;
	    SetTranslation(xx, 0, camera_disp, ViewMatrix);
        
	}
	else if(xx < 3){
	    xx += 0.1;	      
	    SetTranslation(xx, 0, camera_disp, ViewMatrix);
	}
	else{
	  anim_cam = GL_FALSE;
	}
    }
    
    
    /* If animation is set to true, rotate */
    if(anim){
        /* Increment rotation angles and update matrix */
        if(axis == YaxisStop){
	      angleY = 0;
	      SetRotationY(0, RotationMatrixAnimY);  
	      
	      
	}
	else if(axis == Yaxis){
	    angleY = fmod(angleY + delta/20.0, 360.0); 
	    SetRotationY(angleY, RotationMatrixAnimY);  
	}  
    }
    else {	/* else: do not apply rotation below */
      return;
    }

    
    // ROTATE AROUND OBJECTS CENTER?: translate to center, than rotate, and translate back    
    int k;
    for(k=1; k<13; ++k){
            
      /* Update of transformation matrices 
       * Note order of transformations and rotation of reference axes */
      SetIdentityMatrix(IdentityMatrixAnim);
      MultiplyMatrix(IdentityMatrixAnim, RotationMatrixAnimY, RotationMatrixAnim);
      		
      
      if(k==1 || k == 6 || k == 7){			
	// do not rotate top bar
	
	angleY2 = -fmod(angleY + delta/20.0, 360.0);	
	SetRotationY(angleY2, RotationMatrixAnimY2);	
	MultiplyMatrix(RotationMatrixAnimY2, IdentityMatrixAnim, ModelMatrix[k]);					
	// winkel einfach umdrehen
	// fuer die andre richtung
					
	// by evgenij zuenko
	// damit dreht sich der ball  doppelt so schnell
	MultiplyMatrix(RotationMatrixAnimY2,RotationMatrixAnimY2,ModelMatrix[k]);
	
      }
      else {
        MultiplyMatrix(RotationMatrixAnim, IdentityMatrixAnim, ModelMatrix[k]);			
      }
    }
  
 
    
    /* Issue display refresh */
    glutPostRedisplay();
}


/******************************************************************
*
* SetupDataBuffers
*
* Create buffer objects and load data into buffers
*
*******************************************************************/

void SetupDataBuffers()
{
  
    int i;
    for(i=0; i<14; ++i){
      glGenBuffers(1, &VBO[i]);
      glBindBuffer(GL_ARRAY_BUFFER, VBO[i]);
      glBufferData(GL_ARRAY_BUFFER, data[i].vertex_count*3*sizeof(GLfloat), vertex_buffer_data[i], GL_STATIC_DRAW);   
    }
    
  
    for(i=0; i<13; ++i){
      glGenBuffers(1, &IBO[i]);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO[i]);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, data[i].face_count*3*sizeof(GLushort), index_buffer_data[i], GL_STATIC_DRAW);
    }
    
 
}


/******************************************************************
*
* AddShader
*
* This function creates and adds individual shaders
*
*******************************************************************/

void AddShader(GLuint ShaderProgram, const char* ShaderCode, GLenum ShaderType)
{
    /* Create shader object */
    GLuint ShaderObj = glCreateShader(ShaderType);

    if (ShaderObj == 0) 
    {
        fprintf(stderr, "Error creating shader type %d\n", ShaderType);
        exit(0);
    }

    /* Associate shader source code string with shader object */
    glShaderSource(ShaderObj, 1, &ShaderCode, NULL);

    GLint success = 0;
    GLchar InfoLog[1024];

    /* Compile shader source code */
    glCompileShader(ShaderObj);
    glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);

    if (!success) 
    {
        glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
        fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
        exit(1);
    }

    /* Associate shader with shader program */
    glAttachShader(ShaderProgram, ShaderObj); 
}


/******************************************************************
*
* CreateShaderProgram
*
* This function creates the shader program; vertex and fragment
* shaders are loaded and linked into program; final shader program
* is put into the rendering pipeline 
*
*******************************************************************/

void CreateShaderProgram()
{
    /* Allocate shader object */
    ShaderProgram = glCreateProgram();

    if (ShaderProgram == 0) 
    {
        fprintf(stderr, "Error creating shader program\n");
        exit(1);
    }

    /* Load shader code from file */
    VertexShaderString = LoadShader("shaders/vertexshader.vs");
    FragmentShaderString = LoadShader("shaders/fragmentshader.fs");

    /* Separately add vertex and fragment shader to program */
    AddShader(ShaderProgram, VertexShaderString, GL_VERTEX_SHADER);
    AddShader(ShaderProgram, FragmentShaderString, GL_FRAGMENT_SHADER);

    GLint Success = 0;
    GLchar ErrorLog[1024];

    /* Link shader code into executable shader program */
    glLinkProgram(ShaderProgram);

    /* Check results of linking step */
    glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);

    if (Success == 0) 
    {
        glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
        exit(1);
    }

    /* Check if shader program can be executed */ 
    glValidateProgram(ShaderProgram);
    glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);

    if (!Success) 
    {
        glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
        exit(1);
    }

    /* Put linked shader program into drawing pipeline */
    glUseProgram(ShaderProgram);
}


/******************************************************************
*
* Initialize
*
* This function is called to initialize rendering elements, setup
* vertex buffer objects, and to setup the vertex and fragment shader;
* meshes are loaded from files in OBJ format; data is copied from
* structures into vertex and index arrays
*
*******************************************************************/

void Initialize()
{   
    int k, i;
    int success;
    int vert, indx;

    /* Load OBJ models */
    char* filename[13];
    filename[0] = "models/ring.obj";
    filename[1] = "models/bar_01.obj"; 
    filename[2] = "models/bar_02.obj";
    filename[3] = "models/bar_03.obj";
    filename[4] = "models/bar_04.obj";
    filename[5] = "models/cone.obj";
    filename[6] = "models/ball_01.obj";
    filename[7] = "models/ball_02.obj";
    filename[8] = "models/cone_small.obj";
    filename[9] = "models/rectangle_small.obj";
    filename[10] = "models/rectangle_big.obj";
    filename[11] = "models/elliptic_ring.obj";
    filename[12] = "models/ellipse.obj";
    
    for(k=0; k<13; ++k){
      
      success = parse_obj_scene(&data[k], filename[k]);

      if(!success)
	  printf("Could not load file. Exiting.\n");
    }
    

  
    /*  Copy mesh data from structs into appropriate arrays */ 
    
    for(k=0; k<13; ++k){
      vert = data[k].vertex_count;
      indx = data[k].face_count;

      vertex_buffer_data[k] = (GLfloat*) calloc (vert*3, sizeof(GLfloat));
      index_buffer_data[k] = (GLushort*) calloc (indx*3, sizeof(GLushort));
  
    /* Vertices */
      for(i=0; i<vert; i++)
      {
	vertex_buffer_data[k][i*3] = (GLfloat)(*data[k].vertex_list[i]).e[0] ; 			
	vertex_buffer_data[k][i*3+1] = (GLfloat)(*data[k].vertex_list[i]).e[1];
	vertex_buffer_data[k][i*3+2] = (GLfloat)(*data[k].vertex_list[i]).e[2];
	}

    /* Indices */
      for(i=0; i<indx; i++){
	index_buffer_data[k][i*3] = (GLushort)(*data[k].face_list[i]).vertex_index[0];
	index_buffer_data[k][i*3+1] = (GLushort)(*data[k].face_list[i]).vertex_index[1];
	index_buffer_data[k][i*3+2] = (GLushort)(*data[k].face_list[i]).vertex_index[2];
      }
    }
    
 
    /* Set background (clear) color to blue */ 
    glClearColor(0.1, 0.2, 0.5, 0.0);

    /* Setup Vertex array object - INTEL FIX*/ 
    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);	 
    
									      // NEW: code here for fixed background object?
    
    
    /* Enable depth testing */
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);    

    /* Setup vertex, color, and index buffer objects */
    SetupDataBuffers();

    /* Setup shaders and shader program */
    CreateShaderProgram();  

    /* Initialize matrices */
    SetIdentityMatrix(ProjectionMatrix);
    SetIdentityMatrix(ViewMatrix);	
  
    // NEW set all Model matrices
    for(k=0; k<13; ++k){
       SetIdentityMatrix(ModelMatrix[k]);
    }
    
    
    /* Initialize animation matrices */
    SetIdentityMatrix(RotationMatrixAnimX);
    SetIdentityMatrix(RotationMatrixAnimY);
    SetIdentityMatrix(RotationMatrixAnimZ);
    SetIdentityMatrix(RotationMatrixAnim);
    
    /* Set projection transform */
    float fovy = 45.0;
    float aspect = 1.0; 
    float nearPlane = 1.0; 
    float farPlane = 50.0;
    SetPerspectiveMatrix(fovy, aspect, nearPlane, farPlane, ProjectionMatrix);

    /* Set initial viewing transform */
    SetTranslation(0, 0.0, camera_disp, ViewMatrix);
    	
}


/******************************************************************
*
* main
*
* Main function to setup GLUT, GLEW, and enter rendering loop
*
*******************************************************************/

int main(int argc, char** argv)
{
    /* Initialize GLUT; set double buffered window and RGBA color model */
    glutInit(&argc, argv);
     // NEW re-able keyboard for camera modes
    /*Intel HD Fix*/
    glutInitContextVersion(3, 3);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(750, 750);
    glutInitWindowPosition(200, 100);
    glutCreateWindow("CG Proseminar - Assignment 1");
    
    /*Initialize GL extension wrangler - Intel HD Fix*/
    glewExperimental = GL_TRUE;
    
    /* Initialize GL extension wrangler */
    GLenum res = glewInit();
    if (res != GLEW_OK) 
    {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    }

    /* Setup scene and rendering parameters */
    Initialize();

    /* Specify callback functions;enter GLUT event processing loop, 
     * handing control over to GLUT */
    glutIdleFunc(OnIdle);
    glutDisplayFunc(Display);
    glutKeyboardFunc(Keyboard); 					// NEW re-enable keyboard for camera modes 
    glutMouseFunc(Mouse);  

    glutMainLoop();

    /* ISO C requires main to return int */
    return 0;
}
