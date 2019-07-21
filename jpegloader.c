#include "accimage.h"

#include <setjmp.h>

#include <jpeglib.h>
#ifndef LIBJPEG_TURBO_VERSION
    #error libjpeg-turbo required (not IJG libjpeg)
#endif


struct accimage_jpeg_error_mgr {
    struct jpeg_error_mgr pub;
    jmp_buf setjmp_buffer;
};


static void accimage_jpeg_error_exit(j_common_ptr cinfo) {
    struct accimage_jpeg_error_mgr* accimage_err = (struct accimage_jpeg_error_mgr*) cinfo->err;
    longjmp(accimage_err->setjmp_buffer, 1);
}

void _image_from_jpeg(ImageObject* self, unsigned int input_type, const char* input, unsigned long length) {
    /*
    input_type == 0: input is a `path` string to the jpeg file (example: "foo/a.jpg"
        - in this case, the `length` argument is unused
    input_type == 1: input is a pointer to file contents loaded into memory
        - in this case, `length` is the length of the contents in memory in bytes
    */
    struct jpeg_decompress_struct state = { 0 };
    struct accimage_jpeg_error_mgr jpeg_error;
    FILE* file = NULL;
    unsigned char* buffer = NULL;

    if (input_type == 0) {
      file = fopen(input, "rb");
      if (file == NULL) {
        PyErr_Format(PyExc_IOError, "failed to open file %s", input);
        goto cleanup;
      }
    }

    state.err = jpeg_std_error(&jpeg_error.pub);
    jpeg_error.pub.error_exit = accimage_jpeg_error_exit;
    if (setjmp(jpeg_error.setjmp_buffer)) {
        char error_message[JMSG_LENGTH_MAX];
        (*state.err->format_message)((j_common_ptr) &state, error_message);
	if (input_type == 0) {
	  PyErr_Format(PyExc_IOError, "JPEG decoding failed: %s in file %s",
		       error_message, input);
	} else { /* input_type == 1 */
	  PyErr_Format(PyExc_IOError, "JPEG decoding failed: %s on buffer",
		       error_message);
	}
        goto cleanup;
    }

    jpeg_create_decompress(&state);
    if (input_type == 0) {
      jpeg_stdio_src(&state, file);
    } else { /* input_type == 1 */
      jpeg_mem_src(&state, (unsigned char*) input, length);
    }
    jpeg_read_header(&state, TRUE);

    state.dct_method = JDCT_ISLOW;
    state.output_components = 3;
    state.out_color_components = 3;
    state.out_color_space = JCS_RGB;

    jpeg_start_decompress(&state);

    buffer = malloc(state.output_components * state.output_width * state.output_height);
    if (buffer == NULL) {
        PyErr_NoMemory();
        goto cleanup;
    }

    while (state.output_scanline < state.output_height) {
        unsigned char* row = buffer + state.output_scanline * state.output_width * state.output_components;
        jpeg_read_scanlines(&state, &row, 1);
    }

    jpeg_finish_decompress(&state);

    /* Success. Commit object state */
    self->buffer = buffer;
    buffer = NULL;

    self->channels = 3;
    self->height = state.output_height;
    self->width = state.output_width;
    self->row_stride = state.output_width;
    self->y_offset = 0;
    self->x_offset = 0;

cleanup:
    jpeg_destroy_decompress(&state);

    free(buffer);

    if (input_type == 0) {
      if (file != NULL) {
        fclose(file);
      }
    }
}


void image_from_jpeg_path(ImageObject* self, const char* path) {
  _image_from_jpeg(self, 0, path, 0);
}

void image_from_jpeg_memory(ImageObject* self, const char* inmem, unsigned long inmem_size) {
  _image_from_jpeg(self, 1, inmem, inmem_size);
}
