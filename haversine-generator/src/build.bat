@echo off

IF NOT EXIST ..\build mkdir ..\build

pushd ..\build

cl ..\src\haversine-json.cpp /O2 /EHa- /nologo /wd4711 /WX /FAcs /Fahaversine-generator.asm /Fm /Fe: haversine-generator.exe /fp:except- /fp:fast /GS- /guard:cf- /Gy- /Wall /Zi /DEBUG:FULL /link /incremental:no /subsystem:console /opt:ref

popd
