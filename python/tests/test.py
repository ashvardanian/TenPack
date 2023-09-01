import os
import pathlib
import numpy as np

import cv2
import wave
import imageio
from PIL import Image

import tenpack_module

relative_path = '~/samples'


def get_paths() -> list:
    samples_path = os.path.expanduser(relative_path)
    return [os.path.join(samples_path, file) for file in os.listdir(samples_path)]


def export(tenpack, buffer):
    if tenpack.format == tenpack_module.format.png:
        image = Image.fromarray(buffer)
        image.save('output.png', 'PNG')
    elif tenpack.format == tenpack_module.format.jpeg:
        if tenpack.dims.channels == 4:
            raise Exception('Unsupported!')
        else:
            imageio.imwrite('output.jpg', buffer)
    elif tenpack.format == tenpack_module.format.gif:
        imageio.mimsave('output.gif', buffer, format='GIF', loop=0, duration=0.01)
    elif tenpack.format == tenpack_module.format.wav:
        with wave.open('output.wav', 'w') as wav_file:
            wav_file.setnchannels(tenpack.dims.channels)
            wav_file.setsampwidth(tenpack.dims.bytes_per_channel)
            wav_file.setframerate(tenpack.dims.height)
            wav_file.writeframes(buffer.tobytes())
    else:
        return False
    return True


def open_and_reshape(lhs, rhs, tenpack):
    image1 = imageio.v2.imread(lhs)
    image2 = imageio.v2.imread(rhs)
    return [
        image1.reshape(tenpack.dims.height, tenpack.dims.width, tenpack.dims.channels),
        image2.reshape(tenpack.dims.height, tenpack.dims.width, tenpack.dims.channels),
    ]


def compare_image_content(lhs, rhs, tenpack):
    if tenpack.format == tenpack_module.format.png:
        [image1, image2] = open_and_reshape(lhs, rhs, tenpack)
        assert np.array_equal(image1, image2)
    elif tenpack.format == tenpack_module.format.jpeg:
        [image1, image2] = open_and_reshape(lhs, rhs, tenpack)
        tolerance = 80
        assert np.all(np.isclose(image1, image2, tolerance, tolerance))
    elif tenpack.format == tenpack_module.format.wav:
        with wave.open(lhs, 'rb') as wav1, wave.open(rhs, 'rb') as wav2:
            frames1 = wav1.readframes(wav1.getnframes())
            frames2 = wav2.readframes(wav2.getnframes())
            assert frames1 == frames2
    elif tenpack.format == tenpack_module.format.gif:
        gif1 = imageio.mimread(lhs)
        gif2 = imageio.mimread(rhs)
        assert len(gif1) == len(gif2)

        hist_diffs = []
        for frame_num in range(len(gif1)):
            image1 = gif1[frame_num]
            image2 = gif2[frame_num]

            if image1.shape[-1] == 3:
                image1 = cv2.cvtColor(image1, cv2.COLOR_RGB2GRAY)
            if image2.shape[-1] == 3:
                image2 = cv2.cvtColor(image2, cv2.COLOR_RGB2GRAY)

            hist1 = cv2.calcHist([image1], [0], None, [256], [0, 256])
            hist2 = cv2.calcHist([image2], [0], None, [256], [0, 256])

            hist_diff = cv2.compareHist(hist1, hist2, cv2.HISTCMP_CORREL)
            hist_diffs.append(hist_diff)

        avg_hist_diff = sum(hist_diffs) / len(hist_diffs)
        assert avg_hist_diff == 1.0


def test_equality():
    tenpack = tenpack_module.tenpack()
    paths = get_paths()

    for path in paths:
        output_file = 'output' + pathlib.Path(path).suffix
        with open(path, 'rb') as file:
            file_data = bytes(file.read())
        output_data = np.array(tenpack.unpack(file_data))
        assert len(output_data) != 0
        assert export(tenpack, output_data) == True
        compare_image_content(path, output_file, tenpack)
        os.remove(output_file)


def test_equality_many(threads=1):
    tenpack = tenpack_module.tenpack()
    paths = get_paths()
    pack_tuples, array_tuples = tenpack.unpack_many(paths, 1)
    for i in range(len(paths)):
        output_file = 'output' + pathlib.Path(paths[i]).suffix
        assert export(pack_tuples[i], np.array(array_tuples[i])) == True
        compare_image_content(paths[i], output_file, tenpack)
        os.remove(output_file)


if __name__ == '__main__':
    test_equality()
    test_equality_many()
    test_equality_many(2)
    test_equality_many(4)
    test_equality_many(8)
