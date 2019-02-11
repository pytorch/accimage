#include "accimage.h"

#include <stdlib.h>

#include <ippi.h>


void image_copy_deinterleave(ImageObject* self, unsigned char* output_buffer) {
    unsigned char* channel_buffers[3] = {
        output_buffer,
        output_buffer + self->height * self->width,
        output_buffer + 2 * self->height * self->width
    };
    IppiSize roi = { self->width, self->height };
    IppStatus ipp_status = ippiCopy_8u_C3P3R(
        self->buffer + (self->y_offset * self->row_stride + self->x_offset) * self->channels,
        self->row_stride * self->channels,
        channel_buffers, self->width, roi);
    if (ipp_status != ippStsNoErr) {
        PyErr_Format(PyExc_SystemError, "ippiCopy_8u_C3P3R failed with status %d", ipp_status);
    }
}


void image_copy_deinterleave_float(ImageObject* self, float* output_buffer) {
    unsigned char* tmp_buffer = NULL;
    IppiSize roi = { self->width, self->height };

    tmp_buffer = malloc(self->height * self->width * self->channels);
    if (!tmp_buffer) {
        PyErr_NoMemory();
        goto cleanup;
    }

    image_copy_deinterleave(self, tmp_buffer);
    if (PyErr_Occurred()) {
      goto cleanup;
    }

    IppStatus ipp_status = ippiConvert_8u32f_C3R(
        tmp_buffer, self->width * self->channels,
        output_buffer, self->width * self->channels * sizeof(float),
        roi);
    if (ipp_status != ippStsNoErr) {
        PyErr_Format(PyExc_SystemError, "ippiConvert_8u32f_C3R failed with status %d", ipp_status);
    }

    Ipp32f value[3] = {255.0f, 255.0f, 255.0f};
    ipp_status = ippiDivC_32f_C3IR(
        value, output_buffer, self->width * self->channels * sizeof(float),
        roi);
    if (ipp_status != ippStsNoErr) {
        PyErr_Format(PyExc_SystemError, "ippiDivC_32f_C3IR failed with status %d", ipp_status);
    }

cleanup:
    free(tmp_buffer);
}


void image_resize(ImageObject* self, int new_height, int new_width, int antialiasing) {
    IppStatus ipp_status;
    unsigned char* new_buffer = NULL;
    IppiSize old_size = { self->width, self->height };
    IppiSize new_size = { new_width, new_height };
    IppiPoint new_offset = { 0, 0 };
    int specification_size = 0, initialization_buffer_size = 0, scratch_buffer_size = 0;
    IppiResizeSpec_32f* specification = NULL;
    Ipp8u* scratch_buffer = NULL;
    Ipp8u* initialization_buffer = NULL;

    new_buffer = malloc(new_height * new_width * self->channels);
    if (new_buffer == NULL) {
        PyErr_NoMemory();
        goto cleanup;
    }

    ipp_status = ippiResizeGetSize_8u(old_size, new_size, ippLinear, antialiasing,
        &specification_size, &initialization_buffer_size);
    if (ipp_status != ippStsNoErr) {
        PyErr_Format(PyExc_SystemError,
            "ippiResizeGetSize_8u failed with status %d", ipp_status);
        goto cleanup;
    }

    initialization_buffer = malloc(initialization_buffer_size);
    if (initialization_buffer == NULL) {
        PyErr_NoMemory();
        goto cleanup;
    }

    specification = malloc(specification_size);
    if (specification == NULL) {
        PyErr_NoMemory();
        goto cleanup;
    }

    if (antialiasing) {
      ipp_status = ippiResizeAntialiasingLinearInit(
          old_size, new_size, specification, initialization_buffer);
    } else {
      ipp_status = ippiResizeLinearInit_8u(old_size, new_size, specification);
    }
    if (ipp_status != ippStsNoErr) {
        PyErr_Format(PyExc_SystemError,
            "ippiResizeLinearInit_8u failed with status %d", ipp_status);
        goto cleanup;
    }

    ipp_status = ippiResizeGetBufferSize_8u(specification, new_size, self->channels, &scratch_buffer_size);
    if (ipp_status != ippStsNoErr) {
        PyErr_Format(PyExc_SystemError,
            "ippiResizeGetBufferSize_8u failed with status %d", ipp_status);
        goto cleanup;
    }

    scratch_buffer = malloc(scratch_buffer_size);
    if (scratch_buffer == NULL) {
        PyErr_NoMemory();
        goto cleanup;
    }

    if (antialiasing) {
      ipp_status = ippiResizeAntialiasing_8u_C3R(
          self->buffer + (self->y_offset * self->row_stride + self->x_offset) * self->channels,
          self->row_stride * self->channels,
          new_buffer, new_width * self->channels, new_offset, new_size,
          ippBorderRepl, NULL, specification, scratch_buffer);
    } else {
      ipp_status = ippiResizeLinear_8u_C3R(
          self->buffer + (self->y_offset * self->row_stride + self->x_offset) * self->channels,
          self->row_stride * self->channels,
          new_buffer, new_width * self->channels, new_offset, new_size,
          ippBorderRepl, NULL, specification, scratch_buffer);
    }
    if (ipp_status != ippStsNoErr) {
        PyErr_Format(PyExc_SystemError,
            "ippiResizeLinear_8u_C3R failed with status %d", ipp_status);
        goto cleanup;
    }

    free(self->buffer);
    self->buffer = new_buffer;
    new_buffer = NULL;

    self->height = new_height;
    self->width = new_width;
    self->row_stride = new_width;
    self->x_offset = 0;
    self->y_offset = 0;

cleanup:
    free(new_buffer);
    free(specification);
    free(initialization_buffer);
    free(scratch_buffer);
}


void image_flip_left_right(ImageObject* self) {
    IppiSize roi = { self->width, self->height };
    IppStatus ipp_status = ippiMirror_8u_C3IR(
        self->buffer + (self->y_offset * self->row_stride + self->x_offset) * self->channels,
        self->row_stride * self->channels, 
        roi, ippAxsVertical);
    if (ipp_status != ippStsNoErr)
        PyErr_Format(PyExc_SystemError, "ippiMirror_8u_C3IR failed with status %d", ipp_status);
}
