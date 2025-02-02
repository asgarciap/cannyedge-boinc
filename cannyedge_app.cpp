﻿// cannyedge.cpp : Este archivo contiene la función "main". La ejecución del programa comienza y termina ahí.
//
#include <iostream>
#include "contrib/bmp.h"
#include "contrib/CannyEdgeDetector.h"
#include "boinc_api.h"

int main(int argc, char **argv)
{
    char buf[256];
    char inbmp[1024];
    char outbmp[1024];

    if (argc < 3) {
        std::cerr << "Missing filename to process. Usage: cannyedgeapp input_filename output_filename" << std::endl;
        return 1;
    }
    int retval = boinc_init();
    if (retval) {
        fprintf(stderr, "%s boinc_init returned %d\n",
            boinc_msg_prefix(buf, sizeof(buf)), retval
        );
        exit(retval);
    }
    boinc_resolve_filename(argv[1], inbmp, sizeof(inbmp));
    boinc_resolve_filename(argv[2], outbmp, sizeof(inbmp));

	Bitmap* bm = bm_load(inbmp);
    int w, h;
    w = bm->w;
    h = bm->h;

	//remove alpha from image
	uint8_t* img = new uint8_t[w*h * 3];
	memset(img, 0, sizeof(img));
	uint64_t k = 0;
	for (uint64_t i = 0; i < bm->w*bm->h * 4; i+=4) {
		img[k++] = bm->data[i];
		img[k++] = bm->data[i + 1];
		img[k++] = bm->data[i + 2];
	}
    bm_free(bm);

	uint8_t *d = NULL;
	CannyEdgeDetector* canny = new CannyEdgeDetector();
	d = canny->ProcessImage(img, w, h,1.0f,30,30);
    delete canny;

	//restore alpha to save the new image
	uint8_t* imgfinal = new uint8_t[w*h*4];
	k = 0;
	for (uint64_t i = 0; i < w*h * 3; i+=3) {
		imgfinal[k++] = d[i];
		imgfinal[k++] = d[i+1];
		imgfinal[k++] = d[i+2];
		imgfinal[k++] = 0xFF;  //add alpha channel. no transparency
	}
    delete[] d;
	Bitmap* bm2 = bm_bind(w, h, imgfinal);
	bm_save(bm2, outbmp);
    bm_free(bm2);
    boinc_finish(0);
}