# accimage

[![Build status](https://travis-ci.org/pytorch/accimage.svg?branch=master)](https://travis-ci.org/pytorch/accimage)

An accelerated Image loader and preprocessor leveraging [Intel
IPP](https://software.intel.com/en-us/intel-ipp).

accimage mimics the PIL API and can be used as a backend for
[`torchvision`](https://github.com/pytorch/vision).

Operations implemented:

- `Image.resize((width, height))`
- `Image.crop((left, upper, right, lower))`
- `Image.transpose(PIL.Image.FLIP_LEFT_RIGHT)`

Enable the torchvision accimage backend with:

```python
torchvision.set_image_backend('accimage')
```
