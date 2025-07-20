import freetype
import os
import glob
import datetime
import re

HEADER = """
/*
 * <filename>.h
 *
 *  Created on: <DATE>
 *  
 * THIS IS AN AUTO GENERATED FILE.
 * DO NOT MODIFY
 */

#ifndef <FILENAME>_H_
#define <FILENAME>_H_

// Standard library
#include <stdint.h>
#include "font_library/font.h"

const uint8_t <filename> [<NUMBYTES>] = 

"""

FOOTER = """

font_t font_<filename> = {
    .rom_font = false,
    .ft81x_font_index = 0, // NOTE: This needs to be added at runtime
    .font_name = "<filename>\\0",
    .font_format = <font_format>,
    .font_size = <font_size>,
    .font_baseline = <font_baseline>,
    .font_caps_height = <font_caps_height>,
    .font_x_width = <font_x_width>,
    .pFontTable = <filename>,
    .fontTableSize = <NUMBYTES>
};

#endif
"""


def convert_font_to_c_code(font_path: str, font_size: int = 24):
    """
    Converts a TTF font to a C source file with bitmap data and glyph metadata,
    with support for the FT81x format, found here: https://www.ftdichip.com/Support/Documents/ProgramGuides/FT81X_Series_Programmer_Guide.pdf

    Note that the FT81x only supports 128 characters, so only ASCII printable characters are converted.
    What's written to the FT81x is an array of bytes, where:
    Address | Size(B)|  Value
    p + 0       128     width of each font character, in pixels
    p + 128     4       font bitmap format, for example L1, L4 or L8
    p + 132     4       font line stride, in bytes
    p + 136     4       font width, in pixels
    p + 140     4       font height, in pixels
    p + 144     4       pointer to font graphic data in memory
    
    """
    face = freetype.Face(font_path)
    face.set_pixel_sizes(0, font_size)
    bitmap = face.glyph.bitmap
    width = bitmap.width
    height = bitmap.rows

    # Assume 8 bits per pixel (grayscale)
    bits_per_pixel = 8  # Could be 1, 4, or 8 depending on your use case
    stride = ((width * bits_per_pixel + 7) // 8)

    bitmap_data = []

    widths = []
    max_width = 0
    max_height = 0

    for char_code in range(0x20, 0x7F):  # ASCII printable range
        face.load_char(chr(char_code), freetype.FT_LOAD_RENDER | freetype.FT_LOAD_TARGET_MONO)
        bitmap = face.glyph.bitmap
        baseline = face.glyph.bitmap_top
        font_caps_height = face.size.ascender >> 6
        font_x_width = face.size.max_advance >> 6
        widths.append(bitmap.width)


        height = bitmap.rows
        if height > max_height:
            max_height = height
        if bitmap.width > max_width:
            max_width = bitmap.width

        # Write bitmap into final buffer
        bitmap_data.extend(bitmap.buffer)
    
    # Ft81x font header data
    ft81x_bitmap_data = {
        "widths": widths,
        "format":[17,0,0,0], # L2 format per default
        "stride":[stride,0,0,0],
        "max_width":[max_width,0,0,0],
        "max_height":[max_height,0,0,0],
        "start_addr":[148,0,0,0], # 148 is where the bitmap data starts for pracitally all fonts.
        "bitmap_data": bitmap_data,
    }

    # Write the bitmap data to a C source file
    orig_font_name = os.path.splitext(os.path.basename(font_path))[0]
    font_name = orig_font_name
    font_name = font_name.replace("-", "_")
    font_name = font_name.replace(" ", "_")
    c_filename = f"{os.path.dirname(os.path.abspath(__file__))}/{orig_font_name}_{font_size}_L2.h"
    with open(c_filename, "w") as c_file:
        # Write the header
        header = HEADER
        h_header = header.replace("<filename>", font_name)
        h_header = h_header.replace("<FILENAME>", font_name.upper())
        h_header = h_header.replace("<NUMBYTES>", str(148 + len(bitmap_data)))
        date = datetime.datetime.now()
        h_header = h_header.replace("<DATE>", date.strftime("%Y-%m-%d %H:%M:%S"))
        # Write the header to the file
        c_file.write(h_header)

        # Write the bitmap metadata:
        c_file.write("{\n")
        for key, bytes in ft81x_bitmap_data.items():
            c_file.write(f"// {key}\n")
            c_file.write(f"{",".join(str(i) for i in bytes)},\n")
        c_file.write("};\n")
        # Write the footer
        footer = FOOTER
        footer = footer.replace("<filename>", font_name)
        footer = footer.replace("<FILENAME>", font_name.upper())
        footer = footer.replace("<font_format>", str(2))
        footer = footer.replace("<font_size>", str(font_size))
        footer = footer.replace("<font_baseline>", str(baseline))
        footer = footer.replace("<font_caps_height>", str(font_caps_height))
        footer = footer.replace("<font_x_width>", str(font_x_width))
        footer = footer.replace("<NUMBYTES>", str(148 + len(bitmap_data)))
        c_file.write(footer)
    

def find_ttf_files_in_script_dir():
    """
    Finds all TTF files in the same directory as this script.
    """
    script_dir = os.path.dirname(os.path.abspath(__file__))
    ttf_files = glob.glob(os.path.join(script_dir, "*.ttf"))
    return ttf_files

def include_fonts_in_library():
    """
    Includes all the generated font headers in the fonts library file.
    """
    script_dir = os.path.dirname(os.path.abspath(__file__))
    all_header_files = glob.glob(os.path.join(script_dir, "*.h"))
    # all_header_files now contain all the .h files in this directory, 
    # Filter out the ones that are font files (those that match the pattern 
    # *_<size>_L<format>.h)
    font_header_files = []
    for file in all_header_files:
        if re.search(r".*_\d+_L\d+\.h$", file):
            font_header_files.append(file)

    font_library_file = script_dir + "/font_library/font_library.c"
    font_library_template_file = script_dir + "/font_library/font_library_template.c"
    # Copy the template file to the font library file if it doesn't exist
    if not os.path.exists(font_library_file):
        if os.path.exists(font_library_template_file):
            with open(font_library_template_file, "r") as f:
                template_content = f.read()
            with open(font_library_file, "w") as f:
                f.write(template_content)
        else:
            raise FileNotFoundError(f"Font library file {font_library_file} does not exist and no template found at {font_library_template_file}")
    # Replace the section starting with 
    # ""// <START INCLUDE FONTS>"" and ending with ""// <END INCLUDE FONTS>"" 
    # with the new includes
    includes = "// <START INCLUDE FONTS>\n"
    for font_header in font_header_files:
        font_header_basename = os.path.basename(font_header)
        includes += f'#include "../{font_header_basename}"\n'
    includes += "// <END INCLUDE FONTS>\n"
    with open(font_library_file, "r") as f:
        font_library_content = f.read()
    font_library_content = re.sub(
        r"// <START INCLUDE FONTS>.*// <END INCLUDE FONTS>", 
        includes, 
        font_library_content, 
        flags=re.DOTALL
    )
    

    # Update the pFontLibraryTable array
    pFontLibraryTable_entries = []
    for font_header in font_header_files:
        # Extract the font_t variable name from the header file name
        with open(font_header, "r") as f:
            content = f.read()
            match = re.search(r"font_t font_(\w+)", content)
            if match:
                font_var_name = f"&font_{match.group(1)}"
                pFontLibraryTable_entries.append(font_var_name)
    # Find the "MAX_LEN_FONT_LIBRARY_TABLE" definition in the font_library.h file
    with open(script_dir + "/font_library/font_library.h", "r") as f:
        font_library_h_content = f.read()
    max_len_match = re.search(r"#define MAX_LEN_FONT_LIBRARY_TABLE (\d+)", font_library_h_content)

    # construct the new pFontLibraryTable array
    pFontLibraryTable_array = "// <START FONT DEFINITIONS>\n"
    pFontLibraryTable_array += "font_t* pFontLibraryTable[MAX_LEN_FONT_LIBRARY_TABLE] = {\n"
    for i in range(int(max_len_match.group(1))):
        if i < len(pFontLibraryTable_entries):
            pFontLibraryTable_array += f"    [{i}] = {pFontLibraryTable_entries[i]},\n"
        else:
            pFontLibraryTable_array += f"    [{i}] = NULL,\n"
    #remove the last comma
    if pFontLibraryTable_array.endswith(",\n"):
        pFontLibraryTable_array = pFontLibraryTable_array[:-2] + "\n"
    pFontLibraryTable_array += "};\n"
    pFontLibraryTable_array += "// <END FONT DEFINITIONS>\n"

    # Replace the existing pFontLibraryTable array in the font_library_content
    font_library_content = re.sub(
        r"// <START FONT DEFINITIONS>.*// <END FONT DEFINITIONS>", 
        pFontLibraryTable_array, 
        font_library_content, 
        flags=re.DOTALL
    )

    with open(font_library_file, "w") as f:
        f.write(font_library_content)
    print(f"Updated {font_library_file} with {len(font_header_files)} fonts.")


if __name__ == "__main__":
    # Locate the fonts in this folder
    ttf_files = find_ttf_files_in_script_dir()
    print(f"Converting the following font files: {ttf_files}")
    for ttf_file in ttf_files:
        print(f"Converting {ttf_file}")
        convert_font_to_c_code(font_path = ttf_file, font_size=24)

    # Update the font library to include the new fonts
    include_fonts_in_library()

    print("Font conversion complete. Font library updated.")
