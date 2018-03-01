//###########################################################################
// This file is part of ProcessLib, a submodule of LImA project the
// Library for Image Acquisition
//
// Copyright (C) : 2009-2011
// European Synchrotron Radiation Facility
// BP 220, Grenoble 38043
// FRANCE
//
// This is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//###########################################################################
#ifndef WITHOUT_GSL
#include <cstring>

#include <gsl/gsl_fft_complex.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_multifit.h>
#include <gsl/gsl_spline.h>
#include <math.h>
#ifdef __unix
#include <sys/time.h>
#endif
#include <time.h>

#include "processlib/ProcessExceptions.h"
#include "processlib/Bpm.h"
#include "processlib/GslErrorMgr.h"
#include "processlib/PoolThreadMgr.h"
#include "processlib/SoftRoi.h"


using namespace Tasks;
#ifdef __unix
template<class INPUT> static inline INPUT max(INPUT a,INPUT b)
{
  return a > b ? a : b;
}
template<class INPUT> static inline INPUT min(INPUT a,INPUT b)
{
  return a < b ? a : b;
}
#endif

// oldest windows C++ (VStudio 2008) does not provide round() func with math.h, so wrap it with local function 
#ifdef __unix
#define ROUND(a) round(a)
#else
#define ROUND(a) win_specific_round(a)
#endif
/** @brief Calculate the image projections in X and Y and
 *  the integrated pixel intensity on the image.
 */
template<class INPUT,class SUMMTYPE> 
void BpmTask::_treat_image(const Data &aSrc,
			   Buffer &projection_x,Buffer &projection_y,
			   BpmResult &aResult)
{
  SUMMTYPE *aProjectionX = (SUMMTYPE*)projection_x.data;
  SUMMTYPE *aProjectionY = (SUMMTYPE*)projection_y.data;
  Buffer *aBufferPt = aSrc.buffer;
  INPUT *aSrcPt = (INPUT*)aBufferPt->data;
  for(int y = 0;y < aSrc.dimensions[1];++y)
    for(int x = 0;x < aSrc.dimensions[0];++x)
      {
	if(*aSrcPt > INPUT(mThreshold))
	  {
	    aProjectionX[x] += *aSrcPt;
	    aProjectionY[y] += *aSrcPt;
	    aResult.beam_intensity += *aSrcPt;
	  }
	++aSrcPt;
      }
}
/** @brief Calculates the projection X on a limitied area of
 *  the image around the maximum.
 */
template<class INPUT,class SUMMTYPE>
void BpmTask::_tune_projection(const Data &aSrc,
			       Buffer &projection_x,Buffer &projection_y,
			       const BpmResult &aResult)
{
  SUMMTYPE *aProjectionX = (SUMMTYPE*)projection_x.data;
  memset(aProjectionX,0,aSrc.dimensions[0] * sizeof(SUMMTYPE));
  
  SUMMTYPE *aProjectionY = (SUMMTYPE*)projection_y.data;
  memset(aProjectionY,0,aSrc.dimensions[1] * sizeof(SUMMTYPE));

  Buffer *aBufferPt = aSrc.buffer;
  INPUT *aSrcPt = (INPUT*)aBufferPt->data;
  
  // X tuning profile
  int tuning_margin = (int)ROUND((aResult.beam_fwhm_y * (mFwhmTunningExtension - 1)) / 2.);
  int fwhm_min = max(mBorderExclusion,aResult.beam_fwhm_min_y_index - tuning_margin);
  int fwhm_max = min(aResult.beam_fwhm_max_y_index + tuning_margin,
		     aSrc.dimensions[1] - 1 - mBorderExclusion);
  for(int y = fwhm_min;y < (fwhm_max + 1);++y)
    {
      INPUT *aBufferPt = aSrcPt + (y * aSrc.dimensions[0]) + mBorderExclusion;
      for(int x = mBorderExclusion;x < (aSrc.dimensions[0] - mBorderExclusion);++x,++aBufferPt)
	aProjectionX[x] += *aBufferPt;
    }

  // Y tuning profile
  tuning_margin = (int)ROUND((aResult.beam_fwhm_x * (mFwhmTunningExtension - 1)) / 2.);
  fwhm_min = max(mBorderExclusion,aResult.beam_fwhm_min_x_index - tuning_margin);
  fwhm_max = min(aResult.beam_fwhm_max_x_index + tuning_margin,
		 aSrc.dimensions[0] - 1 - mBorderExclusion);
  
  for(int y = mBorderExclusion;y < (aSrc.dimensions[1] - mBorderExclusion);++y)
    {
      INPUT *aBufferPt = aSrcPt + (y * aSrc.dimensions[0]) + fwhm_min;
      for(int x = fwhm_min;x < (fwhm_max + 1);++x,++aBufferPt)
	aProjectionY[y] += *aBufferPt;
    }
}
/** @brief Find the position where the maximum is.
 *  average over three values and find the maximum in Y
 */
