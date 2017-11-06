from distutils.core import setup, Extension
import os
from os.path import expanduser, exists, join, realpath, basename, dirname


def find_path(candidates):
    for path in candidates:
        if exists(path):
            return path


def find_path_with_members(candidates, required_names_we):
    """
    Args:
        candidates (list): possible directories
        required_names_we (list) : candidate directory must contain
            filenames in this list (extensions are ignored)
    """
    for path in candidates:
        if exists(path):
            members_we = [basename(p).split('.')[0] for p in os.listdir(path)]
            if all(want in members_we for want in required_names_we):
                return path


ipp_root = find_path([
    expanduser('~/intel/ipp'),
    expanduser('/opt/intel/ipp'),
])
if ipp_root is None:
    raise Exception('Cannot find path to Intel IPP')
else:
    ipp_lib_dir = find_path_with_members(
        candidates=[
            join(ipp_root, 'lib'),
            # join(ipp_root, 'lib', 'ia32'),
            join(ipp_root, 'lib', 'intel64'),
        ],
        required_names_we=['libippi', 'libipps']
    )
    if ipp_lib_dir is None:
        raise Exception('Cannot find path to Intel IPP')

    # Ensure that the image and signal processing lib are in the libdir
    ipp = {
        'include_dir': join(ipp_root, 'include'),
        'lib_dir': ipp_lib_dir,
    }

    join(ipp_root, 'lib', 'ia32', 'libippi')


jpeg_turbo_root = '/usr/local/opt/jpeg-turbo'
if exists(jpeg_turbo_root):
    jpeg_turbo = {
        'lib_dir':  join('/usr/local/opt/jpeg-turbo', 'lib'),
        'include_dir': join('/usr/local/opt/jpeg-turbo', 'include'),
    }
else:
    jpeg_turbo_header = find_path([
        '/usr/include/jpeglib.h'
    ])
    jpeg_turbo_lib = find_path([
        '/usr/lib/x86_64-linux-gnu/libjpeg.so',
        '/usr/lib/i386-linux-gnu/libjpeg.so',
    ])

    # We can use the system libjpeg if its version is at least 8
    jpeg_version_info = basename(realpath(jpeg_turbo_lib)).split('.')[2:]
    jpeg_version_major = int(jpeg_version_info[0])
    if jpeg_version_major < 8:
        raise Exception('Cannot find LibJpegTurbo')

    if jpeg_turbo_header is None or jpeg_turbo_lib is None:
        raise Exception('Cannot find LibJpegTurbo')

    jpeg_turbo = {
        'lib_dir':  dirname(jpeg_turbo_lib),
        'include_dir': dirname(jpeg_turbo_header),
    }

accimage = Extension(
    'accimage',
    include_dirs=[
        jpeg_turbo['include_dir'],
        ipp['include_dir']
    ],
    libraries=['jpeg', 'ippi', 'ipps'],
    library_dirs=[
        jpeg_turbo['lib_dir'],
        ipp['lib_dir']
    ],
    sources=[
        'accimagemodule.c',
        'jpegloader.c',
        'imageops.c'
    ])

setup(name='accimage',
      version='0.1',
      description='Accelerated image loader and preprocessor for Torch',
      author='Marat Dukhan',
      author_email='maratek@gmail.com',
      ext_modules=[accimage])
