@ECHO Off
SET EMULATOR_DIR=d:\joengine\Emulators
SET MEDNAFEN_EXECUTABLE_PATH=%EMULATOR_DIR%\mednafen\mednafen.exe

if not exist %MEDNAFEN_EXECUTABLE_PATH% (
	echo ---
	echo Please install Mednafen here %EMULATOR_DIR%
	echo ---
	pause
	exit
)

if exist rom.bin (
"%MEDNAFEN_EXECUTABLE_PATH%" "rom.bin" -sound.volume "150"
) else (
echo Please compile first !
)
