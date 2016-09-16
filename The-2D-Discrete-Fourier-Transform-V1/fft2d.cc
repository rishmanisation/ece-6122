// Distributed two-dimensional Discrete FFT transform
// Rishabh Ananthan
// ECE6122 Project 1

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <signal.h>
#include <math.h>
#include <mpi.h>

#include "Complex.h"
#include "InputImage.h"

using namespace std;

void Transform1D(Complex* h, int w, Complex* H, int a)
{
	// Implement a simple 1-d DFT using the double summation equation
	// given in the assignment handout.  h is the time-domain input
	// data, w is the width (N), and H is the output array. 'a' is a 
	// parameter that causes the function to implement the forward or 
	// reverse transform depending on its value.
	Complex W;
	Complex sum(0,0);
	//Forward dft
	if(a)
	{
		for (int n = 0; n <= w - 1; n++)
		{
			for (int k = 0; k <= w - 1; k++)
			{	
				Complex W(cos(2*M_PI*n*k/w), -sin(2*M_PI*n*k/w));
				sum = sum + W*h[k];
			}
			H[n] = sum;
			sum = (0,0);
		}
	}
	
	//inverse dft
	else
	{
		for (int n = 0; n <= w - 1; n++)
		{
			for (int k = 0; k <= w - 1; k++)
			{
				W = Complex(cos(2*M_PI*n*k/w), sin(2*M_PI*n*k/w));
				sum = sum + h[k]*W;				
			}
			H[n] = sum;
			H[n].real = H[n].real/w;
			H[n].imag = H[n].imag/w;
			sum = (0,0);
		}
	}
	  
}

void sendToMainCPU(Complex* vec, int tasks, int rank, int size)
{
	int rc;
	//We want to send all data to CPU0
	if (rank != 0)
	{
		Complex* Vs = vec;
		MPI_Request request;
		rc = MPI_Isend(Vs,size*2/tasks,MPI_COMPLEX,0,0,MPI_COMM_WORLD,&request);
	}
	
	//If we are currently CPU0, we receive data from the other CPUs
	else
	{ 
		for (int i = 1; i < tasks; i++) //all CPUs except CPU0
		{
			Complex* Vr = vec + size*i/tasks;
			MPI_Status status;
			rc = MPI_Recv(Vr,size*2/tasks,MPI_COMPLEX,i,0,MPI_COMM_WORLD,&status);
		}
	}
}

void sendFromMainCPU(Complex* vec, int tasks, int rank, int size)
{
	int rc;
	//If we are CPU0, we send to all other CPUs 
	if (rank == 0)
	{
		for (int i = 1; i < tasks; i++) 
		{
			Complex* Vs = vec;
			MPI_Request request;
			rc = MPI_Isend(Vs,size*2,MPI_COMPLEX,i,0,MPI_COMM_WORLD,&request);
		}
	}
	
	//If we are not CPU0, we receive the data from CPU0
	else
	{
		Complex* Vr = vec;
		MPI_Status status;
		rc = MPI_Recv(Vr,size*2,MPI_COMPLEX,0,0,MPI_COMM_WORLD,&status);
	}
}

void transposeof(Complex* in, Complex* out, int w, int h, int rank)
{
	if (rank == 0)
	{
		int k = 0;
		for(int i = 0; i < h; i++)
		{
			for(int j = 0; j < w; j++)
			{
				out[k] = in[i + j*w];
				k++;
			}
		}
	}
}

