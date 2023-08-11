cd /d "%~dp0\..\.."
call make dll
call make dll nofwk demos\physics\demo_car.cpp
call make dll nofwk demos\physics\demo_dom.cpp
call make dll nofwk demos\physics\demo_pool.cpp
call make dll nofwk demos\physics\demo_rag.cpp
