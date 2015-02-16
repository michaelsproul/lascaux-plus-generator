#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <algorithm>

#include <ft2build.h>
#include FT_FREETYPE_H

using std::min;

static const unsigned UNICODE_START = 97; // a
static const unsigned UNICODE_END = 123; // z

void output_glyph_to_json(FILE * file, unsigned char_code, FT_Face face);
void print_glyph(FT_Face face);

#define attempt(x, message) \
    if (x) { \
        fprintf(stderr, "Error: %s", message); \
        return 1; \
    }

// Get the ith bit of the given number.
#define get_bit(x, i) (((x) >> (i)) & 1)

int main() {
    FT_Library library;
    FT_Face face;

    // Font library and font face initialisation.
    attempt(FT_Init_FreeType(&library), "library init");

    attempt(
        FT_New_Face(library, "/usr/share/fonts/TTF/DejaVuSans.ttf", 0, &face),
        "font loading"
    );

    attempt(FT_Set_Char_Size(
          face,    /* handle to face object           */
          0,       /* char_width in 1/64th of points  */
          10*64,   /* char_height in 1/64th of points */
          200,     /* horizontal device resolution    */
          200
    ), "set char size");

    FILE * json_output = fopen("snapshot.json", "w");
    fprintf(json_output, "{");

    // Write JSON for each unicode character.
    for (unsigned char_code = UNICODE_START; char_code < UNICODE_END; char_code++) {
        int status = FT_Load_Char(face, char_code, FT_LOAD_DEFAULT | FT_LOAD_TARGET_MONO);

        if (status != 0) {
            printf("No glyph found for character %x.\n", char_code);
            continue;
        }

        status = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_MONO);

        if (status != 0) {
            printf("Unable to render character %x.\n", char_code);
            continue;
        }

        output_glyph_to_json(json_output, char_code, face);
        print_glyph(face);
    }

    fprintf(json_output, "}");
    fclose(json_output);

    return 0;
}

void output_glyph_to_json(FILE * file, unsigned char_code, FT_Face face) {
    FT_Bitmap * bitmap = &face->glyph->bitmap;

    // Write JSON in the format "XXXXXXXX": [{"strokes": [[{"x": x, "y": y} ...]]}],
    // where XXXXXXXX is the unicode character code in hexadecimal.
    fprintf(file, "\"%08x\": [{\"strokes\": [[\n", char_code);

    for (unsigned i = 0; i < bitmap->rows; i++) {
        for (int j = 0; j < bitmap->pitch; j++) {
            char byte = bitmap->buffer[i * bitmap->pitch + j];

            int bits_remaining = bitmap->width - 8 * j;
            int k_stop = min(8, bits_remaining);

            // Note, pixels are stored left-to-right from the MSB to the LSB.
            for (int k = 0; k < k_stop; k++) {
                if (get_bit(byte, 7 - k) == 1) {
                    fprintf(file, "{\"x\": %d, \"y\": %d},\n", 8 * j + k, i);
                }
            }
        }
    }

    fprintf(file, "]]}],\n");
}

void print_glyph(FT_Face face) {
    FT_Bitmap * bitmap = &face->glyph->bitmap;
    unsigned width = bitmap->width;

    for (unsigned i = 0; i < bitmap->rows; i++) {
        for (int j = 0; j < bitmap->pitch; j++) {
            char byte = bitmap->buffer[i * bitmap->pitch + j];

            int bits_remaining = width - 8 * j;
            int k_stop = min(8, bits_remaining);

            for (int k = 0; k < k_stop; k++) {
                if (get_bit(byte, 7 - k) == 1) {
                    //printf("{x: %d, y: %d}\n", j, i);
                    printf("\033[44;37m.\033[m");
                } else {
                    printf("|");
                }
            }
        }
        printf("\n");
    }
    printf("\n");
}
