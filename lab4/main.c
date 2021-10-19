#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>

typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef unsigned int LONG;
struct tagBITMAPFILEHEADER
{
    WORD bfType;  //specifies the file type
    DWORD bfSize;  //specifies the size in bytes of the bitmap file
    WORD bfReserved1;  //reserved; must be 0
    WORD bfReserved2;  //reserved;must be 0
    DWORD bfOffBits;  //species the offset in bytes from the bitmapfileheader to the bitmap bits
};
struct tagBITMAPINFOHEADER
{
    DWORD biSize;  //specifies the number of bytes required by the struct
    LONG biWidth;  //specifies width in pixels
    LONG biHeight;  //species height in pixels
    WORD biPlanes; //specifies the number of color planes, must be 1
    WORD biBitCount; //specifies the number of bit per pixel
    DWORD biCompression;//spcifies the type of compression
    DWORD biSizeImage;  //size of image in bytes
    LONG biXPelsPerMeter;  //number of pixels er meter in x axis
    LONG biYPelsPerMeter;  //number of pixels per meter in y axis
    DWORD biClrUsed;  //number of colors used by the bitmap
    DWORD biClrImportant;  //number of colors that are important
};
unsigned char getColor(unsigned char *data, int width, int x, int y, int color)
{
    int bplo =  width * 3;
    if (bplo % 4 != 0)
    {
        bplo = bplo + 4 - (bplo % 4);
    }
    bplo = bplo*y;
    return data[x*3 + bplo + color];
}

int readFile(char *file1, float brightness, char *out, int parallel)
{
    FILE *file = fopen(file1, "rb");

    struct tagBITMAPFILEHEADER fh;
    struct tagBITMAPINFOHEADER fih;
    fread(&fh.bfType, 1, 2, file);
    fread(&fh.bfSize, 1, 4, file);
    fread(&fh.bfReserved1, 1, 2, file);
    fread(&fh.bfReserved2, 1, 2, file);
    fread(&fh.bfOffBits, 1, 4, file);
    fread(&fih, 1, sizeof(struct tagBITMAPINFOHEADER), file);
    unsigned char *data = mmap(NULL, fih.biSizeImage, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    fread(data, 1, fih.biSizeImage, file);
    fclose(file);

    DWORD biSizeImagef;
    int height, width;
    biSizeImagef = fih.biSizeImage;
    width = fih.biWidth;
    height = fih.biHeight;

    fflush(0);
    int g;

    time_t tstart, tstop;
    clock_t start, end;
    double cpu_time_used;
    start = clock();
    sleep(1);

    if (parallel == 1)
    {

    if (fork() == 0)
    {
        for (int y = 0; y < height/2; y++)
        {
            for (int x = 0; x < width; x++)
            {
                float ratioX;
                float ratioY;
                float dx;
                float dy;
                int bplo = width * 3;
                if (bplo % 4 != 0)
                {
                    bplo = bplo + 4 - (bplo % 4);
                }
                bplo = bplo*y;
                unsigned char red1 = getColor(data, fih.biWidth, x, y, 2);
                int red = (int)red1 + (255 * brightness);
                if (red > 255)
                {
                    red = 255;
                }
                red1 = (unsigned char)red;
                data[x*3 + bplo + 2] = red1;

                unsigned char green1 = getColor(data, fih.biWidth, x, y, 1);
                int green = (int)green1 + (255 * brightness);
                if (green > 255)
                {
                    green = 255;
                }
                green1 = (unsigned char)green;
                data[x*3 + bplo + 1] = green1;

                unsigned char blue1 = getColor(data, fih.biWidth, x, y, 0);
                int blue = (int)blue1 + (255 * brightness);
                if (blue > 255)
                {
                    blue = 255;
                }
                blue1 = (unsigned char)blue;
                data[x*3 + bplo + 0] = blue1;
            }
            
        }
        return 0;
    }
    else
    {
        for (int y = height/2; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                float ratioX;
                float ratioY;
                float dx;
                float dy;
                int bplo = width * 3;
                if (bplo % 4 != 0)
                {
                    bplo = bplo + 4 - (bplo % 4);
                }
                bplo = bplo*y;
                unsigned char red1 = getColor(data, fih.biWidth, x, y, 2);
                int red = (int)red1 + (255 * brightness);
                if (red > 255)
                {
                    red = 255;
                }
                red1 = (unsigned char)red;
                data[x*3 + bplo + 2] = red1;

                unsigned char green1 = getColor(data, fih.biWidth, x, y, 1);
                int green = (int)green1 + (255 * brightness);
                if (green > 255)
                {
                    green = 255;
                }
                green1 = (unsigned char)green;
                data[x*3 + bplo + 1] = green1;

                unsigned char blue1 = getColor(data, fih.biWidth, x, y, 0);
                int blue = (int)blue1 + (255 * brightness);
                if (blue > 255)
                {
                    blue = 255;
                }
                blue1 = (unsigned char)blue;
                data[x*3 + bplo + 0] = blue1;
            }
            
        }
        
    }
        wait(&g);
    }
    else
    {
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                float ratioX;
                float ratioY;
                float dx;
                float dy;
                int bplo = width * 3;
                if (bplo % 4 != 0)
                {
                    bplo = bplo + 4 - (bplo % 4);
                }
                bplo = bplo*y;
                unsigned char red1 = getColor(data, fih.biWidth, x, y, 2);
                int red = (int)red1 + (255 * brightness);
                if (red > 255)
                {
                    red = 255;
                }
                red1 = (unsigned char)red;
                data[x*3 + bplo + 2] = red1;

                unsigned char green1 = getColor(data, fih.biWidth, x, y, 1);
                int green = (int)green1 + (255 * brightness);
                if (green > 255)
                {
                    green = 255;
                }
                green1 = (unsigned char)green;
                data[x*3 + bplo + 1] = green1;

                unsigned char blue1 = getColor(data, fih.biWidth, x, y, 0);
                int blue = (int)blue1 + (255 * brightness);
                if (blue > 255)
                {
                    blue = 255;
                }
                blue1 = (unsigned char)blue;
                data[x*3 + bplo + 0] = blue1;
            }
            
        }
        
    }
    end = clock();
    cpu_time_used = ((double)(end - start))/CLOCKS_PER_SEC;
    printf("\nCPU Time Used: %f seconds\n", cpu_time_used);
    
    file = fopen(out, "wb+");
    fwrite(&fh.bfType, 1, 2, file);
    fwrite(&fh.bfSize, 1, 4, file);
    fwrite(&fh.bfReserved1, 1, 2, file);
    fwrite(&fh.bfReserved2, 1, 2, file);
    fwrite(&fh.bfOffBits, 1, 4, file);
    fwrite(&fih, 1, sizeof(struct tagBITMAPINFOHEADER), file);
    fwrite(data, 1, fih.biSizeImage, file);
    fclose(file);
    
    munmap(data, fih.biSizeImage);
    return 0;
}

int main(int argc, char *argv[])
{
    char *imagefile = "lion.bmp";
    readFile(imagefile, atof(argv[1]), argv[3], atof(argv[2]));
    return 0;
}
