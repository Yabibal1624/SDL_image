/*
    IMGLIB:  An example image loading library for use with SDL
    Copyright (C) 1999  Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Sam Lantinga
    5635-34 Springhouse Dr.
    Pleasanton, CA 94588 (USA)
    slouken@devolution.com
*/

/* A simple library to load images of various formats as SDL surfaces */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "SDL_image.h"

/* Table of image detection and loading functions */
static struct {
	char *type;
	int (*is)(SDL_RWops *src);
	SDL_Surface *(*load)(SDL_RWops *src);
} supported[] = {
        /* keep magicless formats first (denoted by is==NULL) */
	{ "TGA", NULL,      IMG_LoadTGA_RW },
	{ "BMP", IMG_isBMP, IMG_LoadBMP_RW },
	{ "PPM", IMG_isPPM, IMG_LoadPPM_RW },
	{ "PCX", IMG_isPCX, IMG_LoadPCX_RW },
	{ "GIF", IMG_isGIF, IMG_LoadGIF_RW },
	{ "JPG", IMG_isJPG, IMG_LoadJPG_RW },
	{ "TIF", IMG_isTIF, IMG_LoadTIF_RW },
	{ "PNG", IMG_isPNG, IMG_LoadPNG_RW },
};

/* Does the alpha value correspond to transparency or opacity?
   Default: transparency
*/
int IMG_invert_alpha = 0;

/* Load an image from a file */
SDL_Surface *IMG_Load(const char *file)
{
    SDL_RWops *src = SDL_RWFromFile(file, "rb");
    char *ext = strrchr(file, '.');
    if(ext)
	ext++;
    return IMG_LoadTyped_RW(src, 1, ext);
}

/* Load an image from an SDL datasource (for compatibility) */
SDL_Surface *IMG_Load_RW(SDL_RWops *src, int freesrc)
{
    return IMG_LoadTyped_RW(src, freesrc, NULL);
}

/* Portable case-insensitive string compare function */
static int string_equals(const char *str1, const char *str2)
{
	while ( *str1 && *str2 ) {
		if ( toupper((unsigned char)*str1) !=
		     toupper((unsigned char)*str2) )
			break;
		++str1;
		++str2;
	}
	return (!*str1 && !*str2);
}

/* Load an image from an SDL datasource, optionally specifying the type */
SDL_Surface *IMG_LoadTyped_RW(SDL_RWops *src, int freesrc, char *type)
{
	int i, start;
	SDL_Surface *image;

	/* Make sure there is something to do.. */
	if ( src == NULL ) {
		return(NULL);
	}

	/* See whether or not this data source can handle seeking */
	if ( SDL_RWseek(src, 0, SEEK_CUR) < 0 ) {
		IMG_SetError("Can't seek in this data source");
		return(NULL);
	}

	/* Detect the type of image being loaded */
	start = SDL_RWtell(src);
	image = NULL;
	for ( i=0; supported[i].type && !image; ++i ) {
	        if( (supported[i].is
		     && (SDL_RWseek(src, start, SEEK_SET),
			 supported[i].is(src)))
		    || (type && string_equals(type, supported[i].type))) {
#ifdef DEBUG_IMGLIB
			fprintf(stderr, "IMGLIB: Loading image as %s\n",
							supported[i].type);
#endif
			SDL_RWseek(src, start, SEEK_SET);
			image = supported[i].load(src);
			break;
		}
	}

	/* Clean up, check for errors, and return */
	if ( freesrc ) {
		SDL_RWclose(src);
	}
	if ( image == NULL ) {
		IMG_SetError("Unsupported image format");
	}
	return(image);
}

/* Invert the alpha of a surface for use with OpenGL
   If you want to use a surface loaded with this library as an OpenGL texture,
   set invart_alpha to 1.  If you want to use it with SDL alpha blit routines,
   set it to 0.
   This function returns the old alpha inversion value.

   Currently this is is only used by the PNG and TGA loaders.
 */
int IMG_InvertAlpha(int on)
{
	int old_alpha_value;

	old_alpha_value = IMG_invert_alpha;
	IMG_invert_alpha = on;
	return(old_alpha_value);
}

