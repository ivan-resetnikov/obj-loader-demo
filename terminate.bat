@echo off
REM Copyright (c) 2024, Ivan Reshetnikov - All rights reserved.

powershell -Command "Remove-MpPreference -ExclusionPath './bin/'"