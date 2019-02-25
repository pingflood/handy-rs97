
//
// Copyright (c) 2004 SDLemu Team
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from
// the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//

//////////////////////////////////////////////////////////////////////////////
//                       Handy/SDL - An Atari Lynx Emulator                 //
//                             Copyright (c) 2005                           //
//                                SDLemu Team                               //
//                                                                          //
//                          Based upon Handy v0.90 WIN32                    //
//                            Copyright (c) 1996,1997                       //
//                                  K. Wilkins                              //
//////////////////////////////////////////////////////////////////////////////
// handy_sdl_graphics.cpp                                                   //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This is the Handy/SDL graphics. It manages the graphics functions        //
// for emulating the Atari Lynx emulator using the SDL Library.             //
//                                                                          //
//    N. Wagenaar                                                           //
// December 2005                                                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
// Revision History:                                                        //
// -----------------                                                        //
//                                                                          //
// December 2005 :                                                          //
//  Since the 14th of April, the WIN32 of Handy (written by Keith Wilkins)  //
//  Handy has become OpenSource. Handy/SDL v0.82 R1 was based upon the old  //
//  v0.82 sources and was released closed source.                           //
//                                                                          //
//  Because of this event, the new Handy/SDL will be released as OpenSource //
//  but is rewritten from scratch because of lost sources (tm). The SDLemu  //
//  team has tried to bring Handy/SDL v0.1 with al the functions from the   //
//  closed source version.                                                  //
//////////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cctype>
#include <SDL/SDL.h>
#include <SDL/SDL_main.h>
#include <SDL/SDL_timer.h>
#ifdef GCWZERO
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#endif
#include "handy_sdl_main.h"
#include "handy_sdl_graphics.h"
#ifndef DINGUX
#include "sdlemu/sdlemu_opengl.h"
#include "sdlemu/sdlemu_overlay.h"
#include "sdlemu/sdlemu_video.h"
#endif
#include "sdlemu/sdlemu_filter.h"
#include "gui/gui.h"
/*
    This is called also from gui when initializing for rom browser
*/
int handy_sdl_video_early_setup(int surfacewidth, int surfaceheight, int sdl_bpp_flag, int videoflags)
{
    // Setup the main SDL surface
#ifdef DINGUX
    {
        uint32 vm = 0; // 0 - 320x240, 1 - 400x240, 2 - 480x272

#ifdef GCWZERO //GCWZERO add unscaled mode as 0 as we use the IPU for HW upscaling
        #define NUMOFVIDEOMODES 4
        struct { uint32 x; uint32 y; } VModes[NUMOFVIDEOMODES] = { {160, 102}, {320, 240}, {400, 240}, {480, 272} };
#else
        #define NUMOFVIDEOMODES 3
        struct { uint32 x; uint32 y; } VModes[NUMOFVIDEOMODES] = { {320, 240}, {400, 240}, {480, 272} };
#endif

        // check 3 videomodes: 480x272, 400x240, 320x240
        for(vm = NUMOFVIDEOMODES-1; vm >= 0; vm--) {
            if(SDL_VideoModeOK(VModes[vm].x, VModes[vm].y, 16, videoflags) != 0) {
                surfacewidth = VModes[vm].x;
                surfaceheight = VModes[vm].y;
                break;
            }
        }
    }

#endif

#ifdef GCWZERO //force unscaled as default is using the IPU to upscale
    // surfacewidth  = 160;
    // surfaceheight = 102;
    surfacewidth  = 320;
    surfaceheight = 240;
#endif
if (originalshow)
    mainSurface  = SDL_SetVideoMode(320, 240, sdl_bpp_flag, SDL_HWSURFACE|
    #ifdef SDL_TRIPLEBUF
        SDL_TRIPLEBUF
    #else
        SDL_DOUBLEBUF
    #endif
    );
else
    mainSurface  = SDL_SetVideoMode(surfacewidth, surfaceheight, sdl_bpp_flag, SDL_HWSURFACE|
    #ifdef SDL_TRIPLEBUF
        SDL_TRIPLEBUF
    #else
        SDL_DOUBLEBUF
    #endif
    );

    if (mainSurface == NULL)
    {
        printf("Could not create primary SDL surface: %s\n", SDL_GetError());
        return 0;
    }
    return 1;
}

