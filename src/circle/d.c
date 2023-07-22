/*

https://www.shadertoy.com/view/3ltSW2
 Disk - distance 2D 
 https://iquilezles.org/articles/distfunctions2d/
 2D distance functions
 
 https://www.youtube.com/watch?v=pEdlZ9W2Xs0
 explanation : Shader Tutorial | Intro to Signed Distance Fields by Suboptimal Engineer

  Adam Majewski
  adammaj1 aaattt o2 dot pl  // o like oxygen not 0 like zero 
  
  graphic as an array of pixel colors
	* creata array
	* fill array
	* save array as a pgm file
	* remove array
	
	
  
  
  ---------------------------------
  indent d.c 
  default is gnu style 
  -------------------



  c console progam 
  
  	
  gcc d.c -lm -Wall -march=native 
  time ./a.out > j.txt


  gcc d.c -lm -Wall -march=native 




 convert -limit memory 1000mb -limit disk 1gb dd30010000_20_3_0.90.pgm -resize 2000x2000 10.png
*/




#include <stdio.h>
#include <stdlib.h>		// malloc
#include <string.h>		// strcat
#include <math.h>		// M_PI; needs -lm also
#include <complex.h>
#include <limits.h>		// Maximum value for an unsigned long long int

// https://sourceforge.net/p/predef/wiki/Standards/

#if defined(__STDC__)
#define PREDEF_STANDARD_C_1989
#if defined(__STDC_VERSION__)
#if (__STDC_VERSION__ >= 199409L)
#define PREDEF_STANDARD_C_1994
#endif
#if (__STDC_VERSION__ >= 199901L)
#define PREDEF_STANDARD_C_1999
#endif
#endif
#endif

/* --------------------------------- global variables and consts ------------------------------------------------------------ */
#define PROGRAM_VERSION 20230409

int NumberOfImages = 0;


// virtual 2D array and integer ( screen) coordinate
// Indexes of array starts from 0 not 1 
//unsigned int ix, iy; // var

static unsigned int iHeight = 1000;	//  
static unsigned int iyMin = 0;	// Indexes of array starts from 0 not 1
static unsigned int iyMax;	//

static unsigned int ixMin = 0;	// Indexes of array starts from 0 not 1
static unsigned int ixMax;	//
static unsigned int iWidth;	// horizontal dimension of array




// The size of array has to be a positive constant integer 
static unsigned long long int iSize;	// = iWidth*iHeight; 

// memmory 1D array for 8 bit color 
unsigned char *data8bit;
unsigned char *data8bit2;
unsigned char *edge;


// rgb array = 24bit color = 3 bytes
int iColorSize = 3 ; // RGB = 3*(unsigned char)
unsigned int iSize_rgb; // number of elements in rgb array
unsigned char *data24bit; // for ppm file



double distanceMax = -10.0;;
double distanceMin = +10.0;



// unsigned int i; // var = index of 1D array
//static unsigned int iMin = 0; // Indexes of array starts from 0 not 1
unsigned int iMax;	// = i2Dsize-1  = 
// The size of array has to be a positive constant integer 
// unsigned int i1Dsize ; // = i2Dsize  = (iMax -iMin + 1) =  ;  1D array with the same size as 2D array





// see SetPlane

double plane_radius = 2.0; 
complex double plane_center = 0.0 ;
double  DisplayAspectRatio  = 1.0; // https://en.wikipedia.org/wiki/Aspect_ratio_(image)
// dx = dy compare setup : iWidth = iHeight;
double ZxMin; //= -1.3;	//-0.05;
double ZxMax;// = 1.3;	//0.75;
double ZyMin;// = -1.3;	//-0.1;
double ZyMax;// = 1.3;	//0.7;
double PixelWidth;	// =(ZxMax-ZxMin)/ixMax;
double PixelHeight;	// =(ZyMax-ZyMin)/iyMax;

// dem
double BoundaryWidth ; //= 1.0*iWidth/2000.0  ; //  measured in pixels ( when iWidth = 2000) 
double distanceMax ; //= BoundaryWidth*PixelWidth;


double ratio; 




double PixelWidth2;
double circle_width;

 
 
/* colors = shades of gray from 0 to 255 */
unsigned char iColorOfBasin1 = 255;
unsigned char iColorOfInterior = 150;
unsigned char iColorOfExterior = 225;



// rgb colors 		r   , g,    b	in [0,1] range
// one can change it here
double exterior24[3] = {0.9 , 0.58, 0.25};
double interior24[3] = {0.65, 0.85, 1.0}; // vec3(0.65,0.85,1.0);
 

unsigned char iColorOfBoundary = 0;
unsigned char iColorOfUnknown = 5;

