#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <complex.h>

//#undef I
//#define I csqrt(-1.0)                                                           /* fixes some weird behavior with default I macro on gcc */

#include <libpng16/png.h>

#define MAX_N 16L                                                               /* max number of iterations for quadratic map */
#define RES   1024L                                                              /* square image resolution */

complex double quad_map(complex double c, long n)
{
    complex double z = c;
    for (long i = 0; i < n; ++i) {
        z = z*z + c;
    }

    return z;
}

bool Mandelbrot(complex double c)
{
    for (long n = 0; n < MAX_N; ++n) {
        if (cabs(quad_map(c, n)) > 2) {
            return false;
        }
    }

    return true;
}

complex double px_to_C(double x, double y) {
    complex double c_re = ( (x/RES)*4.0 - 2.0);
    complex double c_im = (-(y/RES)*4.0 + 2.0);
    complex double c = c_re + I*c_im;

    return c;
}

double area(bool grid[RES][RES])
{
    const double scale = 1.0/(RES*RES);
    const double total = 16.0;
    double portion = 0;

    for (long y = 0; y < RES; ++y) {
        for (long x = 0; x < RES; ++x) {
            if (grid[y][x]) {
                ++portion;
            }
        }
    }

    return portion*total*scale;
}

/* begin image related stuff */
typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t alpha;
} pixel_t;

typedef struct {
    pixel_t *pixels;
    size_t width;
    size_t height;
} bitmap_t;

pixel_t *pixel_at(bitmap_t *bitmap, long x, long y)
{
    return bitmap->pixels + bitmap->width * y + x;
}

int save_png_to_file(bitmap_t *bitmap, const char *path)
{
    FILE *fp;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    size_t x, y;
    png_byte **row_pointers = NULL;
    /* "status" contains the return value of this function. At first
    it is set to a value which means 'failure'. When the routine
    has finished its work, it is set to a value which means
    'success'. */
    int status = -1;
    /* The following number is set by trial and error only. I cannot
    see where it it is documented in the libpng manual.
    */
    int pixel_size = 4;
    int depth = 8;

    fp = fopen(path, "wb");
    if (!fp) {
        goto fopen_failed;
    }

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL) {
        goto png_create_write_struct_failed;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
        goto png_create_info_struct_failed;
    }

    /* Set up error handling. */

    if (setjmp(png_jmpbuf(png_ptr))) {
        goto png_failure;
    }

    /* Set image attributes. */

    png_set_IHDR(png_ptr,
                 info_ptr,
                 bitmap->width,
                 bitmap->height,
                 depth,
                 PNG_COLOR_TYPE_RGBA,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);

    /* Initialize rows of PNG. */

    row_pointers = png_malloc(png_ptr, bitmap->height * sizeof(png_byte *));
    for (y = 0; y < bitmap->height; ++y) {
        png_byte *row =
                png_malloc(png_ptr, sizeof(uint8_t) * bitmap->width * pixel_size);
        row_pointers[y] = row;
        for (x = 0; x < bitmap->width; ++x) {
            pixel_t * pixel = pixel_at(bitmap, (long) x, (long) y);
            *row++ = pixel->red;
            *row++ = pixel->green;
            *row++ = pixel->blue;
            *row++ = pixel->alpha;
        }
    }

    /* Write the image data to "fp". */

    png_init_io(png_ptr, fp);
    png_set_rows(png_ptr, info_ptr, row_pointers);
    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    /* The routine has successfully written the file, so we set
    "status" to a value which indicates success. */

    status = 0;

    for (y = 0; y < bitmap->height; y++) {
        png_free(png_ptr, row_pointers[y]);
    }
    png_free(png_ptr, row_pointers);

    png_failure:
    png_create_info_struct_failed:
    png_destroy_write_struct(&png_ptr, &info_ptr);
    png_create_write_struct_failed:
    fclose(fp);
    fopen_failed:
    return status;
}

void write_png(bool grid[RES][RES])                                             /* x and y resolution arguments refer to grid's */
{
    bitmap_t graph;
    long x, y;

    graph.width  = (size_t) RES;
    graph.height = (size_t) RES;

    graph.pixels = calloc(graph.width*graph.height, sizeof(pixel_t));

    for (y = 0; y < RES; ++y) {
        for (x = 0; x < RES; ++x) {
            pixel_t *px = pixel_at(&graph, x, y);
            if (grid[y][x]) {
                px->red   = 0xFF;
                px->blue  = 0xFF;
                px->green = 0xFF;
                px->alpha = 0xFF;
            } else {
                px->alpha = 0xFF;
            }
        }
    }

    save_png_to_file(&graph, "Mandelbrot.png");
}
/* end image related stuff */

int main()
{
    bool grid[RES][RES];

    const long SYM = (RES % 2) ? (RES+1)/2 : (RES/2)+1;                         /* take advantage of symmetry across real axis */
    long x, y;
    double progress = 0;
    for (y = 0; y < SYM; ++y) {
        for (x = 0; x < RES; ++x) {
            complex double c = px_to_C((double) x,(double) y);
            grid[y][x] = Mandelbrot(c);
            printf("%2.2lf\n", 100*(++progress/(RES*RES)));
        }
    }
    for (y = SYM; y < RES; ++y) {
        for (x = 0; x < RES; ++x) {
            grid[y][x] = grid[RES-y][x];
            printf("%2.2lf\n", 100*(++progress/(RES*RES)));
        }
    }

    fprintf(stdout, "The area of the Mandelbrot set is approximately %lf.\n",
            area(grid));

    write_png(grid);

    return 0;
}