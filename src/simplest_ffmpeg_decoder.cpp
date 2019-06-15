/**
 * 最简单的基于FFmpeg的解码器
 * Simplest FFmpeg Decoder
 *
 * 雷霄骅 Lei Xiaohua
 * leixiaohua1020@126.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 * 本程序实现了视频文件的解码(支持HEVC，H.264，MPEG2等)。
 * 是最简单的FFmpeg视频解码方面的教程。
 * 通过学习本例子可以了解FFmpeg的解码流程。
 * This software is a simplest video decoder based on FFmpeg.
 * Suitable for beginner of FFmpeg.
 *
 */


#include <math.h>
#include <stdio.h>
#include <iostream>

#define __STDC_CONSTANT_MACROS

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
};


int simple_ffmpeg_decoder()
{
	//封装格式上下文结构体，也是统领全局的结构体，保存了视频文件封装 格式相关信息
	AVFormatContext	*pFormatCtx;
	int				i, videoindex;
	//编码器上下文结构体，保存了视频(音频)编解码相关信息
	AVCodecContext	*pCodecCtx;
	//每种视频(音频)编解码器(例如H.264解码器)对应一个该结构体
	AVCodec			*pCodec;
	//存储一帧解码后像素(采样)数据
	AVFrame	*pFrame,*pFrameYUV;
	uint8_t *out_buffer;
	//存储一帧压缩编码数据
	AVPacket *packet;
	int y_size;
	int ret, got_picture;
	struct SwsContext *img_convert_ctx;
	//输入文件路径
	char filepath[]="Titanic.ts";

	int frame_cnt;
	//Initialize libavformat and register all the muxers, demuxers and protocols
	av_register_all();
	//Do global initialization of network libraries
	avformat_network_init();
	pFormatCtx = avformat_alloc_context();

	//Open an input stream and read the header
	if(avformat_open_input(&pFormatCtx,filepath,NULL,NULL)!=0){
		printf("Couldn't open input stream.\n");
		return -1;
	}
	//Read packets of a media file to get stream information
	if(avformat_find_stream_info(pFormatCtx,NULL)<0){
		printf("Couldn't find stream information.\n");
		return -1;
	}
	std::cout << "Video Format Duration " << pFormatCtx->duration / pow(10, 6) << std::endl;

	videoindex=-1;
	std::cout << "Video Streams " << pFormatCtx->nb_streams << std::endl;
	for(i=0; i<pFormatCtx->nb_streams; i++) 
		//视频数据在哪个流里
		if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO){
			videoindex=i;
			break;
		}
	if(videoindex==-1){
		printf("Didn't find a video stream.\n");
		return -1;
	}

	pCodecCtx=pFormatCtx->streams[videoindex]->codec;
	//Find a registered decoder with a matching codec ID.
	pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
	if(pCodec==NULL){
		printf("Codec not found.\n");
		return -1;
	}
	//Initialize the AVCodecContext to use the given AVCodec
	if(avcodec_open2(pCodecCtx, pCodec,NULL)<0){
		printf("Could not open codec.\n");
		return -1;
	}

	FILE *fp = fopen("info.txt","wb+");
	fprintf(fp, "During: %d\n", pFormatCtx->duration);
	fprintf(fp, "Format: %s\n", pFormatCtx->iformat->long_name);
	fprintf(fp, "Width: %d  Height %d\n", pFormatCtx->streams[videoindex]->codec->width, pFormatCtx->streams[videoindex]->codec->height);
	fclose(fp);
	/*
	 * 在此处添加输出视频信息的代码
	 * 取自于pFormatCtx，使用fprintf()
	 */
	//Allocate an AVFrame and set its fields to default values.
	pFrame=av_frame_alloc();
	pFrameYUV=av_frame_alloc();
	//分配内存
	out_buffer=(uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));
	avpicture_fill((AVPicture *)pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);
	packet=(AVPacket *)av_malloc(sizeof(AVPacket));
	//Output Info-----------------------------
	printf("--------------- File Information ----------------\n");
	av_dump_format(pFormatCtx,0,filepath,0);
	printf("-------------------------------------------------\n");
	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, 
		pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL); 

	frame_cnt=0;

	FILE *fp_264 = fopen("test.h264", "wb+");
	FILE *fp_yuv = fopen("test.yuv", "wb+");

	//从输入文件读取一帧压缩数据
	while(av_read_frame(pFormatCtx, packet)>=0){
		if(packet->stream_index==videoindex){
		 
		   /*
			* 在此处添加输出H264码流的代码
			* 取自于packet，使用fwrite()
			*/
			fwrite(packet->data, 1, packet->size, fp_264);
			//解码一帧压缩数据
			ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
			if(ret < 0){
				printf("Decode Error.\n");
				return -1;
			}
			if(got_picture){
				//scale frame data, used to remove stride of image
				sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, 
					pFrameYUV->data, pFrameYUV->linesize);
				printf("Decoded frame index: %d\n",frame_cnt);
				//写yuv数据
				fwrite(pFrameYUV->data[0], 1, (pCodecCtx->width * pCodecCtx->height), fp_yuv);
				fwrite(pFrameYUV->data[1], 1, (pCodecCtx->width * pCodecCtx->height) / 4, fp_yuv);
				fwrite(pFrameYUV->data[2], 1, (pCodecCtx->width * pCodecCtx->height) / 4, fp_yuv);
				/*
				 * 在此处添加输出YUV的代码
				 * 取自于pFrameYUV，使用fwrite()
				 */

				frame_cnt++;

			}
		}
		av_free_packet(packet);
	}
	fclose(fp_264);
	fclose(fp_yuv);
	sws_freeContext(img_convert_ctx);

	av_frame_free(&pFrameYUV);
	av_frame_free(&pFrame);
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);

	return 0;
}

