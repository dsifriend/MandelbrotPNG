#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>

//#undef I
//#define I csqrt(-1.0)                                                           /* fixes some weird behavior with default I macro on gcc */

#include <libpng16/png.h>

#define MAX_N 512L                                                              /* max number of iterations for quadratic map */
#define RES   16384L                                                            /* square image resolution */

/* original Mandelbrot function and quadratic mapping */
//complex double quad_map(complex double c, long n)
//{
//    complex double z = c;
//    for (long i = 0; i < n; ++i) {
//        z = z*z + c;
//    }
//
//    return z;
//}
//
//bool Mandelbrot(complex double c)
//{
//    for (long n = 0; n < MAX_N; ++n) {
//        if (cabs(quad_map(c, n)) > 2) {
//            return false;
//        }
//    }
//
//    return true;
//}

/* integrated Mandelbrot function and quadratic mapping */
u_int8_t Mandelbrot(complex double c)
{
    complex double z = c;
    for (long i = 1; i <= MAX_N; ++i) {
        if (cabs(z) > 2) {
            return (u_int8_t) i;
        }
        z = z*z + c;
    }

    return 0;
}

complex double px_to_C(double x, double y) {
//    complex double c_re = ( (x/RES)*4.0 - 2.0);
//    complex double c_im = (-(y/RES)*4.0 + 2.0);
//    complex double c = c_re + I*c_im;

    return ( (x/RES)*4.0 - 2.0) + I*(-(y/RES)*4.0 + 2.0);
}

double area(const u_int8_t *grid)
{
    const double scale = 1.0/(RES*RES);
    const double total = 16.0;
    double portion = 0;

    for (long y = 0; y < RES; ++y) {
        for (long x = 0; x < RES; ++x) {
            if (grid[y*RES + x]) {
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

void color_pixel(pixel_t *px, u_int32_t color, u_int8_t shade)                  /* shade the hex RGB color value and apply it to px */
{
    px->red   = (u_int8_t) ceil(((color/0x10000        )*shade)/256.0);
    px->green = (u_int8_t) ceil(((color/0x100   % 0x100)*shade)/256.0);
    px->blue  = (u_int8_t) ceil(((color         % 0x100)*shade)/256.0);
}

void write_png(const u_int8_t *grid)                                               /* x and y resolution arguments refer to grid's */
{
    bitmap_t graph;
    long x, y;

    graph.width  = (size_t) RES;
    graph.height = (size_t) RES;

    graph.pixels = calloc(graph.width*graph.height, sizeof(pixel_t));

    for (y = 0; y < RES; ++y) {
        for (x = 0; x < RES; ++x) {
            pixel_t *px = pixel_at(&graph, x, y);
            u_int8_t val = grid[y*RES + x];
            if (val) {
//                px->red   = 0xFF;
//                px->blue  = 0xFF;
//                px->green = 0xFF;

                color_pixel(px, 0xFFFFFF, val);

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
    u_int8_t *grid = calloc(RES*RES, sizeof(u_int8_t));

    FILE *grid_file = fopen("grid.txt", "w");
    const long SYM = (RES % 2) ? (RES+1)/2 : (RES/2)+1;                         /* take advantage of symmetry across real axis */
    const double px_quant = (double) RES*RES;
    double progress = 0;
    long x, y;
    for (y = 0; y < SYM; ++y) {
        for (x = 0; x < RES; ++x) {
            complex double c = px_to_C((double) x,(double) y);
            grid[y*RES + x] = Mandelbrot(c);
            fprintf(grid_file, "%c", (grid[y*RES + x]) ? 'X' : ' ');
            printf("%16.0lf of %16.0lf values determined so far.\n",
                   ++progress, px_quant);
        }
        fprintf(grid_file, "\n");
    }
    for (y = SYM; y < RES; ++y) {
        for (x = 0; x < RES; ++x) {
            grid[y*RES + x] = grid[(RES-y)*RES + x];
            fprintf(grid_file, "%c", (grid[y*RES + x]) ? 'X' : ' ');
            printf("%16.0lf of %16.0lf values determined so far.\n",
                   ++progress, px_quant);
        }
        fprintf(grid_file, "\n");
    }
    fclose(grid_file);

    FILE *area_file = fopen("area.txt", "w");
    fprintf(area_file, "The area of the Mandelbrot set is approximately %1.16lf.\n",
            area(grid));
    fclose(area_file);

    write_png(grid);

    return 0;
}