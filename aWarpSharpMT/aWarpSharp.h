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

#include "./avisynth.h"
#include "./ThreadPoolInterface.h"

#define AWARPSHARP_VERSION "aWarpSharpMT 2.1.10 JPSDR"

#define myfree(ptr) if (ptr!=NULL) { free(ptr); ptr=NULL;}
#define myAlignedFree(ptr) if (ptr!=NULL) { _aligned_free(ptr); ptr=NULL;}

typedef struct _MT_Data_Info_WarpSharp
{
	void *src_Y1,*src_Y2,*src_Y3;
	void *dst_Y1,*dst_Y2,*dst_Y3;
	int32_t src_pitch_Y1,src_pitch_Y2,src_pitch_Y3;
	int32_t dst_pitch_Y1,dst_pitch_Y2,dst_pitch_Y3;
	void *src_U1,*src_U2,*src_U3;
	void *dst_U1,*dst_U2,*dst_U3;
	int32_t src_pitch_U1,src_pitch_U2,src_pitch_u3;
	int32_t dst_pitch_U1,dst_pitch_U2,dst_pitch_U3;
	void *src_V1,*src_V2,*src_V3;
	void *dst_V1,*dst_V2,*dst_V3;
	int32_t src_pitch_V1,src_pitch_V2,src_pitch_V3;
	int32_t dst_pitch_V1,dst_pitch_V2,dst_pitch_V3;

	int32_t row_size_Y1,row_size_Y2,row_size_Y3;
	int32_t row_size_U1,row_size_U2,row_size_U3;
	int32_t row_size_V1,row_size_V2,row_size_V3;

	int32_t src_Y_h,src_U_h,src_V_h,dst_Y_h,dst_U_h,dst_V_h;

	int32_t src_Y_h_min,src_Y_h_max,src_Y_w;
	int32_t src_UV_h_min,src_UV_h_max,src_UV_w;
	int32_t dst_Y_h_min,dst_Y_h_max,dst_Y_w;
	int32_t dst_UV_h_min,dst_UV_h_max,dst_UV_w;
	bool top,bottom;

	bool processH,processV,cprocessH,cprocessV;
	int SubW_U,SubH_U;

} MT_Data_Info_WarpSharp;


class aWarpSharp : public GenericVideoFilter
{
public:
  aWarpSharp(PClip _child, int _thresh, int _blur_level, int _blur_type, int _depth, int _chroma, int _depthC, 
	  bool _cplace_mpeg2_flag, int _blur_levelV, int _depthV, int _depthVC, int _blur_levelC, int _blur_levelVC,
	  int _threshC,uint8_t _threads,bool _sleep, bool negativePrefetch, bool _avsp, IScriptEnvironment *env);
  virtual ~aWarpSharp();

  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment *env);
  int __stdcall SetCacheHints(int cachehints, int frame_range);

private:
  int thresh,threshC;
  int blur_level,blur_levelV;
  int blur_levelC,blur_levelVC;
  int depth,depthV;
  int depthC,depthVC;
  int chroma;
  int blur_type;
  bool cplace_mpeg2_flag;

  bool grey,avsp,isRGBPfamily,isAlphaChannel,has_at_least_v8;
  uint8_t pixelsize; // AVS16
  uint8_t bits_per_pixel;

	Public_MT_Data_Thread MT_Thread[MAX_MT_THREADS];
	MT_Data_Info_WarpSharp MT_Data[MAX_MT_THREADS];
	uint8_t threads,threads_number;
	bool sleep;
	uint32_t UserId;
	
	ThreadPoolFunction StaticThreadpoolF;

	static void StaticThreadpool(void *ptr);
};


class aSobel : public GenericVideoFilter
{
public:
  aSobel(PClip _child, int _thresh, int _chroma, int _threshC,uint8_t _threads,bool _sleep, bool negativePrefetch,
	  bool _avsp, IScriptEnvironment *env);
  virtual ~aSobel();

  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment *env);
  int __stdcall SetCacheHints(int cachehints, int frame_range);