/*
    Name                :     handy_sdl_video_setup
    Parameters          :     rendertype ( 1 = SDL, 2 = OpenGL, 3 = YUV )
                            fsaa ( 0 = off, 1 = on ) -> OpenGL specific
                            fullscreen ( 0 = off, 1 = on )
    Function            :   Initialisation of the video using the SDL libary.

    Uses                :   N/A

    Information            :    This is our setup function for getting our desired
                            video setup.
*/
int handy_sdl_video_setup(int rendertype, int fsaa, int fullscreen, int bpp, int scale, int accel, int sync)
{
    const    SDL_VideoInfo     *info;
            Uint32             videoflags;
            int              value;
            int                 sdl_bpp_flag;
            int                 surfacewidth;
            int                 surfaceheight;

    // Since we first checked the rotation, based upon that information
    // We setup the width and height of the display.
    //
    // If OpenGL rendering is selected, we choose 640x480 or 480x640 as
    // output. OpenGL rendering can be funky with non-standard resolutions
    // with buggy OpenGL drivers and/or videocards.
    //
    // This is really ugly but good enough for a first version :)
    switch(LynxRotate) {
        case MIKIE_NO_ROTATE:
            LynxWidth  = 160;
            LynxHeight = 102;
#ifndef DINGUX
            if ( rendertype != 2 )
            {
                surfacewidth  = LynxWidth * scale;
                surfaceheight = LynxHeight * scale;
            }
            else
            {
                surfacewidth  = 640;
                surfaceheight = 480;
            }
#endif
            break;
        case MIKIE_ROTATE_L:
        case MIKIE_ROTATE_R:
            LynxWidth  = 102;
            LynxHeight = 160;
#ifndef DINGUX
            if ( rendertype != 2 )
            {
                surfacewidth  = LynxWidth * scale;
                surfaceheight = LynxHeight * scale;
            }
            else
            {
                surfacewidth  = 480;
                surfaceheight = 640;
            }
#endif
            break;
    }

    info = SDL_GetVideoInfo();

    // Let us check if SDL could get information about the videodriver.
    if (!info)
    {
        printf("ERROR: SDL is unable to get the video info: %s\n", SDL_GetError());
        return false;
    }

    if( bpp != 0 )
    {
            sdl_bpp_flag = bpp;
    }
    else
    {
            switch(info->vfmt->BitsPerPixel)
            {
                case 8:
                    sdl_bpp_flag = 8;
                    break;
                case 16:
                    sdl_bpp_flag = 16;
                    break;
                case 24:
                    sdl_bpp_flag = 24;
                    break;
                case 32:
                    sdl_bpp_flag = 32;
                    break;
                default:
                    sdl_bpp_flag = 8;  // Default : 8bpp
                    break;
            }
    }
    mpBpp = sdl_bpp_flag;

    printf("\nSDL Rendering : ");
#ifdef DINGUX
    videoflags = handy_sdl_video_setup_sdl(info);
#else
    switch(rendertype)
    {
        case 1:
            videoflags = handy_sdl_video_setup_sdl(info);
            break;
        case 2:
            videoflags = handy_sdl_video_setup_opengl(fsaa, accel, sync);
            break;
        case 3:
            videoflags = handy_sdl_video_setup_yuv();
            break;
        default:
            videoflags = handy_sdl_video_setup_sdl(info);
            break;
    }
#endif

    printf("SDL Rendering : %dBPP\n", sdl_bpp_flag);

/*
    Quick fix for handling fullscreen on WIN32. Fullscreen seems to break when using
    hardware surfaces in normal SDL rendering or when using a frame with OpenGL
    rendering. It just goes back in windowed mode or displays it in the window res.
*/
#ifdef DINGUX
    // ignore fullscreen for dingux
#elif defined(HANDY_SDL_WIN32)
    if (fullscreen)
    {
        if ( rendertype != 2 )
            videoflags  = SDL_SWSURFACE | SDL_FULLSCREEN;
        else
            videoflags  = SDL_OPENGL | SDL_NOFRAME | SDL_FULLSCREEN;
    }
#else
    if (fullscreen)
        videoflags |= SDL_FULLSCREEN;
#endif

    // setup SDL video mode
    handy_sdl_video_early_setup(surfacewidth, surfaceheight, sdl_bpp_flag, videoflags);

    // Setup the Handy Graphics Buffer.
    //
    // All the rendering is done in the graphics buffer and is then
    // blitted to the mainSurface and thus to the screen.

	HandyBuffer = SDL_CreateRGBSurface(SDL_SWSURFACE,
        LynxWidth,
        LynxHeight,
        sdl_bpp_flag,
        0x00000000, 0x00000000, 0x00000000, 0x00000000);

    if (HandyBuffer == NULL)
    {
        printf("Could not create secondary SDL surface: %s\n", SDL_GetError());
        return 0;
    }

#ifndef DINGUX
    if (rendertype == 2 )
    // Let us setup OpenGL and our rendering texture. We give the src (HandyBuffer) and the
    // dst (mainSurface) display as well as the automatic bpp selection as options so that
    // our texture is automaticly created :)
    {
    sdlemu_init_opengl(HandyBuffer, mainSurface, 1 /*method*/,
            fsaa /*texture type (linear, nearest)*/,
            0 /* Automatic bpp selection based upon src */);
    }

    if (rendertype == 3 )
    {
            sdlemu_init_overlay(mainSurface, overlay_format, LynxWidth , LynxHeight );
    }
#endif

    /* Setting Window Caption */
    SDL_WM_SetCaption( "Handy/SDL", "HANDY");
    SDL_EnableKeyRepeat( 0, 0); // Best options to use
    SDL_EventState( SDL_MOUSEMOTION, SDL_IGNORE); // Ignoring mouse stuff.
    SDL_ShowCursor( 0 ); // Removing mouse from window. Very handy in fullscreen mode :)

    delta = (uint8*)malloc(LynxWidth*LynxHeight*sizeof(Uint32)*4);
    memset(delta, 255, LynxWidth*LynxHeight*sizeof(Uint32)*4);

    Init_2xSaI (565);
    systemRedShift   = sdlCalculateShift(HandyBuffer->format->Rmask);
    systemGreenShift = sdlCalculateShift(HandyBuffer->format->Gmask);
    systemBlueShift  = sdlCalculateShift(HandyBuffer->format->Bmask);


    return 1;
}