template<class SUMMTYPE>
inline int _max_intensity_position(const Buffer &projection,int aSize) 
{
  int aMaxPos = -1;
  SUMMTYPE aMaxValue = 0;
  const SUMMTYPE *aProjection = (SUMMTYPE*)projection.data;

  for(int i = 1;i < (aSize -2);++i)
    {
      SUMMTYPE aValue = aProjection[i - 1] + aProjection[i] + aProjection[i + 1];
      if(aValue > aMaxValue)
	aMaxValue = aValue,aMaxPos = i;
    }
  return aMaxPos;
}
/** @brief Find the maximum from the x and y projections.
 *  and get the intensity at this point averaged
 *  over 5 pixels.
 *  Normally this should be the position of the beam spot.
 */
template<class INPUT,class SUMMTYPE> 
static void _max_intensity(const Data &aSrc,
			   const Buffer &projection_x,const Buffer & projection_y,
			   BpmResult &aResult)
{
  Buffer *aBufferPt = aSrc.buffer;
  INPUT *aSrcPt = (INPUT*)aBufferPt->data;
  
  int yMaxPos = _max_intensity_position<SUMMTYPE>(projection_y,aSrc.dimensions[1]);
  int xMaxPos = _max_intensity_position<SUMMTYPE>(projection_x,aSrc.dimensions[0]);
  bool xPositionFound = xMaxPos > 0 && xMaxPos < (aSrc.dimensions[0] - 1);
  bool yPositionFound = yMaxPos > 0 && yMaxPos < (aSrc.dimensions[1] - 1);
  if(xPositionFound)
    {
      aResult.max_pixel_x = xMaxPos;
      aResult.beam_center_x = (double)xMaxPos;
    }
  if(yPositionFound)
    {
      aResult.max_pixel_y = yMaxPos;
      aResult.beam_center_y = (double)yMaxPos;
    }
  // get the five pixel values around x_max_pos and y_max_pos
  if(xPositionFound && yPositionFound)
    {
      // Center left Pixel
      aSrcPt += yMaxPos * aSrc.dimensions[0] + xMaxPos - 1;
      unsigned long long aBeamSum = *aSrcPt;
      aResult.max_pixel_value = (unsigned int)aBeamSum;
     // Center Pixel
      ++aSrcPt;
      aBeamSum += *aSrcPt;
      // Center right Pixel
      ++aSrcPt;
      aBeamSum += *aSrcPt;
      // Top Pixel
      aSrcPt -= aSrc.dimensions[0] + 1;
      aBeamSum += *aSrcPt;
      // Bottom Pixel
      aSrcPt += aSrc.dimensions[0] << 1;
      aBeamSum += *aSrcPt;
      aResult.beam_intensity = (double)(aBeamSum) / 5.;
    }
  else
    aResult.beam_intensity = -1.;
}
template<class SUMMTYPE>
double BpmTask::_calculate_fwhm(const Buffer &projectionBuffer,int size,
				int peak_index,double background_level,
				int &min_index,int &max_index)
{
  const SUMMTYPE *aProjectionPt = (const SUMMTYPE*)projectionBuffer.data;
  SUMMTYPE max_value = aProjectionPt[peak_index];
  double usedBackgroundLevel;
  if(mEnableBackgroundSubstration) 
    usedBackgroundLevel = background_level;
  else
    {
      usedBackgroundLevel = double(aProjectionPt[mBorderExclusion] +
				   aProjectionPt[mBorderExclusion + 1] +
				   aProjectionPt[mBorderExclusion + 2] +
				   aProjectionPt[size - mBorderExclusion - 1] +
				   aProjectionPt[size - mBorderExclusion - 2] +
				   aProjectionPt[size - mBorderExclusion - 3]) / 6.;
    }
  // treat the case of strange data
  if(max_value < (3. + usedBackgroundLevel))
    {
      min_index = max_index = 0;
      return -1.;
    }
  double hm = ((max_value - usedBackgroundLevel) / 2.) + usedBackgroundLevel;
  // FIRST ON THE LEFT SIDE OF THE CURVE
  int index;
  bool err = false;
  index = peak_index;
  while( aProjectionPt[index] > hm ){
    index = index - 1;
    if(index < 0){
      index=0;
      err = true;
      break;
    }
  }

  if(err)			// Error
    return -1.;

  double val_h = aProjectionPt[index + 1];
  double val_l = aProjectionPt[index];
  double locut;
  if(!(val_h> val_l && val_h> hm && hm> val_l))
    {
      std::cout << "val_h" << val_h
		<< "val_l" << val_l
		<< "hm" << hm
		<< std::endl;
      return -1.;
    }

  locut = index + (hm - val_l)/(val_h-val_l);
  // NOW ON THE RIGHT SIDE OF THE CURVE
  index = peak_index;
  while( aProjectionPt[index] > hm ){
    index = index + 1;
    if(index > (size-1)){
      index=size;
      err = 1;
      break;
    }
  }
  val_h = aProjectionPt[index - 1];
  val_l = aProjectionPt[index];
  if(!(val_h> val_l && val_h> hm && hm> val_l))
    {
      std::cout << "val_h" << val_h
		<< "val_l" << val_l
		<< "hm" << hm
		<< std::endl;
      return -1.;
    }

  double hicut;
  hicut = index - (hm - val_l)/(val_h-val_l);

  min_index = int(locut);
  max_index = int(hicut);

  return hicut - locut;
}
/** @brief Caluclate the background level around the peak
 */