// pixel counters; pixel counters work not good with OpenMP !!!
unsigned long long int uUnknown = 0;
unsigned long long int uInterior = 0;
unsigned long long int uExterior = 0;



// for MBD
static double TwoPi=2.0*M_PI; // texture
//double t0 ; //  t for MBD
// see https://www.youtube.com/watch?v=JttLtB0Gkdk&t=894s
// 


double p_step = 0.1; // for discrete gradient




// ************************************************** functions *********************************************************************888

// The function mix(x, y, a) calculates always x * (1−a) + y * a.
double mix(const double x, const double y, const double a){

	return x*(1.0 - a) +y*a;


}


double clamp(const double x, const double lowerlimit , const double upperlimit ) {
  if (x < lowerlimit) return lowerlimit;
  if (x > upperlimit) return upperlimit;
  return x;
}

// https://en.wikipedia.org/wiki/Smoothstep
double smoothstep (const double  x0, const double edge0 , const double edge1 ) {
   // Scale, and clamp x to 0..1 range
   double x = clamp((x0 - edge0) / (edge1 - edge0), 0.0, 1.0);

   return x * x * (3.0f - 2.0f * x);
}






// fractional part of real (double) number
double frac(double d){

	double fraction = d - ((long)d);
	return fraction;
}


// for discrete gradient, smth like round(double>1.0) but for double in range [0,1]
double double2steps(double p){

	double s = p / p_step;
	s = s - frac(s);
	s *= p_step;
	return s;


}


double c_arg(complex double z)
{
  double arg;
  arg = carg(z);
  if (arg<0.0) arg+= TwoPi ; 
  return arg; 
}

double c_turn(complex double z)
{
  double arg;
  arg = c_arg(z);
  return arg/TwoPi; 
}



double cabs2(complex double z){

  return creal(z)*creal(z)+cimag(z)*cimag(z);
}





int is_z_outside(complex double z){

  if (creal(z) >ZxMax ||
      creal(z) <ZxMin ||
      cimag(z) >ZyMax ||
      cimag(z) <ZyMin)
    {return 1; } // is outside = true
      
    
  return 0; // is inside = false
}



// from screen to world coordinate ; linear mapping
// uses global cons
double GiveZx (int ix)
{
  return (ZxMin + ix * PixelWidth);
}

// uses globaal cons
double GiveZy (int iy)
{
  return (ZyMax - iy * PixelHeight);
}				// reverse y axis


complex double GiveZ (int ix, int iy)
{
  double Zx = GiveZx (ix);
  double Zy = GiveZy (iy);

  return Zx + Zy * I;

}

//------------------complex numbers -----------------------------------------------------




/* -----------  array functions = drawing -------------- */

/* gives position of 2D point (ix,iy) in 1D array  ; uses also global variable iWidth */
unsigned int Give_i (unsigned int ix, unsigned int iy)
{
  return ix + iy * iWidth;
}






// ****************************************************************************************************
// ***************************** 8bitTestImage = unit circle ***********************************************************
// *******************************************************************************************************




unsigned char Compute8BitColorForTestImage(complex double z){

  unsigned char iColor = 255;
  // draw a unit circle
  // In the complex plane, a circle with a centre at c = 0 and radius r = 1 has the equation
  //|z - c| = r 
  
  
  // Composite Regions in the Complex Plane https://www.nagwa.com/en/explainers/320146108949/
  if (cabs(z) < (1.0 + circle_width) && cabs(z) >   (1.0 - circle_width) ) // 
  	iColor = iColorOfBoundary;
  	else {iColor = iColorOfBasin1;}
  	
	
	
  
  return iColor;



}






// plots raster point (ix,iy) 
int Draw8bitPointForTestImage ( unsigned char A[],  int ix, int iy)
{
  int i;			/* index of 1D array */
  unsigned char iColor;
  complex double z;


  i = Give_i (ix, iy);		/* compute index of 1D array from indices of 2D array */
  if(i<0 && i> iMax)
    { return 1;}
  
  z = GiveZ(ix,iy);
  iColor = Compute8BitColorForTestImage( z);
  A[i] = iColor ;		// 
  		
  	  
  return 0;
}



// change color of all pixels of the image
int Draw8bitTestImage ( unsigned char A[])
{
  unsigned int ix, iy;		// pixel coordinate 

  fprintf (stderr, "compute image \n");
  // for all pixels of image 

  for (iy = iyMin; iy <= iyMax; ++iy)
    {
      fprintf (stderr, " %d from %d \r", iy, iyMax);	//info 
      for (ix = ixMin; ix <= ixMax; ++ix)
	Draw8bitPointForTestImage(A, ix, iy);	//  
    }
  fprintf (stderr, "\n");	//info 
  return 0;
}






