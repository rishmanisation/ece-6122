// Calculate and display the Mandelbrot set
// ECE6612 Final Project
// Rishabh Ananthan

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <vector>
#include <GL/glut.h>
#include <GL/glext.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "complex.h"

using namespace std;

// Min and max complex plane values
Complex  minC(-2.0, -1.2);
Complex  maxC( 1.0, 1.8);
int      maxIt = 2000;     // Max iterations for the set computations 
int      nThreads = 16;    // Number of threads
int      P = nThreads + 1;
pthread_mutex_t MBMutex;
bool* localSense;
bool globalSense;
int elementCount = 0;
int a;
class Memory                                                                                    
{
	public:
		float minC_real, minC_imag, maxC_real, maxC_imag;
		Memory(float a, float b, float c, float d)
		{
			minC_real = a;
			minC_imag = b;
			maxC_real = c;
			maxC_imag = d;
		}
};
vector<Memory> mv;  

// Global variables for mouse ops
bool mousevar;

struct Point
{
	float x,y;
	Point()
	{
		x = 0.0;
		y = 0.0;
	}
};

Point A,B;
float dx,dy,d;
#define WINDOW_SIZE 512
Complex* c = new Complex[WINDOW_SIZE * WINDOW_SIZE];
int pixel_iterations[WINDOW_SIZE * WINDOW_SIZE];

// Define the RGB Class
class RGB
{
public:
  RGB()
    : r(0), g(0), b(0) {}
  RGB(double r0, double g0, double b0)
    : r(r0), g(g0), b(b0) {}
public:
  double r;
  double g;
  double b;
};

RGB* colors = 0; // Array of color values

void InitializeColors()
{
  colors = new RGB[maxIt + 1];
  for (int i = 0; i < maxIt; ++i)
    {
      if (i < 5)
        { // Try this.. just white for small it counts
          colors[i] = RGB(1, 1, 1);
        }
      else
        {
          colors[i] = RGB(drand48(), drand48(), drand48());
        }
    }
  colors[maxIt] = RGB(); // black
}

// Call MyBarrier_Init once in main
void MyBarrier_Init()
{	
	elementCount = P;
	pthread_mutex_init(&MBMutex,0);
	localSense = new bool[P];
	for (int i = 0; i < P; i++)
	{
		localSense[i] = true;
	}
	globalSense = true;
}

// Each thread calls MyBarrier after completing the row-wise DFT
void MyBarrier(int myId) 
{
	localSense[myId] = !localSense[myId];
	pthread_mutex_lock(&MBMutex);
	int myCount = elementCount;
	elementCount--;
	pthread_mutex_unlock(&MBMutex);
	if (myCount == 1)
	{
		elementCount = P;
		globalSense = localSense[myId];
	}
	
	else
	{
		while (globalSense != localSense[myId])
		{
			//spin
		}
	}
}

Complex complexNumber(int i, int j)
{
	double realDiff = maxC.real - minC.real;
	double imagDiff = maxC.imag - minC.imag;
	double nReal = (double) i/WINDOW_SIZE;
	double nImag = (double) j/WINDOW_SIZE;
	return minC + Complex(nReal*realDiff,nImag*imagDiff);
}

void calcComplexArray()
{
	for (int i = 0; i < WINDOW_SIZE; i++)
	{
		for (int j = 0; j < WINDOW_SIZE; j++)
		{
			c[i*WINDOW_SIZE + j] = complexNumber(i,j);
		}
	}
}

void* threadsdrawMBSet(void* v)
{
	unsigned long myId = (unsigned long) v;
	unsigned long localCount = 0;
	int rowsPerThread = WINDOW_SIZE/nThreads;
	int startingRow = myId * rowsPerThread;
	for (int i = 0; i < rowsPerThread; i++)
	{
		int thisRow = startingRow + i;
		for (int j = 0; j < WINDOW_SIZE; j++)
		{
			// Initialize z and set it to the value of 'c' for current pixel
			Complex z = c[thisRow*WINDOW_SIZE + j];
			// Set the number of iterations for current pixel to zero
			pixel_iterations[thisRow*WINDOW_SIZE + j] = 0;
			localCount++;
			/* Keep iterating until one of the conditions is violated.
			   Then, all the values in the complex array at the time of 
			   termination are in the Mandelbrot set. 
			*/
			while(pixel_iterations[thisRow*WINDOW_SIZE+j] < maxIt && z.Mag2() < 4.0)
			{
				pixel_iterations[thisRow*WINDOW_SIZE + j]++;
				z = (z*z) + c[thisRow*WINDOW_SIZE + j];
			}	
		}
	}
}

