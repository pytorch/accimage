#ifndef ACCIMAGE_H
#define ACCIMAGE_H

#include <Python.h>

typedef struct {
    PyObject_HEAD
    unsigned char* buffer;
    int channels;
    int height;
    int width;
    int row_stride;
    int y_offset;
    int x_offset;
} ImageObject;

void image_copy_deinterleave(ImageObject* self, unsigned char* output_buffer);
void image_copy_deinterleave_float(ImageObject* self, float* output_buffer);
void image_from_buffer(ImageObject* self, void* buf, size_t size);
void image_from_jpeg(ImageObject* self, const char* path);
void image_resize(ImageObject* self, int new_height, int new_width, int antialiasing);
void image_flip_left_right(ImageObject* self);

#endif