// ****************************************************************************************************
// **************************  Signed distance to a disk *************************************************
// *******************************************************************************************************








// Signed distance to a disk

// List of some other 2D distances: https://www.shadertoy.com/playlist/MXdSRf
//
// and iquilezles.org/articles/distfunctions2d


double  SignedDistance2Circle( const complex double z, const complex double circle_center, const double circle_radius ) 
{
    return cabs(z - circle_center) - circle_radius;
}




// ****************************************************************************************************
// ***************************** 8 bit *******************************************************************
// *******************************************************************************************************

// -------------------------------------- 8 bit continous ----------------------------------
unsigned char  Compute8BitColorForSignedDistance2Circle(const complex double z){

	unsigned char iColor = 255;
	double sdf = SignedDistance2Circle(z, 0.0, 1.0);
	
	if (sdf > distanceMax) {distanceMax = sdf;}
	if (sdf < distanceMin) {distanceMin = sdf;}
	
	if (sdf < 0.0) // interior
		iColor = (unsigned char) (fabs(sdf) * 255.0);
		// exterior: remove banding 
		else {iColor = (unsigned char) (sdf * 255.0/ plane_radius );}
	
	return iColor;
}

// plots raster point (ix,iy) 
int Draw8bitPointForSignedDistance2Circle ( unsigned char A[],  const int ix, const int iy)
{
  int i;			/* index of 1D array */
  unsigned char iColor;
  complex double z;


  i = Give_i (ix, iy);		/* compute index of 1D array from indices of 2D array */
  if(i<0 && i> iMax)
    { return 1;}
  
  z = GiveZ(ix,iy);
  iColor = Compute8BitColorForSignedDistance2Circle( z);
  A[i] = iColor ;		// 
  		
  	  
  return 0;
}



// change color of all pixels of the image
int Draw8bitSignedDistance2Circle ( unsigned char A[])
{
  unsigned int ix, iy;		// pixel coordinate 

  fprintf (stderr, "compute image \n");
  // for all pixels of image 

  for (iy = iyMin; iy <= iyMax; ++iy)
    {
      fprintf (stderr, " %d from %d \r", iy, iyMax);	//info 
      for (ix = ixMin; ix <= ixMax; ++ix)
	Draw8bitPointForSignedDistance2Circle(A, ix, iy);	//  
    }
  fprintf (stderr, "\n");	//info 
  return 0;
}


// ------------------------------------------
// draws only cirlce, does not change other pixels
int Draw8bitCircle ( unsigned char A[], const unsigned char iColor)
{
  unsigned int ix, iy;		// pixel coordinate 

  fprintf (stderr, "draw 8 bit circle \n");
  // for all pixels of image 

  for (iy = iyMin; iy <= iyMax; ++iy)
    {
      fprintf (stderr, " %d from %d \r", iy, iyMax);	//info 
      for (ix = ixMin; ix <= ixMax; ++ix)
      	{
      		// Draw8bitPointForSignedDistance2Circle(A, ix, iy);	//  
      		complex double z = GiveZ(ix,iy);
		// Composite Regions in the Complex Plane https://www.nagwa.com/en/explainers/320146108949/
  		if (cabs(z) < (1.0 + circle_width) && cabs(z) >   (1.0 - circle_width) ) // 
      			{
      				int i = Give_i (ix, iy);		/* compute index of 1D array from indices of 2D array */
      				
      				A[i] = iColor;
      			
      			
      			}
      	}
    }
  fprintf (stderr, "\n");	//info 
  return 0;
}


// ---------------------------------------------------------------------------------------------------
//------------------------------------------- discrete ----------------------------------------------
// ----------------------------------------------------------------------------------------------------


unsigned char  Compute8BitDiscreteColorForSignedDistance2Circle(const complex double z){

	unsigned char iColor = 255;
	double sdf = SignedDistance2Circle(z, 0.0, 1.0);
	
	sdf = double2steps(sdf);
	
	if (sdf < 0.0) // interior
		iColor = (unsigned char) (fabs(sdf) * 255.0);
		// exterior: remove banding 
		else {iColor = (unsigned char) (sdf * 255.0/ plane_radius );}
	
	return iColor;
}

// plots raster point (ix,iy) 
int Draw8bitDiscretePointForSignedDistance2Circle ( unsigned char A[],  const int ix, const int iy)
{
  int i;			/* index of 1D array */
  unsigned char iColor;
  complex double z;


  i = Give_i (ix, iy);		/* compute index of 1D array from indices of 2D array */
  if(i<0 && i> iMax)
    { return 1;}
  
  z = GiveZ(ix,iy);
  iColor = Compute8BitDiscreteColorForSignedDistance2Circle( z);
  A[i] = iColor ;		// 
  		
  	  
  return 0;
}