/*
    Name                :     handy_sdl_video_setup_opengl
    Parameters          :     fsaa ( 0 = off, 1 = on ) -> OpenGL specific
    Function            :   Initialisation of OpenGL videodriver.

    Uses                :   N/A

    Information            :    This is our setup function for getting our desired
                            OpenGL video setup.
*/
#ifndef DINGUX
int handy_sdl_video_setup_opengl(int fsaa,int accel, int sync)
{
    Uint32             videoflags;

    printf("OpenGL\n");
    // Initializing SDL attributes with OpenGL
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE  , 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        printf("OpenGL OPTION : Enable  SDL_GL_DOUBLEBUFFER\n");

    // Setup FSAA
    if ( fsaa )
    {
        SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1 );
        SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, fsaa );
        printf("OpenGL FSAA   : Enable  SDL_GL_MULTISAMPLEBUFFERS\n");
    }
    else
    {
        printf("OpenGL FSAA   : Disable SDL_GL_MULTISAMPLEBUFFERS\n");
//        SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 0 );
        accel = 0;
    }

    if ( accel )
    {
        printf("OpenGL ACCEL  : Enable  SDL_GL_ACCELERATED_VISUAL\n");
        SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 );
    }
    else
    {
        printf("OpenGL ACCEL  : Disable SDL_GL_ACCELERATED_VISUAL\n");
//        SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 0 );
    }

    if ( sync )
    {
        printf("OpenGL VSYNC  : Enable  SDL_GL_SWAP_CONTROL\n");
        SDL_GL_SetAttribute( SDL_GL_SWAP_CONTROL, 1 );
    }
    else
    {
        printf("OpenGL VSYNC  : Disable SDL_GL_SWAP_CONTROL\n");
        SDL_GL_SetAttribute( SDL_GL_SWAP_CONTROL, 0 );
    }

    videoflags = SDL_OPENGL;

    return videoflags;

}
#endif

/*
    Name                :     handy_sdl_video_setup_yuv
    Parameters          :     N/A
    Function            :   Initialisation of YUV videodriver.

    Uses                :   N/A

    Information            :    This is our setup function for getting our desired
                            YUV video setup.
*/
#ifndef DINGUX
int handy_sdl_video_setup_yuv(void)
{
    Uint32             videoflags;

    printf("YUV Overlay\n");

    videoflags = SDL_SWSURFACE;
    
    return videoflags;
}
#endif

/*
    Name                :     handy_sdl_video_setup_sdl
    Parameters          :     info (SDL videoinfo)
    Function            :   Initialisation of SDL videodriver.

    Uses                :   N/A

    Information            :    This is our setup function for getting our desired
                            SDL video setup. Using info it automaticly checks
                            if we can use hardware acceleration or going back
                            to software display.
*/
int handy_sdl_video_setup_sdl(const SDL_VideoInfo *info)
{
    Uint32             videoflags;
#ifdef DINGUX

#ifdef GCWZERO
    videoflags = SDL_HWSURFACE | SDL_TRIPLEBUF;
#else
    videoflags = SDL_SWSURFACE;
#endif

#else
    if (info->hw_available)
    {
        printf("SDL Hardware\n");
        videoflags = SDL_HWSURFACE | SDL_HWPALETTE | SDL_TRIPLEBUF;

        if (info->blit_hw) videoflags |= SDL_HWACCEL;
    }
    else
    {
            printf("SDL Software\n");
            videoflags = SDL_SWSURFACE;
    }
#endif
    return videoflags;
}

