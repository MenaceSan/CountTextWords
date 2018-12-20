rem Cleaner For Windows

del %1\*.exe
del %1\*.log
del %1\*.aps
del %1\*.plg
del %1\*.ncb
del %1\*.suo

del %1\NonCompilable /Q
rmdir /s/q %1\NonCompilable

del %1\Debug /Q
rmdir /s/q %1\Debug
del %1\Release /Q
rmdir /s/q %1\Release

del %1\Win32 /Q
rmdir /s/q %1\Win32
del %1\Win32_8 /Q
rmdir /s/q %1\Win32_8
del %1\Win32_10 /Q
rmdir /s/q %1\Win32_10

del %1\x64 /Q
rmdir /s/q %1\x64
del %1\x64_8 /Q
rmdir /s/q %1\x64_8
del %1\x64_10 /Q
rmdir /s/q %1\x64_10

del %1\.vs /Q
rmdir /s/q %1\.vs