// change color of all pixels of the image
int Draw8bitDiscreteSignedDistance2Circle ( unsigned char A[])
{
  unsigned int ix, iy;		// pixel coordinate 

  fprintf (stderr, "compute image \n");
  // for all pixels of image 

  for (iy = iyMin; iy <= iyMax; ++iy)
    {
      fprintf (stderr, " %d from %d \r", iy, iyMax);	//info 
      for (ix = ixMin; ix <= ixMax; ++ix)
	Draw8bitDiscretePointForSignedDistance2Circle(A, ix, iy);	//  
    }
  fprintf (stderr, "\n");	//info 
  return 0;
}




// ****************************************************************************************************
// *****************************24 bit *******************************************************************
// *******************************************************************************************************

// https://www.shadertoy.com/view/3ltSW2
// Disk - distance 2D  by iq
// here output is rgb = array[3] of unsigned char
int  Compute24BitColorForSignedDistance2Circle(const complex double z, unsigned char RGB[]){

	double sdf = SignedDistance2Circle(z, 0.0, 1.0);
	double dRGB[3] = {0.0}; // vec3 col
	// 230, 150,64 or 169, 209,138
	// vec3 col = (d>0.0) ? vec3(0.9,0.6,0.25) : vec3(0.65,0.85,1.0);
	if (sdf > 0.0) // exterior / interior
		{	// exterior
			dRGB[0] = exterior24[0];
			dRGB[1] = exterior24[1];
			dRGB[0] = exterior24[2];
		
		}
		else {
			// interior, read from const array
			dRGB[0] = interior24[0];
			dRGB[1] = interior24[1];
			dRGB[0] = interior24[2];
		
		}
		
		
		for (int i =0; i<3; ++i)
			{
				if (sdf<0.0) 
					{dRGB[i] *= 1.0 -  clamp(exp(sdf), 0.0, 1.0);}
					else {dRGB[i] *= 1.0 -  clamp(exp(-sdf), 0.0, 1.0);}
				// RGB[i] *= 1.0 - clamp(exp(sdf), 0.0, 1.0); // col *= 1.0 - exp(-6.0*abs(d)); // adding a black outline to the circle
				dRGB[i] *= 0.8 + 0.2*cos(150.0*sdf); //  adding waves
				//dRGB[i] = mix(dRGB[i], 1.0, 1.0 -smoothstep(0.0,0.01,abs(sdf))   ); //  note: adding white border to the circle
			
			}
		
		
	        /* coloring
    			vec3 col = (d>0.0) ? 
    				vec3(0.9,0.6,0.3) : vec3(0.65,0.85,1.0);
    				
    				
    			col *= 1.0 - exp(-6.0*abs(d));
			col *= 0.8 + 0.2*cos(150.0*d);
			col = mix( col, vec3(1.0), 1.0-smoothstep(0.0,0.01,abs(d)) );
		*/
		//iColor = (unsigned char) (fabs(sdf) * 255.0);
		// exterior: remove banding 
		//else {iColor = (unsigned char) (sdf * 255.0/ plane_radius );}
	// change range
	RGB[0] = 255 * dRGB[0];
	RGB[1] = 255 * dRGB[1];
	RGB[2] = 255 * dRGB[2];
	
	return 0;
}

// plots raster point (ix,iy) 
int Draw24bitPointForSignedDistance2Circle ( unsigned char A[],  int ix, int iy)
{
  int i;			/* index of 1D array */
  
  complex double z;
  unsigned char rgb[3];

  i = Give_i (ix, iy);		/* compute index of 1D array from indices of 2D array */
  int iC = i*iColorSize; // compute index of F array
  //if(i<0 && i> iMax)
    //{ return 1;}
  
  z = GiveZ(ix,iy);
  Compute24BitColorForSignedDistance2Circle( z, rgb);
  A[iC]   = rgb[0];
  A[iC+1] = rgb[1];		// 
  A[iC+2] = rgb[2];		
  	  
  return 0;
}




// change color of all pixels of the image
int Draw24bitSignedDistance2Circle ( unsigned char A[])
{
  unsigned int ix, iy;		// pixel coordinate 

  fprintf (stderr, "compute image \n");
  // for all pixels of image 

  for (iy = iyMin; iy <= iyMax; ++iy)
    {
      fprintf (stderr, " %d from %d \r", iy, iyMax);	//info 
      for (ix = ixMin; ix <= ixMax; ++ix)
	Draw24bitPointForSignedDistance2Circle(A, ix, iy);	//  
    }
  fprintf (stderr, "\n");	//info 
  return 0;
}



