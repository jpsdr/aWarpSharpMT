// aWarpSharp package 2016.06.23 for Avisynth+ and Avisynth 2.6
// based on Firesledge's 2015.12.30 for Avisynth 2.5
// aWarpSharp package 2012.03.28 for Avisynth 2.5
// Copyright (C) 2003 MarcFD, 2012 Skakov Pavel
// 2015 Firesledge
// 2016 pinterf
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

#define NOMINMAX
#include <algorithm>
#include <emmintrin.h>
#include <smmintrin.h>
#include <immintrin.h> // _mm_undefined
#include "./aWarpSharp.h"

// For VS2010
#ifndef _mm_undefined_si128
#define _mm_undefined_si128 _mm_setzero_si128
#endif

static bool aWarpSharp_Enable_SSE2,aWarpSharp_Enable_SSE41,aWarpSharp_Enable_AVX;

static ThreadPoolInterface *poolInterface;

extern "C" void JPSDR_Warp2_8_SSE2(const unsigned char *psrc,const unsigned char *pedg,unsigned char *pdst,int32_t src_pitch,
	int32_t edg_pitchp,int32_t edg_pitchn,int32_t y_limit_min,int32_t y_limit_max,const short *x_limit_min,const short *x_limit_max,
	int32_t i_,int32_t depthH,int32_t depthV);
extern "C" void JPSDR_Warp2_8_AVX(const unsigned char *psrc,const unsigned char *pedg,unsigned char *pdst,int32_t src_pitch,
	int32_t edg_pitchp,int32_t edg_pitchn,int32_t y_limit_min,int32_t y_limit_max,const short *x_limit_min,const short *x_limit_max,
	int32_t i_,int32_t depthH,int32_t depthV);
extern "C" void JPSDR_Warp0_8_SSE2(const unsigned char *psrc,const unsigned char *pedg,unsigned char *pdst,int32_t src_pitch,
	int32_t edg_pitchp,int32_t edg_pitchn,int32_t y_limit_min,int32_t y_limit_max,const short *x_limit_min,const short *x_limit_max,
	int32_t i_,uint32_t depthH,int32_t depthV);
extern "C" void JPSDR_Warp0_8_AVX(const unsigned char *psrc,const unsigned char *pedg,unsigned char *pdst,int32_t src_pitch,
	int32_t edg_pitchp,int32_t edg_pitchn,int32_t y_limit_min,int32_t y_limit_max,const short *x_limit_min,const short *x_limit_max,
	int32_t i_,int32_t depthH,int32_t depthV);

extern "C" void JPSDR_Sobel_8_SSE2(const unsigned char *psrc,unsigned char *pdst,int32_t src_pitch,int32_t y_,int32_t height,
	int32_t i_,int32_t thresh);
extern "C" void JPSDR_Sobel_8_AVX(const unsigned char *psrc,unsigned char *pdst,int32_t src_pitch,int32_t y_,int32_t height,
	int32_t i_,int32_t thresh);
extern "C" void JPSDR_Sobel_16_SSE2(const unsigned char *psrc,unsigned char *pdst,int32_t src_pitch,int32_t y_,int32_t height,
	int32_t i_,int32_t thresh);
extern "C" void JPSDR_Sobel_16_AVX(const unsigned char *psrc,unsigned char *pdst,int32_t src_pitch,int32_t y_,int32_t height,
	int32_t i_,int32_t thresh);

extern "C" void JPSDR_H_BlurR6_8_SSE2(unsigned char *psrc2,unsigned char *ptmp2,int32_t src_row_size_16);
extern "C" void JPSDR_H_BlurR6_8_AVX(unsigned char *psrc2,unsigned char *ptmp2,int32_t src_row_size,const unsigned char *dq0toF);

extern "C" void JPSDR_H_BlurR6a_8_SSE2(unsigned char *psrc2,unsigned char *ptmp2);
extern "C" void JPSDR_H_BlurR6b_8_SSE2(unsigned char *psrc2,unsigned char *ptmp2);
extern "C" void JPSDR_H_BlurR6c_8_SSE2(unsigned char *psrc2,unsigned char *ptmp2);

extern "C" void JPSDR_V_BlurR6a_8_SSE2(unsigned char *psrc2,unsigned char *ptmp2,int32_t tmp_pitch, int32_t src_row_size_16);
extern "C" void JPSDR_V_BlurR6b_8_SSE2(unsigned char *psrc2,unsigned char *ptmp2,int32_t tmp_pitch, int32_t src_row_size_16);
extern "C" void JPSDR_V_BlurR6c_8_SSE2(unsigned char *psrc2,unsigned char *ptmp2,int32_t tmp_pitch, int32_t src_row_size_16);

extern "C" void JPSDR_V_BlurR6a_8_AVX(unsigned char *psrc2,unsigned char *ptmp2,int32_t tmp_pitch, int32_t src_row_size_16);
extern "C" void JPSDR_V_BlurR6b_8_AVX(unsigned char *psrc2,unsigned char *ptmp2,int32_t tmp_pitch, int32_t src_row_size_16);
extern "C" void JPSDR_V_BlurR6c_8_AVX(unsigned char *psrc2,unsigned char *ptmp2,int32_t tmp_pitch, int32_t src_row_size_16);

extern "C" void JPSDR_H_BlurR6_16_SSE2(unsigned char *psrc2,unsigned char *ptmp2,int32_t src_row_size_16);
extern "C" void JPSDR_H_BlurR6a_16_SSE2(unsigned char *psrc2,unsigned char *ptmp2);
extern "C" void JPSDR_H_BlurR6b_16_SSE2(unsigned char *psrc2,unsigned char *ptmp2);
extern "C" void JPSDR_H_BlurR6c_16_SSE2(unsigned char *psrc2,unsigned char *ptmp2);

extern "C" void JPSDR_H_BlurR6_16_AVX(unsigned char *psrc2,unsigned char *ptmp2,int32_t src_row_size_16);
extern "C" void JPSDR_H_BlurR6a_16_AVX(unsigned char *psrc2,unsigned char *ptmp2);
extern "C" void JPSDR_H_BlurR6b_16_AVX(unsigned char *psrc2,unsigned char *ptmp2);
extern "C" void JPSDR_H_BlurR6c_16_AVX(unsigned char *psrc2,unsigned char *ptmp2);

extern "C" void JPSDR_V_BlurR6a_16_SSE2(unsigned char *psrc2,unsigned char *ptmp2,int32_t tmp_pitch, int32_t src_row_size_16);
extern "C" void JPSDR_V_BlurR6b_16_SSE2(unsigned char *psrc2,unsigned char *ptmp2,int32_t tmp_pitch, int32_t src_row_size_16);
extern "C" void JPSDR_V_BlurR6c_16_SSE2(unsigned char *psrc2,unsigned char *ptmp2,int32_t tmp_pitch, int32_t src_row_size_16);

extern "C" void JPSDR_V_BlurR6a_16_AVX(unsigned char *psrc2,unsigned char *ptmp2,int32_t tmp_pitch, int32_t src_row_size_16);
extern "C" void JPSDR_V_BlurR6b_16_AVX(unsigned char *psrc2,unsigned char *ptmp2,int32_t tmp_pitch, int32_t src_row_size_16);
extern "C" void JPSDR_V_BlurR6c_16_AVX(unsigned char *psrc2,unsigned char *ptmp2,int32_t tmp_pitch, int32_t src_row_size_16);

extern "C" void JPSDR_H_BlurR2_8_SSE2(unsigned char *psrc2,unsigned char *ptmp2,int32_t src_row_size_16);
extern "C" void JPSDR_H_BlurR2_8_AVX(unsigned char *psrc2,unsigned char *ptmp2,int32_t src_row_size,const unsigned char *dq0toF);

extern "C" void JPSDR_H_BlurR2a_8_SSE2(unsigned char *psrc2,unsigned char *ptmp2);

extern "C" void JPSDR_V_BlurR2_8_SSE2(unsigned char *psrc2,unsigned char *ptmp2,int32_t src_row_size_16,
	int32_t tmp_pitchp1,int32_t tmp_pitchp2,int32_t tmp_pitchn1,int32_t tmp_pitchn2);
extern "C" void JPSDR_V_BlurR2_8_AVX(unsigned char *psrc2,unsigned char *ptmp2,int32_t src_row_size_16,
	int32_t tmp_pitchp1,int32_t tmp_pitchp2,int32_t tmp_pitchn1,int32_t tmp_pitchn2);

extern "C" void JPSDR_H_BlurR2_16_SSE2(unsigned char *psrc2,unsigned char *ptmp2,int32_t src_row_size_16);
extern "C" void JPSDR_H_BlurR2a_16_SSE2(unsigned char *psrc2,unsigned char *ptmp2);

extern "C" void JPSDR_H_BlurR2_16_AVX(unsigned char *psrc2,unsigned char *ptmp2,int32_t src_row_size_16);
extern "C" void JPSDR_H_BlurR2a_16_AVX(unsigned char *psrc2,unsigned char *ptmp2);

extern "C" void JPSDR_V_BlurR2_16_SSE2(unsigned char *psrc2,unsigned char *ptmp2,int32_t src_row_size_16,
	int32_t tmp_pitchp1,int32_t tmp_pitchp2,int32_t tmp_pitchn1,int32_t tmp_pitchn2);
extern "C" void JPSDR_V_BlurR2_16_AVX(unsigned char *psrc2,unsigned char *ptmp2,int32_t src_row_size_16,
	int32_t tmp_pitchp1,int32_t tmp_pitchp2,int32_t tmp_pitchn1,int32_t tmp_pitchn2);

extern "C" void JPSDR_GuideChroma1_8_SSE2(const unsigned char *py,unsigned char *pu,int32_t pitch_y,int32_t width_uv_8);
extern "C" void JPSDR_GuideChroma2_8_SSE2(const unsigned char *py,unsigned char *pu,int32_t width_uv_8);
extern "C" void JPSDR_GuideChroma1_8_AVX(const unsigned char *py,unsigned char *pu,int32_t pitch_y,int32_t width_uv_8);
extern "C" void JPSDR_GuideChroma2_8_AVX(const unsigned char *py,unsigned char *pu,int32_t width_uv_8);

extern "C" void JPSDR_GuideChroma1_16_AVX(const uint16_t *py,uint16_t *pu,int32_t pitch_y,int32_t width_uv_8);
extern "C" void JPSDR_GuideChroma2_16_AVX(const uint16_t *py,uint16_t *pu,int32_t width_uv_8);

__declspec(align(16)) static const unsigned char dq0toF[0x10]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};


// warp0: SMAGL is 0
// warp2: SMAGL is 2 called from aWarp4
// uint8_t or uint16_t
template<int SMAGL,typename pixel_t>
static void warp_c(const unsigned char *srcp8,const unsigned char *edgep8,unsigned char *dstp8,const int32_t src_pitch_,
  const int32_t edge_pitch_,const int32_t dst_pitch_,const int32_t width,const int32_t height,int depth,int depthV,
  const uint8_t bits_per_sample)
{
  const pixel_t *srcp = (const pixel_t *)srcp8;
  const pixel_t *srcp2 = (const pixel_t *)(srcp8 + src_pitch_);
  const pixel_t *edgeptr = (const pixel_t *)edgep8;
  pixel_t *dstp = (pixel_t *)dstp8;

  const int32_t src_pitch = src_pitch_/sizeof(pixel_t);
  const int32_t edge_pitch = edge_pitch_/sizeof(pixel_t);
  const int32_t dst_pitch = dst_pitch_/sizeof(pixel_t);

  const int SMAG = 1 << SMAGL; // 180313

  const int wmod8 = (width >> 3) << 3;

  const int32_t c = width-1;

  const int32_t x_limit_min[8] = {0*SMAG,-1*SMAG,-2*SMAG,-3*SMAG,-4*SMAG,-5*SMAG,-6*SMAG,-7*SMAG};
  const int32_t x_limit_max[8] = {c*SMAG,(c-1)*SMAG,(c-2)*SMAG,(c-3)*SMAG,(c-4)*SMAG,(c-5)*SMAG,(c-6)*SMAG,(c-7)*SMAG};

  const int32_t pixel_max = (1 << bits_per_sample)-1;

  const int ARITH_BITS = 7;
  const int ARITH_ONE = (1 << ARITH_BITS); // 128
  const int ARITH_ROUNDER = (1 << (ARITH_BITS - 1)); // 64

  // depth is 8 bits, scale up to always have 30 bit result
  // bits_per_sample  diff of two uint_16   depth bitdepth   result bitdepth
  //                                        mul'd depth bd   max safe result bd
  //        16               17                    8            17+8=25
  //                                               13           17+13=30
  //        14               15                    8            15+8=23
  //                                               15           15+15=30
  //        12               13                    8            13+8=21
  //                                               17           13+17=30
  //        10               11                    8            11+8=19
  //                                               19           11+19=30
  //         8                9                    8             9+8=17  (like at 8 bit pixels, but there we shift the pixels by 7 instead of depthV )
  //                    diff shl7: 16 bits         8            16+8=24

  // good for 8 bit
  depth <<= (21-bits_per_sample);
  depthV <<= (21-bits_per_sample);

  for (int32_t y=0; y<height; y++)
  {
    const int32_t y_limit_min = -y*ARITH_ONE;
    const int32_t y_limit_max = (height-1-y)*ARITH_ONE -1;
    const int32_t edg_pitchp = (y!=0) ? -edge_pitch:0;
    const int32_t edg_pitchn = (y!=(height-1)) ? edge_pitch:0;

    const pixel_t *edgprev_ptr=edgeptr,*edgnext_ptr=edgeptr;
    pixel_t *dst=dstp;

    edgprev_ptr += edg_pitchp;
    edgnext_ptr += edg_pitchn;

    int32_t edge_right[8],edge_left[8];

    for (uint8_t i=1; i<8; i++)
    {
      edge_left[i]=edgeptr[i-1];
      edge_right[i]=edgeptr[i+1];
    }
    edge_left[0]=edge_left[1];
    edge_right[0]=edgeptr[1];

    for (int x=0; x<width; x+=8)
    {
      int32_t vert[8], horiz[8];
      for (uint8_t i=0; i<8; i++)
      {
        // depth is -128..127, src is scaled up to give mul result of 30 bits
        // we are shifting it back by 12 to get a 7-bit fractional integer arithmetic
        horiz[i]=((edge_left[i]-edge_right[i])*depth) >> 14; // depth scaled left-right
        vert[i]=((edgprev_ptr[x+i]-edgnext_ptr[x+i])*depthV) >> 14; // depthV scaled top-bottom
      }
      // guard vertical offsets
      for (uint8_t i=0; i<8; i++)
        vert[i]=std::max(std::min(vert[i],y_limit_max),y_limit_min);

      int32_t horiz_weight[8];
      int32_t vert_weight[8];
      for (uint8_t i=0; i<8; i++)
      {
        // remainder of the division by 128 (or 32 if it was shifted left by 2 above)
        horiz_weight[i]=(horiz[i] << SMAGL) & 0x7F; // fractional part: for fine weighting
        vert_weight[i]=(vert[i]  << SMAGL) & 0x7F;
        horiz[i] >>= (7-SMAGL); // integer part: the offset itself
        vert[i] >>= (7-SMAGL); // shift by 7 (or 5); division by 128 (or 32)
      }

      // horizontal things
      for (uint8_t i=0; i<8; i++)
        horiz[i] += x << SMAGL;

      // guard horizontal offsets min/max
      int32_t horiz_offset_x1[8];
      for (uint8_t i=0; i<8; i++)
        horiz_offset_x1[i]=std::max(std::min(x_limit_max[i],horiz[i]),x_limit_min[i]);

      // mask out out-of-screen offset weights
      bool b0[8],b3[8];
      for (uint8_t i=0; i<8; i++)
      {
        b3[i] = x_limit_max[i]>(horiz[i]);
        b0[i] = x_limit_min[i]>(horiz[i]);
      }
      for (uint8_t i=0; i<8; i++)
        horiz_weight[i] = b3[i] ? horiz_weight[i]:0; // x7 = keep x7 where b3=true (over max)
      for (uint8_t i=0; i<8; i++)
        horiz_weight[i] = (!b0[i]) ? horiz_weight[i]:0; // x0 = x7 where b0=false (below min)
      // keep horizontal weights where out of limit, otherwise zero

      int32_t curr_0[8],curr_1[8];
      int32_t bottom_0[8],bottom_1[8];
      for (uint8_t i=0; i<8; i++)
      {
        // combine vertical and horizontal offset things
        const int32_t offs=vert[i]*src_pitch+horiz_offset_x1[i]+i;
        // for 8 bit: _mm_insert_epi16 is split to 00FF (offs) and FF00 (offs)
        // for 16 bit: _mm_insert_epi32 is split to 0000FFFF (offs) and FFFF0000 (offs+1)
        curr_0[i] = (int32_t)srcp[offs];       // lo FFFF from 
        curr_1[i] = (int32_t)srcp[offs+1];
        bottom_0[i] = (int32_t)srcp2[offs];    // next line
        bottom_1[i] = (int32_t)srcp2[offs+1];// next line
      }

      int32_t curr[8],bottom[8];
      // curr = curr_0 * (1 - x0/128) + curr_1 * (x0/128)
      // bottom = bottom_0 * (1 - x0/128) + bottom_1 * (x0/128)
      for (int i=0; i<8; i++)
      {
        const int32_t a=horiz_weight[i],b=ARITH_ONE-a;

        curr[i] = ((curr_0[i]*b+curr_1[i]*a)+ARITH_ROUNDER) >> ARITH_BITS;
        bottom[i] = ((bottom_0[i]*b+bottom_1[i]*a)+ARITH_ROUNDER) >> ARITH_BITS;
      }

      // result = result * (1 - vert_weight_x4/128) + x1 * (vert_weight_x4/128)
      int32_t result[8];
      for (int i=0; i<8; i++)
      {
        const int32_t a=vert_weight[i],b=ARITH_ONE-a;
        result[i] = ((curr[i]*b+bottom[i]*a)+ARITH_ROUNDER) >> ARITH_BITS;
      }

      // clamp pixel min max
      for (int i=0; i<8; i++)
        result[i] = std::min(std::max(result[i],0),pixel_max);

      if (x>=wmod8)
      {
        for (int i=0; i<(width-wmod8); i++)
          dst[x+i]=result[i];
        break; // finito, no more preload
      }
      for (int i=0; i<8; i++)
        dst[x+i]=result[i];

      // preload for next loop
      for (int i=0; i<8; i++)
      {
        edge_left[i]=edgeptr[x+i+8-1];
        edge_right[i]=edgeptr[x+i+8+1];
      }
    }

    srcp += src_pitch*SMAG; // 180313
    srcp2 += src_pitch*SMAG;// 180313
    edgeptr += edge_pitch;
    dstp += dst_pitch;
  }
}

// warp0: SMAGL is 0
// warp2: SMAGL is 2 called from aWarp4
// uint8_t or uint16_t
template<int SMAGL,typename pixel_t>
static void warp_c_MT(const unsigned char *srcp8,const unsigned char *edgep8,unsigned char *dstp8,const int32_t src_pitch_,
  const int32_t edge_pitch_,const int32_t dst_pitch_,const int32_t width,const int32_t height,int depth,int depthV,
  const uint8_t bits_per_sample,const int32_t ymin,const int32_t ymax)
{
  const int SMAG = 1 << SMAGL; // 180313

  const pixel_t *srcp = (const pixel_t *)(srcp8+(ymin*SMAG*src_pitch_));
  const pixel_t *srcp2 = (const pixel_t *)(srcp8+((ymin*SMAG+1)*src_pitch_));
  const pixel_t *edgeptr = (const pixel_t *)(edgep8+(ymin*edge_pitch_));
  pixel_t *dstp = (pixel_t *)(dstp8+(ymin*dst_pitch_));

  const int32_t src_pitch = src_pitch_/sizeof(pixel_t);
  const int32_t edge_pitch = edge_pitch_/sizeof(pixel_t);
  const int32_t dst_pitch = dst_pitch_/sizeof(pixel_t);

  const int wmod8 = (width >> 3) << 3;

  const int32_t c = width-1;

  const int32_t x_limit_min[8] = {0*SMAG,-1*SMAG,-2*SMAG,-3*SMAG,-4*SMAG,-5*SMAG,-6*SMAG,-7*SMAG};
  const int32_t x_limit_max[8] = {c*SMAG,(c-1)*SMAG,(c-2)*SMAG,(c-3)*SMAG,(c-4)*SMAG,(c-5)*SMAG,(c-6)*SMAG,(c-7)*SMAG};

  const int32_t pixel_max = (1 << bits_per_sample)-1;

  const int ARITH_BITS = 7;
  const int ARITH_ONE = (1 << ARITH_BITS); // 128
  const int ARITH_ROUNDER = (1 << (ARITH_BITS - 1)); // 64

  // depth is 8 bits, scale up to always have 30 bit result
  // bits_per_sample  diff of two uint_16   depth bitdepth   result bitdepth
  //                                        mul'd depth bd   max safe result bd
  //        16               17                    8            17+8=25
  //                                               13           17+13=30
  //        14               15                    8            15+8=23
  //                                               15           15+15=30
  //        12               13                    8            13+8=21
  //                                               17           13+17=30
  //        10               11                    8            11+8=19
  //                                               19           11+19=30
  //         8                9                    8             9+8=17  (like at 8 bit pixels, but there we shift the pixels by 7 instead of depthV )
  //                    diff shl7: 16 bits         8            16+8=24

  // good for 8 bit
  depth <<= (21-bits_per_sample);
  depthV <<= (21-bits_per_sample);

  for (int32_t y=ymin; y<ymax; y++)
  {
    const int32_t y_limit_min = -y*ARITH_ONE;
    const int32_t y_limit_max = (height-1-y)*ARITH_ONE -1;
    const int32_t edg_pitchp = (y!=0) ? -edge_pitch:0;
    const int32_t edg_pitchn = (y!=(height-1)) ? edge_pitch:0;

    const pixel_t *edgprev_ptr=edgeptr,*edgnext_ptr=edgeptr;
    pixel_t *dst=dstp;

    edgprev_ptr += edg_pitchp;
    edgnext_ptr += edg_pitchn;

    int32_t edge_right[8],edge_left[8];

    for (uint8_t i=1; i<8; i++)
    {
      edge_left[i]=edgeptr[i-1];
      edge_right[i]=edgeptr[i+1];
    }
    edge_left[0]=edge_left[1];
    edge_right[0]=edgeptr[1];

    for (int x=0; x<width; x+=8)
    {
      int32_t vert[8], horiz[8];
      for (uint8_t i=0; i<8; i++)
      {
        // depth is -128..127, src is scaled up to give mul result of 30 bits
        // we are shifting it back by 12 to get a 7-bit fractional integer arithmetic
        horiz[i]=((edge_left[i]-edge_right[i])*depth) >> 14; // depth scaled left-right
        vert[i]=((edgprev_ptr[x+i]-edgnext_ptr[x+i])*depthV) >> 14; // depthV scaled top-bottom
      }
      // guard vertical offsets
      for (uint8_t i=0; i<8; i++)
        vert[i]=std::max(std::min(vert[i],y_limit_max),y_limit_min);

      int32_t horiz_weight[8];
      int32_t vert_weight[8];
      for (uint8_t i=0; i<8; i++)
      {
        // remainder of the division by 128 (or 32 if it was shifted left by 2 above)
        horiz_weight[i]=(horiz[i] << SMAGL) & 0x7F; // fractional part: for fine weighting
        vert_weight[i]=(vert[i]  << SMAGL) & 0x7F;
        horiz[i] >>= (7-SMAGL); // integer part: the offset itself
        vert[i] >>= (7-SMAGL); // shift by 7 (or 5); division by 128 (or 32)
      }

      // horizontal things
      for (uint8_t i=0; i<8; i++)
        horiz[i] += x << SMAGL;

      // guard horizontal offsets min/max
      int32_t horiz_offset_x1[8];
      for (uint8_t i=0; i<8; i++)
        horiz_offset_x1[i]=std::max(std::min(x_limit_max[i],horiz[i]),x_limit_min[i]);

      // mask out out-of-screen offset weights
      bool b0[8],b3[8];
      for (uint8_t i=0; i<8; i++)
      {
        b3[i] = x_limit_max[i]>(horiz[i]);
        b0[i] = x_limit_min[i]>(horiz[i]);
      }
      for (uint8_t i=0; i<8; i++)
        horiz_weight[i] = b3[i] ? horiz_weight[i]:0; // x7 = keep x7 where b3=true (over max)
      for (uint8_t i=0; i<8; i++)
        horiz_weight[i] = (!b0[i]) ? horiz_weight[i]:0; // x0 = x7 where b0=false (below min)
      // keep horizontal weights where out of limit, otherwise zero

      int32_t curr_0[8],curr_1[8];
      int32_t bottom_0[8],bottom_1[8];
      for (uint8_t i=0; i<8; i++)
      {
        // combine vertical and horizontal offset things
        const int32_t offs=vert[i]*src_pitch+horiz_offset_x1[i]+i;
        // for 8 bit: _mm_insert_epi16 is split to 00FF (offs) and FF00 (offs)
        // for 16 bit: _mm_insert_epi32 is split to 0000FFFF (offs) and FFFF0000 (offs+1)
        curr_0[i] = (int32_t)srcp[offs];       // lo FFFF from 
        curr_1[i] = (int32_t)srcp[offs+1];
        bottom_0[i] = (int32_t)srcp2[offs];    // next line
        bottom_1[i] = (int32_t)srcp2[offs+1];// next line
      }

      int32_t curr[8],bottom[8];
      // curr = curr_0 * (1 - x0/128) + curr_1 * (x0/128)
      // bottom = bottom_0 * (1 - x0/128) + bottom_1 * (x0/128)
      for (int i=0; i<8; i++)
      {
        const int32_t a=horiz_weight[i],b=ARITH_ONE-a;

        curr[i] = ((curr_0[i]*b+curr_1[i]*a)+ARITH_ROUNDER) >> ARITH_BITS;
        bottom[i] = ((bottom_0[i]*b+bottom_1[i]*a)+ARITH_ROUNDER) >> ARITH_BITS;
      }

      // result = result * (1 - vert_weight_x4/128) + x1 * (vert_weight_x4/128)
      int32_t result[8];
      for (int i=0; i<8; i++)
      {
        const int32_t a=vert_weight[i],b=ARITH_ONE-a;
        result[i] = ((curr[i]*b+bottom[i]*a)+ARITH_ROUNDER) >> ARITH_BITS;
      }

      // clamp pixel min max
      for (int i=0; i<8; i++)
        result[i] = std::min(std::max(result[i],0),pixel_max);

      if (x>=wmod8)
      {
        for (int i=0; i<(width-wmod8); i++)
          dst[x+i]=result[i];
        break; // finito, no more preload
      }
      for (int i=0; i<8; i++)
        dst[x+i]=result[i];

      // preload for next loop
      for (int i=0; i<8; i++)
      {
        edge_left[i]=edgeptr[x+i+8-1];
        edge_right[i]=edgeptr[x+i+8+1];
      }
    }

    srcp += src_pitch*SMAG; // 180313
    srcp2 += src_pitch*SMAG;// 180313
    edgeptr += edge_pitch;
    dstp += dst_pitch;
  }
}


// minimum sse4. _mm_mullo_epi32, _mm_insert_epi32, etc...
// warp0: SMAGL is 0, call with warp<0,
// warp2: SMAGL is 2, call with warp called from aWarp4
// uint8_t or uint16_t
template<int SMAGL, bool lessthan16bits>
#if defined(CLANG)
__attribute__((__target__("sse4.1")))
#endif
static void warp_u16_sse41_core(const unsigned char *srcp8,const unsigned char *edgep8,unsigned char *dstp8,const int32_t src_pitch_,
  const int32_t edge_pitch_,const int32_t dst_pitch_,const int32_t width,const int32_t height,int depth,int depthV,
  const uint8_t bits_per_sample)
{
  const uint16_t *srcp = (const uint16_t *)srcp8;
  const uint16_t *srcp2 = (const uint16_t *)(srcp8+src_pitch_);
  const uint16_t *edgeptr = (const uint16_t *)edgep8;
  uint16_t *dstp = (uint16_t *)dstp8;

  const int32_t src_pitch = src_pitch_ >> 1;
  const int32_t edge_pitch = edge_pitch_ >> 1;
  const int32_t dst_pitch = dst_pitch_ >> 1;

  const int SMAG = 1 << SMAGL; // 180313

  const int wmod8 = (width >> 3) << 3;

  const int32_t c=width-1;

  // xlimits: 16 bits
  const __m128i x_limit_min = _mm_setr_epi16(0*SMAG,-1*SMAG,-2*SMAG,-3*SMAG,-4*SMAG,-5*SMAG,-6*SMAG,-7*SMAG);
  const __m128i x_limit_max = _mm_setr_epi16(c*SMAG,(c-1)*SMAG,(c-2)*SMAG,(c-3)*SMAG,(c-4)*SMAG,(c-5)*SMAG,(c-6)*SMAG,(c-7)*SMAG);

  const __m128i pixel_max = _mm_set1_epi16((1 << bits_per_sample)-1);

  const int ARITH_BITS = 7;
  const int ARITH_ONE = (1 << ARITH_BITS); // 128
  const int ARITH_ROUNDER = (1 << (ARITH_BITS - 1)); // 64

  // depth is 8 bits, scale up to always have 30 bit result
  // bits_per_sample  diff of two uint_16   depth bitdepth   result bitdepth
  //                                        mul'd depth bd   max safe result bd
  //        16               17                    8            17+8=25
  //                                               13           17+13=30
  //        14               15                    8            15+8=23
  //                                               15           15+15=30
  //        12               13                    8            13+8=21
  //                                               17           13+17=30
  //        10               11                    8            11+8=19
  //                                               19           11+19=30
  //         8                9                    8             9+8=17  (like at 8 bit pixels, but there we shift the pixels by 7 instead of depthV )
  //                    diff shl7: 16 bits         8            16+8=24

  depth <<= (21-bits_per_sample);
  depthV <<= (21-bits_per_sample);

  const __m128i depth128 = _mm_set1_epi32(depth);
  const __m128i depthV128 = _mm_set1_epi32(depthV);
  const __m128i zero = _mm_setzero_si128();

  for (int32_t y=0; y<height; y++)
  {
    const __m128i y_limit_min = _mm_set1_epi32(-y*ARITH_ONE);
    const __m128i y_limit_max = _mm_set1_epi32((height-1-y)*ARITH_ONE -1);
    const int32_t edg_pitchp = (y!=0) ? -edge_pitch:0;
    const int32_t edg_pitchn = (y!=(height-1)) ? edge_pitch:0;

    const uint16_t *edgprev_ptr=edgeptr,*edgnext_ptr=edgeptr;
    uint16_t *dst = dstp;

    edgprev_ptr += edg_pitchp;
    edgnext_ptr += edg_pitchn;

    // fill leftmost (cannot do -1)
    __m128i edge_left = _mm_load_si128(reinterpret_cast<const __m128i *>(edgeptr)); // edgeptr - 1
    __m128i edge_leftmost = _mm_and_si128(edge_left,_mm_setr_epi16(-1,0,0,0,0,0,0,0)); // 0xFFFF
    edge_left = _mm_or_si128(_mm_slli_si128(edge_left,2),edge_leftmost); // shift 1 words, keep lo word

   // FIX 180318 PF: loadu instead of load.
    __m128i edge_right = _mm_loadu_si128(reinterpret_cast<const __m128i *>(edgeptr+1));

    // 8 pixel at a time (16 bytes, full 128 bit lane)
    // when 32 bit arithmetic needed we have to separate to low and high
    for (int x=0; x<width; x+=8) 
    {
      // FIX 180318 PF: depth instead of depthV in remark.
      // (left-right)*depth  >> 14
      __m128i edge_left_lo = _mm_unpacklo_epi16(edge_left,zero);
      __m128i edge_left_hi = _mm_unpackhi_epi16(edge_left,zero);
      __m128i edge_right_lo = _mm_unpacklo_epi16(edge_right,zero);
      __m128i edge_right_hi = _mm_unpackhi_epi16(edge_right,zero);

      __m128i horiz_lo = _mm_srai_epi32(_mm_mullo_epi32(_mm_sub_epi32(edge_left_lo,edge_right_lo),depth128),14);
      __m128i horiz_hi = _mm_srai_epi32(_mm_mullo_epi32(_mm_sub_epi32(edge_left_hi,edge_right_hi),depth128),14);

      // (bottom-top)*depthV  >> 14
      __m128i edge_prev = _mm_load_si128(reinterpret_cast<const __m128i *>(edgprev_ptr+x));
      __m128i edge_next = _mm_load_si128(reinterpret_cast<const __m128i *>(edgnext_ptr+x));
      __m128i edge_prev_lo = _mm_unpacklo_epi16(edge_prev,zero);
      __m128i edge_prev_hi = _mm_unpackhi_epi16(edge_prev,zero);
      __m128i edge_next_lo = _mm_unpacklo_epi16(edge_next,zero);
      __m128i edge_next_hi = _mm_unpackhi_epi16(edge_next,zero);

      __m128i vert_lo = _mm_srai_epi32(_mm_mullo_epi32(_mm_sub_epi32(edge_prev_lo,edge_next_lo),depthV128),14);
      __m128i vert_hi = _mm_srai_epi32(_mm_mullo_epi32(_mm_sub_epi32(edge_prev_hi,edge_next_hi),depthV128),14);

      // guard vertical offsets
      vert_lo = _mm_max_epi32(_mm_min_epi32(vert_lo,y_limit_max),y_limit_min);
      vert_hi = _mm_max_epi32(_mm_min_epi32(vert_hi,y_limit_max),y_limit_min);

      // 7-bit fractional part integer arithmetic

      // fractional part: for fine weighting
      const __m128i mask7F_32bit = _mm_set1_epi32(0x7F);
      __m128i horiz_lo_pre = horiz_lo;
      __m128i horiz_hi_pre = horiz_hi;
      __m128i vert_lo_pre = vert_lo;
      __m128i vert_hi_pre = vert_hi;
      if (SMAGL)
	  {
        // shift by 2; multiply by 4
        // FIX 180318 PF: slli instead of srli (when warp2).
        horiz_lo_pre = _mm_slli_epi32(horiz_lo_pre,SMAGL);
        horiz_hi_pre = _mm_slli_epi32(horiz_hi_pre,SMAGL);
        vert_lo_pre = _mm_slli_epi32(vert_lo_pre,SMAGL);
        vert_hi_pre = _mm_slli_epi32(vert_hi_pre,SMAGL);
      }
      __m128i horiz_weight = _mm_packs_epi32(_mm_and_si128(horiz_lo_pre, mask7F_32bit),_mm_and_si128(horiz_hi_pre, mask7F_32bit));
      __m128i vert_weight = _mm_packs_epi32(_mm_and_si128(vert_lo_pre, mask7F_32bit),_mm_and_si128(vert_hi_pre, mask7F_32bit));

      // integer part: the offset itself
      // FIX 180318 PF: No rounding. Same as C.
      horiz_lo = _mm_srai_epi32(horiz_lo,(7-SMAGL));
      horiz_hi = _mm_srai_epi32(horiz_hi,(7-SMAGL));
      __m128i horiz = _mm_packs_epi32(horiz_lo,horiz_hi); // signed 16 bits
      vert_lo = _mm_srai_epi32(vert_lo,7-SMAGL);
      vert_hi = _mm_srai_epi32(vert_hi,7-SMAGL);
      __m128i vert = _mm_packs_epi32(vert_lo,vert_hi);

      // horizontal things
      // add offset
      horiz = _mm_add_epi16(horiz,_mm_set1_epi16(x << SMAGL)); // still 16 bits
                                                         // guard horizontal offsets min/max
      __m128i horiz_offset = _mm_max_epi16(_mm_min_epi16(x_limit_max,horiz),x_limit_min);

      // mask out-of-screen offset weights
      horiz_weight = _mm_and_si128(horiz_weight,_mm_cmpgt_epi16(x_limit_max,horiz)); // mask out over max
      // FIX 180318 PF: change param order in andnot.
      horiz_weight = _mm_andnot_si128(_mm_cmpgt_epi16(x_limit_min,horiz),horiz_weight); // mask out below min

      // combine vertical and horizontal offset
      __m128i srcpitch_and_one = _mm_unpacklo_epi16(_mm_set1_epi16(src_pitch),_mm_set1_epi16(1));
      // const int32_t offs = (vert[i] * src_pitch) + (horiz_offset[i] * 1)     ?? + i; <- i offset later directly
      __m128i offs_lo = _mm_madd_epi16(_mm_unpacklo_epi16(vert,horiz_offset),srcpitch_and_one);
      __m128i offs_hi = _mm_madd_epi16(_mm_unpackhi_epi16(vert,horiz_offset),srcpitch_and_one);

      // read 2x8 pixels
      __m128i curr_01_lo = _mm_undefined_si128();
      __m128i curr_01_hi = _mm_undefined_si128();
      __m128i bottom_01_lo = _mm_undefined_si128();
      __m128i bottom_01_hi = _mm_undefined_si128();
      int offs;
      // lo
      // #0
      offs = _mm_cvtsi128_si32(offs_lo);
      offs_lo = _mm_srli_si128(offs_lo,4);

      // FIX 180318 PF: use *(uint32_t*)& everywhere (8x2 line)!.

      // stuff even and odd pixel pair, see later curr_0 and curr_1, bottom_0 and bottom_1
      curr_01_lo = _mm_insert_epi32(curr_01_lo,*(uint32_t *)&srcp[offs+0],0);
      bottom_01_lo = _mm_insert_epi32(bottom_01_lo,*(uint32_t *)&srcp2[offs+0],0);
      // #1
      offs = _mm_cvtsi128_si32(offs_lo);
      offs_lo = _mm_srli_si128(offs_lo,4);
      curr_01_lo = _mm_insert_epi32(curr_01_lo,*(uint32_t *)&srcp[offs+1],1);
      bottom_01_lo = _mm_insert_epi32(bottom_01_lo,*(uint32_t *)&srcp2[offs+1],1);
      // #2
      offs = _mm_cvtsi128_si32(offs_lo);
      offs_lo = _mm_srli_si128(offs_lo,4);
      curr_01_lo = _mm_insert_epi32(curr_01_lo,*(uint32_t *)&srcp[offs+2],2);
      bottom_01_lo = _mm_insert_epi32(bottom_01_lo,*(uint32_t *)&srcp2[offs+2],2);
      // #3
      offs = _mm_cvtsi128_si32(offs_lo);
      // not used anymore // offs_lo = _mm_srli_si128(offs_lo,4);
      curr_01_lo = _mm_insert_epi32(curr_01_lo,*(uint32_t *)&srcp[offs+ 3],3);
      bottom_01_lo = _mm_insert_epi32(bottom_01_lo,*(uint32_t *)&srcp2[offs+3],3);
      // high part, source offset is 4-7, target offset is 0-3 again
      // #4
      offs = _mm_cvtsi128_si32(offs_hi);
      offs_hi = _mm_srli_si128(offs_hi,4);
      curr_01_hi = _mm_insert_epi32(curr_01_hi,*(uint32_t *)&srcp[offs+4],0);
      bottom_01_hi = _mm_insert_epi32(bottom_01_hi,*(uint32_t *)&srcp2[offs+4],0);
      // #5
      offs = _mm_cvtsi128_si32(offs_hi);
      offs_hi = _mm_srli_si128(offs_hi,4);
      curr_01_hi = _mm_insert_epi32(curr_01_hi,*(uint32_t *)&srcp[offs+5],1);
      bottom_01_hi = _mm_insert_epi32(bottom_01_hi,*(uint32_t *)&srcp2[offs+5],1);
      // #6
      offs = _mm_cvtsi128_si32(offs_hi);
      offs_hi = _mm_srli_si128(offs_hi,4);
      curr_01_hi = _mm_insert_epi32(curr_01_hi,*(uint32_t *)&srcp[offs+6],2);
      bottom_01_hi = _mm_insert_epi32(bottom_01_hi,*(uint32_t *)&srcp2[offs+6],2);
      // #7
      offs = _mm_cvtsi128_si32(offs_hi);
      // not used anymore // offs_hi = _mm_srli_si128(offs_hi, 4);
      curr_01_hi = _mm_insert_epi32(curr_01_hi,*(uint32_t *)&srcp[offs+7],3);
      bottom_01_hi = _mm_insert_epi32(bottom_01_hi,*(uint32_t *)&srcp2[offs+7],3);

      // combine even and odds together
      __m128i mask0000FFFF = _mm_set1_epi32(0xFFFF);
      __m128i curr_0 = _mm_packus_epi32(_mm_and_si128(curr_01_lo,mask0000FFFF),_mm_and_si128(curr_01_hi,mask0000FFFF)); // evens
      __m128i curr_1 = _mm_packus_epi32(_mm_srli_epi32(curr_01_lo,16),_mm_srli_epi32(curr_01_hi,16)); // odds
      __m128i bottom_0 = _mm_packus_epi32(_mm_and_si128(bottom_01_lo,mask0000FFFF),_mm_and_si128(bottom_01_hi,mask0000FFFF)); // evens
      __m128i bottom_1 = _mm_packus_epi32(_mm_srli_epi32(bottom_01_lo,16),_mm_srli_epi32(bottom_01_hi,16)); // odds

      // calculate weighted averages of the four pixel groups
      // each pixel position has one vertical and one horizontal weight (0 to 127)
      __m128i one = _mm_set1_epi16(ARITH_ONE);
      __m128i rounder32bit = _mm_set1_epi32(ARITH_ROUNDER);
      __m128i shifter_to_signed = _mm_set1_epi16(-32768); // to allow signed mul for unsigned 16 bit

      // trick: convert to signed 16 bit (really this is not needed for 10-14 bits)
      if (!lessthan16bits)
      {
        curr_0 = _mm_sub_epi16(curr_0,shifter_to_signed);
        curr_1 = _mm_sub_epi16(curr_1,shifter_to_signed);
        bottom_0 = _mm_sub_epi16(bottom_0,shifter_to_signed);
        bottom_1 = _mm_sub_epi16(bottom_1,shifter_to_signed);
      }

      // curr = curr_0 * (1 - x0/128) + curr_1 * (x0/128)
      // bottom = bottom_0 * (1 - x0/128) + bottom_1 * (x0/128)
      __m128i inv_horiz_weight = _mm_sub_epi16(one,horiz_weight);
      
      __m128i curr_lo = _mm_madd_epi16(_mm_unpacklo_epi16(curr_0,curr_1),_mm_unpacklo_epi16(inv_horiz_weight,horiz_weight));
      __m128i curr_hi = _mm_madd_epi16(_mm_unpackhi_epi16(curr_0,curr_1),_mm_unpackhi_epi16(inv_horiz_weight,horiz_weight));
      curr_lo = _mm_srai_epi32(_mm_add_epi32(curr_lo,rounder32bit),ARITH_BITS);
      curr_hi = _mm_srai_epi32(_mm_add_epi32(curr_hi,rounder32bit),ARITH_BITS);
      // back to signed 16 bits
      __m128i curr = _mm_packs_epi32(curr_lo,curr_hi);

      __m128i bottom_lo = _mm_madd_epi16(_mm_unpacklo_epi16(bottom_0,bottom_1),_mm_unpacklo_epi16(inv_horiz_weight,horiz_weight));
      __m128i bottom_hi = _mm_madd_epi16(_mm_unpackhi_epi16(bottom_0,bottom_1),_mm_unpackhi_epi16(inv_horiz_weight,horiz_weight));
      bottom_lo = _mm_srai_epi32(_mm_add_epi32(bottom_lo,rounder32bit),ARITH_BITS);
      bottom_hi = _mm_srai_epi32(_mm_add_epi32(bottom_hi,rounder32bit),ARITH_BITS);
      // back to signed 16 bits
      __m128i bottom = _mm_packs_epi32(bottom_lo,bottom_hi);

      // result = curr * (1 - vert_weight/128) + bottom * (vert_weight/128)
      __m128i inv_vert_weight = _mm_sub_epi16(one,vert_weight);
      __m128i result_lo = _mm_madd_epi16(_mm_unpacklo_epi16(curr,bottom),_mm_unpacklo_epi16(inv_vert_weight,vert_weight));
      __m128i result_hi = _mm_madd_epi16(_mm_unpackhi_epi16(curr,bottom),_mm_unpackhi_epi16(inv_vert_weight,vert_weight));
      result_lo = _mm_srai_epi32(_mm_add_epi32(result_lo,rounder32bit),ARITH_BITS);
      result_hi = _mm_srai_epi32(_mm_add_epi32(result_hi,rounder32bit),ARITH_BITS);

      // combine clamp pixel min max

      // packs clamps -32768..32767 (0..65535 after going back to unsigned)
      __m128i result = _mm_packs_epi32(result_lo,result_hi);
      if (!lessthan16bits)
	  {
        // trick: convert to back to unsigned 16 bit (really this is not needed for 10-14 bits)
        result = _mm_add_epi16(result,shifter_to_signed);
      }
      if (lessthan16bits)
	  {
        // clamp to valid pixel range, not needed for exact 16 bit, since packs has done that
        result = _mm_min_epu16(result,pixel_max);
      }
      
      // for avs+ and 10-16bits pixels we can safely ignore range check, avs+ is guaranteed to have at least 32 bit alignment
      // we need here only align 16
      _mm_store_si128(reinterpret_cast<__m128i *>(dst+x),result);
      // if (x >= wmod8), store less bytes // not needed!

      // preload next 8 pixels
      if (x<wmod8)
	  { // prevent overread
        edge_left = _mm_loadu_si128(reinterpret_cast<const __m128i *>(edgeptr+x+8-1));
        edge_right = _mm_loadu_si128(reinterpret_cast<const __m128i *>(edgeptr+x+8+1));
      }
    }

    srcp += src_pitch*SMAG;
    srcp2 += src_pitch*SMAG;
    edgeptr += edge_pitch;
    dstp += dst_pitch;
  }
}


// minimum sse4. _mm_mullo_epi32, _mm_insert_epi32, etc...
// warp0: SMAGL is 0, call with warp<0,
// warp2: SMAGL is 2, call with warp called from aWarp4
// uint8_t or uint16_t
template<int SMAGL,bool lessthan16bits>
#if defined(CLANG)
__attribute__((__target__("sse4.1")))
#endif
static void warp_u16_sse41_core_MT(const unsigned char *srcp8,const unsigned char *edgep8,unsigned char *dstp8,const int32_t src_pitch_,
  const int32_t edge_pitch_,const int32_t dst_pitch_,const int32_t width,const int32_t height,int depth,int depthV,
  const uint8_t bits_per_sample,const int32_t ymin,const int32_t ymax)
{

  const int SMAG = 1 << SMAGL; // 180313

  const uint16_t *srcp = (const uint16_t *)(srcp8+(ymin*SMAG*src_pitch_));
  const uint16_t *srcp2 = (const uint16_t *)(srcp8+((ymin*SMAG+1)*src_pitch_));
  const uint16_t *edgeptr = (const uint16_t *)(edgep8+(ymin*edge_pitch_));
  uint16_t *dstp = (uint16_t *)(dstp8+(ymin*dst_pitch_));

  const int32_t src_pitch = src_pitch_ >> 1;
  const int32_t edge_pitch = edge_pitch_ >> 1;
  const int32_t dst_pitch = dst_pitch_ >> 1;

  const int wmod8 = (width >> 3) << 3;

  const int32_t c=width-1;

  // xlimits: 16 bits
  const __m128i x_limit_min = _mm_setr_epi16(0*SMAG,-1*SMAG,-2*SMAG,-3*SMAG,-4*SMAG,-5*SMAG,-6*SMAG,-7*SMAG);
  const __m128i x_limit_max = _mm_setr_epi16(c*SMAG,(c-1)*SMAG,(c-2)*SMAG,(c-3)*SMAG,(c-4)*SMAG,(c-5)*SMAG,(c-6)*SMAG,(c-7)*SMAG);

  const __m128i pixel_max = _mm_set1_epi16((1 << bits_per_sample)-1);

  const int ARITH_BITS = 7;
  const int ARITH_ONE = (1 << ARITH_BITS); // 128
  const int ARITH_ROUNDER = (1 << (ARITH_BITS - 1)); // 64

  // depth is 8 bits, scale up to always have 30 bit result
  // bits_per_sample  diff of two uint_16   depth bitdepth   result bitdepth
  //                                        mul'd depth bd   max safe result bd
  //        16               17                    8            17+8=25
  //                                               13           17+13=30
  //        14               15                    8            15+8=23
  //                                               15           15+15=30
  //        12               13                    8            13+8=21
  //                                               17           13+17=30
  //        10               11                    8            11+8=19
  //                                               19           11+19=30
  //         8                9                    8             9+8=17  (like at 8 bit pixels, but there we shift the pixels by 7 instead of depthV )
  //                    diff shl7: 16 bits         8            16+8=24

  depth <<= (21-bits_per_sample);
  depthV <<= (21-bits_per_sample);

  const __m128i depth128 = _mm_set1_epi32(depth);
  const __m128i depthV128 = _mm_set1_epi32(depthV);
  const __m128i zero = _mm_setzero_si128();

  for (int32_t y=ymin; y<ymax; y++)
  {
    const __m128i y_limit_min = _mm_set1_epi32(-y*ARITH_ONE);
    const __m128i y_limit_max = _mm_set1_epi32((height-1-y)*ARITH_ONE -1);
    const int32_t edg_pitchp = (y!=0) ? -edge_pitch:0;
    const int32_t edg_pitchn = (y!=(height-1)) ? edge_pitch:0;

    const uint16_t *edgprev_ptr=edgeptr,*edgnext_ptr=edgeptr;
    uint16_t *dst = dstp;

    edgprev_ptr += edg_pitchp;
    edgnext_ptr += edg_pitchn;

    // fill leftmost (cannot do -1)
    __m128i edge_left = _mm_load_si128(reinterpret_cast<const __m128i *>(edgeptr)); // edgeptr - 1
    __m128i edge_leftmost = _mm_and_si128(edge_left,_mm_setr_epi16(-1,0,0,0,0,0,0,0)); // 0xFFFF
    edge_left = _mm_or_si128(_mm_slli_si128(edge_left,2),edge_leftmost); // shift 1 words, keep lo word

   // FIX 180318 PF: loadu instead of load.
    __m128i edge_right = _mm_loadu_si128(reinterpret_cast<const __m128i *>(edgeptr+1));

    // 8 pixel at a time (16 bytes, full 128 bit lane)
    // when 32 bit arithmetic needed we have to separate to low and high
    for (int x=0; x<width; x+=8) 
    {
      // FIX 180318 PF: depth instead of depthV in remark.
      // (left-right)*depth  >> 14
      __m128i edge_left_lo = _mm_unpacklo_epi16(edge_left,zero);
      __m128i edge_left_hi = _mm_unpackhi_epi16(edge_left,zero);
      __m128i edge_right_lo = _mm_unpacklo_epi16(edge_right,zero);
      __m128i edge_right_hi = _mm_unpackhi_epi16(edge_right,zero);

      __m128i horiz_lo = _mm_srai_epi32(_mm_mullo_epi32(_mm_sub_epi32(edge_left_lo,edge_right_lo),depth128),14);
      __m128i horiz_hi = _mm_srai_epi32(_mm_mullo_epi32(_mm_sub_epi32(edge_left_hi,edge_right_hi),depth128),14);

      // (bottom-top)*depthV  >> 14
      __m128i edge_prev = _mm_load_si128(reinterpret_cast<const __m128i *>(edgprev_ptr+x));
      __m128i edge_next = _mm_load_si128(reinterpret_cast<const __m128i *>(edgnext_ptr+x));
      __m128i edge_prev_lo = _mm_unpacklo_epi16(edge_prev,zero);
      __m128i edge_prev_hi = _mm_unpackhi_epi16(edge_prev,zero);
      __m128i edge_next_lo = _mm_unpacklo_epi16(edge_next,zero);
      __m128i edge_next_hi = _mm_unpackhi_epi16(edge_next,zero);

      __m128i vert_lo = _mm_srai_epi32(_mm_mullo_epi32(_mm_sub_epi32(edge_prev_lo,edge_next_lo),depthV128),14);
      __m128i vert_hi = _mm_srai_epi32(_mm_mullo_epi32(_mm_sub_epi32(edge_prev_hi,edge_next_hi),depthV128),14);

      // guard vertical offsets
      vert_lo = _mm_max_epi32(_mm_min_epi32(vert_lo,y_limit_max),y_limit_min);
      vert_hi = _mm_max_epi32(_mm_min_epi32(vert_hi,y_limit_max),y_limit_min);

      // 7-bit fractional part integer arithmetic

      // fractional part: for fine weighting
      const __m128i mask7F_32bit = _mm_set1_epi32(0x7F);
      __m128i horiz_lo_pre = horiz_lo;
      __m128i horiz_hi_pre = horiz_hi;
      __m128i vert_lo_pre = vert_lo;
      __m128i vert_hi_pre = vert_hi;
      if (SMAGL)
	  {
        // shift by 2; multiply by 4
        // FIX 180318 PF: slli instead of srli (when warp2).
        horiz_lo_pre = _mm_slli_epi32(horiz_lo_pre,SMAGL);
        horiz_hi_pre = _mm_slli_epi32(horiz_hi_pre,SMAGL);
        vert_lo_pre = _mm_slli_epi32(vert_lo_pre,SMAGL);
        vert_hi_pre = _mm_slli_epi32(vert_hi_pre,SMAGL);
      }
      __m128i horiz_weight = _mm_packs_epi32(_mm_and_si128(horiz_lo_pre, mask7F_32bit),_mm_and_si128(horiz_hi_pre, mask7F_32bit));
      __m128i vert_weight = _mm_packs_epi32(_mm_and_si128(vert_lo_pre, mask7F_32bit),_mm_and_si128(vert_hi_pre, mask7F_32bit));

      // integer part: the offset itself
      // FIX 180318 PF: No rounding. Same as C.
      horiz_lo = _mm_srai_epi32(horiz_lo,(7-SMAGL));
      horiz_hi = _mm_srai_epi32(horiz_hi,(7-SMAGL));
      __m128i horiz = _mm_packs_epi32(horiz_lo,horiz_hi); // signed 16 bits
      vert_lo = _mm_srai_epi32(vert_lo,7-SMAGL);
      vert_hi = _mm_srai_epi32(vert_hi,7-SMAGL);
      __m128i vert = _mm_packs_epi32(vert_lo,vert_hi);

      // horizontal things
      // add offset
      horiz = _mm_add_epi16(horiz,_mm_set1_epi16(x << SMAGL)); // still 16 bits
                                                         // guard horizontal offsets min/max
      __m128i horiz_offset = _mm_max_epi16(_mm_min_epi16(x_limit_max,horiz),x_limit_min);

      // mask out-of-screen offset weights
      horiz_weight = _mm_and_si128(horiz_weight,_mm_cmpgt_epi16(x_limit_max,horiz)); // mask out over max
      // FIX 180318 PF: change param order in andnot.
      horiz_weight = _mm_andnot_si128(_mm_cmpgt_epi16(x_limit_min,horiz),horiz_weight); // mask out below min

      // combine vertical and horizontal offset
      __m128i srcpitch_and_one = _mm_unpacklo_epi16(_mm_set1_epi16(src_pitch),_mm_set1_epi16(1));
      // const int32_t offs = (vert[i] * src_pitch) + (horiz_offset[i] * 1)     ?? + i; <- i offset later directly
      __m128i offs_lo = _mm_madd_epi16(_mm_unpacklo_epi16(vert,horiz_offset),srcpitch_and_one);
      __m128i offs_hi = _mm_madd_epi16(_mm_unpackhi_epi16(vert,horiz_offset),srcpitch_and_one);

      // read 2x8 pixels
      __m128i curr_01_lo = _mm_undefined_si128();
      __m128i curr_01_hi = _mm_undefined_si128();
      __m128i bottom_01_lo = _mm_undefined_si128();
      __m128i bottom_01_hi = _mm_undefined_si128();
      int offs;
      // lo
      // #0
      offs = _mm_cvtsi128_si32(offs_lo);
      offs_lo = _mm_srli_si128(offs_lo,4);

      // FIX 180318 PF: use *(uint32_t*)& everywhere (8x2 line)!.

      // stuff even and odd pixel pair, see later curr_0 and curr_1, bottom_0 and bottom_1
      curr_01_lo = _mm_insert_epi32(curr_01_lo,*(uint32_t *)&srcp[offs+0],0);
      bottom_01_lo = _mm_insert_epi32(bottom_01_lo,*(uint32_t *)&srcp2[offs+0],0);
      // #1
      offs = _mm_cvtsi128_si32(offs_lo);
      offs_lo = _mm_srli_si128(offs_lo,4);
      curr_01_lo = _mm_insert_epi32(curr_01_lo,*(uint32_t *)&srcp[offs+1],1);
      bottom_01_lo = _mm_insert_epi32(bottom_01_lo,*(uint32_t *)&srcp2[offs+1],1);
      // #2
      offs = _mm_cvtsi128_si32(offs_lo);
      offs_lo = _mm_srli_si128(offs_lo,4);
      curr_01_lo = _mm_insert_epi32(curr_01_lo,*(uint32_t *)&srcp[offs+2],2);
      bottom_01_lo = _mm_insert_epi32(bottom_01_lo,*(uint32_t *)&srcp2[offs+2],2);
      // #3
      offs = _mm_cvtsi128_si32(offs_lo);
      // not used anymore // offs_lo = _mm_srli_si128(offs_lo,4);
      curr_01_lo = _mm_insert_epi32(curr_01_lo,*(uint32_t *)&srcp[offs+ 3],3);
      bottom_01_lo = _mm_insert_epi32(bottom_01_lo,*(uint32_t *)&srcp2[offs+3],3);
      // high part, source offset is 4-7, target offset is 0-3 again
      // #4
      offs = _mm_cvtsi128_si32(offs_hi);
      offs_hi = _mm_srli_si128(offs_hi,4);
      curr_01_hi = _mm_insert_epi32(curr_01_hi,*(uint32_t *)&srcp[offs+4],0);
      bottom_01_hi = _mm_insert_epi32(bottom_01_hi,*(uint32_t *)&srcp2[offs+4],0);
      // #5
      offs = _mm_cvtsi128_si32(offs_hi);
      offs_hi = _mm_srli_si128(offs_hi,4);
      curr_01_hi = _mm_insert_epi32(curr_01_hi,*(uint32_t *)&srcp[offs+5],1);
      bottom_01_hi = _mm_insert_epi32(bottom_01_hi,*(uint32_t *)&srcp2[offs+5],1);
      // #6
      offs = _mm_cvtsi128_si32(offs_hi);
      offs_hi = _mm_srli_si128(offs_hi,4);
      curr_01_hi = _mm_insert_epi32(curr_01_hi,*(uint32_t *)&srcp[offs+6],2);
      bottom_01_hi = _mm_insert_epi32(bottom_01_hi,*(uint32_t *)&srcp2[offs+6],2);
      // #7
      offs = _mm_cvtsi128_si32(offs_hi);
      // not used anymore // offs_hi = _mm_srli_si128(offs_hi, 4);
      curr_01_hi = _mm_insert_epi32(curr_01_hi,*(uint32_t *)&srcp[offs+7],3);
      bottom_01_hi = _mm_insert_epi32(bottom_01_hi,*(uint32_t *)&srcp2[offs+7],3);

      // combine even and odds together
      __m128i mask0000FFFF = _mm_set1_epi32(0xFFFF);
      __m128i curr_0 = _mm_packus_epi32(_mm_and_si128(curr_01_lo,mask0000FFFF),_mm_and_si128(curr_01_hi,mask0000FFFF)); // evens
      __m128i curr_1 = _mm_packus_epi32(_mm_srli_epi32(curr_01_lo,16),_mm_srli_epi32(curr_01_hi,16)); // odds
      __m128i bottom_0 = _mm_packus_epi32(_mm_and_si128(bottom_01_lo,mask0000FFFF),_mm_and_si128(bottom_01_hi,mask0000FFFF)); // evens
      __m128i bottom_1 = _mm_packus_epi32(_mm_srli_epi32(bottom_01_lo,16),_mm_srli_epi32(bottom_01_hi,16)); // odds

      // calculate weighted averages of the four pixel groups
      // each pixel position has one vertical and one horizontal weight (0 to 127)
      __m128i one = _mm_set1_epi16(ARITH_ONE);
      __m128i rounder32bit = _mm_set1_epi32(ARITH_ROUNDER);
      __m128i shifter_to_signed = _mm_set1_epi16(-32768); // to allow signed mul for unsigned 16 bit

      // trick: convert to signed 16 bit (really this is not needed for 10-14 bits)
      if (!lessthan16bits)
      {
        curr_0 = _mm_sub_epi16(curr_0,shifter_to_signed);
        curr_1 = _mm_sub_epi16(curr_1,shifter_to_signed);
        bottom_0 = _mm_sub_epi16(bottom_0,shifter_to_signed);
        bottom_1 = _mm_sub_epi16(bottom_1,shifter_to_signed);
      }

      // curr = curr_0 * (1 - x0/128) + curr_1 * (x0/128)
      // bottom = bottom_0 * (1 - x0/128) + bottom_1 * (x0/128)
      __m128i inv_horiz_weight = _mm_sub_epi16(one,horiz_weight);
      
      __m128i curr_lo = _mm_madd_epi16(_mm_unpacklo_epi16(curr_0,curr_1),_mm_unpacklo_epi16(inv_horiz_weight,horiz_weight));
      __m128i curr_hi = _mm_madd_epi16(_mm_unpackhi_epi16(curr_0,curr_1),_mm_unpackhi_epi16(inv_horiz_weight,horiz_weight));
      curr_lo = _mm_srai_epi32(_mm_add_epi32(curr_lo,rounder32bit),ARITH_BITS);
      curr_hi = _mm_srai_epi32(_mm_add_epi32(curr_hi,rounder32bit),ARITH_BITS);
      // back to signed 16 bits
      __m128i curr = _mm_packs_epi32(curr_lo,curr_hi);

      __m128i bottom_lo = _mm_madd_epi16(_mm_unpacklo_epi16(bottom_0,bottom_1),_mm_unpacklo_epi16(inv_horiz_weight,horiz_weight));
      __m128i bottom_hi = _mm_madd_epi16(_mm_unpackhi_epi16(bottom_0,bottom_1),_mm_unpackhi_epi16(inv_horiz_weight,horiz_weight));
      bottom_lo = _mm_srai_epi32(_mm_add_epi32(bottom_lo,rounder32bit),ARITH_BITS);
      bottom_hi = _mm_srai_epi32(_mm_add_epi32(bottom_hi,rounder32bit),ARITH_BITS);
      // back to signed 16 bits
      __m128i bottom = _mm_packs_epi32(bottom_lo,bottom_hi);

      // result = curr * (1 - vert_weight/128) + bottom * (vert_weight/128)
      __m128i inv_vert_weight = _mm_sub_epi16(one,vert_weight);
      __m128i result_lo = _mm_madd_epi16(_mm_unpacklo_epi16(curr,bottom),_mm_unpacklo_epi16(inv_vert_weight,vert_weight));
      __m128i result_hi = _mm_madd_epi16(_mm_unpackhi_epi16(curr,bottom),_mm_unpackhi_epi16(inv_vert_weight,vert_weight));
      result_lo = _mm_srai_epi32(_mm_add_epi32(result_lo,rounder32bit),ARITH_BITS);
      result_hi = _mm_srai_epi32(_mm_add_epi32(result_hi,rounder32bit),ARITH_BITS);

      // combine clamp pixel min max

      // packs clamps -32768..32767 (0..65535 after going back to unsigned)
      __m128i result = _mm_packs_epi32(result_lo,result_hi);
      if (!lessthan16bits)
	  {
        // trick: convert to back to unsigned 16 bit (really this is not needed for 10-14 bits)
        result = _mm_add_epi16(result,shifter_to_signed);
      }
      if (lessthan16bits)
	  {
        // clamp to valid pixel range, not needed for exact 16 bit, since packs has done that
        result = _mm_min_epu16(result,pixel_max);
      }
      
      // for avs+ and 10-16bits pixels we can safely ignore range check, avs+ is guaranteed to have at least 32 bit alignment
      // we need here only align 16
      _mm_store_si128(reinterpret_cast<__m128i *>(dst+x),result);
      // if (x >= wmod8), store less bytes // not needed!

      // preload next 8 pixels
      if (x<wmod8)
	  { // prevent overread
        edge_left = _mm_loadu_si128(reinterpret_cast<const __m128i *>(edgeptr+x+8-1));
        edge_right = _mm_loadu_si128(reinterpret_cast<const __m128i *>(edgeptr+x+8+1));
      }
    }

    srcp += src_pitch*SMAG;
    srcp2 += src_pitch*SMAG;
    edgeptr += edge_pitch;
    dstp += dst_pitch;
  }
}


// this has to be called
// PF
static void warp0_u16(const unsigned char *srcp8,const unsigned char *edgep8,unsigned char *dstp8,const int32_t src_pitch_,
  const int32_t edge_pitch_,const int32_t dst_pitch_,const int32_t width,const int32_t height,int depth,int depthV,
  const uint8_t bits_per_sample)
{
  if (aWarpSharp_Enable_SSE41)
  {
    if (bits_per_sample<16)
      warp_u16_sse41_core<0,true>(srcp8,edgep8,dstp8,src_pitch_,edge_pitch_,dst_pitch_,width,height,depth,depthV,bits_per_sample);
    else
      warp_u16_sse41_core<0,false>(srcp8,edgep8,dstp8,src_pitch_,edge_pitch_,dst_pitch_,width,height,depth,depthV,bits_per_sample);
  }
  else
  {
    warp_c<0,uint16_t>(srcp8,edgep8,dstp8,src_pitch_,edge_pitch_,dst_pitch_,width,height,depth,depthV,bits_per_sample);
  }
}

// PF180313
static void warp2_u16(const unsigned char *srcp8,const unsigned char *edgep8,unsigned char *dstp8,const int32_t src_pitch_,
  const int32_t edge_pitch_,const int32_t dst_pitch_,const int32_t width,const int32_t height,int depth,int depthV,
  const uint8_t bits_per_sample)
{
  if (aWarpSharp_Enable_SSE41)
  {
    if (bits_per_sample<16)
      warp_u16_sse41_core<2,true>(srcp8,edgep8,dstp8,src_pitch_,edge_pitch_,dst_pitch_,width,height,depth,depthV,bits_per_sample);
    else
      warp_u16_sse41_core<2,false>(srcp8,edgep8,dstp8,src_pitch_,edge_pitch_,dst_pitch_,width,height,depth,depthV,bits_per_sample);
  }
  else
  {
    warp_c<2,uint16_t>(srcp8,edgep8,dstp8,src_pitch_,edge_pitch_,dst_pitch_,width,height,depth,depthV,bits_per_sample);
  }
}


// this has to be called
// PF
static void warp0_u16_MT(const unsigned char *srcp8,const unsigned char *edgep8,unsigned char *dstp8,const int32_t src_pitch_,
  const int32_t edge_pitch_,const int32_t dst_pitch_,const int32_t width,const int32_t height,int depth,int depthV,
  const uint8_t bits_per_sample,const int32_t ymin,const int32_t ymax)
{
  if (aWarpSharp_Enable_SSE41)
  {
    if (bits_per_sample<16)
      warp_u16_sse41_core_MT<0,true>(srcp8,edgep8,dstp8,src_pitch_,edge_pitch_,dst_pitch_,width,height,depth,depthV,bits_per_sample,
		ymin,ymax);
    else
      warp_u16_sse41_core_MT<0,false>(srcp8,edgep8,dstp8,src_pitch_,edge_pitch_,dst_pitch_,width,height,depth,depthV,bits_per_sample,
		ymin,ymax);
  }
  else
  {
    warp_c_MT<0,uint16_t>(srcp8,edgep8,dstp8,src_pitch_,edge_pitch_,dst_pitch_,width,height,depth,depthV,bits_per_sample,ymin,ymax);
  }
}


// PF180313
static void warp2_u16_MT(const unsigned char *srcp8,const unsigned char *edgep8,unsigned char *dstp8,const int32_t src_pitch_,
  const int32_t edge_pitch_,const int32_t dst_pitch_,const int32_t width,const int32_t height,int depth,int depthV,
  const uint8_t bits_per_sample,const int32_t ymin,const int32_t ymax)
{
  if (aWarpSharp_Enable_SSE41)
  {
    if (bits_per_sample<16)
      warp_u16_sse41_core_MT<2,true>(srcp8,edgep8,dstp8,src_pitch_,edge_pitch_,dst_pitch_,width,height,depth,depthV,bits_per_sample,
		ymin,ymax);
    else
      warp_u16_sse41_core_MT<2,false>(srcp8,edgep8,dstp8,src_pitch_,edge_pitch_,dst_pitch_,width,height,depth,depthV,bits_per_sample,
		ymin,ymax);
  }
  else
  {
    warp_c_MT<2,uint16_t>(srcp8,edgep8,dstp8,src_pitch_,edge_pitch_,dst_pitch_,width,height,depth,depthV,bits_per_sample,
		ymin,ymax);
  }
}


static void Warp0_8(const unsigned char *psrc,const unsigned char *pedg,unsigned char *pdst,const int32_t src_pitch,
	const int32_t edg_pitch,const int32_t dst_pitch,const int32_t dst_row_size,const int32_t dst_height,int32_t depthH,
	int32_t depthV)
{
	const int32_t i = -((dst_row_size+3)&~3);
	const int32_t c = dst_row_size + i - 1;

	psrc -= i;
	pedg -= i;
	pdst -= i;

	depthH <<= 8;
	depthV <<= 8;

	const short x_limit_min[8] = {(short)i,(short)(i-1),(short)(i-2),(short)(i-3),(short)(i-4),(short)(i-5),(short)(i-6),
		(short)(i-7)};
	const short x_limit_max[8] = {(short)c,(short)(c-1),(short)(c-2),(short)(c-3),(short)(c-4),(short)(c-5),(short)(c-6),
		(short)(c-7)};

  if (aWarpSharp_Enable_AVX)
  {
    for (int32_t y=0; y<dst_height; y++)
    {
      int32_t y_limit_min = -y * 0x80;
      int32_t y_limit_max = (dst_height - y) * 0x80 - 0x81;
      int32_t edg_pitchp = -(y ? edg_pitch : 0);
      int32_t edg_pitchn = y != dst_height - 1 ? edg_pitch : 0;

	  JPSDR_Warp0_8_AVX(psrc,pedg,pdst,src_pitch,edg_pitchp,edg_pitchn,y_limit_min,y_limit_max,
		  x_limit_min,x_limit_max,i,depthH,depthV);

      psrc += src_pitch;
      pedg += edg_pitch;
      pdst += dst_pitch;
	}
  }
  else
  {
    for (int32_t y=0; y<dst_height; y++)
    {
      int32_t y_limit_min = -y * 0x80;
      int32_t y_limit_max = (dst_height - y) * 0x80 - 0x81;
      int32_t edg_pitchp = -(y ? edg_pitch : 0);
      int32_t edg_pitchn = y != dst_height - 1 ? edg_pitch : 0;

	  JPSDR_Warp0_8_SSE2(psrc,pedg,pdst,src_pitch,edg_pitchp,edg_pitchn,y_limit_min,y_limit_max,
		  x_limit_min,x_limit_max,i,depthH,depthV);

      psrc += src_pitch;
      pedg += edg_pitch;
      pdst += dst_pitch;
	}
  }
}


static void Warp0_8_MT(const unsigned char *psrc,const unsigned char *pedg,unsigned char *pdst,const int32_t src_pitch,
	const int32_t edg_pitch,const int32_t dst_pitch,const int32_t dst_row_size,const int32_t dst_height,int32_t depthH,
	int32_t depthV,const int32_t ymin,const int32_t ymax)
{
	const int32_t i = -((dst_row_size+3)&~3);
	const int32_t c = dst_row_size + i - 1;

	psrc -= i;
	pedg -= i;
	pdst -= i;

	depthH <<= 8;
	depthV <<= 8;

	const short x_limit_min[8] = {(short)i,(short)(i-1),(short)(i-2),(short)(i-3),(short)(i-4),(short)(i-5),(short)(i-6),
		(short)(i-7)};
	const short x_limit_max[8] = {(short)c,(short)(c-1),(short)(c-2),(short)(c-3),(short)(c-4),(short)(c-5),(short)(c-6),
		(short)(c-7)};

  psrc += src_pitch*ymin;
  pedg += edg_pitch*ymin;
  pdst += dst_pitch*ymin;

  if (aWarpSharp_Enable_AVX)
  {
    for (int32_t y=ymin; y<ymax; y++)
    {
      int32_t y_limit_min = -y * 0x80;
      int32_t y_limit_max = (dst_height - y) * 0x80 - 0x81;
      int32_t edg_pitchp = -(y ? edg_pitch : 0);
      int32_t edg_pitchn = y != dst_height - 1 ? edg_pitch : 0;

	  JPSDR_Warp0_8_AVX(psrc,pedg,pdst,src_pitch,edg_pitchp,edg_pitchn,y_limit_min,y_limit_max,
		  x_limit_min,x_limit_max,i,depthH,depthV);

      psrc += src_pitch;
      pedg += edg_pitch;
      pdst += dst_pitch;
	}
  }
  else
  {
    for (int32_t y=ymin; y<ymax; y++)
    {
      int32_t y_limit_min = -y * 0x80;
      int32_t y_limit_max = (dst_height - y) * 0x80 - 0x81;
      int32_t edg_pitchp = -(y ? edg_pitch : 0);
      int32_t edg_pitchn = y != dst_height - 1 ? edg_pitch : 0;

	  JPSDR_Warp0_8_SSE2(psrc,pedg,pdst,src_pitch,edg_pitchp,edg_pitchn,y_limit_min,y_limit_max,
		  x_limit_min,x_limit_max,i,depthH,depthV);

      psrc += src_pitch;
      pedg += edg_pitch;
      pdst += dst_pitch;
	}
  }
}


static void Warp2_8(const unsigned char *psrc,const unsigned char *pedg,unsigned char *pdst,const int32_t src_pitch,
	const int32_t edg_pitch,const int32_t dst_pitch,const int32_t dst_row_size,const int32_t dst_height,int32_t depthH,
	int32_t depthV)
{
	const int32_t i = -((dst_row_size+3)&~3);
	const int32_t c = dst_row_size + i - 1;
	const int32_t src_pitch4=src_pitch<<2;

	psrc -= (i << 2);
	pedg -= i;
	pdst -= i;

	depthH <<= 8;
	depthV <<= 8;

	const short x_limit_min[8] = {(short)(i<<2),(short)((i-1)<<2),(short)((i-2)<<2),(short)((i-3)<<2),(short)((i-4)<<2),
		(short)((i-5)<<2),(short)((i-6)<<2),(short)((i-7)<<2)};
	const short x_limit_max[8] = {(short)(c<<2),(short)((c-1)<<2),(short)((c-2)<<2),(short)((c-3)<<2),(short)((c-4)<<2),
		(short)((c-5)<<2),(short)((c-6)<<2),(short)((c-7)<<2)};

  if (aWarpSharp_Enable_AVX)
  {
    for (int32_t y=0; y<dst_height; y++)
    {
      int32_t y_limit_min = -y * 0x80;
      int32_t y_limit_max = (dst_height - y) * 0x80 - 0x81;
      int32_t edg_pitchp = -(y ? edg_pitch : 0);
      int32_t edg_pitchn = y != dst_height - 1 ? edg_pitch : 0;

	  JPSDR_Warp2_8_AVX(psrc,pedg,pdst,src_pitch,edg_pitchp,edg_pitchn,y_limit_min,y_limit_max,
		  x_limit_min,x_limit_max,i,depthH,depthV);

      psrc += src_pitch4;
      pedg += edg_pitch;
      pdst += dst_pitch;
	}
  }
  else
  {
    for (int32_t y=0; y<dst_height; y++)
    {
      int32_t y_limit_min = -y * 0x80;
      int32_t y_limit_max = (dst_height - y) * 0x80 - 0x81;
      int32_t edg_pitchp = -(y ? edg_pitch : 0);
      int32_t edg_pitchn = y != dst_height - 1 ? edg_pitch : 0;

	  JPSDR_Warp2_8_SSE2(psrc,pedg,pdst,src_pitch,edg_pitchp,edg_pitchn,y_limit_min,y_limit_max,
		  x_limit_min,x_limit_max,i,depthH,depthV);

      psrc += src_pitch4;
      pedg += edg_pitch;
      pdst += dst_pitch;
	}
  }
}


static void Warp2_8_MT(const unsigned char *psrc,const unsigned char *pedg,unsigned char *pdst,const int32_t src_pitch,
	const int32_t edg_pitch,const int32_t dst_pitch,const int32_t dst_row_size,const int32_t dst_height,int32_t depthH,
	int32_t depthV,const int32_t ymin,const int32_t ymax)
{
	const int32_t i = -((dst_row_size+3)&~3);
	const int32_t c = dst_row_size + i - 1;
	const int32_t src_pitch4=src_pitch<<2;

	psrc -= (i << 2);
	pedg -= i;
	pdst -= i;

	depthH <<= 8;
	depthV <<= 8;

	const short x_limit_min[8] = {(short)(i<<2),(short)((i-1)<<2),(short)((i-2)<<2),(short)((i-3)<<2),(short)((i-4)<<2),
		(short)((i-5)<<2),(short)((i-6)<<2),(short)((i-7)<<2)};
	const short x_limit_max[8] = {(short)(c<<2),(short)((c-1)<<2),(short)((c-2)<<2),(short)((c-3)<<2),(short)((c-4)<<2),
		(short)((c-5)<<2),(short)((c-6)<<2),(short)((c-7)<<2)};

  psrc += src_pitch4*ymin;
  pedg += edg_pitch*ymin;
  pdst += dst_pitch*ymin;

  if (aWarpSharp_Enable_AVX)
  {
    for (int32_t y=ymin; y<ymax; y++)
    {
      int32_t y_limit_min = -y * 0x80;
      int32_t y_limit_max = (dst_height - y) * 0x80 - 0x81;
      int32_t edg_pitchp = -(y ? edg_pitch : 0);
      int32_t edg_pitchn = y != dst_height - 1 ? edg_pitch : 0;

	  JPSDR_Warp2_8_AVX(psrc,pedg,pdst,src_pitch,edg_pitchp,edg_pitchn,y_limit_min,y_limit_max,
		  x_limit_min,x_limit_max,i,depthH,depthV);

      psrc += src_pitch4;
      pedg += edg_pitch;
      pdst += dst_pitch;
	}
  }
  else
  {
    for (int32_t y=ymin; y<ymax; y++)
    {
      int32_t y_limit_min = -y * 0x80;
      int32_t y_limit_max = (dst_height - y) * 0x80 - 0x81;
      int32_t edg_pitchp = -(y ? edg_pitch : 0);
      int32_t edg_pitchn = y != dst_height - 1 ? edg_pitch : 0;

	  JPSDR_Warp2_8_SSE2(psrc,pedg,pdst,src_pitch,edg_pitchp,edg_pitchn,y_limit_min,y_limit_max,
		  x_limit_min,x_limit_max,i,depthH,depthV);

      psrc += src_pitch4;
      pedg += edg_pitch;
      pdst += dst_pitch;
	}
  }
}


// WxH min: 4x1, mul: 4x1
// thresh: 0..FFh
// dummy first and last result columns
static void Sobel_8(const unsigned char *psrc,unsigned char *pdst,const int32_t src_pitch, const int32_t dst_pitch,
	const int32_t src_height,const int32_t dst_row_size, int32_t thresh)
{
  const int32_t i = (dst_row_size + 3) >> 2;

  if (aWarpSharp_Enable_AVX)
  {
	for (int32_t y=0; y<src_height; y++)
    {
		JPSDR_Sobel_8_AVX(psrc,pdst,src_pitch,y,src_height,i,thresh);
		pdst[0] = pdst[1];
		pdst[dst_row_size-1] = pdst[dst_row_size-2];
		psrc += src_pitch;
		pdst += dst_pitch;
    }
  }
  else
  {
	for (int32_t y=0; y<src_height; y++)
    {
		JPSDR_Sobel_8_SSE2(psrc,pdst,src_pitch,y,src_height,i,thresh);
		pdst[0] = pdst[1];
		pdst[dst_row_size-1] = pdst[dst_row_size-2];
		psrc += src_pitch;
		pdst += dst_pitch;
    }
  }
}


static void Sobel_16(const unsigned char *psrc,unsigned char *pdst,const int32_t src_pitch, const int32_t dst_pitch,
	const int32_t src_height,int32_t dst_row_size, int32_t thresh,uint8_t bit_pixel)
{
  const int32_t i = (dst_row_size + 3) >> 2;

  dst_row_size >>= 1;
  thresh <<= (bit_pixel-8);

  if (aWarpSharp_Enable_AVX)
  {
    for (int32_t y=0; y<src_height; y++)
    {
		uint16_t *dst=(uint16_t *)pdst;

		JPSDR_Sobel_16_AVX(psrc,pdst,src_pitch,y,src_height,i,thresh);
		dst[0]=dst[1];
		dst[dst_row_size-1]=dst[dst_row_size-2];

		psrc += src_pitch;
		pdst += dst_pitch;
    }
  }
  else
  {
    for (int32_t y=0; y<src_height; y++)
    {
		uint16_t *dst=(uint16_t *)pdst;

		JPSDR_Sobel_16_SSE2(psrc,pdst,src_pitch,y,src_height,i,thresh);
		dst[0]=dst[1];
		dst[dst_row_size-1]=dst[dst_row_size-2];

		psrc += src_pitch;
		pdst += dst_pitch;
    }
  }
}


static void Sobel_8_MT(const unsigned char *psrc,unsigned char *pdst,const int32_t src_pitch, const int32_t dst_pitch,
	const int32_t src_height,const int32_t dst_row_size, int32_t thresh,const int32_t ymin,const int32_t ymax)
{
  const int32_t i = (dst_row_size + 3) >> 2;

  psrc += src_pitch*ymin;
  pdst += dst_pitch*ymin;

  if (aWarpSharp_Enable_AVX)
  {
    for (int32_t y=ymin; y<ymax; y++)
    {
		JPSDR_Sobel_8_AVX(psrc,pdst,src_pitch,y,src_height,i,thresh);
		pdst[0] = pdst[1];
		pdst[dst_row_size-1] = pdst[dst_row_size-2];
		psrc += src_pitch;
		pdst += dst_pitch;
    }
  }
  else
  {
    for (int32_t y=ymin; y<ymax; y++)
    {
		JPSDR_Sobel_8_SSE2(psrc,pdst,src_pitch,y,src_height,i,thresh);
		pdst[0] = pdst[1];
		pdst[dst_row_size-1] = pdst[dst_row_size-2];
		psrc += src_pitch;
		pdst += dst_pitch;
    }
  }
}


static void Sobel_16_MT(const unsigned char *psrc,unsigned char *pdst,const int32_t src_pitch, const int32_t dst_pitch,
	const int32_t src_height,int32_t dst_row_size, int32_t thresh,uint8_t bit_pixel,const int32_t ymin,const int32_t ymax)
{
  const int32_t i = (dst_row_size + 3) >> 2;

  dst_row_size >>= 1;
  thresh <<= (bit_pixel-8);
  psrc += src_pitch*ymin;
  pdst += dst_pitch*ymin;

  if (aWarpSharp_Enable_AVX)
  {
    for (int32_t y=ymin; y<ymax; y++)
    {
		uint16_t *dst=(uint16_t *)pdst;

		JPSDR_Sobel_16_AVX(psrc,pdst,src_pitch,y,src_height,i,thresh);
		dst[0]=dst[1];
		dst[dst_row_size-1]=dst[dst_row_size-2];

		psrc += src_pitch;
		pdst += dst_pitch;
    }
  }
  else
  {
    for (int32_t y=ymin; y<ymax; y++)
    {
		uint16_t *dst=(uint16_t *)pdst;

		JPSDR_Sobel_16_SSE2(psrc,pdst,src_pitch,y,src_height,i,thresh);
		dst[0]=dst[1];
		dst[dst_row_size-1]=dst[dst_row_size-2];

		psrc += src_pitch;
		pdst += dst_pitch;
    }
  }
}


// WxH min: 1x12, mul: 1x1 (write 16x1)
// (6+5+4+3)/16 + (2+1)*3/16 + (0)*6/16
static void BlurR6_8(unsigned char *const psrc,unsigned char *const ptmp,const int32_t src_pitch,const int32_t tmp_pitch,
	const int32_t src_height,const int32_t src_row_size,bool processH,bool processV)
{
  unsigned char *psrc2,*ptmp2;
  const int32_t src_row_size_16 = (src_row_size + 15) >> 4;

  psrc2 = psrc;
  ptmp2 = ptmp;

  // Horizontal Blur
  // WxH min: 1x1, mul: 1x1 (write 16x1)
  if (processH)
  {
	  if (aWarpSharp_Enable_AVX)
	  {
	    for (int32_t y=0; y<src_height; y++)
		{
			JPSDR_H_BlurR6_8_AVX(psrc2,ptmp2,src_row_size,dq0toF);
			psrc2 += src_pitch;
			ptmp2 += tmp_pitch;
		}
	  }
	else
	{
		const int32_t offsetw1=src_row_size-16-6,offsetw2=src_row_size-16;
		int32_t src_row_size_16w = (src_row_size-12 + 15) >> 4;
		const bool testw=(12+(src_row_size_16w << 4))>abs(src_pitch)?true:false;
		if (testw) src_row_size_16w--;

		// SSE2 version
		for (int32_t y=0; y<src_height; y++)
		{
			JPSDR_H_BlurR6a_8_SSE2(psrc2,ptmp2);
			JPSDR_H_BlurR6_8_SSE2(psrc2+6,ptmp2+6,src_row_size_16w);
			if (testw) JPSDR_H_BlurR6b_8_SSE2(psrc2+offsetw1,ptmp2+offsetw1);
			JPSDR_H_BlurR6c_8_SSE2(psrc2+offsetw2,ptmp2+offsetw2);

			psrc2 += src_pitch;
			ptmp2 += tmp_pitch;
		}
	  }
  }
  else
  {
	  for (int32_t y=0; y<src_height; y++)
	  {
		  memcpy(ptmp2,psrc2,src_row_size);
		  psrc2 += src_pitch;
		  ptmp2 += tmp_pitch;
	  }
  }

  // Vertical Blur
  // WxH min: 1x12, mul: 1x1 (write 16x1)
  psrc2 = psrc;
  ptmp2 = ptmp;

  if (processV)
  {
    int32_t y;
	const int32_t height_6=src_height-6;

	  if (aWarpSharp_Enable_AVX)
	  {
    for (y=0; y<6; y++)
    {
		JPSDR_V_BlurR6a_8_AVX(psrc2,ptmp2,tmp_pitch,src_row_size_16);
		psrc2 += src_pitch;
		ptmp2 += tmp_pitch;
    }

	ptmp2 = ptmp;
    for (; y<height_6; y++)
    {
		JPSDR_V_BlurR6b_8_AVX(psrc2,ptmp2,tmp_pitch,src_row_size_16);
		psrc2 += src_pitch;
		ptmp2 += tmp_pitch;
    }

    for (; y<src_height; y++)
    {
		JPSDR_V_BlurR6c_8_AVX(psrc2,ptmp2,tmp_pitch,src_row_size_16);
		psrc2 += src_pitch;
		ptmp2 += tmp_pitch;
    }

	  }
	  else
	  {
    // SSE2 version
    for (y=0; y<6; y++)
    {
		JPSDR_V_BlurR6a_8_SSE2(psrc2,ptmp2,tmp_pitch,src_row_size_16);
		psrc2 += src_pitch;
		ptmp2 += tmp_pitch;
    }

	ptmp2 = ptmp;
    for (; y<height_6; y++)
    {
		JPSDR_V_BlurR6b_8_SSE2(psrc2,ptmp2,tmp_pitch,src_row_size_16);
		psrc2 += src_pitch;
		ptmp2 += tmp_pitch;
    }

    for (; y<src_height; y++)
    {
		JPSDR_V_BlurR6c_8_SSE2(psrc2,ptmp2,tmp_pitch,src_row_size_16);
		psrc2 += src_pitch;
		ptmp2 += tmp_pitch;
    }

	  }
  }
  else
  {
	  for (int32_t y=0; y<src_height; y++)
	  {
		  memcpy(psrc2,ptmp2,src_row_size);
		  psrc2 += src_pitch;
		  ptmp2 += tmp_pitch;
	  }
  }

}


// WxH min: 1x12, mul: 1x1 (write 16x1)
// (6+5+4+3)/16 + (2+1)*3/16 + (0)*6/16
static void BlurR6_16(unsigned char *const psrc,unsigned char *const ptmp,const int32_t src_pitch,const int32_t tmp_pitch,
	const int32_t src_height,const int32_t src_row_size,bool processH,bool processV)
{
  const int32_t src_row_size_16 = (src_row_size + 15) >> 4;
  unsigned char *psrc2,*ptmp2;

  psrc2 = psrc;
  ptmp2 = ptmp;

  // Horizontal Blur
  // WxH min: 1x1, mul: 1x1 (write 16x1)
  if (processH)
  {
  		const int32_t offsetw1=src_row_size-16-12,offsetw2=src_row_size-16;
		int32_t src_row_size_16w = (src_row_size-24 + 15) >> 4;
		const bool testw=(24+(src_row_size_16w << 4))>abs(src_pitch)?true:false;
		if (testw) src_row_size_16w--;

	  if (aWarpSharp_Enable_AVX)
	  {
		for (int32_t y=0; y<src_height; y++)
		{
			JPSDR_H_BlurR6a_16_AVX(psrc2,ptmp2);
			JPSDR_H_BlurR6_16_AVX(psrc2+12,ptmp2+12,src_row_size_16w);
			if (testw) JPSDR_H_BlurR6b_16_AVX(psrc2+offsetw1,ptmp2+offsetw1);
			JPSDR_H_BlurR6c_16_AVX(psrc2+offsetw2,ptmp2+offsetw2);

			psrc2 += src_pitch;
			ptmp2 += tmp_pitch;
		}
	  }
	  else
	  {
		// SSE2 version
		for (int32_t y=0; y<src_height; y++)
		{
			JPSDR_H_BlurR6a_16_SSE2(psrc2,ptmp2);
			JPSDR_H_BlurR6_16_SSE2(psrc2+12,ptmp2+12,src_row_size_16w);
			if (testw) JPSDR_H_BlurR6b_16_SSE2(psrc2+offsetw1,ptmp2+offsetw1);
			JPSDR_H_BlurR6c_16_SSE2(psrc2+offsetw2,ptmp2+offsetw2);
			
			psrc2 += src_pitch;
			ptmp2 += tmp_pitch;
		}
	  }
  }
  else
  {
	  for (int32_t y=0; y<src_height; y++)
	  {
		  memcpy(ptmp2,psrc2,src_row_size);
		  psrc2 += src_pitch;
		  ptmp2 += tmp_pitch;
	  }
  }

  // Vertical Blur
  // WxH min: 1x12, mul: 1x1 (write 16x1)
  psrc2 = psrc;
  ptmp2 = ptmp;

  if (processV)
  {
    int32_t y;
	const int32_t height_6=src_height-6;

	  if (aWarpSharp_Enable_AVX)
	  {
    for (y=0; y<6; y++)
    {
		JPSDR_V_BlurR6a_16_AVX(psrc2,ptmp2,tmp_pitch,src_row_size_16);
		psrc2 += src_pitch;
		ptmp2 += tmp_pitch;
    }

	ptmp2 = ptmp;
    for (; y<height_6; y++)
    {
		JPSDR_V_BlurR6b_16_AVX(psrc2,ptmp2,tmp_pitch,src_row_size_16);
		psrc2 += src_pitch;
		ptmp2 += tmp_pitch;
    }

    for (; y<src_height; y++)
    {
		JPSDR_V_BlurR6c_16_AVX(psrc2,ptmp2,tmp_pitch,src_row_size_16);
		psrc2 += src_pitch;
		ptmp2 += tmp_pitch;
    }
	  }
	  else
	  {
    // SSE2 version
    for (y=0; y<6; y++)
    {
		JPSDR_V_BlurR6a_16_SSE2(psrc2,ptmp2,tmp_pitch,src_row_size_16);
		psrc2 += src_pitch;
		ptmp2 += tmp_pitch;
    }

	ptmp2 = ptmp;
    for (; y<height_6; y++)
    {
		JPSDR_V_BlurR6b_16_SSE2(psrc2,ptmp2,tmp_pitch,src_row_size_16);
		psrc2 += src_pitch;
		ptmp2 += tmp_pitch;
    }

    for (; y<src_height; y++)
    {
		JPSDR_V_BlurR6c_16_SSE2(psrc2,ptmp2,tmp_pitch,src_row_size_16);
		psrc2 += src_pitch;
		ptmp2 += tmp_pitch;
    }

	  }
  }
  else
  {
	  for (int32_t y=0; y<src_height; y++)
	  {
		  memcpy(psrc2,ptmp2,src_row_size);
		  psrc2 += src_pitch;
		  ptmp2 += tmp_pitch;
	  }
  }
}


static void BlurR6_8_MT_H(unsigned char *const psrc,unsigned char *const ptmp,const int32_t src_pitch,const int32_t tmp_pitch,
	const int32_t src_height,const int32_t src_row_size,bool process,const int32_t ymin,const int32_t ymax)
{
  unsigned char *psrc2,*ptmp2;

  psrc2 = psrc+ymin*src_pitch;
  ptmp2 = ptmp+ymin*tmp_pitch;

  // Horizontal Blur
  // WxH min: 1x1, mul: 1x1 (write 16x1)
  if (process)
  {
	  if (aWarpSharp_Enable_AVX)
	  {
	    for (int32_t y=ymin; y<ymax; y++)
		{
			JPSDR_H_BlurR6_8_AVX(psrc2,ptmp2,src_row_size,dq0toF);
			psrc2 += src_pitch;
			ptmp2 += tmp_pitch;
		}
	  }
	else
	{
		const int32_t offsetw1=src_row_size-16-6,offsetw2=src_row_size-16;
		int32_t src_row_size_16w = (src_row_size-12 + 15) >> 4;
		const bool testw=(12+(src_row_size_16w << 4))>abs(src_pitch)?true:false;
		if (testw) src_row_size_16w--;

		// SSE2 version
		for (int32_t y=ymin; y<ymax; y++)
		{
			JPSDR_H_BlurR6a_8_SSE2(psrc2,ptmp2);
			JPSDR_H_BlurR6_8_SSE2(psrc2+6,ptmp2+6,src_row_size_16w);
			if (testw) JPSDR_H_BlurR6b_8_SSE2(psrc2+offsetw1,ptmp2+offsetw1);
			JPSDR_H_BlurR6c_8_SSE2(psrc2+offsetw2,ptmp2+offsetw2);

			psrc2 += src_pitch;
			ptmp2 += tmp_pitch;
		}
	  }
  }
  else
  {
	  for (int32_t y=ymin; y<ymax; y++)
	  {
		  memcpy(ptmp2,psrc2,src_row_size);
		  psrc2 += src_pitch;
		  ptmp2 += tmp_pitch;
	  }
  }
}


static void BlurR6_16_MT_H(unsigned char *const psrc,unsigned char *const ptmp,const int32_t src_pitch,const int32_t tmp_pitch,
	const int32_t src_height,const int32_t src_row_size,bool process,const int32_t ymin,const int32_t ymax)
{
  unsigned char *psrc2,*ptmp2;

  psrc2 = psrc+ymin*src_pitch;
  ptmp2 = ptmp+ymin*tmp_pitch;

  // Horizontal Blur
  // WxH min: 1x1, mul: 1x1 (write 16x1)
  if (process)
  {
  		const int32_t offsetw1=src_row_size-16-12,offsetw2=src_row_size-16;
		int32_t src_row_size_16w = (src_row_size-24 + 15) >> 4;
		const bool testw=(24+(src_row_size_16w << 4))>abs(src_pitch)?true:false;
		if (testw) src_row_size_16w--;

	  if (aWarpSharp_Enable_AVX)
	  {
		for (int32_t y=ymin; y<ymax; y++)
		{
			JPSDR_H_BlurR6a_16_AVX(psrc2,ptmp2);
			JPSDR_H_BlurR6_16_AVX(psrc2+12,ptmp2+12,src_row_size_16w);
			if (testw) JPSDR_H_BlurR6b_16_AVX(psrc2+offsetw1,ptmp2+offsetw1);
			JPSDR_H_BlurR6c_16_AVX(psrc2+offsetw2,ptmp2+offsetw2);

			psrc2 += src_pitch;
			ptmp2 += tmp_pitch;
		}
	  }
	  else
	  {
		// SSE2 version
		for (int32_t y=ymin; y<ymax; y++)
		{
			JPSDR_H_BlurR6a_16_SSE2(psrc2,ptmp2);
			JPSDR_H_BlurR6_16_SSE2(psrc2+12,ptmp2+12,src_row_size_16w);
			if (testw) JPSDR_H_BlurR6b_16_SSE2(psrc2+offsetw1,ptmp2+offsetw1);
			JPSDR_H_BlurR6c_16_SSE2(psrc2+offsetw2,ptmp2+offsetw2);

			psrc2 += src_pitch;
			ptmp2 += tmp_pitch;
		}
	  }
  }
  else
  {
	  for (int32_t y=ymin; y<ymax; y++)
	  {
		  memcpy(ptmp2,psrc2,src_row_size);
		  psrc2 += src_pitch;
		  ptmp2 += tmp_pitch;
	  }
  }
}


static void BlurR6_8_MT_V(unsigned char *const psrc,unsigned char *const ptmp,const int32_t src_pitch,const int32_t tmp_pitch,
	const int32_t src_height,const int32_t src_row_size,bool process,const int32_t ymin,const int32_t ymax)
{
  const int32_t src_row_size_16 = (src_row_size + 15) >> 4;
  unsigned char *psrc2,*ptmp2;
   
  // Vertical Blur
  // WxH min: 1x12, mul: 1x1 (write 16x1)
  psrc2 = psrc+ymin*src_pitch;

  if (process)
  {
    int32_t y;
	const int32_t height_6=src_height-6;

	  if (aWarpSharp_Enable_AVX)
	  {
	if (ymin<6)
	{
		const int32_t ymax0=std::min(6,ymax);

		ptmp2 = ptmp+ymin*tmp_pitch;

	    for (y=ymin; y<ymax0; y++)
		{
			JPSDR_V_BlurR6a_8_AVX(psrc2,ptmp2,tmp_pitch,src_row_size_16);
			psrc2 += src_pitch;
			ptmp2 += tmp_pitch;
		}
	}
	else y=ymin;

	if (ymax>6)
	{
		const int32_t ymax0=std::min(height_6,ymax);

		ptmp2 = ptmp+(y-6)*tmp_pitch;

		if (y<height_6)
		{
			for (; y<ymax0; y++)
			{
				JPSDR_V_BlurR6b_8_AVX(psrc2,ptmp2,tmp_pitch,src_row_size_16);
				psrc2 += src_pitch;
				ptmp2 += tmp_pitch;
			}
		}

		if (ymax>height_6)
		{
			for (; y<ymax; y++)
			{
				JPSDR_V_BlurR6c_8_AVX(psrc2,ptmp2,tmp_pitch,src_row_size_16);
				psrc2 += src_pitch;
				ptmp2 += tmp_pitch;
			}
		}
	}
	  }
	  else
	  {
    // SSE2 version
	if (ymin<6)
	{
		const int32_t ymax0=std::min(6,ymax);

		ptmp2 = ptmp+ymin*tmp_pitch;

	    for (y=ymin; y<ymax0; y++)
		{
			JPSDR_V_BlurR6a_8_SSE2(psrc2,ptmp2,tmp_pitch,src_row_size_16);
			psrc2 += src_pitch;
			ptmp2 += tmp_pitch;
		}
	}
	else y=ymin;

	if (ymax>6)
	{
		const int32_t ymax0=std::min(height_6,ymax);

		ptmp2 = ptmp+(y-6)*tmp_pitch;

		if (y<height_6)
		{
			for (; y<ymax0; y++)
			{
				JPSDR_V_BlurR6b_8_SSE2(psrc2,ptmp2,tmp_pitch,src_row_size_16);
				psrc2 += src_pitch;
				ptmp2 += tmp_pitch;
			}
		}

		if (ymax>height_6)
		{
			for (; y<ymax; y++)
			{
				JPSDR_V_BlurR6c_8_SSE2(psrc2,ptmp2,tmp_pitch,src_row_size_16);
				psrc2 += src_pitch;
				ptmp2 += tmp_pitch;
			}
		}
	}

	  }
  }
  else
  {
	  ptmp2 = ptmp+ymin*tmp_pitch;

	  for (int32_t y=ymin; y<ymax; y++)
	  {
		  memcpy(psrc2,ptmp2,src_row_size);
		  psrc2 += src_pitch;
		  ptmp2 += tmp_pitch;
	  }
  }

}


static void BlurR6_16_MT_V(unsigned char *const psrc,unsigned char *const ptmp,const int32_t src_pitch,const int32_t tmp_pitch,
	const int32_t src_height,const int32_t src_row_size,bool process,const int32_t ymin,const int32_t ymax)
{
  const int32_t src_row_size_16 = (src_row_size + 15) >> 4;
  unsigned char *psrc2,*ptmp2;
   
  // Vertical Blur
  // WxH min: 1x12, mul: 1x1 (write 16x1)
  psrc2 = psrc+ymin*src_pitch;

  if (process)
  {
    int32_t y;
	const int32_t height_6=src_height-6;

	  if (aWarpSharp_Enable_AVX)
	  {
	if (ymin<6)
	{
		const int32_t ymax0=std::min(6,ymax);

		ptmp2 = ptmp+ymin*tmp_pitch;

	    for (y=ymin; y<ymax0; y++)
		{
			JPSDR_V_BlurR6a_16_AVX(psrc2,ptmp2,tmp_pitch,src_row_size_16);
			psrc2 += src_pitch;
			ptmp2 += tmp_pitch;
		}
	}
	else y=ymin;

	if (ymax>6)
	{
		const int32_t ymax0=std::min(height_6,ymax);

		ptmp2 = ptmp+(y-6)*tmp_pitch;

		if (y<height_6)
		{
			for (; y<ymax0; y++)
			{
				JPSDR_V_BlurR6b_16_AVX(psrc2,ptmp2,tmp_pitch,src_row_size_16);
				psrc2 += src_pitch;
				ptmp2 += tmp_pitch;
			}
		}

		if (ymax>height_6)
		{
			for (; y<ymax; y++)
			{
				JPSDR_V_BlurR6c_16_AVX(psrc2,ptmp2,tmp_pitch,src_row_size_16);
				psrc2 += src_pitch;
				ptmp2 += tmp_pitch;
			}
		}
	}
	  }
	  else
	  {
    // SSE2 version
	if (ymin<6)
	{
		const int32_t ymax0=std::min(6,ymax);

		ptmp2 = ptmp+ymin*tmp_pitch;

	    for (y=ymin; y<ymax0; y++)
		{
			JPSDR_V_BlurR6a_16_SSE2(psrc2,ptmp2,tmp_pitch,src_row_size_16);
			psrc2 += src_pitch;
			ptmp2 += tmp_pitch;
		}
	}
	else y=ymin;

	if (ymax>6)
	{
		const int32_t ymax0=std::min(height_6,ymax);

		ptmp2 = ptmp+(y-6)*tmp_pitch;

		if (y<height_6)
		{
			for (; y<ymax0; y++)
			{
				JPSDR_V_BlurR6b_16_SSE2(psrc2,ptmp2,tmp_pitch,src_row_size_16);
				psrc2 += src_pitch;
				ptmp2 += tmp_pitch;
			}
		}

		if (ymax>height_6)
		{
			for (; y<ymax; y++)
			{
				JPSDR_V_BlurR6c_16_SSE2(psrc2,ptmp2,tmp_pitch,src_row_size_16);
				psrc2 += src_pitch;
				ptmp2 += tmp_pitch;
			}
		}
	}

	  }
  }
  else
  {
	  ptmp2 = ptmp+ymin*tmp_pitch;

	  for (int32_t y=ymin; y<ymax; y++)
	  {
		  memcpy(psrc2,ptmp2,src_row_size);
		  psrc2 += src_pitch;
		  ptmp2 += tmp_pitch;
	  }
  }

}


// WxH min: 1x1, mul: 1x1 (write 16x1)
// (2)/8 + (1)*4/8 + (0)*3/8
static void BlurR2_8(unsigned char *const psrc,unsigned char *const ptmp,const int32_t src_pitch,const int32_t tmp_pitch,
	 const int32_t src_height,const int32_t src_row_size,bool processH,bool processV)
{
  const int32_t src_row_size_16 = (src_row_size+0xF) & ~0xF;
  unsigned char *psrc2,*ptmp2;

  psrc2 = psrc;
  ptmp2 = ptmp;
  // Horizontal Blur
  // WxH min: 1x1, mul: 1x1 (write 16x1)
  if (processH)
  {
	  if (aWarpSharp_Enable_AVX)
	  {
	    for (int32_t y=0; y<src_height; y++)
		{
			JPSDR_H_BlurR2_8_AVX(psrc2,ptmp2,src_row_size,dq0toF);

	      psrc2 += src_pitch;
		  ptmp2 += tmp_pitch;
		}
	  }
	else
	{
		// SSE2 version
		const int32_t offsetw=src_row_size-16-2;
		int32_t src_row_size_16w = ((src_row_size-4 + 15) >> 4) << 4;
		const bool testw=(4+src_row_size_16w)>abs(src_pitch)?true:false;
		if (testw) src_row_size_16w-=16;
		const int32_t wm1=src_row_size-1,wm2=src_row_size-2,wm3=src_row_size-3,wm4=src_row_size-4;

		for (int32_t y=0; y<src_height; y++)
		{
			JPSDR_H_BlurR2_8_SSE2(psrc2+2,ptmp2+2,src_row_size_16w);
			if (testw) JPSDR_H_BlurR2a_8_SSE2(psrc2+offsetw,ptmp2+offsetw);

			uint16_t avg,avg1,avg2;

			avg1=(psrc2[0]+psrc2[1]+1) >> 1;
			avg2=(psrc2[0]+psrc2[2]+1) >> 1;
			avg=(avg2+psrc2[0]+1) >> 1;
			avg=(avg+psrc2[0]+1) >> 1;
			avg=(avg+avg1+1) >> 1;
			ptmp2[0]=(unsigned char)avg;

			avg1=(psrc2[0]+psrc2[2]+1) >> 1;
			avg2=(psrc2[0]+psrc2[3]+1) >> 1;
			avg=(avg2+psrc2[1]+1) >> 1;
			avg=(avg+psrc2[1]+1) >> 1;
			avg=(avg+avg1+1) >> 1;
			ptmp2[1]=(unsigned char)avg;

			avg1=(psrc2[wm3]+psrc2[wm1]+1) >> 1;
			avg2=(psrc2[wm4]+psrc2[wm1]+1) >> 1;
			avg=(avg2+psrc2[wm2]+1) >> 1;
			avg=(avg+psrc2[wm2]+1) >> 1;
			avg=(avg+avg1+1) >> 1;
			ptmp2[wm2]=(unsigned char)avg;

			avg1=(psrc2[wm2]+psrc2[wm1]+1) >> 1;
			avg2=(psrc2[wm3]+psrc2[wm1]+1) >> 1;
			avg=(avg2+psrc2[wm1]+1) >> 1;
			avg=(avg+psrc2[wm1]+1) >> 1;
			avg=(avg+avg1+1) >> 1;
			ptmp2[wm1]=(unsigned char)avg;

			psrc2 += src_pitch;
			ptmp2 += tmp_pitch;
		}

	  }
  }
  else
  {
	  for (int32_t y=0; y<src_height; y++)
	  {
		  memcpy(ptmp2,psrc2,src_row_size);
		  psrc2 += src_pitch;
		  ptmp2 += tmp_pitch;
	  }
  }

  psrc2 = psrc;
  ptmp2 = ptmp;
  // Vertical Blur
  // WxH min: 1x1, mul: 1x1 (write 16x1)
  if (processV)
  {
	const int32_t height_1=src_height-1,height_2=src_height-2;

	  if (aWarpSharp_Enable_AVX)
	  {
    for (int32_t y=0; y<src_height; y++)
    {
      int32_t tmp_pitchp1 = y ? -tmp_pitch : 0;
      int32_t tmp_pitchp2 = y > 1 ? (tmp_pitchp1<<1) : tmp_pitchp1;
      int32_t tmp_pitchn1 = y < height_1 ? tmp_pitch : 0;
      int32_t tmp_pitchn2 = y < height_2 ? (tmp_pitchn1<<1) : tmp_pitchn1;

	  JPSDR_V_BlurR2_8_AVX(psrc2,ptmp2,src_row_size_16,tmp_pitchp1,tmp_pitchp2,tmp_pitchn1,tmp_pitchn2);

      psrc2 += src_pitch;
      ptmp2 += tmp_pitch;
    }
	  }
	  else
	  {
	 // SSE2 version
    for (int32_t y=0; y<src_height; y++)
    {
      int32_t tmp_pitchp1 = y ? -tmp_pitch : 0;
      int32_t tmp_pitchp2 = y > 1 ? (tmp_pitchp1<<1) : tmp_pitchp1;
      int32_t tmp_pitchn1 = y < height_1 ? tmp_pitch : 0;
      int32_t tmp_pitchn2 = y < height_2 ? (tmp_pitchn1<<1) : tmp_pitchn1;

	  JPSDR_V_BlurR2_8_SSE2(psrc2,ptmp2,src_row_size_16,tmp_pitchp1,tmp_pitchp2,tmp_pitchn1,tmp_pitchn2);

      psrc2 += src_pitch;
      ptmp2 += tmp_pitch;
    }

	  }
  }
  else
  {
	  for (int32_t y=0; y<src_height; y++)
	  {
		  memcpy(psrc2,ptmp2,src_row_size);
		  psrc2 += src_pitch;
		  ptmp2 += tmp_pitch;
	  }
  }
}


// WxH min: 1x1, mul: 1x1 (write 16x1)
// (2)/8 + (1)*4/8 + (0)*3/8
static void BlurR2_16(unsigned char *const psrc,unsigned char *const ptmp,const int32_t src_pitch,const int32_t tmp_pitch,
	 const int32_t src_height,const int32_t src_row_size,bool processH,bool processV)
{
  const int32_t src_row_size_16 = (src_row_size+0xF) & ~0xF;
  unsigned char *psrc2,*ptmp2;

  psrc2 = psrc;
  ptmp2 = ptmp;
  // Horizontal Blur
  // WxH min: 1x1, mul: 1x1 (write 16x1)
  if (processH)
  {
		const int32_t offsetw=src_row_size-16-4;
		int32_t src_row_size_16w = ((src_row_size-8 + 15) >> 4) << 4;
		const bool testw=(8+src_row_size_16w)>abs(src_pitch)?true:false;
		if (testw) src_row_size_16w-=16;
		const int32_t width=src_row_size >> 1;
		const int32_t wm1=width-1,wm2=width-2,wm3=width-3,wm4=width-4;

	  if (aWarpSharp_Enable_AVX)
	  {
		for (int32_t y=0; y<src_height; y++)
		{
			JPSDR_H_BlurR2_16_AVX(psrc2+4,ptmp2+4,src_row_size_16w);
			if (testw) JPSDR_H_BlurR2a_16_AVX(psrc2+offsetw,ptmp2+offsetw);

			const uint16_t *src=(const uint16_t *)psrc2;
			uint16_t *dst=(uint16_t *)ptmp2;

			uint32_t avg,avg1,avg2;

			avg1=(src[0]+src[1]+1) >> 1;
			avg2=(src[0]+src[2]+1) >> 1;
			avg=(avg2+src[0]+1) >> 1;
			avg=(avg+src[0]+1) >> 1;
			avg=(avg+avg1+1) >> 1;
			dst[0]=(uint16_t)avg;

			avg1=(src[0]+src[2]+1) >> 1;
			avg2=(src[0]+src[3]+1) >> 1;
			avg=(avg2+src[1]+1) >> 1;
			avg=(avg+src[1]+1) >> 1;
			avg=(avg+avg1+1) >> 1;
			dst[1]=(uint16_t)avg;

			avg1=(src[wm3]+src[wm1]+1) >> 1;
			avg2=(src[wm4]+src[wm1]+1) >> 1;
			avg=(avg2+src[wm2]+1) >> 1;
			avg=(avg+src[wm2]+1) >> 1;
			avg=(avg+avg1+1) >> 1;
			dst[wm2]=(uint16_t)avg;

			avg1=(src[wm2]+src[wm1]+1) >> 1;
			avg2=(src[wm3]+src[wm1]+1) >> 1;
			avg=(avg2+src[wm1]+1) >> 1;
			avg=(avg+src[wm1]+1) >> 1;
			avg=(avg+avg1+1) >> 1;
			dst[wm1]=(uint16_t)avg;

			psrc2 += src_pitch;
			ptmp2 += tmp_pitch;
		}
	  }
	  else
	  {
		// SSE2 version
		for (int32_t y=0; y<src_height; y++)
		{
			JPSDR_H_BlurR2_16_SSE2(psrc2+4,ptmp2+4,src_row_size_16w);
			if (testw) JPSDR_H_BlurR2a_16_SSE2(psrc2+offsetw,ptmp2+offsetw);

			const uint16_t *src=(const uint16_t *)psrc2;
			uint16_t *dst=(uint16_t *)ptmp2;

			uint32_t avg,avg1,avg2;

			avg1=(src[0]+src[1]+1) >> 1;
			avg2=(src[0]+src[2]+1) >> 1;
			avg=(avg2+src[0]+1) >> 1;
			avg=(avg+src[0]+1) >> 1;
			avg=(avg+avg1+1) >> 1;
			dst[0]=(uint16_t)avg;

			avg1=(src[0]+src[2]+1) >> 1;
			avg2=(src[0]+src[3]+1) >> 1;
			avg=(avg2+src[1]+1) >> 1;
			avg=(avg+src[1]+1) >> 1;
			avg=(avg+avg1+1) >> 1;
			dst[1]=(uint16_t)avg;

			avg1=(src[wm3]+src[wm1]+1) >> 1;
			avg2=(src[wm4]+src[wm1]+1) >> 1;
			avg=(avg2+src[wm2]+1) >> 1;
			avg=(avg+src[wm2]+1) >> 1;
			avg=(avg+avg1+1) >> 1;
			dst[wm2]=(uint16_t)avg;

			avg1=(src[wm2]+src[wm1]+1) >> 1;
			avg2=(src[wm3]+src[wm1]+1) >> 1;
			avg=(avg2+src[wm1]+1) >> 1;
			avg=(avg+src[wm1]+1) >> 1;
			avg=(avg+avg1+1) >> 1;
			dst[wm1]=(uint16_t)avg;

			psrc2 += src_pitch;
			ptmp2 += tmp_pitch;
		}
	  }
  }
  else
  {
	  for (int32_t y=0; y<src_height; y++)
	  {
		  memcpy(ptmp2,psrc2,src_row_size);
		  psrc2 += src_pitch;
		  ptmp2 += tmp_pitch;
	  }
  }

  psrc2 = psrc;
  ptmp2 = ptmp;
  // Vertical Blur
  // WxH min: 1x1, mul: 1x1 (write 16x1)
  if (processV)
  {
	const int32_t height_1=src_height-1,height_2=src_height-2;

	  if (aWarpSharp_Enable_AVX)
	  {
    for (int32_t y=0; y<src_height; y++)
    {
      int32_t tmp_pitchp1 = y ? -tmp_pitch : 0;
      int32_t tmp_pitchp2 = y > 1 ? (tmp_pitchp1<<1) : tmp_pitchp1;
      int32_t tmp_pitchn1 = y < height_1 ? tmp_pitch : 0;
      int32_t tmp_pitchn2 = y < height_2 ? (tmp_pitchn1<<1) : tmp_pitchn1;

	  JPSDR_V_BlurR2_16_AVX(psrc2,ptmp2,src_row_size_16,tmp_pitchp1,tmp_pitchp2,tmp_pitchn1,tmp_pitchn2);

      psrc2 += src_pitch;
      ptmp2 += tmp_pitch;
    }
	  }
	  else
	  {
	 // SSE2 version
    for (int32_t y=0; y<src_height; y++)
    {
      int32_t tmp_pitchp1 = y ? -tmp_pitch : 0;
      int32_t tmp_pitchp2 = y > 1 ? (tmp_pitchp1<<1) : tmp_pitchp1;
      int32_t tmp_pitchn1 = y < height_1 ? tmp_pitch : 0;
      int32_t tmp_pitchn2 = y < height_2 ? (tmp_pitchn1<<1) : tmp_pitchn1;

	  JPSDR_V_BlurR2_16_SSE2(psrc2,ptmp2,src_row_size_16,tmp_pitchp1,tmp_pitchp2,tmp_pitchn1,tmp_pitchn2);

      psrc2 += src_pitch;
      ptmp2 += tmp_pitch;
    }

	  }
  }
  else
  {
	  for (int32_t y=0; y<src_height; y++)
	  {
		  memcpy(psrc2,ptmp2,src_row_size);
		  psrc2 += src_pitch;
		  ptmp2 += tmp_pitch;
	  }
  }
}


// WxH min: 1x1, mul: 1x1 (write 16x1)
// (2)/8 + (1)*4/8 + (0)*3/8
static void BlurR2_8_MT_H(unsigned char *const psrc,unsigned char *const ptmp,const int32_t src_pitch,const int32_t tmp_pitch,
	 const int32_t src_height,const int32_t src_row_size,bool process,const int32_t ymin,const int32_t ymax)
{
  unsigned char *psrc2,*ptmp2;

  psrc2 = psrc+src_pitch*ymin;
  ptmp2 = ptmp+tmp_pitch*ymin;

  // Horizontal Blur
  // WxH min: 1x1, mul: 1x1 (write 16x1)
  if (process)
  {
	  if (aWarpSharp_Enable_AVX)
	  {
	    for (int32_t y=ymin; y<ymax; y++)
		{
			JPSDR_H_BlurR2_8_AVX(psrc2,ptmp2,src_row_size,dq0toF);
			psrc2 += src_pitch;
			ptmp2 += tmp_pitch;
		}
	  }
	else
	{
		// SSE2 version
		const int32_t offsetw=src_row_size-16-2;
		int32_t src_row_size_16w = ((src_row_size-4 + 15) >> 4) << 4;
		const bool testw=(4+src_row_size_16w)>abs(src_pitch)?true:false;
		if (testw) src_row_size_16w-=16;
		const int32_t wm1=src_row_size-1,wm2=src_row_size-2,wm3=src_row_size-3,wm4=src_row_size-4;

		for (int32_t y=ymin; y<ymax; y++)
		{
			JPSDR_H_BlurR2_8_SSE2(psrc2+2,ptmp2+2,src_row_size_16w);
			if (testw) JPSDR_H_BlurR2a_8_SSE2(psrc2+offsetw,ptmp2+offsetw);

			uint16_t avg,avg1,avg2;

			avg1=(psrc2[0]+psrc2[1]+1) >> 1;
			avg2=(psrc2[0]+psrc2[2]+1) >> 1;
			avg=(avg2+psrc2[0]+1) >> 1;
			avg=(avg+psrc2[0]+1) >> 1;
			avg=(avg+avg1+1) >> 1;
			ptmp2[0]=(unsigned char)avg;

			avg1=(psrc2[0]+psrc2[2]+1) >> 1;
			avg2=(psrc2[0]+psrc2[3]+1) >> 1;
			avg=(avg2+psrc2[1]+1) >> 1;
			avg=(avg+psrc2[1]+1) >> 1;
			avg=(avg+avg1+1) >> 1;
			ptmp2[1]=(unsigned char)avg;

			avg1=(psrc2[wm3]+psrc2[wm1]+1) >> 1;
			avg2=(psrc2[wm4]+psrc2[wm1]+1) >> 1;
			avg=(avg2+psrc2[wm2]+1) >> 1;
			avg=(avg+psrc2[wm2]+1) >> 1;
			avg=(avg+avg1+1) >> 1;
			ptmp2[wm2]=(unsigned char)avg;

			avg1=(psrc2[wm2]+psrc2[wm1]+1) >> 1;
			avg2=(psrc2[wm3]+psrc2[wm1]+1) >> 1;
			avg=(avg2+psrc2[wm1]+1) >> 1;
			avg=(avg+psrc2[wm1]+1) >> 1;
			avg=(avg+avg1+1) >> 1;
			ptmp2[wm1]=(unsigned char)avg;

			psrc2 += src_pitch;
			ptmp2 += tmp_pitch;
		}
	  }
  }
  else
  {
	  for (int32_t y=ymin; y<ymax; y++)
	  {
		  memcpy(ptmp2,psrc2,src_row_size);
		  psrc2 += src_pitch;
		  ptmp2 += tmp_pitch;
	  }
  }
}


// WxH min: 1x1, mul: 1x1 (write 16x1)
// (2)/8 + (1)*4/8 + (0)*3/8
static void BlurR2_16_MT_H(unsigned char *const psrc,unsigned char *const ptmp,const int32_t src_pitch,const int32_t tmp_pitch,
	 const int32_t src_height,const int32_t src_row_size,bool process,const int32_t ymin,const int32_t ymax)
{
  unsigned char *psrc2,*ptmp2;

  psrc2 = psrc+src_pitch*ymin;
  ptmp2 = ptmp+tmp_pitch*ymin;

  // Horizontal Blur
  // WxH min: 1x1, mul: 1x1 (write 16x1)
  if (process)
  {
		const int32_t offsetw=src_row_size-16-4;
		int32_t src_row_size_16w = ((src_row_size-8 + 15) >> 4) << 4;
		const bool testw=(8+src_row_size_16w)>abs(src_pitch)?true:false;
		if (testw) src_row_size_16w-=16;
		const int32_t width=src_row_size >> 1;
		const int32_t wm1=width-1,wm2=width-2,wm3=width-3,wm4=width-4;

	  if (aWarpSharp_Enable_AVX)
	  {
		for (int32_t y=ymin; y<ymax; y++)
		{
			JPSDR_H_BlurR2_16_AVX(psrc2+4,ptmp2+4,src_row_size_16w);
			if (testw) JPSDR_H_BlurR2a_16_AVX(psrc2+offsetw,ptmp2+offsetw);

			const uint16_t *src=(const uint16_t *)psrc2;
			uint16_t *dst=(uint16_t *)ptmp2;

			uint32_t avg,avg1,avg2;

			avg1=(src[0]+src[1]+1) >> 1;
			avg2=(src[0]+src[2]+1) >> 1;
			avg=(avg2+src[0]+1) >> 1;
			avg=(avg+src[0]+1) >> 1;
			avg=(avg+avg1+1) >> 1;
			dst[0]=(uint16_t)avg;

			avg1=(src[0]+src[2]+1) >> 1;
			avg2=(src[0]+src[3]+1) >> 1;
			avg=(avg2+src[1]+1) >> 1;
			avg=(avg+src[1]+1) >> 1;
			avg=(avg+avg1+1) >> 1;
			dst[1]=(uint16_t)avg;

			avg1=(src[wm3]+src[wm1]+1) >> 1;
			avg2=(src[wm4]+src[wm1]+1) >> 1;
			avg=(avg2+src[wm2]+1) >> 1;
			avg=(avg+src[wm2]+1) >> 1;
			avg=(avg+avg1+1) >> 1;
			dst[wm2]=(uint16_t)avg;

			avg1=(src[wm2]+src[wm1]+1) >> 1;
			avg2=(src[wm3]+src[wm1]+1) >> 1;
			avg=(avg2+src[wm1]+1) >> 1;
			avg=(avg+src[wm1]+1) >> 1;
			avg=(avg+avg1+1) >> 1;
			dst[wm1]=(uint16_t)avg;

			psrc2 += src_pitch;
			ptmp2 += tmp_pitch;
		}
	  }
	  else
	  {
		// SSE2 version
		for (int32_t y=ymin; y<ymax; y++)
		{
			JPSDR_H_BlurR2_16_SSE2(psrc2+4,ptmp2+4,src_row_size_16w);
			if (testw) JPSDR_H_BlurR2a_16_SSE2(psrc2+offsetw,ptmp2+offsetw);

			const uint16_t *src=(const uint16_t *)psrc2;
			uint16_t *dst=(uint16_t *)ptmp2;

			uint32_t avg,avg1,avg2;

			avg1=(src[0]+src[1]+1) >> 1;
			avg2=(src[0]+src[2]+1) >> 1;
			avg=(avg2+src[0]+1) >> 1;
			avg=(avg+src[0]+1) >> 1;
			avg=(avg+avg1+1) >> 1;
			dst[0]=(uint16_t)avg;

			avg1=(src[0]+src[2]+1) >> 1;
			avg2=(src[0]+src[3]+1) >> 1;
			avg=(avg2+src[1]+1) >> 1;
			avg=(avg+src[1]+1) >> 1;
			avg=(avg+avg1+1) >> 1;
			dst[1]=(uint16_t)avg;

			avg1=(src[wm3]+src[wm1]+1) >> 1;
			avg2=(src[wm4]+src[wm1]+1) >> 1;
			avg=(avg2+src[wm2]+1) >> 1;
			avg=(avg+src[wm2]+1) >> 1;
			avg=(avg+avg1+1) >> 1;
			dst[wm2]=(uint16_t)avg;

			avg1=(src[wm2]+src[wm1]+1) >> 1;
			avg2=(src[wm3]+src[wm1]+1) >> 1;
			avg=(avg2+src[wm1]+1) >> 1;
			avg=(avg+src[wm1]+1) >> 1;
			avg=(avg+avg1+1) >> 1;
			dst[wm1]=(uint16_t)avg;

			psrc2 += src_pitch;
			ptmp2 += tmp_pitch;
		}

	  }
  }
  else
  {
	  for (int32_t y=ymin; y<ymax; y++)
	  {
		  memcpy(ptmp2,psrc2,src_row_size);
		  psrc2 += src_pitch;
		  ptmp2 += tmp_pitch;
	  }
  }
}


// WxH min: 1x1, mul: 1x1 (write 16x1)
// (2)/8 + (1)*4/8 + (0)*3/8
static void BlurR2_8_MT_V(unsigned char *const psrc,unsigned char *const ptmp,const int32_t src_pitch,const int32_t tmp_pitch,
	 const int32_t src_height,const int32_t src_row_size,bool process,const int32_t ymin,const int32_t ymax)
{
  const int32_t src_row_size_16 = (src_row_size+0xF) & ~0xF;
  unsigned char *psrc2,*ptmp2;

  psrc2 = psrc+src_pitch*ymin;
  ptmp2 = ptmp+tmp_pitch*ymin;

  // Vertical Blur
  // WxH min: 1x1, mul: 1x1 (write 16x1)
  if (process)
  {
	const int32_t height_1=src_height-1,height_2=src_height-2;

	  if (aWarpSharp_Enable_AVX)
	  {
    for (int32_t y=ymin; y<ymax; y++)
    {
      int32_t tmp_pitchp1 = y ? -tmp_pitch : 0;
      int32_t tmp_pitchp2 = y > 1 ? (tmp_pitchp1<<1) : tmp_pitchp1;
      int32_t tmp_pitchn1 = y < height_1 ? tmp_pitch : 0;
      int32_t tmp_pitchn2 = y < height_2 ? (tmp_pitchn1<<1) : tmp_pitchn1;

	  JPSDR_V_BlurR2_8_AVX(psrc2,ptmp2,src_row_size_16,tmp_pitchp1,tmp_pitchp2,tmp_pitchn1,tmp_pitchn2);

      psrc2 += src_pitch;
      ptmp2 += tmp_pitch;
    }
	  }
	  else
	  {
	 // SSE2 version
    for (int32_t y=ymin; y<ymax; y++)
    {
      int32_t tmp_pitchp1 = y ? -tmp_pitch : 0;
      int32_t tmp_pitchp2 = y > 1 ? (tmp_pitchp1<<1) : tmp_pitchp1;
      int32_t tmp_pitchn1 = y < height_1 ? tmp_pitch : 0;
      int32_t tmp_pitchn2 = y < height_2 ? (tmp_pitchn1<<1) : tmp_pitchn1;

	  JPSDR_V_BlurR2_8_SSE2(psrc2,ptmp2,src_row_size_16,tmp_pitchp1,tmp_pitchp2,tmp_pitchn1,tmp_pitchn2);

      psrc2 += src_pitch;
      ptmp2 += tmp_pitch;
    }

	  }
  }
  else
  {
	  for (int32_t y=ymin; y<ymax; y++)
	  {
		  memcpy(psrc2,ptmp2,src_row_size);
		  psrc2 += src_pitch;
		  ptmp2 += tmp_pitch;
	  }
  }
}


// WxH min: 1x1, mul: 1x1 (write 16x1)
// (2)/8 + (1)*4/8 + (0)*3/8
static void BlurR2_16_MT_V(unsigned char *const psrc,unsigned char *const ptmp,const int32_t src_pitch,const int32_t tmp_pitch,
	 const int32_t src_height,const int32_t src_row_size,bool process,const int32_t ymin,const int32_t ymax)
{
  const int32_t src_row_size_16 = (src_row_size+0xF) & ~0xF;
  unsigned char *psrc2,*ptmp2;

  psrc2 = psrc+src_pitch*ymin;
  ptmp2 = ptmp+tmp_pitch*ymin;

  // Vertical Blur
  // WxH min: 1x1, mul: 1x1 (write 16x1)
  if (process)
  {
	const int32_t height_1=src_height-1,height_2=src_height-2;

	  if (aWarpSharp_Enable_AVX)
	  {
    for (int32_t y=ymin; y<ymax; y++)
    {
      int32_t tmp_pitchp1 = y ? -tmp_pitch : 0;
      int32_t tmp_pitchp2 = y > 1 ? (tmp_pitchp1<<1) : tmp_pitchp1;
      int32_t tmp_pitchn1 = y < height_1 ? tmp_pitch : 0;
      int32_t tmp_pitchn2 = y < height_2 ? (tmp_pitchn1<<1) : tmp_pitchn1;

	  JPSDR_V_BlurR2_16_AVX(psrc2,ptmp2,src_row_size_16,tmp_pitchp1,tmp_pitchp2,tmp_pitchn1,tmp_pitchn2);

      psrc2 += src_pitch;
      ptmp2 += tmp_pitch;
    }
	  }
	  else
	  {
	 // SSE2 version
    for (int32_t y=ymin; y<ymax; y++)
    {
      int32_t tmp_pitchp1 = y ? -tmp_pitch : 0;
      int32_t tmp_pitchp2 = y > 1 ? (tmp_pitchp1<<1) : tmp_pitchp1;
      int32_t tmp_pitchn1 = y < height_1 ? tmp_pitch : 0;
      int32_t tmp_pitchn2 = y < height_2 ? (tmp_pitchn1<<1) : tmp_pitchn1;

	  JPSDR_V_BlurR2_16_SSE2(psrc2,ptmp2,src_row_size_16,tmp_pitchp1,tmp_pitchp2,tmp_pitchn1,tmp_pitchn2);

      psrc2 += src_pitch;
      ptmp2 += tmp_pitch;
    }

	  }
  }
  else
  {
	  for (int32_t y=ymin; y<ymax; y++)
	  {
		  memcpy(psrc2,ptmp2,src_row_size);
		  psrc2 += src_pitch;
		  ptmp2 += tmp_pitch;
	  }
  }
}


static bool GuideChroma_8(const unsigned char *py,unsigned char *pu,const int32_t src_pitch_y,const int32_t dst_pitch_uv,
	const int32_t dst_height_uv,const int32_t dst_width_uv,const int32_t subspl_dst_h_l2,
	const int32_t subspl_dst_v_l2,bool cplace_mpeg2_flag)
{
  const int32_t width_uv_8 = -((dst_width_uv + 7) & ~7);
  const int32_t src_pitch_y2=src_pitch_y << 1;

  // 4:2:0
  if ((subspl_dst_h_l2==1) && (subspl_dst_v_l2==1))
  {
    // MPEG-2 chroma placement
    if (cplace_mpeg2_flag)
    {
      for (int32_t y=0; y<dst_height_uv; y++)
      {
        int32_t c2 = py[0]+py[src_pitch_y];

        for (int32_t x=0; x<dst_width_uv; x++)
        {
		  const int32_t x_2=x << 1;
          const int32_t c0=c2;
          const int32_t c1=py[x_2]+py[src_pitch_y+x_2];
          c2 = py[x_2+1]+py[src_pitch_y+x_2+1];
          pu[x] = static_cast <unsigned char> ((c0 + (c1<<1) + c2 + 4) >> 3);
        }
        py += src_pitch_y2;
        pu += dst_pitch_uv;
      }
    }

    // MPEG-1
    else
    {
		if (aWarpSharp_Enable_AVX)
		{
			for (int32_t y=0; y<dst_height_uv; y++)
	        {
				JPSDR_GuideChroma1_8_AVX(py,pu,src_pitch_y,width_uv_8);

				py += src_pitch_y2;
				pu += dst_pitch_uv;
			}
		}
		else
		{
			// SSE2 version
			for (int32_t y=0; y<dst_height_uv; y++)
			{
				JPSDR_GuideChroma1_8_SSE2(py,pu,src_pitch_y,width_uv_8);

				py += src_pitch_y2;
				pu += dst_pitch_uv;
			}
		}
    }	// MPEG-1

	return(true);
  }

  // 4:2:2
  else if ((subspl_dst_h_l2==1) && (subspl_dst_v_l2==0))
  {
    // MPEG-2 chroma placement
    if (cplace_mpeg2_flag)
    {
      for (int32_t y=0; y<dst_height_uv; y++)
      {
        int32_t c2 = py[0];

        for (int32_t x=0; x<dst_width_uv; x++)
        {
		  const int32_t x_2=x<<1;
          const int32_t c0=c2;
          const int32_t c1=py[x_2];

          c2 = py[x_2+1];
          pu[x] = static_cast <unsigned char> ((c0 + (c1 << 1) + c2 + 2) >> 2);
        }
        py += src_pitch_y;
        pu += dst_pitch_uv;
      }
    }

    // MPEG-1
    else
    {
		if (aWarpSharp_Enable_AVX)
		{
			for (int32_t y=0; y<dst_height_uv; y++)
			{
				JPSDR_GuideChroma2_8_AVX(py,pu,width_uv_8);

				py += src_pitch_y;
				pu += dst_pitch_uv;
			}
		}
		else
		{
			// SSE2 version
			for (int32_t y=0; y<dst_height_uv; y++)
			{
				JPSDR_GuideChroma2_8_SSE2(py,pu,width_uv_8);

				py += src_pitch_y;
				pu += dst_pitch_uv;
			}
		}
    }	// MPEG-1

	return(true);
  }

  // 4:4:4
  else if ((subspl_dst_h_l2==0) && (subspl_dst_v_l2==0))
  {
    for (int32_t y=0; y<dst_height_uv; y++)
    {
		memcpy(pu,py,dst_width_uv);
		pu+=dst_pitch_uv;
		py+=src_pitch_y;
    }

	return(true);
  }

  else return(false);
}


static bool GuideChroma_16(const unsigned char *py_,unsigned char *pu_,const int32_t src_pitch_y,const int32_t dst_pitch_uv,
	const int32_t dst_height_uv,const int32_t dst_width_uv,const int32_t subspl_dst_h_l2,
	const int32_t subspl_dst_v_l2,bool cplace_mpeg2_flag)
{
  const int32_t src_pitch_y2=src_pitch_y >> 1;
  const int32_t dst_pitch_uv2 = dst_pitch_uv >> 1;
  const int32_t dst_width_uv2 = dst_width_uv << 1;
  const uint16_t *py=(const uint16_t *)py_;
  uint16_t *pu=(uint16_t *)pu_;
  const int32_t width_uv_8 = -((dst_width_uv2 + 7) & ~7);

  // 4:2:0
  if ((subspl_dst_h_l2==1) && (subspl_dst_v_l2==1))
  {
    // MPEG-2 chroma placement
    if (cplace_mpeg2_flag)
    {
      for (int32_t y=0; y<dst_height_uv; y++)
      {
        int32_t c2 = py[0]+py[src_pitch_y2];

        for (int32_t x=0; x<dst_width_uv; x++)
        {
		  const int32_t x_2=x << 1;
          const int32_t c0=c2;
          const int32_t c1=py[x_2]+py[src_pitch_y2+x_2];
          c2 = py[x_2+1]+py[src_pitch_y2+x_2+1];
          pu[x] = static_cast <uint16_t> ((c0 + (c1<<1) + c2 + 4) >> 3);
        }
        py += src_pitch_y;
        pu += dst_pitch_uv2;
      }
    }

    // MPEG-1
    else
    {
		if (aWarpSharp_Enable_AVX)
		{
			for (int32_t y=0; y<dst_height_uv; y++)
	        {
				JPSDR_GuideChroma1_16_AVX(py,pu,src_pitch_y,width_uv_8);

				py += src_pitch_y;
				pu += dst_pitch_uv2;
			}
		}
		else
		{
	        for (int32_t y=0; y<dst_height_uv; y++)
		    {
				for (int32_t x=0; x<dst_width_uv; x++)
				{
					const int32_t x_2=x << 1;
					const int avg1=(py[x_2]+py[x_2+1]+1)>>1;
					const int avg2=(py[src_pitch_y2+x_2]+py[src_pitch_y2+x_2+1]+1)>>1;
					pu[x]= static_cast <uint16_t> (avg1 + avg2 + 1) >> 1;
				}
		      py += src_pitch_y;
			  pu += dst_pitch_uv2;
			}
		}
    }	// MPEG-1

	return(true);
  }

  // 4:2:2
  else if ((subspl_dst_h_l2==1) && (subspl_dst_v_l2==0))
  {
    // MPEG-2 chroma placement
    if (cplace_mpeg2_flag)
    {
      for (int32_t y=0; y<dst_height_uv; y++)
      {
        int32_t c2 = py[0];

        for (int32_t x=0; x<dst_width_uv; x++)
        {
		  const int32_t x_2=x<<1;
          const int32_t c0=c2;
          const int32_t c1=py[x_2];

          c2 = py[x_2+1];
          pu[x] = static_cast <uint16_t> ((c0 + (c1 << 1) + c2 + 2) >> 2);
        }
        py += src_pitch_y2;
        pu += dst_pitch_uv2;
      }
    }

    // MPEG-1
    else
    {
		if (aWarpSharp_Enable_AVX)
		{
			for (int32_t y=0; y<dst_height_uv; y++)
			{
				JPSDR_GuideChroma2_16_AVX(py,pu,width_uv_8);

				py += src_pitch_y2;
				pu += dst_pitch_uv2;
			}
		}
		else
		{
	        for (int32_t y=0; y<dst_height_uv; y++)
		    {
				for (int32_t x=0; x<dst_width_uv; x++)
				{
					const int32_t x_2=x << 1;
					pu[x] = static_cast <uint16_t> (py[x_2]+py[x_2]+1)>>1;
				}
			py += src_pitch_y2;
			pu += dst_pitch_uv2;
			}
		}
    }	// MPEG-1

	return(true);
  }

  // 4:4:4
  else if ((subspl_dst_h_l2==0) && (subspl_dst_v_l2==0))
  {
    for (int32_t y=0; y<dst_height_uv; y++)
    {
		memcpy(pu,py,dst_width_uv2);
		pu+=dst_pitch_uv2;
		py+=src_pitch_y2;
    }

	return(true);
  }

  else return(false);
}


void GuideChroma_8_MT(const unsigned char *py,unsigned char *pu,const int32_t src_pitch_y,const int32_t dst_pitch_uv,
	const int32_t dst_height_uv,const int32_t dst_width_uv,const int32_t subspl_dst_h_l2,
	const int32_t subspl_dst_v_l2,bool cplace_mpeg2_flag,const int32_t ymin,const int32_t ymax)
{
  const int32_t width_uv_8 = -((dst_width_uv + 7) & ~7);
  const int32_t src_pitch_y2=src_pitch_y << 1;
  
  // 4:2:0
  if ((subspl_dst_h_l2==1) && (subspl_dst_v_l2==1))
  {
	  py += src_pitch_y2*ymin;
	  pu += dst_pitch_uv*ymin;
    // MPEG-2 chroma placement
    if (cplace_mpeg2_flag)
    {
      for (int32_t y=ymin; y<ymax; y++)
      {
        int32_t c2 = py[0]+py[src_pitch_y];

        for (int32_t x=0; x<dst_width_uv; x++)
        {
		  const int32_t x_2=x << 1;
          const int32_t c0=c2;
          const int32_t c1=py[x_2]+py[src_pitch_y+x_2];
          c2 = py[x_2+1]+py[src_pitch_y+x_2+1];
          pu[x] = static_cast <unsigned char> ((c0 + (c1<<1) + c2 + 4) >> 3);
        }
        py += src_pitch_y2;
        pu += dst_pitch_uv;
      }
    }

    // MPEG-1
    else
    {
		if (aWarpSharp_Enable_AVX)
		{
		    for (int32_t y=ymin; y<ymax; y++)
			{
				JPSDR_GuideChroma1_8_AVX(py,pu,src_pitch_y,width_uv_8);

				py += src_pitch_y2;
				pu += dst_pitch_uv;
			}
		}
		else
		{
	        // SSE2 version
		    for (int32_t y=ymin; y<ymax; y++)
			{
				JPSDR_GuideChroma1_8_SSE2(py,pu,src_pitch_y,width_uv_8);

				py += src_pitch_y2;
				pu += dst_pitch_uv;
			}
		}
    }	// MPEG-1
  }

  // 4:2:2
  else if ((subspl_dst_h_l2==1) && (subspl_dst_v_l2==0))
  {
	  py += src_pitch_y*ymin;
	  pu += dst_pitch_uv*ymin;

    // MPEG-2 chroma placement
    if (cplace_mpeg2_flag)
    {
      for (int32_t y=ymin; y<ymax; y++)
      {
        int32_t c2 = py[0];

        for (int32_t x=0; x<dst_width_uv; x++)
        {
		  const int32_t x_2=x<<1;
          const int32_t c0=c2;
          const int32_t c1=py[x_2];

          c2 = py[x_2+1];
          pu[x] = static_cast <unsigned char> ((c0 + (c1 << 1) + c2 + 2) >> 2);
        }
        py += src_pitch_y;
        pu += dst_pitch_uv;
      }
    }

    // MPEG-1
    else
    {
		if (aWarpSharp_Enable_AVX)
		{
			for (int32_t y=ymin; y<ymax; y++)
			{
				JPSDR_GuideChroma2_8_AVX(py,pu,width_uv_8);

				py += src_pitch_y;
				pu += dst_pitch_uv;
			}
		}
		else
		{
			// SSE2 version
			for (int32_t y=ymin; y<ymax; y++)
			{
				JPSDR_GuideChroma2_8_SSE2(py,pu,width_uv_8);

				py += src_pitch_y;
				pu += dst_pitch_uv;
			}
		}
    }	// MPEG-1

  }

  // 4:4:4
  else if ((subspl_dst_h_l2==0) && (subspl_dst_v_l2==0))
  {
	  py += src_pitch_y*ymin;
	  pu += dst_pitch_uv*ymin;

    for (int32_t y=ymin; y<ymax; y++)
    {
		memcpy(pu,py,dst_width_uv);
		pu+=dst_pitch_uv;
		py+=src_pitch_y;
    }
  }

}


void GuideChroma_16_MT(const unsigned char *py_,unsigned char *pu_,const int32_t src_pitch_y,const int32_t dst_pitch_uv,
	const int32_t dst_height_uv,const int32_t dst_width_uv,const int32_t subspl_dst_h_l2,
	const int32_t subspl_dst_v_l2,bool cplace_mpeg2_flag,const int32_t ymin,const int32_t ymax)
{
  const int32_t src_pitch_y2=src_pitch_y >> 1;
  const int32_t dst_pitch_uv2 = dst_pitch_uv >> 1;
  const int32_t dst_width_uv2 = dst_width_uv << 1;
  const uint16_t *py=(const uint16_t *)py_;
  uint16_t *pu=(uint16_t *)pu_;
  const int32_t width_uv_8 = -((dst_width_uv2 + 7) & ~7);
  
  // 4:2:0
  if ((subspl_dst_h_l2==1) && (subspl_dst_v_l2==1))
  {
	  py += src_pitch_y*ymin;
	  pu += dst_pitch_uv2*ymin;
    // MPEG-2 chroma placement
    if (cplace_mpeg2_flag)
    {
      for (int32_t y=ymin; y<ymax; y++)
      {
        int32_t c2 = py[0]+py[src_pitch_y];

        for (int32_t x=0; x<dst_width_uv; x++)
        {
		  const int32_t x_2=x << 1;
          const int32_t c0=c2;
          const int32_t c1=py[x_2]+py[src_pitch_y+x_2];

          c2 = py[x_2+1]+py[src_pitch_y+x_2+1];
          pu[x] = static_cast <unsigned char> ((c0 + (c1<<1) + c2 + 4) >> 3);
        }
        py += src_pitch_y;
        pu += dst_pitch_uv2;
      }
    }

    // MPEG-1
    else
    {
		if (aWarpSharp_Enable_AVX)
		{
			for (int32_t y=ymin; y<ymax; y++)
	        {
				JPSDR_GuideChroma1_16_AVX(py,pu,src_pitch_y,width_uv_8);

				py += src_pitch_y;
				pu += dst_pitch_uv2;
			}
		}
		else
		{
			for (int32_t y=ymin; y<ymax; y++)
			{
				for (int32_t x=0; x<dst_width_uv; x++)
				{
					const int32_t x_2=x << 1;
					const int avg1=(py[x_2]+py[x_2+1]+1)>>1;
					const int avg2=(py[src_pitch_y2+x_2]+py[src_pitch_y2+x_2+1]+1)>>1;

					pu[x]= static_cast <uint16_t> (avg1 + avg2 + 1) >> 1;
				}
				py += src_pitch_y;
				pu += dst_pitch_uv2;
			}
		}
    }	// MPEG-1
  }

  // 4:2:2
  else if ((subspl_dst_h_l2==1) && (subspl_dst_v_l2==0))
  {
	  py += src_pitch_y2*ymin;
	  pu += dst_pitch_uv2*ymin;

    // MPEG-2 chroma placement
    if (cplace_mpeg2_flag)
    {
      for (int32_t y=ymin; y<ymax; y++)
      {
        int32_t c2 = py[0];

        for (int32_t x=0; x<dst_width_uv; x++)
        {
		  const int32_t x_2=x<<1;
          const int32_t c0=c2;
          const int32_t c1=py[x_2];

          c2 = py[x_2+1];
          pu[x] = static_cast <unsigned char> ((c0 + (c1 << 1) + c2 + 2) >> 2);
        }
        py += src_pitch_y2;
        pu += dst_pitch_uv2;
      }
    }

    // MPEG-1
    else
    {
		if (aWarpSharp_Enable_AVX)
		{
			for (int32_t y=ymin; y<ymax; y++)
			{
				JPSDR_GuideChroma2_16_AVX(py,pu,width_uv_8);

				py += src_pitch_y2;
				pu += dst_pitch_uv2;
			}
		}
		else
		{
			for (int32_t y=ymin; y<ymax; y++)
			{
				for (int32_t x=0; x<dst_width_uv; x++)
				{
					const int32_t x_2=x << 1;

					pu[x] = static_cast <uint16_t> (py[x_2]+py[x_2]+1)>>1;
				}
				py += src_pitch_y2;
				pu += dst_pitch_uv2;
			}
		}
    }	// MPEG-1

  }

  // 4:4:4
  else if ((subspl_dst_h_l2==0) && (subspl_dst_v_l2==0))
  {
	  py += src_pitch_y2*ymin;
	  pu += dst_pitch_uv2*ymin;

    for (int32_t y=ymin; y<ymax; y++)
    {
		memcpy(pu,py,dst_width_uv2);
		pu+=dst_pitch_uv2;
		py+=src_pitch_y2;
    }
  }

}


static bool GuideChroma_Test(const int32_t subspl_dst_h_l2,const int32_t subspl_dst_v_l2)
{
  // 4:2:0
  if ((subspl_dst_h_l2==1) && (subspl_dst_v_l2==1))
  {
	return(true);
  }

  // 4:2:2
  else if ((subspl_dst_h_l2==1) && (subspl_dst_v_l2==0))
  {
	return(true);
  }

  // 4:4:4
  else if ((subspl_dst_h_l2==0) && (subspl_dst_v_l2==0))
  {
	return(true);
  }
  else return(false);
}


static void SetPlane(PVideoFrame &dst,const int plane,const uint16_t value, const VideoInfo &dst_vi)
{
  const int dst_pitch = dst->GetPitch(plane);
  unsigned char *pdst = dst->GetWritePtr(plane);
  const int dst_row_size = dst->GetRowSize(plane);
  int height = dst->GetHeight(plane);

  if (dst_pitch == dst_row_size)
    memset(pdst, value, dst_pitch*height);
  else
    for (; height--; pdst += dst_pitch)
      memset(pdst, value, dst_row_size);
}


static void SetPlane_16(PVideoFrame &dst, int plane,const uint16_t value, const VideoInfo &dst_vi)
{
  const int dst_pitch = dst->GetPitch(plane);
  unsigned char *pdst = dst->GetWritePtr(plane);
  const int width = dst->GetRowSize(plane) >> 1;
  int height = dst->GetHeight(plane);

    for (; height--; pdst += dst_pitch)
	{
		uint16_t *p=(uint16_t *)pdst;

		for(uint16_t i=0; i<width; i++)
			*p++=value;
	}
}


static void CopyPlane(PVideoFrame &src, PVideoFrame &dst, int plane, const VideoInfo &dst_vi)
{
  const int src_pitch = src->GetPitch(plane);
  const int dst_pitch = dst->GetPitch(plane);
  const unsigned char *psrc = src->GetReadPtr(plane);
  unsigned char *pdst = dst->GetWritePtr(plane);
  const int dst_row_size = dst->GetRowSize(plane);
  int height = dst->GetHeight(plane);

  if ((dst_pitch==src_pitch) && (dst_pitch==dst_row_size))
    memcpy(pdst, psrc, dst_pitch*height);
  else
    for (; height--; psrc += src_pitch, pdst += dst_pitch)
      memcpy(pdst, psrc, dst_row_size);
}


static uint8_t CreateMTData(MT_Data_Info_WarpSharp MT_Data[],uint8_t threads_number,uint8_t max_threads,int32_t size_x,int32_t size_y,int UV_w,int UV_h)
{
	int32_t _y_min,_dh;

	if ((max_threads<=1) || (max_threads>threads_number))
	{
		MT_Data[0].top=true;
		MT_Data[0].bottom=true;
		MT_Data[0].src_Y_h_min=0;
		MT_Data[0].dst_Y_h_min=0;
		MT_Data[0].src_Y_h_max=size_y;
		MT_Data[0].dst_Y_h_max=size_y;
		MT_Data[0].src_UV_h_min=0;
		MT_Data[0].dst_UV_h_min=0;
		if (UV_h>0)
		{
			MT_Data[0].src_UV_h_max=size_y >> UV_h;
			MT_Data[0].dst_UV_h_max=size_y >> UV_h;
		}
		else
		{
			MT_Data[0].src_UV_h_max=size_y;
			MT_Data[0].dst_UV_h_max=size_y;
		}
		MT_Data[0].src_Y_w=size_x;
		MT_Data[0].dst_Y_w=size_x;
		if (UV_w>0)
		{
			MT_Data[0].src_UV_w=size_x >> UV_w;
			MT_Data[0].dst_UV_w=size_x >> UV_w;
		}
		else
		{
			MT_Data[0].src_UV_w=size_x;
			MT_Data[0].dst_UV_w=size_x;
		}
		return(1);
	}

	int32_t src_dh_Y,src_dh_UV,dst_dh_Y,dst_dh_UV;
	int32_t h_y;
	uint8_t i,max=1;

	dst_dh_Y=(size_y+(uint32_t)max_threads-1)/(uint32_t)max_threads;
	if (dst_dh_Y<16) dst_dh_Y=16;
	if ((dst_dh_Y & 3)!=0) dst_dh_Y=((dst_dh_Y+3) >> 2) << 2;

	src_dh_Y=dst_dh_Y;

	_y_min=size_y;
	_dh=dst_dh_Y;

	h_y=_dh;
	while (h_y<(_y_min-16))
	{
		max++;
		h_y+=_dh;
	}

	if (max==1)
	{
		MT_Data[0].top=true;
		MT_Data[0].bottom=true;
		MT_Data[0].src_Y_h_min=0;
		MT_Data[0].dst_Y_h_min=0;
		MT_Data[0].src_Y_h_max=size_y;
		MT_Data[0].dst_Y_h_max=size_y;
		MT_Data[0].src_UV_h_min=0;
		MT_Data[0].dst_UV_h_min=0;
		if (UV_h>0)
		{
			MT_Data[0].src_UV_h_max=size_y >> UV_h;
			MT_Data[0].dst_UV_h_max=size_y >> UV_h;
		}
		else
		{
			MT_Data[0].src_UV_h_max=size_y;
			MT_Data[0].dst_UV_h_max=size_y;
		}
		MT_Data[0].src_Y_w=size_x;
		MT_Data[0].dst_Y_w=size_x;
		if (UV_w>0)
		{
			MT_Data[0].src_UV_w=size_x >> UV_w;
			MT_Data[0].dst_UV_w=size_x >> UV_w;
		}
		else
		{
			MT_Data[0].src_UV_w=size_x;
			MT_Data[0].dst_UV_w=size_x;
		}
		return(1);
	}

	src_dh_UV= (UV_h>0) ? src_dh_Y>>UV_h : src_dh_Y;
	dst_dh_UV= (UV_h>0) ? dst_dh_Y>>UV_h : dst_dh_Y;

	MT_Data[0].top=true;
	MT_Data[0].bottom=false;
	MT_Data[0].src_Y_h_min=0;
	MT_Data[0].src_Y_h_max=src_dh_Y;
	MT_Data[0].dst_Y_h_min=0;
	MT_Data[0].dst_Y_h_max=dst_dh_Y;
	MT_Data[0].src_UV_h_min=0;
	MT_Data[0].src_UV_h_max=src_dh_UV;
	MT_Data[0].dst_UV_h_min=0;
	MT_Data[0].dst_UV_h_max=dst_dh_UV;

	i=1;
	while (i<max)
	{
		MT_Data[i].top=false;
		MT_Data[i].bottom=false;
		MT_Data[i].src_Y_h_min=MT_Data[i-1].src_Y_h_max;
		MT_Data[i].src_Y_h_max=MT_Data[i].src_Y_h_min+src_dh_Y;
		MT_Data[i].dst_Y_h_min=MT_Data[i-1].dst_Y_h_max;
		MT_Data[i].dst_Y_h_max=MT_Data[i].dst_Y_h_min+dst_dh_Y;
		MT_Data[i].src_UV_h_min=MT_Data[i-1].src_UV_h_max;
		MT_Data[i].src_UV_h_max=MT_Data[i].src_UV_h_min+src_dh_UV;
		MT_Data[i].dst_UV_h_min=MT_Data[i-1].dst_UV_h_max;
		MT_Data[i].dst_UV_h_max=MT_Data[i].dst_UV_h_min+dst_dh_UV;
		i++;
	}

	MT_Data[max-1].bottom=true;
	MT_Data[max-1].src_Y_h_max=size_y;
	MT_Data[max-1].dst_Y_h_max=size_y;
	if (UV_h>0)
	{
		MT_Data[max-1].src_UV_h_max=size_y >> UV_h;
		MT_Data[max-1].dst_UV_h_max=size_y >> UV_h;
	}
	else
	{
		MT_Data[max-1].src_UV_h_max=size_y;
		MT_Data[max-1].dst_UV_h_max=size_y;
	}

	for (i=0; i<max; i++)
	{
		MT_Data[i].src_Y_w=size_x;
		MT_Data[i].dst_Y_w=size_x;
		if (UV_w>0)
		{
			MT_Data[i].src_UV_w=size_x >> UV_w;
			MT_Data[i].dst_UV_w=size_x >> UV_w;
		}
		else
		{
			MT_Data[i].src_UV_w=size_x;
			MT_Data[i].dst_UV_w=size_x;
		}
	}

	return(max);
}


aWarpSharp::aWarpSharp(PClip _child, int _thresh, int _blur_level, int _blur_type, int _depth, int _chroma, int _depthC,
	bool _cplace_mpeg2_flag, int _blur_levelV, int _depthV, int _depthVC, int _blur_levelC, int _blur_levelVC,
	int _threshC,uint8_t _threads,bool _sleep,bool _avsp,IScriptEnvironment *env) :
    GenericVideoFilter(_child), thresh(_thresh), blur_level(_blur_level), blur_type(_blur_type), depth(_depth),
		chroma(_chroma), depthC(_depthC), cplace_mpeg2_flag(_cplace_mpeg2_flag), blur_levelV(_blur_levelV),
		depthV(_depthV),depthVC(_depthVC),blur_levelC(_blur_levelC),blur_levelVC(_blur_levelVC),threshC(_threshC),
		sleep(_sleep),threads(_threads),avsp(_avsp)
{
	grey = vi.IsY();
	isRGBPfamily = vi.IsPlanarRGB() || vi.IsPlanarRGBA();
	isAlphaChannel = vi.IsYUVA() || vi.IsPlanarRGBA();

	pixelsize = (uint8_t)vi.ComponentSize(); // AVS16
	bits_per_pixel = (uint8_t)vi.BitsPerComponent();

	UserId=0;

	if (grey) chroma = 1;

	if (pixelsize>2)
	{
		if (threads>1) poolInterface->DeAllocateAllThreads(true);
		env->ThrowError("aWarpSharp: Only 8 -> 16 bits supported");
	}
	if (!(vi.IsYUV() && vi.IsPlanar()))
	{
		if (threads>1) poolInterface->DeAllocateAllThreads(true);
		env->ThrowError("aWarpSharp: Planar YUV input is required");
	}
	if ((thresh<0) || (thresh>255))
	{
		if (threads>1) poolInterface->DeAllocateAllThreads(true);
		env->ThrowError("aWarpSharp: 'thresh' must be 0..255");
	}
	if ((threshC<0) || (threshC>255))
	{
		if (threads>1) poolInterface->DeAllocateAllThreads(true);
		env->ThrowError("aWarpSharp: 'threshC' must be 0..255");
	}
	if (blur_level<0)
	{
		if (threads>1) poolInterface->DeAllocateAllThreads(true);
		env->ThrowError("aWarpSharp: 'blur' must be >=0");
	}
	if (blur_levelV<0)
	{
		if (threads>1) poolInterface->DeAllocateAllThreads(true);
		env->ThrowError("aWarpSharp: 'blurV' must be >=0");
	}
	if (blur_levelC<0)
	{
		if (threads>1) poolInterface->DeAllocateAllThreads(true);
		env->ThrowError("aWarpSharp: 'blurC' must be >=0");
	}
	if (blur_levelVC<0)
	{
		if (threads>1) poolInterface->DeAllocateAllThreads(true);
		env->ThrowError("aWarpSharp: 'blurVC' must be >=0");
	}
	if ((blur_type<0) || (blur_type>1))
	{
		if (threads>1) poolInterface->DeAllocateAllThreads(true);
		env->ThrowError("aWarpSharp: 'type' must be 0,1");
	}
	if ((depth<-128) || (depth>127))
	{
		if (threads>1) poolInterface->DeAllocateAllThreads(true);
		env->ThrowError("aWarpSharp: 'depth' must be -128..127");
	}
	if ((depthC<-128) || (depthC>127))
	{
		if (threads>1) poolInterface->DeAllocateAllThreads(true);
		env->ThrowError("aWarpSharp: 'depthC' must be -128..127");
	}
	if ((chroma<0) || (chroma>6))
	{
		if (threads>1) poolInterface->DeAllocateAllThreads(true);
		env->ThrowError("aWarpSharp: 'chroma' must be 0..6");
	}
	if ((depthV<-128) || (depthV>127))
	{
		if (threads>1) poolInterface->DeAllocateAllThreads(true);
		env->ThrowError("aWarpSharp: 'depthV' must be -128..127");
	}
	if ((depthVC<-128) || (depthVC>127))
	{
		if (threads>1) poolInterface->DeAllocateAllThreads(true);
		env->ThrowError("aWarpSharp: 'depthVC' must be -128..127");
	}

	StaticThreadpoolF=StaticThreadpool;

	for (uint8_t i=0; i<MAX_MT_THREADS; i++)
	{
		MT_Thread[i].pClass=this;
		MT_Thread[i].f_process=0;
		MT_Thread[i].thread_Id=(uint8_t)i;
		MT_Thread[i].pFunc=StaticThreadpoolF;
	}

	const int shift_w = (!grey && vi.IsPlanar() && !isRGBPfamily) ? vi.GetPlaneWidthSubsampling(PLANAR_U) : 0;
	const int shift_h = (!grey && vi.IsPlanar() && !isRGBPfamily) ? vi.GetPlaneHeightSubsampling(PLANAR_U) : 0;

	if (vi.height<32) threads_number = 1;
	else threads_number = threads;

	threads_number=CreateMTData(MT_Data,threads,threads_number,vi.width,vi.height,shift_w,shift_h);

	if (threads_number>1)
	{
		if (!poolInterface->GetUserId(UserId))
		{
			poolInterface->DeAllocateAllThreads(true);
			env->ThrowError("aWarpSharp: Error with the TheadPool while getting UserId!");
		}
	}

	has_at_least_v8=true;
	try { env->CheckVersion(8); } catch (const AvisynthError&) { has_at_least_v8=false; }
}



aWarpSharp::~aWarpSharp()
{
	if (threads_number>1) poolInterface->RemoveUserId(UserId);
	if (threads>1) poolInterface->DeAllocateAllThreads(true);
}


int __stdcall aWarpSharp::SetCacheHints(int cachehints,int frame_range)
{
  switch (cachehints)
  {
  case CACHE_GET_MTMODE :
    return MT_NICE_FILTER;
  default :
    return 0;
  }
}



void aWarpSharp::StaticThreadpool(void *ptr)
{
	Public_MT_Data_Thread *data=(Public_MT_Data_Thread *)ptr;
	aWarpSharp *ptrClass=(aWarpSharp *)data->pClass;

	MT_Data_Info_WarpSharp *mt_data_inf=((MT_Data_Info_WarpSharp *)data->pData)+data->thread_Id;
	
	switch(data->f_process)
	{
		case 1 :
			Sobel_8_MT((const unsigned char *)mt_data_inf->src_Y1,(unsigned char *)mt_data_inf->dst_Y1,
				mt_data_inf->src_pitch_Y1,mt_data_inf->src_pitch_Y2,mt_data_inf->src_Y_h,
				mt_data_inf->row_size_Y2,ptrClass->thresh,mt_data_inf->src_Y_h_min,mt_data_inf->src_Y_h_max);
			break;
		case 2 :
			BlurR2_8_MT_H((unsigned char *const)mt_data_inf->dst_Y1,(unsigned char *const)mt_data_inf->dst_Y2,
				mt_data_inf->src_pitch_Y2,mt_data_inf->dst_pitch_Y2,mt_data_inf->dst_Y_h,
				mt_data_inf->row_size_Y2,true,mt_data_inf->dst_Y_h_min,mt_data_inf->dst_Y_h_max);
			break;
		case 3 :
			BlurR2_8_MT_V((unsigned char *const)mt_data_inf->dst_Y1,(unsigned char *const)mt_data_inf->dst_Y2,
				mt_data_inf->src_pitch_Y2,mt_data_inf->dst_pitch_Y2,mt_data_inf->dst_Y_h,
				mt_data_inf->row_size_Y2,true,mt_data_inf->dst_Y_h_min,mt_data_inf->dst_Y_h_max);
			break;
		case 4 :
			BlurR6_8_MT_H((unsigned char *const)mt_data_inf->dst_Y1,(unsigned char *const)mt_data_inf->dst_Y2,
				mt_data_inf->src_pitch_Y2,mt_data_inf->dst_pitch_Y2,mt_data_inf->dst_Y_h,
				mt_data_inf->row_size_Y2,true,mt_data_inf->dst_Y_h_min,mt_data_inf->dst_Y_h_max);
			break;
		case 5 :
			BlurR6_8_MT_V((unsigned char *const)mt_data_inf->dst_Y1,(unsigned char *const)mt_data_inf->dst_Y2,
				mt_data_inf->src_pitch_Y2,mt_data_inf->dst_pitch_Y2,mt_data_inf->dst_Y_h,
				mt_data_inf->row_size_Y2,true,mt_data_inf->dst_Y_h_min,mt_data_inf->dst_Y_h_max);
			break;
		case 6 :
			BlurR2_8_MT_H((unsigned char *const)mt_data_inf->dst_Y1,(unsigned char *const)mt_data_inf->dst_Y2,
				mt_data_inf->src_pitch_Y2,mt_data_inf->dst_pitch_Y2,mt_data_inf->dst_Y_h,
				mt_data_inf->row_size_Y2,mt_data_inf->processH,mt_data_inf->dst_Y_h_min,mt_data_inf->dst_Y_h_max);
			break;
		case 7 :
			BlurR2_8_MT_V((unsigned char *const)mt_data_inf->dst_Y1,(unsigned char *const)mt_data_inf->dst_Y2,
				mt_data_inf->src_pitch_Y2,mt_data_inf->dst_pitch_Y2,mt_data_inf->dst_Y_h,
				mt_data_inf->row_size_Y2,mt_data_inf->processV,mt_data_inf->dst_Y_h_min,mt_data_inf->dst_Y_h_max);
			break;
		case 8 :
			BlurR6_8_MT_H((unsigned char *const)mt_data_inf->dst_Y1,(unsigned char *const)mt_data_inf->dst_Y2,
				mt_data_inf->src_pitch_Y2,mt_data_inf->dst_pitch_Y2,mt_data_inf->dst_Y_h,
				mt_data_inf->row_size_Y2,mt_data_inf->processH,mt_data_inf->dst_Y_h_min,mt_data_inf->dst_Y_h_max);
			break;
		case 9 :
			BlurR6_8_MT_V((unsigned char *const)mt_data_inf->dst_Y1,(unsigned char *const)mt_data_inf->dst_Y2,
				mt_data_inf->src_pitch_Y2,mt_data_inf->dst_pitch_Y2,mt_data_inf->dst_Y_h,
				mt_data_inf->row_size_Y2,mt_data_inf->processV,mt_data_inf->dst_Y_h_min,mt_data_inf->dst_Y_h_max);
			break;
		case 10 :
			Warp0_8_MT((const unsigned char *)mt_data_inf->src_Y1,(const unsigned char *)mt_data_inf->src_Y2,
				(unsigned char *)mt_data_inf->dst_Y2,
				mt_data_inf->src_pitch_Y1,mt_data_inf->src_pitch_Y2,mt_data_inf->dst_pitch_Y2,
				mt_data_inf->row_size_Y3,mt_data_inf->dst_Y_h,ptrClass->depth,ptrClass->depthV,
				mt_data_inf->dst_Y_h_min,mt_data_inf->dst_Y_h_max);
			break;
		case 11 :
			Sobel_8_MT((const unsigned char *)mt_data_inf->src_U1,(unsigned char *)mt_data_inf->dst_U1,
				mt_data_inf->src_pitch_U1,mt_data_inf->src_pitch_U2,mt_data_inf->src_U_h,
				mt_data_inf->row_size_U1,ptrClass->threshC,mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 12 :
			BlurR2_8_MT_H((unsigned char *const)mt_data_inf->dst_U1,(unsigned char *const)mt_data_inf->dst_U2,
				mt_data_inf->src_pitch_U2,mt_data_inf->dst_pitch_U2,mt_data_inf->dst_U_h,
				mt_data_inf->row_size_U1,true,mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 13 :
			BlurR2_8_MT_V((unsigned char *const)mt_data_inf->dst_U1,(unsigned char *const)mt_data_inf->dst_U2,
				mt_data_inf->src_pitch_U2,mt_data_inf->dst_pitch_U2,mt_data_inf->dst_U_h,
				mt_data_inf->row_size_U1,true,mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 14 :
			BlurR6_8_MT_H((unsigned char *const)mt_data_inf->dst_U1,(unsigned char *const)mt_data_inf->dst_U2,
				mt_data_inf->src_pitch_U2,mt_data_inf->dst_pitch_U2,mt_data_inf->dst_U_h,
				mt_data_inf->row_size_U1,true,mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 15 :
			BlurR6_8_MT_V((unsigned char *const)mt_data_inf->dst_U1,(unsigned char *const)mt_data_inf->dst_U2,
				mt_data_inf->src_pitch_U2,mt_data_inf->dst_pitch_U2,mt_data_inf->dst_U_h,
				mt_data_inf->row_size_U1,true,mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 16 :
			BlurR2_8_MT_H((unsigned char *const)mt_data_inf->dst_U1,(unsigned char *const)mt_data_inf->dst_U2,
				mt_data_inf->src_pitch_U2,mt_data_inf->dst_pitch_U2,mt_data_inf->dst_U_h,
				mt_data_inf->row_size_U1,mt_data_inf->cprocessH,
				mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 17 :
			BlurR2_8_MT_V((unsigned char *const)mt_data_inf->dst_U1,(unsigned char *const)mt_data_inf->dst_U2,
				mt_data_inf->src_pitch_U2,mt_data_inf->dst_pitch_U2,mt_data_inf->dst_U_h,
				mt_data_inf->row_size_U1,mt_data_inf->cprocessV,
				mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 18 :
			BlurR6_8_MT_H((unsigned char *const)mt_data_inf->dst_U1,(unsigned char *const)mt_data_inf->dst_U2,
				mt_data_inf->src_pitch_U2,mt_data_inf->dst_pitch_U2,mt_data_inf->dst_U_h,
				mt_data_inf->row_size_U1,mt_data_inf->cprocessH,
				mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 19 :
			BlurR6_8_MT_V((unsigned char *const)mt_data_inf->dst_U1,(unsigned char *const)mt_data_inf->dst_U2,
				mt_data_inf->src_pitch_U2,mt_data_inf->dst_pitch_U2,mt_data_inf->dst_U_h,
				mt_data_inf->row_size_U1,mt_data_inf->cprocessV,
				mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 20 :
			Warp0_8_MT((const unsigned char *)mt_data_inf->src_U1,(const unsigned char *)mt_data_inf->src_U2,
				(unsigned char *)mt_data_inf->dst_U2,
				mt_data_inf->src_pitch_U1,mt_data_inf->src_pitch_U2,mt_data_inf->dst_pitch_U2,
				mt_data_inf->row_size_U2,mt_data_inf->dst_U_h,ptrClass->depthC,ptrClass->depthVC,
				mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 21 :
			Sobel_8_MT((const unsigned char *)mt_data_inf->src_V1,(unsigned char *)mt_data_inf->dst_V1,
				mt_data_inf->src_pitch_V1,mt_data_inf->src_pitch_V2,mt_data_inf->src_V_h,
				mt_data_inf->row_size_V1,ptrClass->threshC,mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 22 :
			BlurR2_8_MT_H((unsigned char *const)mt_data_inf->dst_V1,(unsigned char *const)mt_data_inf->dst_V2,
				mt_data_inf->src_pitch_V2,mt_data_inf->dst_pitch_V2,mt_data_inf->dst_V_h,
				mt_data_inf->row_size_V1,true,mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 23 :
			BlurR2_8_MT_V((unsigned char *const)mt_data_inf->dst_V1,(unsigned char *const)mt_data_inf->dst_V2,
				mt_data_inf->src_pitch_V2,mt_data_inf->dst_pitch_V2,mt_data_inf->dst_V_h,
				mt_data_inf->row_size_V1,true,mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 24 :
			BlurR6_8_MT_H((unsigned char *const)mt_data_inf->dst_V1,(unsigned char *const)mt_data_inf->dst_V2,
				mt_data_inf->src_pitch_V2,mt_data_inf->dst_pitch_V2,mt_data_inf->dst_V_h,
				mt_data_inf->row_size_V1,true,mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 25 :
			BlurR6_8_MT_V((unsigned char *const)mt_data_inf->dst_V1,(unsigned char *const)mt_data_inf->dst_V2,
				mt_data_inf->src_pitch_V2,mt_data_inf->dst_pitch_V2,mt_data_inf->dst_V_h,
				mt_data_inf->row_size_V1,true,mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 26 :
			BlurR2_8_MT_H((unsigned char *const)mt_data_inf->dst_V1,(unsigned char *const)mt_data_inf->dst_V2,
				mt_data_inf->src_pitch_V2,mt_data_inf->dst_pitch_V2,mt_data_inf->dst_V_h,
				mt_data_inf->row_size_V1,mt_data_inf->cprocessH,
				mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 27 :
			BlurR2_8_MT_V((unsigned char *const)mt_data_inf->dst_V1,(unsigned char *const)mt_data_inf->dst_V2,
				mt_data_inf->src_pitch_V2,mt_data_inf->dst_pitch_V2,mt_data_inf->dst_V_h,
				mt_data_inf->row_size_V1,mt_data_inf->cprocessV,
				mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 28 :
			BlurR6_8_MT_H((unsigned char *const)mt_data_inf->dst_V1,(unsigned char *const)mt_data_inf->dst_V2,
				mt_data_inf->src_pitch_V2,mt_data_inf->dst_pitch_V2,mt_data_inf->dst_V_h,
				mt_data_inf->row_size_V1,mt_data_inf->cprocessH,
				mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 29 :
			BlurR6_8_MT_V((unsigned char *const)mt_data_inf->dst_V1,(unsigned char *const)mt_data_inf->dst_V2,
				mt_data_inf->src_pitch_V2,mt_data_inf->dst_pitch_V2,mt_data_inf->dst_V_h,
				mt_data_inf->row_size_V1,mt_data_inf->cprocessV,
				mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 30 :
			Warp0_8_MT((const unsigned char *)mt_data_inf->src_V1,(const unsigned char *)mt_data_inf->src_V2,
				(unsigned char *)mt_data_inf->dst_V2,
				mt_data_inf->src_pitch_V1,mt_data_inf->src_pitch_V2,mt_data_inf->dst_pitch_V2,
				mt_data_inf->row_size_V2,mt_data_inf->dst_V_h,ptrClass->depthC,ptrClass->depthVC,
				mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 31 :
			GuideChroma_8_MT((const unsigned char *)mt_data_inf->src_Y2,(unsigned char *)mt_data_inf->dst_U1,
				mt_data_inf->src_pitch_Y2,mt_data_inf->src_pitch_U2,mt_data_inf->dst_U_h,
				mt_data_inf->row_size_U1,mt_data_inf->SubW_U,mt_data_inf->SubH_U,ptrClass->cplace_mpeg2_flag,
				mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 32 :
			Warp0_8_MT((const unsigned char *)mt_data_inf->src_U1,(const unsigned char *)mt_data_inf->src_U2,
				(unsigned char *)mt_data_inf->dst_U2,
				mt_data_inf->src_pitch_U1,mt_data_inf->src_pitch_U2,mt_data_inf->dst_pitch_U2,
				mt_data_inf->row_size_U2,mt_data_inf->dst_U_h,ptrClass->depthC,ptrClass->depthVC,
				mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 33 :
			Warp0_8_MT((const unsigned char *)mt_data_inf->src_V1,(const unsigned char *)mt_data_inf->src_U2,
				(unsigned char *)mt_data_inf->dst_V2,
				mt_data_inf->src_pitch_V1,mt_data_inf->src_pitch_U2,mt_data_inf->dst_pitch_V2,
				mt_data_inf->row_size_V2,mt_data_inf->dst_V_h,ptrClass->depthC,ptrClass->depthVC,
				mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 34 :
			Warp0_8_MT((const unsigned char *)mt_data_inf->src_U1,(const unsigned char *)mt_data_inf->src_Y2,
				(unsigned char *)mt_data_inf->dst_U2,
				mt_data_inf->src_pitch_U1,mt_data_inf->src_pitch_Y2,mt_data_inf->dst_pitch_U2,
				mt_data_inf->row_size_U2,mt_data_inf->dst_U_h,ptrClass->depthC,ptrClass->depthVC,
				mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 35 :
			Warp0_8_MT((const unsigned char *)mt_data_inf->src_V1,(const unsigned char *)mt_data_inf->src_Y2,
				(unsigned char *)mt_data_inf->dst_V2,
				mt_data_inf->src_pitch_V1,mt_data_inf->src_pitch_Y2,mt_data_inf->dst_pitch_V2,
				mt_data_inf->row_size_V2,mt_data_inf->dst_V_h,ptrClass->depthC,ptrClass->depthVC,
				mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
			// 16 bits
		case 36 :
			Sobel_16_MT((const unsigned char *)mt_data_inf->src_Y1,(unsigned char *)mt_data_inf->dst_Y1,
				mt_data_inf->src_pitch_Y1,mt_data_inf->src_pitch_Y2,mt_data_inf->src_Y_h,mt_data_inf->row_size_Y2,
				ptrClass->thresh,ptrClass->bits_per_pixel,mt_data_inf->src_Y_h_min,mt_data_inf->src_Y_h_max);
			break;
		case 37 :
			BlurR2_16_MT_H((unsigned char *const)mt_data_inf->dst_Y1,(unsigned char *const)mt_data_inf->dst_Y2,
				mt_data_inf->src_pitch_Y2,mt_data_inf->dst_pitch_Y2,mt_data_inf->dst_Y_h,
				mt_data_inf->row_size_Y2,true,mt_data_inf->dst_Y_h_min,mt_data_inf->dst_Y_h_max);
			break;
		case 38 :
			BlurR2_16_MT_V((unsigned char *const)mt_data_inf->dst_Y1,(unsigned char *const)mt_data_inf->dst_Y2,
				mt_data_inf->src_pitch_Y2,mt_data_inf->dst_pitch_Y2,mt_data_inf->dst_Y_h,
				mt_data_inf->row_size_Y2,true,mt_data_inf->dst_Y_h_min,mt_data_inf->dst_Y_h_max);
			break;
		case 39 :
			BlurR6_16_MT_H((unsigned char *const)mt_data_inf->dst_Y1,(unsigned char *const)mt_data_inf->dst_Y2,
				mt_data_inf->src_pitch_Y2,mt_data_inf->dst_pitch_Y2,mt_data_inf->dst_Y_h,
				mt_data_inf->row_size_Y2,true,mt_data_inf->dst_Y_h_min,mt_data_inf->dst_Y_h_max);
			break;
		case 40 :
			BlurR6_16_MT_V((unsigned char *const)mt_data_inf->dst_Y1,(unsigned char *const)mt_data_inf->dst_Y2,
				mt_data_inf->src_pitch_Y2,mt_data_inf->dst_pitch_Y2,mt_data_inf->dst_Y_h,
				mt_data_inf->row_size_Y2,true,mt_data_inf->dst_Y_h_min,mt_data_inf->dst_Y_h_max);
			break;
		case 41 :
			BlurR2_16_MT_H((unsigned char *const)mt_data_inf->dst_Y1,(unsigned char *const)mt_data_inf->dst_Y2,
				mt_data_inf->src_pitch_Y2,mt_data_inf->dst_pitch_Y2,mt_data_inf->dst_Y_h,
				mt_data_inf->row_size_Y2,mt_data_inf->processH,mt_data_inf->dst_Y_h_min,mt_data_inf->dst_Y_h_max);
			break;
		case 42 :
			BlurR2_16_MT_V((unsigned char *const)mt_data_inf->dst_Y1,(unsigned char *const)mt_data_inf->dst_Y2,
				mt_data_inf->src_pitch_Y2,mt_data_inf->dst_pitch_Y2,mt_data_inf->dst_Y_h,
				mt_data_inf->row_size_Y2,mt_data_inf->processV,mt_data_inf->dst_Y_h_min,mt_data_inf->dst_Y_h_max);
			break;
		case 43 :
			BlurR6_16_MT_H((unsigned char *const)mt_data_inf->dst_Y1,(unsigned char *const)mt_data_inf->dst_Y2,
				mt_data_inf->src_pitch_Y2,mt_data_inf->dst_pitch_Y2,mt_data_inf->dst_Y_h,
				mt_data_inf->row_size_Y2,mt_data_inf->processH,mt_data_inf->dst_Y_h_min,mt_data_inf->dst_Y_h_max);
			break;
		case 44 :
			BlurR6_16_MT_V((unsigned char *const)mt_data_inf->dst_Y1,(unsigned char *const)mt_data_inf->dst_Y2,
				mt_data_inf->src_pitch_Y2,mt_data_inf->dst_pitch_Y2,mt_data_inf->dst_Y_h,
				mt_data_inf->row_size_Y2,mt_data_inf->processV,mt_data_inf->dst_Y_h_min,mt_data_inf->dst_Y_h_max);
			break;
		case 45 :
			warp0_u16_MT((const unsigned char *)mt_data_inf->src_Y1,(const unsigned char *)mt_data_inf->src_Y2,
				(unsigned char *)mt_data_inf->dst_Y2,mt_data_inf->src_pitch_Y1,mt_data_inf->src_pitch_Y2,mt_data_inf->dst_pitch_Y2,
				mt_data_inf->row_size_Y3 >> 1,mt_data_inf->dst_Y_h,ptrClass->depth,ptrClass->depthV,ptrClass->bits_per_pixel,
				mt_data_inf->dst_Y_h_min,mt_data_inf->dst_Y_h_max);
			break;
		case 46 :
			Sobel_16_MT((const unsigned char *)mt_data_inf->src_U1,(unsigned char *)mt_data_inf->dst_U1,
				mt_data_inf->src_pitch_U1,mt_data_inf->src_pitch_U2,mt_data_inf->src_U_h,mt_data_inf->row_size_U1,
				ptrClass->threshC,ptrClass->bits_per_pixel,mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 47 :
			BlurR2_16_MT_H((unsigned char *const)mt_data_inf->dst_U1,(unsigned char *const)mt_data_inf->dst_U2,
				mt_data_inf->src_pitch_U2,mt_data_inf->dst_pitch_U2,mt_data_inf->dst_U_h,
				mt_data_inf->row_size_U1,true,mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 48 :
			BlurR2_16_MT_V((unsigned char *const)mt_data_inf->dst_U1,(unsigned char *const)mt_data_inf->dst_U2,
				mt_data_inf->src_pitch_U2,mt_data_inf->dst_pitch_U2,mt_data_inf->dst_U_h,
				mt_data_inf->row_size_U1,true,mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 49 :
			BlurR6_16_MT_H((unsigned char *const)mt_data_inf->dst_U1,(unsigned char *const)mt_data_inf->dst_U2,
				mt_data_inf->src_pitch_U2,mt_data_inf->dst_pitch_U2,mt_data_inf->dst_U_h,
				mt_data_inf->row_size_U1,true,mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 50 :
			BlurR6_16_MT_V((unsigned char *const)mt_data_inf->dst_U1,(unsigned char *const)mt_data_inf->dst_U2,
				mt_data_inf->src_pitch_U2,mt_data_inf->dst_pitch_U2,mt_data_inf->dst_U_h,
				mt_data_inf->row_size_U1,true,mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 51 :
			BlurR2_16_MT_H((unsigned char *const)mt_data_inf->dst_U1,(unsigned char *const)mt_data_inf->dst_U2,
				mt_data_inf->src_pitch_U2,mt_data_inf->dst_pitch_U2,mt_data_inf->dst_U_h,
				mt_data_inf->row_size_U1,mt_data_inf->cprocessH,
				mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 52 :
			BlurR2_16_MT_V((unsigned char *const)mt_data_inf->dst_U1,(unsigned char *const)mt_data_inf->dst_U2,
				mt_data_inf->src_pitch_U2,mt_data_inf->dst_pitch_U2,mt_data_inf->dst_U_h,
				mt_data_inf->row_size_U1,mt_data_inf->cprocessV,
				mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 53 :
			BlurR6_16_MT_H((unsigned char *const)mt_data_inf->dst_U1,(unsigned char *const)mt_data_inf->dst_U2,
				mt_data_inf->src_pitch_U2,mt_data_inf->dst_pitch_U2,mt_data_inf->dst_U_h,
				mt_data_inf->row_size_U1,mt_data_inf->cprocessH,
				mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 54 :
			BlurR6_16_MT_V((unsigned char *const)mt_data_inf->dst_U1,(unsigned char *const)mt_data_inf->dst_U2,
				mt_data_inf->src_pitch_U2,mt_data_inf->dst_pitch_U2,mt_data_inf->dst_U_h,
				mt_data_inf->row_size_U1,mt_data_inf->cprocessV,
				mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 55 :
			warp0_u16_MT((const unsigned char *)mt_data_inf->src_U1,(const unsigned char *)mt_data_inf->src_U2,
				(unsigned char *)mt_data_inf->dst_U2,mt_data_inf->src_pitch_U1,mt_data_inf->src_pitch_U2,mt_data_inf->dst_pitch_U2,
				mt_data_inf->row_size_U2 >> 1,mt_data_inf->dst_U_h,ptrClass->depthC,ptrClass->depthVC,ptrClass->bits_per_pixel,
				mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 56 :
			Sobel_16_MT((const unsigned char *)mt_data_inf->src_V1,(unsigned char *)mt_data_inf->dst_V1,
				mt_data_inf->src_pitch_V1,mt_data_inf->src_pitch_V2,mt_data_inf->src_V_h,mt_data_inf->row_size_V1,
				ptrClass->threshC,ptrClass->bits_per_pixel,mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 57 :
			BlurR2_16_MT_H((unsigned char *const)mt_data_inf->dst_V1,(unsigned char *const)mt_data_inf->dst_V2,
				mt_data_inf->src_pitch_V2,mt_data_inf->dst_pitch_V2,mt_data_inf->dst_V_h,
				mt_data_inf->row_size_V1,true,mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 58 :
			BlurR2_16_MT_V((unsigned char *const)mt_data_inf->dst_V1,(unsigned char *const)mt_data_inf->dst_V2,
				mt_data_inf->src_pitch_V2,mt_data_inf->dst_pitch_V2,mt_data_inf->dst_V_h,
				mt_data_inf->row_size_V1,true,mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 59 :
			BlurR6_16_MT_H((unsigned char *const)mt_data_inf->dst_V1,(unsigned char *const)mt_data_inf->dst_V2,
				mt_data_inf->src_pitch_V2,mt_data_inf->dst_pitch_V2,mt_data_inf->dst_V_h,
				mt_data_inf->row_size_V1,true,mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 60 :
			BlurR6_16_MT_V((unsigned char *const)mt_data_inf->dst_V1,(unsigned char *const)mt_data_inf->dst_V2,
				mt_data_inf->src_pitch_V2,mt_data_inf->dst_pitch_V2,mt_data_inf->dst_V_h,
				mt_data_inf->row_size_V1,true,mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 61 :
			BlurR2_16_MT_H((unsigned char *const)mt_data_inf->dst_V1,(unsigned char *const)mt_data_inf->dst_V2,
				mt_data_inf->src_pitch_V2,mt_data_inf->dst_pitch_V2,mt_data_inf->dst_V_h,
				mt_data_inf->row_size_V1,mt_data_inf->cprocessH,
				mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 62 :
			BlurR2_16_MT_V((unsigned char *const)mt_data_inf->dst_V1,(unsigned char *const)mt_data_inf->dst_V2,
				mt_data_inf->src_pitch_V2,mt_data_inf->dst_pitch_V2,mt_data_inf->dst_V_h,
				mt_data_inf->row_size_V1,mt_data_inf->cprocessV,
				mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 63 :
			BlurR6_16_MT_H((unsigned char *const)mt_data_inf->dst_V1,(unsigned char *const)mt_data_inf->dst_V2,
				mt_data_inf->src_pitch_V2,mt_data_inf->dst_pitch_V2,mt_data_inf->dst_V_h,
				mt_data_inf->row_size_V1,mt_data_inf->cprocessH,
				mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 64 :
			BlurR6_16_MT_V((unsigned char *const)mt_data_inf->dst_V1,(unsigned char *const)mt_data_inf->dst_V2,
				mt_data_inf->src_pitch_V2,mt_data_inf->dst_pitch_V2,mt_data_inf->dst_V_h,
				mt_data_inf->row_size_V1,mt_data_inf->cprocessV,
				mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 65 :
			warp0_u16_MT((const unsigned char *)mt_data_inf->src_V1,(const unsigned char *)mt_data_inf->src_V2,
				(unsigned char *)mt_data_inf->dst_V2,mt_data_inf->src_pitch_V1,mt_data_inf->src_pitch_V2,mt_data_inf->dst_pitch_V2,
				mt_data_inf->row_size_V2 >> 1,mt_data_inf->dst_V_h,ptrClass->depthC,ptrClass->depthVC,ptrClass->bits_per_pixel,
				mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 66 :
			GuideChroma_16_MT((const unsigned char *)mt_data_inf->src_Y2,(unsigned char *)mt_data_inf->dst_U1,
				mt_data_inf->src_pitch_Y2,mt_data_inf->src_pitch_U2,mt_data_inf->dst_U_h,
				mt_data_inf->row_size_U1 >> 1,mt_data_inf->SubW_U,mt_data_inf->SubH_U,ptrClass->cplace_mpeg2_flag,
				mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 67 :
			warp0_u16_MT((const unsigned char *)mt_data_inf->src_U1,(const unsigned char *)mt_data_inf->src_U2,
				(unsigned char *)mt_data_inf->dst_U2,mt_data_inf->src_pitch_U1,mt_data_inf->src_pitch_U2,mt_data_inf->dst_pitch_U2,
				mt_data_inf->row_size_U2 >> 1,mt_data_inf->dst_U_h,ptrClass->depthC,ptrClass->depthVC,ptrClass->bits_per_pixel,
				mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 68 :
			warp0_u16_MT((const unsigned char *)mt_data_inf->src_V1,(const unsigned char *)mt_data_inf->src_U2,
				(unsigned char *)mt_data_inf->dst_V2,mt_data_inf->src_pitch_V1,mt_data_inf->src_pitch_U2,mt_data_inf->dst_pitch_V2,
				mt_data_inf->row_size_V2 >> 1,mt_data_inf->dst_V_h,ptrClass->depthC,ptrClass->depthVC,ptrClass->bits_per_pixel,
				mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 69 :
			warp0_u16_MT((const unsigned char *)mt_data_inf->src_U1,(const unsigned char *)mt_data_inf->src_Y2,
				(unsigned char *)mt_data_inf->dst_U2,mt_data_inf->src_pitch_U1,mt_data_inf->src_pitch_Y2,mt_data_inf->dst_pitch_U2,
				mt_data_inf->row_size_U2 >> 1,mt_data_inf->dst_U_h,ptrClass->depthC,ptrClass->depthVC,ptrClass->bits_per_pixel,
				mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 70 :
			warp0_u16_MT((const unsigned char *)mt_data_inf->src_V1,(const unsigned char *)mt_data_inf->src_Y2,
				(unsigned char *)mt_data_inf->dst_V2,mt_data_inf->src_pitch_V1,mt_data_inf->src_pitch_Y2,mt_data_inf->dst_pitch_V2,
				mt_data_inf->row_size_V2 >> 1,mt_data_inf->dst_V_h,ptrClass->depthC,ptrClass->depthVC,ptrClass->bits_per_pixel,
				mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		default : ;
	}
}


PVideoFrame __stdcall aWarpSharp::GetFrame(int n, IScriptEnvironment *env)
{
  PVideoFrame src = child->GetFrame(n, env);
  PVideoFrame tmp = env->NewVideoFrame(vi,64);
  PVideoFrame dst = (has_at_least_v8)?env->NewVideoFrameP(vi,&src):env->NewVideoFrame(vi,64);

  const int32_t src_pitch_Y = src->GetPitch(PLANAR_Y);
  const int32_t tmp_pitch_Y = tmp->GetPitch(PLANAR_Y);
  const int32_t dst_pitch_Y = dst->GetPitch(PLANAR_Y);
  const int32_t src_pitch_U = src->GetPitch(PLANAR_U);
  const int32_t tmp_pitch_U = tmp->GetPitch(PLANAR_U);
  const int32_t dst_pitch_U = dst->GetPitch(PLANAR_U);
  const int32_t src_pitch_V = src->GetPitch(PLANAR_V);
  const int32_t tmp_pitch_V = tmp->GetPitch(PLANAR_V);
  const int32_t dst_pitch_V = dst->GetPitch(PLANAR_V);

  const unsigned char *psrc_Y = src->GetReadPtr(PLANAR_Y);
  const unsigned char *ptmp_Y = tmp->GetReadPtr(PLANAR_Y);
  unsigned char *const wptmp_Y= tmp->GetWritePtr(PLANAR_Y);
  unsigned char *dptmp_Y = tmp->GetWritePtr(PLANAR_Y);
  unsigned char *const wpdst_Y = dst->GetWritePtr(PLANAR_Y);
  unsigned char *pdst_Y = dst->GetWritePtr(PLANAR_Y);

  const unsigned char *psrc_U = src->GetReadPtr(PLANAR_U);
  const unsigned char *ptmp_U = tmp->GetReadPtr(PLANAR_U);
  unsigned char *const wptmp_U = tmp->GetWritePtr(PLANAR_U);
  unsigned char *dptmp_U = tmp->GetWritePtr(PLANAR_U);
  unsigned char *const wpdst_U = dst->GetWritePtr(PLANAR_U);
  unsigned char *pdst_U = dst->GetWritePtr(PLANAR_U);

  const unsigned char *psrc_V = src->GetReadPtr(PLANAR_V);
  const unsigned char *ptmp_V = tmp->GetReadPtr(PLANAR_V);
  unsigned char *const wptmp_V = tmp->GetWritePtr(PLANAR_V);
  unsigned char *dptmp_V = tmp->GetWritePtr(PLANAR_V);
  unsigned char *const wpdst_V = dst->GetWritePtr(PLANAR_V);
  unsigned char *pdst_V = dst->GetWritePtr(PLANAR_V);

  const int SubH_U = vi.IsY() ? 0:vi.GetPlaneHeightSubsampling(PLANAR_U);
  const int SubW_U = vi.IsY() ? 0:vi.GetPlaneWidthSubsampling(PLANAR_U);

  const int32_t src_height_Y = src->GetHeight(PLANAR_Y);
  const int32_t tmp_height_Y = tmp->GetHeight(PLANAR_Y);
  const int32_t dst_height_Y = dst->GetHeight(PLANAR_Y);

  const int32_t src_height_U = src->GetHeight(PLANAR_U);
  const int32_t tmp_height_U = tmp->GetHeight(PLANAR_U);
  const int32_t dst_height_U = dst->GetHeight(PLANAR_U);

  const int32_t src_height_V = src->GetHeight(PLANAR_V);
  const int32_t tmp_height_V = tmp->GetHeight(PLANAR_V);
  const int32_t dst_height_V = dst->GetHeight(PLANAR_V);

  const int32_t src_row_size_Y = src->GetRowSize(PLANAR_Y);
  const int32_t tmp_row_size_Y = tmp->GetRowSize(PLANAR_Y);
  const int32_t dst_row_size_Y = dst->GetRowSize(PLANAR_Y);

  const int32_t tmp_row_size_U = tmp->GetRowSize(PLANAR_U);
  const int32_t dst_row_size_U = dst->GetRowSize(PLANAR_U);

  const int32_t tmp_row_size_V = tmp->GetRowSize(PLANAR_V);
  const int32_t dst_row_size_V = dst->GetRowSize(PLANAR_V);

  const int blurL=std::min(blur_level,blur_levelV);
  const int blurLr=std::max(blur_level,blur_levelV)-blurL;
  const bool processH=blur_level>blurL,processV=blur_levelV>blurL;

  const int cblurL=std::min(blur_levelC,blur_levelVC);
  const int cblurLr=std::max(blur_levelC,blur_levelVC)-cblurL;
  const bool cprocessH=blur_levelC>cblurL,cprocessV=blur_levelVC>cblurL;

  Public_MT_Data_Thread MT_ThreadGF[MAX_MT_THREADS];
  MT_Data_Info_WarpSharp MT_DataGF[MAX_MT_THREADS];

  memcpy(MT_ThreadGF,MT_Thread,sizeof(MT_Thread));
  memcpy(MT_DataGF,MT_Data,sizeof(MT_Data));

  int8_t nPool=-1;

  for(uint8_t i=0; i<threads_number; i++)
	MT_ThreadGF[i].pData=(void *)MT_DataGF;

  if (threads_number>1)
  {
	  if ((!poolInterface->RequestThreadPool(UserId,threads_number,MT_ThreadGF,nPool,false,true)) || (nPool==-1))
		  env->ThrowError("aWarpSharp: Error with the TheadPool while requesting threadpool!");
  }

	if (threads_number>1)
	{

	for(uint8_t i=0; i<threads_number; i++)
	{
		MT_DataGF[i].src_Y1=(void *)psrc_Y;
		MT_DataGF[i].src_Y2=(void *)ptmp_Y;
		MT_DataGF[i].src_pitch_Y1=src_pitch_Y;
		MT_DataGF[i].src_pitch_Y2=tmp_pitch_Y;
		MT_DataGF[i].row_size_Y1=src_row_size_Y;
		MT_DataGF[i].row_size_Y2=tmp_row_size_Y;
		MT_DataGF[i].row_size_Y3=dst_row_size_Y;
		MT_DataGF[i].dst_Y1=(void *)dptmp_Y;
		MT_DataGF[i].dst_Y2=(void *)pdst_Y;
		MT_DataGF[i].dst_pitch_Y1=tmp_pitch_Y;
		MT_DataGF[i].dst_pitch_Y2=dst_pitch_Y;

		MT_DataGF[i].src_U1=(void *)psrc_U;
		MT_DataGF[i].src_U2=(void *)ptmp_U;
		MT_DataGF[i].src_pitch_U1=src_pitch_U;
		MT_DataGF[i].src_pitch_U2=tmp_pitch_U;
		MT_DataGF[i].row_size_U1=tmp_row_size_U;
		MT_DataGF[i].row_size_U2=dst_row_size_U;
		MT_DataGF[i].dst_U1=(void *)dptmp_U;
		MT_DataGF[i].dst_U2=(void *)pdst_U;
		MT_DataGF[i].dst_pitch_U1=tmp_pitch_U;
		MT_DataGF[i].dst_pitch_U2=dst_pitch_U;

		MT_DataGF[i].src_V1=(void *)psrc_V;
		MT_DataGF[i].src_V2=(void *)ptmp_V;
		MT_DataGF[i].src_pitch_V1=src_pitch_V;
		MT_DataGF[i].src_pitch_V2=tmp_pitch_V;
		MT_DataGF[i].row_size_V1=tmp_row_size_V;
		MT_DataGF[i].row_size_V2=dst_row_size_V;
		MT_DataGF[i].dst_V1=(void *)dptmp_V;
		MT_DataGF[i].dst_V2=(void *)pdst_V;
		MT_DataGF[i].dst_pitch_V1=tmp_pitch_V;
		MT_DataGF[i].dst_pitch_V2=dst_pitch_V;

		MT_DataGF[i].src_Y_h=src_height_Y;
		MT_DataGF[i].src_U_h=src_height_U;
		MT_DataGF[i].src_V_h=src_height_V;
		MT_DataGF[i].dst_Y_h=dst_height_Y;
		MT_DataGF[i].dst_U_h=dst_height_U;
		MT_DataGF[i].dst_V_h=dst_height_V;

		MT_DataGF[i].processH=processH;
		MT_DataGF[i].processV=processV;
		MT_DataGF[i].cprocessH=cprocessH;
		MT_DataGF[i].cprocessV=cprocessV;
		MT_DataGF[i].SubW_U=SubW_U;
		MT_DataGF[i].SubH_U=SubH_U;
	}

		uint8_t f_proc;

		const uint8_t offs_16b=(pixelsize==1) ? 0:35;

  if (chroma!=5)
  {
	  f_proc=1+offs_16b;

	  for(uint8_t i=0; i<threads_number; i++)
		  MT_ThreadGF[i].f_process=f_proc;

	  if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);

	  f_proc=(blur_type==1) ? (2+offs_16b):(4+offs_16b);

	for (int i=0; i<blurL; i++)
	{
		for(uint8_t i=0; i<threads_number; i++)
			MT_ThreadGF[i].f_process=f_proc;
		if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);

		for(uint8_t i=0; i<threads_number; i++)
			MT_ThreadGF[i].f_process++;
		if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);
	}

	f_proc=(blur_type==1) ? (6+offs_16b):(8+offs_16b);

	for (int i=0; i<blurLr; i++)
	{
		for(uint8_t i=0; i<threads_number; i++)
			MT_ThreadGF[i].f_process=f_proc;
		if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);

		for(uint8_t i=0; i<threads_number; i++)
			MT_ThreadGF[i].f_process++;
		if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);
	}

    if ((chroma!=6) && ((depth!=0) || (depthV!=0)))
	{
		f_proc=10+offs_16b;

		for(uint8_t i=0; i<threads_number; i++)
			MT_ThreadGF[i].f_process=f_proc;

		if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);
	}
    else
      CopyPlane(src,dst,PLANAR_Y,vi);
  }
  else
    CopyPlane(src,dst,PLANAR_Y,vi);

  switch (chroma)
  {
  case 0 :
	  if (pixelsize==1)
	  {
		  SetPlane(dst,PLANAR_U,0x80,vi);
		  SetPlane(dst,PLANAR_V,0x80,vi);
	  }
	  else
	  {
		  SetPlane_16(dst,PLANAR_U,0x80 << (bits_per_pixel-8),vi);
		  SetPlane_16(dst,PLANAR_V,0x80 << (bits_per_pixel-8),vi);
	  }
    break;
  case 1 : break;
  case 2 :
    CopyPlane(src,dst,PLANAR_U,vi);
    CopyPlane(src,dst,PLANAR_V,vi);
    break;
  case 3 :
  case 5 :
	  if ((depthC!=0) || (depthVC!=0))
	  {
		  f_proc=11+offs_16b;

		  for(uint8_t i=0; i<threads_number; i++)
			  MT_ThreadGF[i].f_process=f_proc;

		  if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);

		  f_proc=(blur_type==1) ? (12+offs_16b):(14+offs_16b);

	for (int i=0; i<cblurL; i++)
	{
		for(uint8_t i=0; i<threads_number; i++)
			MT_ThreadGF[i].f_process=f_proc;
		if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);

		for(uint8_t i=0; i<threads_number; i++)
			MT_ThreadGF[i].f_process++;
		if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);
	}

		  f_proc=(blur_type==1) ? (16+offs_16b):(18+offs_16b);

	for (int i=0; i<cblurLr; i++)
	{
		for(uint8_t i=0; i<threads_number; i++)
			MT_ThreadGF[i].f_process=f_proc;
		if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);

		for(uint8_t i=0; i<threads_number; i++)
			MT_ThreadGF[i].f_process++;
		if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);
	}

	f_proc=20+offs_16b;

	for(uint8_t i=0; i<threads_number; i++)
		MT_ThreadGF[i].f_process=f_proc;

	if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);

	f_proc=21+offs_16b;

	for(uint8_t i=0; i<threads_number; i++)
		MT_ThreadGF[i].f_process=f_proc;

	if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);

	f_proc=(blur_type==1) ? (22+offs_16b):(24+offs_16b);

	for (int i=0; i<cblurL; i++)
	{
		for(uint8_t i=0; i<threads_number; i++)
			MT_ThreadGF[i].f_process=f_proc;
		if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);

		for(uint8_t i=0; i<threads_number; i++)
			MT_ThreadGF[i].f_process++;
		if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);
	}

	f_proc=(blur_type==1) ? (26+offs_16b):(28+offs_16b);

	for (int i=0; i<cblurLr; i++)
	{
		for(uint8_t i=0; i<threads_number; i++)
			MT_ThreadGF[i].f_process=f_proc;
		if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);

		for(uint8_t i=0; i<threads_number; i++)
			MT_ThreadGF[i].f_process++;
		if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);
	}

	f_proc=30+offs_16b;

	for(uint8_t i=0; i<threads_number; i++)
		MT_ThreadGF[i].f_process=f_proc;

	if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);

	  }
	  else
	  {
		   CopyPlane(src,dst,PLANAR_U,vi);
		   CopyPlane(src,dst,PLANAR_V,vi);
	  }
    break;
  case 4 :
  case 6 :
	  if ((depthC!=0) || (depthVC!=0))
	  {
    if (!vi.Is444())
    {
	  if (!GuideChroma_Test(SubW_U,SubH_U)) env->ThrowError("aWarpSharp: Unsuported colorspace");
	  f_proc=31+offs_16b;

	  for(uint8_t i=0; i<threads_number; i++)
		  MT_ThreadGF[i].f_process=f_proc;

	  if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);

	  f_proc=32+offs_16b;

	  for(uint8_t i=0; i<threads_number; i++)
		  MT_ThreadGF[i].f_process=f_proc;

	  if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);

	  f_proc=33+offs_16b;

	  for(uint8_t i=0; i<threads_number; i++)
		  MT_ThreadGF[i].f_process=f_proc;

	  if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);
    }
    else
    {
	  f_proc=34+offs_16b;

	  for(uint8_t i=0; i<threads_number; i++)
		  MT_ThreadGF[i].f_process=f_proc;

	  if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);

	  f_proc=35+offs_16b;

	  for(uint8_t i=0; i<threads_number; i++)
		  MT_ThreadGF[i].f_process=f_proc;

	  if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);
    }
	  }
	  else
	  {
		   CopyPlane(src,dst,PLANAR_U,vi);
		   CopyPlane(src,dst,PLANAR_V,vi);
	  }
	  break;
  default : break;
  }

  for(uint8_t i=0; i<threads_number; i++)
	  MT_ThreadGF[i].f_process=0;

  poolInterface->ReleaseThreadPool(UserId,sleep,nPool);

	}
	else
	{

  if (chroma!=5)
  {
	if (pixelsize==1) Sobel_8(psrc_Y,dptmp_Y,src_pitch_Y,tmp_pitch_Y,src_height_Y,tmp_row_size_Y,thresh);
	else Sobel_16(psrc_Y,dptmp_Y,src_pitch_Y,tmp_pitch_Y,src_height_Y,tmp_row_size_Y,thresh,bits_per_pixel);
	for (int i=0; i<blurL; i++)
	{
		if (pixelsize==1)
		{
			if (blur_type==1) BlurR2_8(wptmp_Y,wpdst_Y,tmp_pitch_Y,dst_pitch_Y,tmp_height_Y,tmp_row_size_Y,true,true);
			else BlurR6_8(wptmp_Y,wpdst_Y,tmp_pitch_Y,dst_pitch_Y,tmp_height_Y,tmp_row_size_Y,true,true);
		}
		else
		{
			if (blur_type==1) BlurR2_16(wptmp_Y,wpdst_Y,tmp_pitch_Y,dst_pitch_Y,tmp_height_Y,tmp_row_size_Y,true,true);
			else BlurR6_16(wptmp_Y,wpdst_Y,tmp_pitch_Y,dst_pitch_Y,tmp_height_Y,tmp_row_size_Y,true,true);
		}
	}
	for (int i=0; i<blurLr; i++)
	{
		if (pixelsize==1)
		{
			if (blur_type==1) BlurR2_8(wptmp_Y,wpdst_Y,tmp_pitch_Y,dst_pitch_Y,tmp_height_Y,tmp_row_size_Y,processH,processV);
			else BlurR6_8(wptmp_Y,wpdst_Y,tmp_pitch_Y,dst_pitch_Y,tmp_height_Y,tmp_row_size_Y,processH,processV);
		}
		else
		{
			if (blur_type==1) BlurR2_16(wptmp_Y,wpdst_Y,tmp_pitch_Y,dst_pitch_Y,tmp_height_Y,tmp_row_size_Y,processH,processV);
			else BlurR6_16(wptmp_Y,wpdst_Y,tmp_pitch_Y,dst_pitch_Y,tmp_height_Y,tmp_row_size_Y,processH,processV);
		}
	}
    if ((chroma!=6) && ((depth!=0) || (depthV!=0)))
	{
		if (pixelsize==1) Warp0_8(psrc_Y,ptmp_Y,pdst_Y,src_pitch_Y,tmp_pitch_Y,dst_pitch_Y,dst_row_size_Y,dst_height_Y,depth,depthV);
		else warp0_u16(psrc_Y,ptmp_Y,pdst_Y,src_pitch_Y,tmp_pitch_Y,dst_pitch_Y,dst_row_size_Y >> 1,dst_height_Y,
			depth,depthV,bits_per_pixel);
	}
    else
      CopyPlane(src,dst,PLANAR_Y,vi);
  }
  else
    CopyPlane(src,dst,PLANAR_Y,vi);

  switch (chroma)
  {
  case 0 :
	  if (pixelsize==1)
	  {
		  SetPlane(dst,PLANAR_U,0x80,vi);
		  SetPlane(dst,PLANAR_V,0x80,vi);
	  }
	  else
	  {
		  SetPlane_16(dst,PLANAR_U,0x80 << (bits_per_pixel-8),vi);
		  SetPlane_16(dst,PLANAR_V,0x80 << (bits_per_pixel-8),vi);
	  }
    break;
  case 1 : break;
  case 2 :
    CopyPlane(src,dst,PLANAR_U,vi);
    CopyPlane(src,dst,PLANAR_V,vi);
    break;
  case 3 :
  case 5 :
	  if ((depthC!=0) || (depthVC!=0))
	  {
	if (pixelsize==1) Sobel_8(psrc_U,dptmp_U,src_pitch_U,tmp_pitch_U,src_height_U,tmp_row_size_U,threshC);
	else Sobel_16(psrc_U,dptmp_U,src_pitch_U,tmp_pitch_U,src_height_U,tmp_row_size_U,threshC,bits_per_pixel);
	for (int i=0; i<cblurL; i++)
	{
		if (pixelsize==1)
		{
			if (blur_type==1) BlurR2_8(wptmp_U,wpdst_U,tmp_pitch_U,dst_pitch_U,tmp_height_U,tmp_row_size_U,true,true);
			else BlurR6_8(wptmp_U,wpdst_U,tmp_pitch_U,dst_pitch_U,tmp_height_U,tmp_row_size_U,true,true);
		}
		else
		{
			if (blur_type==1) BlurR2_16(wptmp_U,wpdst_U,tmp_pitch_U,dst_pitch_U,tmp_height_U,tmp_row_size_U,true,true);
			else BlurR6_16(wptmp_U,wpdst_U,tmp_pitch_U,dst_pitch_U,tmp_height_U,tmp_row_size_U,true,true);
		}
	}
	for (int i=0; i<cblurLr; i++)
	{
		if (pixelsize==1)
		{
			if (blur_type) BlurR2_8(wptmp_U,wpdst_U,tmp_pitch_U,dst_pitch_U,tmp_height_U,tmp_row_size_U,cprocessH,cprocessV);
			else BlurR6_8(wptmp_U,wpdst_U,tmp_pitch_U,dst_pitch_U,tmp_height_U,tmp_row_size_U,cprocessH,cprocessV);
		}
		else
		{
			if (blur_type) BlurR2_16(wptmp_U,wpdst_U,tmp_pitch_U,dst_pitch_U,tmp_height_U,tmp_row_size_U,cprocessH,cprocessV);
			else BlurR6_16(wptmp_U,wpdst_U,tmp_pitch_U,dst_pitch_U,tmp_height_U,tmp_row_size_U,cprocessH,cprocessV);
		}
	}
	if (pixelsize==1) Warp0_8(psrc_U,ptmp_U,pdst_U,src_pitch_U,tmp_pitch_U,dst_pitch_U,dst_row_size_U,dst_height_U,depthC,depthVC);
	else warp0_u16(psrc_U,ptmp_U,pdst_U,src_pitch_U,tmp_pitch_U,dst_pitch_U,dst_row_size_U >> 1,dst_height_U,
		depthC,depthVC,bits_per_pixel);

	if (pixelsize==1) Sobel_8(psrc_V,dptmp_V,src_pitch_V,tmp_pitch_V,src_height_V,tmp_row_size_V,threshC);
	else Sobel_16(psrc_V,dptmp_V,src_pitch_V,tmp_pitch_V,src_height_V,tmp_row_size_V,threshC,bits_per_pixel);
	for (int i=0; i<cblurL; i++)
	{
		if (pixelsize==1)
		{
			if (blur_type==1) BlurR2_8(wptmp_V,wpdst_V,tmp_pitch_V,dst_pitch_V,tmp_height_V,tmp_row_size_V,true,true);
			else BlurR6_8(wptmp_V,wpdst_V,tmp_pitch_V,dst_pitch_V,tmp_height_V,tmp_row_size_V,true,true);
		}
		else
		{
			if (blur_type==1) BlurR2_16(wptmp_V,wpdst_V,tmp_pitch_V,dst_pitch_V,tmp_height_V,tmp_row_size_V,true,true);
			else BlurR6_16(wptmp_V,wpdst_V,tmp_pitch_V,dst_pitch_V,tmp_height_V,tmp_row_size_V,true,true);
		}
	}
	for (int i=0; i<cblurLr; i++)
	{
		if (pixelsize==1)
		{
			if (blur_type==1) BlurR2_8(wptmp_V,wpdst_V,tmp_pitch_V,dst_pitch_V,tmp_height_V,tmp_row_size_V,cprocessH,cprocessV);
			else BlurR6_8(wptmp_V,wpdst_V,tmp_pitch_V,dst_pitch_V,tmp_height_V,tmp_row_size_V,cprocessH,cprocessV);
		}
		else
		{
			if (blur_type==1) BlurR2_16(wptmp_V,wpdst_V,tmp_pitch_V,dst_pitch_V,tmp_height_V,tmp_row_size_V,cprocessH,cprocessV);
			else BlurR6_16(wptmp_V,wpdst_V,tmp_pitch_V,dst_pitch_V,tmp_height_V,tmp_row_size_V,cprocessH,cprocessV);
		}
	}
	if (pixelsize==1) Warp0_8(psrc_V,ptmp_V,pdst_V,src_pitch_V,tmp_pitch_V,dst_pitch_V,dst_row_size_V,dst_height_V,depthC,depthVC);
	else warp0_u16(psrc_V,ptmp_V,pdst_V,src_pitch_V,tmp_pitch_V,dst_pitch_V,dst_row_size_V >> 1,dst_height_V,
		depthC,depthVC,bits_per_pixel);
	  }
	  else
	  {
		   CopyPlane(src,dst,PLANAR_U,vi);
		   CopyPlane(src,dst,PLANAR_V,vi);
	  }
    break;
  case 4 :
  case 6 :
	  if ((depthC!=0) || (depthVC!=0))
	  {
    if (!vi.Is444())
    {
		const bool testC=(pixelsize==1) ? GuideChroma_8(ptmp_Y,dptmp_U,tmp_pitch_Y,tmp_pitch_U,tmp_height_U,tmp_row_size_U,SubW_U,
			SubH_U,cplace_mpeg2_flag):GuideChroma_16(ptmp_Y,dptmp_U,tmp_pitch_Y,tmp_pitch_U,tmp_height_U,tmp_row_size_U >> 1,SubW_U,
			SubH_U,cplace_mpeg2_flag);

	  if (!testC) env->ThrowError("aWarpSharp: Unsuported colorspace");
	  if (pixelsize==1)
	  {
		  Warp0_8(psrc_U,ptmp_U,pdst_U,src_pitch_U,tmp_pitch_U,dst_pitch_U,dst_row_size_U,dst_height_U,depthC,depthVC);
		  Warp0_8(psrc_V,ptmp_U,pdst_V,src_pitch_V,tmp_pitch_U,dst_pitch_V,dst_row_size_V,dst_height_V,depthC,depthVC);
	  }
	  else
	  {
		  warp0_u16(psrc_U,ptmp_U,pdst_U,src_pitch_U,tmp_pitch_U,dst_pitch_U,dst_row_size_U >> 1,dst_height_U,
			  depthC,depthVC,bits_per_pixel);
		  warp0_u16(psrc_V,ptmp_U,pdst_V,src_pitch_V,tmp_pitch_U,dst_pitch_V,dst_row_size_V >> 1,dst_height_V,
			  depthC,depthVC,bits_per_pixel);
	  }
    }
    else
    {
		if (pixelsize==1)
		{
			Warp0_8(psrc_U,ptmp_Y,pdst_U,src_pitch_U,tmp_pitch_Y,dst_pitch_U,dst_row_size_U,dst_height_U,depthC,depthVC);
			Warp0_8(psrc_V,ptmp_Y,pdst_V,src_pitch_V,tmp_pitch_Y,dst_pitch_V,dst_row_size_V,dst_height_V,depthC,depthVC);
		}
		else
		{
			warp0_u16(psrc_U,ptmp_Y,pdst_U,src_pitch_U,tmp_pitch_Y,dst_pitch_U,dst_row_size_U >> 1,dst_height_U,
				depthC,depthVC,bits_per_pixel);
			warp0_u16(psrc_V,ptmp_Y,pdst_V,src_pitch_V,tmp_pitch_Y,dst_pitch_V,dst_row_size_V >> 1,dst_height_V,
				depthC,depthVC,bits_per_pixel);
		}
    }
	  }
	  else
	  {
		   CopyPlane(src,dst,PLANAR_U,vi);
		   CopyPlane(src,dst,PLANAR_V,vi);
	  }
	  break;
  default : break;
  }

	}

  return dst;
}


aSobel::aSobel(PClip _child, int _thresh, int _chroma, int _threshC,uint8_t _threads,bool _sleep,bool _avsp, IScriptEnvironment *env) :
    GenericVideoFilter(_child), thresh(_thresh), chroma(_chroma),threshC(_threshC),sleep(_sleep),threads(_threads),avsp(_avsp)
{
	grey = vi.IsY();
	isRGBPfamily = vi.IsPlanarRGB() || vi.IsPlanarRGBA();
	isAlphaChannel = vi.IsYUVA() || vi.IsPlanarRGBA();

	pixelsize = (uint8_t)vi.ComponentSize(); // AVS16
	bits_per_pixel = (uint8_t)vi.BitsPerComponent();

	UserId=0;

    if (grey) chroma = 1;

	if (pixelsize>2)
	{
		if (threads>1) poolInterface->DeAllocateAllThreads(true);
		env->ThrowError("aSobel: Only 8 -> 16 bits supported");
	}
	if (!(vi.IsYUV() && vi.IsPlanar()))
	{
		if (threads>1) poolInterface->DeAllocateAllThreads(true);
		env->ThrowError("aSobel: Planar YUV input is required");
	}
	if ((thresh<0) || (thresh>255))
	{
		if (threads>1) poolInterface->DeAllocateAllThreads(true);
		env->ThrowError("aSobel: 'thresh' must be 0..255");
	}
	if ((threshC<0) || (threshC>255))
	{
		if (threads>1) poolInterface->DeAllocateAllThreads(true);
		env->ThrowError("aSobel: 'threshC' must be 0..255");
	}
	if ((chroma<0) || (chroma>6))
	{
		if (threads>1) poolInterface->DeAllocateAllThreads(true);
		env->ThrowError("aSobel: 'chroma' must be 0..6");
	}

	StaticThreadpoolF=StaticThreadpool;

	for (uint8_t i=0; i<MAX_MT_THREADS; i++)
	{
		MT_Thread[i].pClass=this;
		MT_Thread[i].f_process=0;
		MT_Thread[i].thread_Id=(uint8_t)i;
		MT_Thread[i].pFunc=StaticThreadpoolF;
	}

	const int shift_w = (!grey && vi.IsPlanar() && !isRGBPfamily) ? vi.GetPlaneWidthSubsampling(PLANAR_U) : 0;
	const int shift_h = (!grey && vi.IsPlanar() && !isRGBPfamily) ? vi.GetPlaneHeightSubsampling(PLANAR_U) : 0;

	if (vi.height<32) threads_number=1;
	else threads_number=threads;
	
	threads_number=CreateMTData(MT_Data,threads,threads_number,vi.width,vi.height,shift_w,shift_h);

	if (threads_number>1)
	{
		if (!poolInterface->GetUserId(UserId))
		{
			poolInterface->DeAllocateAllThreads(true);
			env->ThrowError("aSobel: Error with the TheadPool while getting UserId!");
		}
	}

	has_at_least_v8=true;
	try { env->CheckVersion(8); } catch (const AvisynthError&) { has_at_least_v8=false; }
}



aSobel::~aSobel()
{
	if (threads_number>1) poolInterface->RemoveUserId(UserId);
	if (threads>1) poolInterface->DeAllocateAllThreads(true);
}


int __stdcall aSobel::SetCacheHints(int cachehints,int frame_range)
{
  switch (cachehints)
  {
  case CACHE_GET_MTMODE :
    return MT_NICE_FILTER;
  default :
    return 0;
  }
}


void aSobel::StaticThreadpool(void *ptr)
{
	Public_MT_Data_Thread *data=(Public_MT_Data_Thread *)ptr;
	aSobel *ptrClass=(aSobel *)data->pClass;

	MT_Data_Info_WarpSharp *mt_data_inf=((MT_Data_Info_WarpSharp *)data->pData)+data->thread_Id;
	
	switch(data->f_process)
	{
		case 1 :
			Sobel_8_MT((const unsigned char *)mt_data_inf->src_Y1,(unsigned char *)mt_data_inf->dst_Y1,
				mt_data_inf->src_pitch_Y1,mt_data_inf->dst_pitch_Y1,mt_data_inf->src_Y_h,
				mt_data_inf->row_size_Y1,ptrClass->thresh,mt_data_inf->src_Y_h_min,mt_data_inf->src_Y_h_max);
			break;
		case 2 :
			Sobel_8_MT((const unsigned char *)mt_data_inf->src_U1,(unsigned char *)mt_data_inf->dst_U1,
				mt_data_inf->src_pitch_U1,mt_data_inf->dst_pitch_U1,mt_data_inf->src_U_h,
				mt_data_inf->row_size_U1,ptrClass->threshC,mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 3 :
			Sobel_8_MT((const unsigned char *)mt_data_inf->src_V1,(unsigned char *)mt_data_inf->dst_V1,
				mt_data_inf->src_pitch_V1,mt_data_inf->dst_pitch_V1,mt_data_inf->src_V_h,
				mt_data_inf->row_size_V1,ptrClass->threshC,mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
			// 16 bits
		case 4 :
			Sobel_16_MT((const unsigned char *)mt_data_inf->src_Y1,(unsigned char *)mt_data_inf->dst_Y1,
				mt_data_inf->src_pitch_Y1,mt_data_inf->dst_pitch_Y1,mt_data_inf->src_Y_h,mt_data_inf->row_size_Y1,
				ptrClass->thresh,ptrClass->bits_per_pixel,mt_data_inf->src_Y_h_min,mt_data_inf->src_Y_h_max);
			break;
		case 5 :
			Sobel_16_MT((const unsigned char *)mt_data_inf->src_U1,(unsigned char *)mt_data_inf->dst_U1,
				mt_data_inf->src_pitch_U1,mt_data_inf->dst_pitch_U1,mt_data_inf->src_U_h,mt_data_inf->row_size_U1,
				ptrClass->threshC,ptrClass->bits_per_pixel,mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 6 :
			Sobel_16_MT((const unsigned char *)mt_data_inf->src_V1,(unsigned char *)mt_data_inf->dst_V1,
				mt_data_inf->src_pitch_V1,mt_data_inf->dst_pitch_V1,mt_data_inf->src_V_h,mt_data_inf->row_size_V1,
				ptrClass->threshC,ptrClass->bits_per_pixel,mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		default : ;
	}
}


PVideoFrame __stdcall aSobel::GetFrame(int n, IScriptEnvironment *env)
{
  PVideoFrame src = child->GetFrame(n, env);
  PVideoFrame dst = (has_at_least_v8)?env->NewVideoFrameP(vi,&src):env->NewVideoFrame(vi,64);

  const int32_t src_pitch_Y = src->GetPitch(PLANAR_Y);
  const int32_t dst_pitch_Y = dst->GetPitch(PLANAR_Y);
  const int32_t src_pitch_U = src->GetPitch(PLANAR_U);
  const int32_t dst_pitch_U = dst->GetPitch(PLANAR_U);
  const int32_t src_pitch_V = src->GetPitch(PLANAR_V);
  const int32_t dst_pitch_V = dst->GetPitch(PLANAR_V);

  const unsigned char *psrc_Y = src->GetReadPtr(PLANAR_Y);
  unsigned char *pdst_Y = dst->GetWritePtr(PLANAR_Y);
  const unsigned char *psrc_U = src->GetReadPtr(PLANAR_U);
  unsigned char *pdst_U = dst->GetWritePtr(PLANAR_U);
  const unsigned char *psrc_V = src->GetReadPtr(PLANAR_V);
  unsigned char *pdst_V = dst->GetWritePtr(PLANAR_V);

  const int32_t src_height_Y = src->GetHeight(PLANAR_Y);
  const int32_t src_height_U = src->GetHeight(PLANAR_U);
  const int32_t src_height_V = src->GetHeight(PLANAR_V);

  const int32_t dst_row_size_Y = dst->GetRowSize(PLANAR_Y);
  const int32_t dst_row_size_U = dst->GetRowSize(PLANAR_U);
  const int32_t dst_row_size_V = dst->GetRowSize(PLANAR_V);

  Public_MT_Data_Thread MT_ThreadGF[MAX_MT_THREADS];
  MT_Data_Info_WarpSharp MT_DataGF[MAX_MT_THREADS];

  memcpy(MT_ThreadGF,MT_Thread,sizeof(MT_Thread));
  memcpy(MT_DataGF,MT_Data,sizeof(MT_Data));

  for(uint8_t i=0; i<threads_number; i++)
	MT_ThreadGF[i].pData=(void *)MT_DataGF;

  int8_t nPool=-1;

  if (threads_number>1)
  {
	  if ((!poolInterface->RequestThreadPool(UserId,threads_number,MT_ThreadGF,nPool,false,true)) || (nPool==-1))
		  env->ThrowError("aSobel: Error with the TheadPool while requesting threadpool!");
  }

  if (threads_number>1)
  {

	for(uint8_t i=0; i<threads_number; i++)
	{
		MT_DataGF[i].src_Y1=(void *)psrc_Y;
		MT_DataGF[i].src_pitch_Y1=src_pitch_Y;
		MT_DataGF[i].row_size_Y1=dst_row_size_Y;
		MT_DataGF[i].dst_Y1=(void *)pdst_Y;
		MT_DataGF[i].dst_pitch_Y1=dst_pitch_Y;

		MT_DataGF[i].src_U1=(void *)psrc_U;
		MT_DataGF[i].src_pitch_U1=src_pitch_U;
		MT_DataGF[i].row_size_U1=dst_row_size_U;
		MT_DataGF[i].dst_U1=(void *)pdst_U;
		MT_DataGF[i].dst_pitch_U1=dst_pitch_U;

		MT_DataGF[i].src_V1=(void *)psrc_V;
		MT_DataGF[i].src_pitch_V1=src_pitch_V;
		MT_DataGF[i].row_size_V1=dst_row_size_V;
		MT_DataGF[i].dst_V1=(void *)pdst_V;
		MT_DataGF[i].dst_pitch_V1=dst_pitch_V;

		MT_DataGF[i].src_Y_h=src_height_Y;
		MT_DataGF[i].src_U_h=src_height_U;
		MT_DataGF[i].src_V_h=src_height_V;
	}

	uint8_t f_proc;
	const uint8_t offs_16b=(pixelsize==1) ? 0:3;

  if (chroma<5)
  {
	  f_proc=1+offs_16b;

	  for(uint8_t i=0; i<threads_number; i++)
		  MT_ThreadGF[i].f_process=f_proc;

	  if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);
  }
  else
    CopyPlane(src,dst,PLANAR_Y,vi);

  switch (chroma)
  {
  case 0 :
	  if (pixelsize==1)
	  {
		  SetPlane(dst,PLANAR_U,0x80,vi);
		  SetPlane(dst,PLANAR_V,0x80,vi);
	  }
	  else
	  {
		  SetPlane_16(dst,PLANAR_U,0x80 << (bits_per_pixel-8),vi);
		  SetPlane_16(dst,PLANAR_V,0x80 << (bits_per_pixel-8),vi);
	  }
    break;
  case 1 : break;
  case 2 :
    CopyPlane(src,dst,PLANAR_U,vi);
    CopyPlane(src,dst,PLANAR_V,vi);
    break;
  default :
	  f_proc=2+offs_16b;

	  for(uint8_t i=0; i<threads_number; i++)
		  MT_ThreadGF[i].f_process=f_proc;

	  if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);

	  f_proc=3+offs_16b;

	  for(uint8_t i=0; i<threads_number; i++)
		  MT_ThreadGF[i].f_process=f_proc;

	  if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);
    break;
  }

	for(uint8_t i=0; i<threads_number; i++)
		MT_ThreadGF[i].f_process=0;

    poolInterface->ReleaseThreadPool(UserId,sleep,nPool);
  }
  else
  {

  if (chroma<5)
  {
	if (pixelsize==1) Sobel_8(psrc_Y,pdst_Y,src_pitch_Y,dst_pitch_Y,src_height_Y,dst_row_size_Y,thresh);
	else Sobel_16(psrc_Y,pdst_Y,src_pitch_Y,dst_pitch_Y,src_height_Y,dst_row_size_Y,thresh,bits_per_pixel);
  }
  else
    CopyPlane(src,dst,PLANAR_Y,vi);

  switch (chroma)
  {
  case 0 :
	  if (pixelsize==1)
	  {
		  SetPlane(dst,PLANAR_U,0x80,vi);
		  SetPlane(dst,PLANAR_V,0x80,vi);
	  }
	  else
	  {
		  SetPlane_16(dst,PLANAR_U,0x80 << (bits_per_pixel-8),vi);
		  SetPlane_16(dst,PLANAR_V,0x80 << (bits_per_pixel-8),vi);
	  }
    break;
  case 1 : break;
  case 2 :
    CopyPlane(src,dst,PLANAR_U,vi);
    CopyPlane(src,dst,PLANAR_V,vi);
    break;
  default :
	  if (pixelsize==1)
	  {
		  Sobel_8(psrc_U,pdst_U,src_pitch_U,dst_pitch_U,src_height_U,dst_row_size_U,threshC);
		  Sobel_8(psrc_V,pdst_V,src_pitch_V,dst_pitch_V,src_height_V,dst_row_size_V,threshC);
	  }
	  else
	  {
		  Sobel_16(psrc_U,pdst_U,src_pitch_U,dst_pitch_U,src_height_U,dst_row_size_U,threshC,bits_per_pixel);
		  Sobel_16(psrc_V,pdst_V,src_pitch_V,dst_pitch_V,src_height_V,dst_row_size_V,threshC,bits_per_pixel);
	  }
    break;
  }

  }

  return dst;
}


aBlur::aBlur(PClip _child, int _blur_level, int _blur_type, int _chroma, int _blur_levelV,
	int _blur_levelC, int _blur_levelVC,uint8_t _threads,bool _sleep,bool _avsp, IScriptEnvironment *env) :
    GenericVideoFilter(_child), blur_level(_blur_level), blur_type(_blur_type), chroma(_chroma),blur_levelV(_blur_levelV),
		blur_levelC(_blur_levelC),blur_levelVC(_blur_levelVC),sleep(_sleep),threads(_threads),avsp(_avsp)
{
	grey = vi.IsY();
	isRGBPfamily = vi.IsPlanarRGB() || vi.IsPlanarRGBA();
	isAlphaChannel = vi.IsYUVA() || vi.IsPlanarRGBA();

	pixelsize = (uint8_t)vi.ComponentSize(); // AVS16
	bits_per_pixel = (uint8_t)vi.BitsPerComponent();

	UserId=0;

	if (grey) chroma = 1;

	if (pixelsize>2)
	{
		if (threads>1) poolInterface->DeAllocateAllThreads(true);
		env->ThrowError("aBlur: Only 8 -> 16 bits supported");
	}
	if (!(vi.IsYUV() && vi.IsPlanar()))
	{
		if (threads>1) poolInterface->DeAllocateAllThreads(true);
		env->ThrowError("aBlur: Planar YUV input is required");
	}
	if (blur_level<0)
	{
		if (threads>1) poolInterface->DeAllocateAllThreads(true);
		env->ThrowError("aBlur: 'blur' must be >=0");
	}
	if (blur_levelV<0)
	{
		if (threads>1) poolInterface->DeAllocateAllThreads(true);
		env->ThrowError("aBlur: 'blurV' must be >=0");
	}
	if (blur_levelC<0)
	{
		if (threads>1) poolInterface->DeAllocateAllThreads(true);
		env->ThrowError("aBlur: 'blurC' must be >=0");
	}
	if (blur_levelVC<0)
	{
		if (threads>1) poolInterface->DeAllocateAllThreads(true);
		env->ThrowError("aBlur: 'blurVC' must be >=0");
	}
	if ((blur_type<0) || (blur_type>1))
	{
		if (threads>1) poolInterface->DeAllocateAllThreads(true);
		env->ThrowError("aBlur: 'type' must be 0,1");
	}
	if ((chroma<0) || (chroma>6))
	{
		if (threads>1) poolInterface->DeAllocateAllThreads(true);
		env->ThrowError("aBlur: 'chroma' must be 0..6");
	}

	StaticThreadpoolF=StaticThreadpool;

	for (uint8_t i=0; i<MAX_MT_THREADS; i++)
	{
		MT_Thread[i].pClass=this;
		MT_Thread[i].f_process=0;
		MT_Thread[i].thread_Id=(uint8_t)i;
		MT_Thread[i].pFunc=StaticThreadpoolF;
	}

	const int shift_w = (!grey && vi.IsPlanar() && !isRGBPfamily) ? vi.GetPlaneWidthSubsampling(PLANAR_U) : 0;
	const int shift_h = (!grey && vi.IsPlanar() && !isRGBPfamily) ? vi.GetPlaneHeightSubsampling(PLANAR_U) : 0;

	if (vi.height<32) threads_number=1;
	else threads_number=threads;

	threads_number=CreateMTData(MT_Data,threads,threads_number,vi.width,vi.height,shift_w,shift_h);

	if (threads_number>1)
	{
		if (!poolInterface->GetUserId(UserId))
		{
			poolInterface->DeAllocateAllThreads(true);
			env->ThrowError("aBlur: Error with the TheadPool while getting UserId!");
		}
	}

	has_at_least_v8=true;
	try { env->CheckVersion(8); } catch (const AvisynthError&) { has_at_least_v8=false; }
}



aBlur::~aBlur()
{
	if (threads_number>1) poolInterface->RemoveUserId(UserId);
	if (threads>1) poolInterface->DeAllocateAllThreads(true);
}


int __stdcall aBlur::SetCacheHints(int cachehints,int frame_range)
{
  switch (cachehints)
  {
  case CACHE_GET_MTMODE :
    return MT_NICE_FILTER;
  default :
    return 0;
  }
}


void aBlur::StaticThreadpool(void *ptr)
{
	Public_MT_Data_Thread *data=(Public_MT_Data_Thread *)ptr;

	MT_Data_Info_WarpSharp *mt_data_inf=((MT_Data_Info_WarpSharp *)data->pData)+data->thread_Id;
	
	switch(data->f_process)
	{
		case 1 :
			BlurR2_8_MT_H((unsigned char *const)mt_data_inf->src_Y1,(unsigned char *const)mt_data_inf->dst_Y1,
				mt_data_inf->src_pitch_Y1,mt_data_inf->dst_pitch_Y1,mt_data_inf->src_Y_h,mt_data_inf->row_size_Y1,true,
				mt_data_inf->src_Y_h_min,mt_data_inf->src_Y_h_max);
			break;
		case 2 :
			BlurR2_8_MT_V((unsigned char *const)mt_data_inf->src_Y1,(unsigned char *const)mt_data_inf->dst_Y1,
				mt_data_inf->src_pitch_Y1,mt_data_inf->dst_pitch_Y1,mt_data_inf->src_Y_h,mt_data_inf->row_size_Y1,true,
				mt_data_inf->src_Y_h_min,mt_data_inf->src_Y_h_max);
			break;
		case 3 :
			BlurR6_8_MT_H((unsigned char *const)mt_data_inf->src_Y1,(unsigned char *const)mt_data_inf->dst_Y1,
				mt_data_inf->src_pitch_Y1,mt_data_inf->dst_pitch_Y1,mt_data_inf->src_Y_h,mt_data_inf->row_size_Y1,true,
				mt_data_inf->src_Y_h_min,mt_data_inf->src_Y_h_max);
			break;
		case 4 :
			BlurR6_8_MT_V((unsigned char *const)mt_data_inf->src_Y1,(unsigned char *const)mt_data_inf->dst_Y1,
				mt_data_inf->src_pitch_Y1,mt_data_inf->dst_pitch_Y1,mt_data_inf->src_Y_h,mt_data_inf->row_size_Y1,true,
				mt_data_inf->src_Y_h_min,mt_data_inf->src_Y_h_max);
			break;
		case 5 :
			BlurR2_8_MT_H((unsigned char *const)mt_data_inf->src_Y1,(unsigned char *const)mt_data_inf->dst_Y1,
				mt_data_inf->src_pitch_Y1,mt_data_inf->dst_pitch_Y1,mt_data_inf->src_Y_h,mt_data_inf->row_size_Y1,
				mt_data_inf->processH,mt_data_inf->src_Y_h_min,mt_data_inf->src_Y_h_max);
			break;
		case 6 :
			BlurR2_8_MT_V((unsigned char *const)mt_data_inf->src_Y1,(unsigned char *const)mt_data_inf->dst_Y1,
				mt_data_inf->src_pitch_Y1,mt_data_inf->dst_pitch_Y1,mt_data_inf->src_Y_h,mt_data_inf->row_size_Y1,
				mt_data_inf->processV,mt_data_inf->src_Y_h_min,mt_data_inf->src_Y_h_max);
			break;
		case 7 :
			BlurR6_8_MT_H((unsigned char *const)mt_data_inf->src_Y1,(unsigned char *const)mt_data_inf->dst_Y1,
				mt_data_inf->src_pitch_Y1,mt_data_inf->dst_pitch_Y1,mt_data_inf->src_Y_h,mt_data_inf->row_size_Y1,
				mt_data_inf->processH,mt_data_inf->src_Y_h_min,mt_data_inf->src_Y_h_max);
			break;
		case 8 :
			BlurR6_8_MT_V((unsigned char *const)mt_data_inf->src_Y1,(unsigned char *const)mt_data_inf->dst_Y1,
				mt_data_inf->src_pitch_Y1,mt_data_inf->dst_pitch_Y1,mt_data_inf->src_Y_h,mt_data_inf->row_size_Y1,
				mt_data_inf->processV,mt_data_inf->src_Y_h_min,mt_data_inf->src_Y_h_max);
			break;
		case 9 :
			BlurR2_8_MT_H((unsigned char *const)mt_data_inf->src_U1,(unsigned char *const)mt_data_inf->dst_U1,
				mt_data_inf->src_pitch_U1,mt_data_inf->dst_pitch_U1,mt_data_inf->src_U_h,mt_data_inf->row_size_U1,true,
				mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 10 :
			BlurR2_8_MT_V((unsigned char *const)mt_data_inf->src_U1,(unsigned char *const)mt_data_inf->dst_U1,
				mt_data_inf->src_pitch_U1,mt_data_inf->dst_pitch_U1,mt_data_inf->src_U_h,mt_data_inf->row_size_U1,true,
				mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 11 :
			BlurR6_8_MT_H((unsigned char *const)mt_data_inf->src_U1,(unsigned char *const)mt_data_inf->dst_U1,
				mt_data_inf->src_pitch_U1,mt_data_inf->dst_pitch_U1,mt_data_inf->src_U_h,mt_data_inf->row_size_U1,true,
				mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 12 :
			BlurR6_8_MT_V((unsigned char *const)mt_data_inf->src_U1,(unsigned char *const)mt_data_inf->dst_U1,
				mt_data_inf->src_pitch_U1,mt_data_inf->dst_pitch_U1,mt_data_inf->src_U_h,mt_data_inf->row_size_U1,true,
				mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 13 :
			BlurR2_8_MT_H((unsigned char *const)mt_data_inf->src_U1,(unsigned char *const)mt_data_inf->dst_U1,
				mt_data_inf->src_pitch_U1,mt_data_inf->dst_pitch_U1,mt_data_inf->src_U_h,mt_data_inf->row_size_U1,
				mt_data_inf->cprocessH,mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 14 :
			BlurR2_8_MT_V((unsigned char *const)mt_data_inf->src_U1,(unsigned char *const)mt_data_inf->dst_U1,
				mt_data_inf->src_pitch_U1,mt_data_inf->dst_pitch_U1,mt_data_inf->src_U_h,mt_data_inf->row_size_U1,
				mt_data_inf->cprocessV,mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 15 :
			BlurR6_8_MT_H((unsigned char *const)mt_data_inf->src_U1,(unsigned char *const)mt_data_inf->dst_U1,
				mt_data_inf->src_pitch_U1,mt_data_inf->dst_pitch_U1,mt_data_inf->src_U_h,mt_data_inf->row_size_U1,
				mt_data_inf->cprocessH,mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 16 :
			BlurR6_8_MT_V((unsigned char *const)mt_data_inf->src_U1,(unsigned char *const)mt_data_inf->dst_U1,
				mt_data_inf->src_pitch_U1,mt_data_inf->dst_pitch_U1,mt_data_inf->src_U_h,mt_data_inf->row_size_U1,
				mt_data_inf->cprocessV,mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 17 :
			BlurR2_8_MT_H((unsigned char *const)mt_data_inf->src_V1,(unsigned char *const)mt_data_inf->dst_V1,
				mt_data_inf->src_pitch_V1,mt_data_inf->dst_pitch_V1,mt_data_inf->src_V_h,mt_data_inf->row_size_V1,true,
				mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 18 :
			BlurR2_8_MT_V((unsigned char *const)mt_data_inf->src_V1,(unsigned char *const)mt_data_inf->dst_V1,
				mt_data_inf->src_pitch_V1,mt_data_inf->dst_pitch_V1,mt_data_inf->src_V_h,mt_data_inf->row_size_V1,true,
				mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 19 :
			BlurR6_8_MT_H((unsigned char *const)mt_data_inf->src_V1,(unsigned char *const)mt_data_inf->dst_V1,
				mt_data_inf->src_pitch_V1,mt_data_inf->dst_pitch_V1,mt_data_inf->src_V_h,mt_data_inf->row_size_V1,true,
				mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 20 :
			BlurR6_8_MT_V((unsigned char *const)mt_data_inf->src_V1,(unsigned char *const)mt_data_inf->dst_V1,
				mt_data_inf->src_pitch_V1,mt_data_inf->dst_pitch_V1,mt_data_inf->src_V_h,mt_data_inf->row_size_V1,true,
				mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 21 :
			BlurR2_8_MT_H((unsigned char *const)mt_data_inf->src_V1,(unsigned char *const)mt_data_inf->dst_V1,
				mt_data_inf->src_pitch_V1,mt_data_inf->dst_pitch_V1,mt_data_inf->src_V_h,mt_data_inf->row_size_V1,
				mt_data_inf->cprocessH,mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 22 :
			BlurR2_8_MT_V((unsigned char *const)mt_data_inf->src_V1,(unsigned char *const)mt_data_inf->dst_V1,
				mt_data_inf->src_pitch_V1,mt_data_inf->dst_pitch_V1,mt_data_inf->src_V_h,mt_data_inf->row_size_V1,
				mt_data_inf->cprocessV,mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 23 :
			BlurR6_8_MT_H((unsigned char *const)mt_data_inf->src_V1,(unsigned char *const)mt_data_inf->dst_V1,
				mt_data_inf->src_pitch_V1,mt_data_inf->dst_pitch_V1,mt_data_inf->src_V_h,mt_data_inf->row_size_V1,
				mt_data_inf->cprocessH,mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 24 :
			BlurR6_8_MT_V((unsigned char *const)mt_data_inf->src_V1,(unsigned char *const)mt_data_inf->dst_V1,
				mt_data_inf->src_pitch_V1,mt_data_inf->dst_pitch_V1,mt_data_inf->src_V_h,mt_data_inf->row_size_V1,
				mt_data_inf->cprocessV,mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
			// 16 bits
		case 25 :
			BlurR2_16_MT_H((unsigned char *const)mt_data_inf->src_Y1,(unsigned char *const)mt_data_inf->dst_Y1,
				mt_data_inf->src_pitch_Y1,mt_data_inf->dst_pitch_Y1,mt_data_inf->src_Y_h,mt_data_inf->row_size_Y1,true,
				mt_data_inf->src_Y_h_min,mt_data_inf->src_Y_h_max);
			break;
		case 26 :
			BlurR2_16_MT_V((unsigned char *const)mt_data_inf->src_Y1,(unsigned char *const)mt_data_inf->dst_Y1,
				mt_data_inf->src_pitch_Y1,mt_data_inf->dst_pitch_Y1,mt_data_inf->src_Y_h,mt_data_inf->row_size_Y1,true,
				mt_data_inf->src_Y_h_min,mt_data_inf->src_Y_h_max);
			break;
		case 27 :
			BlurR6_16_MT_H((unsigned char *const)mt_data_inf->src_Y1,(unsigned char *const)mt_data_inf->dst_Y1,
				mt_data_inf->src_pitch_Y1,mt_data_inf->dst_pitch_Y1,mt_data_inf->src_Y_h,mt_data_inf->row_size_Y1,true,
				mt_data_inf->src_Y_h_min,mt_data_inf->src_Y_h_max);
			break;
		case 28 :
			BlurR6_16_MT_V((unsigned char *const)mt_data_inf->src_Y1,(unsigned char *const)mt_data_inf->dst_Y1,
				mt_data_inf->src_pitch_Y1,mt_data_inf->dst_pitch_Y1,mt_data_inf->src_Y_h,mt_data_inf->row_size_Y1,true,
				mt_data_inf->src_Y_h_min,mt_data_inf->src_Y_h_max);
			break;
		case 29 :
			BlurR2_16_MT_H((unsigned char *const)mt_data_inf->src_Y1,(unsigned char *const)mt_data_inf->dst_Y1,
				mt_data_inf->src_pitch_Y1,mt_data_inf->dst_pitch_Y1,mt_data_inf->src_Y_h,mt_data_inf->row_size_Y1,
				mt_data_inf->processH,mt_data_inf->src_Y_h_min,mt_data_inf->src_Y_h_max);
			break;
		case 30 :
			BlurR2_16_MT_V((unsigned char *const)mt_data_inf->src_Y1,(unsigned char *const)mt_data_inf->dst_Y1,
				mt_data_inf->src_pitch_Y1,mt_data_inf->dst_pitch_Y1,mt_data_inf->src_Y_h,mt_data_inf->row_size_Y1,
				mt_data_inf->processV,mt_data_inf->src_Y_h_min,mt_data_inf->src_Y_h_max);
			break;
		case 31 :
			BlurR6_16_MT_H((unsigned char *const)mt_data_inf->src_Y1,(unsigned char *const)mt_data_inf->dst_Y1,
				mt_data_inf->src_pitch_Y1,mt_data_inf->dst_pitch_Y1,mt_data_inf->src_Y_h,mt_data_inf->row_size_Y1,
				mt_data_inf->processH,mt_data_inf->src_Y_h_min,mt_data_inf->src_Y_h_max);
			break;
		case 32 :
			BlurR6_16_MT_V((unsigned char *const)mt_data_inf->src_Y1,(unsigned char *const)mt_data_inf->dst_Y1,
				mt_data_inf->src_pitch_Y1,mt_data_inf->dst_pitch_Y1,mt_data_inf->src_Y_h,mt_data_inf->row_size_Y1,
				mt_data_inf->processV,mt_data_inf->src_Y_h_min,mt_data_inf->src_Y_h_max);
			break;
		case 33 :
			BlurR2_16_MT_H((unsigned char *const)mt_data_inf->src_U1,(unsigned char *const)mt_data_inf->dst_U1,
				mt_data_inf->src_pitch_U1,mt_data_inf->dst_pitch_U1,mt_data_inf->src_U_h,mt_data_inf->row_size_U1,true,
				mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 34 :
			BlurR2_16_MT_V((unsigned char *const)mt_data_inf->src_U1,(unsigned char *const)mt_data_inf->dst_U1,
				mt_data_inf->src_pitch_U1,mt_data_inf->dst_pitch_U1,mt_data_inf->src_U_h,mt_data_inf->row_size_U1,true,
				mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 35 :
			BlurR6_16_MT_H((unsigned char *const)mt_data_inf->src_U1,(unsigned char *const)mt_data_inf->dst_U1,
				mt_data_inf->src_pitch_U1,mt_data_inf->dst_pitch_U1,mt_data_inf->src_U_h,mt_data_inf->row_size_U1,true,
				mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 36 :
			BlurR6_16_MT_V((unsigned char *const)mt_data_inf->src_U1,(unsigned char *const)mt_data_inf->dst_U1,
				mt_data_inf->src_pitch_U1,mt_data_inf->dst_pitch_U1,mt_data_inf->src_U_h,mt_data_inf->row_size_U1,true,
				mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 37 :
			BlurR2_16_MT_H((unsigned char *const)mt_data_inf->src_U1,(unsigned char *const)mt_data_inf->dst_U1,
				mt_data_inf->src_pitch_U1,mt_data_inf->dst_pitch_U1,mt_data_inf->src_U_h,mt_data_inf->row_size_U1,
				mt_data_inf->cprocessH,mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 38 :
			BlurR2_16_MT_V((unsigned char *const)mt_data_inf->src_U1,(unsigned char *const)mt_data_inf->dst_U1,
				mt_data_inf->src_pitch_U1,mt_data_inf->dst_pitch_U1,mt_data_inf->src_U_h,mt_data_inf->row_size_U1,
				mt_data_inf->cprocessV,mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 39 :
			BlurR6_16_MT_H((unsigned char *const)mt_data_inf->src_U1,(unsigned char *const)mt_data_inf->dst_U1,
				mt_data_inf->src_pitch_U1,mt_data_inf->dst_pitch_U1,mt_data_inf->src_U_h,mt_data_inf->row_size_U1,
				mt_data_inf->cprocessH,mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 40 :
			BlurR6_16_MT_V((unsigned char *const)mt_data_inf->src_U1,(unsigned char *const)mt_data_inf->dst_U1,
				mt_data_inf->src_pitch_U1,mt_data_inf->dst_pitch_U1,mt_data_inf->src_U_h,mt_data_inf->row_size_U1,
				mt_data_inf->cprocessV,mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 41 :
			BlurR2_16_MT_H((unsigned char *const)mt_data_inf->src_V1,(unsigned char *const)mt_data_inf->dst_V1,
				mt_data_inf->src_pitch_V1,mt_data_inf->dst_pitch_V1,mt_data_inf->src_V_h,mt_data_inf->row_size_V1,true,
				mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 42 :
			BlurR2_16_MT_V((unsigned char *const)mt_data_inf->src_V1,(unsigned char *const)mt_data_inf->dst_V1,
				mt_data_inf->src_pitch_V1,mt_data_inf->dst_pitch_V1,mt_data_inf->src_V_h,mt_data_inf->row_size_V1,true,
				mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 43 :
			BlurR6_16_MT_H((unsigned char *const)mt_data_inf->src_V1,(unsigned char *const)mt_data_inf->dst_V1,
				mt_data_inf->src_pitch_V1,mt_data_inf->dst_pitch_V1,mt_data_inf->src_V_h,mt_data_inf->row_size_V1,true,
				mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 44 :
			BlurR6_16_MT_V((unsigned char *const)mt_data_inf->src_V1,(unsigned char *const)mt_data_inf->dst_V1,
				mt_data_inf->src_pitch_V1,mt_data_inf->dst_pitch_V1,mt_data_inf->src_V_h,mt_data_inf->row_size_V1,true,
				mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 45 :
			BlurR2_16_MT_H((unsigned char *const)mt_data_inf->src_V1,(unsigned char *const)mt_data_inf->dst_V1,
				mt_data_inf->src_pitch_V1,mt_data_inf->dst_pitch_V1,mt_data_inf->src_V_h,mt_data_inf->row_size_V1,
				mt_data_inf->cprocessH,mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 46 :
			BlurR2_16_MT_V((unsigned char *const)mt_data_inf->src_V1,(unsigned char *const)mt_data_inf->dst_V1,
				mt_data_inf->src_pitch_V1,mt_data_inf->dst_pitch_V1,mt_data_inf->src_V_h,mt_data_inf->row_size_V1,
				mt_data_inf->cprocessV,mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 47 :
			BlurR6_16_MT_H((unsigned char *const)mt_data_inf->src_V1,(unsigned char *const)mt_data_inf->dst_V1,
				mt_data_inf->src_pitch_V1,mt_data_inf->dst_pitch_V1,mt_data_inf->src_V_h,mt_data_inf->row_size_V1,
				mt_data_inf->cprocessH,mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 48 :
			BlurR6_16_MT_V((unsigned char *const)mt_data_inf->src_V1,(unsigned char *const)mt_data_inf->dst_V1,
				mt_data_inf->src_pitch_V1,mt_data_inf->dst_pitch_V1,mt_data_inf->src_V_h,mt_data_inf->row_size_V1,
				mt_data_inf->cprocessV,mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		default : ;
	}
}


PVideoFrame __stdcall aBlur::GetFrame(int n, IScriptEnvironment *env)
{
  PVideoFrame src = child->GetFrame(n,env);
  PVideoFrame tmp = (has_at_least_v8)?env->NewVideoFrameP(vi,&src):env->NewVideoFrame(vi,64);
  env->MakeWritable(&src);

  const int32_t src_pitch_Y = src->GetPitch(PLANAR_Y);
  const int32_t tmp_pitch_Y = tmp->GetPitch(PLANAR_Y);
  const int32_t src_pitch_U = src->GetPitch(PLANAR_U);
  const int32_t tmp_pitch_U = tmp->GetPitch(PLANAR_U);
  const int32_t src_pitch_V = src->GetPitch(PLANAR_V);
  const int32_t tmp_pitch_V = tmp->GetPitch(PLANAR_V);

  unsigned char *const wpsrc_Y = src->GetWritePtr(PLANAR_Y);
  unsigned char *const wptmp_Y= tmp->GetWritePtr(PLANAR_Y);
  unsigned char *const wpsrc_U = src->GetWritePtr(PLANAR_U);
  unsigned char *const wptmp_U= tmp->GetWritePtr(PLANAR_U);
  unsigned char *const wpsrc_V = src->GetWritePtr(PLANAR_V);
  unsigned char *const wptmp_V= tmp->GetWritePtr(PLANAR_V);

  const int32_t src_height_Y = src->GetHeight(PLANAR_Y);
  const int32_t src_height_U = src->GetHeight(PLANAR_U);
  const int32_t src_height_V = src->GetHeight(PLANAR_V);

  const int32_t src_row_size_Y = src->GetRowSize(PLANAR_Y);
  const int32_t src_row_size_U = src->GetRowSize(PLANAR_U);
  const int32_t src_row_size_V = src->GetRowSize(PLANAR_V);
 
  const int blurL=std::min(blur_level,blur_levelV);
  const int blurLr=std::max(blur_level,blur_levelV)-blurL;
  const bool processH=blur_level>blurL,processV=blur_levelV>blurL;

  const int cblurL=std::min(blur_levelC,blur_levelVC);
  const int cblurLr=std::max(blur_levelC,blur_levelVC)-cblurL;
  const bool cprocessH=blur_levelC>cblurL,cprocessV=blur_levelVC>cblurL;

  Public_MT_Data_Thread MT_ThreadGF[MAX_MT_THREADS];
  MT_Data_Info_WarpSharp MT_DataGF[MAX_MT_THREADS];

  memcpy(MT_ThreadGF,MT_Thread,sizeof(MT_Thread));
  memcpy(MT_DataGF,MT_Data,sizeof(MT_Data));

  for(uint8_t i=0; i<threads_number; i++)
	MT_ThreadGF[i].pData=MT_DataGF;

  int8_t nPool=-1;

  if (threads_number>1)
  {
	  if ((!poolInterface->RequestThreadPool(UserId,threads_number,MT_ThreadGF,nPool,false,true)) || (nPool==-1))
		  env->ThrowError("aBlur: Error with the TheadPool while requesting threadpool!");
  }

  if (threads_number>1)
  {

	for(uint8_t i=0; i<threads_number; i++)
	{
		MT_DataGF[i].src_Y1=(void *)wpsrc_Y;
		MT_DataGF[i].src_pitch_Y1=src_pitch_Y;
		MT_DataGF[i].row_size_Y1=src_row_size_Y;
		MT_DataGF[i].dst_Y1=(void *)wptmp_Y;
		MT_DataGF[i].dst_pitch_Y1=tmp_pitch_Y;

		MT_DataGF[i].src_U1=(void *)wpsrc_U;
		MT_DataGF[i].src_pitch_U1=src_pitch_U;
		MT_DataGF[i].row_size_U1=src_row_size_U;
		MT_DataGF[i].dst_U1=(void *)wptmp_U;
		MT_DataGF[i].dst_pitch_U1=tmp_pitch_U;

		MT_DataGF[i].src_V1=(void *)wpsrc_V;
		MT_DataGF[i].src_pitch_V1=src_pitch_V;
		MT_DataGF[i].row_size_V1=src_row_size_V;
		MT_DataGF[i].dst_V1=(void *)wptmp_V;
		MT_DataGF[i].dst_pitch_V1=tmp_pitch_V;

		MT_DataGF[i].src_Y_h=src_height_Y;
		MT_DataGF[i].src_U_h=src_height_U;
		MT_DataGF[i].src_V_h=src_height_V;

		MT_DataGF[i].processH=processH;
		MT_DataGF[i].processV=processV;
		MT_DataGF[i].cprocessH=cprocessH;
		MT_DataGF[i].cprocessV=cprocessV;
	}

	  uint8_t f_proc;
	  const uint8_t offs_16b=(pixelsize==1) ? 0:24;

  if (chroma<5)
  {
	  f_proc=(blur_type==1) ? (1+offs_16b):(3+offs_16b);

	for (int i=0; i<blurL; i++)
	{
		for(uint8_t i=0; i<threads_number; i++)
			MT_ThreadGF[i].f_process=f_proc;
		if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);

		for(uint8_t i=0; i<threads_number; i++)
			MT_ThreadGF[i].f_process++;
		if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);
	}

	  f_proc=(blur_type==1) ? (5+offs_16b):(7+offs_16b);

	for (int i=0; i<blurLr; i++)
	{
		for(uint8_t i=0; i<threads_number; i++)
			MT_ThreadGF[i].f_process=f_proc;
		if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);

		for(uint8_t i=0; i<threads_number; i++)
			MT_ThreadGF[i].f_process++;
		if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);
	}
  }
  switch (chroma)
  {
  case 0 :
	  if (pixelsize==1)
	  {
		  SetPlane(src,PLANAR_U,0x80,vi);
		  SetPlane(src,PLANAR_V,0x80,vi);
	  }
	  else
	  {
		  SetPlane_16(src,PLANAR_U,0x80 << (bits_per_pixel-8),vi);
		  SetPlane_16(src,PLANAR_V,0x80 << (bits_per_pixel-8),vi);
	  }
    break;
  case 1 :
  case 2 :
    break;
  default :
	  f_proc=(blur_type==1) ? (9+offs_16b):(11+offs_16b);

	for (int i=0; i<cblurL; i++)
	{
		for(uint8_t i=0; i<threads_number; i++)
			MT_ThreadGF[i].f_process=f_proc;
		if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);

		for(uint8_t i=0; i<threads_number; i++)
			MT_ThreadGF[i].f_process++;
		if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);
	}

	  f_proc=(blur_type==1) ? (13+offs_16b):(15+offs_16b);

	for (int i=0; i<cblurLr; i++)
	{
		for(uint8_t i=0; i<threads_number; i++)
			MT_ThreadGF[i].f_process=f_proc;
		if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);

		for(uint8_t i=0; i<threads_number; i++)
			MT_ThreadGF[i].f_process++;
		if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);
	}

	  f_proc=(blur_type==1) ? (17+offs_16b):(19+offs_16b);

	  for(uint8_t i=0; i<threads_number; i++)
		  MT_ThreadGF[i].f_process=f_proc;

	for (int i=0; i<cblurL; i++)
	{
		for(uint8_t i=0; i<threads_number; i++)
			MT_ThreadGF[i].f_process=f_proc;
		if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);

		for(uint8_t i=0; i<threads_number; i++)
			MT_ThreadGF[i].f_process++;
		if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);
	}

	  f_proc=(blur_type==1) ? (21+offs_16b):(23+offs_16b); // 21 22  23 24

	for (int i=0; i<cblurLr; i++)
	{
		for(uint8_t i=0; i<threads_number; i++)
			MT_ThreadGF[i].f_process=f_proc;
		if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);

		for(uint8_t i=0; i<threads_number; i++)
			MT_ThreadGF[i].f_process++;
		if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);
	}
    break;
  }

	for(uint8_t i=0; i<threads_number; i++)
		MT_ThreadGF[i].f_process=0;

    poolInterface->ReleaseThreadPool(UserId,sleep,nPool);
  }
  else
  {

  if (chroma<5)
  {
	for (int i=0; i<blurL; i++)
	{
		if (pixelsize==1)
		{
			if (blur_type==1) BlurR2_8(wpsrc_Y,wptmp_Y,src_pitch_Y,tmp_pitch_Y,src_height_Y,src_row_size_Y,true,true);
			else BlurR6_8(wpsrc_Y,wptmp_Y,src_pitch_Y,tmp_pitch_Y,src_height_Y,src_row_size_Y,true,true);
		}
		else
		{
			if (blur_type==1) BlurR2_16(wpsrc_Y,wptmp_Y,src_pitch_Y,tmp_pitch_Y,src_height_Y,src_row_size_Y,true,true);
			else BlurR6_16(wpsrc_Y,wptmp_Y,src_pitch_Y,tmp_pitch_Y,src_height_Y,src_row_size_Y,true,true);
		}
	}
	for (int i=0; i<blurLr; i++)
	{
		if (pixelsize==1)
		{
			if (blur_type==1) BlurR2_8(wpsrc_Y,wptmp_Y,src_pitch_Y,tmp_pitch_Y,src_height_Y,src_row_size_Y,processH,processV);
			else BlurR6_8(wpsrc_Y,wptmp_Y,src_pitch_Y,tmp_pitch_Y,src_height_Y,src_row_size_Y,processH,processV);
		}
		else
		{
			if (blur_type==1) BlurR2_16(wpsrc_Y,wptmp_Y,src_pitch_Y,tmp_pitch_Y,src_height_Y,src_row_size_Y,processH,processV);
			else BlurR6_16(wpsrc_Y,wptmp_Y,src_pitch_Y,tmp_pitch_Y,src_height_Y,src_row_size_Y,processH,processV);
		}
	}
  }
  switch (chroma)
  {
  case 0 :
	  if (pixelsize==1)
	  {
		  SetPlane(src,PLANAR_U,0x80,vi);
		  SetPlane(src,PLANAR_V,0x80,vi);
	  }
	  else
	  {
		  SetPlane_16(src,PLANAR_U,0x80 << (bits_per_pixel-8),vi);
		  SetPlane_16(src,PLANAR_V,0x80 << (bits_per_pixel-8),vi);
	  }
    break;
  case 1 :
  case 2 :
    break;
  default :
	for (int i=0; i<cblurL; i++)
	{
		if (pixelsize==1)
		{
			if (blur_type==1) BlurR2_8(wpsrc_U,wptmp_U,src_pitch_U,tmp_pitch_U,src_height_U,src_row_size_U,true,true);
			else BlurR6_8(wpsrc_U,wptmp_U,src_pitch_U,tmp_pitch_U,src_height_U,src_row_size_U,true,true);
		}
		else
		{
			if (blur_type==1) BlurR2_16(wpsrc_U,wptmp_U,src_pitch_U,tmp_pitch_U,src_height_U,src_row_size_U,true,true);
			else BlurR6_16(wpsrc_U,wptmp_U,src_pitch_U,tmp_pitch_U,src_height_U,src_row_size_U,true,true);
		}
	}
	for (int i=0; i<cblurLr; i++)
	{
		if (pixelsize==1)
		{
			if (blur_type==1) BlurR2_8(wpsrc_U,wptmp_U,src_pitch_U,tmp_pitch_U,src_height_U,src_row_size_U,cprocessH,cprocessV);
			else BlurR6_8(wpsrc_U,wptmp_U,src_pitch_U,tmp_pitch_U,src_height_U,src_row_size_U,cprocessH,cprocessV);
		}
		else
		{
			if (blur_type==1) BlurR2_16(wpsrc_U,wptmp_U,src_pitch_U,tmp_pitch_U,src_height_U,src_row_size_U,cprocessH,cprocessV);
			else BlurR6_16(wpsrc_U,wptmp_U,src_pitch_U,tmp_pitch_U,src_height_U,src_row_size_U,cprocessH,cprocessV);
		}
	}
	for (int i=0; i<cblurL; i++)
	{
		if (pixelsize==1)
		{
			if (blur_type==1) BlurR2_8(wpsrc_V,wptmp_V,src_pitch_V,tmp_pitch_V,src_height_V,src_row_size_V,true,true);
			else BlurR6_8(wpsrc_V,wptmp_V,src_pitch_V,tmp_pitch_V,src_height_V,src_row_size_V,true,true);
		}
		else
		{
			if (blur_type==1) BlurR2_16(wpsrc_V,wptmp_V,src_pitch_V,tmp_pitch_V,src_height_V,src_row_size_V,true,true);
			else BlurR6_16(wpsrc_V,wptmp_V,src_pitch_V,tmp_pitch_V,src_height_V,src_row_size_V,true,true);
		}
	}
	for (int i=0; i<cblurLr; i++)
	{
		if (pixelsize==1)
		{
			if (blur_type==1) BlurR2_8(wpsrc_V,wptmp_V,src_pitch_V,tmp_pitch_V,src_height_V,src_row_size_V,cprocessH,cprocessV);
			else BlurR6_8(wpsrc_V,wptmp_V,src_pitch_V,tmp_pitch_V,src_height_V,src_row_size_V,cprocessH,cprocessV);
		}
		else
		{
			if (blur_type==1) BlurR2_16(wpsrc_V,wptmp_V,src_pitch_V,tmp_pitch_V,src_height_V,src_row_size_V,cprocessH,cprocessV);
			else BlurR6_16(wpsrc_V,wptmp_V,src_pitch_V,tmp_pitch_V,src_height_V,src_row_size_V,cprocessH,cprocessV);
		}
	}
    break;
  }

  }

  return src;
}


aWarp::aWarp(PClip _child, PClip _edges, int _depth, int _chroma, int _depthC, bool _cplace_mpeg2_flag,
	int _depthV, int _depthVC,uint8_t _threads,bool _sleep,bool _avsp, IScriptEnvironment *env) :
    GenericVideoFilter(_child), edges(_edges), depth(_depth), chroma(_chroma), depthC(_depthC), cplace_mpeg2_flag(_cplace_mpeg2_flag),
		depthV(_depthV),depthVC(_depthVC),sleep(_sleep),threads(_threads),avsp(_avsp)
{
    const VideoInfo &vi2 = edges->GetVideoInfo();

	grey = vi.IsY();
	isRGBPfamily = vi.IsPlanarRGB() || vi.IsPlanarRGBA();
	isAlphaChannel = vi.IsYUVA() || vi.IsPlanarRGBA();

	pixelsize = (uint8_t)vi.ComponentSize(); // AVS16
	bits_per_pixel = (uint8_t)vi.BitsPerComponent();

	UserId=0;

    if (grey) chroma = 1;

	if (pixelsize>2)
	{
		if (threads>1) poolInterface->DeAllocateAllThreads(true);
		env->ThrowError("aWarp: Only 8 -> 16 bits supported");
	}
	if (!(vi.IsYUV() && vi.IsPlanar() && vi2.IsYUV() && vi2.IsPlanar()))
	{
		if (threads>1) poolInterface->DeAllocateAllThreads(true);
		env->ThrowError("aWarp: Planar YUV input is required");
	}
	if ((depth<-128) || (depth>127))
	{
		if (threads>1) poolInterface->DeAllocateAllThreads(true);
		env->ThrowError("aWarp: 'depth' must be -128..127");
	}
	if ((depthC<-128) || (depthC>127))
	{
		if (threads>1) poolInterface->DeAllocateAllThreads(true);
		env->ThrowError("aWarp: 'depthC' must be -128..127");
	}
	if ((chroma<0) || (chroma>6))
	{
		if (threads>1) poolInterface->DeAllocateAllThreads(true);
		env->ThrowError("aWarp: 'chroma' must be 0..6");
	}
	if ((depthV<-128) || (depthV>127))
	{
		if (threads>1) poolInterface->DeAllocateAllThreads(true);
		env->ThrowError("aWarp: 'depthV' must be -128..127");
	}
	if ((depthVC<-128) || (depthVC>127))
	{
		if (threads>1) poolInterface->DeAllocateAllThreads(true);
		env->ThrowError("aWarp: 'depthVC' must be -128..127");
	}

    if ((vi.width!=vi2.width) || (vi.height!=vi2.height))
	{
		if (threads>1) poolInterface->DeAllocateAllThreads(true);
		env->ThrowError("aWarp: both sources must have the same width and height");
	}
    if (vi.pixel_type!=vi2.pixel_type)
	{
		if (threads>1) poolInterface->DeAllocateAllThreads(true);
		env->ThrowError("aWarp: both sources must have the same colorspace");
	}

	StaticThreadpoolF=StaticThreadpool;

	for (uint8_t i=0; i<MAX_MT_THREADS; i++)
	{
		MT_Thread[i].pClass=this;
		MT_Thread[i].f_process=0;
		MT_Thread[i].thread_Id=(uint8_t)i;
		MT_Thread[i].pFunc=StaticThreadpoolF;
	}

	const int shift_w = (!grey && vi.IsPlanar() && !isRGBPfamily) ? vi.GetPlaneWidthSubsampling(PLANAR_U) : 0;
	const int shift_h = (!grey && vi.IsPlanar() && !isRGBPfamily) ? vi.GetPlaneHeightSubsampling(PLANAR_U) : 0;

	if (vi.height<32) threads_number=1;
	else threads_number=threads;

	threads_number=CreateMTData(MT_Data,threads,threads_number,vi.width,vi.height,shift_w,shift_h);

	if (threads_number>1)
	{
		if (!poolInterface->GetUserId(UserId))
		{
			poolInterface->DeAllocateAllThreads(true);
			env->ThrowError("aWarp: Error with the TheadPool while getting UserId!");
		}
	}

	has_at_least_v8=true;
	try { env->CheckVersion(8); } catch (const AvisynthError&) { has_at_least_v8=false; }
}



aWarp::~aWarp()
{
	if (threads_number>1) poolInterface->RemoveUserId(UserId);
	if (threads>1) poolInterface->DeAllocateAllThreads(true);
}


int __stdcall aWarp::SetCacheHints(int cachehints,int frame_range)
{
  switch (cachehints)
  {
  case CACHE_GET_MTMODE :
    return MT_NICE_FILTER;
  default :
    return 0;
  }
}


void aWarp::StaticThreadpool(void *ptr)
{
	Public_MT_Data_Thread *data=(Public_MT_Data_Thread *)ptr;
	aWarp *ptrClass=(aWarp *)data->pClass;

	MT_Data_Info_WarpSharp *mt_data_inf=((MT_Data_Info_WarpSharp *)data->pData)+data->thread_Id;
	
	switch(data->f_process)
	{
		case 1 :
			Warp0_8_MT((const unsigned char*)mt_data_inf->src_Y1,(const unsigned char*)mt_data_inf->src_Y2,
				(unsigned char*)mt_data_inf->dst_Y1,
				mt_data_inf->src_pitch_Y1,mt_data_inf->src_pitch_Y2,mt_data_inf->dst_pitch_Y1,mt_data_inf->row_size_Y1,
				mt_data_inf->dst_Y_h,ptrClass->depth,ptrClass->depthV,mt_data_inf->src_Y_h_min,mt_data_inf->src_Y_h_max);
			break;
		case 2 :
			Warp0_8_MT((const unsigned char*)mt_data_inf->src_U1,(const unsigned char*)mt_data_inf->src_U2,
				(unsigned char*)mt_data_inf->dst_U2,
				mt_data_inf->src_pitch_U1,mt_data_inf->src_pitch_U2,mt_data_inf->dst_pitch_U1,mt_data_inf->row_size_U1,
				mt_data_inf->dst_U_h,ptrClass->depthC,ptrClass->depthVC,mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 3 :
			Warp0_8_MT((const unsigned char*)mt_data_inf->src_V1,(const unsigned char*)mt_data_inf->src_V2,
				(unsigned char*)mt_data_inf->dst_V1,
				mt_data_inf->src_pitch_V1,mt_data_inf->src_pitch_V2,mt_data_inf->dst_pitch_V1,mt_data_inf->row_size_V1,
				mt_data_inf->dst_U_h,ptrClass->depthC,ptrClass->depthVC,mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 4 :
			GuideChroma_8_MT((const unsigned char*)mt_data_inf->src_Y2,(unsigned char*)mt_data_inf->dst_U1,
				mt_data_inf->src_pitch_Y2,mt_data_inf->src_pitch_U2,mt_data_inf->src_U_h,mt_data_inf->row_size_U2,
				mt_data_inf->SubW_U,mt_data_inf->SubH_U,ptrClass->cplace_mpeg2_flag,
				mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 5 :
			Warp0_8_MT((const unsigned char*)mt_data_inf->src_U1,(const unsigned char*)mt_data_inf->src_U2,
				(unsigned char*)mt_data_inf->dst_U2,
				mt_data_inf->src_pitch_U1,mt_data_inf->src_pitch_U2,mt_data_inf->dst_pitch_U1,mt_data_inf->row_size_U1,
				mt_data_inf->dst_U_h,ptrClass->depthC,ptrClass->depthVC,mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 6 :
			Warp0_8_MT((const unsigned char*)mt_data_inf->src_V1,(const unsigned char*)mt_data_inf->src_U2,
				(unsigned char*)mt_data_inf->dst_V1,
				mt_data_inf->src_pitch_V1,mt_data_inf->src_pitch_U2,mt_data_inf->dst_pitch_V1,mt_data_inf->row_size_V1,
				mt_data_inf->dst_V_h,ptrClass->depthC,ptrClass->depthVC,mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 7 :
			Warp0_8_MT((const unsigned char*)mt_data_inf->src_U1,(const unsigned char*)mt_data_inf->src_Y2,
				(unsigned char*)mt_data_inf->dst_U2,
				mt_data_inf->src_pitch_U1,mt_data_inf->src_pitch_Y2,mt_data_inf->dst_pitch_U1,mt_data_inf->row_size_U1,
				mt_data_inf->dst_U_h,ptrClass->depthC,ptrClass->depthVC,mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 8 :
			Warp0_8_MT((const unsigned char*)mt_data_inf->src_V1,(const unsigned char*)mt_data_inf->src_Y2,
				(unsigned char*)mt_data_inf->dst_V1,
				mt_data_inf->src_pitch_V1,mt_data_inf->src_pitch_Y2,mt_data_inf->dst_pitch_V1,mt_data_inf->row_size_V1,
				mt_data_inf->dst_V_h,ptrClass->depthC,ptrClass->depthVC,mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
			// 16 bits
		case 9 :
			warp0_u16_MT((const unsigned char*)mt_data_inf->src_Y1,(const unsigned char*)mt_data_inf->src_Y2,
				(unsigned char*)mt_data_inf->dst_Y1,mt_data_inf->src_pitch_Y1,mt_data_inf->src_pitch_Y2,mt_data_inf->dst_pitch_Y1,
				mt_data_inf->row_size_Y1 >> 1,mt_data_inf->dst_Y_h,ptrClass->depth,ptrClass->depthV,ptrClass->bits_per_pixel,
				mt_data_inf->src_Y_h_min,mt_data_inf->src_Y_h_max);
			break;
		case 10 :
			warp0_u16_MT((const unsigned char*)mt_data_inf->src_U1,(const unsigned char*)mt_data_inf->src_U2,
				(unsigned char*)mt_data_inf->dst_U2,mt_data_inf->src_pitch_U1,mt_data_inf->src_pitch_U2,mt_data_inf->dst_pitch_U1,
				mt_data_inf->row_size_U1 >> 1,mt_data_inf->dst_U_h,ptrClass->depthC,ptrClass->depthVC,ptrClass->bits_per_pixel,
				mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 11 :
			warp0_u16_MT((const unsigned char*)mt_data_inf->src_V1,(const unsigned char*)mt_data_inf->src_V2,
				(unsigned char*)mt_data_inf->dst_V1,mt_data_inf->src_pitch_V1,mt_data_inf->src_pitch_V2,mt_data_inf->dst_pitch_V1,
				mt_data_inf->row_size_V1 >> 1,mt_data_inf->dst_U_h,ptrClass->depthC,ptrClass->depthVC,ptrClass->bits_per_pixel,
				mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 12 :
			GuideChroma_16_MT((const unsigned char*)mt_data_inf->src_Y2,(unsigned char*)mt_data_inf->dst_U1,
				mt_data_inf->src_pitch_Y2,mt_data_inf->src_pitch_U2,mt_data_inf->src_U_h,mt_data_inf->row_size_U2 >> 1,
				mt_data_inf->SubW_U,mt_data_inf->SubH_U,ptrClass->cplace_mpeg2_flag,
				mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 13 :
			warp0_u16_MT((const unsigned char*)mt_data_inf->src_U1,(const unsigned char*)mt_data_inf->src_U2,
				(unsigned char*)mt_data_inf->dst_U2,mt_data_inf->src_pitch_U1,mt_data_inf->src_pitch_U2,mt_data_inf->dst_pitch_U1,
				mt_data_inf->row_size_U1 >> 1,mt_data_inf->dst_U_h,ptrClass->depthC,ptrClass->depthVC,ptrClass->bits_per_pixel,
				mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 14 :
			warp0_u16_MT((const unsigned char*)mt_data_inf->src_V1,(const unsigned char*)mt_data_inf->src_U2,
				(unsigned char*)mt_data_inf->dst_V1,mt_data_inf->src_pitch_V1,mt_data_inf->src_pitch_U2,mt_data_inf->dst_pitch_V1,
				mt_data_inf->row_size_V1 >> 1,mt_data_inf->dst_V_h,ptrClass->depthC,ptrClass->depthVC,ptrClass->bits_per_pixel,
				mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 15 :
			warp0_u16_MT((const unsigned char*)mt_data_inf->src_U1,(const unsigned char*)mt_data_inf->src_Y2,
				(unsigned char*)mt_data_inf->dst_U2,mt_data_inf->src_pitch_U1,mt_data_inf->src_pitch_Y2,mt_data_inf->dst_pitch_U1,
				mt_data_inf->row_size_U1 >> 1,mt_data_inf->dst_U_h,ptrClass->depthC,ptrClass->depthVC,ptrClass->bits_per_pixel,
				mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 16 :
			warp0_u16_MT((const unsigned char*)mt_data_inf->src_V1,(const unsigned char*)mt_data_inf->src_Y2,
				(unsigned char*)mt_data_inf->dst_V1,mt_data_inf->src_pitch_V1,mt_data_inf->src_pitch_Y2,mt_data_inf->dst_pitch_V1,
				mt_data_inf->row_size_V1 >> 1,mt_data_inf->dst_V_h,ptrClass->depthC,ptrClass->depthVC,ptrClass->bits_per_pixel,
				mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		default : ;
	}
}


PVideoFrame __stdcall aWarp::GetFrame(int n, IScriptEnvironment *env)
{
  PVideoFrame src = child->GetFrame(n, env);
  PVideoFrame edg = edges->GetFrame(n, env);
  PVideoFrame dst = (has_at_least_v8)?env->NewVideoFrameP(vi,&src):env->NewVideoFrame(vi,64);

  const int32_t src_pitch_Y = src->GetPitch(PLANAR_Y);
  const int32_t edg_pitch_Y = edg->GetPitch(PLANAR_Y);
  const int32_t dst_pitch_Y = dst->GetPitch(PLANAR_Y);
  const int32_t src_pitch_U = src->GetPitch(PLANAR_U);
  int32_t edg_pitch_U = edg->GetPitch(PLANAR_U);
  const int32_t dst_pitch_U = dst->GetPitch(PLANAR_U);
  const int32_t src_pitch_V = src->GetPitch(PLANAR_V);
  const int32_t edg_pitch_V = edg->GetPitch(PLANAR_V);
  const int32_t dst_pitch_V = dst->GetPitch(PLANAR_V);

  const unsigned char *psrc_Y = src->GetReadPtr(PLANAR_Y);
  const unsigned char *pedg_Y = edg->GetReadPtr(PLANAR_Y);
  unsigned char *pdst_Y = dst->GetWritePtr(PLANAR_Y);

  const unsigned char *psrc_U = src->GetReadPtr(PLANAR_U);
  const unsigned char *pedg_U = edg->GetReadPtr(PLANAR_U);
  unsigned char *dpedg_U = edg->GetWritePtr(PLANAR_U);
  unsigned char *pdst_U = dst->GetWritePtr(PLANAR_U);

  const unsigned char *psrc_V = src->GetReadPtr(PLANAR_V);
  const unsigned char *pedg_V = edg->GetReadPtr(PLANAR_V);
  unsigned char *pdst_V = dst->GetWritePtr(PLANAR_V);

  const int SubH_U = vi.IsY() ? 0:vi.GetPlaneHeightSubsampling(PLANAR_U);
  const int SubW_U = vi.IsY() ? 0:vi.GetPlaneWidthSubsampling(PLANAR_U);

  const int32_t dst_height_Y = dst->GetHeight(PLANAR_Y);
  const int32_t dst_height_U = dst->GetHeight(PLANAR_U);
  const int32_t dst_height_V = dst->GetHeight(PLANAR_V);
  const int32_t dst_row_size_Y = dst->GetRowSize(PLANAR_Y);
  const int32_t dst_row_size_U = dst->GetRowSize(PLANAR_U);
  const int32_t dst_row_size_V = dst->GetRowSize(PLANAR_V);

  const int32_t edg_height_UV = edg->GetHeight(PLANAR_U);
  const int32_t edg_width_UV = edg->GetRowSize(PLANAR_U);

  Public_MT_Data_Thread MT_ThreadGF[MAX_MT_THREADS];
  MT_Data_Info_WarpSharp MT_DataGF[MAX_MT_THREADS];

  memcpy(MT_ThreadGF,MT_Thread,sizeof(MT_Thread));
  memcpy(MT_DataGF,MT_Data,sizeof(MT_Data));

  for(uint8_t i=0; i<threads_number; i++)
	MT_ThreadGF[i].pData=(void *)MT_DataGF;

  int8_t nPool=-1;

  if (threads_number>1)
  {
	  if ((!poolInterface->RequestThreadPool(UserId,threads_number,MT_ThreadGF,nPool,false,true)) || (nPool==-1))
		  env->ThrowError("aWarp: Error with the TheadPool while requesting threadpool!");
  }

  if (threads_number>1)
  {

	for(uint8_t i=0; i<threads_number; i++)
	{
		MT_DataGF[i].src_Y1=(void *)psrc_Y;
		MT_DataGF[i].src_Y2=(void *)pedg_Y;
		MT_DataGF[i].src_pitch_Y1=src_pitch_Y;
		MT_DataGF[i].src_pitch_Y2=edg_pitch_Y;
		MT_DataGF[i].row_size_Y1=dst_row_size_Y;
		MT_DataGF[i].dst_Y1=(void *)pdst_Y;
		MT_DataGF[i].dst_pitch_Y1=dst_pitch_Y;

		MT_DataGF[i].src_U1=(void *)psrc_U;
		MT_DataGF[i].src_U2=(void *)pedg_U;
		MT_DataGF[i].src_pitch_U1=src_pitch_U;
		MT_DataGF[i].src_pitch_U2=edg_pitch_U;
		MT_DataGF[i].row_size_U1=dst_row_size_U;
		MT_DataGF[i].row_size_U2=edg_width_UV;
		MT_DataGF[i].dst_U1=(void *)dpedg_U;
		MT_DataGF[i].dst_U2=(void *)pdst_U;
		MT_DataGF[i].dst_pitch_U1=dst_pitch_U;

		MT_DataGF[i].src_V1=(void *)psrc_V;
		MT_DataGF[i].src_V2=(void *)pedg_V;
		MT_DataGF[i].src_pitch_V1=src_pitch_V;
		MT_DataGF[i].src_pitch_V2=edg_pitch_V;
		MT_DataGF[i].row_size_V1=dst_row_size_V;
		MT_DataGF[i].dst_V1=(void *)pdst_V;
		MT_DataGF[i].dst_pitch_V1=dst_pitch_V;

		MT_DataGF[i].src_U_h=edg_height_UV;
		MT_DataGF[i].dst_Y_h=dst_height_Y;
		MT_DataGF[i].dst_U_h=dst_height_U;
		MT_DataGF[i].dst_V_h=dst_height_V;

		MT_DataGF[i].SubW_U=SubW_U;
		MT_DataGF[i].SubH_U=SubH_U;
	}

	uint8_t f_proc;
	const uint8_t offs_16b=(pixelsize==1) ? 0:8;

  if ((chroma<5) && ((depth!=0) || (depthV!=0)))
  {
	  f_proc=1+offs_16b;

	  for(uint8_t i=0; i<threads_number; i++)
		  MT_ThreadGF[i].f_process=f_proc;

	  if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);
  }
  else
    CopyPlane(src,dst,PLANAR_Y,vi);

  switch (chroma)
  {
  case 0 :
	  if (pixelsize==1)
	  {
		  SetPlane(dst,PLANAR_U,0x80,vi);
		  SetPlane(dst,PLANAR_V,0x80,vi);
	  }
	  else
	  {
		  SetPlane_16(dst,PLANAR_U,0x80 << (bits_per_pixel-8),vi);
		  SetPlane_16(dst,PLANAR_V,0x80 << (bits_per_pixel-8),vi);
	  }
    break;
  case 1 : break;
  case 2 :
    CopyPlane(src,dst,PLANAR_U,vi);
    CopyPlane(src,dst,PLANAR_V,vi);
    break;
  case 3 :
  case 5 :
	  if ((depthC!=0) || (depthVC!=0))
	  {
		  f_proc=2+offs_16b;

		  for(uint8_t i=0; i<threads_number; i++)
			  MT_ThreadGF[i].f_process=f_proc;

		  if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);

		  f_proc=3+offs_16b;

		  for(uint8_t i=0; i<threads_number; i++)
			  MT_ThreadGF[i].f_process=f_proc;

		  if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);
	  }
	  else
	  {
			CopyPlane(src,dst,PLANAR_U,vi);
			CopyPlane(src,dst,PLANAR_V,vi);
	  }
    break;
  case 4 :
  case 6 :
	   if ((depthC!=0) || (depthVC!=0))
	   {
    if (!vi.Is444())
    {
	  if (!GuideChroma_Test(SubW_U,SubH_U)) env->ThrowError("aWarp: Unsuported colorspace");

      if (edg->IsWritable())
	  {
		  f_proc=4+offs_16b;

		  for(uint8_t i=0; i<threads_number; i++)
			  MT_ThreadGF[i].f_process=f_proc;

		  if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);
	  }
      else
      {
        PVideoFrame tmp = env->NewVideoFrame(vi,64);

		unsigned char *dptmp_U = tmp->GetWritePtr(PLANAR_U);

		edg_pitch_U = tmp->GetPitch(PLANAR_U);
		pedg_U = tmp->GetReadPtr(PLANAR_U);
		
		f_proc=4+offs_16b;

		for(uint8_t i=0; i<threads_number; i++)
		{
			MT_ThreadGF[i].f_process=f_proc;

			MT_DataGF[i].dst_U1=(void *)dptmp_U;
			MT_DataGF[i].src_U2=(void *)pedg_U;
			MT_DataGF[i].src_pitch_U2=edg_pitch_U;
		}

		if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);
      }

	  f_proc=5+offs_16b;

	  for(uint8_t i=0; i<threads_number; i++)
		  MT_ThreadGF[i].f_process=f_proc;

	  if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);

	  f_proc=6+offs_16b;

	  for(uint8_t i=0; i<threads_number; i++)
		  MT_ThreadGF[i].f_process=f_proc;

	  if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);
    }
    else
    {
	  f_proc=7+offs_16b;

	  for(uint8_t i=0; i<threads_number; i++)
		  MT_ThreadGF[i].f_process=f_proc;

	  if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);

	  f_proc=8+offs_16b;

	  for(uint8_t i=0; i<threads_number; i++)
		  MT_ThreadGF[i].f_process=f_proc;

	  if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);
    }
	   }
	   else
	   {
		    CopyPlane(src,dst,PLANAR_U,vi);
			CopyPlane(src,dst,PLANAR_V,vi);
	   }
    break;
  default : break;
  }

	for(uint8_t i=0; i<threads_number; i++)
		MT_ThreadGF[i].f_process=0;

    poolInterface->ReleaseThreadPool(UserId,sleep,nPool);
  }
  else
  {

  if ((chroma<5) && ((depth!=0) || (depthV!=0)))
  {
	  if (pixelsize==1)
		  Warp0_8(psrc_Y,pedg_Y,pdst_Y,src_pitch_Y,edg_pitch_Y,dst_pitch_Y,dst_row_size_Y,dst_height_Y,depth,depthV);
	  else
		  warp0_u16(psrc_Y,pedg_Y,pdst_Y,src_pitch_Y,edg_pitch_Y,dst_pitch_Y,dst_row_size_Y >> 1,dst_height_Y,depth,depthV,bits_per_pixel);
  }
  else
    CopyPlane(src,dst,PLANAR_Y,vi);

  switch (chroma)
  {
  case 0 :
	  if (pixelsize==1)
	  {
		  SetPlane(dst,PLANAR_U,0x80,vi);
		  SetPlane(dst,PLANAR_V,0x80,vi);
	  }
	  else
	  {
		  SetPlane_16(dst,PLANAR_U,0x80 << (bits_per_pixel-8),vi);
		  SetPlane_16(dst,PLANAR_V,0x80 << (bits_per_pixel-8),vi);
	  }
    break;
  case 1 : break;
  case 2 :
    CopyPlane(src,dst,PLANAR_U,vi);
    CopyPlane(src,dst,PLANAR_V,vi);
    break;
  case 3 :
  case 5 :
	  if ((depthC!=0) || (depthVC!=0))
	  {
		  if (pixelsize==1)
		  {
			  Warp0_8(psrc_U,pedg_U,pdst_U,src_pitch_U,edg_pitch_U,dst_pitch_U,dst_row_size_U,dst_height_U,depthC,depthVC);
			  Warp0_8(psrc_V,pedg_V,pdst_V,src_pitch_V,edg_pitch_V,dst_pitch_V,dst_row_size_V,dst_height_V,depthC,depthVC);
		  }
		  else
		  {
			  warp0_u16(psrc_U,pedg_U,pdst_U,src_pitch_U,edg_pitch_U,dst_pitch_U,dst_row_size_U >> 1,dst_height_U,
				  depthC,depthVC,bits_per_pixel);
			  warp0_u16(psrc_V,pedg_V,pdst_V,src_pitch_V,edg_pitch_V,dst_pitch_V,dst_row_size_V >> 1,dst_height_V,
				  depthC,depthVC,bits_per_pixel);
		  }
	  }
	  else
	  {
			CopyPlane(src,dst,PLANAR_U,vi);
			CopyPlane(src,dst,PLANAR_V,vi);
	  }
    break;
  case 4 :
  case 6 :
	   if ((depthC!=0) || (depthVC!=0))
	   {
    if (!vi.Is444())
    {
      if (edg->IsWritable())
	  {
		  const bool TestC = (pixelsize==1) ? GuideChroma_8(pedg_Y,dpedg_U,edg_pitch_Y,edg_pitch_U,edg_height_UV,edg_width_UV,SubW_U,
			  SubH_U,cplace_mpeg2_flag):GuideChroma_16(pedg_Y,dpedg_U,edg_pitch_Y,edg_pitch_U,edg_height_UV,edg_width_UV >> 1,SubW_U,
			  SubH_U,cplace_mpeg2_flag);

		  if (!TestC) env->ThrowError("aWarp: Unsuported colorspace");
	  }
      else
      {
        PVideoFrame tmp = env->NewVideoFrame(vi,64);

		const int32_t tmp_pitch_U = tmp->GetPitch(PLANAR_U);
		unsigned char *dptmp_U = tmp->GetWritePtr(PLANAR_U);
		const int32_t tmp_height_UV = tmp->GetHeight(PLANAR_U);
		const int32_t tmp_width_UV = tmp->GetRowSize(PLANAR_U);

		const bool TestC = (pixelsize==1) ? GuideChroma_8(pedg_Y,dptmp_U,edg_pitch_Y,tmp_pitch_U,tmp_height_UV,tmp_width_UV,SubW_U,
			SubH_U,cplace_mpeg2_flag):GuideChroma_16(pedg_Y,dptmp_U,edg_pitch_Y,tmp_pitch_U,tmp_height_UV,tmp_width_UV >> 1,SubW_U,
			SubH_U,cplace_mpeg2_flag);

		if (!TestC) env->ThrowError("aWarp: Unsuported colorspace");

		pedg_U = tmp->GetReadPtr(PLANAR_U);
		edg_pitch_U = tmp_pitch_U;
      }
	  if (pixelsize==1)
	  {
		  Warp0_8(psrc_U,pedg_U,pdst_U,src_pitch_U,edg_pitch_U,dst_pitch_U,dst_row_size_U,dst_height_U,depthC,depthVC);
		  Warp0_8(psrc_V,pedg_U,pdst_V,src_pitch_V,edg_pitch_U,dst_pitch_V,dst_row_size_V,dst_height_V,depthC,depthVC);
	  }
	  else
	  {
		  warp0_u16(psrc_U,pedg_U,pdst_U,src_pitch_U,edg_pitch_U,dst_pitch_U,dst_row_size_U >> 1,dst_height_U,
			  depthC,depthVC,bits_per_pixel);
		  warp0_u16(psrc_V,pedg_U,pdst_V,src_pitch_V,edg_pitch_U,dst_pitch_V,dst_row_size_V >> 1,dst_height_V,
			  depthC,depthVC,bits_per_pixel);
	  }
    }
    else
    {
		if (pixelsize==1)
		{
			Warp0_8(psrc_U,pedg_Y,pdst_U,src_pitch_U,edg_pitch_Y,dst_pitch_U,dst_row_size_U,dst_height_U,depthC,depthVC);
			Warp0_8(psrc_V,pedg_Y,pdst_V,src_pitch_V,edg_pitch_Y,dst_pitch_V,dst_row_size_V,dst_height_V,depthC,depthVC);
		}
		else
		{
			warp0_u16(psrc_U,pedg_Y,pdst_U,src_pitch_U,edg_pitch_Y,dst_pitch_U,dst_row_size_U >> 1,dst_height_U,
				depthC,depthVC,bits_per_pixel);
			warp0_u16(psrc_V,pedg_Y,pdst_V,src_pitch_V,edg_pitch_Y,dst_pitch_V,dst_row_size_V >> 1,dst_height_V,
				depthC,depthVC,bits_per_pixel);
		}
    }
	   }
	   else
	   {
		    CopyPlane(src,dst,PLANAR_U,vi);
			CopyPlane(src,dst,PLANAR_V,vi);
	   }
    break;
  default : break;
  }

  }

  return dst;
}


aWarp4::aWarp4(PClip _child, PClip _edges, int _depth, int _chroma, int _depthC, bool _cplace_mpeg2_flag,
	int _depthV, int _depthVC,uint8_t _threads,bool _sleep,bool _avsp, IScriptEnvironment *env) :
    GenericVideoFilter(_child), edges(_edges), depth(_depth), chroma(_chroma), depthC(_depthC), cplace_mpeg2_flag(_cplace_mpeg2_flag),
		depthV(_depthV),depthVC(_depthVC),sleep(_sleep),threads(_threads),avsp(_avsp)
{
    const VideoInfo &vi2 = edges->GetVideoInfo();

	grey = vi.IsY();
	isRGBPfamily = vi.IsPlanarRGB() || vi.IsPlanarRGBA();
	isAlphaChannel = vi.IsYUVA() || vi.IsPlanarRGBA();

	pixelsize = (uint8_t)vi.ComponentSize(); // AVS16
	bits_per_pixel = (uint8_t)vi.BitsPerComponent();

	UserId=0;

    if (grey) chroma = 1;

	if (pixelsize>2)
	{
		if (threads>1) poolInterface->DeAllocateAllThreads(true);
		env->ThrowError("aWarp4: Only 8 -> 16 bits supported");
	}
	if (!(vi.IsYUV() && vi.IsPlanar() && vi2.IsYUV() && vi2.IsPlanar()))
	{
		if (threads>1) poolInterface->DeAllocateAllThreads(true);
		env->ThrowError("aWarp4: Planar YUV input is required");
	}
	if ((depth<-128) || (depth>127))
	{
		if (threads>1) poolInterface->DeAllocateAllThreads(true);
		env->ThrowError("aWarp4: 'depth' must be -128..127");
	}
	if ((depthC<-128) || (depthC>127))
	{
		if (threads>1) poolInterface->DeAllocateAllThreads(true);
		env->ThrowError("aWarp4: 'depthC' must be -128..127");
	}
	if ((chroma<0) || (chroma>6))
	{
		if (threads>1) poolInterface->DeAllocateAllThreads(true);
		env->ThrowError("aWarp4: 'chroma' must be 0..6");
	}
	if ((depthV<-128) || (depthV>127))
	{
		if (threads>1) poolInterface->DeAllocateAllThreads(true);
		env->ThrowError("aWarp4: 'depthV' must be -128..127");
	}
	if ((depthVC<-128) || (depthVC>127))
	{
		if (threads>1) poolInterface->DeAllocateAllThreads(true);
		env->ThrowError("aWarp4: 'depthVC' must be -128..127");
	}

    if ((vi.width!=(vi2.width<<2)) || (vi.height!=(vi2.height<<2)))
	{
		if (threads>1) poolInterface->DeAllocateAllThreads(true);
		env->ThrowError("aWarp4: first source must be excatly 4 times width and height of second source");
	}
    if (vi.pixel_type!=vi2.pixel_type)
	{
		if (threads>1) poolInterface->DeAllocateAllThreads(true);
		env->ThrowError("aWarp4: both sources must have the colorspace");
	}

    vi=vi2;

	StaticThreadpoolF=StaticThreadpool;

	for (uint8_t i=0; i<MAX_MT_THREADS; i++)
	{
		MT_Thread[i].pClass=this;
		MT_Thread[i].f_process=0;
		MT_Thread[i].thread_Id=(uint8_t)i;
		MT_Thread[i].pFunc=StaticThreadpoolF;
	}

	const int shift_w = (!grey && vi.IsPlanar() && !isRGBPfamily) ? vi.GetPlaneWidthSubsampling(PLANAR_U) : 0;
	const int shift_h = (!grey && vi.IsPlanar() && !isRGBPfamily) ? vi.GetPlaneHeightSubsampling(PLANAR_U) : 0;

	if (vi.height<32) threads_number=1;
	else threads_number=threads;

	threads_number=CreateMTData(MT_Data,threads,threads_number,vi.width,vi.height,shift_w,shift_h);

	if (threads_number>1)
	{
		if (!poolInterface->GetUserId(UserId))
		{
			poolInterface->DeAllocateAllThreads(true);
			env->ThrowError("aWarp4: Error with the TheadPool while getting UserId!");
		}
	}

	has_at_least_v8=true;
	try { env->CheckVersion(8); } catch (const AvisynthError&) { has_at_least_v8=false; }
}


aWarp4::~aWarp4()
{
	if (threads_number>1) poolInterface->RemoveUserId(UserId);
	if (threads>1) poolInterface->DeAllocateAllThreads(true);
}


int __stdcall aWarp4::SetCacheHints(int cachehints,int frame_range)
{
  switch (cachehints)
  {
  case CACHE_GET_MTMODE :
    return MT_NICE_FILTER;
  default :
    return 0;
  }
}


void aWarp4::StaticThreadpool(void *ptr)
{
	Public_MT_Data_Thread *data=(Public_MT_Data_Thread *)ptr;
	aWarp4 *ptrClass=(aWarp4 *)data->pClass;

	MT_Data_Info_WarpSharp *mt_data_inf=((MT_Data_Info_WarpSharp *)data->pData)+data->thread_Id;
	
	switch(data->f_process)
	{
		case 1 :
			Warp2_8_MT((const unsigned char*)mt_data_inf->src_Y1,(const unsigned char*)mt_data_inf->src_Y2,
				(unsigned char*)mt_data_inf->dst_Y1,
				mt_data_inf->src_pitch_Y1,mt_data_inf->src_pitch_Y2,mt_data_inf->dst_pitch_Y1,mt_data_inf->row_size_Y1,
				mt_data_inf->dst_Y_h,ptrClass->depth,ptrClass->depthV,mt_data_inf->src_Y_h_min,mt_data_inf->src_Y_h_max);
			break;
		case 2 :
			Warp2_8_MT((const unsigned char*)mt_data_inf->src_U1,(const unsigned char*)mt_data_inf->src_U2,
				(unsigned char*)mt_data_inf->dst_U2,
				mt_data_inf->src_pitch_U1,mt_data_inf->src_pitch_U2,mt_data_inf->dst_pitch_U1,mt_data_inf->row_size_U1,
				mt_data_inf->dst_U_h,ptrClass->depthC,ptrClass->depthVC,mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 3 :
			Warp2_8_MT((const unsigned char*)mt_data_inf->src_V1,(const unsigned char*)mt_data_inf->src_V2,
				(unsigned char*)mt_data_inf->dst_V1,
				mt_data_inf->src_pitch_V1,mt_data_inf->src_pitch_V2,mt_data_inf->dst_pitch_V1,mt_data_inf->row_size_V1,
				mt_data_inf->dst_U_h,ptrClass->depthC,ptrClass->depthVC,mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 4 :
			GuideChroma_8_MT((const unsigned char*)mt_data_inf->src_Y2,(unsigned char*)mt_data_inf->dst_U1,
				mt_data_inf->src_pitch_Y2,mt_data_inf->src_pitch_U2,mt_data_inf->src_U_h,mt_data_inf->row_size_U2,
				mt_data_inf->SubW_U,mt_data_inf->SubH_U,ptrClass->cplace_mpeg2_flag,
				mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 5 :
			Warp2_8_MT((const unsigned char*)mt_data_inf->src_U1,(const unsigned char*)mt_data_inf->src_U2,
				(unsigned char*)mt_data_inf->dst_U2,
				mt_data_inf->src_pitch_U1,mt_data_inf->src_pitch_U2,mt_data_inf->dst_pitch_U1,mt_data_inf->row_size_U1,
				mt_data_inf->dst_U_h,ptrClass->depthC,ptrClass->depthVC,mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 6 :
			Warp2_8_MT((const unsigned char*)mt_data_inf->src_V1,(const unsigned char*)mt_data_inf->src_U2,
				(unsigned char*)mt_data_inf->dst_V1,
				mt_data_inf->src_pitch_V1,mt_data_inf->src_pitch_U2,mt_data_inf->dst_pitch_V1,mt_data_inf->row_size_V1,
				mt_data_inf->dst_V_h,ptrClass->depthC,ptrClass->depthVC,mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 7 :
			Warp2_8_MT((const unsigned char*)mt_data_inf->src_U1,(const unsigned char*)mt_data_inf->src_Y2,
				(unsigned char*)mt_data_inf->dst_U2,
				mt_data_inf->src_pitch_U1,mt_data_inf->src_pitch_Y2,mt_data_inf->dst_pitch_U1,mt_data_inf->row_size_U1,
				mt_data_inf->dst_U_h,ptrClass->depthC,ptrClass->depthVC,mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 8 :
			Warp2_8_MT((const unsigned char*)mt_data_inf->src_V1,(const unsigned char*)mt_data_inf->src_Y2,
				(unsigned char*)mt_data_inf->dst_V1,
				mt_data_inf->src_pitch_V1,mt_data_inf->src_pitch_Y2,mt_data_inf->dst_pitch_V1,mt_data_inf->row_size_V1,
				mt_data_inf->dst_V_h,ptrClass->depthC,ptrClass->depthVC,mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
			// 16 bits
		case 9 :
			warp2_u16_MT((const unsigned char*)mt_data_inf->src_Y1,(const unsigned char*)mt_data_inf->src_Y2,
				(unsigned char*)mt_data_inf->dst_Y1,mt_data_inf->src_pitch_Y1,mt_data_inf->src_pitch_Y2,mt_data_inf->dst_pitch_Y1,
				mt_data_inf->row_size_Y1 >> 1,mt_data_inf->dst_Y_h,ptrClass->depth,ptrClass->depthV,ptrClass->bits_per_pixel,
				mt_data_inf->src_Y_h_min,mt_data_inf->src_Y_h_max);
			break;
		case 10 :
			warp2_u16_MT((const unsigned char*)mt_data_inf->src_U1,(const unsigned char*)mt_data_inf->src_U2,
				(unsigned char*)mt_data_inf->dst_U2,mt_data_inf->src_pitch_U1,mt_data_inf->src_pitch_U2,mt_data_inf->dst_pitch_U1,
				mt_data_inf->row_size_U1 >> 1,mt_data_inf->dst_U_h,ptrClass->depthC,ptrClass->depthVC,ptrClass->bits_per_pixel,
				mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 11 :
			warp2_u16_MT((const unsigned char*)mt_data_inf->src_V1,(const unsigned char*)mt_data_inf->src_V2,
				(unsigned char*)mt_data_inf->dst_V1,mt_data_inf->src_pitch_V1,mt_data_inf->src_pitch_V2,mt_data_inf->dst_pitch_V1,
				mt_data_inf->row_size_V1 >> 1,mt_data_inf->dst_U_h,ptrClass->depthC,ptrClass->depthVC,ptrClass->bits_per_pixel,
				mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 12 :
			GuideChroma_16_MT((const unsigned char*)mt_data_inf->src_Y2,(unsigned char*)mt_data_inf->dst_U1,
				mt_data_inf->src_pitch_Y2,mt_data_inf->src_pitch_U2,mt_data_inf->src_U_h,mt_data_inf->row_size_U2 >> 1,
				mt_data_inf->SubW_U,mt_data_inf->SubH_U,ptrClass->cplace_mpeg2_flag,
				mt_data_inf->dst_UV_h_min,mt_data_inf->dst_UV_h_max);
			break;
		case 13 :
			warp2_u16_MT((const unsigned char*)mt_data_inf->src_U1,(const unsigned char*)mt_data_inf->src_U2,
				(unsigned char*)mt_data_inf->dst_U2,mt_data_inf->src_pitch_U1,mt_data_inf->src_pitch_U2,mt_data_inf->dst_pitch_U1,
				mt_data_inf->row_size_U1 >> 1,mt_data_inf->dst_U_h,ptrClass->depthC,ptrClass->depthVC,ptrClass->bits_per_pixel,
				mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 14 :
			warp2_u16_MT((const unsigned char*)mt_data_inf->src_V1,(const unsigned char*)mt_data_inf->src_U2,
				(unsigned char*)mt_data_inf->dst_V1,mt_data_inf->src_pitch_V1,mt_data_inf->src_pitch_U2,mt_data_inf->dst_pitch_V1,
				mt_data_inf->row_size_V1 >> 1,mt_data_inf->dst_V_h,ptrClass->depthC,ptrClass->depthVC,ptrClass->bits_per_pixel,
				mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 15 :
			warp2_u16_MT((const unsigned char*)mt_data_inf->src_U1,(const unsigned char*)mt_data_inf->src_Y2,
				(unsigned char*)mt_data_inf->dst_U2,mt_data_inf->src_pitch_U1,mt_data_inf->src_pitch_Y2,mt_data_inf->dst_pitch_U1,
				mt_data_inf->row_size_U1 >> 1,mt_data_inf->dst_U_h,ptrClass->depthC,ptrClass->depthVC,ptrClass->bits_per_pixel,
				mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		case 16 :
			warp2_u16_MT((const unsigned char*)mt_data_inf->src_V1,(const unsigned char*)mt_data_inf->src_Y2,
				(unsigned char*)mt_data_inf->dst_V1,mt_data_inf->src_pitch_V1,mt_data_inf->src_pitch_Y2,mt_data_inf->dst_pitch_V1,
				mt_data_inf->row_size_V1 >> 1,mt_data_inf->dst_V_h,ptrClass->depthC,ptrClass->depthVC,ptrClass->bits_per_pixel,
				mt_data_inf->src_UV_h_min,mt_data_inf->src_UV_h_max);
			break;
		default : ;
	}
}


PVideoFrame __stdcall aWarp4::GetFrame(int n, IScriptEnvironment *env)
{
  PVideoFrame src = child->GetFrame(n, env);
  PVideoFrame edg = edges->GetFrame(n, env);
  PVideoFrame dst = (has_at_least_v8)?env->NewVideoFrameP(vi,&src):env->NewVideoFrame(vi,64);

  const int32_t src_pitch_Y = src->GetPitch(PLANAR_Y);
  const int32_t edg_pitch_Y = edg->GetPitch(PLANAR_Y);
  const int32_t dst_pitch_Y = dst->GetPitch(PLANAR_Y);
  const int32_t src_pitch_U = src->GetPitch(PLANAR_U);
  int32_t edg_pitch_U = edg->GetPitch(PLANAR_U);
  const int32_t dst_pitch_U = dst->GetPitch(PLANAR_U);
  const int32_t src_pitch_V = src->GetPitch(PLANAR_V);
  const int32_t edg_pitch_V = edg->GetPitch(PLANAR_V);
  const int32_t dst_pitch_V = dst->GetPitch(PLANAR_V);

  const unsigned char *psrc_Y = src->GetReadPtr(PLANAR_Y);
  const unsigned char *pedg_Y = edg->GetReadPtr(PLANAR_Y);
  unsigned char *pdst_Y = dst->GetWritePtr(PLANAR_Y);

  const unsigned char *psrc_U = src->GetReadPtr(PLANAR_U);
  const unsigned char *pedg_U = edg->GetReadPtr(PLANAR_U);
  unsigned char *dpedg_U = edg->GetWritePtr(PLANAR_U);
  unsigned char *pdst_U = dst->GetWritePtr(PLANAR_U);

  const unsigned char *psrc_V = src->GetReadPtr(PLANAR_V);
  const unsigned char *pedg_V = edg->GetReadPtr(PLANAR_V);
  unsigned char *pdst_V = dst->GetWritePtr(PLANAR_V);

  const int SubH_U = vi.IsY() ? 0:vi.GetPlaneHeightSubsampling(PLANAR_U);
  const int SubW_U = vi.IsY() ? 0:vi.GetPlaneWidthSubsampling(PLANAR_U);

  const int32_t dst_height_Y = dst->GetHeight(PLANAR_Y);
  const int32_t dst_height_U = dst->GetHeight(PLANAR_U);
  const int32_t dst_height_V = dst->GetHeight(PLANAR_V);

  const int32_t dst_row_size_Y = dst->GetRowSize(PLANAR_Y);
  const int32_t dst_row_size_U = dst->GetRowSize(PLANAR_U);
  const int32_t dst_row_size_V = dst->GetRowSize(PLANAR_V);

  const int32_t edg_height_UV = edg->GetHeight(PLANAR_U);
  const int32_t edg_width_UV = edg->GetRowSize(PLANAR_U);

  Public_MT_Data_Thread MT_ThreadGF[MAX_MT_THREADS];
  MT_Data_Info_WarpSharp MT_DataGF[MAX_MT_THREADS];

  memcpy(MT_ThreadGF,MT_Thread,sizeof(MT_Thread));
  memcpy(MT_DataGF,MT_Data,sizeof(MT_Data));

  for(uint8_t i=0; i<threads_number; i++)
	MT_ThreadGF[i].pData=MT_DataGF;

  int8_t nPool=-1;

  if (threads_number>1)
  {
	  if ((!poolInterface->RequestThreadPool(UserId,threads_number,MT_ThreadGF,nPool,false,true)) || (nPool==-1))
		  env->ThrowError("aWarp4: Error with the TheadPool while requesting threadpool!");
  }

  if (threads_number>1)
  {

	for(uint8_t i=0; i<threads_number; i++)
	{
		MT_DataGF[i].src_Y1=(void *)psrc_Y;
		MT_DataGF[i].src_Y2=(void *)pedg_Y;
		MT_DataGF[i].src_pitch_Y1=src_pitch_Y;
		MT_DataGF[i].src_pitch_Y2=edg_pitch_Y;
		MT_DataGF[i].row_size_Y1=dst_row_size_Y;
		MT_DataGF[i].dst_Y1=(void *)pdst_Y;
		MT_DataGF[i].dst_pitch_Y1=dst_pitch_Y;

		MT_DataGF[i].src_U1=(void *)psrc_U;
		MT_DataGF[i].src_U2=(void *)pedg_U;
		MT_DataGF[i].src_pitch_U1=src_pitch_U;
		MT_DataGF[i].src_pitch_U2=edg_pitch_U;
		MT_DataGF[i].row_size_U1=dst_row_size_U;
		MT_DataGF[i].row_size_U2=edg_width_UV;
		MT_DataGF[i].dst_U1=(void *)dpedg_U;
		MT_DataGF[i].dst_U2=(void *)pdst_U;
		MT_DataGF[i].dst_pitch_U1=dst_pitch_U;

		MT_DataGF[i].src_V1=(void *)psrc_V;
		MT_DataGF[i].src_V2=(void *)pedg_V;
		MT_DataGF[i].src_pitch_V1=src_pitch_V;
		MT_DataGF[i].src_pitch_V2=edg_pitch_V;
		MT_DataGF[i].row_size_V1=dst_row_size_V;
		MT_DataGF[i].dst_V1=(void *)pdst_V;
		MT_DataGF[i].dst_pitch_V1=dst_pitch_V;

		MT_DataGF[i].src_U_h=edg_height_UV;
		MT_DataGF[i].dst_Y_h=dst_height_Y;
		MT_DataGF[i].dst_U_h=dst_height_U;
		MT_DataGF[i].dst_V_h=dst_height_V;

		MT_DataGF[i].SubW_U=SubW_U;
		MT_DataGF[i].SubH_U=SubH_U;
	}

	uint8_t f_proc;
	const uint8_t offs_16b=(pixelsize==1) ? 0:8;

  if (chroma<5)
  {
	  f_proc=1+offs_16b;

	  for(uint8_t i=0; i<threads_number; i++)
		  MT_ThreadGF[i].f_process=f_proc;

	  if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);
  }

  switch (chroma)
  {
  case 0 :
	  if (pixelsize==1)
	  {
		  SetPlane(dst,PLANAR_U,0x80,vi);
		  SetPlane(dst,PLANAR_V,0x80,vi);
	  }
	  else
	  {
		  SetPlane_16(dst,PLANAR_U,0x80 << (bits_per_pixel-8),vi);
		  SetPlane_16(dst,PLANAR_V,0x80 << (bits_per_pixel-8),vi);
	  }
    break;
  case 1 :
  case 2 :
    break;
  case 3 :
  case 5 :
		  f_proc=2+offs_16b;

		  for(uint8_t i=0; i<threads_number; i++)
			  MT_ThreadGF[i].f_process=f_proc;

		  if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);

		  f_proc=3+offs_16b;

		  for(uint8_t i=0; i<threads_number; i++)
			  MT_ThreadGF[i].f_process=f_proc;

		  if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);
    break;
  case 4 :
  case 6 :
    if (!vi.Is444())
    {
	  if (!GuideChroma_Test(SubW_U,SubH_U)) env->ThrowError("aWarp4: Unsuported colorspace");

      if (edg->IsWritable())
	  {
		  f_proc=4+offs_16b;

		  for(uint8_t i=0; i<threads_number; i++)
			  MT_ThreadGF[i].f_process=f_proc;

		  if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);
	  }
      else
      {
        PVideoFrame tmp = env->NewVideoFrame(vi,64);

		unsigned char *dptmp_U = tmp->GetWritePtr(PLANAR_U);

		edg_pitch_U = tmp->GetPitch(PLANAR_U);
		pedg_U = tmp->GetReadPtr(PLANAR_U);
		
		f_proc=4+offs_16b;

		for(uint8_t i=0; i<threads_number; i++)
		{
			MT_ThreadGF[i].f_process=f_proc;

			MT_DataGF[i].dst_U1=dptmp_U;
			MT_DataGF[i].src_U2=(void *)pedg_U;
			MT_DataGF[i].src_pitch_U2=edg_pitch_U;
		}

		if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);
      }

	  f_proc=5+offs_16b;

	  for(uint8_t i=0; i<threads_number; i++)
		  MT_ThreadGF[i].f_process=f_proc;

	  if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);

	  f_proc=6+offs_16b;

	  for(uint8_t i=0; i<threads_number; i++)
		  MT_ThreadGF[i].f_process=f_proc;

	  if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);
    }
    else
    {
	  f_proc=7+offs_16b;

	  for(uint8_t i=0; i<threads_number; i++)
		  MT_ThreadGF[i].f_process=f_proc;

	  if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);

	  f_proc=8+offs_16b;

	  for(uint8_t i=0; i<threads_number; i++)
		  MT_ThreadGF[i].f_process=f_proc;

	  if (poolInterface->StartThreads(UserId,nPool)) poolInterface->WaitThreadsEnd(UserId,nPool);
    }
    break;
  default : break;
  }

	for(uint8_t i=0; i<threads_number; i++)
		MT_ThreadGF[i].f_process=0;

    poolInterface->ReleaseThreadPool(UserId,sleep,nPool);
  }
  else
  {

  if (chroma<5)
  {
	  if (pixelsize==1)
		  Warp2_8(psrc_Y,pedg_Y,pdst_Y,src_pitch_Y,edg_pitch_Y,dst_pitch_Y,dst_row_size_Y,dst_height_Y,depth,depthV);
	  else
		  warp2_u16(psrc_Y,pedg_Y,pdst_Y,src_pitch_Y,edg_pitch_Y,dst_pitch_Y,dst_row_size_Y >> 1,dst_height_Y,
			depth,depthV,bits_per_pixel);
  }

  switch (chroma)
  {
  case 0 :
	  if (pixelsize==1)
	  {
		  SetPlane(dst,PLANAR_U,0x80,vi);
		  SetPlane(dst,PLANAR_V,0x80,vi);
	  }
	  else
	  {
		  SetPlane_16(dst,PLANAR_U,0x80 << (bits_per_pixel-8),vi);
		  SetPlane_16(dst,PLANAR_V,0x80 << (bits_per_pixel-8),vi);
	  }
    break;
  case 1 :
  case 2 :
    break;
  case 3 :
  case 5 :
	  if (pixelsize==1)
	  {
		  Warp2_8(psrc_U,pedg_U,pdst_U,src_pitch_U,edg_pitch_U,dst_pitch_U,dst_row_size_U,dst_height_U,depthC,depthVC);
		  Warp2_8(psrc_V,pedg_V,pdst_V,src_pitch_V,edg_pitch_V,dst_pitch_V,dst_row_size_V,dst_height_V,depthC,depthVC);
	  }
	  else
	  {
		  warp2_u16(psrc_U,pedg_U,pdst_U,src_pitch_U,edg_pitch_U,dst_pitch_U,dst_row_size_U >> 1,dst_height_U,
			  depthC,depthVC,bits_per_pixel);
		  warp2_u16(psrc_V,pedg_V,pdst_V,src_pitch_V,edg_pitch_V,dst_pitch_V,dst_row_size_V >> 1,dst_height_V,
			  depthC,depthVC,bits_per_pixel);
	  }
    break;
  case 4 :
  case 6 :
    if (!vi.Is444())
    {
      if (edg->IsWritable())
	  {
		  const bool TestC = (pixelsize==1) ? GuideChroma_8(pedg_Y,dpedg_U,edg_pitch_Y,edg_pitch_U,edg_height_UV,edg_width_UV,SubW_U,
			  SubH_U,cplace_mpeg2_flag):GuideChroma_16(pedg_Y,dpedg_U,edg_pitch_Y,edg_pitch_U,edg_height_UV,edg_width_UV >> 1,SubW_U,
			  SubH_U,cplace_mpeg2_flag);

		  if (!TestC) env->ThrowError("aWarp4: Unsuported colorspace");
	  }
      else
      {
        PVideoFrame tmp = env->NewVideoFrame(vi,64);

		const int32_t tmp_pitch_U = tmp->GetPitch(PLANAR_U);
		unsigned char *dptmp_U = tmp->GetWritePtr(PLANAR_U);
		const int32_t tmp_height_UV = tmp->GetHeight(PLANAR_U);
		const int32_t tmp_width_UV = tmp->GetRowSize(PLANAR_U);

		const bool TestC = (pixelsize==1) ? GuideChroma_8(pedg_Y,dptmp_U,edg_pitch_Y,tmp_pitch_U,tmp_height_UV,tmp_width_UV,SubW_U,
			SubH_U,cplace_mpeg2_flag):GuideChroma_16(pedg_Y,dptmp_U,edg_pitch_Y,tmp_pitch_U,tmp_height_UV,tmp_width_UV >> 1,SubW_U,
			SubH_U,cplace_mpeg2_flag);

		if (!TestC) env->ThrowError("aWarp4: Unsuported colorspace");

		pedg_U = tmp->GetReadPtr(PLANAR_U);
		edg_pitch_U = tmp_pitch_U;
      }
	  if (pixelsize==1)
	  {
		  Warp2_8(psrc_U,pedg_U,pdst_U,src_pitch_U,edg_pitch_U,dst_pitch_U,dst_row_size_U,dst_height_U,depthC,depthVC);
		  Warp2_8(psrc_V,pedg_U,pdst_V,src_pitch_V,edg_pitch_U,dst_pitch_V,dst_row_size_V,dst_height_V,depthC,depthVC);
	  }
	  else
	  {
		  warp2_u16(psrc_U,pedg_U,pdst_U,src_pitch_U,edg_pitch_U,dst_pitch_U,dst_row_size_U >> 1,dst_height_U,
			  depthC,depthVC,bits_per_pixel);
		  warp2_u16(psrc_V,pedg_U,pdst_V,src_pitch_V,edg_pitch_U,dst_pitch_V,dst_row_size_V >> 1,dst_height_V,
			  depthC,depthVC,bits_per_pixel);
	  }
    }
    else
    {
		 if (pixelsize==1)
		 {
			 Warp2_8(psrc_U,pedg_Y,pdst_U,src_pitch_U,edg_pitch_Y,dst_pitch_U,dst_row_size_U,dst_height_U,depthC,depthVC);
			 Warp2_8(psrc_V,pedg_Y,pdst_V,src_pitch_V,edg_pitch_Y,dst_pitch_V,dst_row_size_V,dst_height_V,depthC,depthVC);
		 }
		 else
		 {
			 warp2_u16(psrc_U,pedg_Y,pdst_U,src_pitch_U,edg_pitch_Y,dst_pitch_U,dst_row_size_U >> 1,dst_height_U,
				 depthC,depthVC,bits_per_pixel);
			 warp2_u16(psrc_V,pedg_Y,pdst_V,src_pitch_V,edg_pitch_Y,dst_pitch_V,dst_row_size_V >> 1,dst_height_V,
				 depthC,depthVC,bits_per_pixel);
		 }
    }
    break;
  default : break;
  }

  }

  return dst;
}


static bool is_cplace_mpeg2(const AVSValue &args, int pos)
{
  const char *cplace_0=args[pos].AsString("");
  const bool cplace_mpeg2_flag=(_stricmp(cplace_0, "MPEG2")==0);
  return (cplace_mpeg2_flag);
}


AVSValue __cdecl Create_aWarpSharp(AVSValue args, void *user_data, IScriptEnvironment *env)
{
	int threads,prefetch,thread_level;
	bool LogicalCores,MaxPhysCores,SetAffinity,sleep;

	uint8_t threads_number=1;

	if (!args[0].IsClip()) env->ThrowError("aWarpSharpMT: arg 0 must be a clip!");
	VideoInfo vi = args[0].AsClip()->GetVideoInfo();

	const bool avsp=env->FunctionExists("ConvertBits");

	int thresh,blur,blurt,depth,depthC,blurV,depthV,depthVC,blurC,blurVC,threshC;

	const ThreadLevelName TabLevel[8]={NoneThreadLevel,IdleThreadLevel,LowestThreadLevel,
		BelowThreadLevel,NormalThreadLevel,AboveThreadLevel,HighestThreadLevel,CriticalThreadLevel};

  switch ((int)(size_t)user_data)
  {
  case 0 :
	  {
	  if (!aWarpSharp_Enable_SSE2) env->ThrowError("aWarpSharp2: SSE2 capable CPU is required");

	  threads=args[14].AsInt(0);
	  LogicalCores=args[15].AsBool(true);
	  MaxPhysCores=args[16].AsBool(true);
	  SetAffinity=args[17].AsBool(false);
	  sleep = args[18].AsBool(false);
	  prefetch=args[19].AsInt(0);
	  thread_level=args[20].AsInt(6);

	  if ((threads<0) || (threads>MAX_MT_THREADS))
		  env->ThrowError("aWarpSharp2: [threads] must be between 0 and %ld.",MAX_MT_THREADS);
	  if (prefetch==0) prefetch=1;
	  if ((prefetch<0) || (prefetch>MAX_THREAD_POOL))
		  env->ThrowError("aWarpSharp2: [prefetch] must be between 0 and %d.",MAX_THREAD_POOL);
	if ((thread_level<1) || (thread_level>7))
		env->ThrowError("aWarpSharp2: [ThreadLevel] must be between 1 and 7.");

	  if (threads!=1)
	  {
		  if (!poolInterface->CreatePool(prefetch)) env->ThrowError("aWarpSharp2: Unable to create ThreadPool!");

		  threads_number=poolInterface->GetThreadNumber(threads,LogicalCores);

		  if (threads_number==0) env->ThrowError("aWarpSharp2: Error with the TheadPool while getting CPU info!");

		  if (threads_number>1)
		  {
			  if (prefetch>1)
			  {
				  if (SetAffinity && (prefetch<=poolInterface->GetPhysicalCoreNumber()))
				  {
					  float delta=(float)poolInterface->GetPhysicalCoreNumber()/(float)prefetch,Offset=0.0f;

					  for(uint8_t i=0; i<prefetch; i++)
					  {
						  if (!poolInterface->AllocateThreads(threads_number,(uint8_t)ceil(Offset),0,
							  MaxPhysCores,true,true,TabLevel[thread_level],i))
						  {
							  poolInterface->DeAllocateAllThreads(true);
							  env->ThrowError("aWarpSharp2: Error with the TheadPool while allocating threadpool!");
						  }
						  Offset+=delta;
					  }
				  }
				  else
				  {
					  if (!poolInterface->AllocateThreads(threads_number,0,0,MaxPhysCores,false,true,TabLevel[thread_level],-1))
					  {
						  poolInterface->DeAllocateAllThreads(true);
						  env->ThrowError("aWarpSharp2: Error with the TheadPool while allocating threadpool!");
					  }
				  }
			  }
			  else
			  {
				  if (!poolInterface->AllocateThreads(threads_number,0,0,MaxPhysCores,SetAffinity,true,TabLevel[thread_level],-1))
				  {
					  poolInterface->DeAllocateAllThreads(true);
					  env->ThrowError("aWarpSharp2: Error with the TheadPool while allocating threadpool!");
				  }
			  }
		  }
	  }

	  thresh=args[1].AsInt(0x80);
	  blurt=args[3].AsInt(0);
	  args[2].Defined() ? blur=args[2].AsInt(-1) : blur=((blurt==0) ? 2:3);
	  depth=args[4].AsInt(16);
	  args[6].Defined() ? depthC=args[6].AsInt(128) : depthC=(vi.Is444() ? depth:(depth>>1));
	  args[8].Defined() ? blurV=args[8].AsInt(-1) : blurV=blur;
	  args[9].Defined() ? depthV=args[9].AsInt(128) : depthV=depth;
	  args[10].Defined() ? depthVC=args[10].AsInt(128) : depthVC=depthC;
	  args[11].Defined() ? blurC=args[11].AsInt(-1) : blurC=(blur+1)>>1;
	  args[12].Defined() ? blurVC=args[12].AsInt(-1) : blurVC=blurC;
	  args[13].Defined() ? threshC=args[13].AsInt(-1) : threshC=thresh;

    return new aWarpSharp(args[0].AsClip(),thresh,blur,blurt,depth,args[5].AsInt(4),depthC,is_cplace_mpeg2(args,7),
		blurV,depthV,depthVC,blurC,blurVC,threshC,threads_number,sleep,avsp,env);
	break;
	  }
  case 1 :
	  {
	  if (!aWarpSharp_Enable_SSE2) env->ThrowError("aWarpSharp: SSE2 capable CPU is required");

    blurt = (args[5].AsInt(2)!=2)?1:0;
    const int blurlevel = args[2].AsInt(2);
    const unsigned int cm = args[4].AsInt(1);
    static const char map[4] = {1,4,3,2};

	  threads=args[7].AsInt(0);
	  LogicalCores=args[8].AsBool(true);
	  MaxPhysCores=args[9].AsBool(true);
	  SetAffinity=args[10].AsBool(false);
	  sleep = args[11].AsBool(false);
	  prefetch=args[12].AsInt(0);
	  thread_level=args[13].AsInt(6);

	  if ((threads<0) || (threads>MAX_MT_THREADS))
		  env->ThrowError("aWarpSharp: [threads] must be between 0 and %ld.",MAX_MT_THREADS);
	  if (prefetch==0) prefetch=1;
	  if ((prefetch<0) || (prefetch>MAX_THREAD_POOL))
		  env->ThrowError("aWarpSharp: [prefetch] must be between 0 and %d.",MAX_THREAD_POOL);
	if ((thread_level<1) || (thread_level>7))
		env->ThrowError("aWarpSharp: [ThreadLevel] must be between 1 and 7.");

	  if (threads!=1)
	  {
		  if (!poolInterface->CreatePool(prefetch)) env->ThrowError("aWarpSharp: Unable to create ThreadPool!");

		  threads_number=poolInterface->GetThreadNumber(threads,LogicalCores);

		  if (threads_number==0) env->ThrowError("aWarpSharp: Error with the TheadPool while getting CPU info!");

		  if (threads_number>1)
		  {
			  if (prefetch>1)
			  {
				  if (SetAffinity && (prefetch<=poolInterface->GetPhysicalCoreNumber()))
				  {
					  float delta=(float)poolInterface->GetPhysicalCoreNumber()/(float)prefetch,Offset=0.0f;

					  for(uint8_t i=0; i<prefetch; i++)
					  {
						  if (!poolInterface->AllocateThreads(threads_number,(uint8_t)ceil(Offset),0,
							  MaxPhysCores,true,true,TabLevel[thread_level],i))
						  {
							  poolInterface->DeAllocateAllThreads(true);
							  env->ThrowError("aWarpSharp: Error with the TheadPool while allocating threadpool!");
						  }
						  Offset+=delta;
					  }
				  }
				  else
				  {
					  if (!poolInterface->AllocateThreads(threads_number,0,0,MaxPhysCores,false,true,TabLevel[thread_level],-1))
					  {
						  poolInterface->DeAllocateAllThreads(true);
						  env->ThrowError("aWarpSharp: Error with the TheadPool while allocating threadpool!");
					  }
				  }
			  }
			  else
			  {
				  if (!poolInterface->AllocateThreads(threads_number,0,0,MaxPhysCores,SetAffinity,true,TabLevel[thread_level],-1))
				  {
					  poolInterface->DeAllocateAllThreads(true);
					  env->ThrowError("aWarpSharp: Error with the TheadPool while allocating threadpool!");
				  }
			  }
		  }
	  }

	  thresh=int(args[3].AsFloat(0.5)*256.0);
	  blur=(blurt==1)?(blurlevel*3):blurlevel;
	  depth=int(args[1].AsFloat(16.0)*blurlevel*0.5);
	  depthC=vi.Is444() ? depth:(depth>>1);
	  blurV=blur;
	  depthV=depth;
	  depthVC=depthC;
	  blurC=(blur+1)>>1;
	  blurVC=blurC;
	  threshC=thresh;

    return new aWarpSharp(args[0].AsClip(),thresh,blur,blurt,depth,(cm<4)?map[cm]:-1,depthC,false,
		blurV,depthV,depthVC,blurC,blurVC,threshC,threads_number,sleep,avsp,env);
	break;
	  }
  case 2 :
	  {

	  threads=args[4].AsInt(0);
	  LogicalCores=args[5].AsBool(true);
	  MaxPhysCores=args[6].AsBool(true);
	  SetAffinity=args[7].AsBool(false);
	  sleep = args[8].AsBool(false);
	  prefetch=args[9].AsInt(0);
	  thread_level=args[10].AsInt(6);

	  if (!aWarpSharp_Enable_SSE2) env->ThrowError("aSobel: SSE2 capable CPU is required");

	  if ((threads<0) || (threads>MAX_MT_THREADS))
		  env->ThrowError("aSobel: [threads] must be between 0 and %ld.",MAX_MT_THREADS);
	  if (prefetch==0) prefetch=1;
	  if ((prefetch<0) || (prefetch>MAX_THREAD_POOL))
		  env->ThrowError("aSobel: [prefetch] must be between 0 and %d.",MAX_THREAD_POOL);
	if ((thread_level<1) || (thread_level>7))
		env->ThrowError("aSobel: [ThreadLevel] must be between 1 and 7.");

	  if (threads!=1)
	  {
		  if (!poolInterface->CreatePool(prefetch)) env->ThrowError("aSobel: Unable to create ThreadPool!");

		  threads_number=poolInterface->GetThreadNumber(threads,LogicalCores);

		  if (threads_number==0) env->ThrowError("aSobel: Error with the TheadPool while getting CPU info!");

		  if (threads_number>1)
		  {
			  if (prefetch>1)
			  {
				  if (SetAffinity && (prefetch<=poolInterface->GetPhysicalCoreNumber()))
				  {
					  float delta=(float)poolInterface->GetPhysicalCoreNumber()/(float)prefetch,Offset=0.0f;

					  for(uint8_t i=0; i<prefetch; i++)
					  {
						  if (!poolInterface->AllocateThreads(threads_number,(uint8_t)ceil(Offset),0,
							  MaxPhysCores,true,true,TabLevel[thread_level],i))
						  {
							  poolInterface->DeAllocateAllThreads(true);
							  env->ThrowError("aSobel: Error with the TheadPool while allocating threadpool!");
						  }
						  Offset+=delta;
					  }
				  }
				  else
				  {
					  if (!poolInterface->AllocateThreads(threads_number,0,0,MaxPhysCores,false,true,TabLevel[thread_level],-1))
					  {
						  poolInterface->DeAllocateAllThreads(true);
						  env->ThrowError("aSobel: Error with the TheadPool while allocating threadpool!");
					  }
				  }
			  }
			  else
			  {
				  if (!poolInterface->AllocateThreads(threads_number,0,0,MaxPhysCores,SetAffinity,true,TabLevel[thread_level],-1))
				  {
					  poolInterface->DeAllocateAllThreads(true);
					  env->ThrowError("aSobel: Error with the TheadPool while allocating threadpool!");
				  }
			  }
		  }
	  }

	  thresh=args[1].AsInt(0x80);
	  args[3].Defined() ? threshC=args[3].AsInt(-1) : threshC=thresh;

	return new aSobel(args[0].AsClip(),thresh,args[2].AsInt(1),threshC,threads_number,sleep,avsp,env);
	break;
	  }
  case 3 :
	  {

	  threads=args[7].AsInt(0);
	  LogicalCores=args[8].AsBool(true);
	  MaxPhysCores=args[9].AsBool(true);
	  SetAffinity=args[10].AsBool(false);
	  sleep = args[11].AsBool(false);
	  prefetch=args[12].AsInt(0);
	  thread_level=args[13].AsInt(6);

	  if (!aWarpSharp_Enable_SSE2) env->ThrowError("aBlur: SSE2 capable CPU is required");

	  if ((threads<0) || (threads>MAX_MT_THREADS))
		  env->ThrowError("aBlur: [threads] must be between 0 and %ld.",MAX_MT_THREADS);
	  if (prefetch==0) prefetch=1;
	  if ((prefetch<0) || (prefetch>MAX_THREAD_POOL))
		  env->ThrowError("aBlur: [prefetch] must be between 0 and %d.",MAX_THREAD_POOL);
	if ((thread_level<1) || (thread_level>7))
		env->ThrowError("aBlur: [ThreadLevel] must be between 1 and 7.");

	  if (threads!=1)
	  {
		  if (!poolInterface->CreatePool(prefetch)) env->ThrowError("aBlur: Unable to create ThreadPool!");

		  threads_number=poolInterface->GetThreadNumber(threads,LogicalCores);

		  if (threads_number==0) env->ThrowError("aBlur: Error with the TheadPool while getting CPU info!");

		  if (threads_number>1)
		  {
			  if (prefetch>1)
			  {
				  if (SetAffinity && (prefetch<=poolInterface->GetPhysicalCoreNumber()))
				  {
					  float delta=(float)poolInterface->GetPhysicalCoreNumber()/(float)prefetch,Offset=0.0f;

					  for(uint8_t i=0; i<prefetch; i++)
					  {
						  if (!poolInterface->AllocateThreads(threads_number,(uint8_t)ceil(Offset),0,
							  MaxPhysCores,true,true,TabLevel[thread_level],i))
						  {
							  poolInterface->DeAllocateAllThreads(true);
							  env->ThrowError("aBlur: Error with the TheadPool while allocating threadpool!");
						  }
						  Offset+=delta;
					  }
				  }
				  else
				  {
					  if (!poolInterface->AllocateThreads(threads_number,0,0,MaxPhysCores,false,true,TabLevel[thread_level],-1))
					  {
						  poolInterface->DeAllocateAllThreads(true);
						  env->ThrowError("aBlur: Error with the TheadPool while allocating threadpool!");
					  }
				  }
			  }
			  else
			  {
				  if (!poolInterface->AllocateThreads(threads_number,0,0,MaxPhysCores,SetAffinity,true,TabLevel[thread_level],-1))
				  {
					  poolInterface->DeAllocateAllThreads(true);
					  env->ThrowError("aBlur: Error with the TheadPool while allocating threadpool!");
				  }
			  }
		  }
	  }

	  blurt=args[2].AsInt(1);
	  args[1].Defined() ? blur=args[1].AsInt(-1) : blur=((blurt==0) ? 2:3);
	  args[4].Defined() ? blurV=args[4].AsInt(-1) : blurV=blur;
	  args[5].Defined() ? blurC=args[5].AsInt(-1) : blurC=(blur+1)>>1;
	  args[6].Defined() ? blurVC=args[6].AsInt(-1) : blurVC=blurC;

    return new aBlur(args[0].AsClip(),blur,blurt,args[3].AsInt(1),blurV,blurC,blurVC,threads_number,sleep,avsp,env);
	break;
	  }
  case 4 :
	  {

	  threads=args[8].AsInt(0);
	  LogicalCores=args[9].AsBool(true);
	  MaxPhysCores=args[10].AsBool(true);
	  SetAffinity=args[11].AsBool(false);
	  sleep = args[12].AsBool(false);
	  prefetch=args[13].AsInt(0);
	  thread_level=args[14].AsInt(6);

	  if (!aWarpSharp_Enable_SSE2) env->ThrowError("aWarp: SSE2 capable CPU is required");

	  if ((threads<0) || (threads>MAX_MT_THREADS))
		  env->ThrowError("aWarp: [threads] must be between 0 and %ld.",MAX_MT_THREADS);
	  if (prefetch==0) prefetch=1;
	  if ((prefetch<0) || (prefetch>MAX_THREAD_POOL))
		  env->ThrowError("aWarp: [prefetch] must be between 0 and %d.",MAX_THREAD_POOL);
	if ((thread_level<1) || (thread_level>7))
		env->ThrowError("aWarp: [ThreadLevel] must be between 1 and 7.");

	  if (threads!=1)
	  {
		  if (!poolInterface->CreatePool(prefetch)) env->ThrowError("aWarp: Unable to create ThreadPool!");

		  threads_number=poolInterface->GetThreadNumber(threads,LogicalCores);

		  if (threads_number==0) env->ThrowError("aWarp: Error with the TheadPool while getting CPU info!");

		  if (threads_number>1)
		  {
			  if (prefetch>1)
			  {
				  if (SetAffinity && (prefetch<=poolInterface->GetPhysicalCoreNumber()))
				  {
					  float delta=(float)poolInterface->GetPhysicalCoreNumber()/(float)prefetch,Offset=0.0f;

					  for(uint8_t i=0; i<prefetch; i++)
					  {
						  if (!poolInterface->AllocateThreads(threads_number,(uint8_t)ceil(Offset),0,
							  MaxPhysCores,true,true,TabLevel[thread_level],i))
						  {
							  poolInterface->DeAllocateAllThreads(true);
							  env->ThrowError("aWarp: Error with the TheadPool while allocating threadpool!");
						  }
						  Offset+=delta;
					  }
				  }
				  else
				  {
					  if (!poolInterface->AllocateThreads(threads_number,0,0,MaxPhysCores,false,true,TabLevel[thread_level],-1))
					  {
						  poolInterface->DeAllocateAllThreads(true);
						  env->ThrowError("aWarp: Error with the TheadPool while allocating threadpool!");
					  }
				  }
			  }
			  else
			  {
				  if (!poolInterface->AllocateThreads(threads_number,0,0,MaxPhysCores,SetAffinity,true,TabLevel[thread_level],-1))
				  {
					  poolInterface->DeAllocateAllThreads(true);
					  env->ThrowError("aWarp: Error with the TheadPool while allocating threadpool!");
				  }
			  }
		  }
	  }

	  depth=args[2].AsInt(3);
	  args[4].Defined() ? depthC=args[4].AsInt(128) : depthC=(vi.Is444() ? depth:(depth>>1));
	  args[6].Defined() ? depthV=args[6].AsInt(128) : depthV=depth;
	  args[7].Defined() ? depthVC=args[7].AsInt(128) : depthVC=depthC;

    return new aWarp(args[0].AsClip(),args[1].AsClip(),depth,args[3].AsInt(4),depthC,is_cplace_mpeg2(args,5),
		depthV,depthVC,threads_number,sleep,avsp,env);
	break;
	  }
  case 5 :
	  {
	  threads=args[8].AsInt(0);
	  LogicalCores=args[9].AsBool(true);
	  MaxPhysCores=args[10].AsBool(true);
	  SetAffinity=args[11].AsBool(false);
	  sleep = args[12].AsBool(false);
	  prefetch=args[13].AsInt(0);
	  thread_level=args[14].AsInt(6);

	  if (!aWarpSharp_Enable_SSE2) env->ThrowError("aWarp4: SSE2 capable CPU is required");

	  if ((threads<0) || (threads>MAX_MT_THREADS))
		  env->ThrowError("aWarp4: [threads] must be between 0 and %ld.",MAX_MT_THREADS);
	  if (prefetch==0) prefetch=1;
	  if ((prefetch<0) || (prefetch>MAX_THREAD_POOL))
		  env->ThrowError("aWarp4: [prefetch] must be between 0 and %d.",MAX_THREAD_POOL);
	if ((thread_level<1) || (thread_level>7))
		env->ThrowError("aWarp4: [ThreadLevel] must be between 1 and 7.");

	  if (threads!=1)
	  {
		  if (!poolInterface->CreatePool(prefetch)) env->ThrowError("aWarp4: Unable to create ThreadPool!");

		  threads_number=poolInterface->GetThreadNumber(threads,LogicalCores);

		  if (threads_number==0) env->ThrowError("aWarp4: Error with the TheadPool while getting CPU info!");

		  if (threads_number>1)
		  {
			  if (prefetch>1)
			  {
				  if (SetAffinity && (prefetch<=poolInterface->GetPhysicalCoreNumber()))
				  {
					  float delta=(float)poolInterface->GetPhysicalCoreNumber()/(float)prefetch,Offset=0.0f;

					  for(uint8_t i=0; i<prefetch; i++)
					  {
						  if (!poolInterface->AllocateThreads(threads_number,(uint8_t)ceil(Offset),0,
							  MaxPhysCores,true,true,TabLevel[thread_level],i))
						  {
							  poolInterface->DeAllocateAllThreads(true);
							  env->ThrowError("aWarp4: Error with the TheadPool while allocating threadpool!");
						  }
						  Offset+=delta;
					  }
				  }
				  else
				  {
					  if (!poolInterface->AllocateThreads(threads_number,0,0,MaxPhysCores,false,true,TabLevel[thread_level],-1))
					  {
						  poolInterface->DeAllocateAllThreads(true);
						  env->ThrowError("aWarp4: Error with the TheadPool while allocating threadpool!");
					  }
				  }
			  }
			  else
			  {
				  if (!poolInterface->AllocateThreads(threads_number,0,0,MaxPhysCores,SetAffinity,true,TabLevel[thread_level],-1))
				  {
					  poolInterface->DeAllocateAllThreads(true);
					  env->ThrowError("aWarp4: Error with the TheadPool while allocating threadpool!");
				  }
			  }
		  }
	  }

	  depth=args[2].AsInt(3);
	  args[4].Defined() ? depthC=args[4].AsInt(128) : depthC=(vi.Is444() ? depth:(depth>>1));
	  args[6].Defined() ? depthV=args[6].AsInt(128) : depthV=depth;
	  args[7].Defined() ? depthVC=args[7].AsInt(128) : depthVC=depthC;

    return new aWarp4(args[0].AsClip(),args[1].AsClip(),depth,args[3].AsInt(4),depthC,is_cplace_mpeg2(args,5),
		depthV,depthVC,threads_number,sleep,avsp,env);
	break;
	  }
  default : break;
  }
  return NULL;
}

// thresh: 0..255
// blur:   0..?
// type:   0..1
// depth:  -128..127
// chroma modes:
// 0 - zero
// 1 - don't care
// 2 - copy
// 3 - process
// 4 - guide by luma - warp only
// remap from MarcFD's aWarpSharp: thresh=_thresh*256, blur=_blurlevel, type= (bm=0)->1, (bm=2)->0, depth=_depth*_blurlevel/2, chroma= 0->2, 1->4, 2->3

/* New 2.6 requirement!!! */
// Declare and initialise server pointers static storage.
const AVS_Linkage *AVS_linkage = 0;

/* New 2.6 requirement!!! */
// DLL entry point called from LoadPlugin() to setup a user plugin.
extern "C" __declspec(dllexport) const char* __stdcall
AvisynthPluginInit3(IScriptEnvironment* env, const AVS_Linkage* const vectors)
{

  /* New 2.6 requirment!!! */
  // Save the server pointers.
  AVS_linkage = vectors;

  poolInterface=ThreadPoolInterface::Init(0);

  aWarpSharp_Enable_SSE2=(env->GetCPUFlags() & CPUF_SSE2)!=0;
  aWarpSharp_Enable_SSE41=(env->GetCPUFlags() & CPUF_SSE4_1)!=0;
  aWarpSharp_Enable_AVX=(env->GetCPUFlags() & CPUF_AVX)!=0;

  env->AddFunction("aWarpSharp2", "c[thresh]i[blur]i[type]i[depth]i[chroma]i[depthC]i[cplace]s[blurV]i[depthV]i[depthVC]i" \
	  "[blurC]i[blurVC]i[threshC]i[threads]i[logicalCores]b[MaxPhysCore]b[SetAffinity]b[sleep]b[prefetch]i[ThreadLevel]i", Create_aWarpSharp, (void*)0);
  env->AddFunction("aWarpSharp", "c[depth]f[blurlevel]i[thresh]f[cm]i[bm]i[show]b" \
	  "[threads]i[logicalCores]b[MaxPhysCore]b[SetAffinity]b[sleep]b[prefetch]i[ThreadLevel]i", Create_aWarpSharp, (void*)1);
  env->AddFunction("aSobel", "c[thresh]i[chroma]i[threshC]i" \
	  "[threads]i[logicalCores]b[MaxPhysCore]b[SetAffinity]b[sleep]b[prefetch]i[ThreadLevel]i", Create_aWarpSharp, (void*)2);
  env->AddFunction("aBlur", "c[blur]i[type]i[chroma]i[blurV]i[blurC]i[blurVC]i" \
	  "[threads]i[logicalCores]b[MaxPhysCore]b[SetAffinity]b[sleep]b[prefetch]i[ThreadLevel]i", Create_aWarpSharp, (void*)3);
  env->AddFunction("aWarp", "cc[depth]i[chroma]i[depthC]i[cplace]s[depthV]i[depthVC]i" \
	  "[threads]i[logicalCores]b[MaxPhysCore]b[SetAffinity]b[sleep]b[prefetch]i[ThreadLevel]i", Create_aWarpSharp, (void*)4);
  env->AddFunction("aWarp4", "cc[depth]i[chroma]i[depthC]i[cplace]s[depthV]i[depthVC]i" \
	  "[threads]i[logicalCores]b[MaxPhysCore]b[SetAffinity]b[sleep]b[prefetch]i[ThreadLevel]i", Create_aWarpSharp, (void*)5);

  return AWARPSHARP_VERSION;
}
