// -----------------------------------------------
// 2D sprites
// -----------------------------------------------
#include "GLSprite.h"
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#undef LoadImage

#include <CImage.h>

#if defined(PLATFORM_WII)
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

#include "SDL/SDL.h"

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

static unsigned int pitch;
#endif

#if defined(PSVITA_DEBUG)
#define printf(...) psp2shell_print(__VA_ARGS__)
#endif

extern char *LID(char *fileName);

// -------------------------------------------

Sprite2D::Sprite2D() {
}

// -------------------------------------------

void Sprite2D::SetSpriteMapping(float mx1,float my1,float mx2,float my2) {

  this->mx1 = mx1;
  this->my1 = my1;
  this->mx2 = mx2;
  this->my2 = my2;

}

// -------------------------------------------
#if !defined(PLATFORM_PSVITA) && !defined(PLATFORM_WII)
void Sprite2D::UpdateSprite(int x1,int y1,int x2,int y2,float mx1,float my1,float mx2,float my2) {

  this->x1 = x1;
  this->y1 = y1;
  this->x2 = x2;
  this->y2 = y2;
  this->mx1 = mx1;
  this->my1 = my1;
  this->mx2 = mx2;
  this->my2 = my2;

}

// -------------------------------------------

void Sprite2D::UpdateSprite(int x1,int y1,int x2,int y2) {

  this->x1 = x1;
  this->y1 = y1;
  this->x2 = x2;
  this->y2 = y2;

}
#else
void Sprite2D::UpdateSprite(float x1,float y1,float x2,float y2,float mx1,float my1,float mx2,float my2) {

  this->x1 = x1;
  this->y1 = y1;
  this->x2 = x2;
  this->y2 = y2;
  this->mx1 = mx1;
  this->my1 = my1;
  this->mx2 = mx2;
  this->my2 = my2;

}

// -------------------------------------------

void Sprite2D::UpdateSprite(float x1,float y1,float x2,float y2) {

  this->x1 = x1;
  this->y1 = y1;
  this->x2 = x2;
  this->y2 = y2;

}
#endif

// -------------------------------------------

