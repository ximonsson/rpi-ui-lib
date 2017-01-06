#include <freetype2/ft2build.h>
#include FT_FREETYPE_H
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>


FT_Library   library = NULL;    // FreeType library
FT_Face      face    = NULL;    // FreeType face to use


/**
 *  Load a glyph from input character.
 *  Glyph is loaded to, and can be accessed through, the current loaded face.
 *  Returns non-zero if a glyph could not be found.
 */
static int load_glyph (unsigned int character)
{
	// get character index in glyph map
	FT_UInt index = FT_Get_Char_Index (face, character);
	if (index == 0)
	{
		return 1;
	}

	// load the glyph from current face
	FT_Error error = FT_Load_Glyph (face, index, FT_LOAD_DEFAULT);
	if (error)
	{
		fprintf (stderr, "Error loading glyph (%d)\n", error);
		return 1;
	}

    return 0;
}


int rpi_ui_text_init ()
{
	// init library
	FT_Error error = FT_Init_FreeType (&library);
	if (error)
	{
		fprintf (stderr, "error init ft library (%d)\n", error);
		return 1;
	}
	return 0;
}


int rpi_ui_text_deinit ()
{
	FT_Error error = FT_Done_FreeType (library);
	if (error)
	{
		fprintf (stderr, "Error destroying library (%d)\n", error);
		return 1;
	}
	return 0;
}


int rpi_ui_text_load_font (const char* filepath, size_t size)
{
	// if a face is already loaded we need to deallocate it
	if (face)
		FT_Done_Face (face);

	// create new face
	FT_Error error = FT_New_Face (library, filepath, 0, &face);
	if (error == FT_Err_Unknown_File_Format)
	{
		// we did not recognize the file format
		fprintf (stderr, "unknown file format\n");
		return 1;
	}
	else if (error)
	{
		// unknown error
		fprintf (stderr, "error creating new face (%d)\n", error);
		return 1;
	}

	// set the size of the glyhps
	error = FT_Set_Pixel_Sizes (face, 0, size);
	if (error)
	{
		fprintf (stderr, "Error setting pixel size (%d)\n", error);
		return 1;
	}

	return 0;
}


int rpi_ui_text_render (const char* txt, size_t length, uint8_t** bp, int width, uint32_t color)
{
	int column = 0;
	for (size_t i = 0; i < length; i ++)
	{
		load_glyph (txt[i]);
		if (txt[i] == '\n' || column >= width) // TODO column + glyph.width >= width
		{
			// new line
			*bp -= column;
			*bp += width * 20; // TODO change 20 to actual line height
			column = 0;
			if (txt[i] == '\n')
				continue;
		}
		rpi_ui_text_render_glyph (*bp, width, color);
		column += 20; // TODO change to actual glyph width
	}
	return 0;
}


void rpi_ui_text_render_glyph (uint8_t* bp, int width, uint32_t color)
{

}
