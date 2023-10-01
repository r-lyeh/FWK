import os
from fwk import fwk

## easiest python debugging is to install vscode, then add python addon, and launch it from there
## the other way around, is to go with lldb/gdb, or `python -Xfrozen_modules=off -m debugpy --connect 5678 -m hello.py`
# import debugpy ## launched like
# debugpy.listen(5678)
# debugpy.wait_for_client()
# debugpy.breakpoint()

fwk.cook_config(b'../../tools/cook.ini')

fwk.window_create(75.0, fwk.WINDOW_MSAA2)
fwk.window_title(b'hello Python')
cam = fwk.camera()
while fwk.window_swap():
    fwk.ddraw_grid(0)

os._exit(0)