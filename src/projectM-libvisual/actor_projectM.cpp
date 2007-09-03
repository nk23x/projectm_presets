#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <string>
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <config.h>
#include <libvisual/libvisual.h>
#include <libprojectM/BeatDetect.hpp>
#include <libprojectM/PCM.hpp>
#include <libprojectM/projectM.hpp>
#include <libprojectM/console_interface.h>
#include "lvtoprojectM.h"
#include "ConfigFile.h"

#define CONFIG_FILE "/share/projectM/config.inp"




std::string read_config();

int texsize=512;
int gx=32,gy=24;
int wvw=512,wvh=512;
int fvw=1024,fvh=768;
int fps=30, fullscreen=0;

/* Private context sensitive data goes here, */
typedef struct {
	projectM *PM;
} ProjectmPrivate;

extern "C" int lv_projectm_init (VisPluginData *plugin);
extern "C" int lv_projectm_cleanup (VisPluginData *plugin);
extern "C" int lv_projectm_requisition (VisPluginData *plugin, int *width, int *height);
extern "C" int lv_projectm_dimension (VisPluginData *plugin, VisVideo *video, int width, int height);
extern "C" int lv_projectm_events (VisPluginData *plugin, VisEventQueue *events);
extern "C" VisPalette *lv_projectm_palette (VisPluginData *plugin);
extern "C" int lv_projectm_render (VisPluginData *plugin, VisVideo *video, VisAudio *audio);

VISUAL_PLUGIN_API_VERSION_VALIDATOR

/* Main plugin stuff */
/* The get_plugin_info function provides the libvisual plugin registry, and plugin loader
 * with the very basic plugin information */
extern "C" const VisPluginInfo *get_plugin_info (int *count)
{
  /* Initialize the plugin specific data structure
   * with pointers to the functions that represent
   * the plugin interface it's implementation, more info:
   * http://libvisual.sourceforge.net/newdocs/docs/html/struct__VisActorPlugin.html */
  static VisActorPlugin actor[1];
  static VisPluginInfo info[1];
  
  actor[0].requisition = lv_projectm_requisition;
  actor[0].palette = lv_projectm_palette;
  actor[0].render = lv_projectm_render;
    actor[0].vidoptions.depth = VISUAL_VIDEO_DEPTH_GL; /* We want GL clearly */
    
    
    info[0].type = VISUAL_PLUGIN_TYPE_ACTOR;
    
    info[0].plugname = "projectM";
    info[0].name = "libvisual projectM";
    info[0].author = "Peter Sperl";
    info[0].version = "1.00";
    info[0].about = "projectM";
    info[0].help =  "";

    info[0].init = lv_projectm_init;
    info[0].cleanup = lv_projectm_cleanup;
    info[0].events = lv_projectm_events;

    info[0].plugin = VISUAL_OBJECT (&actor[0]);
    *count = sizeof (info) / sizeof (*info);

	VISUAL_VIDEO_ATTRIBUTE_OPTIONS_GL_ENTRY(actor[0].vidoptions, VISUAL_GL_ATTRIBUTE_ALPHA_SIZE, 8);
	VISUAL_VIDEO_ATTRIBUTE_OPTIONS_GL_ENTRY(actor[0].vidoptions, VISUAL_GL_ATTRIBUTE_DEPTH_SIZE, 16);
	VISUAL_VIDEO_ATTRIBUTE_OPTIONS_GL_ENTRY(actor[0].vidoptions, VISUAL_GL_ATTRIBUTE_DOUBLEBUFFER, 1);

	VISUAL_VIDEO_ATTRIBUTE_OPTIONS_GL_ENTRY(actor[0].vidoptions, VISUAL_GL_ATTRIBUTE_RED_SIZE, 8);
	VISUAL_VIDEO_ATTRIBUTE_OPTIONS_GL_ENTRY(actor[0].vidoptions, VISUAL_GL_ATTRIBUTE_GREEN_SIZE, 8);
	VISUAL_VIDEO_ATTRIBUTE_OPTIONS_GL_ENTRY(actor[0].vidoptions, VISUAL_GL_ATTRIBUTE_BLUE_SIZE, 8);
    return info;
}


