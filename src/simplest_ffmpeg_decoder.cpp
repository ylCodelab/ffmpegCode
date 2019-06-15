/**
 * ��򵥵Ļ���FFmpeg�Ľ�����
 * Simplest FFmpeg Decoder
 *
 * ������ Lei Xiaohua
 * leixiaohua1020@126.com
 * �й���ý��ѧ/���ֵ��Ӽ���
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 * ������ʵ������Ƶ�ļ��Ľ���(֧��HEVC��H.264��MPEG2��)��
 * ����򵥵�FFmpeg��Ƶ���뷽��Ľ̡̳�
 * ͨ��ѧϰ�����ӿ����˽�FFmpeg�Ľ������̡�
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
	//��װ��ʽ�����Ľṹ�壬Ҳ��ͳ��ȫ�ֵĽṹ�壬��������Ƶ�ļ���װ ��ʽ�����Ϣ
	AVFormatContext	*pFormatCtx;
	int				i, videoindex;
	//�����������Ľṹ�壬��������Ƶ(��Ƶ)����������Ϣ
	AVCodecContext	*pCodecCtx;
	//ÿ����Ƶ(��Ƶ)�������(����H.264������)��Ӧһ���ýṹ��
	AVCodec			*pCodec;
	//�洢һ֡���������(����)����
	AVFrame	*pFrame,*pFrameYUV;
	uint8_t *out_buffer;
	//�洢һ֡ѹ����������
	AVPacket *packet;
	int y_size;
	int ret, got_picture;
	struct SwsContext *img_convert_ctx;
	//�����ļ�·��
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
		//��Ƶ�������ĸ�����
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
	 * �ڴ˴���������Ƶ��Ϣ�Ĵ���
	 * ȡ����pFormatCtx��ʹ��fprintf()
	 */
	//Allocate an AVFrame and set its fields to default values.
	pFrame=av_frame_alloc();
	pFrameYUV=av_frame_alloc();
	//�����ڴ�
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

	//�������ļ���ȡһ֡ѹ������
	while(av_read_frame(pFormatCtx, packet)>=0){
		if(packet->stream_index==videoindex){
		 
		   /*
			* �ڴ˴�������H264�����Ĵ���
			* ȡ����packet��ʹ��fwrite()
			*/
			fwrite(packet->data, 1, packet->size, fp_264);
			//����һ֡ѹ������
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
				//дyuv����
				fwrite(pFrameYUV->data[0], 1, (pCodecCtx->width * pCodecCtx->height), fp_yuv);
				fwrite(pFrameYUV->data[1], 1, (pCodecCtx->width * pCodecCtx->height) / 4, fp_yuv);
				fwrite(pFrameYUV->data[2], 1, (pCodecCtx->width * pCodecCtx->height) / 4, fp_yuv);
				/*
				 * �ڴ˴�������YUV�Ĵ���
				 * ȡ����pFrameYUV��ʹ��fwrite()
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

