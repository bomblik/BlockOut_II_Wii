// -------------------------------------------
// SDL/OpenGL OpenGL application framework
// Jean-Luc PONS (2007)
// -------------------------------------------
#include "GLApp.h"
#ifndef WINDOWS
#include <unistd.h>
#endif

#ifdef PLATFORM_WII
#include <gccore.h>
#include <wiiuse/wpad.h>

#include <fat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <zbuffer.h>

#include <math.h>

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

#include <GL/gl.h>
#include <GL/glu.h>

ZBuffer *frameBuffer;
static unsigned int pitch;
#endif

#undef LoadImage

#include <CImage.h>

extern char *LID(char *fileName);

// -------------------------------------------

GLApplication::GLApplication() {

  m_bWindowed = true;
  m_bVSync = false;
  m_strWindowTitle = (char *)"GL application";
  strcpy((char *)m_strFrameStats,"");

#if defined(PLATFORM_PSP)
  m_screenWidth = 480;
  m_screenHeight = 272;
#elif defined(PLATFORM_PSVITA)
  m_screenWidth = 960;
  m_screenHeight = 544;
#else
  m_screenWidth = 640;
  m_screenHeight = 480;
#endif

}

// -------------------------------------------

int GLApplication::SetVideoMode() {

#if !defined(PLATFORM_PSP) && !defined(PLATFORM_PSVITA) && !defined(PLATFORM_WII)
  // Enable/Disable vertical blanking 
  // Work only at application startup
  SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL,m_bVSync);
#endif

  // Set the video mode
  Uint32 flags;

#if defined(PLATFORM_PSP)
  flags = SDL_DOUBLEBUF | SDL_OPENGL | SDL_FULLSCREEN;
#elif defined(PLATFORM_PSVITA) || defined(PLATFORM_WII) 
  flags = SDL_SWSURFACE | SDL_ANYFORMAT;
#else
  if( m_bWindowed ) flags = SDL_DOUBLEBUF | SDL_OPENGL;
  else              flags = SDL_DOUBLEBUF | SDL_OPENGL | SDL_FULLSCREEN;
#endif

#if !defined(PLATFORM_PSP) && !defined(PLATFORM_PSVITA) && !defined(PLATFORM_WII)
  if( SDL_SetVideoMode( m_screenWidth, m_screenHeight, 0, flags ) == NULL )
#elif defined(PLATFORM_WII)
  if( SDL_SetVideoMode( m_screenWidth, m_screenHeight, 24, flags ) == NULL )
#else
  if( SDL_SetVideoMode( m_screenWidth, m_screenHeight, 32, flags ) == NULL )
#endif
  {
#ifdef WINDOWS
    char message[256];
	sprintf(message,"SDL_SetVideoMode() failed : %s\n",SDL_GetError());
	MessageBox(NULL,message,"ERROR",MB_OK|MB_ICONERROR);
#else
    printf("SDL_SetVideoMode() failed : %s\n",SDL_GetError());
#endif
    return GL_FAIL;
  }

  if( !m_bWindowed )
    SDL_ShowCursor(SDL_DISABLE); 
  else
    SDL_ShowCursor(SDL_ENABLE); 

  return GL_OK;

}

// -------------------------------------------

int GLApplication::ToggleFullscreen() {

  int errCode;

  InvalidateDeviceObjects();

  m_bWindowed = !m_bWindowed;

#if !defined(PLATFORM_PSVITA)
  if( !SetVideoMode() ) return GL_FAIL;

  SDL_Surface *vSurf = SDL_GetVideoSurface();
  m_bitsPerPixel = vSurf->format->BitsPerPixel;
#endif

  errCode = RestoreDeviceObjects();
  if( !errCode ) {
    printGlError();
    exit(1);
  }

  return GL_OK;
    
}

// -------------------------------------------

int GLApplication::Create(int width, int height, BOOL bFullScreen, BOOL bVSync ) {

  int errCode;

  m_bVSync = bVSync;
  m_screenWidth = width;
  m_screenHeight = height;
  m_bWindowed = !bFullScreen;
  
#if !defined(WINDOWS) && !defined(PLATFORM_PSVITA) && !defined(PLATFORM_WII)
  if( getenv("DISPLAY")==NULL ) {
    printf("Warning, DISPLAY not defined, it may not work.\n");
  }
#endif

  //Initialize SDL
#if !defined(PLATFORM_PSVITA) && !defined(PLATFORM_WII)
  if( SDL_Init( SDL_INIT_EVERYTHING ) < 0 )
#else
  if( SDL_Init( SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK ) < 0 )
#endif
  {
#ifdef WINDOWS
    char message[256];
	sprintf(message,"SDL_Init() failed : %s\n" , SDL_GetError() );
	MessageBox(NULL,message,"Error",MB_OK|MB_ICONERROR);
#else
    printf("SDL_Init() failed : %s\n" , SDL_GetError() );
#endif
	return GL_FAIL;    
  }

#if !defined(PLATFORM_PSP) && !defined(PLATFORM_PSVITA) && !defined(PLATFORM_WII)
  SDL_WM_SetCaption(m_strWindowTitle, NULL);

  //SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 0);
#else
  SDL_JoystickEventState (SDL_ENABLE);

  if(SDL_NumJoysticks()>0)
  {
      joy=SDL_JoystickOpen(0);
  }
#endif

#if defined(PLATFORM_PSVITA)
  vglInit();
#endif

  SDL_EnableUNICODE( 1 );
  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,SDL_DEFAULT_REPEAT_INTERVAL);

  //Create Window
  if( !SetVideoMode() ) return GL_FAIL;