/* This function is called before we really start rendering, it's the init function */
extern "C" int lv_projectm_init (VisPluginData *plugin)
{
        char projectM_data[1024];
	ProjectmPrivate *priv;
	 std::string config_file;
 config_file = read_config();

 ConfigFile config(config_file);
        wvw = config.read<int>( "Window Width", 512 );
  wvh = config.read<int>( "Window Height", 512 );
 fullscreen = 0;
	/* Allocate the projectm private data structure, and register it as a private */

	priv = new ProjectmPrivate;
	visual_mem_set (priv, 0, sizeof (ProjectmPrivate));


	//priv = visual_mem_new0 (ProjectmPrivate, 1);
	visual_object_set_private (VISUAL_OBJECT (plugin), priv);

	//FIXME
	priv->PM = new projectM(config_file);
	//globalPM = (projectM *)wipemalloc( sizeof( projectM ) );
       
    

	return 0;
}

extern "C" int lv_projectm_cleanup (VisPluginData *plugin)
{
	ProjectmPrivate *priv = (ProjectmPrivate*)visual_object_get_private (VISUAL_OBJECT (plugin));

	/* Cleanup, and thus also free our private */
	visual_mem_free (priv->PM);
	visual_mem_free (priv);
	return 0;
}

/* This is used to ask a plugin if it can handle a certain size, and if not, to
 * set the size it wants by putting a value in width, height that represents the
 * required size */
extern "C" int lv_projectm_requisition (VisPluginData *plugin, int *width, int *height)
{
	int reqw, reqh;

	/* Size negotiate with the window */
	reqw = *width;
	reqh = *height;

	if (reqw < 64)
		reqw = 64;

	if (reqh < 64)
		reqh = 64;

	*width = reqw;
	*height = reqh;

	return 0;
}

extern "C" int lv_projectm_dimension (VisPluginData *plugin, VisVideo *video, int width, int height)
{
	ProjectmPrivate *priv = (ProjectmPrivate*)visual_object_get_private (VISUAL_OBJECT (plugin));

	visual_video_set_dimension (video, width, height);

	priv->PM->projectM_resetGL( width, height ); 

	return 0;
}

/* This is the main event loop, where all kind of events can be handled, more information
 * regarding these can be found at:
 * http://libvisual.sourceforge.net/newdocs/docs/html/union__VisEvent.html 
 */
extern "C" int lv_projectm_events (VisPluginData *plugin, VisEventQueue *events)
{
	ProjectmPrivate *priv = (ProjectmPrivate*)visual_object_get_private (VISUAL_OBJECT (plugin));
	VisEvent ev;
	VisParamEntry *param;

	projectMEvent evt;
        projectMKeycode key;
        projectMModifier mod;
	
	while (visual_event_queue_poll (events, &ev)) 
	  {
	    switch (ev.type) 
	      {
	      case VISUAL_EVENT_KEYUP:
		
		evt = lv2pmEvent( ev.type );
		key = lv2pmKeycode( ev.event.keyboard.keysym.sym );
		mod = lv2pmModifier( ev.event.keyboard.keysym.mod );
		priv->PM->key_handler(PROJECTM_KEYDOWN, key,mod);

		break;
	      case VISUAL_EVENT_RESIZE:
		lv_projectm_dimension (plugin, ev.event.resize.video,
				       ev.event.resize.width, ev.event.resize.height);
		break;
		
	      default: /* to avoid warnings */
		break;
	      }
	}
	
	return 0;
}

/* Using this function we can update the palette when we're in 8bits mode, which
 * we aren't with projectm, so just ignore :) */
extern "C" VisPalette *lv_projectm_palette (VisPluginData *plugin)
{
	return NULL;
}

/* This is where the real rendering happens! This function is what we call, many times
 * a second to get our graphical frames. */
extern "C" int lv_projectm_render (VisPluginData *plugin, VisVideo *video, VisAudio *audio)
{
	ProjectmPrivate *priv = (ProjectmPrivate*)visual_object_get_private (VISUAL_OBJECT (plugin));
	VisBuffer pcmb;
	float pcm[2][512];
	//short pcms[2][512];
	int i;

	visual_buffer_set_data_pair (&pcmb, pcm[0], sizeof (pcm[0]));
	visual_audio_get_sample (audio, &pcmb, VISUAL_AUDIO_CHANNEL_LEFT);

	visual_buffer_set_data_pair (&pcmb, pcm[1], sizeof (pcm[1]));
	visual_audio_get_sample (audio, &pcmb, VISUAL_AUDIO_CHANNEL_RIGHT);

	/*
	for (i = 0; i < 512; i++) {
		pcms[0][i] = pcm[0][i] * 32768.0;
		pcms[1][i] = pcm[1][i] * 32768.0;
	}

	addPCM16Data(pcms,512);
	*/

	priv->PM->beatDetect->pcm->addPCMfloat(*pcm,512);
	
	priv->PM->renderFrame();

	return 0;
}



