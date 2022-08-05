# TenPack

Three simple things this library does to you:

1. Guess the media type from raw bytes,
2. Parse its' dimensions, sizes, lengths, etc.,
3. Unpack data into regular preallocated Tensors.

Where do we use it?
To connect the Data Storage layer of [UKV](github.com/unum-cloud/ukv) to High-Performance Computing libraries like [TensorFlow](tensorflow.org) and [PyTorch](pytorch.org).

## How it works?

Most common file-formats have "signatures" or "magic numbers" embedded into them.
Often, as the prefix of the byte-stream.

* [List of file signatures](https://en.wikipedia.org/wiki/List_of_file_signatures)
* [Magic numbers in programming](https://en.wikipedia.org/wiki/Magic_number_(programming)#Magic_numbers_in_files)

Libraries implementing the first step have been implemented for other languages:

* [filetype](https://github.com/h2non/filetype) for GoLang
* [filetype.py](https://github.com/h2non/filetype.py) for Python
* [FileType](https://github.com/rzane/file_type) for Elixir
* [FileSignatures](https://github.com/neilharvey/FileSignatures) for C#

## Alternatives for Tensor Exports

* [Pillow](https://pillow.readthedocs.io/en/stable/) and [Pillow-SIMD](https://github.com/uploadcare/pillow-simd) for [image formats](https://pillow.readthedocs.io/en/stable/handbook/image-file-formats.html).
* [FFmpeg](https://ffmpeg.org/), for video formats.
* [Nyquist](https://github.com/ddiakopoulos/libnyquist), for audio formats.

In fact, TenPack is just a CMake-friendly generalization of those libraries with a C interface and focus on memory reusing.
