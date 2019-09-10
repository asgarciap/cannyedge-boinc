// cannyedge.cpp : Este archivo contiene la función "main". La ejecución del programa comienza y termina ahí.
//
#include <iostream>
#include "contrib/bmp.h"
#include "contrib/CannyEdgeDetector.h"
#include "boinc_api.h"
#include "util.h"

//do 5 extra minutes of cpu computing to set 
//the total computing job time bigger
#define CPU_TIME 600

// do about .5 seconds of computing
// (note: I needed to add an arg to this;
// otherwise the MS C++ compiler optimizes away
// all but the first call to it!)
//
static double do_some_computing(int foo) {
    double x = 3.14159*foo;
    int i;
    for (i=0; i<50000000; i++) {
        x += 5.12313123;
        x *= 0.5398394834;
    }
    return x;
}

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

    //burn up some CPU time
    double start = dtime();
    for(int i=0; ; i++) {
        double e = dtime() - start;
        if (e > CPU_TIME) break;
        double fd = .5 + .5*(e/CPU_TIME);
        boinc_fraction_done(fd);
        double comp_result = do_some_computing(i);
    }
    boinc_fraction_done(1);
    boinc_finish(0);
}
