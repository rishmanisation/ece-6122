// Threaded two-dimensional Discrete FFT transform
// Rishabh Ananthan
// ECE6122 Project 2

#include <iostream>
#include <string>
#include <math.h>
#include "Complex.h"
#include "InputImage.h"

using namespace std;
Complex* imdata;
int imwidth;
int imheight;
int N = 1024;
Complex* temp = new Complex[N*N];
Complex* W = new Complex[N/2];
int numofThreads = 16; 
int P = numofThreads + 1;
int count;
pthread_mutex_t countMutex;
bool* localSense;
bool globalSense;
bool state;

// Declaration of all functions 
unsigned ReverseBits(unsigned v);
void MyBarrier_Init();
void MyBarrier(int myId);
void Transform1D(Complex* h, int N, Complex* W, bool a);
void calcweights(bool a);
void transposeof(Complex* in, Complex* out);
void reversebits(Complex* in, Complex* out);
void threadcreation();
void* Transform2DTHread(void* v);
void Transform2D(const char* inputFN);

// Function to reverse bits in an unsigned integer
// This assumes there is a global variable N that is the
// number of points in the 1D transform.
unsigned ReverseBits(unsigned v)
{ //  Provided to students
  unsigned n = N; // Size of array (which is even 2 power k value)
  unsigned r = 0; // Return value
   
  for (--n; n > 0; n >>= 1)
    {
      r <<= 1;        // Shift return value
      r |= (v & 0x1); // Merge in next bit
      v >>= 1;        // Shift reversal value
    }
  return r;
}

