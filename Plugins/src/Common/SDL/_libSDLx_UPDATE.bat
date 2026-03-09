@ECHO OFF
cls
COLOR 1B
rem ---------------------------------------------
TITLE libSDLx Build Script
ECHO Preparing the build.
ECHO ------------------------------
rem	CONFIG START

	set MODE=debug
	set ERROR=0

	IF "%VS71COMNTOOLS%"=="" (
	  set NET="%ProgramFiles%\Microsoft Visual Studio .NET 2003\Common7\IDE\devenv.com"
	) ELSE (
	  set NET="%VS71COMNTOOLS%\..\IDE\devenv.com"
	)
	IF NOT EXIST %NET% (
	  set DIETEXT=Visual Studio .NET 2003 was not found.
	  goto DIE
	)
	
	::set CFG_LIST=(debug profile profile_fastcap release release_ltcg)

	set SOL_LIST=(libSDLx_Audio.sln)

	set LIB_SDLX_LIST=(libSDLx_Audiod.lib libSDLx_Audiop.lib libSDLx_Audiof.lib libSDLx_Audio.lib libSDLx_Audiol.lib)
	
	rmdir "lib\TMP" /S /Q
	md "lib\TMP"
	
	goto COMPILE

:COMPILE
	ECHO Compiling Solution(s) [%MODE% mode]
	ECHO ------------------------------
	
	for %%i in %SOL_LIST% do (
		ECHO Cleaning...
		%NET% %%i /clean %MODE%

		ECHO Compiling...
		%NET% %%i /build %MODE%
		
		:: sleep for a bit while vs writes out lib
		ping 1.1.1.1 -n 1 -w 2000>nul
		
		ECHO Copying files to tmp folder...
		xcopy ".\%MODE%\*.lib" ".\lib\TMP"
		
		ECHO Cleaning...
		%NET% %%i /clean %MODE%
	)

	if %MODE%==release_ltcg (
		goto MAKE_BUILD
	)
	if %MODE%==release (
		set MODE=release_ltcg
	)
	if %MODE%==profile_fastcap (
		set MODE=release
	)
	if %MODE%==profile (
		set MODE=profile_fastcap
	)
	if %MODE%==debug (
		set MODE=profile
	)

	goto COMPILE

:MAKE_BUILD
	for %%i in %LIB_SDLX_LIST% do IF NOT EXIST ".\lib\TMP\%%i" (
		ECHO ----
		echo %%i failed to build!
		set ERROR=1
	)
	if %ERROR%==1 (
		set DIETEXT="Build Failed! (See BuildLog.htm(s) for details)"
		goto DIE
	)
	
	for %%i in %LIB_SDLX_LIST% do del ".\lib\%%i"
	
	ECHO Copying files...
	xcopy ".\lib\TMP\*.lib" ".\lib"
	
	ECHO Removing TMP folder...
	rmdir ".\lib\TMP" /S /Q

	ECHO ------------------------------
	ECHO Build Succeeded!
	ECHO ------------------------------
	
	goto VIEWPAUSE

:DIE
	ECHO !-!-!-!-!-!-!-!-!-!-!-!-!-!-!-
	set DIETEXT=ERROR: %DIETEXT%
	echo %DIETEXT%
	ECHO !-!-!-!-!-!-!-!-!-!-!-!-!-!-!-
	goto VIEWPAUSE

:VIEWPAUSE
	ECHO Press any key to exit...
	pause > NUL
  
:END