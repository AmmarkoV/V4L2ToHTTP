/*
V4L2ToHTTP , main executable

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


#include "../VideoInput/VideoInput.h"
#include <linux/videodev2.h>

#define SIMPLE_INDEX 0
#define ENABLE_PASSWORD_PROTECTION 1

char webcam[MAX_FILE_PATH]="/dev/video0";
char webserver_root[MAX_FILE_PATH]="public_html/";
char templates_root[MAX_FILE_PATH]="public_html/templates/";



struct AmmServer_RH_Context index_page= {0};

struct AmmServer_RH_Context jpeg_picture= {0};
unsigned int jpg_width=320,jpg_height=240;


void * prepare_index_page_callback(unsigned int ignored_associated_vars)
{
  if (SIMPLE_INDEX)
    {
      strcpy(index_page.content,"<html><head><meta http-equiv=\"refresh\" content=\"1\"><title>V4L2ToHTTP</title></head><body><br><br><center><img src=\"cam.jpg\"><br><h3><a href=\"index.html\">-- Manually Reload Page --</a></h3></center></body></html>");
    }
  else
    {
      strcpy(index_page.content,"<html>\n");
      strcat(index_page.content,"    <head>\n");
      strcat(index_page.content,"         <title>V4L2ToHTTP</title>\n");
      strcat(index_page.content,"         <script language=\"JavaScript\"><!--\n");
      strcat(index_page.content,"                var newImage = new Image();\n");
      strcat(index_page.content,"                var number = 0;\n");
      strcat(index_page.content,"                newImage.src = \"cam.jpg?i=0\";\n");
      strcat(index_page.content,"                 function updateImage()\n");
      strcat(index_page.content,"                    {\n");
      strcat(index_page.content,"                           if(newImage.complete)\n");
      strcat(index_page.content,"                           {\n");
      strcat(index_page.content,"                                document.getElementById(\"LiveImage\").src = newImage.src;\n");
      strcat(index_page.content,"                                if(newImage.complete)\n");
      strcat(index_page.content,"                                 {\n");
      strcat(index_page.content,"                                      document.getElementById(\"LiveImage\").src = newImage.src;\n");
      strcat(index_page.content,"                                      newImage = new Image();\n");
      strcat(index_page.content,"                                      number++;\n");
      strcat(index_page.content,"                                     newImage.src = \"cam.jpg?i=\"+number\n"); //Later on make it cam.jpg?inc=\" + number;
      strcat(index_page.content,"                                   }\n");
      strcat(index_page.content,"                        }\n                     setTimeout('updateImage()',550);\n");
      strcat(index_page.content,"                   }\n");
      strcat(index_page.content,"               //--></script>\n");
      strcat(index_page.content,"</head>\n");
      strcat(index_page.content,"<body  onLoad=\"setTimeout('updateImage()',550)\">\n");
      strcat(index_page.content,"<br><br><center><img src=\"cam.jpg?i=0\" id=\"LiveImage\"><br>");
      strcat(index_page.content,"<h5>Active Clients : ");

      if (AmmServer_GetInfo(AMMINF_ACTIVE_CLIENTS) < 10000 )
           {
             char viewers_str[150]={0};
             sprintf(viewers_str,"%u",AmmServer_GetInfo(AMMINF_ACTIVE_CLIENTS));
             strcat(index_page.content,viewers_str);
           } else
           {
               strcat(index_page.content,"N/A");
           }

      strcat(index_page.content,"<br>\n");
      strcat(index_page.content,"<br><a href=\"index.html\">-- Manually Reload Page --</a>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\n");
      strcat(index_page.content,"<a href=\"https://github.com/AmmarkoV/V4L2ToHTTP\">-- Get V4L2ToHTTP --</a></h5></center></body></html>\n");
    }

  index_page.MAX_content_size=strlen((char *)index_page.content);
  index_page.content_size=index_page.MAX_content_size;
  return 0;
}

int open_camera(char * webcam_dev,unsigned int width,unsigned int height,unsigned int framerate)
{
  if ( !InitVideoInputs(1) )
    {
      fprintf(stderr,"Could not open Video Input\n");
      return 0;
    }

  char BITRATE=32;
  struct VideoFeedSettings feedsettings= {0};
  //videosettings.PixelFormat=V4L2_PIX_FMT_YUYV; BITRATE=16; // <- Common compressed setting for UVC webcams
  feedsettings.PixelFormat=V4L2_PIX_FMT_RGB24;
  BITRATE=24;   //   <- Common raw setting for UVC webcams ( Run Compat )
  jpg_width=width;
  jpg_height=height;

  if (! InitVideoFeed(0,webcam_dev,jpg_width,jpg_height,BITRATE,framerate,1,feedsettings) )
    {
      fprintf(stderr,"Could not set Video feed settings consider running with v4l2convert.so preloaded\n");
      return 0;
    }

  int MAX_waittime=10000;
  int waittime=0;
  while ( ( !FeedReceiveLoopAlive(0) )&& (waittime<MAX_waittime) )
    {
      ++waittime;
    }

  return 1;
}

void * prepare_camera_data_callback(unsigned int ignored_associated_vars)
{
  if (VideoSimulationState()!=LIVE_ON)
    {
      fprintf(stderr,"Camera already snapping\n");
      return 0;
    }
  fprintf(stderr,"Calling Camera callback \n");
  jpeg_picture.content_size=jpg_width * jpg_height * 3; //Signal the max allocated buffer , this value will be changed by RecordOneInMem
  RecordOneInMem((char*) "servedAtMem_DummyFilename.jpg",0,1,jpeg_picture.content,&jpeg_picture.content_size);
  while (VideoSimulationState()!=LIVE_ON)
    {
      usleep(1);  // Wait until recording is complete..!
    }

  fprintf(stderr,"Calling Camera callback success new picture ( %u bytes long ) ready !\n",(unsigned int) jpeg_picture.content_size);
  return 0;
}

int close_camera()
{
  if (jpeg_picture.content!=0)
    {
      free(jpeg_picture.content);
      jpeg_picture.content=0;
    }
  CloseVideoInputs();
  return 1;
}

void init_dynamic_pages()
{
  //int AmmServer_AddResourceHandler(struct AmmServer_RH_Context * context, char * resource_name , char * web_root, unsigned int allocate_mem_bytes,unsigned int callback_every_x_msec,void * callback);
  AmmServer_AddResourceHandler(&index_page,(char *) "/index.html",webserver_root,4096,0,(void *) &prepare_index_page_callback);

  //Do not empty jpeg_picture struct since mallocs have already happened.. memset(&jpeg_picture,0,sizeof(struct AmmServer_RH_Context));

  AmmServer_AddResourceHandler(&jpeg_picture,(char *) "/cam.jpg",webserver_root,jpg_width * jpg_height * 3, 500 /*Poll camera no sooner than once every x ms*/,(void *) &prepare_camera_data_callback);
  jpeg_picture.content_size=jpg_width * jpg_height * 3;
  jpeg_picture.MAX_content_size=jpg_width * jpg_height * 3;

  prepare_camera_data_callback(0); //Do a callback to populate content..!
}

