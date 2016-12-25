// Draw an Icosahedron
// ECE6122 Project 5
// RISHABH ANANTHAN

#include <iostream>
#include <math.h>
#include <GL/glut.h>
#include <GL/glext.h>
#include <GL/gl.h>
#include <GL/glu.h>

using namespace std;

#define NFACE 20
#define NVERTEX 12

#define X .525731112119133606 
#define Z .850650808352039932

// These are the 12 vertices for the icosahedron
static GLfloat vdata[NVERTEX][3] = {    
   {-X, 0.0, Z}, {X, 0.0, Z}, {-X, 0.0, -Z}, {X, 0.0, -Z},    
   {0.0, Z, X}, {0.0, Z, -X}, {0.0, -Z, X}, {0.0, -Z, -X},    
   {Z, X, 0.0}, {-Z, X, 0.0}, {Z, -X, 0.0}, {-Z, -X, 0.0} 
};

// These are the 20 faces.  Each of the three entries for each 
// vertex gives the 3 vertices that make the face.
static GLint tindices[NFACE][3] = { 
   {0,4,1}, {0,9,4}, {9,5,4}, {4,5,8}, {4,8,1},    
   {8,10,1}, {8,3,10}, {5,3,8}, {5,2,3}, {2,7,3},    
   {7,10,3}, {7,6,10}, {7,11,6}, {11,0,6}, {0,1,6}, 
   {6,1,10}, {9,0,11}, {9,11,2}, {9,2,5}, {7,2,11} };

int testNumber; // Global variable indicating which test number is desired
int depth; // global variable for depth
static float updateRate = 10.0; // updateRate for timer function
float* Rval;
float* Gval;
float* Bval;
GLfloat rotLim = 360.0; // rotation limit

// Default functions
void init()
{
  glClearColor(0.0,0.0,0.0,0.0);
  glShadeModel(GL_FLAT);
  glEnable(GL_LINE_SMOOTH);
  glLoadIdentity();
  gluLookAt(0.0,0.0,1.0,0.0,0.0,0.0,0.0,1.0,0.0);
}

void reshape(int w, int h)
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glViewport(0,0,(GLsizei)w,(GLsizei)h);
}

void timer(int)
{
  glutPostRedisplay();
  glutTimerFunc(1000.0/updateRate,timer,0);
}

// Function to draw a triangle and the edge lines
void drawTriangle(GLfloat* v1, GLfloat* v2, GLfloat* v3, int face)
{
  glBegin(GL_TRIANGLES);
  glColor3f(Rval[face],Gval[face],Bval[face]);
  glVertex3fv(v1);
  glVertex3fv(v2);
  glVertex3fv(v3);
  glEnd();

  glBegin(GL_LINE_LOOP);
  glColor3f(1.0,1.0,1.0);
  glVertex3fv(v1);
  glVertex3fv(v2);
  glVertex3fv(v3);
  glEnd();
}

// Function to randomly assign a color to each face. The colors change on repeated execution of the program.
void assignColors(int n)
{
  srand(time(NULL));
  for (int i = 0; i < n; i++)
    {
      Rval[i] = (float) rand()/RAND_MAX;
      Gval[i] = (float) rand()/RAND_MAX;
      Bval[i] = (float) rand()/RAND_MAX;
    }
}

// Function to implement rotation 
void Rotate()
{
  static GLfloat rotX = 0.0;
  static GLfloat rotY = 0.0;
  glRotatef(rotX,1.0,0.0,0.0);
  glRotatef(rotY,0.0,1.0,0.0);
  rotX += 1.0;
  rotY -= 1.0;
  if (rotX == rotLim)
    {
      rotX = (GLfloat)0.0;
    }
  if (rotY == rotLim)
    {
      rotY = (GLfloat)0.0;
    }
}

// Function to normalize the coordinates
void Normalize(GLfloat* v)
{
  // normalize coordinates by dividing each by the square root of the sum of all three
  // this ensures that each point vector has unit magnitude
  GLfloat V = sqrt(pow(v[0],2)+pow(v[1],2)+pow(v[2],2));
  v[0] = (GLfloat) v[0]/V;
  v[1] = (GLfloat) v[1]/V;
  v[2] = (GLfloat) v[2]/V;
}

