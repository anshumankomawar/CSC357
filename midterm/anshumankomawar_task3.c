#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef unsigned int LONG;

typedef struct {
  WORD bfType;      // specifies the file type
  DWORD bfSize;     // specifies the size in bytes of the bitmap file
  WORD bfReserved1; // reserved; must be 0
  WORD bfReserved2; // reserved;must be 0
  DWORD bfOffBits;  // species the offset in bytes from the bitmapfileheader to
                    // the bitmap bits
} tagBITMAPFILEHEADER;

typedef struct {
  DWORD biSize;         // specifies the number of bytes required by the struct
  LONG biWidth;         // specifies width in pixels
  LONG biHeight;        // species height in pixels
  WORD biPlanes;        // specifies the number of color planes, must be 1
  WORD biBitCount;      // specifies the number of bit per pixel
  DWORD biCompression;  // spcifies the type of compression
  DWORD biSizeImage;    // size of image in bytes
  LONG biXPelsPerMeter; // number of pixels er meter in x axis
  LONG biYPelsPerMeter; // number of pixels per meter in y axis
  DWORD biClrUsed;      // number of colors used by the bitmap
  DWORD biClrImportant; // number of colors that are important
} tagBITMAPINFOHEADER;

typedef struct {
  tagBITMAPFILEHEADER fh;
  tagBITMAPINFOHEADER fih;
  unsigned char *data;
} BITMAP;

BITMAP readfile(char *filename, float ratio) {
  FILE *file = fopen(filename, "rb");

  tagBITMAPFILEHEADER fh;
  tagBITMAPINFOHEADER fih;

  fread(&fh.bfType, 1, 2, file);
  fread(&fh.bfSize, 1, 4, file);
  fread(&fh.bfReserved1, 1, 2, file);
  fread(&fh.bfReserved2, 1, 2, file);
  fread(&fh.bfOffBits, 1, 4, file);
  fread(&fih, 1, sizeof(tagBITMAPINFOHEADER), file);

  unsigned char *data = (unsigned char*) malloc(sizeof(char)*fh.bfSize);
  fread(data, 1, fih.biSizeImage, file);
  fclose(file);

  BITMAP bmp = {fh, fih, data};
  return bmp;
}

unsigned char getColor(unsigned char *data, int width, int x, int y, int color) {
  int realWidth = (!(width % 4)) ? 3 * width : (width * 3) + 4 - (width * 3) % 4;
  return data[(x * 3) + (y * realWidth) + color];
}

