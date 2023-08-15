import os
import pathlib
import numpy as np

import wave
import imageio
from PIL import Image
from skimage import io
from skimage.metrics import structural_similarity as ssim

import tenpack_module

relative_path = '~/samples'
global samples_path


def get_paths() -> list:
    samples_path = os.path.expanduser(relative_path)
    return [os.path.join(samples_path, file) for file in os.listdir(samples_path)]


def export(tenpack, buffer: list):
    if tenpack.format == tenpack_module.format.png:
        media = np.array(buffer, dtype=np.uint8)
        image = Image.fromarray(media)
        image.save('output.png', 'PNG')
    elif tenpack.format == tenpack_module.format.jpeg:
        if tenpack.dims.channels == 4:
            raise Exception('Unsupported!')
        else:
            media = np.array(buffer, dtype=np.uint8)
            media = media.reshape(tenpack.dims.height, tenpack.dims.width, 3)
            imageio.imwrite('output.jpg', media)
    elif tenpack.format == tenpack_module.format.gif:
        images = [np.array(tenpack.dims.frames, dtype=np.uint8) for tenpack.dims.frames in buffer]
        imageio.mimsave('output.gif', images)
    elif tenpack.format == tenpack_module.format.wav:
        media = np.array(buffer, dtype=np.int16)
        with wave.open('output.wav', 'w') as wav_file:
            wav_file.setnchannels(tenpack.dims.channels)
            wav_file.setsampwidth(tenpack.dims.bytes_per_channel)
            wav_file.setframerate(tenpack.dims.height)
            wav_file.writeframes(media.tobytes())
    else:
        return False
    return True


def open_and_reshape(lhs, rhs, tenpack):
    image1 = imageio.v2.imread(lhs)
    image2 = imageio.v2.imread(rhs)
    return [
        image1.reshape(tenpack.dims.height, tenpack.dims.width,
                       tenpack.dims.channels),
        image2.reshape(tenpack.dims.height, tenpack.dims.width,
                       tenpack.dims.channels),
    ]


def compare_image_content(lhs, rhs, tenpack):
    if tenpack.format == tenpack_module.format.png:
        [image1, image2] = open_and_reshape(lhs, rhs, tenpack)
        assert np.array_equal(image1, image2)
    elif tenpack.format == tenpack_module.format.jpeg:
        [image1, image2] = open_and_reshape(lhs, rhs, tenpack)
        assert np.array_equal(image1, image2)
    elif tenpack.format == tenpack_module.format.wav:
        with wave.open(lhs, 'rb') as wav1, wave.open(rhs, 'rb') as wav2:
            frames1 = wav1.readframes(wav1.getnframes())
            frames2 = wav2.readframes(wav2.getnframes())
            assert frames1 == frames2


def test_equality():
    tenpack = tenpack_module.tenpack()
    paths = get_paths()

    for path in paths:
        output_file = 'output' + pathlib.Path(path).suffix
        with open(path, 'rb') as file:
            file_data = list(file.read())
        assert tenpack.guess_format(file_data) == True
        assert tenpack.guess_dims(file_data) == True
        output_data = tenpack.unpack(file_data)
        assert len(output_data) != 0
        assert export(tenpack, output_data) == True
        compare_image_content(path, output_file, tenpack)
        os.remove(output_file)


if __name__ == '__main__':
    test_equality()