template<class SUMMTYPE>
void BpmTask::_calculate_background(const Buffer &projection,double &background_level,
				    int min_index,int max_index)
{
#if DEBUG
  printf("min_index %d, max_index %d\n",min_index,max_index);
#endif
  SUMMTYPE *aProjectionPt = (SUMMTYPE*)projection.data;
  if(mEnableBackgroundSubstration)
    background_level = (aProjectionPt[min_index] + aProjectionPt[max_index]) / 2.;
  else
    background_level = 0.;
}
/** @brief the distribution support is extended from ly to n
 *  the values of y at the (n-ly) new points tend to zero linerarly
 */
static inline void _extend_y(double arr[],int size,
			     double ret_arr[],int ret_size)
{
  double offset = arr[0];
  for(int i = 0;i < size;++i)
    arr[i] -= offset;
  double yend = arr[size - 1];
  double incz = -yend/(ret_size - size - 1);
  memcpy(ret_arr,arr,size * sizeof(double));
  for(int i = size;i < ret_size;++i,yend += incz)
    ret_arr[i] = yend;
}
/** @brief parabolic fit of a data set of datalength length...
 *  the parabola coefficients are returned in X
 */
static inline int _solve(double data[], int datalength, int first, double x[])
{
#ifdef DEBUG
  printf("6a\n");
  printf("n: %d\n",datalength);
#endif
 
  gsl_matrix *X = gsl_matrix_alloc (datalength, 3);
  gsl_vector *y = gsl_vector_alloc (datalength);
  
  int error_flag = GslErrorMgr::get().lastErrno();
#ifdef DEBUG
  printf("error_flag: %d\n",error_flag);
#endif
  if (error_flag) return 1;

#ifdef DEBUG
  printf("6b\n");
#endif

  gsl_vector *c = gsl_vector_alloc(3);
  gsl_matrix *cov = gsl_matrix_alloc (3, 3);
  for (int i = 0; i < datalength;++i)
    {
      gsl_matrix_set (X, i, 0, 1.0);
      gsl_matrix_set (X, i, 1, double(i+first));
      gsl_matrix_set (X, i, 2, double((i+first)*(i+first)));
      gsl_vector_set (y, i, data[i]);
    }
  gsl_multifit_linear_workspace *work = gsl_multifit_linear_alloc (datalength, 3);
  double chisq;
  gsl_multifit_linear (X, y, c, cov, &chisq, work);
  gsl_multifit_linear_free (work);

  for(int i = 0;i < 3;++i)
    x[i] = gsl_vector_get(c,i);

  gsl_matrix_free(X);
  gsl_vector_free(y);
  gsl_matrix_free(cov);
  gsl_vector_free(c);

  return 0;
}

