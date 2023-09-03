import faulthandler

faulthandler.enable()

import tenpack

png_image = None
with open("1983160.jpeg", "rb") as f:
    png_image = f.read()

print(f"{tenpack.format(png_image) = }")
print(f"{tenpack.shape(png_image) = }")

format, shape, tensor = tenpack.unpack(png_image)
print(f"{tensor.shape = }")
