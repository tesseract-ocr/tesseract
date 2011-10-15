rem deploy tesseract binaries

set SourceFolder=%1
set TargetFolder=%2
set DebugVersion=%3
set DataFolder=%TargetFolder%\tessdata

if not exist %TargetFolder% (
    md %TargetFolder%
)

xcopy %SourceFolder%*.exe %TargetFolder% /Y /I /D
xcopy %SourceFolder%*.dll %TargetFolder% /Y /I /D
xcopy %SourceFolder%*.pdb %TargetFolder% /Y /I /D

rem copy leptonica
xcopy ..\vs2008\lib\liblept168%DebugVersion%.dll %TargetFolder% /Y /I /D

rem copy data
if not exist %DataFolder% (
    md %DataFolder%
)
xcopy ..\TessData\eng.traineddata %DataFolder% /Y /I /D
