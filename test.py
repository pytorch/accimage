import accimage
image = accimage.Image("chicago.jpg")
import numpy
buffer = numpy.empty([image.channels, image.height, image.width], dtype=numpy.uint8)
#buffer = bytearray(256)
image.copyto(buffer)
import scipy.misc
scipy.misc.imsave('chicago-back.png', numpy.transpose(buffer, (1, 2, 0)))

print(image.size)

image.resize(size=(200, 200))
buffer = numpy.empty([image.channels, image.height, image.width], dtype=numpy.uint8)
image.copyto(buffer)
import scipy.misc
scipy.misc.imsave('chicago-200x200.png', numpy.transpose(buffer, (1, 2, 0)))

image.crop(box=(50, 50, 150, 150))
buffer = numpy.empty([image.channels, image.height, image.width], dtype=numpy.uint8)
image.copyto(buffer)
import scipy.misc
scipy.misc.imsave('chicago-100x100.png', numpy.transpose(buffer, (1, 2, 0)))