// ****************************************************************************************************
// ***************************** 24 bit white circle  ***********************************************************
// *******************************************************************************************************



// plots raster point (ix,iy) !!! only if it is inside circle 
int Draw24bitWhiteCirclePoint ( unsigned char A[],  int ix, int iy)
{
  int i;			/* index of 1D array */
  complex double z;


  i = Give_i (ix, iy);		/* compute index of 1D 8 bit array from indices of 2D array */
  if(i<0 && i> iMax)
    { return 1;}
  int iC = i*iColorSize; // compute index of 1D 24 bit  array
  
  z = GiveZ(ix,iy);
  // draw a unit circle
  // In the complex plane, a circle with a centre at c = 0 and radius r = 1 has the equation
  //|z - c| = r 
  
  // change not all but only circle pixels
  // Composite Regions in the Complex Plane https://www.nagwa.com/en/explainers/320146108949/
  if (cabs(z) < (1.0 + circle_width) && cabs(z) >   (1.0 - circle_width) ) // 
  	{	// set pixel white
  		A[iC] = 255 ;
  	  	A[iC+1] = 255 ;		// 
  	  	A[iC+2] = 255 ;
  	}
  		
  	  
  return 0;
}



// change color of all pixels of the image
int Draw24bitWhiteCircle ( unsigned char A[])
{
  unsigned int ix, iy;		// pixel coordinate 

  fprintf (stderr, "compute image \n");
  // for all pixels of image 

  for (iy = iyMin; iy <= iyMax; ++iy)
    {
      fprintf (stderr, " %d from %d \r", iy, iyMax);	//info 
      for (ix = ixMin; ix <= ixMax; ++ix)
	Draw24bitWhiteCirclePoint(A, ix, iy);	//  
    }
  fprintf (stderr, "\n");	//info 
  return 0;
}







// ****************************************************************************************************
// ******************************************************************************************************
// *******************************************************************************************************





int PlotPoint(const complex double z, unsigned char A[]){

	
  unsigned int ix = (creal(z)-ZxMin)/PixelWidth;
  unsigned int iy = (ZyMax - cimag(z))/PixelHeight;
  unsigned int i = Give_i(ix,iy); /* index of _data8bit array */
	
  	
	
  if(i>-1 && i< iMax)
    {A[i]= 0; // 255-A[i];
    }
	
	
  return 0;
	
}






int IsInsideCircle (int x, int y, int xplane_center, int yplane_center, int r){

	
  double dx = x- xplane_center;
  double dy = y - yplane_center;
  double d = sqrt(dx*dx+dy*dy);
  if (d<r) {    return 1;}
  return 0;
	  

} 

// Big point = disk 
int PlotBigPoint(const complex double z, double p_size, unsigned char A[]){

	
  unsigned int ix_seed = (creal(z)-ZxMin)/PixelWidth;
  unsigned int iy_seed = (ZyMax - cimag(z))/PixelHeight;
  unsigned int i;
  //unsigned char temp;
	
	
  if (  is_z_outside(z)) 
    {fprintf (stdout,"PlotBigPoint :  z= %.16f %+.16f*I is outside\n", creal(z), cimag(z)); return 1;} // do not plot	
	
  /* mark seed point by big pixel */
  int iSide =p_size*iWidth/2000.0 ; /* half of width or height of big pixel */
  int iY;
  int iX;
  for(iY=iy_seed-iSide;iY<=iy_seed+iSide;++iY){ 
    for(iX=ix_seed-iSide;iX<=ix_seed+iSide;++iX){ 
      if (IsInsideCircle(iX, iY, ix_seed, iy_seed, iSide)) {
	i= Give_i(iX,iY); /* index of _data8bit array */
	//if(i>-1 && i< iMax)
	//temp = A[i];
	//if ( temp > 110 && temp < 160)
	//	{ A[i] = 0; }
	//	else {A[i]= 255 - temp  ; }
	A[i] = 0; 
      }
      // else {printf(" bad point \n");}
	
    }}
	
	
  return 0;
	
}






// ***********************************************************************************************
// ********************** edge detection usung Sobel filter ***************************************
// ***************************************************************************************************

