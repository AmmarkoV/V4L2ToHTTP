/*
AmmarServer , main executable

URLs: http://ammar.gr
Written by Ammar Qammaz a.k.a. AmmarkoV 2012

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "../AmmarServer/src/AmmServerlib/AmmServerlib.h"

#define MAX_BINDING_PORT 65534
#define DEFAULT_BINDING_PORT 8081
#define MAX_INPUT_IP 256
#define MAX_FILE_PATH 512


#include "../VideoInput/VideoInput.h"
#include <linux/videodev2.h>

#define SIMPLE_INDEX 0

char webcam[MAX_FILE_PATH]="/dev/video0";
char webserver_root[MAX_FILE_PATH]="public_html/";
char templates_root[MAX_FILE_PATH]="public_html/templates/";

char * index_page_mem=0;
unsigned long index_page_size=0;


unsigned int jpg_width=320,jpg_height=240;
char * jpg_snap_mem=0; //Memory Should be allocated fitting size of picture..
unsigned long jpg_snap_size=0;
unsigned long jpg_snap_full_size=0;


void create_index_page()
{
  index_page_mem  = (char * ) malloc(sizeof(char) * 4096);

  if (SIMPLE_INDEX)
  {
    strcpy(index_page_mem,"<html><head><meta http-equiv=\"refresh\" content=\"1\"><title>V4L2ToHTTP</title></head><body><br><br><center><img src=\"cam.jpg\"><br><h3><a href=\"index.html\">-- Manually Reload Page --</a></h3></center></body></html>");
  } else
  {

    strcpy(index_page_mem,"<html>\n");
    strcat(index_page_mem,"    <head>\n");
    strcat(index_page_mem,"         <title>V4L2ToHTTP</title>\n");
    strcat(index_page_mem,"         <script language=\"JavaScript\"><!--\n");
    strcat(index_page_mem,"                var newImage = new Image();\n");
    strcat(index_page_mem,"                var number = 0;\n");
    strcat(index_page_mem,"                newImage.src = \"cam.jpg\";\n");
    strcat(index_page_mem,"                 function updateImage()\n");
    strcat(index_page_mem,"                    {\n");
    strcat(index_page_mem,"                           if(newImage.complete)\n");
    strcat(index_page_mem,"                           {\n");
    strcat(index_page_mem,"                                document.getElementById(\"LiveImage\").src = newImage.src;\n");
    strcat(index_page_mem,"                                if(newImage.complete)\n");
    strcat(index_page_mem,"                                 {\n");
    strcat(index_page_mem,"                                      document.getElementById(\"LiveImage\").src = newImage.src;\n");
    strcat(index_page_mem,"                                      newImage = new Image();\n");
    strcat(index_page_mem,"                                      number++;\n");
    strcat(index_page_mem,"                                     newImage.src = \"cam.jpg\"\n"); //Later on make it cam.jpg?inc=\" + number;
    strcat(index_page_mem,"                                   }\n");
    strcat(index_page_mem,"                        }\n                     setTimeout('updateImage()',550);\n");
    strcat(index_page_mem,"                   }\n");
    strcat(index_page_mem,"               //--></script>\n");
    strcat(index_page_mem,"</head>\n");
    strcat(index_page_mem,"<body  onLoad=\"setTimeout('updateImage()',550)\">\n");
    strcat(index_page_mem,"<br><br><center><img src=\"cam.jpg\" id=\"LiveImage\"><br><a href=\"index.html\">-- Manually Reload Page --</a></h3></center></body></html>\n");
  }

  index_page_size=strlen((char *)index_page_mem);
}

void * prepare_index_page_callback()
{
  //Nothing to do :P
  return 0;
}

int open_camera(unsigned int width,unsigned int height,unsigned int framerate)
{
   if ( !InitVideoInputs(1) ) { fprintf(stderr,"Could not open Video Input\n"); return 0; }

    char BITRATE=32;
    struct VideoFeedSettings feedsettings={0};
    //videosettings.PixelFormat=V4L2_PIX_FMT_YUYV; BITRATE=16; // <- Common compressed setting for UVC webcams
    feedsettings.PixelFormat=V4L2_PIX_FMT_RGB24; BITRATE=24;   //   <- Common raw setting for UVC webcams ( Run Compat )
    jpg_width=width;
    jpg_height=height;

     if (! InitVideoFeed(0,webcam,jpg_width,jpg_height,BITRATE,framerate,1,feedsettings) ) { fprintf(stderr,"Could not set Video feed settings consider running with v4l2convert.so preloaded\n"); return 0; }

     jpg_snap_size=jpg_width * jpg_height * 3;
     jpg_snap_full_size=jpg_snap_size;
     jpg_snap_mem = (char *) malloc(sizeof(char) * jpg_snap_size ); //Jpeg should be smaller than uncompressed RGB :P
     if (jpg_snap_mem==0) { fprintf(stderr,"Could not allocate enough memory for jpg snap\n"); return 0; }


     create_index_page();


    int MAX_waittime=10000;
    int waittime=0;
    while ( ( !FeedReceiveLoopAlive(0) )&& (waittime<MAX_waittime) )
      {
          ++waittime;
      }

   return 1;
}

void * prepare_camera_data_callback()
{
   if (VideoSimulationState()!=LIVE_ON) { fprintf(stderr,"Camera already snapping\n"); return 0; }
   fprintf(stderr,"Calling Camera callback \n");
   jpg_snap_size=jpg_snap_full_size;
   RecordOneInMem((char*) "servedAtMem.jpg",0,1,jpg_snap_mem,&jpg_snap_size);
   while (VideoSimulationState()!=LIVE_ON) { usleep(1); } // Wait until recording is complete..!
   //jpg_snap_size=4096*4;//todo fix auto retrieval of this value in RecordOneInMem
   fprintf(stderr,"Calling Camera callback success new picture ( %u bytes long ) ready !\n",(unsigned int) jpg_snap_size);
   return 0;
}

int close_camera()
{
    if (jpg_snap_mem!=0) { free(jpg_snap_mem); jpg_snap_mem=0; }
    CloseVideoInputs();
    return 1;
}

int main(int argc, char *argv[])
{
    printf("Ammar Server starting up\n");

    char bindIP[MAX_INPUT_IP];
    strcpy(bindIP,"0.0.0.0");

    unsigned int port=DEFAULT_BINDING_PORT;

    if ( argc <1 ) { fprintf(stderr,"Something weird is happening , argument zero should be executable path :S \n"); return 1; } else
    if ( argc <= 2 ) { /*fprintf(stderr,"Please note that you may choose a different binding IP/port as command line parameters.. \"ammarserver 0.0.0.0 8080\"\n");*/ } else
     {
        if (strlen(argv[1])>=MAX_INPUT_IP) { fprintf(stderr,"Console argument for binding IP is too long..!\n"); } else
                                           { strncpy(bindIP,argv[1],MAX_INPUT_IP); }
        port=atoi(argv[2]);
        if (port>=MAX_BINDING_PORT) { port=DEFAULT_BINDING_PORT; }
     }
   if (argc>=3) { strncpy(webserver_root,argv[3],MAX_FILE_PATH); }
   if (argc>=4) { strncpy(templates_root,argv[4],MAX_FILE_PATH); }


    if ( open_camera(640,480,60) )
      {

        AmmServer_Start(bindIP,port,webserver_root,templates_root);

        AmmServer_AddResourceHandler(webserver_root,(char *) "/index.html",index_page_mem,&index_page_size,(void *) &prepare_index_page_callback);
        AmmServer_AddResourceHandler(webserver_root,(char *) "/cam.jpg",jpg_snap_mem,&jpg_snap_size,(void *) &prepare_camera_data_callback);

        while (AmmServer_Running())
           {
             usleep(10000);
           }

         close_camera();

        AmmServer_Stop();
      }


    fprintf(stderr,"Closing application\n");
    return 0;
}
