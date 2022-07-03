@echo off
echo SLIM_HOME was "%SLIM_HOME%"
setx SLIM_HOME %~dp0
echo SLIM_HOME is now set to "%~dp0"
echo Open new command window to see the changes.

@echo on