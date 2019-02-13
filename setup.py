from distutils.core import setup, Extension

accimage = Extension(
    "accimage",
    include_dirs=["/usr/local/opt/jpeg-turbo/include", "/opt/intel/ipp/include"],
    libraries=["jpeg", "ippi", "ipps"],
    library_dirs=["/usr/local/opt/jpeg-turbo/lib", "/opt/intel/ipp/lib"],
    sources=["accimagemodule.c", "jpegloader.c", "imageops.c"],
)

setup(
    name="accimage",
    version="0.1.1",
    description="Accelerated image loader and preprocessor for Torch",
    author="Marat Dukhan",
    author_email="maratek@gmail.com",
    ext_modules=[accimage],
)