/*
    Name                :     handy_sdl_video_init
    Parameters          :     N/A
    Function            :   Initialisation of Handy/SDL graphics.

    Uses                :   mpLynxBuffer ( Handy core rendering buffer )
                            mpLynx Class ( Handy core )
                            HandyBuffer  ( Handy/SDL display buffer )

    Information            :    Creates the backbuffer for the Handy core based
                            upon rotation, format, etc.
*/
UBYTE *handy_sdl_display_fake_callback(ULONG objref);
void handy_sdl_video_init(int bpp)
{

    printf("Initialising Handy Display... ");
    switch(bpp)
    {
        case 8:
            LynxFormat = MIKIE_PIXEL_FORMAT_8BPP;
            break;
        case 15:
            LynxFormat = MIKIE_PIXEL_FORMAT_16BPP_555;
            break;
        case 16:
            LynxFormat = MIKIE_PIXEL_FORMAT_16BPP_565;
            break;
        case 24:
            LynxFormat = MIKIE_PIXEL_FORMAT_24BPP;
            break;
        case 32:
            LynxFormat = MIKIE_PIXEL_FORMAT_32BPP;
            break;
        default:
            LynxFormat = MIKIE_PIXEL_FORMAT_16BPP_565; // Default 16BPP bpp
            break;
    }

    // HandyBuffer must be initialized previously
    mpLynxBuffer = (Uint32 *)HandyBuffer->pixels;
    mpLynx->DisplaySetAttributes( LynxRotate, LynxFormat, (ULONG)HandyBuffer->pitch, handy_sdl_display_callback, (ULONG)mpLynxBuffer);

    printf("[DONE]\n");
}

/*
    Name                :     handy_sdl_display_callback
    Parameters          :     N/A
    Function            :   Handy/SDL display rendering function.

    Uses                :   HandyBuffer  ( Handy/SDL display buffer )
                            mainSurface     ( Handy/SDL primary display )

    Information            :    Renders the graphics from HandyBuffer to
                            the main surface.
*/
UBYTE *handy_sdl_display_fake_callback(ULONG objref)
{
    return (UBYTE *)mpLynxBuffer;
}

UBYTE *handy_sdl_display_callback(ULONG objref)
{
    int filter =  1;


    // Time to render the contents of mLynxBuffer to the SDL gfxBuffer.
    // Now to blit the contents of gfxBuffer to our main SDL surface.
#ifdef DINGUX
    handy_sdl_draw_graphics();
    
    // show fps if needed
    gui_ShowFPS();
#ifdef GCWZERO 
    /*With HWSURFACE and DOUBLEBUF enabled, framerate drops to around 50fps  /
    / so we need to skip alternate flips to maintain 60fps                  */
    if(!everyotherframe) 
	SDL_Flip( mainSurface );
    else { //<60fps so alternate flipping required
        static int flip=0;
	if (flip) {
	    SDL_Flip( mainSurface );
		flip++;
		if (flip=2) flip=0;
	} else flip++;
    }
#else
    SDL_Flip( mainSurface );
#endif
#else
    switch( rendertype )
    {
        case 1:
            handy_sdl_draw_graphics();
            SDL_Flip( mainSurface );
            break;
        case 2:
            sdlemu_draw_texture( HandyBuffer, mainSurface, 1/*1=GL_QUADS*/);
            break;
        case 3:
            sdlemu_draw_overlay( HandyBuffer, LynxScale, LynxWidth, LynxHeight);
        default:
            handy_sdl_draw_graphics();
            break;
    }
#endif

    return (UBYTE *)mpLynxBuffer;
}