//find first poind and width of Y distribution such as y>threshold
static void _find_summit(double arr[],int size,int &first_point,int &nb_points)
{
  static const double THRESHOLD = 0.9;
  double max = arr[0];

  for(int i = 0;i < size;++i)
    if(arr[i]>max) max = arr[i];

  double threshold = max * THRESHOLD;
  nb_points = 0;
  first_point = -1;
  for(int i = 0;i < size;i++)
    if(arr[i] > threshold)
      {
	if(!nb_points) first_point = i;
	++nb_points;
      }
#ifdef DEBUG 
  printf("max %lf,threshold %lf,first_point %d\n",max,threshold,first_point);
#endif
}
#define REAL(z,i) ((z)[i << 1])	// i << 1 == 2i
#define IMAG(z,i) ((z)[(i << 1) + 1])

/** @brief find a center of a gaussian curve
 */
static inline double _compute_center(double y[],int ly)
{
  double result = -1.0;
  int n;
  for(n = 1;n < ly;n <<= 1);	// Find the next above power of two
  n <<= 1;
  int n1 = n << 2;
#ifdef DEBUG
  printf("n : %d , n1 %d\n",n,n1);
#endif
 
  GslErrorMgr::get().resetErrorMsg();

  Buffer *ynRe = new Buffer(n * sizeof(double));
  double *yn_re = (double*)ynRe->data;
  // y is extended from ly to n to give yn
  // the values of yn at the (n-ly) new points tend to zero linerarly
  // function not checked for failure as not much can go wrong
  _extend_y(y,ly,yn_re,n);

  Buffer *dataN = new Buffer(n * 2 * sizeof(double));	// REAL + IMAG in double
  double *data_n = (double*)dataN->data;
  //fn =FFT(yn)
  for(int i = 0;i < n;++i)
    {
      REAL(data_n,i) = yn_re[i];
      IMAG(data_n,i) = 0.0;
    }
  int error_flag = gsl_fft_complex_radix2_forward(data_n,1,n);

  Buffer *dataN1 = new Buffer(2 * n1 * sizeof(double));
  double *data_n1 = (double*)dataN1->data;
  Buffer *yn1Re = new Buffer(n1 * sizeof(double));
  double *yn1_re = (double*)yn1Re->data;
  if (!error_flag)
    {
      // f is extended from n to n1 to give fn1
      // (fn is split in two and zero-padded in the middle)
      memset(data_n1 + n,0,((n1 - n) << 1) * sizeof(double));
      // copy first half of data_n at the beginnig of data_n1
      memcpy(data_n1,data_n,n * sizeof(double));
      // copy second half of data_n at the end of data_n1
      memcpy(data_n1 + ((n1 << 1) - n),data_n + n,n * sizeof(double));
      // fn1 is squared
      for(int i = 0;i < n1;++i)
	{
	  double rl = REAL(data_n1,i);
	  double img = IMAG(data_n1,i);

	  REAL(data_n1,i) = rl*rl-img*img;
	  IMAG(data_n1,i)=2*rl*img;
	}
      //yn1=IFFT(fn1)...
      error_flag = gsl_fft_complex_radix2_inverse (data_n1, 1, n1);
#ifdef DEBUG
      printf("ifft done.. error_flag=%d\n",error_flag);
#endif
       
      //absolute value of yn1
      if (!error_flag) 
	{
	  for (int i = 0;i < n1;++i)
	    {
	      double rl = REAL(data_n1,i);
	      double img = IMAG(data_n1,i);
	      yn1_re[i] = sqrt(rl*rl+img*img);
	    }
	  //find  point and width of yn1 such as yn1>threshold
	  int first_point,nb_points;
	  _find_summit(yn1_re,n1,first_point,nb_points);
#ifdef DEBUG
	  printf("summit done... nb_points = %d\n", nb_points);
#endif
	  if (!nb_points)
	    error_flag = 1;
	  else  //'final' distribution
	    {
#ifdef DEBUG
	      printf("final distribution done..\n");
#endif
	      // given the summit 'final' distribution,
	      // parabolic fit and computation of parabola summit position
	      double X[3];
	      error_flag = _solve(yn1_re + first_point,
				  nb_points, first_point, X);
#ifdef DEBUG
	      printf("solve done..\n");
#endif
	      if (!error_flag)
		{
		  result = -X[1]/X[2]/2.0/8.0;
#ifdef DEBUG
		  printf("X (%lf,%lf,%lf)\t",X[0],X[1],X[2]);
		  printf("fitting result %lf, first_point %d\n",result,first_point);
#endif

		}
	      else 
		{
#ifdef DEBUG
		  printf("ERROR! result = -1.0\n");
#endif
		  throw ProcessException(GslErrorMgr::get().lastErrorMsg());
		  GslErrorMgr::get().resetErrorMsg();
		}
	    }
	}
    }

  //Free allocated variable
  ynRe->unref();
  dataN->unref();
  dataN1->unref();
  yn1Re->unref();
  return result;
}
//@brief constructor
BpmTask::BpmTask(BpmManager &aMgr) :
  SinkTask<BpmResult>(aMgr),
  mFwhmTunning(false),
  mFwhmTunningExtension(1.5),
  mAoiExtension(4.),
  mBorderExclusion(10),