void close_dynamic_pages()
{

}


int main(int argc, char *argv[])
{
  printf("Ammar Server starting up\n");

  char bindIP[MAX_INPUT_IP];
  strcpy(bindIP,"0.0.0.0");

  unsigned int port=DEFAULT_BINDING_PORT;

  if ( argc <1 )
    {
      fprintf(stderr,"Something weird is happening , argument zero should be executable path :S \n");
      return 1;
    }
  else if ( argc <= 2 )
    {
      /*fprintf(stderr,"Please note that you may choose a different binding IP/port as command line parameters.. \"ammarserver 0.0.0.0 8080\"\n");*/
    }
  else
    {
      if (strlen(argv[1])>=MAX_INPUT_IP)
        {
          fprintf(stderr,"Console argument for binding IP is too long..!\n");
        }
      else
        {
          strncpy(bindIP,argv[1],MAX_INPUT_IP);
        }
      port=atoi(argv[2]);
      if (port>=MAX_BINDING_PORT)
        {
          port=DEFAULT_BINDING_PORT;
        }
    }
  if (argc>=3)
    {
      strncpy(webcam,argv[3],MAX_FILE_PATH);
    }

  if ( open_camera(webcam,640,480,60) )
    {
      //We were able to start camera so lets start everything else

      AmmServer_Start(bindIP,port,0,webserver_root,templates_root);

  if (ENABLE_PASSWORD_PROTECTION)
   {
     AmmServer_SetStrSettingValue(AMMSET_USERNAME_STR,(char*) "admin");
     AmmServer_SetStrSettingValue(AMMSET_PASSWORD_STR,(char*) "admin"); //these 2 calls should change BASE64PASSWORD to the proper hex value
    /* It is best to enable password protection after correctly setting both username and password
       to avoid the rare race condition of logging only with username ( i.e. when password hasn't been declared */
     AmmServer_SetIntSettingValue(AMMSET_PASSWORD_PROTECTION,1);
   }

      init_dynamic_pages(); // Map index.html , cam.jpg to their content and callbacks

      while (AmmServer_Running())
        {
          //Serving pages on background..!
          usleep(10000);
        }

      close_dynamic_pages(); // Free mappings of AmmServer

      close_camera();

      AmmServer_Stop();
    }


  fprintf(stderr,"Closing application\n");
  return 0;
}
