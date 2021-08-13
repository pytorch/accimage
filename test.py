import accimage
import numpy as np
import imageio
import os

ACCIMAGE_SAVE = os.environ.get('ACCIMAGE_SAVE', '')
if len(ACCIMAGE_SAVE) and ACCIMAGE_SAVE.lower() not in {'0', 'false', 'no'}:
    SAVE_IMAGES = True
else:
    SAVE_IMAGES = False

def image_to_np(image):
    """
    Returns:
        np.ndarray: Image converted to array with shape (width, height, channels)
    """
    image_np = np.empty([image.channels, image.height, image.width], dtype=np.uint8)
    image.copyto(image_np)
    image_np = np.transpose(image_np, (1, 2, 0))
    return image_np


def save_image(path, image):
    imageio.imwrite(path, image_to_np(image))


def test_reading_image():
    image = accimage.Image("chicago.jpg")
    if SAVE_IMAGES:
        save_image('test_reading_image.jpg', image)
    assert image.width == 1920
    assert image.height == 931


def test_reading_image_from_memory():
    from_file = accimage.Image("chicago.jpg")
    bytes = open("chicago.jpg", "rb").read()
    from_bytes = accimage.Image(bytes)
    if SAVE_IMAGES:
        save_image('test_reading_image_from_memory.jpg', from_bytes)
    assert from_bytes.width == 1920
    assert from_bytes.height == 931
    np.testing.assert_array_equal(image_to_np(from_file), image_to_np(from_bytes))


def test_resizing():
    image = accimage.Image("chicago.jpg")

    image.resize(size=(200, 200))
    if SAVE_IMAGES:
        save_image('test_resizing.jpg', image)

    assert image.width == 200
    assert image.height == 200

def test_cropping():
    image = accimage.Image("chicago.jpg")

    image.crop(box=(50, 50, 150, 150))
    if SAVE_IMAGES:
        save_image('test_cropping.jpg', image)

    assert image.width == 100
    assert image.height == 100

def test_flipping():
    image = accimage.Image("chicago.jpg")
    original_image_np = image_to_np(image)

    FLIP_LEFT_RIGHT = 0
    image.transpose(FLIP_LEFT_RIGHT)
    if SAVE_IMAGES:
        save_image('test_flipping.jpg', image)

    new_image_np = image_to_np(image)
    assert image.width == 1920
    assert image.height == 931
    np.testing.assert_array_equal(new_image_np[:, ::-1, :], original_image_np)
