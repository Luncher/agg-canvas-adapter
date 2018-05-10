#include <stdio.h>
#include <string.h>
#include "agg_pixfmt_rgb.h"

enum {
  frame_width = 320,
  frame_height = 200
};

bool write_ppm(const unsigned char* buf,
              unsigned width,
              unsigned height,
              const char* file_name)
{
    FILE* fd = fopen(file_name, "wb");
    if(fd) {
        fprintf(fd, "P6 %d %d 255", width, height);
        // printf("P6 %d %d 255", width, height);
        fwrite(buf, 1, width * height * 3, fd);
        fclose(fd);
        return true;
    }
    return false;
}


int main() 
{
  unsigned char* buffer = new unsigned char[frame_width * frame_height * 3];
  memset(buffer, 255, frame_width * frame_height * 3);

  agg::rendering_buffer rbuf(buffer, frame_width, frame_height, frame_width * 3);
  agg::pixfmt_rgb24 pixf(rbuf);

  agg::rgba8 span[frame_width];

  unsigned i;
  for(i = 0; i < frame_width; i++)
  {
    agg::rgba c(380.0 + 400.0 * i / frame_width, 0.8);
    span[i] = agg::rgba8(c);
  }

  for(i = 0; i < frame_height; i++) {
    pixf.blend_color_hspan(0, i, frame_width, span, 0, 0);
  }

  write_ppm(buffer, frame_width, frame_height, "agg_test3.ppm");

  delete []buffer;
  return 0;
}