//
//  GaussianBlur.cpp
//  NanoVG
//
//  Created by Даниил Виноградов on 08.06.2022.
//

#include "GaussianBlur.hpp"

namespace NXKit {

void std_to_box(int boxes[], float sigma, int n)
{
    // ideal filter width
    float wi = std::sqrt((12*sigma*sigma/n)+1);
    int wl = std::floor(wi);
    if(wl%2==0) wl--;
    int wu = wl+2;

    float mi = (12*sigma*sigma - n*wl*wl - 4*n*wl - 3*n)/(-4*wl - 4);
    int m = std::round(mi);

    for(int i=0; i<n; i++)
        boxes[i] = ((i < m ? wl : wu) - 1) / 2;
}

void horizontal_blur_rgb(uchar * in, uchar * out, int w, int h, int c, int r)
{
    float iarr = 1.f / (r+r+1);
#pragma omp parallel for
    for(int i=0; i<h; i++)
    {
        int ti = i*w;
        int li = ti;
        int ri = ti+r;

        int fv[3] = { in[ti*c+0], in[ti*c+1], in[ti*c+2] };
        int lv[3] = { in[(ti+w-1)*c+0], in[(ti+w-1)*c+1], in[(ti+w-1)*c+2] };
        int val[3] = { (r+1)*fv[0], (r+1)*fv[1], (r+1)*fv[2] };

        for(int j=0; j<r; j++)
        {
            val[0] += in[(ti+j)*c+0];
            val[1] += in[(ti+j)*c+1];
            val[2] += in[(ti+j)*c+2];
        }

        for(int j=0; j<=r; j++, ri++, ti++)
        {
            val[0] += in[ri*c+0] - fv[0];
            val[1] += in[ri*c+1] - fv[1];
            val[2] += in[ri*c+2] - fv[2];
            out[ti*c+0] = std::round(val[0]*iarr);
            out[ti*c+1] = std::round(val[1]*iarr);
            out[ti*c+2] = std::round(val[2]*iarr);
        }

        for(int j=r+1; j<w-r; j++, ri++, ti++, li++)
        {
            val[0] += in[ri*c+0] - in[li*c+0];
            val[1] += in[ri*c+1] - in[li*c+1];
            val[2] += in[ri*c+2] - in[li*c+2];
            out[ti*c+0] = std::round(val[0]*iarr);
            out[ti*c+1] = std::round(val[1]*iarr);
            out[ti*c+2] = std::round(val[2]*iarr);
        }

        for(int j=w-r; j<w; j++, ti++, li++)
        {
            val[0] += lv[0] - in[li*c+0];
            val[1] += lv[1] - in[li*c+1];
            val[2] += lv[2] - in[li*c+2];
            out[ti*c+0] = std::round(val[0]*iarr);
            out[ti*c+1] = std::round(val[1]*iarr);
            out[ti*c+2] = std::round(val[2]*iarr);
        }
    }
}

void total_blur_rgb(uchar * in, uchar * out, int w, int h, int c, int r)
{
    // radius range on either side of a pixel + the pixel itself
    float iarr = 1.f / (r+r+1);
#pragma omp parallel for
    for(int i=0; i<w; i++)
    {
        int ti = i;
        int li = ti;
        int ri = ti+r*w;

        int fv[3] = {in[ti*c+0], in[ti*c+1], in[ti*c+2] };
        int lv[3] = {in[(ti+w*(h-1))*c+0], in[(ti+w*(h-1))*c+1], in[(ti+w*(h-1))*c+2] };
        int val[3] = {(r+1)*fv[0], (r+1)*fv[1], (r+1)*fv[2] };

        for(int j=0; j<r; j++)
        {
            val[0] += in[(ti+j*w)*c+0];
            val[1] += in[(ti+j*w)*c+1];
            val[2] += in[(ti+j*w)*c+2];
        }

        for(int j=0; j<=r; j++, ri+=w, ti+=w)
        {
            val[0] += in[ri*c+0] - fv[0];
            val[1] += in[ri*c+1] - fv[1];
            val[2] += in[ri*c+2] - fv[2];
            out[ti*c+0] = std::round(val[0]*iarr);
            out[ti*c+1] = std::round(val[1]*iarr);
            out[ti*c+2] = std::round(val[2]*iarr);
        }

        for(int j=r+1; j<h-r; j++, ri+=w, ti+=w, li+=w)
        {
            val[0] += in[ri*c+0] - in[li*c+0];
            val[1] += in[ri*c+1] - in[li*c+1];
            val[2] += in[ri*c+2] - in[li*c+2];
            out[ti*c+0] = std::round(val[0]*iarr);
            out[ti*c+1] = std::round(val[1]*iarr);
            out[ti*c+2] = std::round(val[2]*iarr);
        }

        for(int j=h-r; j<h; j++, ti+=w, li+=w)
        {
            val[0] += lv[0] - in[li*c+0];
            val[1] += lv[1] - in[li*c+1];
            val[2] += lv[2] - in[li*c+2];
            out[ti*c+0] = std::round(val[0]*iarr);
            out[ti*c+1] = std::round(val[1]*iarr);
            out[ti*c+2] = std::round(val[2]*iarr);
        }
    }
}

void box_blur_rgb(uchar *& in, uchar *& out, int w, int h, int c, int r)
{
    std::swap(in, out);
    horizontal_blur_rgb(out, in, w, h, c, r);
    total_blur_rgb(in, out, w, h, c, r);
    // Note to myself :
    // here we could go anisotropic with different radiis rx,ry in HBlur and TBlur
}

void fast_gaussian_blur_rgb(uchar *& in, uchar *& out, int w, int h, int c, float sigma)
{
    // sigma conversion to box dimensions
    int boxes[3];
    std_to_box(boxes, sigma, 3);
    box_blur_rgb(in, out, w, h, c, boxes[0]);
    box_blur_rgb(out, in, w, h, c, boxes[1]);
    box_blur_rgb(in, out, w, h, c, boxes[2]);
}

}