inline void handy_sdl_draw_filter(int filtertype, SDL_Surface *src, SDL_Surface *dst, Uint8 *delta)
{
#ifdef DINGUX
    Uint8 *dst_offset = (Uint8 *)dst->pixels + (dst->w - 320) + (dst->h - 204) * dst->w;

    switch( filter ) {
        case 0: break;
        case 1: TVMode((Uint8 *)src->pixels, src->pitch, delta, dst_offset, dst->pitch, src->w, src->h); break;
        case 2: _2xSaI((Uint8 *)src->pixels, src->pitch, delta, dst_offset, dst->pitch, src->w, src->h); break;
        case 3: Super2xSaI((Uint8 *)src->pixels, src->pitch, delta, dst_offset, dst->pitch, src->w, src->h); break;
        case 4: SuperEagle((Uint8 *)src->pixels, src->pitch, delta, dst_offset, dst->pitch, src->w, src->h); break;
        case 5: MotionBlur((Uint8 *)src->pixels, src->pitch, delta, dst_offset, dst->pitch, src->w, src->h); break;
        case 6: Simple2x((Uint8 *)src->pixels, src->pitch, delta, dst_offset, dst->pitch, src->w, src->h); break;
        case 7: bilinear((Uint8 *)src->pixels, src->pitch, delta, dst_offset, dst->pitch, src->w, src->h); break;
        case 8: bilinearPlus((Uint8 *)src->pixels, src->pitch, delta, dst_offset, dst->pitch, src->w, src->h); break;
        case 9: Pixelate((Uint8 *)src->pixels, src->pitch, delta, dst_offset, dst->pitch, src->w, src->h); break;
        case 10: Average((Uint8 *)src->pixels, src->pitch, delta, dst_offset, dst->pitch, src->w, src->h); break;
    }
#else
    switch( filter ) {
        case 0: break;
        case 1: TVMode((Uint8 *)src->pixels, src->pitch, delta, (Uint8 *)dst->pixels, dst->pitch, src->w, src->h); break;
        case 2: _2xSaI((Uint8 *)src->pixels, src->pitch, delta, (Uint8 *)dst->pixels, dst->pitch, src->w, src->h); break;
        case 3: Super2xSaI((Uint8 *)src->pixels, src->pitch, delta, (Uint8 *)dst->pixels, dst->pitch, src->w, src->h); break;
        case 4: SuperEagle((Uint8 *)src->pixels, src->pitch, delta, (Uint8 *)dst->pixels, dst->pitch, src->w, src->h); break;
        case 5: MotionBlur((Uint8 *)src->pixels, src->pitch, delta, (Uint8 *)dst->pixels, dst->pitch, src->w, src->h); break;
        case 6: Simple2x((Uint8 *)src->pixels, src->pitch, delta, (Uint8 *)dst->pixels, dst->pitch, src->w, src->h); break;
        case 7: bilinear((Uint8 *)src->pixels, src->pitch, delta, (Uint8 *)dst->pixels, dst->pitch, src->w, src->h); break;
        case 8: bilinearPlus((Uint8 *)src->pixels, src->pitch, delta, (Uint8 *)dst->pixels, dst->pitch, src->w, src->h); break;
        case 9: Pixelate((Uint8 *)src->pixels, src->pitch, delta, (Uint8 *)dst->pixels, dst->pitch, src->w, src->h); break;
        case 10: Average((Uint8 *)src->pixels, src->pitch, delta, (Uint8 *)dst->pixels, dst->pitch, src->w, src->h); break;
    }
#endif
}

#if 0
/* Bresenham's upscale routine - SLOW!!! */
void UpscaleBresenham(Uint16 *src, 
                      Uint32 src_pitch,
                      Uint32 src_w, 
                      Uint32 src_h, 
                      Uint16 *dst, 
                      Uint32 dst_pitch, 
                      Uint32 dst_w, 
                      Uint32 dst_h)
{
    int midw = dst_w / 2 * 3 / 2;
    int midh = dst_h / 2 * 3 / 2;
    int Ew = 0;
    int Eh = 0;
    int source = 0, target = 0;
    int dh = 0;
    int i, j;
    
    for(i = 0; i < dst_h; i++) {
        Ew = 0;
        source = dh * src_w;    

        for(j = 0; j < dst_w; j++) {
            Uint32 c;
            
            c = src[source];
            
            #define AVERAGE(z, x) ((((z) & 0xF7DEF7DE) >> 1) + (((x) & 0xF7DEF7DE) >> 1))
            
            if((Ew >= midw) && (Eh >= midh)) {
                c = AVERAGE(src[source+1], src[source+src_w]);
            } else {
                if(Ew >= midw) { // average + 1
                    c = AVERAGE(c, src[source+1]);
                }
                if(Eh >= midh) { // average + src_w
                    c = AVERAGE(c, src[source+src_w]);
                }
            }
                
            dst[target++] = c;
            
            Ew += src_w; if(Ew >= dst_w) { Ew -= dst_w; source += 1; }
        }
        
        Eh += src_h; if(Eh >= dst_h) { Eh -= dst_h; dh++; }
    }
}
#else

#define AVERAGE(z, x) ((((z) & 0xF7DEF7DE) >> 1) + (((x) & 0xF7DEF7DE) >> 1))
/*
    Upscale 160x102 -> 320x240
    Horizontal upscale:
        320/160=2  --  simple doubling of pixels
        [ab][cd] -> [aa][bb][cc][dd]
    Vertical upscale:
        Bresenham algo with simple interpolation
*/
void upscale_320x240(Uint32 *src, Uint32 *dst)
{
    int midh = 240 / 2 * 3 / 2;
    int Eh = 0;
    int source = 0;
    int dh = 0;
    int i, j;

    for (i = 0; i < 240; i++)
    {
        source = dh * 160 / 2; // atari lynx x / 2 

        for (j = 0; j < 320/8; j++)
        {
            Uint32 a, b, c, d, ab, cd;

            __builtin_prefetch(dst + 4, 1);
            __builtin_prefetch(src + source + 4, 0);

            ab = src[source] & 0xF7DEF7DE;
            cd = src[source + 1] & 0xF7DEF7DE;

            if(Eh >= midh) { // average + 160
                ab = AVERAGE(ab, src[source+160/2]);
                cd = AVERAGE(cd, src[source+160/2+1]);
            }

            a = (ab & 0xFFFF) | (ab << 16);
            b = (ab & 0xFFFF0000) | (ab >> 16);
            c = (cd & 0xFFFF) | (cd << 16);
            d = (cd & 0xFFFF0000) | (cd >> 16);

            *dst++ = a;
            *dst++ = b;
            *dst++ = c;
            *dst++ = d;

            source += 2;

        }
        Eh += 102; if(Eh >= 240) { Eh -= 240; dh++; } // 102 - real atari lynx y size
    }
}