#if !defined(PLATFORM_PSVITA)
  SDL_Surface *vSurf = SDL_GetVideoSurface();
  m_bitsPerPixel = vSurf->format->BitsPerPixel;
#endif

#if defined(PLATFORM_PSVITA) && defined(GEKIHEN_CONTEST_SPLASH_SCREEN)
  SDL_Surface *surface= SDL_GetVideoSurface();
  BYTE *pixels = NULL;

  CImage img;

  if( !img.LoadImage(LID("images.psvita/gekihen-splash.png")) )
  {
    printf( "loading splash screen error\n");
  }

  BYTE *data = img.GetData();

  if( SDL_MUSTLOCK( surface ) )
  {
    SDL_LockSurface( surface );
  }

  pixels = (BYTE *)surface->pixels;

  for(int j=0;j<544;j++)
    for(int i=0;i<960;i++ ) {
      pixels[i*4+0 + j*960*4] = data[i*3+2 + j*960*3];
      pixels[i*4+1 + j*960*4] = data[i*3+1 + j*960*3];
      pixels[i*4+2 + j*960*4] = data[i*3+0 + j*960*3];
      pixels[i*4+3 + j*960*4] = 0;
  }

  if( SDL_MUSTLOCK( surface ) )
  {
    SDL_UnlockSurface( surface );
  }

  SDL_Flip (surface);
  sceKernelDelayThread(3*1000000);
#endif

#if defined(PLATFORM_WII)
    // initialize TinyGL
    int	mode;
    SDL_Surface * screen = SDL_GetVideoSurface();
    switch(screen->format->BitsPerPixel) {
    case  8:
        fprintf(stderr,"ERROR: Palettes are currently not supported.\n");
        return 1;
    case 16:
        pitch = screen->pitch;
        mode = ZB_MODE_5R6G5B;
        break;
    case 24:
        pitch = (screen->pitch * 2) / 3;
        mode = ZB_MODE_RGB24;
        break;
    case 32:
        pitch = screen->pitch / 2;
        mode = ZB_MODE_RGBA;
        break;
    default:
        return 1;
        break;
    }
    frameBuffer = ZB_open(m_screenWidth, m_screenHeight, mode, 0, 0, 0, 0);
    glInit(frameBuffer);

    WPAD_Init();
#endif


  OneTimeSceneInit();
  errCode = RestoreDeviceObjects();
#if !defined(PLATFORM_PSP) && !defined(PLATFORM_PSVITA)  && !defined(PLATFORM_WII)
  if( !errCode ) {
    printGlError();
    exit(0);
  }
#endif

  return GL_OK;

}

// -------------------------------------------

void GLApplication::Pause(BOOL bPause) {
}

// -------------------------------------------

int GLApplication::Resize( DWORD width, DWORD height ) {

  int errCode;
  m_screenWidth = width;
  m_screenHeight = height;

  InvalidateDeviceObjects();

#if !defined(PLATFORM_PSVITA)
  if( !SetVideoMode() ) return GL_FAIL;

  SDL_Surface *vSurf = SDL_GetVideoSurface();
  m_bitsPerPixel = vSurf->format->BitsPerPixel;
#endif

  errCode = RestoreDeviceObjects();
  if( !errCode ) {
    printGlError();
    exit(1);
  }

  return GL_OK;

}

// -------------------------------------------

