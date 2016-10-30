tools\nuget restore
msbuild /p:Configuration=Release
xcopy LoWeExposer\bin\Release\*.* out /i /y