/*
    Upscale 160x102 -> 400x240
    Horizontal upscale:
        400/160=2.5  --  do some horizontal interpolation
        [ab][cd] -> [aa][(ab)b][bc][c(cd)][dd]
    Vertical upscale:
        Bresenham algo with simple interpolation
*/
void upscale_400x240(Uint32 *src, Uint32 *dst)
{
    int midh = 240 / 2 * 3 / 2;
    int Eh = 0;
    int source = 0;
    int dh = 0;
    int i, j;

    for (i = 0; i < 240; i++)
    {
        source = dh * 160 / 2; // atari lynx x / 2 

        for (j = 0; j < 400/10; j++)
        {
            Uint32 a, b, c, d, e, ab, cd;

            __builtin_prefetch(dst + 4, 1);
            __builtin_prefetch(src + source + 4, 0);

            ab = src[source] & 0xF7DEF7DE;
            cd = src[source + 1] & 0xF7DEF7DE;

            if(Eh >= midh) { // average + 160
                ab = AVERAGE(ab, src[source+160/2]) & 0xF7DEF7DE; // to prevent overflow
                cd = AVERAGE(cd, src[source+160/2+1]) & 0xF7DEF7DE; // to prevent overflow
            }

            a = (ab & 0xFFFF) | (ab << 16);
            b = (((ab & 0xFFFF) >> 1) + ((ab & 0xFFFF0000) >> 17)) | (ab & 0xFFFF0000);
            c = (ab >> 16) | (cd << 16);
            d = (cd & 0xFFFF) | (((cd & 0xFFFF) << 15) + ((cd & 0xFFFF0000) >> 1));
            e = (cd >> 16) | (cd & 0xFFFF0000);

            *dst++ = a;
            *dst++ = b;
            *dst++ = c;
            *dst++ = d;
            *dst++ = e;

            source += 2;

        }
        Eh += 102; if(Eh >= 240) { Eh -= 240; dh++; } // 102 - real atari lynx y size
    }
}

/*
    Upscale 160x102 -> 480x272
    Horizontal upscale
        480/160=3  --  simple tripling of pixels
        [ab][cd] -> [aa][ab][bb][cc][cd][dd]
    Vertical upscale:
        Bresenham algo with simple interpolation
*/
void upscale_480x272(Uint32 *src, Uint32 *dst)
{
    int midh = 272 / 2 * 3 / 2;
    int Eh = 0;
    int source = 0;
    int dh = 0;
    int i, j;

    for (i = 0; i < 272; i++)
    {
        source = dh * 160 / 2; // atari lynx x / 2 

        for (j = 0; j < 480/12; j++)
        {
            Uint32 a, b, c, d, e, f, ab, cd;

            __builtin_prefetch(dst + 4, 1);
            __builtin_prefetch(src + source + 4, 0);

            ab = src[source] & 0xF7DEF7DE;
            cd = src[source + 1] & 0xF7DEF7DE;

            if(Eh >= midh) { // average + 160
                ab = AVERAGE(ab, src[source+160/2]);
                cd = AVERAGE(cd, src[source+160/2+1]);
            }

            a = (ab & 0xFFFF) | (ab << 16);
            b = ab;
            c = (ab & 0xFFFF0000) | (ab >> 16);
            d = (cd & 0xFFFF) | (cd << 16);
            e = cd;
            f = (cd & 0xFFFF0000) | (cd >> 16);

            *dst++ = a;
            *dst++ = b;
            *dst++ = c;
            *dst++ = d;
            *dst++ = e;
            *dst++ = f;

            source += 2;

        }
        Eh += 102; if(Eh >= 272) { Eh -= 272; dh++; } // 102 - real atari lynx y size
    }
}