int main() {
  char *filename1 = "nopadding.bmp", *filename2 = "yespadding.bmp";
  double ratio = 0.5;
  BITMAP bmp1 = readfile(filename1, ratio);
  BITMAP bmp2 = readfile(filename2, ratio);
  BITMAP largerBMP, smallerBMP;
  unsigned char *smallerData, *largerData;

  int numberOfChildren = 4;
  pid_t *childPids = NULL;
  pid_t p;

  childPids = malloc(numberOfChildren * sizeof(pid_t));

  int startHeight, startWidth, endHeight, endWidth;
  int width1 = bmp1.fih.biWidth, width2 = bmp2.fih.biWidth;
  int height1 = bmp1.fih.biHeight, height2 = bmp2.fih.biHeight;

  clock_t *times = malloc(numberOfChildren * sizeof(clock_t));
  clock_t startTime = clock();
  
  int maxHeight = fmax(height1, height2), minHeight = fmin(height1, height2);
  int maxWidth = fmax(width1, width2), minWidth = fmin(width1, width2);
  double heightRatio = (double)minHeight / (double)maxHeight;
  double widthRatio = (double)minWidth / (double)maxWidth;
  
  if(width1 * height1 > width2 * height2) {
    largerBMP = bmp1, smallerBMP = bmp2;
  } else {
    largerBMP = bmp2, smallerBMP = bmp1;
  }

  unsigned char *data = mmap(NULL, largerBMP.fih.biSizeImage, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  int realWidth = (!(largerBMP.fih.biWidth % 4)) ? 3 * largerBMP.fih.biWidth : (largerBMP.fih.biWidth * 3) + 4 - (largerBMP.fih.biWidth * 3) % 4;

  for (int ii = 0; ii < numberOfChildren; ++ii) {
    if ((p = fork()) == 0) {

      switch(ii) {
        case 0:
          startWidth = 0;
          startHeight = 0;
          endWidth = largerBMP.fih.biWidth / 2;
          endHeight = largerBMP.fih.biHeight / 2;
          break;
        case 1:
          startWidth = largerBMP.fih.biWidth / 2;
          startHeight = 0;
          endWidth = largerBMP.fih.biWidth;
          endHeight = largerBMP.fih.biHeight / 2;
          break;
        case 2:
          startWidth = 0;
          startHeight = largerBMP.fih.biHeight / 2;
          endWidth = largerBMP.fih.biWidth / 2;
          endHeight = largerBMP.fih.biHeight;
          break;
        case 3:
          startWidth = largerBMP.fih.biWidth / 2;
          startHeight = largerBMP.fih.biHeight / 2;
          endWidth = largerBMP.fih.biWidth;
          endHeight = largerBMP.fih.biHeight;
          break;
      }

      for (int y = startHeight; y < endHeight; y++) {
        for (int x = startWidth; x < endWidth; x++) {
          int smallX = x * widthRatio;
          int smallY = y * heightRatio;

          unsigned char red1 = getColor(largerBMP.data, largerBMP.fih.biWidth, x, y, 2);
          unsigned char red2 = getColor(smallerBMP.data, smallerBMP.fih.biWidth, smallX, smallY, 2);
          unsigned char redResult = (ratio * red1) + ((1 - ratio) * red2);
          data[(x * 3) + (y * realWidth) + 2] = redResult;

          unsigned char green1 = getColor(largerBMP.data, largerBMP.fih.biWidth, x, y, 1);
          unsigned char green2 = getColor(smallerBMP.data, smallerBMP.fih.biWidth, smallX, smallY, 1);
          unsigned char greenResult = (ratio * green1) + ((1 - ratio) * green2);
          data[(x * 3) + (y * realWidth) + 1] = greenResult;

          unsigned char blue1 = getColor(largerBMP.data, largerBMP.fih.biWidth, x, y, 0);
          unsigned char blue2 = getColor(smallerBMP.data, smallerBMP.fih.biWidth, smallX, smallY, 0);
          unsigned char blueResult = (ratio * blue1) + ((1 - ratio) * blue2);
          data[(x * 3) + (y * realWidth) + 0] = blueResult;
        }
      }
      exit(0);
    } else {
      childPids[ii] = p;
      times[ii] = clock();
    }
  }

  int stillWaiting;
  do {
    stillWaiting = 0;
    for (int ii = 0; ii < numberOfChildren; ++ii) {
      if (childPids[ii] > 0) {
        if (waitpid(childPids[ii], NULL, WNOHANG) != 0) {
          /* Child is done */
          clock_t end = clock();
          double cpu_time_used = ((double)(end - times[ii]))/CLOCKS_PER_SEC;
          printf("\nCPU Time Used: %f seconds\n", cpu_time_used);
          childPids[ii] = 0;
        } else {
          stillWaiting = 1;
        }
      }
      sleep(0);
    }
  } while (stillWaiting);

  clock_t end = clock();
  double cpu_time_used = ((double)(end - startTime))/CLOCKS_PER_SEC;
  printf("\nTotal CPU Time Used: %f seconds\n", cpu_time_used);

  free(childPids);

  FILE *file;
  file = fopen("output.bmp", "wb+");
  fwrite(&largerBMP.fh.bfType, 1, 2, file);
  fwrite(&largerBMP.fh.bfSize, 1, 4, file);
  fwrite(&largerBMP.fh.bfReserved1, 1, 2, file);
  fwrite(&largerBMP.fh.bfReserved2, 1, 2, file);
  fwrite(&largerBMP.fh.bfOffBits, 1, 4, file);
  fwrite(&largerBMP.fih, 1, sizeof(tagBITMAPINFOHEADER), file);
  fwrite(data, 1, largerBMP.fih.biSizeImage, file);
  fclose(file);
  
  munmap(data, largerBMP.fih.biSizeImage);
  return 0;
}