// Function to divide the triangle into smaller triangles dependent on depth
void divideTriangle(GLfloat* v1, GLfloat* v2, GLfloat* v3, int d, int face)
{
  int i = pow(4,depth-d); // Division of triangles as per assignment description
  GLfloat v12[3]; // midpoint between v1 and v2
  GLfloat v23[3]; // midpoint between v2 and v3
  GLfloat v31[3]; // midpoint between v3 and v1
  for (int k = 0; k < 3; k++)     // loop to find the coordinates of the midpoints
    {
      v12[k] = (GLfloat) (v1[k]+v2[k])/2.0;
      v23[k] = (GLfloat) (v2[k]+v3[k])/2.0;
      v31[k] = (GLfloat) (v3[k]+v1[k])/2.0;
    }
  Normalize(v12);
  Normalize(v23);
  Normalize(v31);
  if (d == 1) // if d = 1 then the triangles cannot be divided any further
    {
      drawTriangle(v1,v12,v31,4*face*i);
      drawTriangle(v12,v31,v23,4*face*i+1);
      drawTriangle(v12,v23,v2,4*face*i+2);
      drawTriangle(v31,v23,v3,4*face*i+3);
    }
  else // if d > 1 then the triangles can further be divided into smaller triangles
    {
      divideTriangle(v1,v12,v31,d-1,face);
      divideTriangle(v12,v31,v23,d-1,face);
      divideTriangle(v12,v23,v2,d-1,face);
      divideTriangle(v31,v23,v3,d-1,face);
    }
}

// Test cas es.  Fill in your code for each test case
void Test1()
{
  // Just draw the Icosahedron
  glPushMatrix();
  for (int i = 0; i < NFACE; i++)
    {
      drawTriangle(&vdata[tindices[i][0]][0],&vdata[tindices[i][1]][0],&vdata[tindices[i][2]][0],i);
    }
  glPopMatrix();
}

void Test2()
{
  // Icosahedron with rotation
  Rotate();
  Test1();
}

void Test3()
{
  // Icosahedron with unit depth and subdivision of triangles
  glPushMatrix();
  for (int i = 0; i < NFACE; i++)
    {
      divideTriangle(&vdata[tindices[i][0]][0],&vdata[tindices[i][1]][0],&vdata[tindices[i][2]][0],1,i);
    }
  glPopMatrix();
  
}


void Test4()
{
  // Icosahedron with unit depth, subdivision of triangles and rotation
  Rotate();
  Test3();
}

void Test5(int depth)
{
  // Icosahedron with depth input and subdivision of triangles
  glPushMatrix();
  glPushMatrix();
  for (int i = 0; i < NFACE; i++)
    {
      divideTriangle(&vdata[tindices[i][0]][0],&vdata[tindices[i][1]][0],&vdata[tindices[i][2]][0],depth,i);
    }
  glPopMatrix();
  glPopMatrix();

}

void Test6(int depth)
{
  // Icosahedron with rotation, depth input and subdivision of triangles
  Rotate();
  Test5(depth);
}

// Display function
void display(void)
{
  glClear(GL_COLOR_BUFFER_BIT);
  glEnable(GL_LINE_SMOOTH);
  glClear(GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
  switch(testNumber)
    {
    case 1:
      Test1();
      break;
    case 2:
      Test2();
      break;
    case 3:
      Test3();
      break;
    case 4:
      Test4();
      break;
    case 5:
      Test5(depth);
      break;
    case 6:
      Test6(depth);
      break;
    default:
      cout << "ERROR: INVALID TEST NUMBER";
      exit(3);
    }
  glutSwapBuffers();
}
  
int main(int argc, char** argv)
{
  if (argc < 2)
    {
      std::cout << "Usage: icosahedron testnumber" << endl;
      exit(1);
    }
  // Set the global test number
  testNumber = atol(argv[1]);
  if (testNumber == 3)
    {
      depth = 1;
    }
  if (testNumber > 4)
    {
      if (argc < 3)
	{
	  std::cout << "ENTER DEPTH VALUE" << endl;
	  exit(2);
	}
      depth = atoi(argv[2]);
      if (depth > 5)
	{
	  std::cout << "DEPTH MUST BE <= 4" << endl;
	  exit(2);
	}
    }
      
  
  // Initialize glut  and create your window here
  glutInit(&argc,argv);
  int n = (int) NFACE*pow(4,depth); // number of triangles for which color has to be assigned
  Rval = new float[n]; 
  Gval = new float[n];
  Bval = new float[n];
  assignColors(n);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(500,500);
  glutInitWindowPosition(100,100);
  glutCreateWindow("Icosahedron");
  init();

  // Set your glut callbacks here
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutTimerFunc(1000.0/updateRate,timer,0);

  // Enter the glut main loop here
  glutMainLoop();
  return 0;
}