#endif
inline void handy_sdl_draw_graphics(void)
{

    if( filter >= 1 )
    {
        if(SDL_MUSTLOCK(mainSurface)) SDL_LockSurface(mainSurface);
        handy_sdl_draw_filter(filter, HandyBuffer, mainSurface, delta);
        if(SDL_MUSTLOCK(mainSurface)) SDL_UnlockSurface(mainSurface);
    }
    else
    {

        if (LynxScale == 1)
        {
            if(SDL_MUSTLOCK(mainSurface)) SDL_LockSurface(mainSurface);
            switch(mainSurface->w) {
                case 400:
                    upscale_400x240((Uint32 *)HandyBuffer->pixels, (Uint32 *)mainSurface->pixels);
                    break;
                case 480:
                    upscale_480x272((Uint32 *)HandyBuffer->pixels, (Uint32 *)mainSurface->pixels);
                    break;
                default:
                    upscale_320x240((Uint32 *)HandyBuffer->pixels, (Uint32 *)mainSurface->pixels);
                    break;
            }

            if(SDL_MUSTLOCK(mainSurface)) SDL_UnlockSurface(mainSurface);
        } else {



//     #ifdef GCWZERO //Use IPU to upscale, just blit default lynx resolution
//need to free the surfaces.
     static int overlaydrawn;
     static int overlaydrawn2;
     static SDL_Surface *overlay;
     static SDL_Surface *overlay2;
     static int backgrounddrawn1;
     static int backgrounddrawn2;
     if (1 ||originalshow) {
         static SDL_Rect background_rect;
         background_rect.x = 0;
         background_rect.y = 0;
         background_rect.h = 240;
         background_rect.w = 320;
         if (lynxversion == 2) {
             static SDL_Surface *background2;
             static SDL_Surface *unoptimisedbackground2;
             if (!backgrounddrawn2) {
                 unoptimisedbackground2 = IMG_Load("./background_lynx2.png");
                 background2 = SDL_DisplayFormat(unoptimisedbackground2);
                 SDL_FreeSurface(unoptimisedbackground2);
                 backgrounddrawn2++;
             }
             if(redrawbackground) {
                 SDL_BlitSurface (background2, NULL, mainSurface, &background_rect);
                 redrawbackground--;
             }
         } else {
             static SDL_Surface *background;
             static SDL_Surface *unoptimisedbackground;
             if (!backgrounddrawn1) {
                 unoptimisedbackground = IMG_Load("./background_lynx.png");
                 background = SDL_DisplayFormat(unoptimisedbackground);
                 SDL_FreeSurface(unoptimisedbackground);
                 backgrounddrawn1++;
             }
             if(redrawbackground) {
                 SDL_BlitSurface (background, NULL, mainSurface, &background_rect);
                 redrawbackground--;
             }
         }

     } 
     // SDL_Rect centrerect;
     // centrerect.x = 320/4;
     // centrerect.y = 240/4 + 9;
     // centrerect.w = 160;
     // centrerect.h = 102;
     // SDL_BlitSurface(HandyBuffer, NULL, mainSurface, &centrerect);



            // char *scp=(char *) HandyBuffer->pixels;
            // char *dcp=(char *) mainSurface->pixels;
            // for (int y=0; y< LynxHeight; y++)
            // {
            //     for (int x=0; x<LynxWidth; x++)
            //     {
            //         *dcp++=*scp++;
            //     }
            // }



        // switch(mpLynx->CartGetRotate()) {
        //     case MIKIE_ROTATE_L:
        //     case MIKIE_ROTATE_R:
        //         LynxWidth  = 102;
        //         LynxHeight = 160;
        //     default:
        //         LynxWidth  = 160;
        //         LynxHeight = 102;
        //     }

                LynxWidth  = 102;
                LynxHeight = 160;


           uint16_t *d = (uint16_t*)mainSurface->pixels + (mainSurface->w - LynxWidth) / 2 + (mainSurface->h - LynxHeight) * mainSurface->pitch / 4 ;
            uint16_t *s = (uint16_t*)HandyBuffer->pixels;
            for (int y = 0; y < LynxHeight; y++)
            {
                for (int x=0; x<LynxWidth; x++)
                {
                    *d++ = s[x * LynxWidth + y];
                }
                d += mainSurface->w - LynxWidth;
            }






           // uint16_t *d = (uint16_t*)mainSurface->pixels + (mainSurface->w - HandyBuffer->w) / 2 + (mainSurface->h - HandyBuffer->h) * mainSurface->pitch / 4 ;
           //  uint16_t *s = (uint16_t*)HandyBuffer->pixels;
           //  for (int y = 0; y < HandyBuffer->h; y++)
           //  {
           //      for (int x=0; x<HandyBuffer->w; x++)
           //      {
           //          *d++ = *s++;
           //      }

           //      // s += HandyBuffer->w;
           //      d += mainSurface->w - HandyBuffer->w;
           //  }





            // uint16_t *d = (uint16_t*)mainSurface->pixels + (mainSurface->w - HandyBuffer->w) / 2 + (mainSurface->h - HandyBuffer->h) * mainSurface->pitch / 4 ;
            // uint16_t *s = (uint16_t*)HandyBuffer->pixels;
            // for (int y = 0; y < HandyBuffer->h; y++)
            // {
            //     memmove(d, s, HandyBuffer->w * sizeof(uint16_t));
            //     s += HandyBuffer->w;
            //     d += mainSurface->w;
            // }








     // else {
     //        redrawbackground = 5;
     //        SDL_BlitSurface(HandyBuffer, NULL, mainSurface, NULL);
     // }

        }
    }
}

