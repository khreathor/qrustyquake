// Copyright (C) 1996-2001 Id Software, Inc.
// Copyright (C) 2002-2009 John Fitzgibbons and others
// Copyright (C) 2010-2014 QuakeSpasm developers
// GPLv3 See LICENSE for details.
#include "quakedef.h"

static s8 loadfilename[MAX_OSPATH];

static stdio_buffer_t *Buf_Alloc(FILE *f)
{
        stdio_buffer_t *buf = (stdio_buffer_t *) calloc(1, sizeof(stdio_buffer_t));
        buf->f = f;
        return buf;
}

static void Buf_Free(stdio_buffer_t *buf)
{
        free(buf);
}

static inline s32 Buf_GetC(stdio_buffer_t *buf)
{
        if (buf->pos >= buf->size)
        {
                buf->size = fread(buf->buffer, 1, sizeof(buf->buffer), buf->f);
                buf->pos = 0;

                if (buf->size == 0)
                        return EOF;
        }

        return buf->buffer[buf->pos++];
}

s32 fgetLittleShort (FILE *f)
{
        u8    b1, b2;

        b1 = fgetc(f);
        b2 = fgetc(f);

        return (s16)(b1 + b2*256);
}

u8 *Image_LoadTGA (FILE *fin, s32 *width, s32 *height)
{
	s32 columns, rows, numPixels;
	u8 *pixbuf;
	s32 row, column;
	u8 *targa_rgba;
	s32 realrow; //johnfitz -- fix for upside-down targas
	bool upside_down; //johnfitz -- fix for upside-down targas
	stdio_buffer_t *buf;
	targaheader_t targa_header;

	targa_header.id_length = fgetc(fin);
	targa_header.colormap_type = fgetc(fin);
	targa_header.image_type = fgetc(fin);

	targa_header.colormap_index = fgetLittleShort(fin);
	targa_header.colormap_length = fgetLittleShort(fin);
	targa_header.colormap_size = fgetc(fin);
	targa_header.x_origin = fgetLittleShort(fin);
	targa_header.y_origin = fgetLittleShort(fin);
	targa_header.width = fgetLittleShort(fin);
	targa_header.height = fgetLittleShort(fin);
	targa_header.pixel_size = fgetc(fin);
	targa_header.attributes = fgetc(fin);

	if (targa_header.image_type==1)
	{
		if (targa_header.pixel_size != 8 || targa_header.colormap_size != 24 || targa_header.colormap_length > 256)
			Sys_Error ("Image_LoadTGA: %s has an %ibit palette", loadfilename, targa_header.colormap_type);
	}
	else
	{
		if (targa_header.image_type!=2 && targa_header.image_type!=10)
			Sys_Error ("Image_LoadTGA: %s is not a type 2 or type 10 targa (%i)", loadfilename, targa_header.image_type);

		if (targa_header.colormap_type !=0 || (targa_header.pixel_size!=32 && targa_header.pixel_size!=24))
			Sys_Error ("Image_LoadTGA: %s is not a 24bit or 32bit targa", loadfilename);
	}

	columns = targa_header.width;
	rows = targa_header.height;
	numPixels = columns * rows;
	upside_down = !(targa_header.attributes & 0x20); //johnfitz -- fix for upside-down targas

	targa_rgba = (u8 *) Hunk_Alloc (numPixels*4);

	if (targa_header.id_length != 0)
		fseek(fin, targa_header.id_length, SEEK_CUR); // skip TARGA image comment

	buf = Buf_Alloc(fin);

	if (targa_header.image_type==1) // Uncompressed, paletted images
	{
		u8 palette[256*4];
		u64 i;
		//palette data comes first
		for (i = 0; i < targa_header.colormap_length; i++)
		{ //this palette data is bgr.
			palette[i*3+2] = Buf_GetC(buf);
			palette[i*3+1] = Buf_GetC(buf);
			palette[i*3+0] = Buf_GetC(buf);
			palette[i*3+3] = 255;
		}
		for (i = targa_header.colormap_length*4; i < sizeof(palette); i++)
			palette[i] = 0;
		for(row=rows-1; row>=0; row--)
		{
			realrow = upside_down ? row : rows - 1 - row;
			pixbuf = targa_rgba + realrow*columns*4;

			for(column=0; column<columns; column++)
			{
				i = Buf_GetC(buf);
				*pixbuf++= palette[i*3+0];
				*pixbuf++= palette[i*3+1];
				*pixbuf++= palette[i*3+2];
				*pixbuf++= palette[i*3+3];
			}
		}
	}
	else if (targa_header.image_type==2) // Uncompressed, RGB images
	{
		for(row=rows-1; row>=0; row--)
		{
			//johnfitz -- fix for upside-down targas
			realrow = upside_down ? row : rows - 1 - row;
			pixbuf = targa_rgba + realrow*columns*4;
			//johnfitz
			for(column=0; column<columns; column++)
			{
				u8 red,green,blue,alphabyte;
				switch (targa_header.pixel_size)
				{
					case 24:
						blue = Buf_GetC(buf);
						green = Buf_GetC(buf);
						red = Buf_GetC(buf);
						*pixbuf++ = red;
						*pixbuf++ = green;
						*pixbuf++ = blue;
						*pixbuf++ = 255;
						break;
					case 32:
						blue = Buf_GetC(buf);
						green = Buf_GetC(buf);
						red = Buf_GetC(buf);
						alphabyte = Buf_GetC(buf);
						*pixbuf++ = red;
						*pixbuf++ = green;
						*pixbuf++ = blue;
						*pixbuf++ = alphabyte;
						break;
				}
			}
		}
	}
	else if (targa_header.image_type==10) // Runlength encoded RGB images
	{
		u8 red,green,blue,alphabyte,packetHeader,packetSize,j;
		for(row=rows-1; row>=0; row--)
		{
			//johnfitz -- fix for upside-down targas
			realrow = upside_down ? row : rows - 1 - row;
			pixbuf = targa_rgba + realrow*columns*4;
			//johnfitz
			for(column=0; column<columns; )
			{
				packetHeader=Buf_GetC(buf);
				packetSize = 1 + (packetHeader & 0x7f);
				if (packetHeader & 0x80) // run-length packet
				{
					switch (targa_header.pixel_size)
					{
						case 24:
							blue = Buf_GetC(buf);
							green = Buf_GetC(buf);
							red = Buf_GetC(buf);
							alphabyte = 255;
							break;
						case 32:
							blue = Buf_GetC(buf);
							green = Buf_GetC(buf);
							red = Buf_GetC(buf);
							alphabyte = Buf_GetC(buf);
							break;
						default: /* avoid compiler warnings */
							blue = red = green = alphabyte = 0;
					}

					for(j=0;j<packetSize;j++)
					{
						*pixbuf++=red;
						*pixbuf++=green;
						*pixbuf++=blue;
						*pixbuf++=alphabyte;
						column++;
						if (column==columns) // run spans across rows
						{
							column=0;
							if (row>0)
								row--;
							else
								goto breakOut;
							//johnfitz -- fix for upside-down targas
							realrow = upside_down ? row : rows - 1 - row;
							pixbuf = targa_rgba + realrow*columns*4;
							//johnfitz
						}
					}
				}
				else // non run-length packet
				{
					for(j=0;j<packetSize;j++)
					{
						switch (targa_header.pixel_size)
						{
							case 24:
								blue = Buf_GetC(buf);
								green = Buf_GetC(buf);
								red = Buf_GetC(buf);
								*pixbuf++ = red;
								*pixbuf++ = green;
								*pixbuf++ = blue;
								*pixbuf++ = 255;
								break;
							case 32:
								blue = Buf_GetC(buf);
								green = Buf_GetC(buf);
								red = Buf_GetC(buf);
								alphabyte = Buf_GetC(buf);
								*pixbuf++ = red;
								*pixbuf++ = green;
								*pixbuf++ = blue;
								*pixbuf++ = alphabyte;
								break;
							default: /* avoid compiler warnings */
								blue = red = green = alphabyte = 0;
						}
						column++;
						if (column==columns) // pixel packet run spans across rows
						{
							column=0;
							if (row>0)
								row--;
							else
								goto breakOut;
							//johnfitz -- fix for upside-down targas
							realrow = upside_down ? row : rows - 1 - row;
							pixbuf = targa_rgba + realrow*columns*4;
							//johnfitz
						}
					}
				}
			}
breakOut:;
		}
	}

	Buf_Free(buf);
	fclose(fin);

	*width = (s32)(targa_header.width);
	*height = (s32)(targa_header.height);
	return targa_rgba;
}

u8 *Image_LoadImage (const s8 *name, s32 *width, s32 *height)
{ // returns a pointer to hunk allocated RGBA data
	FILE *f;
	q_snprintf (loadfilename, sizeof(loadfilename), "%s.tga", name);
	COM_FOpenFile (loadfilename, &f, NULL);
	if (f)
		return Image_LoadTGA (f, width, height);
	return NULL;
}