// Call MyBarrier_Init once in main
void MyBarrier_Init()
{	
	count = P;
	pthread_mutex_init(&countMutex,0);
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
	pthread_mutex_lock(&countMutex);
	int myCount = count;
	count--;
	pthread_mutex_unlock(&countMutex);
	if (myCount == 1)
	{
		count = P;
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
                    
void Transform1D(Complex* h, int N, Complex* W, bool a)
{
  // Implement the efficient Danielson-Lanczos DFT here.
  // "h" is an input/output parameter
  // "N" is the size of the array (assume even power of 2)
  // "a" is a boolean that allows us to compute the forward (a = 1) or inverse (a = 0) transform
  Complex temp;
  for (int n = 2; n <= N; n = n * 2)
  {
	  for (int i = 0; i < N; i = i + n)
	  {
		  for (int j = 0; j < n/2; j++)
		  {
			  int t = n/2;
			  temp = h[i + j];
			  h[i + j] =  h[i + j] + W[j*N/n]*h[i + j + t];
			  h[i + j + t] = temp - W[j*N/n]*h[i + j + t];
		  }
	  }
  }
  
  if(a == 0)  //inverse transform
  {
	  for (int i = 0; i < N; i++)
	  {
		  h[i].real = h[i].real/N;
		  h[i].imag = h[i].imag/N;
	  }
  }
}

void calcweights()
{ // This function calculates the weights for the transform.
  
  if (state)
  {
	  for (int i = 0; i < N/2; i++)
	  {
		  W[i] = Complex(cos(2*M_PI*i/N), -sin(2*M_PI*i/N));
      }	  
  }
  
  else
  {
	  for (int i = 0; i < N/2; i++)
	  {
		  W[i] = Complex(cos(2*M_PI*i/N),sin(2*M_PI*i/N));
	  }
  }
  
}

void transposeof(Complex* in, Complex* out)
{
  int k = 0;
  for(int i = 0; i < N; i++)
  {
      for(int j = 0; j < N; j++)
      {
		  out[k] = in[i + j*N];
		  k++;
	  }
  }
  
  for (int i = 0; i < N*N; i++) 
  {
	  // Stores the transposed values in the input. Avoids having to declare multiple Complex* variables.
	  in[i] = out[i]; 
  }
}

void reversebits(Complex* in, Complex* out)
{ // This function reverses bits as required by the Danielson-Lanczos algorithm.
  for(int i = 0; i < N; i++)
  {
    int start = i*N;  // first element of each row
    Complex* t = new Complex[N];   // temporary matrix to store the bits from each row 
    for(int j = 0; j < N; j++)
    {
      t[j] = in[j + start];
    }
	
    for(int j = start; j < start + N; j++)
    {
      out[j] = t[ReverseBits(j)];  // reversing the bits from each row
    }
    delete [] t;
  }
  
  for (int i = 0; i < N*N; i++)
  {
	  // Stores the transposed values in the input. Avoids having to declare multiple Complex* variables.	  
	  in[i] = out[i];
  }
} 

void threadcreation()
{
  // Create 16 threads
  MyBarrier_Init();   // initialize MyBarrier
  for (int i = 0; i < numofThreads; ++i) // creating the threads
  {
	  pthread_t pt;
	  pthread_create(&pt,0,Transform2DTHread,(void*)i);
  }
}

void* Transform2DTHread(void* v)
{ // This is the thread starting point.  "v" is the thread number

  unsigned long int myId = (unsigned long) v; 
  
  // Calculating 1D DFT
  int st = myId*N/numofThreads; 
  for (int i = 0; i < N/numofThreads; i++)
  {
	  int start = N*(st + i);
	  Transform1D(imdata + start,N,W,state);
  }
  
  // Decrement active count and signal main if all complete
  MyBarrier(myId);
  return 0;
}

void Transform2D(const char* inputFN) 
{ // Do the 2D transform here.

  InputImage image(inputFN);  // Create the helper object for reading the image
  
  // Create the global pointer to the image array data
  imdata = image.GetImageData();
  imwidth = image.GetWidth();
  imheight = image.GetHeight();
  
  // Bit reversal
  reversebits(imdata,temp);
  
  // Forward Transform begins here...
  
  state = 1;
  
  // Calculating weights
  calcweights();
  
  // Create threads
  threadcreation(); 
  
  // Wait for all threads complete
  MyBarrier(16);
  
  // Writing 1-D transform data to file 
  cout << "1-D Transform Complete! Writing to file 'MyAfter1D.txt'..." << endl;
  image.SaveImageData("MyAfter1D.txt",imdata,imwidth,imheight);
  
  // Transposing 1-D transform in order to be able to compute column transforms
  transposeof(imdata,temp);

  // Bit reversal
  reversebits(imdata,temp);
  
  // Create threads
  threadcreation();
  
  // Wait for all threads to complete
  MyBarrier(16);
  
  // Calculate transpose to put transform back in order 
  transposeof(imdata,temp);
  
  //Write 2-D transform to file
  cout << "2-D Transform Complete! Writing to file 'MyAfter2D.txt'..." << endl;
  image.SaveImageData("MyAfter2D.txt",imdata,imwidth,imheight);
  
  // Forward Transform End
  
  // Inverse Transform begins here...
  
  state = 0; 
  
  // Calculating weights
  calcweights();
  
  // Bit reversal
  reversebits(imdata,temp);
  
  // Create threads
  threadcreation();
  
  // Wait for all threads to complete
  MyBarrier(16);
  
  // Calculate transpose
  transposeof(imdata,temp);

  // Bit reversal
  reversebits(imdata,temp);
  
  // Create threads
  threadcreation();
  
  // Wait for all threads to complete
  MyBarrier(16);
  
  // Calculate transpose
  transposeof(imdata,temp);
  
  //Write Inverse transform to file
  cout << "Inverse Transform Complete! Writing to file 'MyAfterInverse.txt'..." << endl;
  image.SaveImageData("MyAfterInverse.txt",imdata,imwidth,imheight);
  
  // Inverse Transform End 
  
  delete [] temp;
  delete [] W;
}

int main(int argc, char** argv)
{
  string fn("Tower.txt"); // default file name
  if (argc > 1) fn = string(argv[1]);  // if name specified on cmd line
  Transform2D(fn.c_str()); // Perform the transform.
}  
  

  
