// Julia sets
// Authors: Vincent Loechner and ChatGPT (and ORTANCA Baris )

// Usage:
//
// - the generated image is `fractal.bmp`
//
// Some typical runs:
//   NAME: RUN SIZE (arbitrary unit), DESCRIPTION
//
// - base: tiny run (0.01), the whole Julia set in low resolution
//    ./julia 800 600 1. 0 0 1000
// - horse_head_small: small run (0.06) imbalanced
//    ./julia 1600 1200 25. .6 0.29 1000
// - base_medium: medium run (0.28), whole set hi-res, huge imbalance
//    ./julia 8000 6000 0.5 0 0 1000
// - horse_head_medium: medium run (1.38), larger image
//    ./julia 8000 6000 25. .6 0.29 1000
// - nice spiral: large run (4.0), large image
//    ./julia 8000 6000 1e12 -0.07110516009 0.8867120524212 3000
// - horse_head_detail: large run (3.21), large image, large imbalance
//    ./julia 8000 6000 1500000. .5903 0.28002 2000
// - orange_large: large run (4.2), medium image, more iterations
//    ./julia 3000 2000 4e10 -.48429274835 0.1372305145 4500
// - orange_huge: huge run (34.3), even larger image
//    ./julia 8000 6000 4e10 -.48429274835 0.1372305145 4500


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

int WIDTH, HEIGHT;

typedef struct {
    uint8_t b, g, r;  // red green blue
} Pixel;

Pixel julia_set(double x0, double y0, double cx, double cy, int max_iter) {
    double x = x0, y = y0;
    int iter = 0;
    while (x*x + y*y <= 4. && iter < max_iter) {
        double xtemp = x*x - y*y + cx;
        y = 2*x*y + cy;
        x = xtemp;
        iter++;
    }

    Pixel pixel;
    if (iter == max_iter) {
        pixel.r = 0;
        pixel.g = 0;
        pixel.b = 0;
    } else {
        double ratio = (double)iter / max_iter;
        pixel.r = (uint8_t)(9. * (1. - ratio) * ratio * ratio * ratio * 255.);
        pixel.g = (uint8_t)(15. * (1. - ratio) * (1. - ratio) * ratio * ratio * 255.);
        pixel.b = (uint8_t)(8.5 * (1. - ratio) * (1. - ratio) * (1. - ratio) * ratio * 255.);
    }

    return pixel;
}

#pragma pack(push, 1)
typedef struct {
    uint16_t type;
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;
    uint32_t header_size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bits_per_pixel;
    uint32_t compression;
    uint32_t image_size;
    int32_t x_pixels_per_meter;
    int32_t y_pixels_per_meter;
    uint32_t colors_used;
    uint32_t colors_important;
} BMPHeader;
#pragma pack(pop)

void write_bitmap(const char *filename, Pixel *image) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        printf("Error: Unable to create bitmap file\n");
        return;
    }

    BMPHeader header = {
        .type = 0x4D42,
        .size = sizeof(BMPHeader) + WIDTH * HEIGHT * 3,
        .offset = sizeof(BMPHeader),
        .header_size = 40,
        .width = WIDTH,
        .height = HEIGHT,
        .planes = 1,
        .bits_per_pixel = 24,
        .compression = 0,
        .image_size = WIDTH * HEIGHT * 3,
        .x_pixels_per_meter = 0,
        .y_pixels_per_meter = 0,
        .colors_used = 0,
        .colors_important = 0
    };

    fwrite(&header, sizeof(BMPHeader), 1, file);

    fwrite(image, sizeof(Pixel), WIDTH * HEIGHT, file);

    fclose(file);
}

int main(int argc, char **argv)
{
  if(argc != 7)
  {
    fprintf(stderr,
      "Usage: %s WIDTH HEIGHT ZOOM MOVE_X MOVE_Y MAX_ITER\n",
      argv[0]);
    return(1);
  }
  WIDTH = atoi(argv[1]);              // 8000
  HEIGHT = atoi(argv[2]);             // 6000
  double zoom = atof(argv[3]);        // 25
  double moveX = atof(argv[4]);       // 0.6
  double moveY = atof(argv[5]);       // 0.29
  int max_iterations = atoi(argv[6]); // 1000

  // CHANGE TO GET OTHER JULIA SETS (close to Mandelbrot borders is nice)
  double cReal = -0.7;
  double cImag = 0.27015;

  // Init
  Pixel *image = (Pixel *)malloc(WIDTH * HEIGHT * sizeof(Pixel));
  if (!image) {
    printf("Error: Unable to allocate memory\n");
    return 1;
  }

  // Compute
  double t1 = omp_get_wtime();

  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      double zx = 1.5 * ((double)x - WIDTH / 2.) / (0.5 * zoom * WIDTH) + moveX;
      double zy = ((double)y - HEIGHT / 2.) / (0.5 * zoom * HEIGHT) + moveY;
      Pixel color = julia_set(zx, zy, cReal, cImag, max_iterations);
      image[y * WIDTH + x] = color;
    }
  }

  // Save file
  double t2 = omp_get_wtime();

  write_bitmap("fractal.bmp", image);

  // End
  double t3 = omp_get_wtime();

  free(image);
  printf("Compute: %lfs, Savefile: %lf\n", t2-t1, t3-t2);

  return 0;
}