// from Source to Destination
int ComputeBoundaries(unsigned char S[], unsigned char D[])
{
 
  unsigned int iX,iY; /* indices of 2D virtual array (image) = integer coordinate */
  unsigned int i; /* index of 1D array  */
  /* sobel filter */
  unsigned char G, Gh, Gv; 
  // boundaries are in D  array ( global var )
 
  // clear D array
  memset(D, iColorOfBasin1, iSize*sizeof(*D)); // for heap-allocated arrays, where N is the number of elements = FillArrayWithColor(D , iColorOfBasin1);
 
  // printf(" find boundaries in S array using  Sobel filter\n");   

  for(iY=1;iY<iyMax-1;++iY){ 
    for(iX=1;iX<ixMax-1;++iX){ 
      Gv= S[Give_i(iX-1,iY+1)] + 2*S[Give_i(iX,iY+1)] + S[Give_i(iX-1,iY+1)] - S[Give_i(iX-1,iY-1)] - 2*S[Give_i(iX-1,iY)] - S[Give_i(iX+1,iY-1)];
      Gh= S[Give_i(iX+1,iY+1)] + 2*S[Give_i(iX+1,iY)] + S[Give_i(iX-1,iY-1)] - S[Give_i(iX+1,iY-1)] - 2*S[Give_i(iX-1,iY)] - S[Give_i(iX-1,iY-1)];
      G = sqrt(Gh*Gh + Gv*Gv);
      i= Give_i(iX,iY); /* compute index of 1D array from indices of 2D array */
      if (G==0) {D[i]=255;} /* background */
      else {D[i]=0;}  /* boundary */
    }
  }
 
   
 
  return 0;
}



// copy from Source to Destination
int CopyBoundaries(unsigned char S[],  unsigned char D[])
{
 
  unsigned int iX,iY; /* indices of 2D virtual array (image) = integer coordinate */
  unsigned int i; /* index of 1D array  */
 
 
  //printf("copy boundaries from S array to D array \n");
  for(iY=1;iY<iyMax-1;++iY)
    for(iX=1;iX<ixMax-1;++iX)
      {i= Give_i(iX,iY); if (S[i]==0) D[i]=0;}
 
 
 
  return 0;
}


// *******************************************************************************************
// ********************************** save grey A array to pgm file ****************************
// *********************************************************************************************

int Save8bitArray2PGMFile (const unsigned char A[], const char * file_base_name, const unsigned int iHeight, const char * file_comment)
{

  FILE *fp;
  const unsigned int MaxColorComponentValue = 255;	/* color component is coded from 0 to 255 ;  it is 8 bit color file */
  char name[200];		/* name of file */
  snprintf (name, sizeof name, "%s_%d", file_base_name,  iHeight);	/* plane_radius and iHeght are global variables */
  //fprintf(stdout, "name = %s\n", name);
   
  char *filename = strcat (name, ".pgm");
  //fprintf(stdout, "filename = %s\n", filename);
  char long_comment[200]; // to long comment can cause: "*** stack smashing detected ***: terminated"
  sprintf (long_comment, "%s", file_comment); // f_description is global var





  // save image array to the pgm file 
  fp = fopen (filename, "wb");	// create new file,give it a name and open it in binary mode 
  fprintf (fp, "P5\n # %s\n %u %u\n %u\n", long_comment, iWidth, iHeight, MaxColorComponentValue);	// write header to the file
  size_t rSize = fwrite (A, sizeof(A[0]), iSize, fp);	// write whole array with image data8bit bytes to the file in one step 
  fclose (fp);

  // info 
  if ( rSize == iSize) 
    {
      printf ("File %s saved ", filename);
      if (long_comment == NULL || strlen (long_comment) == 0)
	printf ("\n");
      else { printf (". Comment = %s \n", long_comment); }
    }
  else {printf("wrote %zu elements out of %llu requested\n", rSize,  iSize);}
  
  NumberOfImages +=1; // count images using global variable	

  return 0;
}


// *******************************************************************************************
// ********************************** save A array to ppm file ****************************
// *********************************************************************************************



int Save24bitArray2PPMFile( const unsigned char A[], const char* file_base_name, unsigned int iHeight, const char* comment  )
{
  
  	FILE * fp;
  
  	char name [100]; /* name of file */
  	snprintf(name, sizeof name, "%s_%d", file_base_name, iHeight); /*  */
	char *filename =strcat(name,".ppm");
	
	char long_comment[200];
  	sprintf (long_comment, " %s", comment);
  
  
  
  	// save image to the pgm file 
  	fp= fopen(filename,"wb"); // create new file,give it a name and open it in binary mode 
  
  	if (!fp ) { fprintf( stderr, "ERROR saving ( cant open) file %s \n", filename); return 1; }
  	// else
  	fprintf(fp,"P6\n%d %d\n255\n",  iWidth, iHeight);  // write header to the file
  	size_t rSize = fwrite(A, sizeof(A[0]), iSize_rgb,  fp);  // write array with image data bytes to the file in one step 
  	fclose(fp); 
  
  	// info 
  	if ( rSize == iSize_rgb) 
  		{
  			printf ("File %s saved ", filename);
  			if (long_comment == NULL || strlen (long_comment) == 0)
   				{printf ("\n"); }
  				else { printf (". Comment = %s \n", long_comment); }
  		}
  		else {printf("wrote %zu elements out of %u requested\n", rSize,  iSize_rgb);}
  	
  	
  	// 
	NumberOfImages +=1; // count images using global variable	
    
	return 0;
}