std::string read_config()
{

   int n;
   
   char num[512];
   FILE *in; 
   FILE *out;

   char* home;
   char projectM_home[1024];
   char projectM_config[1024];

   strcpy(projectM_config, PROJECTM_DATADIR);
   strcpy(projectM_config+strlen(PROJECTM_DATADIR), CONFIG_FILE);
   projectM_config[strlen(PROJECTM_DATADIR)+strlen(CONFIG_FILE)]='\0';
   printf("dir:%s \n",projectM_config);
   home=getenv("HOME");
   strcpy(projectM_home, home);
   strcpy(projectM_home+strlen(home), "/.projectM/config.inp");
   projectM_home[strlen(home)+strlen("/.projectM/config.inp")]='\0';

  
 if ((in = fopen(projectM_home, "r")) != 0) 
   {
     printf("reading ~/.projectM/config.inp \n");
     fclose(in);
     return std::string(projectM_home);
   }
 else
   {
     printf("trying to create ~/.projectM/config.inp \n");

     strcpy(projectM_home, home);
     strcpy(projectM_home+strlen(home), "/.projectM");
     projectM_home[strlen(home)+strlen("/.projectM")]='\0';
     mkdir(projectM_home,0755);

     strcpy(projectM_home, home);
     strcpy(projectM_home+strlen(home), "/.projectM/config.inp");
     projectM_home[strlen(home)+strlen("/.projectM/config.inp")]='\0';
     
     if((out = fopen(projectM_home,"w"))!=0)
       {
	
	 if ((in = fopen(projectM_config, "r")) != 0) 
	   {
	     
	     while(fgets(num,80,in)!=NULL)
	       {
		 fputs(num,out);
	       }
	     fclose(in);
	     fclose(out);
	    

	     if ((in = fopen(projectM_home, "r")) != 0) 
	       { 
		 printf("created ~/.projectM/config.inp successfully\n");  
		 fclose(in);
		 return std::string(projectM_home);
	       }
	     else{printf("This shouldn't happen, using implementation defualts\n");abort();}
	   }
	 else{printf("Cannot find projectM default config, using implementation defaults\n");abort();}
       }
     else
       {
	 printf("Cannot create ~/.projectM/config.inp, using default config file\n");
	 if ((in = fopen(projectM_config, "r")) != 0) 
	   { printf("Successfully opened default config file\n");
	     fclose(in);
	     return std::string(projectM_config);}
	 else{ printf("Using implementation defaults, your system is really messed up, I'm suprised we even got this far\n");  abort();}
	   
       }

   }


 /*
     fgets(num, 512, in);  fgets(num, 512, in);  fgets(num, 512, in);
     if(fgets(num, 512, in) != NULL) sscanf (num, "%d", &texsize);  

     fgets(num, 512, in);
     if(fgets(num, 512, in) != NULL) sscanf (num, "%d", &gx);  

     fgets(num, 512, in);
     if(fgets(num, 512, in) != NULL) sscanf (num, "%d", &gy);   

     fgets(num, 512, in);
     if(fgets(num, 512, in) != NULL) sscanf (num, "%d", &wvw);  

     fgets(num, 512, in);
     if(fgets(num, 512, in) != NULL) sscanf (num, "%d", &wvh);  
   
     fgets(num, 512, in);
     if(fgets(num, 512, in) != NULL) sscanf (num, "%d", &fps);

     fgets(num, 512, in);
     if(fgets(num, 512, in) != NULL) sscanf (num, "%d", &fullscreen);
     fgets(num, 512, in);

     if(fgets(num, 512, in) != NULL)  strcpy(preset_dir, num);
     preset_dir[strlen(preset_dir)-1]='\0';
 */ 
     /*
     fgets(num, 80, in);
     fgets(num, 80, in);
     
     n=0;
     while (num[n]!=' ' && num[n]!='\n' && n < 80 && num[n]!=EOF)
       {
	 	 disp[n]=num[n];
		 n++;
       }
     disp[n]=0;

    
     // sprintf(disp,"%s",num );
      setenv("DISPLAY",disp,1);
      printf("%s %d\n", disp,strlen(disp));
      setenv("LD_PRELOAD", "/usr/lib/tls/libGL.so.1.0.4496", 1);
     */
 // fclose(in); 
  
 abort();
} 

