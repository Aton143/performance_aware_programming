@echo off

IF NOT EXIST ..\build mkdir ..\build

pushd ..\build

cl ..\src\haversine.c /TC /EHa- /nologo /WX /FAcs /Fahaversine.asm /Fm /fp:except- /GS- /guard:cf- /Gy- /Wall /Zi /DEBUG:FULL /link /incremental:no /subsystem:console 

popd
