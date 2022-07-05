//
//  GaussianBlur.hpp
//  NanoVG
//
//  Created by Даниил Виноградов on 08.06.2022.
//

#pragma once

#include <iostream>
#include <cmath>
#include <cstring>
#include <chrono>

typedef unsigned char uchar;

//!
//! \fn void std_to_box(float boxes[], float sigma, int n)
//!
//! \brief this function converts the standard deviation of
//! Gaussian blur into dimensions of boxes for box blur. For
//! further details please refer to :
//! https://www.peterkovesi.com/matlabfns/#integral
//! https://www.peterkovesi.com/papers/FastGaussianSmoothing.pdf
//!
//! \param[out] boxes   boxes dimensions
//! \param[in] sigma    Gaussian standard deviation
//! \param[in] n        number of boxes
//!
void std_to_box(int boxes[], float sigma, int n);

//!
//! \fn void horizontal_blur_rgb(uchar * in, uchar * out, int w, int h, int c, int r)
//!
//! \brief this function performs the horizontal blur pass for box blur.
//!
//! \param[in,out] in       source channel
//! \param[in,out] out      target channel
//! \param[in] w            image width
//! \param[in] h            image height
//! \param[in] c            image channels
//! \param[in] r            box dimension
//!
void horizontal_blur_rgb(uchar * in, uchar * out, int w, int h, int c, int r);

//!
//! \fn void total_blur_rgb(uchar * in, uchar * out, int w, int h, int c, int r)
//!
//! \brief this function performs the total blur pass for box blur.
//!
//! \param[in,out] in       source channel
//! \param[in,out] out      target channel
//! \param[in] w            image width
//! \param[in] h            image height
//! \param[in] c            image channels
//! \param[in] r            box dimension
//!
void total_blur_rgb(uchar * in, uchar * out, int w, int h, int c, int r);

//!
//! \fn void box_blur_rgb(uchar * in, uchar * out, int w, int h, int c, int r)
//!
//! \brief this function performs a box blur pass.
//!
//! \param[in,out] in       source channel
//! \param[in,out] out      target channel
//! \param[in] w            image width
//! \param[in] h            image height
//! \param[in] c            image channels
//! \param[in] r            box dimension
//!
void box_blur_rgb(uchar *& in, uchar *& out, int w, int h, int c, int r);

//!
//! \fn void fast_gaussian_blur_rgb(uchar * in, uchar * out, int w, int h, int c, float sigma)
//!
//! \brief this function performs a fast Gaussian blur. Applying several
//! times box blur tends towards a true Gaussian blur. Three passes are sufficient
//! for good results. For further details please refer to :
//! http://blog.ivank.net/fastest-gaussian-blur.html
//!
//! \param[in,out] in       source channel
//! \param[in,out] out      target channel
//! \param[in] w            image width
//! \param[in] h            image height
//! \param[in] c            image channels
//! \param[in] sigma        gaussian std dev
//!
void fast_gaussian_blur_rgb(uchar *& in, uchar *& out, int w, int h, int c, float sigma);