int Sprite2D::RestoreDeviceObjects(char *diffName,char *alphaName,int scrWidth,int scrHeight) {

  GLint  bpp;
  GLenum format;

  BYTE *buff32;

  // Load the image
  CImage img;
  CImage imga;

  if( !img.LoadImage(LID(diffName)) ) {
#ifdef WINDOWS
    char message[256];
	sprintf(message,"Failed to load \"%s\"\n",LID(diffName));
	MessageBox(NULL,message,"ERROR",MB_OK|MB_ICONERROR);
#else
    printf("Failed to load \"%s\"\n",LID(diffName));
#endif
    return 0;
  }

  hasAlpha = strcmp(alphaName,"none")!=0;

#if defined(PLATFORM_PSVITA) || defined(PLATFORM_WII)
  hasAlpha = 0;
#endif

  if( hasAlpha ) {
    if( !imga.LoadImage(LID(alphaName)) ) {
#ifdef WINDOWS
      char message[256];
	  sprintf(message,"Failed to load \"%s\"\n",LID(alphaName));
  	  MessageBox(NULL,message,"ERROR",MB_OK|MB_ICONERROR);
#else
      printf("Failed to load \"%s\"\n",LID(alphaName));
#endif
      return 0;
    }
  }

  // Make 32 Bit RGB/RGBA buffer
  int fWidth  = img.Width();
  int fHeight = img.Height();

  if( hasAlpha ) {

    format = GL_RGBA;
    bpp    = 4;
    buff32 = (BYTE *)malloc(fWidth*fHeight*4);
    BYTE *data   = img.GetData();
    BYTE *adata   = imga.GetData();

    for(int y=0;y<fHeight;y++) {
      for(int x=0;x<fWidth;x++) {
        buff32[x*4 + 0 + y*4*fWidth] = data[x*3+2 + y*3*fWidth];
        buff32[x*4 + 1 + y*4*fWidth] = data[x*3+1 + y*3*fWidth];
        buff32[x*4 + 2 + y*4*fWidth] = data[x*3+0 + y*3*fWidth];
        buff32[x*4 + 3 + y*4*fWidth] = adata[x*3+1 + y*3*fWidth];
      }
    }

  } else {

    format = GL_RGB;
    bpp    = 3;
    buff32 = (BYTE *)malloc(fWidth*fHeight*3);
    BYTE *data   = img.GetData();

    for(int y=0;y<fHeight;y++) {
      for(int x=0;x<fWidth;x++) {
        buff32[x*3 + 0 + y*3*fWidth] = data[x*3 + 2 + y*3*fWidth];
        buff32[x*3 + 1 + y*3*fWidth] = data[x*3 + 1 + y*3*fWidth];
        buff32[x*3 + 2 + y*3*fWidth] = data[x*3 + 0 + y*3*fWidth];
      }
    }

  }

  glGenTextures(1,&texId);
  glBindTexture(GL_TEXTURE_2D,texId);

#if !defined(PLATFORM_PSVITA) && !defined(PLATFORM_WII)
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // scale linearly when image bigger than texture
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // scale linearly when image smalled than texture
#else
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
#endif

  glTexImage2D (
    GL_TEXTURE_2D,      // Type
    0,                  // No Mipmap
    bpp,                // Byte per pixel
    fWidth,             // Width
    fHeight,            // Height
    0,                  // Border
    format,             // Format RGB/RGBA
    GL_UNSIGNED_BYTE,   // 8 Bit/color
    buff32              // Data
  );   

  free(buff32);
  img.Release();
  imga.Release();

#if !defined(PLATFORM_PSVITA) && !defined(PLATFORM_WII)
  if( glGetError() != GL_NO_ERROR )
  {
#ifdef WINDOWS
      char message[256];
	  sprintf(message,"Sprite2D::RestoreDeviceObjects(): Failed to create font texture: glcode=%d\n",glGetError());
  	  MessageBox(NULL,message,"ERROR",MB_OK|MB_ICONERROR);
#else
    printf("Sprite2D::RestoreDeviceObjects(): Failed to create font texture: glcode=%d\n",glGetError());
#endif
    return 0;    
  }
#endif

  // Compute othographic matrix (for Transfomed Lit vertex)
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  //glOrtho( 0, scrWidth, scrHeight, 0, -1, 1 );
  glGetFloatv( GL_PROJECTION_MATRIX , pMatrix );

  return 1;

}

// -------------------------------------------

void Sprite2D::InvalidateDeviceObjects() {

  if(texId) glDeleteTextures(1, &texId);
  texId = 0;

}

// -------------------------------------------

void Sprite2D::Render() {

  glDisable(GL_CULL_FACE);
  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D,texId);

#if !defined(PLATFORM_PSVITA) && !defined(PLATFORM_WII)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#else
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
#endif

  if( hasAlpha ) {
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
  } else {
    glDisable(GL_BLEND);
  }
  glColor3f(1.0f,1.0f,1.0f);
  glMatrixMode( GL_PROJECTION );
  glLoadMatrixf(pMatrix);
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();

  glBegin(GL_QUADS);
#if !defined(PLATFORM_PSVITA) && !defined(PLATFORM_WII)
   glTexCoord2f(mx1,my1);glVertex2i(x1,y1);
   glTexCoord2f(mx2,my1);glVertex2i(x2,y1);
   glTexCoord2f(mx2,my2);glVertex2i(x2,y2);
   glTexCoord2f(mx1,my2);glVertex2i(x1,y2);
#else
   glTexCoord2f(mx1,my1);glVertex2f((float) x1,(float) y1);
   glTexCoord2f(mx2,my1);glVertex2f((float) x2,(float) y1);
   glTexCoord2f(mx2,my2);glVertex2f((float) x2,(float) y2);
   glTexCoord2f(mx1,my2);glVertex2f((float) x1,(float) y2);
#endif
  glEnd();

}
