#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef unsigned int LONG;

#pragma pack(push,1)

typedef struct {
    char         filetype[2];
    unsigned int filesize;
    short        reserved1;
    short        reserved2;
    unsigned int dataoffset;
} file_header;

typedef struct {
    file_header  fileheader;
    unsigned int headersize;
    int          width;
    int          height;
    short        planes;
    short        bitsperpixel;
    unsigned int compression;
    unsigned int bitmapsize;
    int          horizontalres;
    int          verticalres;
    unsigned int numcolors;
    unsigned int importantcolors;
} bitmap_header;

#pragma pack(pop)

unsigned char get_red(unsigned char *imagedata, int x, int y, int imageWidth, int imageHeight) {
  int realwidth = 3 * imageWidth;
  if(imageWidth % 4 != 0) {realwidth = (imageWidth * 3) + 4 - (imageWidth * 3) % 4;}
  return imagedata[x*3 + (y*realwidth) + 2];
}

unsigned char get_green(unsigned char *imagedata, int x, int y, int imageWidth, int imageHeight) {
  int realwidth = 3 * imageWidth;
  if(imageWidth % 4 != 0) {realwidth = (imageWidth * 3) + 4 - (imageWidth * 3) % 4;}
  return imagedata[x*3 + (y*realwidth) + 1];
}

unsigned char get_blue(unsigned char *imagedata, int x, int y, int imageWidth, int imageHeight) {
  int realwidth = 3 * imageWidth;
  if(imageWidth % 4 != 0) {realwidth = (imageWidth * 3) + 4 - (imageWidth * 3) % 4;}
  return imagedata[x*3 + (y*realwidth) + 0];
}

