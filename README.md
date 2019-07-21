# accimage

[![Build status](https://travis-ci.org/pytorch/accimage.svg?branch=master)](https://travis-ci.org/pytorch/accimage)
[![Anaconda badge](https://anaconda.org/conda-forge/accimage/badges/version.svg)](https://anaconda.org/conda-forge/accimage)


An accelerated Image loader and preprocessor leveraging [Intel
IPP](https://software.intel.com/en-us/intel-ipp).

accimage mimics the PIL API and can be used as a backend for
[`torchvision`](https://github.com/pytorch/vision).

Operations implemented:

- `Image(path)` - creates an `accimage.Image` from a path to a jpeg file
   - For example: `img = accimage.Image('chicago.jpg')`
- `Image(bytes)` - creates an `accimage.Image` from the bytes of a jpeg file loaded into memory
   - For example: `f = open('chicago.jpg', 'rb'); b = f.read(); img = accimage.Image(b)`
- `Image.resize((width, height))`
- `Image.crop((left, upper, right, lower))`
- `Image.transpose(PIL.Image.FLIP_LEFT_RIGHT)`

Enable the torchvision accimage backend with:

```python
torchvision.set_image_backend('accimage')
```

## Installation

accimage is available on conda-forge, simply run the following to install

```
$ conda install -c conda-forge accimage
```
