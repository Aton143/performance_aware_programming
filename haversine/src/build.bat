@echo off

IF NOT EXIST ..\build mkdir ..\build

pushd ..\build

cl ..\src\haversine.cpp /D_CRT_SECURE_NO_WARNINGS /O2 /EHa- /nologo /wd4711 /wd4710 /WX /FAcs /Fahaversine.asm /Fm /Fe: haversine.exe /fp:except- /fp:fast /GS- /guard:cf- /Gy- /Wall /Zi /DEBUG:FULL /link /incremental:no /subsystem:console /opt:ref

popd