void threadcreation()
{
  // Create 16 threads
  MyBarrier_Init();   // initialize MyBarrier
  for (int i = 0; i < nThreads; ++i) // creating the threads
  {
	  pthread_t pt;
	  pthread_create(&pt,0,threadsdrawMBSet,(void*)i);
  }
}

void drawMBSet()
{
	calcComplexArray();
	threadcreation();		
}

void displayMBSet()
{
	glBegin(GL_POINTS);
	for (int i = 0; i < WINDOW_SIZE; i++)
	{
		for (int j = 0; j < WINDOW_SIZE; j++)
		{
			glColor3f(colors[pixel_iterations[i*WINDOW_SIZE + j]].r,colors[pixel_iterations[i*WINDOW_SIZE + j]].g,colors[pixel_iterations[i*WINDOW_SIZE + j]].b);
			glVertex2d(i,j);
		}
	}
	glEnd();
}

void drawSquareRegion()
{
	glColor3f(1,1,1);
	glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	glBegin(GL_POLYGON);
	glVertex2f(A.x,A.y);
	glVertex2f(B.x,A.y);
	glVertex2f(B.x,B.y);
	glVertex2f(A.x,B.y);
	glEnd();
}

void display(void)
{ // Your OpenGL display code here
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClearColor(1.0,1.0,1.0,1.0);
  glLoadIdentity();
  displayMBSet();  
  if (mousevar)
  {
	  drawSquareRegion();
  }
  glutSwapBuffers();
}

void mouse(int button, int state, int x, int y)
{ // Your mouse click processing here
  // state == 0 means pressed, state != 0 means released
  // Note that the x and y coordinates passed in are in
  // PIXELS, with y = 0 at the top.
  
  // Case 1: If Left button is pressed
  if (button == GLUT_LEFT_BUTTON && state == 0)
  {
	  A.x = x;
	  B.x = x;
	  A.y = y;
	  B.y = y;
	  mousevar = 1;
  }
  
  // Case 2: If Left button is released
  if (button == GLUT_LEFT_BUTTON && state != 0)
  {
	  mv.push_back(Memory(minC.real,minC.imag,maxC.real,maxC.imag));
	  if (x > A.x && y > A.y)
	  {
		  B.x = A.x + d;
		  B.y = A.y + d;
		  
		  for (int i = 0; i < WINDOW_SIZE; i++)
		  {
			  for (int j = 0; j < WINDOW_SIZE; j++)
			  {
				  if (i == A.x && j == A.y)
				  {
					  minC.real = c[i*WINDOW_SIZE + j].real;
					  minC.imag = c[i*WINDOW_SIZE + j].imag;
				  }
				  
				  if (i == B.x && j == B.y)
				  {
					  maxC.real = c[i*WINDOW_SIZE + j].real;
					  maxC.imag = c[i*WINDOW_SIZE + j].imag; 
				  }
			  }
		  }
	  }
	  
	  if (x < A.x && y < A.y)
	  {
		  B.x = A.x - d;
		  B.y = A.y - d;
		  for (int i = 0; i < WINDOW_SIZE; i++)
		  {
			  for (int j = 0; j < WINDOW_SIZE; j++)
			  {
				  if (i == B.x && j == B.y)
				  {
					  minC.real = c[i*WINDOW_SIZE + j].real;
					  minC.imag = c[i*WINDOW_SIZE + j].imag;
				  }
				  
				  if (i == A.x && j == A.y)
				  {
					  maxC.real = c[i*WINDOW_SIZE + j].real;
					  maxC.imag = c[i*WINDOW_SIZE + j].imag; 
				  }
			  }
		  }		  
	  }

	  if (x < A.x && y > A.y)
	  {
		  B.x = A.x - d;
		  B.y = A.y + d;
		  for (int i = 0; i < WINDOW_SIZE; i++)
		  {
			  for (int j = 0; j < WINDOW_SIZE; j++)
			  {
				  if (i == B.x && j == A.y)
				  {
					  minC.real = c[i*WINDOW_SIZE + j].real;
					  minC.imag = c[i*WINDOW_SIZE + j].imag;
				  }
				  
				  if (i == A.x && j == B.y)
				  {
					  maxC.real = c[i*WINDOW_SIZE + j].real;
					  maxC.imag = c[i*WINDOW_SIZE + j].imag; 
				  }
			  }
		  }		  
	  }

	  if (x > A.x && y < A.y)
	  {
		  B.x = A.x + d;
		  B.y = A.y - d;
		  for (int i = 0; i < WINDOW_SIZE; i++)
		  {
			  for (int j = 0; j < WINDOW_SIZE; j++)
			  {
				  if (i == A.x && j == B.y)
				  {
					  minC.real = c[i*WINDOW_SIZE + j].real;
					  minC.imag = c[i*WINDOW_SIZE + j].imag;
				  }
				  
				  if (i == B.x && j == A.y)
				  {
					  maxC.real = c[i*WINDOW_SIZE + j].real;
					  maxC.imag = c[i*WINDOW_SIZE + j].imag; 
				  }
			  }
		  }		  
	  }
	  mousevar = 0;
	  drawMBSet();
	  glutPostRedisplay();
  } 
}