//   mGaussFittMax(false),
//   mAverage(1),
  mThreshold(0),
  mEnableX(true),
  mEnableY(true),
  mEnableBackgroundSubstration(false),
  mRoiAutomatic(true),
  _RoiX1(-1),_RoiX2(-1),
  _RoiY1(-1),_RoiY2(-1)
{
}

BpmTask::BpmTask(const BpmTask &aTask) :
  SinkTask<BpmResult>(aTask),
  mFwhmTunning(aTask.mFwhmTunning),
  mFwhmTunningExtension(aTask.mFwhmTunningExtension),
  mAoiExtension(aTask.mAoiExtension),
  mBorderExclusion(aTask.mBorderExclusion),
  //   mGaussFittMax(aTask.mGaussFittMax),
  //   mAverage(aTask.mAverage),
  mThreshold(aTask.mThreshold),
  mEnableX(aTask.mEnableX),
  mEnableY(aTask.mEnableY),
  mEnableBackgroundSubstration(aTask.mEnableBackgroundSubstration),
  mRoiAutomatic(aTask.mRoiAutomatic),
  _RoiX1(aTask._RoiX1),_RoiX2(aTask._RoiX2),
  _RoiY1(aTask._RoiY1),_RoiY2(aTask._RoiY2)
{
  
}