int GLApplication::Run() {

  SDL_Event event;

  bool quit = false;
  int  nbFrame = 0;
  int  lastTick = 0;
  int  lastFrTick = 0;
  int  errCode;
  int  fTick;
  int  firstTick;

  m_fTime        = 0.0f;
  m_fElapsedTime = 0.0f;
  m_fFPS         = 0.0f;
  lastTick = lastFrTick = firstTick = SDL_GetTicks();

  //Wait for user exit
  while( quit == false )
  {
     
     //While there are events to handle
     while( SDL_PollEvent( &event ) ) {
       if( event.type == SDL_QUIT )
         quit = true;
       else
         EventProc(&event);
     }

     fTick = SDL_GetTicks();

     // Update timing
     nbFrame++;
     if( (fTick - lastTick) >= 1000 ) {
        int t0 = fTick;
        int t = t0 - lastTick;
        m_fFPS = (float)(nbFrame*1000) / (float)t;
        nbFrame = 0;
        lastTick = t0;
        sprintf(m_strFrameStats,"%.2f fps (%dx%dx%d)",m_fFPS,m_screenWidth,m_screenHeight,m_bitsPerPixel);
     }

     m_fTime = (float) ( fTick - firstTick ) / 1000.0f;
     m_fElapsedTime = (fTick - lastFrTick) / 1000.0f;
     lastFrTick = fTick;

     if(!quit) errCode = FrameMove();
     if( !errCode ) quit = true;

#if !defined(PLATFORM_PSVITA) && !defined(PLATFORM_WII)
     if( glGetError() != GL_NO_ERROR ) { printGlError(); quit = true; }
#endif

     if(!quit) errCode = Render();
     if( !errCode ) quit = true;

#if !defined(PLATFORM_PSVITA) && !defined(PLATFORM_WII)
     if( glGetError() != GL_NO_ERROR ) { printGlError(); quit = true; }
#endif

     //Swap buffer
#if defined(PLATFORM_PSVITA)
     vglSwap();
#elif defined(PLATFORM_WII)
      SDL_Surface *screen = SDL_GetVideoSurface();

      // swap buffers:
      if (SDL_MUSTLOCK(screen) && (SDL_LockSurface(screen) < 0)) {
          fprintf(stderr, "SDL ERROR: Can't lock screen: %s\n", SDL_GetError());
          return 1;
      }
      ZB_copyFrameBuffer(frameBuffer, screen->pixels, pitch);
      if (SDL_MUSTLOCK(screen)) {
        SDL_UnlockSurface(screen);
      }

      SDL_Flip(screen);
#else
     SDL_GL_SwapBuffers();
#endif
  }

#if defined(PLATFORM_PSVITA)
  vglClose();
#endif
  
  //Clean up
  SDL_Quit();
  
  return GL_OK;

}

// -------------------------------------------

void GLApplication::SetMaterial(GLMATERIAL *mat) {

  float acolor[] = { mat->Ambient.r, mat->Ambient.g, mat->Ambient.b, mat->Ambient.a };
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, acolor);
  float dcolor[] = { mat->Diffuse.r, mat->Diffuse.g, mat->Diffuse.b, mat->Diffuse.a };
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, dcolor);
  float scolor[] = { mat->Specular.r, mat->Specular.g, mat->Specular.b, mat->Specular.a };
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, scolor);
  float ecolor[] = { mat->Emissive.r, mat->Emissive.g, mat->Emissive.b, mat->Emissive.a };
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, ecolor);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat->Power);
  glColor4f(mat->Ambient.r, mat->Ambient.g, mat->Ambient.b, mat->Ambient.a);

}

// -------------------------------------------

void GLApplication::printGlError() {
  
  char message[256];

#if !defined(PLATFORM_PSVITA) && !defined(PLATFORM_WII)
  GLenum errCode = glGetError();
#else
  GLenum errCode = 0;
#endif

  switch( errCode ) {
    case GL_INVALID_ENUM:
      sprintf(message,"OpenGL failure: An unacceptable value is specified for an enumerated argument. The offending function is ignored, having no side effect other than to set the error flag.");
      break;
    case GL_INVALID_VALUE:
	  sprintf(message, "OpenGL failure: A numeric argument is out of range. The offending function is ignored, having no side effect other than to set the error flag.");
      break;
    case GL_INVALID_OPERATION:
	  sprintf(message, "OpenGL failure: The specified operation is not allowed in the current state. The offending function is ignored, having no side effect other than to set the error flag.");
      break;
    case GL_STACK_OVERFLOW:
	  sprintf(message, "OpenGL failure: This function would cause a stack overflow. The offending function is ignored, having no side effect other than to set the error flag.");
      break;
    case GL_STACK_UNDERFLOW:
	  sprintf(message, "OpenGL failure: This function would cause a stack underflow. The offending function is ignored, having no side effect other than to set the error flag.");
      break;
    case GL_OUT_OF_MEMORY:
      sprintf(message, "OpenGL failure: There is not enough memory left to execute the function. The state of OpenGL is undefined, except for the state of the error flags, after this error is recorded.");
      break;
	default:
      sprintf(message, "Application failure.");
	  break;
  }

#ifdef WINDOWS
  MessageBox(NULL, message, "Error", MB_OK | MB_ICONERROR);
#else
  printf(message);
#endif

}