int main(int argc, char **argv) {
  FILE *file1, *file2, *out;
  bitmap_header *header1, *header2, *header;
  int n;
  unsigned char *data1, *data2, *largerdata, *smallerdata;
  char *input1 = argv[1], *input2 = argv[2], *output = "output.bmp";

  file1 = fopen(input1, "rb");
  file2 = fopen(input2, "rb");

  //Read the input file headers for image 1:
  header1 = (bitmap_header*) malloc(sizeof(bitmap_header));
  fread(header1, sizeof(bitmap_header), 1, file1);

  //Read the input file headers for image 2:
  header2 = (bitmap_header*) malloc(sizeof(bitmap_header));
  fread(header2, sizeof(bitmap_header), 1, file2);

  //Read image 1 data:
  data1 = (unsigned char*) malloc(sizeof(char)*header1->bitmapsize);
  fseek(file1, sizeof(char)*header1->fileheader.dataoffset, SEEK_SET);
  fread(data1, sizeof(char),header1->bitmapsize, file1);

  //Read image 2 data:
  data2 = (unsigned char*) malloc(sizeof(char)*header2->bitmapsize);
  fseek(file2, sizeof(char)*header2->fileheader.dataoffset, SEEK_SET);
  fread(data2, sizeof(char),header2->bitmapsize, file2);

  int width1 = header1->width, width2 = header2->width, width, largerWidth;
  int height1 = header1->height, height2 = header2->height, height, largerHeight;

  unsigned char red_image1, red_image2, red_result;
  unsigned char topLeft[3];
  unsigned char topRight[3];
  unsigned char bottomLeft[3];
  unsigned char bottomRight[3];
  unsigned char green_image1, green_image2, green_result;
  unsigned char blue_image1, blue_image2, blue_result;
  double ratio = 0.5;

  int maxHeight = fmax(height1, height2), minHeight = fmin(height1, height2);
  int maxWidth = fmax(width1, width2), minWidth = fmin(width1, width2);
  double heightRatio = (double)minHeight / (double)maxHeight;
  double widthRatio = (double)minWidth / (double)maxWidth;



  if(width1 * height1 > width2 * height2) {
    smallerdata = data2; largerdata = data1; width = width2; height = height2; largerWidth = width1, largerHeight = height1, header = header1;
  }
  else {
    smallerdata = data1; largerdata = data2; width = width1; height = height1; largerWidth = width2, largerHeight = height2, header = header2;
  }

  int realwidth = 3 * largerWidth;
  if(largerWidth % 4 != 0) {realwidth = (largerWidth * 3) + 4 - (largerWidth * 3) % 4;}

  for(int col = 0; col < largerWidth - 1; col++) {
    for(int row = 0; row < largerHeight - 1; row++) {
      double x = col * widthRatio;
      double y = row * heightRatio;

      if ((x - (int)x == 0) && (y - (int)y == 0)) {
        red_image1 = get_red(data1, col, row, width1, height1); 
        /*printf("%d, %d, %d, %d\n", col, row, width2, height2);*/
        red_image2 = get_red(data2, x, y, width2, height2); 
        red_result = (ratio * red_image1) + ((1 - ratio) * red_image2);

        green_image1 = get_green(data1, col, row, width1, height1); 
        green_image2 = get_green(data2, x, y, width2, height2); 
        green_result = (ratio * green_image1) + ((1 - ratio) * green_image2);

        blue_image1 = get_blue(data1, col, row, width1, height1); 
        blue_image2 = get_blue(data2, x, y, width2, height2); 
        blue_result = (ratio * blue_image1) + ((1 - ratio) * blue_image2);

        largerdata[(col * 3) + (row * realwidth) + 2] = red_result;
        largerdata[(col * 3) + (row * realwidth) + 1] = green_result;
        largerdata[(col * 3) + (row * realwidth) + 0] = blue_result;
      } else {
        // Get smaller image color data
        int x1 = floor(x); 
        int x2 = ceil(x);
        int y1 = floor(y);
        int y2 = ceil(y);

        float dx = x - x1, dy = y - y1;

        topLeft[0] = get_red(smallerdata, x1, y2, width, height);
        topLeft[1] = get_green(smallerdata, x1, y2, width, height);
        topLeft[2] = get_blue(smallerdata, x1, y2, width, height);

        topRight[0] = get_red(smallerdata, x2, y2, width, height);
        topRight[1] = get_green(smallerdata, x2, y2, width, height);
        topRight[2] = get_blue(smallerdata, x2, y2, width, height);

        bottomLeft[0] = get_red(smallerdata, x1, y1, width, height);
        bottomLeft[1] = get_green(smallerdata, x1, y1, width, height);
        bottomLeft[2] = get_blue(smallerdata, x1, y1, width, height);

        bottomRight[0] = get_red(smallerdata, x2, y1, width, height);
        bottomRight[1] = get_green(smallerdata, x2, y1, width, height);
        bottomRight[2] = get_blue(smallerdata, x2, y1, width, height);

        unsigned char red_left = (topLeft[0] * (1 - dy)) + (bottomLeft[0] * dy);
        unsigned char red_right = (topRight[0] * (1 - dy)) + (bottomRight[0] * dy);
        unsigned char red_result = (red_left * (1 - dx)) + (red_right * dx);

        unsigned char green_left = (topLeft[1] * (1 - dy)) + (bottomLeft[1] * dy);
        unsigned char green_right = (topRight[1] * (1 - dy)) + (bottomRight[1] * dy);
        unsigned char green_result = (green_left * (1 - dx)) + (green_right * dx);

        unsigned char blue_left = (topLeft[2] * (1 - dy)) + (bottomLeft[2] * dy);
        unsigned char blue_right = (topRight[2] * (1 - dy)) + (bottomRight[2] * dy);
        unsigned char blue_result = (blue_left * (1 - dx)) + (blue_right * dx);

        /*printf("%d, %d, %d, %d\n", col, row, largerWidth, largerHeight);*/
        red_image1 = get_red(largerdata, col, row, largerWidth, largerHeight);
        red_result = (ratio * red_image1) + ((1 - ratio) * red_result);

        green_image1 = get_green(largerdata, col, row, maxWidth, maxHeight);
        green_result = (ratio * green_image1) + ((1 - ratio) * green_result);

        blue_image1 = get_blue(largerdata, col, row, maxWidth, maxHeight);
        blue_result = (ratio * blue_image1) + ((1 - ratio) * blue_result);


        largerdata[(col * 3) + (row * realwidth) + 2] = red_result;
        largerdata[(col * 3) + (row * realwidth) + 1] = green_result;
        largerdata[(col * 3) + (row * realwidth) + 0] = blue_result;
      }
    }
  }

  //Write the image data
  out = fopen(output, "wb");
  fwrite(header, sizeof(char),sizeof(bitmap_header),out);

  fseek(out, sizeof(char)*header->fileheader.dataoffset,SEEK_SET);
  n=fwrite(largerdata, sizeof(char),header->bitmapsize,out);

  fclose(file1);
  fclose(file2);
  fclose(out);

  free(header1);
  free(header2);
  free(data1);
  free(data2);
  return 0;
}