private:
  int thresh,threshC;
  int chroma;

  bool grey,avsp,isRGBPfamily,isAlphaChannel,has_at_least_v8;
  uint8_t pixelsize; // AVS16
  uint8_t bits_per_pixel;

	Public_MT_Data_Thread MT_Thread[MAX_MT_THREADS];
	MT_Data_Info_WarpSharp MT_Data[MAX_MT_THREADS];
	uint8_t threads,threads_number;
	bool sleep;
	uint32_t UserId;
	
	ThreadPoolFunction StaticThreadpoolF;

	static void StaticThreadpool(void *ptr);
};


class aBlur : public GenericVideoFilter
{
public:
  aBlur(PClip _child, int _blur_level, int _blur_type, int _chroma, int _blur_levelV,
	   int _blur_levelC, int _blur_levelVC,uint8_t _threads,bool _sleep, bool negativePrefetch,
	   bool _avsp,IScriptEnvironment *env);
  virtual ~aBlur();

  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment *env);
  int __stdcall SetCacheHints(int cachehints, int frame_range);

private:
  int blur_level,blur_levelV;
   int blur_levelC,blur_levelVC;
  int blur_type;
  int chroma;

  bool grey,avsp,isRGBPfamily,isAlphaChannel,has_at_least_v8;
  uint8_t pixelsize; // AVS16
  uint8_t bits_per_pixel;

	Public_MT_Data_Thread MT_Thread[MAX_MT_THREADS];
	MT_Data_Info_WarpSharp MT_Data[MAX_MT_THREADS];
	uint8_t threads,threads_number;
	bool sleep;
	uint32_t UserId;
	
	ThreadPoolFunction StaticThreadpoolF;

	static void StaticThreadpool(void *ptr);
};


class aWarp : public GenericVideoFilter
{
public:
  aWarp(PClip _child, PClip _edges, int _depth, int _chroma, int _depthC, bool _cplace_mpeg2_flag,
	  int _depthV, int _depthVC,uint8_t _threads,bool _sleep, bool negativePrefetch,
	  bool _avsp, IScriptEnvironment *env);
  virtual ~aWarp();

  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment *env);
  int __stdcall SetCacheHints(int cachehints, int frame_range);

private:
  PClip edges;
  int depth,depthV;
  int depthC,depthVC;
  int chroma;
  bool cplace_mpeg2_flag;

  bool grey,avsp,isRGBPfamily,isAlphaChannel,has_at_least_v8;
  uint8_t pixelsize; // AVS16
  uint8_t bits_per_pixel;

	Public_MT_Data_Thread MT_Thread[MAX_MT_THREADS];
	MT_Data_Info_WarpSharp MT_Data[MAX_MT_THREADS];
	uint8_t threads,threads_number;
	bool sleep;
	uint32_t UserId;
	
	ThreadPoolFunction StaticThreadpoolF;

	static void StaticThreadpool(void *ptr);
};


class aWarp4 : public GenericVideoFilter
{
public:
  aWarp4(PClip _child, PClip _edges, int _depth, int _chroma, int _depthC, bool _cplace_mpeg2_flag,
	  int _depthV, int _depthVC,uint8_t _threads,bool _sleep, bool negativePrefetch,
	  bool _avsp, IScriptEnvironment *env);
  virtual ~aWarp4();

  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment *env);
  int __stdcall SetCacheHints(int cachehints, int frame_range);

private:
  PClip edges;
  int depth,depthV;
  int depthC,depthVC;
  int chroma;
  bool cplace_mpeg2_flag;

  bool grey,avsp,isRGBPfamily,isAlphaChannel,has_at_least_v8;
  uint8_t pixelsize; // AVS16
  uint8_t bits_per_pixel;

	Public_MT_Data_Thread MT_Thread[MAX_MT_THREADS];
	MT_Data_Info_WarpSharp MT_Data[MAX_MT_THREADS];
	uint8_t threads,threads_number;
	bool sleep;
	uint32_t UserId;
	
	ThreadPoolFunction StaticThreadpoolF;

	static void StaticThreadpool(void *ptr);
};