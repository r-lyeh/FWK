import os
from fwk import fwk

fwk.window_create(75.0, fwk.WINDOW_MSAA2)
fwk.window_title(b'hello Python')
cam = fwk.camera()
while fwk.window_swap():
    fwk.ddraw_grid(0)

os._exit(0)