void motion(int x, int y)                                                                     
{// Mouse pointer motion processing
	dx = abs(x - A.x);
	dy = abs(y - A.y);
	if(x > A.x && y > A.y)
	{
		if(dx > dy)
		{
			d = dy;
		}
	
		if(dx < dy)
		{
			d = dx;
		}
		B.x = A.x + d;  
		B.y = A.y + d;
	}

	if(x < A.x && y < A.y)
	{
		if(dx > dy)
		{
			d = dy;
		}
		if(dx < dy)
		{
			d = dx;
		}
		B.x = A.x - d;  
		B.y = A.y - d;	
	}

	if(x > A.x && y < A.y)
	{
		if(dx > dy)
		{
			d = dy;
		}
		if(dx < dy)
		{
			d = dx;
		}
		B.x = A.x + d;  
		B.y = A.y - d;
	}

	if(x < A.x && y > A.y)
	{
		if(dx > dy)
		{
			d = dy;
		}
	
		if(dx < dy)
		{
			d = dx;
		}
		B.x = A.x - d;  
		B.y = A.y + d;
	}
	glutPostRedisplay();
}

void keyboard(unsigned char c, int x, int y)
{ // Your keyboard processing here
	if (c == 'b' || c == 'B')
	{
		if(mv.size() > 0)
		{
			Memory t = mv.back();
			mv.pop_back();
			minC.real = t.minC_real;
			minC.imag = t.minC_imag;
			maxC.real = t.maxC_real;
			maxC.imag = t.maxC_imag;
			drawMBSet();
			glutPostRedisplay();
		}
	}
}

void init(void)
{ // Your OpenGL initialization code here	
	InitializeColors();
	
	glViewport(0,0,WINDOW_SIZE,WINDOW_SIZE);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	gluOrtho2D(0,WINDOW_SIZE,WINDOW_SIZE,0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

int main(int argc, char** argv)
{
  // Initialize OpenGL, but only on the "master" thread or process.
  // See the assignment writeup to determine which is "master" 
  // and which is slave.
  glutInit(&argc,argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(WINDOW_SIZE,WINDOW_SIZE);
  glutInitWindowPosition(700,250);
  glutCreateWindow("MBSet");
  
  // Initializing threads and OpenGL
  drawMBSet();
  init();
  
  glutDisplayFunc(display);
  glutIdleFunc(display);
  glutKeyboardFunc(keyboard);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  glutMainLoop();
  return 0;
}

