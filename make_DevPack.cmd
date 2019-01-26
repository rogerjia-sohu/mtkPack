@echo off
setlocal
set Me.Path=%~dps0
set mtkPackDir=mtkPack
set DevPackDir=DevPack
set IncPath=%Me.Path%%DevPackDir%\%mtkPackDir%\include
set DebugLibPath=%Me.Path%%DevPackDir%\%mtkPackDir%\lib\debug
set ReleaseLibPath=%Me.Path%%DevPackDir%\%mtkPackDir%\lib\release
if not exist %IncPath%\nul md %IncPath%
if not exist %DebugLibPath%\nul md %DebugLibPath%
if not exist %ReleaseLibPath%\nul md %ReleaseLibPath%
xcopy %mtkPackDir%\*.h %IncPath% /v /y
xcopy Debug\*.lib %DebugLibPath% /v /y
xcopy Release\*.lib %ReleaseLibPath% /v /y

