#define _DEFAULT_SOURCE

#include <assert.h>
#include <string.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include <Imlib2.h>
#include <liby4m.h>
#include <libyuv.h>

#include "imlib2/image.h"

// in the following code, "encoded" refers to YUV data (as present in a
// y4m file).
int load2(ImlibImage *im, int load_data) {
	// Load file.
	int encoded_fd = fileno(im->fp);
	struct stat st;
	if (encoded_fd < 0)
		return LOAD_FAIL;
	if (fstat(encoded_fd, &st) < 0) {
		close(encoded_fd);
		return LOAD_FAIL;
	}

	int rc = LOAD_FAIL;

	// read the input file pointer
	y4mFile_t y4m_file;
	if (y4mOpenFp(&y4m_file, im->fp)) {
		goto quit;
	}

  if (!load_data) {
    rc = LOAD_SUCCESS;
    goto quit;
  }

	// write output data
	im->w = y4mGetWidth(&y4m_file);
	im->h = y4mGetHeight(&y4m_file);
	if (!IMAGE_DIMENSIONS_OK(im->w, im->h))
		goto quit;
	//UNSET_FLAG(im->flags, F_HAS_ALPHA);
	SET_FLAG(im->flags, F_HAS_ALPHA);

	// convert to BGRA as expected by imlib
	uint8_t *data = (uint8_t *)malloc(4 * im->w * im->h);
	if (!data)
		goto quit;
  char* input_ptr = y4mGetFrameDataPointer(&y4m_file);

  // I420RGBA returns ARGB
  if (I420ToRGBA((const uint8_t*)input_ptr,  // src_y,
                 im->w,  // src_stride_y
                 (const uint8_t*)(input_ptr + im->w * im->h),  // src_u
                 im->w / 2,  // src_stride_u
                 (const uint8_t*)(input_ptr + im->w * im->h * 5 / 4),  // src_v
                 im->w / 2,  // src_stride_v
                 data,  // dst_bgra
                 im->w * 4,  // dst_stride_bgra
                 im->w,  // width
                 im->h) != 0) // height
		goto quit;
  // copy ARGB (returned by libyuv) to RGBA (expected by imlib2)
  for (int y = 0; y < im->h; y++) {
    for (int x = 0; x < im->w; x++) {
      uint8_t a = data[4*(y*im->w + x) + 0];
      uint8_t r = data[4*(y*im->w + x) + 1];
      uint8_t g = data[4*(y*im->w + x) + 2];
      uint8_t b = data[4*(y*im->w + x) + 3];
      data[4*(y*im->w + x) + 0] = r;
      data[4*(y*im->w + x) + 1] = g;
      data[4*(y*im->w + x) + 2] = b;
      data[4*(y*im->w + x) + 3] = a;
    }
  }
	im->data = (DATA32 *)data;

	if (im->lc)
		__imlib_LoadProgressRows(im, 0, im->h);
	rc = LOAD_SUCCESS;
quit:
	if (rc <= 0)
		__imlib_FreeData(im);
	close(encoded_fd);
	return rc;
}

char save(ImlibImage *im, ImlibProgressFunction progress, char progress_granularity) {
	(void)progress; (void)progress_granularity;

	int                       rc     = LOAD_FAIL;
	return rc;
}

void formats(ImlibLoader *l) {
	static const char  *const list_formats[] = {"y4m"};
	__imlib_LoaderSetFormats(l, list_formats, sizeof(list_formats) / sizeof(char *));
}