#ifndef DINGUX
inline void handy_sdl_scale(void)
{
    Uint8             bpp;
    char *dcp, *scp, *olddcp;
    int   x, y, *sip, *dip, *olddip, i;
    long *dlp, *slp, *olddlp;
    int increment, copysize;

    bpp = mainSurface->format->BytesPerPixel;

    // SLOW !!!
    if (SDL_MUSTLOCK(mainSurface)) SDL_LockSurface(mainSurface);

    increment = LynxScale*(LynxScale-1)*LynxWidth;
    copysize = increment*bpp;

    switch (bpp)
    {
        case 1:
            scp=(char *) HandyBuffer->pixels;
            dcp=(char *) mainSurface->pixels;
            for (y=0; y< LynxHeight; y++)
            {
                olddcp=dcp;
                for (x=0; x<LynxWidth; x++)
                {
                    for (i=0; i<LynxScale; i++)
                    {
                        *dcp=*scp;
                        dcp++;
                    }
                        scp++;
                }
                if (!LynxLCD)
                    memcpy (dcp, olddcp, copysize);

                dcp+=increment;
            }
            break;
        case 2:
            sip=(int *) HandyBuffer->pixels;
            dip=(int *) mainSurface->pixels;
            for (y=0; y<= LynxHeight; y++)
            {
                olddip=dip;
                for (x=0; x<=LynxWidth; x++)
                {
                    for (i=0; i<LynxScale; i++)
                    {
                        *dip=*sip;
                        dip++;
                    }
                    sip++;
                }
                if (!LynxLCD)
                    memcpy (dip, olddip, copysize);

                dip+=increment;
            }
            break;
        case 3:
            // REALLY SLOW
            scp=(char *) HandyBuffer->pixels;
            dcp=(char *) mainSurface->pixels;
            for (y=0; y< LynxHeight; y++)
            {
                olddcp=dcp;
                for (x=0; x<LynxWidth; x++)
                {
                    for (i=0; i<LynxScale; i++)
                    {
                        *dcp=*scp; dcp++;
                        *dcp=*scp; dcp++;
                        *dcp=*scp; dcp++;
                    }
                    scp+=3;
                }
                if (!LynxLCD)
                    memcpy (dcp, olddcp, copysize);

                dcp+=3*increment;
            }
            break;
        case 4:
        default:
            slp=(long *) HandyBuffer->pixels;
            dlp=(long *) mainSurface->pixels;
            for (y=0; y< LynxHeight; y++)
            {
                olddlp=dlp;
                for (x=0; x<LynxWidth; x++)
                {
                    for (i=0; i<LynxScale; i++)
                    {
                        *dlp=*slp;
                        dlp++;
                    }
                    slp++;
                }
                if (!LynxLCD)
                    memcpy (dlp, olddlp, copysize);

                dlp+=increment;
            }
            break;
    }

    if(SDL_MUSTLOCK(mainSurface)) SDL_UnlockSurface (mainSurface);

}
#endif
/*
    Name                :     handy_sdl_video_close
    Parameters          :     N/A
    Function            :   Handy/SDL Video destroy function.

    Uses                :   mpLynxBuffer ( Handy core rendering buffer )
                            HandyBuffer  ( Handy/SDL buffer display )

    Information            :    Renders the graphics from HandyBuffer to
                            the main surface.
*/
#ifndef DINGUX
void    handy_sdl_video_close(void)
{
    sdlemu_close_overlay();
}
#endif
#ifdef GCWZERO
void gcw_display_bios_warning(void)
{
    mainSurface  = SDL_SetVideoMode(320, 240, 16, SDL_HWSURFACE|
    #ifdef SDL_TRIPLEBUF
        SDL_TRIPLEBUF
    #else
        SDL_DOUBLEBUF
    #endif
    );
    TTF_Init();
    TTF_Font *ttffont = NULL;
    SDL_Color text_color = {255, 0, 0};
    ttffont = TTF_OpenFont("./ProggyTiny.ttf", 16);
    SDL_Surface *textSurface;
    textSurface = TTF_RenderText_Solid(ttffont, "No bios found!", text_color);
    SDL_Rect destination;
    destination.x = 30;
    destination.y = 10;
    destination.w = 290; 
    destination.h = 230;
    SDL_BlitSurface(textSurface, NULL, mainSurface, &destination);
    SDL_FreeSurface(textSurface);
    textSurface = TTF_RenderText_Solid(ttffont, "Place lynxboot.img in $HOME/.handy", text_color);
    destination.y = 40;
    SDL_BlitSurface(textSurface, NULL, mainSurface, &destination);
    SDL_FreeSurface(textSurface);
    TTF_CloseFont (ttffont);
    SDL_Flip(mainSurface);
    SDL_Delay(4000);
    SDL_FreeSurface(mainSurface);
    exit(0);
}
#endif
