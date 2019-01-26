@echo off
setlocal
set Me.Path=%~dps0
for /d /r %Me.Path% %%a in (release,debug,ipch) do if exist %%a rd /s /q %%a
attrib -h %Me.Path%*.suo
for /r %Me.Path% %%a in (*.sdf,*.suo,*.user) do del /f /q /a %%a
