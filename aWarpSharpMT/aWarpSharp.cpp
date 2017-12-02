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

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <algorithm>
#include <stdint.h>
#include <windows.h>
#include "avisynth.h"
#include "aWarpSharp.h"

static int aWarpSharp_g_cpuid;

static ThreadPoolInterface *poolInterface;

extern "C" void JPSDR_Warp2_8_SSE2(const unsigned char *psrc,const unsigned char *pedg,unsigned char *pdst,int32_t src_pitch,
	int32_t edg_pitchp,int32_t edg_pitchn,int32_t y_limit_min,int32_t y_limit_max,const short *x_limit_min,const short *x_limit_max,
	int32_t i_,int32_t depthH,int32_t depthV);
extern "C" void JPSDR_Warp2_8_SSE3(const unsigned char *psrc,const unsigned char *pedg,unsigned char *pdst,int32_t src_pitch,
	int32_t edg_pitchp,int32_t edg_pitchn,int32_t y_limit_min,int32_t y_limit_max,const short *x_limit_min,const short *x_limit_max,
	int32_t i_,int32_t depthH,int32_t depthV);
extern "C" void JPSDR_Warp0_8_SSE2(const unsigned char *psrc,const unsigned char *pedg,unsigned char *pdst,int32_t src_pitch,
	int32_t edg_pitchp,int32_t edg_pitchn,int32_t y_limit_min,int32_t y_limit_max,const short *x_limit_min,const short *x_limit_max,
	int32_t i_,uint32_t depthH,int32_t depthV);
extern "C" void JPSDR_Warp0_8_SSE3(const unsigned char *psrc,const unsigned char *pedg,unsigned char *pdst,int32_t src_pitch,
	int32_t edg_pitchp,int32_t edg_pitchn,int32_t y_limit_min,int32_t y_limit_max,const short *x_limit_min,const short *x_limit_max,
	int32_t i_,int32_t depthH,int32_t depthV);

extern "C" void JPSDR_Sobel_8_SSE2(const unsigned char *psrc,unsigned char *pdst,int32_t src_pitch,int32_t y_,int32_t height,
	int32_t i_,int32_t thresh);

extern "C" void JPSDR_H_BlurR6_8_SSE3(unsigned char *psrc2,unsigned char *ptmp2,int32_t src_row_size,const unsigned char *dq0toF);
extern "C" void JPSDR_H_BlurR6_8_SSE2(unsigned char *psrc2,unsigned char *ptmp2,int32_t src_row_size_16);
extern "C" void JPSDR_V_BlurR6a_8_SSE2(unsigned char *psrc2,unsigned char *ptmp2,int32_t tmp_pitch, int32_t src_row_size_16);
extern "C" void JPSDR_V_BlurR6b_8_SSE2(unsigned char *psrc2,unsigned char *ptmp2,int32_t tmp_pitch, int32_t src_row_size_16);
extern "C" void JPSDR_V_BlurR6c_8_SSE2(unsigned char *psrc2,unsigned char *ptmp2,int32_t tmp_pitch, int32_t src_row_size_16);

extern "C" void JPSDR_H_BlurR2_8_SSE3(unsigned char *psrc2,unsigned char *ptmp2,int32_t src_row_size,const unsigned char *dq0toF);
extern "C" void JPSDR_H_BlurR2_8_SSE2(unsigned char *psrc2,unsigned char *ptmp2,int32_t src_row_size_16);
extern "C" void JPSDR_V_BlurR2_8_SSE2(unsigned char *psrc2,unsigned char *ptmp2,int32_t src_row_size_16,
	int32_t tmp_pitchp1,int32_t tmp_pitchp2,int32_t tmp_pitchn1,int32_t tmp_pitchn2);

extern "C" void JPSDR_GuideChroma1_8_SSE2(const unsigned char *py,unsigned char *pu,int32_t pitch_y,int32_t width_uv_8);
extern "C" void JPSDR_GuideChroma2_8_SSE2(const unsigned char *py,unsigned char *pu,int32_t width_uv_8);


