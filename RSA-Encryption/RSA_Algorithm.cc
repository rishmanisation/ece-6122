// RSA Assignment for ECE4122/6122 Fall 2015

#include <iostream>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "RSA_Algorithm.h"

using namespace std;

// Implement the RSA_Algorithm methods here

// Constructor
RSA_Algorithm::RSA_Algorithm()
  : rng(gmp_randinit_default)
{
  // get a random seed for the random number generator
  int dr = open("/dev/random", O_RDONLY);
  if (dr < 0)
    {
      cout << "Can't open /dev/random, exiting" << endl;
      exit(0);
    }
  unsigned long drValue;
  read(dr, (char*)&drValue, sizeof(drValue));
  //cout << "drValue " << drValue << endl;
  rng.seed(drValue);
// No need to init n, d, or e.
}

// Fill in the remainder of the RSA_Algorithm methods
void RSA_Algorithm::GeneratePrime(mpz_class& x, size_t sz)
{
  bool a = false;
  while(!a) // keep looping until x is a prime number
    {
      x = rng.get_z_bits(sz);
      a = (mpz_probab_prime_p(x.get_mpz_t(),100) != 0);
    }
}

void RSA_Algorithm::GenerateRandomKeyPair(size_t sz)
{
  mpz_class phi,dd,ee,GCD,p,q;
  
  // Create two prime numbers 'p' and 'q'
  GeneratePrime(p,sz);
  GeneratePrime(q,sz);        

  // Compute n = p * q
  n = p * q;
  
  // Compute phi = (p-1)*(q-1)
  phi = (p - 1) * (q - 1);

  // Choose e of sz*2 such that e < phi and gcd(e,phi) = 1
  mpz_class SZ = sz*2;
  bool a = false;
  while(!a)
    {
      ee = rng.get_z_bits(SZ);
      mpz_gcd(GCD.get_mpz_t(),ee.get_mpz_t(),phi.get_mpz_t());
      a = ((GCD == 1) && (ee < phi));
    }
  e = ee;
 
  // Compute d as the multiplicative inverse of e*mod(phi)
  mpz_invert(dd.get_mpz_t(),e.get_mpz_t(),phi.get_mpz_t());
  d = dd;
}

mpz_class RSA_Algorithm::Encrypt(mpz_class M)
{
  // Compute cipher as c = (m^d)*mod(n)
  mpz_class c;
  mpz_powm(c.get_mpz_t(),M.get_mpz_t(),e.get_mpz_t(),n.get_mpz_t());
  return c;
}

mpz_class RSA_Algorithm::Decrypt(mpz_class C)
{
  // Compute decrypted message 
  mpz_class dM;
  mpz_powm(dM.get_mpz_t(),C.get_mpz_t(),d.get_mpz_t(),n.get_mpz_t());
  return dM;
}

mpz_class RSA_Algorithm::Factorization(mpz_class N)
{
  // Using Pollard's rho algorithm

  /* Algorithm works as follows:
     f(x) = (x^2+1)mod n
     Initialize: x = y = 2, d = 1
     while d = 1: x = f(x), y = f(f(y)), d = gcd(|x-y|,n)
     if d = n: fail
     else: return d
  */

  // Initialization of values
  mpz_class x = 2;
  mpz_class y = 2;
  mpz_class d = 1;
  while (d == 1)
    {
      mpz_class fy,abs;
      x = (x*x + 1) % N;
      fy = (y*y + 1) % N;
      y = (fy*fy + 1) % N;
      if (y > x)
	{
	  abs = y - x;
	}
      else
	{
	  abs = x - y;
	}

      // Calculate GCD
      mpz_gcd(d.get_mpz_t(),abs.get_mpz_t(),N.get_mpz_t());
    }
  bool a = (d != n);
  if (a)
    return d;
  else
    return 0;
}

void RSA_Algorithm::PrintND()
{ // Do not change this, right format for the grading script
  cout << "n " << n << " d " << d << endl;
}

void RSA_Algorithm::PrintNE()
{ // Do not change this, right format for the grading script
  cout << "n " << n << " e " << e << endl;
}

void RSA_Algorithm::PrintNDE()
{ // Do not change this, right format for the grading script
  cout << "n " << n << " d " << d << " e " << e << endl;
}

void RSA_Algorithm::PrintM(mpz_class M)
{ // Do not change this, right format for the grading script
  cout << "M " << M << endl;
}

void RSA_Algorithm::PrintC(mpz_class C)
{ // Do not change this, right format for the grading script
  cout << "C " << C << endl;
}