void Transform2D(const char* inputFN) 
{ // Do the 2D transform here.
  // 1) Use the InputImage object to read in the Tower.txt file and
  //    find the width/height of the input image.
  // 2) Use MPI to find how many CPUs in total, and which one
  //    this process is
  // 3) Allocate an array of Complex object of sufficient size to
  //    hold the 2d DFT results (size is width * height)
  // 4) Obtain a pointer to the Complex 1d array of input data
  // 5) Do the individual 1D transforms on the rows assigned to your CPU
  // 6) Send the resultant transformed values to the appropriate
  //    other processors for the next phase.
  // 6a) To send and receive columns, you might need a separate
  //     Complex array of the correct size.
  // 7) Receive messages from other processes to collect your columns
  // 8) When all columns received, do the 1D transforms on the columns
  // 9) Send final answers to CPU 0 (unless you are CPU 0)
  //   9a) If you are CPU 0, collect all values from other processors
  //       and print out with SaveImageData().
  int nCPUs, rank;
  
  // Step (1)
  InputImage image(inputFN);  // Create the helper object for reading the image
  
  // Step (2)
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nCPUs);
  cout << "Number of tasks: " << nCPUs << " My rank: " << rank << endl;
  
  // Step (3)
  int w = image.GetWidth(); // image width
  int h = image.GetHeight(); // image height
  int s = w*h;
  Complex* output1D = new Complex[s]; // array to hold 1D DFT (of rows)
  Complex* output1Dt = new Complex[s]; // array to hold 1D DFT (of columns)
  Complex* output2Dt = new Complex[s]; // array to hold 2D DFT (transposed) results
  Complex* output2D = new Complex[s]; //array to hold 2D DFT results
  
  // Step (4)
  Complex* input = image.GetImageData();
  
  // Step (5)
  int rowsForCPU = h/nCPUs; // this is the number of rows assigned to each CPU
  int firstRowForCPU = rowsForCPU * rank; // this is the first row assigned to each CPU
  for (int i = 0; i < rowsForCPU; i++)
  {
	  Transform1D(input + (firstRowForCPU + i)*w,w,output1D + i*w,1);  //this contains the rows
  }
  sendToMainCPU(output1D,nCPUs,rank,s);
  if (rank == 0)
  {
	  cout << "1-D Transform image generated as MyAfter1D.txt!"<<endl;
	  image.SaveImageData("MyAfter1D.txt",output1D,w,h);
  }
  
  // Step (6)
  transposeof(output1D,output1Dt,w,h,rank); 
    
  // Step (7)
  sendFromMainCPU(output1Dt,nCPUs,rank,s);

  // Step (8)
  for (int i = 0; i < rowsForCPU; i++)
  {
	  Transform1D(output1Dt + (firstRowForCPU + i)*w,w,output2Dt + i*w,1);
  }
  
  // Step (9)
  sendToMainCPU(output2Dt,nCPUs,rank,s);
  transposeof(output2Dt,output2D,w,h,rank);
  if (rank == 0)
  {
	  cout << "2-D Transform image generated as MyAfter2D.txt!" << endl;
	  image.SaveImageData("MyAfter2D.txt",output2D,w,h);
  }
  
  // Inverse DFT
  sendFromMainCPU(output2D,nCPUs,rank,s); //sending the transformed matrix to all CPUs
  Complex* output1Di = new Complex[s];
  Complex* output1Dit = new Complex[s];
  Complex* output2Dit = new Complex[s];
  Complex* output2Di = new Complex[s];
  
  for (int i = 0; i < rowsForCPU; i++)
  {
	  Transform1D(output2D + (firstRowForCPU + i)*w,w,output1Di + i*w,0);
  }
  sendToMainCPU(output1Di,nCPUs,rank,s);
  transposeof(output1Di,output1Dit,w,h,rank);
  sendFromMainCPU(output1Dit,nCPUs,rank,s);
  
  for (int i = 0; i < rowsForCPU; i++)
  {
	  Transform1D(output1Dit + (firstRowForCPU + i)*w,w,output2Dit+ i*w,0);
  }
  sendToMainCPU(output2Dit,nCPUs,rank,s);
  transposeof(output2Dit,output2Di,w,h,rank);
  
  if (rank == 0)
  {
	  cout << "2-D Inverse Transform image generated as MyAfterInverse.txt!" << endl;
	  image.SaveImageDataReal("MyAfterInverse.txt",output2Di,w,h);
  }
}

int main(int argc, char** argv)
{
  int rc;
  string fn("Tower.txt"); // default file name
  if (argc > 1) fn = string(argv[1]);  // if name specified on cmd line
  // MPI initialization here
  rc = MPI_Init(&argc,&argv);
  if (rc != MPI_SUCCESS)
  {
	  cout << "Error starting program. Terminating.\n";
	  MPI_Abort(MPI_COMM_WORLD,rc);
  }
  Transform2D(fn.c_str()); // Perform the transform.
  // Finalize MPI here
  MPI_Finalize();
}  
  

  