__declspec(align(16)) static const unsigned char dq0toF[0x10]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};


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

  if ((aWarpSharp_g_cpuid & CPUF_SSE3)!=0)
  {
    for (int32_t y=0; y<dst_height; y++)
    {
      int32_t y_limit_min = -y * 0x80;
      int32_t y_limit_max = (dst_height - y) * 0x80 - 0x81;
      int32_t edg_pitchp = -(y ? edg_pitch : 0);
      int32_t edg_pitchn = y != dst_height - 1 ? edg_pitch : 0;

	  JPSDR_Warp0_8_SSE3(psrc,pedg,pdst,src_pitch,edg_pitchp,edg_pitchn,y_limit_min,y_limit_max,
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

  if ((aWarpSharp_g_cpuid & CPUF_SSE3)!=0)
  {
    for (int32_t y=ymin; y<ymax; y++)
    {
      int32_t y_limit_min = -y * 0x80;
      int32_t y_limit_max = (dst_height - y) * 0x80 - 0x81;
      int32_t edg_pitchp = -(y ? edg_pitch : 0);
      int32_t edg_pitchn = y != dst_height - 1 ? edg_pitch : 0;

	  JPSDR_Warp0_8_SSE3(psrc,pedg,pdst,src_pitch,edg_pitchp,edg_pitchn,y_limit_min,y_limit_max,
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

  if ((aWarpSharp_g_cpuid & CPUF_SSE3)!=0)
  {
    for (int32_t y=0; y<dst_height; y++)
    {
      int32_t y_limit_min = -y * 0x80;
      int32_t y_limit_max = (dst_height - y) * 0x80 - 0x81;
      int32_t edg_pitchp = -(y ? edg_pitch : 0);
      int32_t edg_pitchn = y != dst_height - 1 ? edg_pitch : 0;

	  JPSDR_Warp2_8_SSE3(psrc,pedg,pdst,src_pitch,edg_pitchp,edg_pitchn,y_limit_min,y_limit_max,
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

  if ((aWarpSharp_g_cpuid & CPUF_SSE3)!=0)
  {
    for (int32_t y=ymin; y<ymax; y++)
    {
      int32_t y_limit_min = -y * 0x80;
      int32_t y_limit_max = (dst_height - y) * 0x80 - 0x81;
      int32_t edg_pitchp = -(y ? edg_pitch : 0);
      int32_t edg_pitchn = y != dst_height - 1 ? edg_pitch : 0;

	  JPSDR_Warp2_8_SSE3(psrc,pedg,pdst,src_pitch,edg_pitchp,edg_pitchn,y_limit_min,y_limit_max,
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

    for (int32_t y=0; y<src_height; y++)
    {
		JPSDR_Sobel_8_SSE2(psrc,pdst,src_pitch,y,src_height,i,thresh);
		pdst[0] = pdst[1];
		pdst[dst_row_size-1] = pdst[dst_row_size-2];
		psrc += src_pitch;
		pdst += dst_pitch;
    }
}


static void Sobel_8_MT(const unsigned char *psrc,unsigned char *pdst,const int32_t src_pitch, const int32_t dst_pitch,
	const int32_t src_height,const int32_t dst_row_size, int32_t thresh,const int32_t ymin,const int32_t ymax)
{
  const int32_t i = (dst_row_size + 3) >> 2;

  psrc += src_pitch*ymin;
  pdst += dst_pitch*ymin;

    for (int32_t y=ymin; y<ymax; y++)
    {
		JPSDR_Sobel_8_SSE2(psrc,pdst,src_pitch,y,src_height,i,thresh);
		pdst[0] = pdst[1];
		pdst[dst_row_size-1] = pdst[dst_row_size-2];
		psrc += src_pitch;
		pdst += dst_pitch;
    }
}


// WxH min: 1x12, mul: 1x1 (write 16x1)
// (6+5+4+3)/16 + (2+1)*3/16 + (0)*6/16
static void BlurR6_8(unsigned char *const psrc,unsigned char *const ptmp,const int32_t src_pitch,const int32_t tmp_pitch,
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
	  if ((aWarpSharp_g_cpuid & CPUF_SSSE3)!=0) // SSSE3
		// SSSE3 version (palignr, pshufb)
	    for (int32_t y=0; y<src_height; y++)
		{
			JPSDR_H_BlurR6_8_SSE3(psrc2,ptmp2,src_row_size,dq0toF);
			psrc2 += src_pitch;
			ptmp2 += tmp_pitch;
		}
	else
		// SSE2 version
		// 6 left and right pixels are wrong
		for (int32_t y=0; y<src_height; y++)
		{
			JPSDR_H_BlurR6_8_SSE2(psrc2,ptmp2,src_row_size_16);
			psrc2 += src_pitch;
			ptmp2 += tmp_pitch;
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
    // SSE2 version
    int32_t y;
	const int32_t height_6=src_height-6;

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


static void BlurR6_8_MT(unsigned char *const psrc,unsigned char *const ptmp,const int32_t src_pitch,const int32_t tmp_pitch,
	const int32_t src_height,const int32_t src_row_size,bool processH,bool processV,const int32_t ymin,const int32_t ymax)
{
  const int32_t src_row_size_16 = (src_row_size + 15) >> 4;
  unsigned char *psrc2,*ptmp2;

  psrc2 = psrc+ymin*src_pitch;
  ptmp2 = ptmp+ymin*tmp_pitch;

  // Horizontal Blur
  // WxH min: 1x1, mul: 1x1 (write 16x1)
  if (processH)
  {
	  if ((aWarpSharp_g_cpuid & CPUF_SSSE3)!=0) // SSSE3
		// SSSE3 version (palignr, pshufb)
	    for (int32_t y=ymin; y<ymax; y++)
		{
			JPSDR_H_BlurR6_8_SSE3(psrc2,ptmp2,src_row_size,dq0toF);
			psrc2 += src_pitch;
			ptmp2 += tmp_pitch;
		}
	else
		// SSE2 version
		// 6 left and right pixels are wrong
		for (int32_t y=ymin; y<ymax; y++)
		{
			JPSDR_H_BlurR6_8_SSE2(psrc2,ptmp2,src_row_size_16);
			psrc2 += src_pitch;
			ptmp2 += tmp_pitch;
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

  // Vertical Blur
  // WxH min: 1x12, mul: 1x1 (write 16x1)
  psrc2 = psrc+ymin*src_pitch;

  if (processV)
  {
    // SSE2 version
    int32_t y;
	const int32_t height_6=src_height-6;
	
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
	  if ((aWarpSharp_g_cpuid & CPUF_SSSE3)!=0)
		// SSSE3 version (palignr, pshufb)
	    for (int32_t y=0; y<src_height; y++)
		{
			JPSDR_H_BlurR2_8_SSE3(psrc2,ptmp2,src_row_size,dq0toF);

	      psrc2 += src_pitch;
		  ptmp2 += tmp_pitch;
		}
	else
		// SSE2 version
		// 2 left and right pixels are wrong
		for (int32_t y=0; y<src_height; y++)
		{
			JPSDR_H_BlurR2_8_SSE2(psrc2,ptmp2,src_row_size_16);

			psrc2 += src_pitch;
			ptmp2 += tmp_pitch;
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
	 // SSE2 version
	const int32_t height_1=src_height-1,height_2=src_height-2;

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
static void BlurR2_8_MT(unsigned char *const psrc,unsigned char *const ptmp,const int32_t src_pitch,const int32_t tmp_pitch,
	 const int32_t src_height,const int32_t src_row_size,bool processH,bool processV,const int32_t ymin,const int32_t ymax)
{
  const int32_t src_row_size_16 = (src_row_size+0xF) & ~0xF;
  unsigned char *psrc2,*ptmp2;

  psrc2 = psrc+src_pitch*ymin;
  ptmp2 = ptmp+tmp_pitch*ymin;

  // Horizontal Blur
  // WxH min: 1x1, mul: 1x1 (write 16x1)
  if (processH)
  {
	  if ((aWarpSharp_g_cpuid & CPUF_SSSE3)!=0)
		// SSSE3 version (palignr, pshufb)
	    for (int32_t y=ymin; y<ymax; y++)
		{
			JPSDR_H_BlurR2_8_SSE3(psrc2,ptmp2,src_row_size,dq0toF);
			psrc2 += src_pitch;
			ptmp2 += tmp_pitch;
		}
	else
		// SSE2 version
		// 2 left and right pixels are wrong
		for (int32_t y=ymin; y<ymax; y++)
		{
			JPSDR_H_BlurR2_8_SSE2(psrc2,ptmp2,src_row_size_16);
			psrc2 += src_pitch;
			ptmp2 += tmp_pitch;
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

  psrc2 = psrc+src_pitch*ymin;
  ptmp2 = ptmp+tmp_pitch*ymin;

  // Vertical Blur
  // WxH min: 1x1, mul: 1x1 (write 16x1)
  if (processV)
  {
	 // SSE2 version
	const int32_t height_1=src_height-1,height_2=src_height-2;

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
        // SSE2 version
        for (int32_t y=0; y<dst_height_uv; y++)
        {
			JPSDR_GuideChroma1_8_SSE2(py,pu,src_pitch_y,width_uv_8);

          py += src_pitch_y2;
          pu += dst_pitch_uv;
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
        // SSE2 version
        for (int32_t y=0; y<dst_height_uv; y++)
        {
			JPSDR_GuideChroma2_8_SSE2(py,pu,width_uv_8);

          py += src_pitch_y;
          pu += dst_pitch_uv;
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
        // SSE2 version
        for (int32_t y=ymin; y<ymax; y++)
        {
			JPSDR_GuideChroma1_8_SSE2(py,pu,src_pitch_y,width_uv_8);

          py += src_pitch_y2;
          pu += dst_pitch_uv;
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
        // SSE2 version
        for (int32_t y=ymin; y<ymax; y++)
        {
			JPSDR_GuideChroma2_8_SSE2(py,pu,width_uv_8);

          py += src_pitch_y;
          pu += dst_pitch_uv;
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


static bool GuideChroma_8_Test(const int32_t subspl_dst_h_l2,const int32_t subspl_dst_v_l2)
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


static void SetPlane(PVideoFrame &dst, int plane, int value, const VideoInfo &dst_vi)
{
  const int dst_pitch = dst->GetPitch(plane);
  unsigned char *pdst = dst->GetWritePtr(plane);
  const int dst_row_size = dst->GetRowSize() >> dst_vi.GetPlaneWidthSubsampling(plane);
  int height = dst->GetHeight() >> dst_vi.GetPlaneHeightSubsampling(plane);

  if (dst_pitch == dst_row_size)
    memset(pdst, value, dst_pitch*height);
  else
    for (; height--; pdst += dst_pitch)
      memset(pdst, value, dst_row_size);
}


static void CopyPlane(PVideoFrame &src, PVideoFrame &dst, int plane, const VideoInfo &dst_vi)
{
  const int src_pitch = src->GetPitch(plane);
  const int dst_pitch = dst->GetPitch(plane);
  const unsigned char *psrc = src->GetReadPtr(plane);
  unsigned char *pdst = dst->GetWritePtr(plane);
  const int dst_row_size = dst->GetRowSize() >> dst_vi.GetPlaneWidthSubsampling(plane);
  int height = dst->GetHeight() >> dst_vi.GetPlaneHeightSubsampling(plane);

  if ((dst_pitch==src_pitch) && (dst_pitch==dst_row_size))
    memcpy(pdst, psrc, dst_pitch*height);
  else
    for (; height--; psrc += src_pitch, pdst += dst_pitch)
      memcpy(pdst, psrc, dst_row_size);
}


static uint8_t CreateMTData(MT_Data_Info_WarpSharp *MT_Data, uint8_t threads_number, uint8_t max_threads,int32_t size_x,int32_t size_y,int UV_w,int UV_h)
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
	uint8_t i,max=0;

	dst_dh_Y=(size_y+(uint32_t)max_threads-1)/(uint32_t)max_threads;
	if (dst_dh_Y<16) dst_dh_Y=16;
	if ((dst_dh_Y & 3)!=0) dst_dh_Y=((dst_dh_Y+3) >> 2) << 2;

	src_dh_Y=dst_dh_Y;

	_y_min=size_y;
	_dh=dst_dh_Y;

	h_y=0;
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

	if (vi.IsY8()) chroma = 1;

	if (threshC==-1) threshC=thresh;
	if (depthV==128) depthV=depth;
	if (depthC==128) depthC = vi.IsYV24() ? depth:(depth>>1);
	if (depthVC==128) depthVC = depthC;

	if (blur_level==-1) blur_level=(blur_type==0)?2:3;
	if (blur_levelV==-1) blur_levelV=blur_level;
	if (blur_levelC==-1) blur_levelC=(blur_level+1)>>1;
	if (blur_levelVC==-1) blur_levelVC=blur_levelC;

	if (!(vi.IsYUV() && vi.IsPlanar())) env->ThrowError("aWarpSharp: Planar YUV input is required");
	if ((thresh<0) || (thresh>255)) env->ThrowError("aWarpSharp: 'thresh' must be 0..255");
	if ((threshC<0) || (threshC>255)) env->ThrowError("aWarpSharp: 'threshC' must be 0..255");
	if (blur_level<0) env->ThrowError("aWarpSharp: 'blur' must be >=0");
	if (blur_levelV<0) env->ThrowError("aWarpSharp: 'blurV' must be >=0");
	if (blur_levelC<0) env->ThrowError("aWarpSharp: 'blurC' must be >=0");
	if (blur_levelVC<0) env->ThrowError("aWarpSharp: 'blurVC' must be >=0");
	if ((blur_type<0) || (blur_type>1)) env->ThrowError("aWarpSharp: 'type' must be 0,1");
	if ((depth<-128) || (depth>127)) env->ThrowError("aWarpSharp: 'depth' must be -128..127");
	if ((depthC<-128) || (depthC>127)) env->ThrowError("aWarpSharp: 'depthC' must be -128..127");
	if ((chroma<0) || (chroma>6)) env->ThrowError("aWarpSharp: 'chroma' must be 0..6");
	if ((depthV<-128) || (depthV>127)) env->ThrowError("aWarpSharp: 'depthV' must be -128..127");
	if ((depthVC<-128) || (depthVC>127)) env->ThrowError("aWarpSharp: 'depthVC' must be -128..127");

	StaticThreadpoolF=StaticThreadpool;

	for (uint8_t i=0; i<MAX_MT_THREADS; i++)
	{
		MT_Thread[i].pClass=this;
		MT_Thread[i].f_process=0;
		MT_Thread[i].thread_Id=(uint8_t)i;
		MT_Thread[i].pFunc=StaticThreadpoolF;
	}

	UserId=0;
	ghMutex=NULL;
	
	const int shift_w = (!grey && vi.IsPlanar() && !isRGBPfamily) ? vi.GetPlaneWidthSubsampling(PLANAR_U) : 0;
	const int shift_h = (!grey && vi.IsPlanar() && !isRGBPfamily) ? vi.GetPlaneHeightSubsampling(PLANAR_U) : 0;

	if (vi.height<32) threads_number=1;
	else threads_number=threads;
	
	ghMutex=CreateMutex(NULL,FALSE,NULL);
	if (ghMutex==NULL) env->ThrowError("aWarpSharp: Unable to create Mutex!");

	threads_number=CreateMTData(MT_Data,threads_number,threads_number,vi.width,vi.height,shift_w,shift_h);

	if (threads_number>1)
	{
		if (!poolInterface->GetUserId(UserId))
		{
			poolInterface->DeAllocateAllThreads(true);
			FreeData();
			env->ThrowError("aWarpSharp: Error with the TheadPool while getting UserId!");
		}
	}
}


void aWarpSharp::FreeData(void) 
{
	myCloseHandle(ghMutex);
}


aWarpSharp::~aWarpSharp()
{
	if (threads_number>1)
	{
		poolInterface->RemoveUserId(UserId);
		poolInterface->DeAllocateAllThreads(true);
	}
	FreeData();
}


int __stdcall aWarpSharp::SetCacheHints(int cachehints,int frame_range)
{
  switch (cachehints)
  {
  case CACHE_GET_MTMODE :
    return MT_MULTI_INSTANCE;
  default :
    return 0;
  }
}



void aWarpSharp::StaticThreadpool(void *ptr)
{
	const Public_MT_Data_Thread *data=(const Public_MT_Data_Thread *)ptr;
	const aWarpSharp *ptrClass=(aWarpSharp *)data->pClass;

	const uint8_t thread_num=data->thread_Id;
	const MT_Data_Info_WarpSharp mt_data_inf=ptrClass->MT_Data[thread_num];
	
	switch(data->f_process)
	{
		case 1 :
			Sobel_8_MT((const unsigned char *)mt_data_inf.src_Y1,(unsigned char *)mt_data_inf.dst_Y1,
				mt_data_inf.src_pitch_Y1,mt_data_inf.src_pitch_Y2,mt_data_inf.src_Y_h,
				mt_data_inf.row_size_Y2,ptrClass->thresh,mt_data_inf.src_Y_h_min,mt_data_inf.src_Y_h_max);
			break;
		case 2 :
			BlurR2_8_MT((unsigned char *const)mt_data_inf.dst_Y1,(unsigned char *const)mt_data_inf.dst_Y2,
				mt_data_inf.src_pitch_Y2,mt_data_inf.dst_pitch_Y2,mt_data_inf.dst_Y_h,
				mt_data_inf.row_size_Y2,true,true,mt_data_inf.dst_Y_h_min,mt_data_inf.dst_Y_h_max);
			break;
		case 3 :
			BlurR6_8_MT((unsigned char *const)mt_data_inf.dst_Y1,(unsigned char *const)mt_data_inf.dst_Y2,
				mt_data_inf.src_pitch_Y2,mt_data_inf.dst_pitch_Y2,mt_data_inf.dst_Y_h,
				mt_data_inf.row_size_Y2,true,true,mt_data_inf.dst_Y_h_min,mt_data_inf.dst_Y_h_max);
			break;
		case 4 :
			BlurR2_8_MT((unsigned char *const)mt_data_inf.dst_Y1,(unsigned char *const)mt_data_inf.dst_Y2,
				mt_data_inf.src_pitch_Y2,mt_data_inf.dst_pitch_Y2,mt_data_inf.dst_Y_h,
				mt_data_inf.row_size_Y2,mt_data_inf.processH,mt_data_inf.processV,mt_data_inf.dst_Y_h_min,mt_data_inf.dst_Y_h_max);
			break;
		case 5 :
			BlurR6_8_MT((unsigned char *const)mt_data_inf.dst_Y1,(unsigned char *const)mt_data_inf.dst_Y2,
				mt_data_inf.src_pitch_Y2,mt_data_inf.dst_pitch_Y2,mt_data_inf.dst_Y_h,
				mt_data_inf.row_size_Y2,mt_data_inf.processH,mt_data_inf.processV,mt_data_inf.dst_Y_h_min,mt_data_inf.dst_Y_h_max);
			break;
		case 6 :
			Warp0_8_MT((const unsigned char *)mt_data_inf.src_Y1,(const unsigned char *)mt_data_inf.src_Y2,
				(unsigned char *)mt_data_inf.dst_Y2,
				mt_data_inf.src_pitch_Y1,mt_data_inf.src_pitch_Y2,mt_data_inf.dst_pitch_Y2,
				mt_data_inf.row_size_Y3,mt_data_inf.dst_Y_h,ptrClass->depth,ptrClass->depthV,
				mt_data_inf.dst_Y_h_min,mt_data_inf.dst_Y_h_max);
			break;
		case 7 :
			Sobel_8_MT((const unsigned char *)mt_data_inf.src_U1,(unsigned char *)mt_data_inf.dst_U1,
				mt_data_inf.src_pitch_U1,mt_data_inf.src_pitch_U2,mt_data_inf.src_U_h,
				mt_data_inf.row_size_U1,ptrClass->threshC,mt_data_inf.dst_UV_h_min,mt_data_inf.dst_UV_h_max);
			break;
		case 8 :
			BlurR2_8_MT((unsigned char *const)mt_data_inf.dst_U1,(unsigned char *const)mt_data_inf.dst_U2,
				mt_data_inf.src_pitch_U2,mt_data_inf.dst_pitch_U2,mt_data_inf.dst_U_h,
				mt_data_inf.row_size_U1,true,true,mt_data_inf.dst_UV_h_min,mt_data_inf.dst_UV_h_max);
			break;
		case 9 :
			BlurR6_8_MT((unsigned char *const)mt_data_inf.dst_U1,(unsigned char *const)mt_data_inf.dst_U2,
				mt_data_inf.src_pitch_U2,mt_data_inf.dst_pitch_U2,mt_data_inf.dst_U_h,
				mt_data_inf.row_size_U1,true,true,mt_data_inf.dst_UV_h_min,mt_data_inf.dst_UV_h_max);
			break;
		case 10 :
			BlurR2_8_MT((unsigned char *const)mt_data_inf.dst_U1,(unsigned char *const)mt_data_inf.dst_U2,
				mt_data_inf.src_pitch_U2,mt_data_inf.dst_pitch_U2,mt_data_inf.dst_U_h,
				mt_data_inf.row_size_U1,mt_data_inf.cprocessH,mt_data_inf.cprocessV,
				mt_data_inf.dst_UV_h_min,mt_data_inf.dst_UV_h_max);
			break;
		case 11 :
			BlurR6_8_MT((unsigned char *const)mt_data_inf.dst_U1,(unsigned char *const)mt_data_inf.dst_U2,
				mt_data_inf.src_pitch_U2,mt_data_inf.dst_pitch_U2,mt_data_inf.dst_U_h,
				mt_data_inf.row_size_U1,mt_data_inf.cprocessH,mt_data_inf.cprocessV,
				mt_data_inf.dst_UV_h_min,mt_data_inf.dst_UV_h_max);
			break;
		case 12 :
			Warp0_8_MT((const unsigned char *)mt_data_inf.src_U1,(const unsigned char *)mt_data_inf.src_U2,
				(unsigned char *)mt_data_inf.dst_U2,
				mt_data_inf.src_pitch_U1,mt_data_inf.src_pitch_U2,mt_data_inf.dst_pitch_U2,
				mt_data_inf.row_size_U2,mt_data_inf.dst_U_h,ptrClass->depthC,ptrClass->depthVC,
				mt_data_inf.dst_UV_h_min,mt_data_inf.dst_UV_h_max);
			break;
		case 13 :
			Sobel_8_MT((const unsigned char *)mt_data_inf.src_V1,(unsigned char *)mt_data_inf.dst_V1,
				mt_data_inf.src_pitch_V1,mt_data_inf.src_pitch_V2,mt_data_inf.src_V_h,
				mt_data_inf.row_size_V1,ptrClass->threshC,mt_data_inf.dst_UV_h_min,mt_data_inf.dst_UV_h_max);
			break;
		case 14 :
			BlurR2_8_MT((unsigned char *const)mt_data_inf.dst_V1,(unsigned char *const)mt_data_inf.dst_V2,
				mt_data_inf.src_pitch_V2,mt_data_inf.dst_pitch_V2,mt_data_inf.dst_V_h,
				mt_data_inf.row_size_V1,true,true,mt_data_inf.dst_UV_h_min,mt_data_inf.dst_UV_h_max);
			break;
		case 15 :
			BlurR6_8_MT((unsigned char *const)mt_data_inf.dst_V1,(unsigned char *const)mt_data_inf.dst_V2,
				mt_data_inf.src_pitch_V2,mt_data_inf.dst_pitch_V2,mt_data_inf.dst_V_h,
				mt_data_inf.row_size_V1,true,true,mt_data_inf.dst_UV_h_min,mt_data_inf.dst_UV_h_max);
			break;
		case 16 :
			BlurR2_8_MT((unsigned char *const)mt_data_inf.dst_V1,(unsigned char *const)mt_data_inf.dst_V2,
				mt_data_inf.src_pitch_V2,mt_data_inf.dst_pitch_V2,mt_data_inf.dst_V_h,
				mt_data_inf.row_size_V1,mt_data_inf.cprocessH,mt_data_inf.cprocessV,
				mt_data_inf.dst_UV_h_min,mt_data_inf.dst_UV_h_max);
			break;
		case 17 :
			BlurR6_8_MT((unsigned char *const)mt_data_inf.dst_V1,(unsigned char *const)mt_data_inf.dst_V2,
				mt_data_inf.src_pitch_V2,mt_data_inf.dst_pitch_V2,mt_data_inf.dst_V_h,
				mt_data_inf.row_size_V1,mt_data_inf.cprocessH,mt_data_inf.cprocessV,
				mt_data_inf.dst_UV_h_min,mt_data_inf.dst_UV_h_max);
			break;
		case 18 :
			Warp0_8_MT((const unsigned char *)mt_data_inf.src_V1,(const unsigned char *)mt_data_inf.src_V2,
				(unsigned char *)mt_data_inf.dst_V2,
				mt_data_inf.src_pitch_V1,mt_data_inf.src_pitch_V2,mt_data_inf.dst_pitch_V2,
				mt_data_inf.row_size_V2,mt_data_inf.dst_V_h,ptrClass->depthC,ptrClass->depthVC,
				mt_data_inf.dst_UV_h_min,mt_data_inf.dst_UV_h_max);
			break;
		case 19 :
			GuideChroma_8_MT((const unsigned char *)mt_data_inf.src_Y2,(unsigned char *)mt_data_inf.dst_U1,
				mt_data_inf.src_pitch_Y2,mt_data_inf.src_pitch_U2,mt_data_inf.dst_U_h,
				mt_data_inf.row_size_U1,mt_data_inf.SubW_U,mt_data_inf.SubH_U,ptrClass->cplace_mpeg2_flag,
				mt_data_inf.dst_UV_h_min,mt_data_inf.dst_UV_h_max);
			break;
		case 20 :
			Warp0_8_MT((const unsigned char *)mt_data_inf.src_U1,(const unsigned char *)mt_data_inf.src_U2,
				(unsigned char *)mt_data_inf.dst_U2,
				mt_data_inf.src_pitch_U1,mt_data_inf.src_pitch_U2,mt_data_inf.dst_pitch_U2,
				mt_data_inf.row_size_U2,mt_data_inf.dst_U_h,ptrClass->depthC,ptrClass->depthVC,
				mt_data_inf.dst_UV_h_min,mt_data_inf.dst_UV_h_max);
			break;
		case 21 :
			Warp0_8_MT((const unsigned char *)mt_data_inf.src_V1,(const unsigned char *)mt_data_inf.src_U2,
				(unsigned char *)mt_data_inf.dst_V2,
				mt_data_inf.src_pitch_V1,mt_data_inf.src_pitch_U2,mt_data_inf.dst_pitch_V2,
				mt_data_inf.row_size_V2,mt_data_inf.dst_V_h,ptrClass->depthC,ptrClass->depthVC,
				mt_data_inf.dst_UV_h_min,mt_data_inf.dst_UV_h_max);
			break;
		case 22 :
			Warp0_8_MT((const unsigned char *)mt_data_inf.src_U1,(const unsigned char *)mt_data_inf.src_Y2,
				(unsigned char *)mt_data_inf.dst_U2,
				mt_data_inf.src_pitch_U1,mt_data_inf.src_pitch_Y2,mt_data_inf.dst_pitch_U2,
				mt_data_inf.row_size_U2,mt_data_inf.dst_U_h,ptrClass->depthC,ptrClass->depthVC,
				mt_data_inf.dst_UV_h_min,mt_data_inf.dst_UV_h_max);
			break;
		case 23 :
			Warp0_8_MT((const unsigned char *)mt_data_inf.src_V1,(const unsigned char *)mt_data_inf.src_Y2,
				(unsigned char *)mt_data_inf.dst_V2,
				mt_data_inf.src_pitch_V1,mt_data_inf.src_pitch_Y2,mt_data_inf.dst_pitch_V2,
				mt_data_inf.row_size_V2,mt_data_inf.dst_V_h,ptrClass->depthC,ptrClass->depthVC,
				mt_data_inf.dst_UV_h_min,mt_data_inf.dst_UV_h_max);
			break;
		default : ;
	}
}


PVideoFrame __stdcall aWarpSharp::GetFrame(int n, IScriptEnvironment *env)
{
  PVideoFrame src = child->GetFrame(n, env);
  PVideoFrame tmp = env->NewVideoFrame(vi,64);
  PVideoFrame dst = env->NewVideoFrame(vi,64);

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

  const int SubH_Y = vi.GetPlaneHeightSubsampling(PLANAR_Y);
  const int SubH_U = vi.IsY() ? 0:vi.GetPlaneHeightSubsampling(PLANAR_U);
  const int SubH_V = vi.IsY() ? 0:vi.GetPlaneHeightSubsampling(PLANAR_V);
  const int SubW_Y = vi.GetPlaneWidthSubsampling(PLANAR_Y);
  const int SubW_U = vi.IsY() ? 0:vi.GetPlaneWidthSubsampling(PLANAR_U);
  const int SubW_V = vi.IsY() ? 0:vi.GetPlaneWidthSubsampling(PLANAR_V);

  const int32_t src_height_Y = src->GetHeight() >> SubH_Y;
  const int32_t tmp_height_Y = tmp->GetHeight() >> SubH_Y;
  const int32_t dst_height_Y = dst->GetHeight() >> SubH_Y;

  const int32_t src_height_U = src->GetHeight() >> SubH_U;
  const int32_t tmp_height_U = tmp->GetHeight() >> SubH_U;
  const int32_t dst_height_U = dst->GetHeight() >> SubH_U;

  const int32_t src_height_V = src->GetHeight() >> SubH_V;
  const int32_t tmp_height_V = tmp->GetHeight() >> SubH_V;
  const int32_t dst_height_V = dst->GetHeight() >> SubH_V;

  const int32_t src_row_size_Y = src->GetRowSize() >> SubW_Y;
  const int32_t tmp_row_size_Y = tmp->GetRowSize() >> SubW_Y;
  const int32_t dst_row_size_Y = dst->GetRowSize() >> SubW_Y;

  const int32_t tmp_row_size_U = tmp->GetRowSize() >> SubW_U;
  const int32_t dst_row_size_U = dst->GetRowSize() >> SubW_U;

  const int32_t tmp_row_size_V = tmp->GetRowSize() >> SubW_V;
  const int32_t dst_row_size_V = dst->GetRowSize() >> SubW_V;

  const int blurL=std::min(blur_level,blur_levelV);
  const int blurLr=std::max(blur_level,blur_levelV)-blurL;
  const bool processH=blur_level>blurL,processV=blur_levelV>blurL;

  const int cblurL=std::min(blur_levelC,blur_levelVC);
  const int cblurLr=std::max(blur_levelC,blur_levelVC)-cblurL;
  const bool cprocessH=blur_levelC>cblurL,cprocessV=blur_levelVC>cblurL;

  if (threads_number>1)
  {
	  WaitForSingleObject(ghMutex,INFINITE);

	  if (!poolInterface->RequestThreadPool(UserId,threads_number,MT_Thread,-1,false))
	  {
		  ReleaseMutex(ghMutex);
		  env->ThrowError("aWarpSharp: Error with the TheadPool while requesting threadpool!");
	  }
  }

	if (threads_number>1)
	{

	for(uint8_t i=0; i<threads_number; i++)
	{
		MT_Data[i].src_Y1=(void *)psrc_Y;
		MT_Data[i].src_Y2=(void *)ptmp_Y;
		MT_Data[i].src_pitch_Y1=src_pitch_Y;
		MT_Data[i].src_pitch_Y2=tmp_pitch_Y;
		MT_Data[i].row_size_Y1=src_row_size_Y;
		MT_Data[i].row_size_Y2=tmp_row_size_Y;
		MT_Data[i].row_size_Y3=dst_row_size_Y;
		MT_Data[i].dst_Y1=dptmp_Y;
		MT_Data[i].dst_Y2=pdst_Y;
		MT_Data[i].dst_pitch_Y1=tmp_pitch_Y;
		MT_Data[i].dst_pitch_Y2=dst_pitch_Y;

		MT_Data[i].src_U1=(void *)psrc_U;
		MT_Data[i].src_U2=(void *)ptmp_U;
		MT_Data[i].src_pitch_U1=src_pitch_U;
		MT_Data[i].src_pitch_U2=tmp_pitch_U;
		MT_Data[i].row_size_U1=tmp_row_size_U;
		MT_Data[i].row_size_U2=dst_row_size_U;
		MT_Data[i].dst_U1=dptmp_U;
		MT_Data[i].dst_U2=pdst_U;
		MT_Data[i].dst_pitch_U1=tmp_pitch_U;
		MT_Data[i].dst_pitch_U2=dst_pitch_U;

		MT_Data[i].src_V1=(void *)psrc_V;
		MT_Data[i].src_V2=(void *)ptmp_V;
		MT_Data[i].src_pitch_V1=src_pitch_V;
		MT_Data[i].src_pitch_V2=tmp_pitch_V;
		MT_Data[i].row_size_V1=tmp_row_size_V;
		MT_Data[i].row_size_V2=dst_row_size_V;
		MT_Data[i].dst_V1=dptmp_V;
		MT_Data[i].dst_V2=pdst_V;
		MT_Data[i].dst_pitch_V1=tmp_pitch_V;
		MT_Data[i].dst_pitch_V2=dst_pitch_V;

		MT_Data[i].src_Y_h=src_height_Y;
		MT_Data[i].src_U_h=src_height_U;
		MT_Data[i].src_V_h=src_height_V;
		MT_Data[i].dst_Y_h=dst_height_Y;
		MT_Data[i].dst_U_h=dst_height_U;
		MT_Data[i].dst_V_h=dst_height_V;

		MT_Data[i].processH=processH;
		MT_Data[i].processV=processV;
		MT_Data[i].cprocessH=cprocessH;
		MT_Data[i].cprocessV=cprocessV;
		MT_Data[i].SubW_U=SubW_U;
		MT_Data[i].SubH_U=SubH_U;
	}

		uint8_t f_proc;

  if (chroma!=5)
  {
	  f_proc=1;

	  for(uint8_t i=0; i<threads_number; i++)
		  MT_Thread[i].f_process=f_proc;

	  if (poolInterface->StartThreads(UserId)) poolInterface->WaitThreadsEnd(UserId);

	  f_proc=blur_type ? 2:3;

	  for(uint8_t i=0; i<threads_number; i++)
		  MT_Thread[i].f_process=f_proc;

	for (int i=0; i<blurL; i++)
		if (poolInterface->StartThreads(UserId)) poolInterface->WaitThreadsEnd(UserId);

	f_proc=blur_type ? 4:5;

	for(uint8_t i=0; i<threads_number; i++)
		MT_Thread[i].f_process=f_proc;

	for (int i=0; i<blurLr; i++)
		if (poolInterface->StartThreads(UserId)) poolInterface->WaitThreadsEnd(UserId);

    if ((chroma!=6) && ((depth!=0) || (depthV!=0)))
	{
		f_proc=6;

		for(uint8_t i=0; i<threads_number; i++)
			MT_Thread[i].f_process=f_proc;

		if (poolInterface->StartThreads(UserId)) poolInterface->WaitThreadsEnd(UserId);
	}
    else
      CopyPlane(src, dst, PLANAR_Y, vi);
  }
  else
    CopyPlane(src, dst, PLANAR_Y, vi);

  switch (chroma)
  {
  case 0:
    SetPlane(dst, PLANAR_U, 0x80, vi);
    SetPlane(dst, PLANAR_V, 0x80, vi);
    break;
  case 2:
    CopyPlane(src, dst, PLANAR_U, vi);
    CopyPlane(src, dst, PLANAR_V, vi);
    break;
  case 3:
  case 5:
	  if ((depthC!=0) || (depthVC!=0))
	  {
		  f_proc=7;

		  for(uint8_t i=0; i<threads_number; i++)
			  MT_Thread[i].f_process=f_proc;

		  if (poolInterface->StartThreads(UserId)) poolInterface->WaitThreadsEnd(UserId);

		  f_proc=blur_type ? 8:9;

		  for(uint8_t i=0; i<threads_number; i++)
			  MT_Thread[i].f_process=f_proc;

	for (int i=0; i<cblurL; i++)
		if (poolInterface->StartThreads(UserId)) poolInterface->WaitThreadsEnd(UserId);

		  f_proc=blur_type ? 10:11;

		  for(uint8_t i=0; i<threads_number; i++)
			  MT_Thread[i].f_process=f_proc;

	for (int i=0; i<cblurLr; i++)
		if (poolInterface->StartThreads(UserId)) poolInterface->WaitThreadsEnd(UserId);

	f_proc=12;

	for(uint8_t i=0; i<threads_number; i++)
		MT_Thread[i].f_process=f_proc;

	if (poolInterface->StartThreads(UserId)) poolInterface->WaitThreadsEnd(UserId);

	f_proc=13;

	for(uint8_t i=0; i<threads_number; i++)
		MT_Thread[i].f_process=f_proc;

	if (poolInterface->StartThreads(UserId)) poolInterface->WaitThreadsEnd(UserId);

	f_proc=blur_type ? 14:15;

	for(uint8_t i=0; i<threads_number; i++)
		MT_Thread[i].f_process=f_proc;

	for (int i=0; i<cblurL; i++)
		if (poolInterface->StartThreads(UserId)) poolInterface->WaitThreadsEnd(UserId);

	f_proc=blur_type ? 16:17;

	for(uint8_t i=0; i<threads_number; i++)
		MT_Thread[i].f_process=f_proc;

	for (int i=0; i<cblurLr; i++)
		if (poolInterface->StartThreads(UserId)) poolInterface->WaitThreadsEnd(UserId);

	f_proc=18;

	for(uint8_t i=0; i<threads_number; i++)
		MT_Thread[i].f_process=f_proc;

	if (poolInterface->StartThreads(UserId)) poolInterface->WaitThreadsEnd(UserId);

	  }
	  else
	  {
		   CopyPlane(src, dst, PLANAR_U, vi);
		   CopyPlane(src, dst, PLANAR_V, vi);
	  }
    break;
  case 4:
  case 6:
	  if ((depthC!=0) || (depthVC!=0))
	  {
    if (!vi.IsYV24())
    {
	  if (!GuideChroma_8_Test(SubW_U,SubH_U))
	  {
		  ReleaseMutex(ghMutex);
		  env->ThrowError("aWarpSharp: Unsuported colorspace");
	  }
	  f_proc=19;

	  for(uint8_t i=0; i<threads_number; i++)
		  MT_Thread[i].f_process=f_proc;

	  if (poolInterface->StartThreads(UserId)) poolInterface->WaitThreadsEnd(UserId);

	  f_proc=20;

	  for(uint8_t i=0; i<threads_number; i++)
		  MT_Thread[i].f_process=f_proc;

	  if (poolInterface->StartThreads(UserId)) poolInterface->WaitThreadsEnd(UserId);

	  f_proc=21;

	  for(uint8_t i=0; i<threads_number; i++)
		  MT_Thread[i].f_process=f_proc;

	  if (poolInterface->StartThreads(UserId)) poolInterface->WaitThreadsEnd(UserId);
    }
    else
    {
	  f_proc=22;

	  for(uint8_t i=0; i<threads_number; i++)
		  MT_Thread[i].f_process=f_proc;

	  if (poolInterface->StartThreads(UserId)) poolInterface->WaitThreadsEnd(UserId);

	  f_proc=23;

	  for(uint8_t i=0; i<threads_number; i++)
		  MT_Thread[i].f_process=f_proc;

	  if (poolInterface->StartThreads(UserId)) poolInterface->WaitThreadsEnd(UserId);
    }
	  }
	  else
	  {
		   CopyPlane(src, dst, PLANAR_U, vi);
		   CopyPlane(src, dst, PLANAR_V, vi);
	  }
  }

  for(uint8_t i=0; i<threads_number; i++)
	  MT_Thread[i].f_process=0;

  poolInterface->ReleaseThreadPool(UserId,sleep);

  ReleaseMutex(ghMutex);

	}
	else
	{

  if (chroma!=5)
  {
	Sobel_8(psrc_Y,dptmp_Y,src_pitch_Y,tmp_pitch_Y,src_height_Y,tmp_row_size_Y,thresh);
	for (int i=0; i<blurL; i++)
	{
		if (blur_type) BlurR2_8(wptmp_Y,wpdst_Y,tmp_pitch_Y,dst_pitch_Y,tmp_height_Y,tmp_row_size_Y,true,true);
		else BlurR6_8(wptmp_Y,wpdst_Y,tmp_pitch_Y,dst_pitch_Y,tmp_height_Y,tmp_row_size_Y,true,true);
	}
	for (int i=0; i<blurLr; i++)
	{
		if (blur_type) BlurR2_8(wptmp_Y,wpdst_Y,tmp_pitch_Y,dst_pitch_Y,tmp_height_Y,tmp_row_size_Y,processH,processV);
		else BlurR6_8(wptmp_Y,wpdst_Y,tmp_pitch_Y,dst_pitch_Y,tmp_height_Y,tmp_row_size_Y,processH,processV);
	}
    if ((chroma!=6) && ((depth!=0) || (depthV!=0)))
		Warp0_8(psrc_Y,ptmp_Y,pdst_Y,src_pitch_Y,tmp_pitch_Y,dst_pitch_Y,dst_row_size_Y,dst_height_Y,depth,depthV);
    else
      CopyPlane(src, dst, PLANAR_Y, vi);
  }
  else
    CopyPlane(src, dst, PLANAR_Y, vi);

  switch (chroma)
  {
  case 0:
    SetPlane(dst, PLANAR_U, 0x80, vi);
    SetPlane(dst, PLANAR_V, 0x80, vi);
    break;
  case 2:
    CopyPlane(src, dst, PLANAR_U, vi);
    CopyPlane(src, dst, PLANAR_V, vi);
    break;
  case 3:
  case 5:
	  if ((depthC!=0) || (depthVC!=0))
	  {
	Sobel_8(psrc_U,dptmp_U,src_pitch_U,tmp_pitch_U,src_height_U,tmp_row_size_U,threshC);
	for (int i=0; i<cblurL; i++)
	{
		if (blur_type) BlurR2_8(wptmp_U,wpdst_U,tmp_pitch_U,dst_pitch_U,tmp_height_U,tmp_row_size_U,true,true);
		else BlurR6_8(wptmp_U,wpdst_U,tmp_pitch_U,dst_pitch_U,tmp_height_U,tmp_row_size_U,true,true);
	}
	for (int i=0; i<cblurLr; i++)
	{
		if (blur_type) BlurR2_8(wptmp_U,wpdst_U,tmp_pitch_U,dst_pitch_U,tmp_height_U,tmp_row_size_U,cprocessH,cprocessV);
		else BlurR6_8(wptmp_U,wpdst_U,tmp_pitch_U,dst_pitch_U,tmp_height_U,tmp_row_size_U,cprocessH,cprocessV);
	}
	Warp0_8(psrc_U,ptmp_U,pdst_U,src_pitch_U,tmp_pitch_U,dst_pitch_U,dst_row_size_U,dst_height_U,depthC,depthVC);

	Sobel_8(psrc_V,dptmp_V,src_pitch_V,tmp_pitch_V,src_height_V,tmp_row_size_V,threshC);
	for (int i=0; i<cblurL; i++)
	{
		if (blur_type) BlurR2_8(wptmp_V,wpdst_V,tmp_pitch_V,dst_pitch_V,tmp_height_V,tmp_row_size_V,true,true);
		else BlurR6_8(wptmp_V,wpdst_V,tmp_pitch_V,dst_pitch_V,tmp_height_V,tmp_row_size_V,true,true);
	}
	for (int i=0; i<cblurLr; i++)
	{
		if (blur_type) BlurR2_8(wptmp_V,wpdst_V,tmp_pitch_V,dst_pitch_V,tmp_height_V,tmp_row_size_V,cprocessH,cprocessV);
		else BlurR6_8(wptmp_V,wpdst_V,tmp_pitch_V,dst_pitch_V,tmp_height_V,tmp_row_size_V,cprocessH,cprocessV);
	}
	Warp0_8(psrc_V,ptmp_V,pdst_V,src_pitch_V,tmp_pitch_V,dst_pitch_V,dst_row_size_V,dst_height_V,depthC,depthVC);
	  }
	  else
	  {
		   CopyPlane(src, dst, PLANAR_U, vi);
		   CopyPlane(src, dst, PLANAR_V, vi);
	  }
    break;
  case 4:
  case 6:
	  if ((depthC!=0) || (depthVC!=0))
	  {
    if (!vi.IsYV24())
    {
	  if (!GuideChroma_8(ptmp_Y,dptmp_U,tmp_pitch_Y,tmp_pitch_U,tmp_height_U,tmp_row_size_U,SubW_U,
		  SubH_U,cplace_mpeg2_flag))
	  {
		  ReleaseMutex(ghMutex);
		  env->ThrowError("aWarpSharp: Unsuported colorspace");
	  }
	  Warp0_8(psrc_U,ptmp_U,pdst_U,src_pitch_U,tmp_pitch_U,dst_pitch_U,dst_row_size_U,dst_height_U,depthC,depthVC);
	  Warp0_8(psrc_V,ptmp_U,pdst_V,src_pitch_V,tmp_pitch_U,dst_pitch_V,dst_row_size_V,dst_height_V,depthC,depthVC);
    }
    else
    {
	  Warp0_8(psrc_U,ptmp_Y,pdst_U,src_pitch_U,tmp_pitch_Y,dst_pitch_U,dst_row_size_U,dst_height_U,depthC,depthVC);
	  Warp0_8(psrc_V,ptmp_Y,pdst_V,src_pitch_V,tmp_pitch_Y,dst_pitch_V,dst_row_size_V,dst_height_V,depthC,depthVC);
    }
	  }
	  else
	  {
		   CopyPlane(src, dst, PLANAR_U, vi);
		   CopyPlane(src, dst, PLANAR_V, vi);
	  }
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

    if (vi.IsY8()) chroma = 1;
	if (threshC==-1) threshC=thresh;

	if (!(vi.IsYUV() && vi.IsPlanar())) env->ThrowError("aSobel: Planar YUV input is required");
	if ((thresh<0) || (thresh>255)) env->ThrowError("aSobel: 'thresh' must be 0..255");
	if ((threshC<0) || (threshC>255)) env->ThrowError("aSobel: 'threshC' must be 0..255");
	if ((chroma<0) || (chroma>6)) env->ThrowError("aSobel: 'chroma' must be 0..6");

	StaticThreadpoolF=StaticThreadpool;

	for (uint8_t i=0; i<MAX_MT_THREADS; i++)
	{
		MT_Thread[i].pClass=this;
		MT_Thread[i].f_process=0;
		MT_Thread[i].thread_Id=(uint8_t)i;
		MT_Thread[i].pFunc=StaticThreadpoolF;
	}

	UserId=0;
	ghMutex=NULL;
	
	const int shift_w = (!grey && vi.IsPlanar() && !isRGBPfamily) ? vi.GetPlaneWidthSubsampling(PLANAR_U) : 0;
	const int shift_h = (!grey && vi.IsPlanar() && !isRGBPfamily) ? vi.GetPlaneHeightSubsampling(PLANAR_U) : 0;

	if (vi.height<32) threads_number=1;
	else threads_number=threads;
	
	ghMutex=CreateMutex(NULL,FALSE,NULL);
	if (ghMutex==NULL) env->ThrowError("aSobel: Unable to create Mutex!");

	threads_number=CreateMTData(MT_Data,threads_number,threads_number,vi.width,vi.height,shift_w,shift_h);

	if (threads_number>1)
	{
		if (!poolInterface->GetUserId(UserId))
		{
			poolInterface->DeAllocateAllThreads(true);
			FreeData();
			env->ThrowError("aSobel: Error with the TheadPool while getting UserId!");
		}
	}
}


void aSobel::FreeData(void)
{
	myCloseHandle(ghMutex);
}


aSobel::~aSobel()
{
	if (threads_number>1)
	{
		poolInterface->RemoveUserId(UserId);
		poolInterface->DeAllocateAllThreads(true);
	}
	FreeData();
}


int __stdcall aSobel::SetCacheHints(int cachehints,int frame_range)
{
  switch (cachehints)
  {
  case CACHE_GET_MTMODE :
    return MT_MULTI_INSTANCE;
  default :
    return 0;
  }
}


void aSobel::StaticThreadpool(void *ptr)
{
	const Public_MT_Data_Thread *data=(const Public_MT_Data_Thread *)ptr;
	const aSobel *ptrClass=(aSobel *)data->pClass;

	const uint8_t thread_num=data->thread_Id;
	const MT_Data_Info_WarpSharp mt_data_inf=ptrClass->MT_Data[thread_num];
	
	switch(data->f_process)
	{
		case 1 :
			Sobel_8_MT((const unsigned char *)mt_data_inf.src_Y1,(unsigned char *)mt_data_inf.dst_Y1,
				mt_data_inf.src_pitch_Y1,mt_data_inf.dst_pitch_Y1,mt_data_inf.src_Y_h,
				mt_data_inf.row_size_Y1,ptrClass->thresh,mt_data_inf.src_Y_h_min,mt_data_inf.src_Y_h_max);
			break;
		case 2 :
			Sobel_8_MT((const unsigned char *)mt_data_inf.src_U1,(unsigned char *)mt_data_inf.dst_U1,
				mt_data_inf.src_pitch_U1,mt_data_inf.dst_pitch_U1,mt_data_inf.src_U_h,
				mt_data_inf.row_size_U1,ptrClass->threshC,mt_data_inf.src_UV_h_min,mt_data_inf.src_UV_h_max);
			break;
		case 3 :
			Sobel_8_MT((const unsigned char *)mt_data_inf.src_V1,(unsigned char *)mt_data_inf.dst_V1,
				mt_data_inf.src_pitch_V1,mt_data_inf.dst_pitch_V1,mt_data_inf.src_V_h,
				mt_data_inf.row_size_V1,ptrClass->threshC,mt_data_inf.src_UV_h_min,mt_data_inf.src_UV_h_max);
			break;
		default : ;
	}
}


PVideoFrame __stdcall aSobel::GetFrame(int n, IScriptEnvironment *env)
{
  PVideoFrame src = child->GetFrame(n, env);
  PVideoFrame dst = env->NewVideoFrame(vi,64);

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

  const int SubH_Y = vi.GetPlaneHeightSubsampling(PLANAR_Y);
  const int SubH_U = vi.IsY() ? 0:vi.GetPlaneHeightSubsampling(PLANAR_U);
  const int SubH_V = vi.IsY() ? 0:vi.GetPlaneHeightSubsampling(PLANAR_V);
  const int SubW_Y = vi.GetPlaneWidthSubsampling(PLANAR_Y);
  const int SubW_U = vi.IsY() ? 0:vi.GetPlaneWidthSubsampling(PLANAR_U);
  const int SubW_V = vi.IsY() ? 0:vi.GetPlaneWidthSubsampling(PLANAR_V);

  const int32_t src_height_Y = src->GetHeight() >> SubH_Y;
  const int32_t src_height_U = src->GetHeight() >> SubH_U;
  const int32_t src_height_V = src->GetHeight() >> SubH_V;

  const int32_t dst_row_size_Y = dst->GetRowSize() >> SubW_Y;
  const int32_t dst_row_size_U = dst->GetRowSize() >> SubW_U;
  const int32_t dst_row_size_V = dst->GetRowSize() >> SubW_V;

  if (threads_number>1)
  {
	  WaitForSingleObject(ghMutex,INFINITE);

	  if (!poolInterface->RequestThreadPool(UserId,threads_number,MT_Thread,-1,false))
	  {
		  ReleaseMutex(ghMutex);
		  env->ThrowError("aSobel: Error with the TheadPool while requesting threadpool!");
	  }
  }


  if (threads_number>1)
  {

	for(uint8_t i=0; i<threads_number; i++)
	{
		MT_Data[i].src_Y1=(void *)psrc_Y;
		MT_Data[i].src_pitch_Y1=src_pitch_Y;
		MT_Data[i].row_size_Y1=dst_row_size_Y;
		MT_Data[i].dst_Y1=pdst_Y;
		MT_Data[i].dst_pitch_Y1=dst_pitch_Y;

		MT_Data[i].src_U1=(void *)psrc_U;
		MT_Data[i].src_pitch_U1=src_pitch_U;
		MT_Data[i].row_size_U1=dst_row_size_U;
		MT_Data[i].dst_U1=pdst_U;
		MT_Data[i].dst_pitch_U1=dst_pitch_U;

		MT_Data[i].src_V1=(void *)psrc_V;
		MT_Data[i].src_pitch_V1=src_pitch_V;
		MT_Data[i].row_size_V1=dst_row_size_V;
		MT_Data[i].dst_V1=pdst_V;
		MT_Data[i].dst_pitch_V1=dst_pitch_V;

		MT_Data[i].src_Y_h=src_height_Y;
		MT_Data[i].src_U_h=src_height_U;
		MT_Data[i].src_V_h=src_height_V;
	}

	uint8_t f_proc;

  if (chroma < 5)
  {
	  f_proc=1;

	  for(uint8_t i=0; i<threads_number; i++)
		  MT_Thread[i].f_process=f_proc;

	  if (poolInterface->StartThreads(UserId)) poolInterface->WaitThreadsEnd(UserId);
  }
  else
    CopyPlane(src, dst, PLANAR_Y, vi);

  switch (chroma)
  {
  case 0:
    SetPlane(dst, PLANAR_U, 0x80, vi);
    SetPlane(dst, PLANAR_V, 0x80, vi);
    break;
  case 1:
    break;
  case 2:
    CopyPlane(src, dst, PLANAR_U, vi);
    CopyPlane(src, dst, PLANAR_V, vi);
    break;
  default:
	  f_proc=2;

	  for(uint8_t i=0; i<threads_number; i++)
		  MT_Thread[i].f_process=f_proc;

	  if (poolInterface->StartThreads(UserId)) poolInterface->WaitThreadsEnd(UserId);

	  f_proc=3;

	  for(uint8_t i=0; i<threads_number; i++)
		  MT_Thread[i].f_process=f_proc;

	  if (poolInterface->StartThreads(UserId)) poolInterface->WaitThreadsEnd(UserId);
    break;
  }

	for(uint8_t i=0; i<threads_number; i++)
		MT_Thread[i].f_process=0;

    poolInterface->ReleaseThreadPool(UserId,sleep);

	ReleaseMutex(ghMutex);

  }
  else
  {

  if (chroma < 5)
	Sobel_8(psrc_Y,pdst_Y,src_pitch_Y,dst_pitch_Y,src_height_Y,dst_row_size_Y,thresh);
  else
    CopyPlane(src, dst, PLANAR_Y, vi);

  switch (chroma)
  {
  case 0:
    SetPlane(dst, PLANAR_U, 0x80, vi);
    SetPlane(dst, PLANAR_V, 0x80, vi);
    break;
  case 1:
    break;
  case 2:
    CopyPlane(src, dst, PLANAR_U, vi);
    CopyPlane(src, dst, PLANAR_V, vi);
    break;
  default:
	Sobel_8(psrc_U,pdst_U,src_pitch_U,dst_pitch_U,src_height_U,dst_row_size_U,threshC);
	Sobel_8(psrc_V,pdst_V,src_pitch_V,dst_pitch_V,src_height_V,dst_row_size_V,threshC);
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

	if (vi.IsY8()) chroma = 1;
	if (blur_level==-1) blur_level=(blur_type==0)?2:3;
	if (blur_levelV==-1) blur_levelV=blur_level;
	if (blur_levelC==-1) blur_levelC=(blur_level+1)>>1;
	if (blur_levelVC==-1) blur_levelVC=blur_levelC;

	if (!(vi.IsYUV() && vi.IsPlanar())) env->ThrowError("aBlur: Planar YUV input is required");
	if (blur_level<0) env->ThrowError("aBlur: 'blur' must be >=0");
	if (blur_levelV<0) env->ThrowError("aBlur: 'blurV' must be >=0");
	if (blur_levelC<0) env->ThrowError("aBlur: 'blurC' must be >=0");
	if (blur_levelVC<0) env->ThrowError("aBlur: 'blurVC' must be >=0");
	if ((blur_type<0) || (blur_type>1)) env->ThrowError("aBlur: 'type' must be 0,1");
	if ((chroma<0) || (chroma>6)) env->ThrowError("aBlur: 'chroma' must be 0..6");

	StaticThreadpoolF=StaticThreadpool;

	for (uint8_t i=0; i<MAX_MT_THREADS; i++)
	{
		MT_Thread[i].pClass=this;
		MT_Thread[i].f_process=0;
		MT_Thread[i].thread_Id=(uint8_t)i;
		MT_Thread[i].pFunc=StaticThreadpoolF;
	}

	UserId=0;
	ghMutex=NULL;
	
	const int shift_w = (!grey && vi.IsPlanar() && !isRGBPfamily) ? vi.GetPlaneWidthSubsampling(PLANAR_U) : 0;
	const int shift_h = (!grey && vi.IsPlanar() && !isRGBPfamily) ? vi.GetPlaneHeightSubsampling(PLANAR_U) : 0;

	if (vi.height<32) threads_number=1;
	else threads_number=threads;
	
	ghMutex=CreateMutex(NULL,FALSE,NULL);
	if (ghMutex==NULL) env->ThrowError("aBlur: Unable to create Mutex!");

	threads_number=CreateMTData(MT_Data,threads_number,threads_number,vi.width,vi.height,shift_w,shift_h);

	if (threads_number>1)
	{
		if (!poolInterface->GetUserId(UserId))
		{
			poolInterface->DeAllocateAllThreads(true);
			FreeData();
			env->ThrowError("aBlur: Error with the TheadPool while getting UserId!");
		}
	}
}


void aBlur::FreeData(void)
{
	myCloseHandle(ghMutex);
}


aBlur::~aBlur()
{
	if (threads_number>1)
	{
		poolInterface->RemoveUserId(UserId);
		poolInterface->DeAllocateAllThreads(true);
	}
	FreeData();
}


int __stdcall aBlur::SetCacheHints(int cachehints,int frame_range)
{
  switch (cachehints)
  {
  case CACHE_GET_MTMODE :
    return MT_MULTI_INSTANCE;
  default :
    return 0;
  }
}


void aBlur::StaticThreadpool(void *ptr)
{
	const Public_MT_Data_Thread *data=(const Public_MT_Data_Thread *)ptr;
	const aBlur *ptrClass=(aBlur *)data->pClass;

	const uint8_t thread_num=data->thread_Id;
	const MT_Data_Info_WarpSharp mt_data_inf=ptrClass->MT_Data[thread_num];
	
	switch(data->f_process)
	{
		case 1 :
			BlurR2_8_MT((unsigned char *const)mt_data_inf.src_Y1,(unsigned char *const)mt_data_inf.dst_Y1,
				mt_data_inf.src_pitch_Y1,mt_data_inf.dst_pitch_Y1,mt_data_inf.src_Y_h,mt_data_inf.row_size_Y1,true,true,
				mt_data_inf.src_Y_h_min,mt_data_inf.src_Y_h_max);
			break;
		case 2 :
			BlurR6_8_MT((unsigned char *const)mt_data_inf.src_Y1,(unsigned char *const)mt_data_inf.dst_Y1,
				mt_data_inf.src_pitch_Y1,mt_data_inf.dst_pitch_Y1,mt_data_inf.src_Y_h,mt_data_inf.row_size_Y1,true,true,
				mt_data_inf.src_Y_h_min,mt_data_inf.src_Y_h_max);
			break;
		case 3 :
			BlurR2_8_MT((unsigned char *const)mt_data_inf.src_Y1,(unsigned char *const)mt_data_inf.dst_Y1,
				mt_data_inf.src_pitch_Y1,mt_data_inf.dst_pitch_Y1,mt_data_inf.src_Y_h,mt_data_inf.row_size_Y1,
				mt_data_inf.processH,mt_data_inf.processV,mt_data_inf.src_Y_h_min,mt_data_inf.src_Y_h_max);
			break;
		case 4 :
			BlurR6_8_MT((unsigned char *const)mt_data_inf.src_Y1,(unsigned char *const)mt_data_inf.dst_Y1,
				mt_data_inf.src_pitch_Y1,mt_data_inf.dst_pitch_Y1,mt_data_inf.src_Y_h,mt_data_inf.row_size_Y1,
				mt_data_inf.processH,mt_data_inf.processV,mt_data_inf.src_Y_h_min,mt_data_inf.src_Y_h_max);
			break;
		case 5 :
			BlurR2_8_MT((unsigned char *const)mt_data_inf.src_U1,(unsigned char *const)mt_data_inf.dst_U1,
				mt_data_inf.src_pitch_U1,mt_data_inf.dst_pitch_U1,mt_data_inf.src_U_h,mt_data_inf.row_size_U1,true,true,
				mt_data_inf.src_UV_h_min,mt_data_inf.src_UV_h_max);
			break;
		case 6 :
			BlurR6_8_MT((unsigned char *const)mt_data_inf.src_U1,(unsigned char *const)mt_data_inf.dst_U1,
				mt_data_inf.src_pitch_U1,mt_data_inf.dst_pitch_U1,mt_data_inf.src_U_h,mt_data_inf.row_size_U1,true,true,
				mt_data_inf.src_UV_h_min,mt_data_inf.src_UV_h_max);
			break;
		case 7 :
			BlurR2_8_MT((unsigned char *const)mt_data_inf.src_U1,(unsigned char *const)mt_data_inf.dst_U1,
				mt_data_inf.src_pitch_U1,mt_data_inf.dst_pitch_U1,mt_data_inf.src_U_h,mt_data_inf.row_size_U1,
				mt_data_inf.cprocessH,mt_data_inf.cprocessV,mt_data_inf.src_UV_h_min,mt_data_inf.src_UV_h_max);
			break;
		case 8 :
			BlurR6_8_MT((unsigned char *const)mt_data_inf.src_U1,(unsigned char *const)mt_data_inf.dst_U1,
				mt_data_inf.src_pitch_U1,mt_data_inf.dst_pitch_U1,mt_data_inf.src_U_h,mt_data_inf.row_size_U1,
				mt_data_inf.cprocessH,mt_data_inf.cprocessV,mt_data_inf.src_UV_h_min,mt_data_inf.src_UV_h_max);
			break;
		case 9 :
			BlurR2_8_MT((unsigned char *const)mt_data_inf.src_V1,(unsigned char *const)mt_data_inf.dst_V1,
				mt_data_inf.src_pitch_V1,mt_data_inf.dst_pitch_V1,mt_data_inf.src_V_h,mt_data_inf.row_size_V1,true,true,
				mt_data_inf.src_UV_h_min,mt_data_inf.src_UV_h_max);
			break;
		case 10 :
			BlurR6_8_MT((unsigned char *const)mt_data_inf.src_V1,(unsigned char *const)mt_data_inf.dst_V1,
				mt_data_inf.src_pitch_V1,mt_data_inf.dst_pitch_V1,mt_data_inf.src_V_h,mt_data_inf.row_size_V1,true,true,
				mt_data_inf.src_UV_h_min,mt_data_inf.src_UV_h_max);
			break;
		case 11 :
			BlurR2_8_MT((unsigned char *const)mt_data_inf.src_V1,(unsigned char *const)mt_data_inf.dst_V1,
				mt_data_inf.src_pitch_V1,mt_data_inf.dst_pitch_V1,mt_data_inf.src_V_h,mt_data_inf.row_size_V1,
				mt_data_inf.cprocessH,mt_data_inf.cprocessV,mt_data_inf.src_UV_h_min,mt_data_inf.src_UV_h_max);
			break;
		case 12 :
			BlurR6_8_MT((unsigned char *const)mt_data_inf.src_V1,(unsigned char *const)mt_data_inf.dst_V1,
				mt_data_inf.src_pitch_V1,mt_data_inf.dst_pitch_V1,mt_data_inf.src_V_h,mt_data_inf.row_size_V1,
				mt_data_inf.cprocessH,mt_data_inf.cprocessV,mt_data_inf.src_UV_h_min,mt_data_inf.src_UV_h_max);
			break;
		default : ;
	}
}


PVideoFrame __stdcall aBlur::GetFrame(int n, IScriptEnvironment *env)
{
  PVideoFrame src = child->GetFrame(n,env);
  PVideoFrame tmp = env->NewVideoFrame(vi,64);
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

  const int SubH_Y = vi.GetPlaneHeightSubsampling(PLANAR_Y);
  const int SubH_U = vi.IsY() ? 0:vi.GetPlaneHeightSubsampling(PLANAR_U);
  const int SubH_V = vi.IsY() ? 0:vi.GetPlaneHeightSubsampling(PLANAR_V);
  const int SubW_Y = vi.GetPlaneWidthSubsampling(PLANAR_Y);
  const int SubW_U = vi.IsY() ? 0:vi.GetPlaneWidthSubsampling(PLANAR_U);
  const int SubW_V = vi.IsY() ? 0:vi.GetPlaneWidthSubsampling(PLANAR_V);

  const int32_t src_height_Y = src->GetHeight() >> SubH_Y;
  const int32_t src_height_U = src->GetHeight() >> SubH_U;
  const int32_t src_height_V = src->GetHeight() >> SubH_V;

  const int32_t src_row_size_Y = src->GetRowSize() >> SubW_Y;
  const int32_t src_row_size_U = src->GetRowSize() >> SubW_U;
  const int32_t src_row_size_V = src->GetRowSize() >> SubW_V;

 
  const int blurL=std::min(blur_level,blur_levelV);
  const int blurLr=std::max(blur_level,blur_levelV)-blurL;
  const bool processH=blur_level>blurL,processV=blur_levelV>blurL;

  const int cblurL=std::min(blur_levelC,blur_levelVC);
  const int cblurLr=std::max(blur_levelC,blur_levelVC)-cblurL;
  const bool cprocessH=blur_levelC>cblurL,cprocessV=blur_levelVC>cblurL;

  if (threads_number>1)
  {
	  WaitForSingleObject(ghMutex,INFINITE);

	  if (!poolInterface->RequestThreadPool(UserId,threads_number,MT_Thread,-1,false))
	  {
		  ReleaseMutex(ghMutex);
		  env->ThrowError("aBlur: Error with the TheadPool while requesting threadpool!");
	  }
  }

  if (threads_number>1)
  {

	for(uint8_t i=0; i<threads_number; i++)
	{
		MT_Data[i].src_Y1=wpsrc_Y;
		MT_Data[i].src_pitch_Y1=src_pitch_Y;
		MT_Data[i].row_size_Y1=src_row_size_Y;
		MT_Data[i].dst_Y1=wptmp_Y;
		MT_Data[i].dst_pitch_Y1=tmp_pitch_Y;

		MT_Data[i].src_U1=wpsrc_U;
		MT_Data[i].src_pitch_U1=src_pitch_U;
		MT_Data[i].row_size_U1=src_row_size_U;
		MT_Data[i].dst_U1=wptmp_U;
		MT_Data[i].dst_pitch_U1=tmp_pitch_U;

		MT_Data[i].src_V1=wpsrc_V;
		MT_Data[i].src_pitch_V1=src_pitch_V;
		MT_Data[i].row_size_V1=src_row_size_V;
		MT_Data[i].dst_V1=wptmp_V;
		MT_Data[i].dst_pitch_V1=tmp_pitch_V;

		MT_Data[i].src_Y_h=src_height_Y;
		MT_Data[i].src_U_h=src_height_U;
		MT_Data[i].src_V_h=src_height_V;

		MT_Data[i].processH=processH;
		MT_Data[i].processV=processV;
		MT_Data[i].cprocessH=cprocessH;
		MT_Data[i].cprocessV=cprocessV;
	}

	  uint8_t f_proc;

  if (chroma < 5)
  {
	  f_proc=blur_type ? 1:2;

	  for(uint8_t i=0; i<threads_number; i++)
		  MT_Thread[i].f_process=f_proc;

	for (int i=0; i<blurL; i++)
		if (poolInterface->StartThreads(UserId)) poolInterface->WaitThreadsEnd(UserId);

	  f_proc=blur_type ? 3:4;

	  for(uint8_t i=0; i<threads_number; i++)
		  MT_Thread[i].f_process=f_proc;

	for (int i=0; i<blurLr; i++)
		if (poolInterface->StartThreads(UserId)) poolInterface->WaitThreadsEnd(UserId);
  }
  switch (chroma)
  {
  case 0:
    SetPlane(src, PLANAR_U, 0x80, vi);
    SetPlane(src, PLANAR_V, 0x80, vi);
    break;
  case 1:
  case 2:
    break;
  default:
	  f_proc=blur_type ? 5:6;

	  for(uint8_t i=0; i<threads_number; i++)
		  MT_Thread[i].f_process=f_proc;

	for (int i=0; i<cblurL; i++)
		if (poolInterface->StartThreads(UserId)) poolInterface->WaitThreadsEnd(UserId);

	  f_proc=blur_type ? 7:8;

	  for(uint8_t i=0; i<threads_number; i++)
		  MT_Thread[i].f_process=f_proc;

	for (int i=0; i<cblurLr; i++)
		if (poolInterface->StartThreads(UserId)) poolInterface->WaitThreadsEnd(UserId);

	  f_proc=blur_type ? 9:10;

	  for(uint8_t i=0; i<threads_number; i++)
		  MT_Thread[i].f_process=f_proc;

	for (int i=0; i<cblurL; i++)
		if (poolInterface->StartThreads(UserId)) poolInterface->WaitThreadsEnd(UserId);

	  f_proc=blur_type ? 11:12;

	  for(uint8_t i=0; i<threads_number; i++)
		  MT_Thread[i].f_process=f_proc;

	for (int i=0; i<cblurLr; i++)
		if (poolInterface->StartThreads(UserId)) poolInterface->WaitThreadsEnd(UserId);
    break;
  }

	for(uint8_t i=0; i<threads_number; i++)
		MT_Thread[i].f_process=0;

    poolInterface->ReleaseThreadPool(UserId,sleep);

	ReleaseMutex(ghMutex);

  }
  else
  {

  if (chroma < 5)
  {
	for (int i=0; i<blurL; i++)
	{
		if (blur_type) BlurR2_8(wpsrc_Y,wptmp_Y,src_pitch_Y,tmp_pitch_Y,src_height_Y,src_row_size_Y,true,true);
		else BlurR6_8(wpsrc_Y,wptmp_Y,src_pitch_Y,tmp_pitch_Y,src_height_Y,src_row_size_Y,true,true);
	}
	for (int i=0; i<blurLr; i++)
	{
		if (blur_type) BlurR2_8(wpsrc_Y,wptmp_Y,src_pitch_Y,tmp_pitch_Y,src_height_Y,src_row_size_Y,processH,processV);
		else BlurR6_8(wpsrc_Y,wptmp_Y,src_pitch_Y,tmp_pitch_Y,src_height_Y,src_row_size_Y,processH,processV);
	}
  }
  switch (chroma)
  {
  case 0:
    SetPlane(src, PLANAR_U, 0x80, vi);
    SetPlane(src, PLANAR_V, 0x80, vi);
    break;
  case 1:
  case 2:
    break;
  default:
	for (int i=0; i<cblurL; i++)
	{
		if (blur_type) BlurR2_8(wpsrc_U,wptmp_U,src_pitch_U,tmp_pitch_U,src_height_U,src_row_size_U,true,true);
		else BlurR6_8(wpsrc_U,wptmp_U,src_pitch_U,tmp_pitch_U,src_height_U,src_row_size_U,true,true);
	}
	for (int i=0; i<cblurLr; i++)
	{
		if (blur_type) BlurR2_8(wpsrc_U,wptmp_U,src_pitch_U,tmp_pitch_U,src_height_U,src_row_size_U,cprocessH,cprocessV);
		else BlurR6_8(wpsrc_U,wptmp_U,src_pitch_U,tmp_pitch_U,src_height_U,src_row_size_U,cprocessH,cprocessV);
	}
	for (int i=0; i<cblurL; i++)
	{
		if (blur_type) BlurR2_8(wpsrc_V,wptmp_V,src_pitch_V,tmp_pitch_V,src_height_V,src_row_size_V,true,true);
		else BlurR6_8(wpsrc_V,wptmp_V,src_pitch_V,tmp_pitch_V,src_height_V,src_row_size_V,true,true);
	}
	for (int i=0; i<cblurLr; i++)
	{
		if (blur_type) BlurR2_8(wpsrc_V,wptmp_V,src_pitch_V,tmp_pitch_V,src_height_V,src_row_size_V,cprocessH,cprocessV);
		else BlurR6_8(wpsrc_V,wptmp_V,src_pitch_V,tmp_pitch_V,src_height_V,src_row_size_V,cprocessH,cprocessV);
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

	if (depthV==128) depthV=depth;
    if (depthC==128) depthC = vi.IsYV24() ? depth:(depth>>1);
	if (depthVC==128) depthVC = depthC;
    if (vi.IsY8()) chroma = 1;

	if (!(vi.IsYUV() && vi.IsPlanar() && vi2.IsYUV() && vi2.IsPlanar())) env->ThrowError("aWarp: Planar YUV input is required");
	if ((depth<-128) || (depth>127)) env->ThrowError("aWarp: 'depth' must be -128..127");
	if ((depthC<-128) || (depthC>127)) env->ThrowError("aWarp: 'depthC' must be -128..127");
	if ((chroma<0) || (chroma>6)) env->ThrowError("aWarp: 'chroma' must be 0..6");
	if ((depthV<-128) || (depthV>127)) env->ThrowError("aWarp: 'depthV' must be -128..127");
	if ((depthVC<-128) || (depthVC>127)) env->ThrowError("aWarp: 'depthVC' must be -128..127");

    if ((vi.width!=vi2.width) || (vi.height!=vi2.height))
		env->ThrowError("aWarp: both sources must have the same width and height");
    if (vi.pixel_type!=vi2.pixel_type) env->ThrowError("aWarp: both sources must have the colorspace");

	StaticThreadpoolF=StaticThreadpool;

	for (uint8_t i=0; i<MAX_MT_THREADS; i++)
	{
		MT_Thread[i].pClass=this;
		MT_Thread[i].f_process=0;
		MT_Thread[i].thread_Id=(uint8_t)i;
		MT_Thread[i].pFunc=StaticThreadpoolF;
	}

	UserId=0;
	ghMutex=NULL;
	
	const int shift_w = (!grey && vi.IsPlanar() && !isRGBPfamily) ? vi.GetPlaneWidthSubsampling(PLANAR_U) : 0;
	const int shift_h = (!grey && vi.IsPlanar() && !isRGBPfamily) ? vi.GetPlaneHeightSubsampling(PLANAR_U) : 0;

	if (vi.height<32) threads_number=1;
	else threads_number=threads;
	
	ghMutex=CreateMutex(NULL,FALSE,NULL);
	if (ghMutex==NULL) env->ThrowError("aWarp: Unable to create Mutex!");

	threads_number=CreateMTData(MT_Data,threads_number,threads_number,vi.width,vi.height,shift_w,shift_h);

	if (threads_number>1)
	{
		if (!poolInterface->GetUserId(UserId))
		{
			poolInterface->DeAllocateAllThreads(true);
			FreeData();
			env->ThrowError("aWarp: Error with the TheadPool while getting UserId!");
		}
	}
}


void aWarp::FreeData(void)
{
	myCloseHandle(ghMutex);
}


aWarp::~aWarp()
{
	if (threads_number>1)
	{
		poolInterface->RemoveUserId(UserId);
		poolInterface->DeAllocateAllThreads(true);
	}
	FreeData();
}


int __stdcall aWarp::SetCacheHints(int cachehints,int frame_range)
{
  switch (cachehints)
  {
  case CACHE_GET_MTMODE :
    return MT_MULTI_INSTANCE;
  default :
    return 0;
  }
}


void aWarp::StaticThreadpool(void *ptr)
{
	const Public_MT_Data_Thread *data=(const Public_MT_Data_Thread *)ptr;
	const aWarp *ptrClass=(aWarp *)data->pClass;

	const uint8_t thread_num=data->thread_Id;
	const MT_Data_Info_WarpSharp mt_data_inf=ptrClass->MT_Data[thread_num];
	
	switch(data->f_process)
	{
		case 1 :
			Warp0_8_MT((const unsigned char*)mt_data_inf.src_Y1,(const unsigned char*)mt_data_inf.src_Y2,
				(unsigned char*)mt_data_inf.dst_Y1,
				mt_data_inf.src_pitch_Y1,mt_data_inf.src_pitch_Y2,mt_data_inf.dst_pitch_Y1,mt_data_inf.row_size_Y1,
				mt_data_inf.dst_Y_h,ptrClass->depth,ptrClass->depthV,mt_data_inf.src_Y_h_min,mt_data_inf.src_Y_h_max);
			break;
		case 2 :
			Warp0_8_MT((const unsigned char*)mt_data_inf.src_U1,(const unsigned char*)mt_data_inf.src_U2,
				(unsigned char*)mt_data_inf.dst_U2,
				mt_data_inf.src_pitch_U1,mt_data_inf.src_pitch_U2,mt_data_inf.dst_pitch_U1,mt_data_inf.row_size_U1,
				mt_data_inf.dst_U_h,ptrClass->depthC,ptrClass->depthVC,mt_data_inf.src_UV_h_min,mt_data_inf.src_UV_h_max);
			break;
		case 3 :
			Warp0_8_MT((const unsigned char*)mt_data_inf.src_V1,(const unsigned char*)mt_data_inf.src_V2,
				(unsigned char*)mt_data_inf.dst_V1,
				mt_data_inf.src_pitch_V1,mt_data_inf.src_pitch_V2,mt_data_inf.dst_pitch_V1,mt_data_inf.row_size_V1,
				mt_data_inf.dst_U_h,ptrClass->depthC,ptrClass->depthVC,mt_data_inf.src_UV_h_min,mt_data_inf.src_UV_h_max);
			break;
		case 4 :
			GuideChroma_8_MT((const unsigned char*)mt_data_inf.src_Y2,(unsigned char*)mt_data_inf.dst_U1,
				mt_data_inf.src_pitch_Y2,mt_data_inf.src_pitch_U2,mt_data_inf.src_U_h,mt_data_inf.row_size_U2,
				mt_data_inf.SubW_U,mt_data_inf.SubH_U,ptrClass->cplace_mpeg2_flag,
				mt_data_inf.dst_UV_h_min,mt_data_inf.dst_UV_h_max);
			break;
		case 5 :
			Warp0_8_MT((const unsigned char*)mt_data_inf.src_U1,(const unsigned char*)mt_data_inf.src_U2,
				(unsigned char*)mt_data_inf.dst_U2,
				mt_data_inf.src_pitch_U1,mt_data_inf.src_pitch_U2,mt_data_inf.dst_pitch_U1,mt_data_inf.row_size_U1,
				mt_data_inf.dst_U_h,ptrClass->depthC,ptrClass->depthVC,mt_data_inf.src_UV_h_min,mt_data_inf.src_UV_h_max);
			break;
		case 6 :
			Warp0_8_MT((const unsigned char*)mt_data_inf.src_V1,(const unsigned char*)mt_data_inf.src_U2,
				(unsigned char*)mt_data_inf.dst_V1,
				mt_data_inf.src_pitch_V1,mt_data_inf.src_pitch_U2,mt_data_inf.dst_pitch_V1,mt_data_inf.row_size_V1,
				mt_data_inf.dst_V_h,ptrClass->depthC,ptrClass->depthVC,mt_data_inf.src_UV_h_min,mt_data_inf.src_UV_h_max);
			break;
		case 7 :
			Warp0_8_MT((const unsigned char*)mt_data_inf.src_U1,(const unsigned char*)mt_data_inf.src_Y2,
				(unsigned char*)mt_data_inf.dst_U2,
				mt_data_inf.src_pitch_U1,mt_data_inf.src_pitch_Y2,mt_data_inf.dst_pitch_U1,mt_data_inf.row_size_U1,
				mt_data_inf.dst_U_h,ptrClass->depthC,ptrClass->depthVC,mt_data_inf.src_UV_h_min,mt_data_inf.src_UV_h_max);
			break;
		case 8 :
			Warp0_8_MT((const unsigned char*)mt_data_inf.src_V1,(const unsigned char*)mt_data_inf.src_Y2,
				(unsigned char*)mt_data_inf.dst_V1,
				mt_data_inf.src_pitch_V1,mt_data_inf.src_pitch_Y2,mt_data_inf.dst_pitch_V1,mt_data_inf.row_size_V1,
				mt_data_inf.dst_V_h,ptrClass->depthC,ptrClass->depthVC,mt_data_inf.src_UV_h_min,mt_data_inf.src_UV_h_max);
			break;
		default : ;
	}
}


PVideoFrame __stdcall aWarp::GetFrame(int n, IScriptEnvironment *env)
{
  PVideoFrame src = child->GetFrame(n, env);
  PVideoFrame edg = edges->GetFrame(n, env);
  PVideoFrame dst = env->NewVideoFrame(vi,64);

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

  const int SubH_Y = vi.GetPlaneHeightSubsampling(PLANAR_Y);
  const int SubH_U = vi.IsY() ? 0:vi.GetPlaneHeightSubsampling(PLANAR_U);
  const int SubH_V = vi.IsY() ? 0:vi.GetPlaneHeightSubsampling(PLANAR_V);
  const int SubW_Y = vi.GetPlaneWidthSubsampling(PLANAR_Y);
  const int SubW_U = vi.IsY() ? 0:vi.GetPlaneWidthSubsampling(PLANAR_U);
  const int SubW_V = vi.IsY() ? 0:vi.GetPlaneWidthSubsampling(PLANAR_V);

  const int32_t dst_height_Y = dst->GetHeight() >> SubH_Y;
  const int32_t dst_height_U = dst->GetHeight() >> SubH_U;
  const int32_t dst_height_V = dst->GetHeight() >> SubH_V;
  const int32_t dst_row_size_Y = dst->GetRowSize() >> SubW_Y;
  const int32_t dst_row_size_U = dst->GetRowSize() >> SubW_U;
  const int32_t dst_row_size_V = dst->GetRowSize() >> SubW_V;

  const int32_t edg_height_UV = edg->GetHeight() >> SubH_U;
  const int32_t edg_width_UV = edg->GetRowSize() >> SubW_U;

  if (threads_number>1)
  {
	  WaitForSingleObject(ghMutex,INFINITE);

	  if (!poolInterface->RequestThreadPool(UserId,threads_number,MT_Thread,-1,false))
	  {
		  ReleaseMutex(ghMutex);
		  env->ThrowError("aWarp: Error with the TheadPool while requesting threadpool!");
	  }
  }


  if (threads_number>1)
  {

	for(uint8_t i=0; i<threads_number; i++)
	{
		MT_Data[i].src_Y1=(void *)psrc_Y;
		MT_Data[i].src_Y2=(void *)pedg_Y;
		MT_Data[i].src_pitch_Y1=src_pitch_Y;
		MT_Data[i].src_pitch_Y2=edg_pitch_Y;
		MT_Data[i].row_size_Y1=dst_row_size_Y;
		MT_Data[i].dst_Y1=pdst_Y;
		MT_Data[i].dst_pitch_Y1=dst_pitch_Y;

		MT_Data[i].src_U1=(void *)psrc_U;
		MT_Data[i].src_U2=(void *)pedg_U;
		MT_Data[i].src_pitch_U1=src_pitch_U;
		MT_Data[i].src_pitch_U2=edg_pitch_U;
		MT_Data[i].row_size_U1=dst_row_size_U;
		MT_Data[i].row_size_U2=edg_width_UV;
		MT_Data[i].dst_U1=dpedg_U;
		MT_Data[i].dst_U2=pdst_U;
		MT_Data[i].dst_pitch_U1=dst_pitch_U;

		MT_Data[i].src_V1=(void *)psrc_V;
		MT_Data[i].src_V2=(void *)pedg_V;
		MT_Data[i].src_pitch_V1=src_pitch_V;
		MT_Data[i].src_pitch_V2=edg_pitch_V;
		MT_Data[i].row_size_V1=dst_row_size_V;
		MT_Data[i].dst_V1=pdst_V;
		MT_Data[i].dst_pitch_V1=dst_pitch_V;

		MT_Data[i].src_U_h=edg_height_UV;
		MT_Data[i].dst_Y_h=dst_height_Y;
		MT_Data[i].dst_U_h=dst_height_U;
		MT_Data[i].dst_V_h=dst_height_V;

		MT_Data[i].SubW_U=SubW_U;
		MT_Data[i].SubH_U=SubH_U;
	}

	uint8_t f_proc;

  if ((chroma<5) && ((depth!=0) || (depthV!=0)))
  {
	  f_proc=1;

	  for(uint8_t i=0; i<threads_number; i++)
		  MT_Thread[i].f_process=f_proc;

	  if (poolInterface->StartThreads(UserId)) poolInterface->WaitThreadsEnd(UserId);
  }
  else
    CopyPlane(src, dst, PLANAR_Y, vi);

  switch (chroma)
  {
  case 0:
    SetPlane(dst, PLANAR_U, 0x80, vi);
    SetPlane(dst, PLANAR_V, 0x80, vi);
    break;
  case 2:
    CopyPlane(src, dst, PLANAR_U, vi);
    CopyPlane(src, dst, PLANAR_V, vi);
    break;
  case 3:
  case 5:
	  if ((depthC!=0) || (depthVC!=0))
	  {
		  f_proc=2;

		  for(uint8_t i=0; i<threads_number; i++)
			  MT_Thread[i].f_process=f_proc;

		  if (poolInterface->StartThreads(UserId)) poolInterface->WaitThreadsEnd(UserId);

		  f_proc=3;

		  for(uint8_t i=0; i<threads_number; i++)
			  MT_Thread[i].f_process=f_proc;

		  if (poolInterface->StartThreads(UserId)) poolInterface->WaitThreadsEnd(UserId);
	  }
	  else
	  {
			CopyPlane(src, dst, PLANAR_U, vi);
			CopyPlane(src, dst, PLANAR_V, vi);
	  }
    break;
  case 4:
  case 6:
	   if ((depthC!=0) || (depthVC!=0))
	   {
    if (!vi.IsYV24())
    {
	  if (!GuideChroma_8_Test(SubW_U,SubH_U))
	  {
		  ReleaseMutex(ghMutex);
		  env->ThrowError("aWarp: Unsuported colorspace");
	  }

      if (edg->IsWritable())
	  {
		  f_proc=4;

		  for(uint8_t i=0; i<threads_number; i++)
			  MT_Thread[i].f_process=f_proc;

		  if (poolInterface->StartThreads(UserId)) poolInterface->WaitThreadsEnd(UserId);
	  }
      else
      {
        PVideoFrame tmp = env->NewVideoFrame(vi,64);

		unsigned char *dptmp_U = tmp->GetWritePtr(PLANAR_U);

		edg_pitch_U = tmp->GetPitch(PLANAR_U);
		pedg_U = tmp->GetReadPtr(PLANAR_U);
		
		f_proc=4;

		for(uint8_t i=0; i<threads_number; i++)
		{
			MT_Thread[i].f_process=f_proc;

			MT_Data[i].dst_U1=dptmp_U;
			MT_Data[i].src_U2=(void *)pedg_U;
			MT_Data[i].src_pitch_U2=edg_pitch_U;
		}

		if (poolInterface->StartThreads(UserId)) poolInterface->WaitThreadsEnd(UserId);
      }

	  f_proc=5;

	  for(uint8_t i=0; i<threads_number; i++)
		  MT_Thread[i].f_process=f_proc;

	  if (poolInterface->StartThreads(UserId)) poolInterface->WaitThreadsEnd(UserId);

	  f_proc=6;

	  for(uint8_t i=0; i<threads_number; i++)
		  MT_Thread[i].f_process=f_proc;

	  if (poolInterface->StartThreads(UserId)) poolInterface->WaitThreadsEnd(UserId);
    }
    else
    {
	  f_proc=7;

	  for(uint8_t i=0; i<threads_number; i++)
		  MT_Thread[i].f_process=f_proc;

	  if (poolInterface->StartThreads(UserId)) poolInterface->WaitThreadsEnd(UserId);

	  f_proc=8;

	  for(uint8_t i=0; i<threads_number; i++)
		  MT_Thread[i].f_process=f_proc;

	  if (poolInterface->StartThreads(UserId)) poolInterface->WaitThreadsEnd(UserId);
    }
	   }
	   else
	   {
		    CopyPlane(src,dst,PLANAR_U,vi);
			CopyPlane(src,dst,PLANAR_V,vi);
	   }
    break;
  }

	for(uint8_t i=0; i<threads_number; i++)
		MT_Thread[i].f_process=0;

    poolInterface->ReleaseThreadPool(UserId,sleep);

	ReleaseMutex(ghMutex);

  }
  else
  {

  if ((chroma<5) && ((depth!=0) || (depthV!=0)))
	Warp0_8(psrc_Y,pedg_Y,pdst_Y,src_pitch_Y,edg_pitch_Y,dst_pitch_Y,dst_row_size_Y,dst_height_Y,depth,depthV);
  else
    CopyPlane(src, dst, PLANAR_Y, vi);

  switch (chroma)
  {
  case 0:
    SetPlane(dst, PLANAR_U, 0x80, vi);
    SetPlane(dst, PLANAR_V, 0x80, vi);
    break;
  case 2:
    CopyPlane(src, dst, PLANAR_U, vi);
    CopyPlane(src, dst, PLANAR_V, vi);
    break;
  case 3:
  case 5:
	  if ((depthC!=0) || (depthVC!=0))
	  {
			Warp0_8(psrc_U,pedg_U,pdst_U,src_pitch_U,edg_pitch_U,dst_pitch_U,dst_row_size_U,dst_height_U,depthC,depthVC);
			Warp0_8(psrc_V,pedg_V,pdst_V,src_pitch_V,edg_pitch_V,dst_pitch_V,dst_row_size_V,dst_height_V,depthC,depthVC);
	  }
	  else
	  {
			CopyPlane(src, dst, PLANAR_U, vi);
			CopyPlane(src, dst, PLANAR_V, vi);
	  }
    break;
  case 4:
  case 6:
	   if ((depthC!=0) || (depthVC!=0))
	   {
    if (!vi.IsYV24())
    {
      if (edg->IsWritable())
	  {
		  if (!GuideChroma_8(pedg_Y,dpedg_U,edg_pitch_Y,edg_pitch_U,edg_height_UV,edg_width_UV,SubW_U,
			  SubH_U,cplace_mpeg2_flag)) env->ThrowError("aWarp: Unsuported colorspace");
	  }
      else
      {
        PVideoFrame tmp = env->NewVideoFrame(vi,64);

		const int32_t tmp_pitch_U = tmp->GetPitch(PLANAR_U);
		unsigned char *dptmp_U = tmp->GetWritePtr(PLANAR_U);
		const int32_t tmp_height_UV = tmp->GetHeight() >> SubH_U;
		const int32_t tmp_width_UV = tmp->GetRowSize() >> SubW_U;

		if (!GuideChroma_8(pedg_Y,dptmp_U,edg_pitch_Y,tmp_pitch_U,tmp_height_UV,tmp_width_UV,SubW_U,
			  SubH_U,cplace_mpeg2_flag)) env->ThrowError("aWarp: Unsuported colorspace");

		pedg_U = tmp->GetReadPtr(PLANAR_U);
		edg_pitch_U = tmp_pitch_U;
      }
	  Warp0_8(psrc_U,pedg_U,pdst_U,src_pitch_U,edg_pitch_U,dst_pitch_U,dst_row_size_U,dst_height_U,depthC,depthVC);
	  Warp0_8(psrc_V,pedg_U,pdst_V,src_pitch_V,edg_pitch_U,dst_pitch_V,dst_row_size_V,dst_height_V,depthC,depthVC);
    }
    else
    {
		Warp0_8(psrc_U,pedg_Y,pdst_U,src_pitch_U,edg_pitch_Y,dst_pitch_U,dst_row_size_U,dst_height_U,depthC,depthVC);
		Warp0_8(psrc_V,pedg_Y,pdst_V,src_pitch_V,edg_pitch_Y,dst_pitch_V,dst_row_size_V,dst_height_V,depthC,depthVC);
    }
	   }
	   else
	   {
		    CopyPlane(src,dst,PLANAR_U,vi);
			CopyPlane(src,dst,PLANAR_V,vi);
	   }
    break;
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

	if (depthV==128) depthV=depth;
    if (depthC==128) depthC = vi.IsYV24() ? depth:(depth>>1);
	if (depthVC==128) depthVC = depthC;
    if (vi.IsY8()) chroma = 1;

	if (!(vi.IsYUV() && vi.IsPlanar() && vi2.IsYUV() && vi2.IsPlanar())) env->ThrowError("aWarp4: Planar YUV input is required");
	if ((depth<-128) || (depth>127)) env->ThrowError("aWarp4: 'depth' must be -128..127");
	if ((depthC<-128) || (depthC>127)) env->ThrowError("aWarp4: 'depthC' must be -128..127");
	if ((chroma<0) || (chroma>6)) env->ThrowError("aWarp4: 'chroma' must be 0..6");
	if ((depthV<-128) || (depthV>127)) env->ThrowError("aWarp4: 'depthV' must be -128..127");
	if ((depthVC<-128) || (depthVC>127)) env->ThrowError("aWarp4: 'depthVC' must be -128..127");

    if ((vi.width!=(vi2.width<<2))|| (vi.height!=(vi2.height<<2)))
      env->ThrowError("aWarp4: first source must be excatly 4 times width and height of second source");
    if (vi.pixel_type!=vi2.pixel_type) env->ThrowError("aWarp4: both sources must have the colorspace");

    vi=vi2;

	StaticThreadpoolF=StaticThreadpool;

	for (uint8_t i=0; i<MAX_MT_THREADS; i++)
	{
		MT_Thread[i].pClass=this;
		MT_Thread[i].f_process=0;
		MT_Thread[i].thread_Id=(uint8_t)i;
		MT_Thread[i].pFunc=StaticThreadpoolF;
	}

	UserId=0;
	ghMutex=NULL;
	
	const int shift_w = (!grey && vi.IsPlanar() && !isRGBPfamily) ? vi.GetPlaneWidthSubsampling(PLANAR_U) : 0;
	const int shift_h = (!grey && vi.IsPlanar() && !isRGBPfamily) ? vi.GetPlaneHeightSubsampling(PLANAR_U) : 0;

	if (vi.height<32) threads_number=1;
	else threads_number=threads;
	
	ghMutex=CreateMutex(NULL,FALSE,NULL);
	if (ghMutex==NULL) env->ThrowError("aWarp4: Unable to create Mutex!");

	threads_number=CreateMTData(MT_Data,threads_number,threads_number,vi.width,vi.height,shift_w,shift_h);

	if (threads_number>1)
	{
		if (!poolInterface->GetUserId(UserId))
		{
			poolInterface->DeAllocateAllThreads(true);
			FreeData();
			env->ThrowError("aWarp4: Error with the TheadPool while getting UserId!");
		}
	}
}


void aWarp4::FreeData(void)
{
	myCloseHandle(ghMutex);
}


aWarp4::~aWarp4()
{
	if (threads_number>1)
	{
		poolInterface->RemoveUserId(UserId);
		poolInterface->DeAllocateAllThreads(true);
	}
	FreeData();
}


int __stdcall aWarp4::SetCacheHints(int cachehints,int frame_range)
{
  switch (cachehints)
  {
  case CACHE_GET_MTMODE :
    return MT_MULTI_INSTANCE;
  default :
    return 0;
  }
}


void aWarp4::StaticThreadpool(void *ptr)
{
	const Public_MT_Data_Thread *data=(const Public_MT_Data_Thread *)ptr;
	const aWarp4 *ptrClass=(aWarp4 *)data->pClass;

	const uint8_t thread_num=data->thread_Id;
	const MT_Data_Info_WarpSharp mt_data_inf=ptrClass->MT_Data[thread_num];
	
	switch(data->f_process)
	{
		case 1 :
			Warp2_8_MT((const unsigned char*)mt_data_inf.src_Y1,(const unsigned char*)mt_data_inf.src_Y2,
				(unsigned char*)mt_data_inf.dst_Y1,
				mt_data_inf.src_pitch_Y1,mt_data_inf.src_pitch_Y2,mt_data_inf.dst_pitch_Y1,mt_data_inf.row_size_Y1,
				mt_data_inf.dst_Y_h,ptrClass->depth,ptrClass->depthV,mt_data_inf.src_Y_h_min,mt_data_inf.src_Y_h_max);
			break;
		case 2 :
			Warp2_8_MT((const unsigned char*)mt_data_inf.src_U1,(const unsigned char*)mt_data_inf.src_U2,
				(unsigned char*)mt_data_inf.dst_U2,
				mt_data_inf.src_pitch_U1,mt_data_inf.src_pitch_U2,mt_data_inf.dst_pitch_U1,mt_data_inf.row_size_U1,
				mt_data_inf.dst_U_h,ptrClass->depthC,ptrClass->depthVC,mt_data_inf.src_UV_h_min,mt_data_inf.src_UV_h_max);
			break;
		case 3 :
			Warp2_8_MT((const unsigned char*)mt_data_inf.src_V1,(const unsigned char*)mt_data_inf.src_V2,
				(unsigned char*)mt_data_inf.dst_V1,
				mt_data_inf.src_pitch_V1,mt_data_inf.src_pitch_V2,mt_data_inf.dst_pitch_V1,mt_data_inf.row_size_V1,
				mt_data_inf.dst_U_h,ptrClass->depthC,ptrClass->depthVC,mt_data_inf.src_UV_h_min,mt_data_inf.src_UV_h_max);
			break;
		case 4 :
			GuideChroma_8_MT((const unsigned char*)mt_data_inf.src_Y2,(unsigned char*)mt_data_inf.dst_U1,
				mt_data_inf.src_pitch_Y2,mt_data_inf.src_pitch_U2,mt_data_inf.src_U_h,mt_data_inf.row_size_U2,
				mt_data_inf.SubW_U,mt_data_inf.SubH_U,ptrClass->cplace_mpeg2_flag,
				mt_data_inf.dst_UV_h_min,mt_data_inf.dst_UV_h_max);
			break;
		case 5 :
			Warp2_8_MT((const unsigned char*)mt_data_inf.src_U1,(const unsigned char*)mt_data_inf.src_U2,
				(unsigned char*)mt_data_inf.dst_U2,
				mt_data_inf.src_pitch_U1,mt_data_inf.src_pitch_U2,mt_data_inf.dst_pitch_U1,mt_data_inf.row_size_U1,
				mt_data_inf.dst_U_h,ptrClass->depthC,ptrClass->depthVC,mt_data_inf.src_UV_h_min,mt_data_inf.src_UV_h_max);
			break;
		case 6 :
			Warp2_8_MT((const unsigned char*)mt_data_inf.src_V1,(const unsigned char*)mt_data_inf.src_U2,
				(unsigned char*)mt_data_inf.dst_V1,
				mt_data_inf.src_pitch_V1,mt_data_inf.src_pitch_U2,mt_data_inf.dst_pitch_V1,mt_data_inf.row_size_V1,
				mt_data_inf.dst_V_h,ptrClass->depthC,ptrClass->depthVC,mt_data_inf.src_UV_h_min,mt_data_inf.src_UV_h_max);
			break;
		case 7 :
			Warp2_8_MT((const unsigned char*)mt_data_inf.src_U1,(const unsigned char*)mt_data_inf.src_Y2,
				(unsigned char*)mt_data_inf.dst_U2,
				mt_data_inf.src_pitch_U1,mt_data_inf.src_pitch_Y2,mt_data_inf.dst_pitch_U1,mt_data_inf.row_size_U1,
				mt_data_inf.dst_U_h,ptrClass->depthC,ptrClass->depthVC,mt_data_inf.src_UV_h_min,mt_data_inf.src_UV_h_max);
			break;
		case 8 :
			Warp2_8_MT((const unsigned char*)mt_data_inf.src_V1,(const unsigned char*)mt_data_inf.src_Y2,
				(unsigned char*)mt_data_inf.dst_V1,
				mt_data_inf.src_pitch_V1,mt_data_inf.src_pitch_Y2,mt_data_inf.dst_pitch_V1,mt_data_inf.row_size_V1,
				mt_data_inf.dst_V_h,ptrClass->depthC,ptrClass->depthVC,mt_data_inf.src_UV_h_min,mt_data_inf.src_UV_h_max);
			break;
		default : ;
	}
}


PVideoFrame __stdcall aWarp4::GetFrame(int n, IScriptEnvironment *env)
{
  PVideoFrame src = child->GetFrame(n, env);
  PVideoFrame edg = edges->GetFrame(n, env);
  PVideoFrame dst = env->NewVideoFrame(vi,64);

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

  const int SubH_Y = vi.GetPlaneHeightSubsampling(PLANAR_Y);
  const int SubH_U = vi.IsY() ? 0:vi.GetPlaneHeightSubsampling(PLANAR_U);
  const int SubH_V = vi.IsY() ? 0:vi.GetPlaneHeightSubsampling(PLANAR_V);
  const int SubW_Y = vi.GetPlaneWidthSubsampling(PLANAR_Y);
  const int SubW_U = vi.IsY() ? 0:vi.GetPlaneWidthSubsampling(PLANAR_U);
  const int SubW_V = vi.IsY() ? 0:vi.GetPlaneWidthSubsampling(PLANAR_V);

  const int32_t dst_height_Y = dst->GetHeight() >> SubH_Y;
  const int32_t dst_height_U = dst->GetHeight() >> SubH_U;
  const int32_t dst_height_V = dst->GetHeight() >> SubH_V;

  const int32_t dst_row_size_Y = dst->GetRowSize() >> SubW_Y;
  const int32_t dst_row_size_U = dst->GetRowSize() >> SubW_U;
  const int32_t dst_row_size_V = dst->GetRowSize() >> SubW_V;

  const int32_t edg_height_UV = edg->GetHeight() >> SubH_U;
  const int32_t edg_width_UV = edg->GetRowSize() >> SubW_U;

  if (threads_number>1)
  {
	  WaitForSingleObject(ghMutex,INFINITE);

	  if (!poolInterface->RequestThreadPool(UserId,threads_number,MT_Thread,-1,false))
	  {
		  ReleaseMutex(ghMutex);
		  env->ThrowError("aWarp4: Error with the TheadPool while requesting threadpool!");
	  }
  }

  if (threads_number>1)
  {

	for(uint8_t i=0; i<threads_number; i++)
	{
		MT_Data[i].src_Y1=(void *)psrc_Y;
		MT_Data[i].src_Y2=(void *)pedg_Y;
		MT_Data[i].src_pitch_Y1=src_pitch_Y;
		MT_Data[i].src_pitch_Y2=edg_pitch_Y;
		MT_Data[i].row_size_Y1=dst_row_size_Y;
		MT_Data[i].dst_Y1=pdst_Y;
		MT_Data[i].dst_pitch_Y1=dst_pitch_Y;

		MT_Data[i].src_U1=(void *)psrc_U;
		MT_Data[i].src_U2=(void *)pedg_U;
		MT_Data[i].src_pitch_U1=src_pitch_U;
		MT_Data[i].src_pitch_U2=edg_pitch_U;
		MT_Data[i].row_size_U1=dst_row_size_U;
		MT_Data[i].row_size_U2=edg_width_UV;
		MT_Data[i].dst_U1=dpedg_U;
		MT_Data[i].dst_U2=pdst_U;
		MT_Data[i].dst_pitch_U1=dst_pitch_U;

		MT_Data[i].src_V1=(void *)psrc_V;
		MT_Data[i].src_V2=(void *)pedg_V;
		MT_Data[i].src_pitch_V1=src_pitch_V;
		MT_Data[i].src_pitch_V2=edg_pitch_V;
		MT_Data[i].row_size_V1=dst_row_size_V;
		MT_Data[i].dst_V1=pdst_V;
		MT_Data[i].dst_pitch_V1=dst_pitch_V;

		MT_Data[i].src_U_h=edg_height_UV;
		MT_Data[i].dst_Y_h=dst_height_Y;
		MT_Data[i].dst_U_h=dst_height_U;
		MT_Data[i].dst_V_h=dst_height_V;

		MT_Data[i].SubW_U=SubW_U;
		MT_Data[i].SubH_U=SubH_U;
	}

	uint8_t f_proc;

  if ((chroma<5) && ((depth!=0) || (depthV!=0)))
  {
	  f_proc=1;

	  for(uint8_t i=0; i<threads_number; i++)
		  MT_Thread[i].f_process=f_proc;

	  if (poolInterface->StartThreads(UserId)) poolInterface->WaitThreadsEnd(UserId);
  }
  else
    CopyPlane(src, dst, PLANAR_Y, vi);

  switch (chroma)
  {
  case 0:
    SetPlane(dst, PLANAR_U, 0x80, vi);
    SetPlane(dst, PLANAR_V, 0x80, vi);
    break;
  case 2:
    CopyPlane(src, dst, PLANAR_U, vi);
    CopyPlane(src, dst, PLANAR_V, vi);
    break;
  case 3:
  case 5:
	  if ((depthC!=0) || (depthVC!=0))
	  {
		  f_proc=2;

		  for(uint8_t i=0; i<threads_number; i++)
			  MT_Thread[i].f_process=f_proc;

		  if (poolInterface->StartThreads(UserId)) poolInterface->WaitThreadsEnd(UserId);

		  f_proc=3;

		  for(uint8_t i=0; i<threads_number; i++)
			  MT_Thread[i].f_process=f_proc;

		  if (poolInterface->StartThreads(UserId)) poolInterface->WaitThreadsEnd(UserId);
	  }
	  else
	  {
			CopyPlane(src, dst, PLANAR_U, vi);
			CopyPlane(src, dst, PLANAR_V, vi);
	  }
    break;
  case 4:
  case 6:
	   if ((depthC!=0) || (depthVC!=0))
	   {
    if (!vi.IsYV24())
    {
	  if (!GuideChroma_8_Test(SubW_U,SubH_U))
	  {
		  ReleaseMutex(ghMutex);
		  env->ThrowError("aWarp4: Unsuported colorspace");
	  }

      if (edg->IsWritable())
	  {
		  f_proc=4;

		  for(uint8_t i=0; i<threads_number; i++)
			  MT_Thread[i].f_process=f_proc;

		  if (poolInterface->StartThreads(UserId)) poolInterface->WaitThreadsEnd(UserId);
	  }
      else
      {
        PVideoFrame tmp = env->NewVideoFrame(vi,64);

		unsigned char *dptmp_U = tmp->GetWritePtr(PLANAR_U);

		edg_pitch_U = tmp->GetPitch(PLANAR_U);
		pedg_U = tmp->GetReadPtr(PLANAR_U);
		
		f_proc=4;

		for(uint8_t i=0; i<threads_number; i++)
		{
			MT_Thread[i].f_process=f_proc;

			MT_Data[i].dst_U1=dptmp_U;
			MT_Data[i].src_U2=(void *)pedg_U;
			MT_Data[i].src_pitch_U2=edg_pitch_U;
		}

		if (poolInterface->StartThreads(UserId)) poolInterface->WaitThreadsEnd(UserId);
      }

	  f_proc=5;

	  for(uint8_t i=0; i<threads_number; i++)
		  MT_Thread[i].f_process=f_proc;

	  if (poolInterface->StartThreads(UserId)) poolInterface->WaitThreadsEnd(UserId);

	  f_proc=6;

	  for(uint8_t i=0; i<threads_number; i++)
		  MT_Thread[i].f_process=f_proc;

	  if (poolInterface->StartThreads(UserId)) poolInterface->WaitThreadsEnd(UserId);
    }
    else
    {
	  f_proc=7;

	  for(uint8_t i=0; i<threads_number; i++)
		  MT_Thread[i].f_process=f_proc;

	  if (poolInterface->StartThreads(UserId)) poolInterface->WaitThreadsEnd(UserId);

	  f_proc=8;

	  for(uint8_t i=0; i<threads_number; i++)
		  MT_Thread[i].f_process=f_proc;

	  if (poolInterface->StartThreads(UserId)) poolInterface->WaitThreadsEnd(UserId);
    }
	   }
	   else
	   {
		    CopyPlane(src,dst,PLANAR_U,vi);
			CopyPlane(src,dst,PLANAR_V,vi);
	   }
    break;
  }

	for(uint8_t i=0; i<threads_number; i++)
		MT_Thread[i].f_process=0;

    poolInterface->ReleaseThreadPool(UserId,sleep);

	ReleaseMutex(ghMutex);

  }
  else
  {

  if ((chroma<5) && ((depth!=0) || (depthV!=0)))
	Warp2_8(psrc_Y,pedg_Y,pdst_Y,src_pitch_Y,edg_pitch_Y,dst_pitch_Y,dst_row_size_Y,dst_height_Y,depth,depthV);
  else
    CopyPlane(src, dst, PLANAR_Y, vi);
  switch (chroma)
  {
  case 0:
    SetPlane(dst, PLANAR_U, 0x80, vi);
    SetPlane(dst, PLANAR_V, 0x80, vi);
    break;
  case 2:
    CopyPlane(src, dst, PLANAR_U, vi);
    CopyPlane(src, dst, PLANAR_V, vi);
    break;
  case 3:
  case 5:
	  if ((depthC!=0) || (depthVC!=0))
	  {
			Warp2_8(psrc_V,pedg_V,pdst_V,src_pitch_V,edg_pitch_V,dst_pitch_V,dst_row_size_V,dst_height_V,depthC,depthVC);
			Warp2_8(psrc_U,pedg_U,pdst_U,src_pitch_U,edg_pitch_U,dst_pitch_U,dst_row_size_U,dst_height_U,depthC,depthVC);
	  }
	  else
	  {
			CopyPlane(src, dst, PLANAR_U, vi);
			CopyPlane(src, dst, PLANAR_V, vi);
	  }
    break;
  case 4:
  case 6:
	  if ((depthC!=0) || (depthVC!=0))
	  {
    if (!vi.IsYV24())
    {
      if (edg->IsWritable())
	  {
		  if (!GuideChroma_8(pedg_Y,dpedg_U,edg_pitch_Y,edg_pitch_U,edg_height_UV,edg_width_UV,SubW_U,
			  SubH_U,cplace_mpeg2_flag)) env->ThrowError("aWarp4: Unsuported colorspace");
	  }
      else
      {
        PVideoFrame tmp = env->NewVideoFrame(vi,64);

		const int32_t tmp_pitch_U = tmp->GetPitch(PLANAR_U);
		unsigned char *dptmp_U = tmp->GetWritePtr(PLANAR_U);
		const int32_t tmp_height_UV = tmp->GetHeight() >> SubH_U;
		const int32_t tmp_width_UV = tmp->GetRowSize() >> SubW_U;

		if (!GuideChroma_8(pedg_Y,dptmp_U,edg_pitch_Y,tmp_pitch_U,tmp_height_UV,tmp_width_UV,SubW_U,
			  SubH_U,cplace_mpeg2_flag)) env->ThrowError("aWarp4: Unsuported colorspace");

		pedg_U = tmp->GetReadPtr(PLANAR_U);
		edg_pitch_U = tmp_pitch_U;
      }
	  Warp2_8(psrc_U,pedg_U,pdst_U,src_pitch_U,edg_pitch_U,dst_pitch_U,dst_row_size_U,dst_height_U,depthC,depthVC);
	  Warp2_8(psrc_V,pedg_U,pdst_V,src_pitch_V,edg_pitch_U,dst_pitch_V,dst_row_size_V,dst_height_V,depthC,depthVC);
    }
    else
    {
		Warp2_8(psrc_U,pedg_Y,pdst_U,src_pitch_U,edg_pitch_Y,dst_pitch_U,dst_row_size_U,dst_height_U,depthC,depthVC);
		Warp2_8(psrc_V,pedg_Y,pdst_V,src_pitch_V,edg_pitch_Y,dst_pitch_V,dst_row_size_V,dst_height_V,depthC,depthVC);
    }
	  }
	  else
	  {
		  CopyPlane(src, dst, PLANAR_U, vi);
		  CopyPlane(src, dst, PLANAR_V, vi);
	  }
    break;
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
	int threads,prefetch;
	bool LogicalCores,MaxPhysCores,SetAffinity,sleep;

	uint8_t threads_number=1;

	const bool avsp=env->FunctionExists("ConvertBits");


  switch ((int)(size_t)user_data)
  {
  case 0:
	  {
	  if ((aWarpSharp_g_cpuid & CPUF_SSE2)==0) env->ThrowError("aWarpSharp2: SSE2 capable CPU is required");

	  threads=args[14].AsInt(0);
	  LogicalCores=args[15].AsBool(true);
	  MaxPhysCores=args[16].AsBool(true);
	  SetAffinity=args[17].AsBool(false);
	  sleep = args[18].AsBool(false);
	  prefetch=args[19].AsInt(0);

	  if ((threads<0) || (threads>MAX_MT_THREADS))
		  env->ThrowError("aWarpSharp2: [threads] must be between 0 and %ld.",MAX_MT_THREADS);
	  if (prefetch==0) prefetch=1;
	  if ((prefetch<0) || (prefetch>MAX_THREAD_POOL))
		  env->ThrowError("aWarpSharp2: [prefetch] must be between 0 and %d.",MAX_THREAD_POOL);

	  if (threads!=1)
	  {
		  if (!poolInterface->CreatePool(prefetch)) env->ThrowError("aWarpSharp2: Unable to create ThreadPool!");

		  threads_number=poolInterface->GetThreadNumber(threads,LogicalCores);

		  if (threads_number==0) env->ThrowError("aWarpSharp2: Error with the TheadPool while getting CPU info!");

		  if (threads_number>1)
		  {
			  if (!poolInterface->AllocateThreads(threads_number,0,0,MaxPhysCores,SetAffinity,true,-1))
				  env->ThrowError("aWarpSharp2: Error with the TheadPool while allocating threadpool!");
		  }
	  }

    return new aWarpSharp(args[0].AsClip(),args[1].AsInt(0x80),args[2].AsInt(-1),args[3].AsInt(0),
		args[4].AsInt(16),args[5].AsInt(4),args[6].AsInt(128),is_cplace_mpeg2(args,7),args[8].AsInt(-1),args[9].AsInt(128),
		args[10].AsInt(128),args[11].AsInt(-1),args[12].AsInt(-1),args[13].AsInt(-1),threads,sleep,avsp,env);
	break;
	  }
  case 1:
	  {
	  if ((aWarpSharp_g_cpuid & CPUF_SSE2)==0) env->ThrowError("aWarpSharp: SSE2 capable CPU is required");

    const int type = (args[5].AsInt(2)!=2)?1:0;
    const int blurlevel = args[2].AsInt(2);
    const unsigned int cm = args[4].AsInt(1);
    static const char map[4] = {1,4,3,2};

	  threads=args[7].AsInt(0);
	  LogicalCores=args[8].AsBool(true);
	  MaxPhysCores=args[9].AsBool(true);
	  SetAffinity=args[10].AsBool(false);
	  sleep = args[11].AsBool(false);
	  prefetch=args[12].AsInt(0);

	  if ((threads<0) || (threads>MAX_MT_THREADS))
		  env->ThrowError("aWarpSharp: [threads] must be between 0 and %ld.",MAX_MT_THREADS);
	  if (prefetch==0) prefetch=1;
	  if ((prefetch<0) || (prefetch>MAX_THREAD_POOL))
		  env->ThrowError("aWarpSharp: [prefetch] must be between 0 and %d.",MAX_THREAD_POOL);

	  if (threads!=1)
	  {
		  if (!poolInterface->CreatePool(prefetch)) env->ThrowError("aWarpSharp: Unable to create ThreadPool!");

		  threads_number=poolInterface->GetThreadNumber(threads,LogicalCores);

		  if (threads_number==0) env->ThrowError("aWarpSharp: Error with the TheadPool while getting CPU info!");

		  if (threads_number>1)
		  {
			  if (!poolInterface->AllocateThreads(threads_number,0,0,MaxPhysCores,SetAffinity,true,-1))
				  env->ThrowError("aWarpSharp: Error with the TheadPool while allocating threadpool!");
		  }
	  }

    return new aWarpSharp(args[0].AsClip(),int(args[3].AsFloat(0.5)*256.0),(type==1)?(blurlevel*3):blurlevel,type,
		int(args[1].AsFloat(16.0)*blurlevel*0.5),(cm<4)?map[cm]:-1,args[6].AsInt(128),false,-1,128,128,-1,-1,-1,threads,sleep,avsp,env);
	break;
	  }
  case 2:
	  {

	  threads=args[4].AsInt(0);
	  LogicalCores=args[5].AsBool(true);
	  MaxPhysCores=args[6].AsBool(true);
	  SetAffinity=args[7].AsBool(false);
	  sleep = args[8].AsBool(false);
	  prefetch=args[9].AsInt(0);

	  if ((aWarpSharp_g_cpuid & CPUF_SSE2)==0) env->ThrowError("aSobel: SSE2 capable CPU is required");

	  if ((threads<0) || (threads>MAX_MT_THREADS))
		  env->ThrowError("aSobel: [threads] must be between 0 and %ld.",MAX_MT_THREADS);
	  if (prefetch==0) prefetch=1;
	  if ((prefetch<0) || (prefetch>MAX_THREAD_POOL))
		  env->ThrowError("aSobel: [prefetch] must be between 0 and %d.",MAX_THREAD_POOL);

	  if (threads!=1)
	  {
		  if (!poolInterface->CreatePool(prefetch)) env->ThrowError("aSobel: Unable to create ThreadPool!");

		  threads_number=poolInterface->GetThreadNumber(threads,LogicalCores);

		  if (threads_number==0) env->ThrowError("aSobel: Error with the TheadPool while getting CPU info!");

		  if (threads_number>1)
		  {
			  if (!poolInterface->AllocateThreads(threads_number,0,0,MaxPhysCores,SetAffinity,true,-1))
				  env->ThrowError("aSobel: Error with the TheadPool while allocating threadpool!");
		  }
	  }

    return new aSobel(args[0].AsClip(),args[1].AsInt(0x80),args[2].AsInt(1),args[3].AsInt(-1),threads,sleep,avsp,env);
	break;
	  }
  case 3:
	  {

	  threads=args[7].AsInt(0);
	  LogicalCores=args[8].AsBool(true);
	  MaxPhysCores=args[9].AsBool(true);
	  SetAffinity=args[10].AsBool(false);
	  sleep = args[11].AsBool(false);
	  prefetch=args[12].AsInt(0);

	  if ((aWarpSharp_g_cpuid & CPUF_SSE2)==0) env->ThrowError("aBlur: SSE2 capable CPU is required");

	  if ((threads<0) || (threads>MAX_MT_THREADS))
		  env->ThrowError("aBlur: [threads] must be between 0 and %ld.",MAX_MT_THREADS);
	  if (prefetch==0) prefetch=1;
	  if ((prefetch<0) || (prefetch>MAX_THREAD_POOL))
		  env->ThrowError("aBlur: [prefetch] must be between 0 and %d.",MAX_THREAD_POOL);

	  if (threads!=1)
	  {
		  if (!poolInterface->CreatePool(prefetch)) env->ThrowError("aBlur: Unable to create ThreadPool!");

		  threads_number=poolInterface->GetThreadNumber(threads,LogicalCores);

		  if (threads_number==0) env->ThrowError("aBlur: Error with the TheadPool while getting CPU info!");

		  if (threads_number>1)
		  {
			  if (!poolInterface->AllocateThreads(threads_number,0,0,MaxPhysCores,SetAffinity,true,-1))
				  env->ThrowError("aBlur: Error with the TheadPool while allocating threadpool!");
		  }
	  }

	  if ((aWarpSharp_g_cpuid & CPUF_SSE2)==0) env->ThrowError("aBlur: SSE2 capable CPU is required");

    return new aBlur(args[0].AsClip(),args[1].AsInt(-1),args[2].AsInt(1),args[3].AsInt(1),args[4].AsInt(-1),
		args[5].AsInt(-1),args[6].AsInt(-1),threads,sleep,avsp,env);
	break;
	  }
  case 4:
	  {

	  threads=args[8].AsInt(0);
	  LogicalCores=args[9].AsBool(true);
	  MaxPhysCores=args[10].AsBool(true);
	  SetAffinity=args[11].AsBool(false);
	  sleep = args[12].AsBool(false);
	  prefetch=args[13].AsInt(0);

	  if ((aWarpSharp_g_cpuid & CPUF_SSE2)==0) env->ThrowError("aWarp: SSE2 capable CPU is required");

	  if ((threads<0) || (threads>MAX_MT_THREADS))
		  env->ThrowError("aWarp: [threads] must be between 0 and %ld.",MAX_MT_THREADS);
	  if (prefetch==0) prefetch=1;
	  if ((prefetch<0) || (prefetch>MAX_THREAD_POOL))
		  env->ThrowError("aWarp: [prefetch] must be between 0 and %d.",MAX_THREAD_POOL);

	  if (threads!=1)
	  {
		  if (!poolInterface->CreatePool(prefetch)) env->ThrowError("aWarp: Unable to create ThreadPool!");

		  threads_number=poolInterface->GetThreadNumber(threads,LogicalCores);

		  if (threads_number==0) env->ThrowError("aWarp: Error with the TheadPool while getting CPU info!");

		  if (threads_number>1)
		  {
			  if (!poolInterface->AllocateThreads(threads_number,0,0,MaxPhysCores,SetAffinity,true,-1))
				  env->ThrowError("aWarp: Error with the TheadPool while allocating threadpool!");
		  }
	  }

	  if ((aWarpSharp_g_cpuid & CPUF_SSE2)==0) env->ThrowError("aWarp: SSE2 capable CPU is required");

    return new aWarp(args[0].AsClip(),args[1].AsClip(),args[2].AsInt(3),args[3].AsInt(4),args[4].AsInt(-1),is_cplace_mpeg2(args,5),
		args[6].AsInt(128),args[7].AsInt(128),threads,sleep,avsp,env);
	break;
	  }
  case 5:
	  {
	  threads=args[8].AsInt(0);
	  LogicalCores=args[9].AsBool(true);
	  MaxPhysCores=args[10].AsBool(true);
	  SetAffinity=args[11].AsBool(false);
	  sleep = args[12].AsBool(false);
	  prefetch=args[13].AsInt(0);

	  if ((aWarpSharp_g_cpuid & CPUF_SSE2)==0) env->ThrowError("aWarp4: SSE2 capable CPU is required");

	  if ((threads<0) || (threads>MAX_MT_THREADS))
		  env->ThrowError("aWarp4: [threads] must be between 0 and %ld.",MAX_MT_THREADS);
	  if (prefetch==0) prefetch=1;
	  if ((prefetch<0) || (prefetch>MAX_THREAD_POOL))
		  env->ThrowError("aWarp4: [prefetch] must be between 0 and %d.",MAX_THREAD_POOL);

	  if (threads!=1)
	  {
		  if (!poolInterface->CreatePool(prefetch)) env->ThrowError("aWarp4: Unable to create ThreadPool!");

		  threads_number=poolInterface->GetThreadNumber(threads,LogicalCores);

		  if (threads_number==0) env->ThrowError("aWarp4: Error with the TheadPool while getting CPU info!");

		  if (threads_number>1)
		  {
			  if (!poolInterface->AllocateThreads(threads_number,0,0,MaxPhysCores,SetAffinity,true,-1))
				  env->ThrowError("aWarp4: Error with the TheadPool while allocating threadpool!");
		  }
	  }

    return new aWarp4(args[0].AsClip(),args[1].AsClip(),args[2].AsInt(3),args[3].AsInt(4),args[4].AsInt(-1),is_cplace_mpeg2(args,5),
		args[6].AsInt(128),args[7].AsInt(128),threads,sleep,avsp,env);
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

  aWarpSharp_g_cpuid = env->GetCPUFlags(); // PF

  env->AddFunction("aWarpSharp2", "c[thresh]i[blur]i[type]i[depth]i[chroma]i[depthC]i[cplace]s[blurV]i[depthV]i[depthVC]i" \
	  "[blurC]i[blurVC]i[threshC]i[threads]i[logicalCores]b[MaxPhysCore]b[SetAffinity]b[sleep]b[prefetch]i", Create_aWarpSharp, (void*)0);
  env->AddFunction("aWarpSharp", "c[depth]f[blurlevel]i[thresh]f[cm]i[bm]i[show]b" \
	  "[threads]i[logicalCores]b[MaxPhysCore]b[SetAffinity]b[sleep]b[prefetch]i", Create_aWarpSharp, (void*)1);
  env->AddFunction("aSobel", "c[thresh]i[chroma]i[threshC]i" \
	  "[threads]i[logicalCores]b[MaxPhysCore]b[SetAffinity]b[sleep]b[prefetch]i", Create_aWarpSharp, (void*)2);
  env->AddFunction("aBlur", "c[blur]i[type]i[chroma]i[blurV]i[blurC]i[blurVC]i" \
	  "[threads]i[logicalCores]b[MaxPhysCore]b[SetAffinity]b[sleep]b[prefetch]i", Create_aWarpSharp, (void*)3);
  env->AddFunction("aWarp", "cc[depth]i[chroma]i[depthC]i[cplace]s[depthV]i[depthVC]i" \
	  "[threads]i[logicalCores]b[MaxPhysCore]b[SetAffinity]b[sleep]b[prefetch]i", Create_aWarpSharp, (void*)4);
  env->AddFunction("aWarp4", "cc[depth]i[chroma]i[depthC]i[cplace]s[depthV]i[depthVC]i" \
	  "[threads]i[logicalCores]b[MaxPhysCore]b[SetAffinity]b[sleep]b[prefetch]i", Create_aWarpSharp, (void*)5);

  return AWARPSHARP_VERSION;
}
