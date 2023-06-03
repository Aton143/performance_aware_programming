@echo off

IF NOT EXIST ..\build mkdir ..\build

pushd ..\build

cl ..\src\haversine.c /O2 /EHa- /nologo /WX /FAcs /Fahaversine.asm /Fm /fp:except- /fp:fast /GS- /guard:cf- /Gy- /Wall /Zi /DEBUG:FULL /link /incremental:no /subsystem:console /opt:ref

popd
