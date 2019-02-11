import accimage
import numpy as np
import imageio
import os

ACCIMAGE_SAVE = os.environ.get('ACCIMAGE_SAVE', '')
if len(ACCIMAGE_SAVE) and ACCIMAGE_SAVE.lower() not in {'0', 'false', 'no'}:
    SAVE_IMAGES = True
else:
    SAVE_IMAGES = False


def save_image(path, image):
    image_np = np.empty([image.channels, image.height, image.width], dtype=np.uint8)
    image.copyto(image_np)
    image_np = np.transpose(image_np, (1, 2, 0))
    imageio.imwrite(path, image_np)


def test_reading_image():
    image = accimage.Image("chicago.jpg")
    assert image.width == 1920
    assert image.height == 931
    if SAVE_IMAGES:
        save_image('test_reading_image.jpg', image)


def test_resizing():
    image = accimage.Image("chicago.jpg")

    image.resize(size=(200, 200))

    assert image.width == 200
    assert image.height == 200
    if SAVE_IMAGES:
        save_image('test_resizing.jpg', image)

def test_cropping():
    image = accimage.Image("chicago.jpg")

    image.crop(box=(50, 50, 150, 150))

    assert image.width == 100
    assert image.height == 100
    if SAVE_IMAGES:
        save_image('test_cropping.jpg', image)