// *******************************************************************************************
// **********************************  ****************************
// *********************************************************************************************





int PrintCInfo ()
{

  printf ("gcc version: %d.%d.%d\n", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);	// https://stackoverflow.com/questions/20389193/how-do-i-check-my-gcc-c-compiler-version-for-my-eclipse
  // OpenMP version is displayed in the console : export  OMP_DISPLAY_ENV="TRUE"

  printf ("__STDC__ = %d\n", __STDC__);
  printf ("__STDC_VERSION__ = %ld\n", __STDC_VERSION__);
  printf ("c dialect = ");
  switch (__STDC_VERSION__)
    {				// the format YYYYMM 
    case 199409L:
      printf ("C94\n");
      break;
    case 199901L:
      printf ("C99\n");
      break;
    case 201112L:
      printf ("C11\n");
      break;
    case 201710L:
      printf ("C18\n");
      break;
      //default : /* Optional */

    }

  return 0;
}


int
PrintProgramInfo ()
{


  // display info messages
  fprintf (stdout, "Program version = %d  \n", PROGRAM_VERSION ); // https://stackoverflow.com/questions/12509038/can-i-print-defines-given-their-values-in-c
  
  
  double ImageWidth = (ZxMax - ZxMin);
  	
  	
  
			
  fprintf (stdout, "Image Width = %f in world coordinate\n", ImageWidth);
  fprintf (stdout, "PixelWidth = %.16f \n", PixelWidth);

  fprintf (stdout, "plane description \n");
  fprintf (stdout, "\tplane_center z =  %.16f %+.16f*i  and plane_radius = %.16f \n", creal (plane_center), cimag (plane_center), plane_radius);
  // plane_center and plane_radius
  // plane_center and zoom
  // GradientRepetition
  
  fprintf (stdout, "ratio of image  = %f ; it should be 1.000 ...\n", ratio);
  
 
  //}
  fprintf (stdout, "Unknown pixels = %llu = %.16f * iSize \n", uUnknown, ((double) uUnknown)/iSize  );
  fprintf (stdout, "Exterior pixels = %llu = %.16f * iSize \n", uExterior, ((double) uExterior)/iSize  );
  fprintf (stdout, "Interior pixels = %llu = %.16f * iSize \n", uInterior, ((double) uInterior)/iSize  );
  //printf("Number of images = %d \n", NumberOfImages);	
  fprintf (stdout, "sdf Max = %f \t sdf min = %f\n", distanceMax, distanceMin);

  return 0;
}



int SetPlane(complex double plane_center, double plane_radius, double a_ratio){

  ZxMin = creal(plane_center) - plane_radius*a_ratio;	
  ZxMax = creal(plane_center) + plane_radius*a_ratio;	//0.75;
  ZyMin = cimag(plane_center) - plane_radius;	// inv
  ZyMax = cimag(plane_center) + plane_radius;	//0.7;
  return 0;

}



// Check Orientation of z-plane image : mark first quadrant of complex plane 
// it should be in the upper right position
// uses global var :  ...
int CheckZPlaneOrientation(unsigned char A[] )
{
 
  double Zx, Zy; //  Z= Zx+ZY*i;
  unsigned i; /* index of 1D array */
  unsigned int ix, iy;		// pixel coordinate 
	
  fprintf(stderr, "compute image CheckOrientation\n");
  // for all pixels of image 

  for (iy = iyMin; iy <= iyMax; ++iy){
    //fprintf (stderr, " %d from %d \r", iy, iyMax);	//info 
    for (ix = ixMin; ix <= ixMax; ++ix){
      // from screen to world coordinate 
      Zy = GiveZy(iy);
      Zx = GiveZx(ix);
      i = Give_i(ix, iy); /* compute index of 1D array from indices of 2D array */
      if (Zx>0 && Zy>0) A[i]=255-A[i];   // check the orientation of Z-plane by marking first quadrant */
    }
  }
   
   
  return 0;
}







// *****************************************************************************
//;;;;;;;;;;;;;;;;;;;;;;  setup ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// **************************************************************************************