#define COMPUTE_BEAM_POSITION(XorY,WidthorHeight,SUMMTYPE)	\
{ \
  int min_index,max_index; \
  aResult.beam_fwhm_##XorY = _calculate_fwhm<SUMMTYPE>(*projection_##XorY,WidthorHeight , \
						       int(aResult.beam_center_##XorY), \
						       aResult.mBackgroundLevel##XorY, \
						       min_index,max_index); \
  /* check for strange results */ \
  if(max_index <= min_index || max_index <= 0 || aResult.beam_fwhm_##XorY <= 0.) \
    { \
      aResult.beam_fwhm_##XorY = -1.; \
      aResult.beam_center_##XorY = -1.; \
      aResult.mBackgroundLevel##XorY = 0; \
      /*@todo maybe register in BpmManager::Result with enum that call failed */ \
    } \
  else \
    { \
      aResult.beam_fwhm_min_##XorY##_index = min_index; \
      aResult.beam_fwhm_max_##XorY##_index = max_index; \
      if(mRoiAutomatic) \
	{ \
	  int AOI_margin = (int)ROUND(((max_index - min_index) * (mAoiExtension - 1)) / 2.); \
	  min_index = max(mBorderExclusion,min_index - AOI_margin); \
	  max_index = min(max_index + AOI_margin,WidthorHeight - 1 - mBorderExclusion); \
	  aResult.AOI_min_##XorY = min_index; \
	  aResult.AOI_max_##XorY = max_index; \
	} \
      else \
	{ \
	  aResult.AOI_automatic = false; \
	  /* @todo No Roi Management for now maybe*/	\
	  min_index = 0; \
	  max_index = WidthorHeight; \
	} \
      if(min_index < max_index)			\
	{								\
	  _calculate_background<SUMMTYPE>(*projection_##XorY,aResult.mBackgroundLevel##XorY, \
					  min_index,max_index);		\
	  /* calculate the beam center */				\
	  int size = max_index - min_index + 1;				\
	  Buffer *profile = new Buffer(size * sizeof(double));		\
	  double *aProfilePt = (double*)profile->data;			\
	  SUMMTYPE *aSrcProfilePt = (SUMMTYPE*)projection_##XorY->data + min_index; \
	  for(int i = 0;i < size;++i) /* @todo optimized if needed */	\
	    aProfilePt[i] = double(aSrcProfilePt[i]);			\
									\
	  aResult.beam_center_##XorY = _compute_center(aProfilePt,size) + min_index; \
	  profile->unref();	/* free */				\
	}								\
	       \
      /*if(aResult.beam_center_x <= 0) @todo should manage error	\
       error = "Beam center calculation failed" \
       error += gsl_cw_error_message();*/	\
    } \
}

#define TUNE_FWHM(XorY,WidthorHeight,SUMMTYPE)		\
{ \
  int min_index,max_index; \
  aResult.beam_fwhm_##XorY = _calculate_fwhm<SUMMTYPE>(*projection_##XorY,WidthorHeight, \
						       _max_intensity_position<SUMMTYPE>(*projection_##XorY,WidthorHeight), \
						       aResult.mBackgroundLevelTune##XorY, \
						       min_index,max_index); \
 \
  int AOI_margin = (int)ROUND((aResult.beam_fwhm_##XorY * (mAoiExtension - 1)) / 2.); \
  min_index = max(mBorderExclusion,min_index - AOI_margin); \
  max_index = min(max_index + AOI_margin,WidthorHeight - 1 - mBorderExclusion); \
	       \
  _calculate_background<SUMMTYPE>(*projection_##XorY,aResult.mBackgroundLevelTune##XorY, \
				  min_index,max_index);			\
}

#define PROCESS(TYPE,SUMMTYPE)				\
{							\
  int aSize = sizeof(SUMMTYPE) * aSrc.dimensions[0];	\
  projection_x = new Buffer(aSize);		\
  memset (projection_x->data, 0,aSize);				\
  aResult.profile_x.dimensions.push_back(aSrc.dimensions[0]);	\
  aResult.profile_x.setBuffer(projection_x);			\
								\
  aSize = sizeof(SUMMTYPE) * aSrc.dimensions[1];		\
  projection_y = new Buffer(aSize);			\
  memset (projection_y->data, 0,aSize);				\
  aResult.profile_y.dimensions.push_back(aSrc.dimensions[1]);	\
  aResult.profile_y.setBuffer(projection_y);			\
								\
									\
_treat_image<TYPE,SUMMTYPE>(aSrc,*projection_x,*projection_y,aResult);	  \
_max_intensity<TYPE,SUMMTYPE>(aSrc,*projection_x,*projection_y,aResult);	\
if(mEnableX || mFwhmTunning) \
    COMPUTE_BEAM_POSITION(x,aSrc.dimensions[0],SUMMTYPE);	\
if(mEnableY || mFwhmTunning) \
    COMPUTE_BEAM_POSITION(y,aSrc.dimensions[1],SUMMTYPE);	\
if(mFwhmTunning) \
  { \
    if(aResult.beam_fwhm_x > 0. && aResult.beam_fwhm_y > 0.) \
      { \
	_tune_projection<TYPE,SUMMTYPE>(aSrc,*projection_x,*projection_y,aResult); \
	TUNE_FWHM(x,aSrc.dimensions[0],SUMMTYPE);				\
	TUNE_FWHM(y,aSrc.dimensions[1],SUMMTYPE);				\
      } \
  } \
}

void BpmTask::process(Data &aInputSrc)
{
  Data aSrc;
  if(aInputSrc.dimensions.size() != 2)
    throw ProcessException("BpmTask : Only manage 2D data");
  else if(!(_RoiX1 < 0 && _RoiX2 < 0 &&
	    _RoiY1 < 0 && _RoiY2 < 0))
    {
      SoftRoi *roiTaskPt = new SoftRoi();
      roiTaskPt->setRoi(_RoiX1,_RoiX2,_RoiY1,_RoiY2);
      roiTaskPt->setProcessingInPlace(false);
      aSrc = roiTaskPt->process(aInputSrc);
      roiTaskPt->unref();
    }
  else
    aSrc = aInputSrc;

  BpmResult aResult;
  BpmResult aLastResult = _mgr.getResult();
  if(aLastResult.errorCode == BpmManager::OK) // copy the previous background level
    {
      aResult.mBackgroundLevelx = aLastResult.mBackgroundLevelx;
      aResult.mBackgroundLevely = aLastResult.mBackgroundLevely;
      aResult.mBackgroundLevelTunex = aLastResult.mBackgroundLevelTunex;
      aResult.mBackgroundLevelTuney = aLastResult.mBackgroundLevelTuney;
    }
  aResult.frameNumber = aSrc.frameNumber;
  Buffer *projection_x = NULL,*projection_y = NULL;
  switch(aSrc.type)
    {
    case Data::UINT8:
      PROCESS(unsigned char,int);break;
    case Data::INT8:
      PROCESS(char,int);break;
    case Data::UINT16:
      PROCESS(unsigned short,int);break;
    case Data::INT16:
      PROCESS(short,int);break;
    case Data::INT32:
      PROCESS(int,long long);break;
    case Data::UINT32:
      PROCESS(unsigned int,long long);break;
    default:
      {
	char aBuffer[256];
	snprintf(aBuffer,sizeof(aBuffer),"BpmManager : Data depth of %d not implemented",aSrc.depth());
	throw ProcessException(aBuffer);
      }
    }
  switch(aSrc.depth())
    {
    case 1:
    case 2:
      aResult.profile_x.type = Data::INT32;
      aResult.profile_y.type = Data::INT32;
      break;
    default:
      aResult.profile_x.type = Data::INT64;
      aResult.profile_y.type = Data::INT64;
      break;
    }

  projection_x->unref();
  projection_y->unref();
  if(!(_RoiX1 < 0 && _RoiX2 < 0 &&
       _RoiY1 < 0 && _RoiY2 < 0))
    {
      aResult.beam_center_x += _RoiX1;
      aResult.beam_fwhm_min_x_index += _RoiX1;
      aResult.beam_fwhm_max_x_index += _RoiX1;
      
      aResult.beam_center_y += _RoiY1;
      aResult.beam_fwhm_min_y_index += _RoiY1;
      aResult.beam_fwhm_max_y_index += _RoiY1;
    }
  aResult.timestamp = aInputSrc.timestamp;
  _mgr.setResult(aResult);
}

/**@brief set roi corner
 * @todo maybe add a lock to be mt-safe!!
 */
void BpmTask::setRoi(int x1,int x2,
		     int y1,int y2)
{
  _RoiX1 = x1,_RoiX2 = x2;
  _RoiY1 = y1,_RoiY2 = y2;
}
//@brief get roi corner
void BpmTask::getRoi(int &x1,int &x2,
		       int &y1,int &y2) const
{
  x1 = _RoiX1,x2 = _RoiX2;
  y1 = _RoiY1,y2 = _RoiY2;
}

#ifndef __unix
extern "C"
{
  static void _impl_bpm()
  {
    BpmManager *bpmMgr = new BpmManager();

    bpmMgr->setMode(BpmManager::Counter);
    bpmMgr->getResult();
    std::list<BpmResult> aResult;
    bpmMgr->getHistory(aResult);
    bpmMgr->resizeHistory(10);
    bpmMgr->resetHistory();
    bpmMgr->historySize();
    bpmMgr->lastFrameNumber();
    bpmMgr->ref();
    bpmMgr->unref();

    BpmTask *bpmTask = new BpmTask(*bpmMgr);
    bpmTask->ref();
    bpmTask->unref();
  }
}
#endif
#endif //WITHOUT_GSL
