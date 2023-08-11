#!/bin/bash 2>nul || goto :windows

sh ../MAKE.bat demos

exit

:windows

pushd  ..
call MAKE.bat demos %*
popd