int setup()
{

  fprintf (stderr, "setup start\n");


 
   
	 
  /* 2D array ranges */
  iWidth = iHeight* DisplayAspectRatio ;
  iSize = iWidth * iHeight;	// size = number of points in array 
  iSize_rgb = iSize* iColorSize;
  
  // iy
  iyMax = iHeight - 1;		// Indexes of array starts from 0 not 1 so the highest elements of an array is = array_name[size-1].

  //ix
  ixMax = iWidth - 1;

  /* 1D array ranges */
  // i1Dsize = i2Dsize; // 1D array with the same size as 2D array
  iMax = iSize - 1;		// Indexes of array starts from 0 not 1 so the highest elements of an array is = array_name[size-1].

  
  SetPlane( plane_center, plane_radius,  DisplayAspectRatio );	
  /* Pixel sizes */
  PixelWidth = (ZxMax - ZxMin) / ixMax;	//  ixMax = (iWidth-1)  step between pixels in world coordinate 
  PixelHeight = (ZyMax - ZyMin) / iyMax;
  ratio = ((ZxMax - ZxMin) / (ZyMax - ZyMin)) / ((double) iWidth / (double) iHeight);	// it should be 1.000 ...
  PixelWidth2 = PixelWidth*PixelWidth;
    
  circle_width = PixelWidth * 2.0;
  /* create dynamic 1D arrays for colors ( shades of gray ) */
  data8bit = malloc (iSize * sizeof (unsigned char));
  data8bit2 = malloc (iSize * sizeof (unsigned char));
  edge = malloc (iSize * sizeof (unsigned char));
  
  data24bit =  malloc (iSize_rgb * sizeof (unsigned char));
  
 	
  if (data8bit == NULL || data8bit2 == NULL || edge == NULL || data24bit == NULL)
    { fprintf (stderr, " Could not allocate memory"); return 1; }

  fprintf (stderr, " end of setup \n");

  return 0;

}				// ;;;;;;;;;;;;;;;;;;;;;;;;; end of the setup ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;




int end(void)
{


  fprintf(stderr, " allways free memory (deallocate )  to avoid memory leaks \n");	// https://en.wikipedia.org/wiki/C_dynamic_memory_allocation
  fprintf(stderr, " warning: too long comment in Save8bitArray2PGMFile can cause: *** stack smashing detected ***: terminated\n");
  free (data8bit);
  free (data8bit2);
  free(edge);
  free(data24bit);
  
	
  PrintCInfo ();
  return 0;

}



int MakeImages( ){
	
	
	// draw unit circle , binary test image for sdf
	Draw8bitTestImage(data8bit);
	Save8bitArray2PGMFile (data8bit, "circle", iHeight, "unit circle ");
	
	
	//
	Draw8bitSignedDistance2Circle(data8bit);
	Save8bitArray2PGMFile (data8bit, "circle_sdf_8bit_cont", iHeight, "sdf for unit circle in 8 bit color");
	
	Draw8bitDiscreteSignedDistance2Circle (data8bit2);
	Save8bitArray2PGMFile (data8bit2, "circle_sdf_8bit_discrete", iHeight, "sdf for unit circle in 8 bit discrete color = LSM");
	
	ComputeBoundaries(data8bit2, edge);
	Save8bitArray2PGMFile(edge, "circle_sdf_8bit_discrete_LCM", iHeight, "sdf for unit circle in 8 bit discrete color = LCM");
	
	CopyBoundaries(edge, data8bit2);
	Save8bitArray2PGMFile(data8bit2, "circle_sdf_8bit_discrete_LSCM", iHeight, "sdf for unit circle in 8 bit discrete color = LSCM");
	
	Draw8bitCircle(data8bit2, 255);
	Save8bitArray2PGMFile(data8bit2, "circle_sdf_8bit_discrete_LSCM_with_circle", iHeight, "sdf for unit circle in 8 bit discrete color (LSCM) with circle ");
	
	Draw8bitCircle(data8bit, 255);
	Save8bitArray2PGMFile (data8bit, "circle_sdf_8bit_cont_with_circle", iHeight, "sdf for unit circle in 8 bit color and circle ");
	
	//
	Draw24bitSignedDistance2Circle(data24bit);
	Save24bitArray2PPMFile (data24bit, "circle_sdf_24bit", iHeight, "sdf for unit circle in 24 bit color");
	
	Draw24bitWhiteCircle(data24bit);
 	Save24bitArray2PPMFile (data24bit, "circle_sdf_24bit_white", iHeight, "sdf for unit circle in 24 bit colorwith white circle");
 	
 	
  return 0;
}


// ********************************************************************************************************************
/* -----------------------------------------  main   -------------------------------------------------------------*/
// ********************************************************************************************************************

int main(void)
{

  
  setup();

	
  
		
  MakeImages();
  PrintProgramInfo();
			
  end();
  return 0;
}
