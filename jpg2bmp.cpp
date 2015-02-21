//-------------------------------------------------
// jpg2bmp -- a tool to convert jpg to bmp24
// The tool code is based on http://blog.csdn.net/kangear/article/details/8576886
// fixed issues:
// * 64bit linux issues
// * unaligned width issue
//
// Author: Aixi Wang <aixi.wang@hotmail.com>
//-------------------------------------------------
#include <iostream>
#include <cstring>
using namespace std;
extern "C"{
#include "jpeglib.h"
#include <string.h>
#include <stdio.h>
};

#pragma pack(2) 
struct sBFH
{
    unsigned short  bfType; 
    unsigned int    bfSize;
    unsigned short  bfReverved1;
    unsigned short  bfReverved2;
    unsigned int    bfOffBits;
};

#pragma pack(2) 
struct sBIH
{
    unsigned int    biSize;
    unsigned int    biWidth;
    unsigned int    biHeight;
    unsigned short  biPlanes;
    unsigned short  biBitCount;
    unsigned int    biCompression;
    unsigned int    biSizeImage;
    unsigned int    biXPelsPerMeter;
    unsigned int    biYpelsPerMeter;
    unsigned int    biClrUsed;
    unsigned int    biClrImportant;
};

FILE *jpg_file;
FILE *bmp_file;
unsigned int everyline_size;

void write_bmp_header(j_decompress_ptr cinfo)
{
    struct sBFH bfh;
    struct sBIH bih;


    unsigned int width;
    unsigned int height;
    unsigned short depth;
    unsigned int headersize;
    unsigned int filesize;
       
    int i;

    width=cinfo->output_width;
    height=cinfo->output_height;
    depth=cinfo->output_components;

    if (depth==1)
    {
        headersize=14+40+256*4;
        filesize=headersize+width*height;
    }

    if (depth==3)
    {
        headersize=14+40;
        if (width*depth%4 == 0)
        {
            everyline_size = width*depth;
            filesize=headersize+everyline_size*height;
        }
        else
        {
            everyline_size = width*depth + (4-width*depth%4);
            filesize=headersize+everyline_size*height;
        }
    }

    memset(&bfh,0,sizeof(struct sBFH));
    memset(&bih,0,sizeof(struct sBIH));
    
    bfh.bfType=0x4D42;
    bfh.bfSize=filesize;
    bfh.bfOffBits=headersize;

    bih.biSize=40;
    bih.biWidth=width;
    bih.biHeight=height;
    bih.biPlanes=1;
    bih.biBitCount=(unsigned short)depth*8;
    bih.biSizeImage=everyline_size*height;
  
    fwrite(&bfh,sizeof(struct sBFH),1,bmp_file);
    fwrite(&bih,sizeof(struct sBIH),1,bmp_file);


    if (depth==1) 
    {
        unsigned char *platte;
        platte=new unsigned char[256*4];
        unsigned char j=0;
        for (int i=0;i<1024;i+=4)
        {
            platte[i]=j;
            platte[i+1]=j;
            platte[i+2]=j;
            platte[i+3]=0;
            j++;
        }
        fwrite(platte,sizeof(unsigned char)*1024,1,bmp_file);
        delete[] platte;
    }
}

void write_bmp_data(j_decompress_ptr cinfo,unsigned char *src_buff)
{
    unsigned char *dst_width_buff;
    unsigned char *point;

    unsigned int width;
    unsigned int height;
    unsigned short depth;


    width=cinfo->output_width;
    height=cinfo->output_height;
    depth=cinfo->output_components;


    dst_width_buff=new unsigned char[everyline_size];
    memset(dst_width_buff,0,everyline_size);

    point=src_buff+width*depth*(height-1);
    for (unsigned int i=0;i<height;i++)
    {
        memset(dst_width_buff,0,everyline_size);    
        for (unsigned int j=0;j<width*depth;j+=depth)
        {
            if (depth==1)
            {
                dst_width_buff[j]=point[j];
            }
            if (depth==3)
            {
                dst_width_buff[j+2]=point[j+0];
                dst_width_buff[j+1]=point[j+1];
                dst_width_buff[j+0]=point[j+2];
            }
        }
        point-=width*depth;
        fwrite(dst_width_buff,sizeof(unsigned char)*everyline_size,1,bmp_file); 
    }

    delete[]dst_width_buff;
}


void do_jpg2bmp_convert()
{
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    JSAMPARRAY buffer;
    unsigned char *src_buff;
    unsigned char *point;


    cinfo.err=jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo,jpg_file);
    jpeg_read_header(&cinfo,TRUE);
    jpeg_start_decompress(&cinfo);


    unsigned int width=cinfo.output_width;
    unsigned int height=cinfo.output_height;
    unsigned short depth=cinfo.output_components;


    src_buff=new unsigned char[width*height*depth];
    memset(src_buff,0,sizeof(unsigned char)*width*height*depth);


    buffer=(*cinfo.mem->alloc_sarray)
        ((j_common_ptr)&cinfo,JPOOL_IMAGE,width*depth,1);


    point=src_buff;
    while (cinfo.output_scanline<height)
    {
        jpeg_read_scanlines(&cinfo,buffer,1);
        memcpy(point,*buffer,width*depth);
        point+=width*depth; 
    }

    write_bmp_header(&cinfo);
    write_bmp_data(&cinfo,src_buff);

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    delete[] src_buff;
}

int main()
{
    jpg_file=fopen("now.jpg","rb");
    bmp_file=fopen("now.bmp","wb");

    do_jpg2bmp_convert();

    fclose(jpg_file);
    fclose(bmp_file);
    printf("ok\r\n");
    return 0;
}
