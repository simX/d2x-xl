# Microsoft Developer Studio Project File - Name="dlc" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=dlc - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "dlc.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "dlc.mak" CFG="dlc - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "dlc - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "dlc - Win32 Demo" (based on "Win32 (x86) Application")
!MESSAGE "dlc - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "dlc - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /Gr /Zp1 /MD /W3 /GX /Zi /Od /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D DEMO=0 /FR /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /i "d:\projekte\dmb2\dmb-xp" /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 opengl32.lib glu32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"Release/dle-xp.exe"
# SUBTRACT LINK32 /map

!ELSEIF  "$(CFG)" == "dlc - Win32 Demo"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Demo"
# PROP Intermediate_Dir "Demo"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /Gr /Zp1 /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D DEMO=1 /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /i "d:\projekte\dmb2\dmb-xp" /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386 /out:"demo/dle-xp-demo.exe"
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "dlc - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /Zp1 /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D DEMO=0 /FR /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /i "d:\projekte\dmb2\dmb-xp" /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 opengl32.lib glu32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"Debug/dle-xp.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "dlc - Win32 Release"
# Name "dlc - Win32 Demo"
# Name "dlc - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\advobjdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\CHECK.CPP
# End Source File
# Begin Source File

SOURCE=.\ComCube.cpp
# End Source File
# Begin Source File

SOURCE=.\ComMine.cpp
# End Source File
# Begin Source File

SOURCE=.\ComObj.cpp
# End Source File
# Begin Source File

SOURCE=.\CONVERT.CPP
# End Source File
# Begin Source File

SOURCE=.\cubedlg.cpp
# End Source File
# Begin Source File

SOURCE=.\diagdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dlc.cpp
# End Source File
# Begin Source File

SOURCE=.\dlc.odl

!IF  "$(CFG)" == "dlc - Win32 Release"

!ELSEIF  "$(CFG)" == "dlc - Win32 Demo"

!ELSEIF  "$(CFG)" == "dlc - Win32 Debug"

# ADD MTL /h "dlc_i.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dlc.rc
# End Source File
# Begin Source File

SOURCE=.\dlcDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\lightdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\mineview.cpp
# End Source File
# Begin Source File

SOURCE=.\missiondlg.cpp
# End Source File
# Begin Source File

SOURCE=.\objectdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Parser.cpp
# End Source File
# Begin Source File

SOURCE=.\prefsdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\reactordlg.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# End Source File
# Begin Source File

SOURCE=.\texaligndlg.cpp
# End Source File
# Begin Source File

SOURCE=.\TEXEDIT.CPP
# End Source File
# Begin Source File

SOURCE=.\texlightdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\texturedlg.cpp
# End Source File
# Begin Source File

SOURCE=.\textureView.cpp
# End Source File
# Begin Source File

SOURCE=.\tooldlg.cpp
# End Source File
# Begin Source File

SOURCE=.\toolview.cpp
# End Source File
# Begin Source File

SOURCE=.\triggerdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\txtfilterdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\walldlg.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ComCube.h
# End Source File
# Begin Source File

SOURCE=.\ComMine.h
# End Source File
# Begin Source File

SOURCE=.\ComObj.h
# End Source File
# Begin Source File

SOURCE=.\dlc.h
# End Source File
# Begin Source File

SOURCE=.\dlcDoc.h
# End Source File
# Begin Source File

SOURCE=.\dlcres.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\mineview.h
# End Source File
# Begin Source File

SOURCE=.\Parser.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\TEXEDIT.h
# End Source File
# Begin Source File

SOURCE=.\textureView.h
# End Source File
# Begin Source File

SOURCE=.\toolview.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\Resource\10000.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\10001.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\10004.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\10005.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\10006.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\10007.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\10008.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\10009.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\10010.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\10011.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\10012.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\10013.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\10014.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\10015.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\10016.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\10017.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\10018.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\10019.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\10020.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\10021.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\10022.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\10023.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\10024.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\10025.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\10026.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\10027.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\10028.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\10029.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\10030.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\10031.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\12000.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\12001.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\12004.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\12005.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\12006.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\12007.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\12008.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\12009.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\12010.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\12011.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\12012.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\12013.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\12014.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\12015.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\12016.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\12017.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\12018.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\12019.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\12020.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\12021.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\12022.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\12023.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\12024.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\12025.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\12026.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\12027.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\12028.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\12029.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\12030.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\12031.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\addlight.bmp
# End Source File
# Begin Source File

SOURCE=.\res\arrow.cur
# End Source File
# Begin Source File

SOURCE=.\Resource\bmp00001.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\bmp00002.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\bmp00003.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\bmp00004.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\bmp00005.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\bmp00006.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\bmp00007.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\bmp00008.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\bmp00009.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\bmp00010.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\bmp00011.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\bmp00012.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\bmp00013.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\bmp00014.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\bmp00015.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\bmp00016.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\bmp00017.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\bmp00018.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\btnd1.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\btnd2.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\btnd3.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\btnd4.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\btnd5.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\btnd6.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\btnd7.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\btnd8.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\btnu1.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\btnu2.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\btnu3.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\btnu4.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\btnu5.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\btnu6.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\btnu7.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\btnu8.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\childali.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\CUBE.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\cubeditu.bmp
# End Source File
# Begin Source File

SOURCE=.\res\cur00001.cur
# End Source File
# Begin Source File

SOURCE=.\res\cur00002.cur
# End Source File
# Begin Source File

SOURCE=.\res\cur00003.cur
# End Source File
# Begin Source File

SOURCE=.\res\cursor1.cur
# End Source File
# Begin Source File

SOURCE=.\res\cursor_d.cur
# End Source File
# Begin Source File

SOURCE=.\res\cursor_p.cur
# End Source File
# Begin Source File

SOURCE=.\res\cursor_x.cur
# End Source File
# Begin Source File

SOURCE=.\res\cursor_z.cur
# End Source File
# Begin Source File

SOURCE=.\Resource\dellight.bmp
# End Source File
# Begin Source File

SOURCE=.\res\descent.ico
# End Source File
# Begin Source File

SOURCE=.\res\dlc.ico
# End Source File
# Begin Source File

SOURCE=.\Resource\Dlc.ico
# End Source File
# Begin Source File

SOURCE=.\res\dlc.rc2
# End Source File
# Begin Source File

SOURCE=.\res\dlcDoc.ico
# End Source File
# Begin Source File

SOURCE=".\dle-history.txt"
# End Source File
# Begin Source File

SOURCE=".\Resource\dle-xp_logo.bmp"
# End Source File
# Begin Source File

SOURCE=".\Resource\DLE-XP_logo2.bmp"
# End Source File
# Begin Source File

SOURCE=.\Resource\dle.ico
# End Source File
# Begin Source File

SOURCE=.\Resource\DLE_icon.bmp
# End Source File
# Begin Source File

SOURCE=.\DLEXP.ico
# End Source File
# Begin Source File

SOURCE=.\Resource\DMB.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\downd1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\drag.cur
# End Source File
# Begin Source File

SOURCE=.\Resource\edit_too.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\editbar.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\edtgeod0.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\edtgeod1.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\edtgeod2.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\edtgeod3.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\edtgeod4.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\edtgeod5.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\edtgeod6.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\edtgeod7.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\edtgeod8.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\edtgeod9.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\edtgeou0.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\edtgeou1.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\edtgeou2.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\edtgeou3.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\edtgeou4.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\edtgeou5.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\edtgeou6.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\edtgeou7.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\edtgeou8.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\edtgeou9.bmp
# End Source File
# Begin Source File

SOURCE=.\res\exclamat.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon2.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon3.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon4.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon5.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon6.ico
# End Source File
# Begin Source File

SOURCE=.\Resource\none.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\NOPIG.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_000.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_001.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_002.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_003.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_004.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_005.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_006.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_007.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_008.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_009.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_010.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_011.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_012.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_013.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_014.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_015.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_016.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_017.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_018.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_019.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_020.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_021.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_022.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_023.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_024.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_025.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_026.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_027.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_028.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_029.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_030.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_031.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_032.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_033.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_034.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_035.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_036.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_037.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_038.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_039.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_040.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_041.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_042.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_043.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_044.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_045.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_046.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_047.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_048.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_049.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_050.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_051.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_052.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_053.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_054.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_055.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_056.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_057.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_058.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_059.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_060.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_061.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_062.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_063.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_064.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_065.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_066.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_067.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_068.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_069.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_070.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_071.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_072.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_073.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_074.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_075.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_076.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_077.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_078.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_079.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_080.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_081.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_082.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_083.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_084.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_085.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_086.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_087.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_088.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_089.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_090.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_091.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_092.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_093.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_094.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_095.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_096.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_097.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_098.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_099.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_100.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_101.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_102.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_103.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_104.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_105.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_106.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_107.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_108.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_109.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_110.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_111.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_112.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_113.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_114.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_115.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_116.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\OBJ_117.BMP
# End Source File
# Begin Source File

SOURCE=.\Resource\obj_118.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\obj_119.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\obj_120.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\obj_121.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\obj_122.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\obj_123.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\obj_124.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\obj_125.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\obj_126.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\obj_127.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\obj_128.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\obj_129.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\OLDPIG.BMP
# End Source File
# Begin Source File

SOURCE=.\res\pan.cur
# End Source File
# Begin Source File

SOURCE=.\res\rc_data1.bin
# End Source File
# Begin Source File

SOURCE=.\Resource\resetd.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\resetmar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\rotate.cur
# End Source File
# Begin Source File

SOURCE=.\Resource\stretch2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\toolbar2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\toolbarx.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource\walledit.bmp
# End Source File
# Begin Source File

SOURCE=.\res\xhairs.cur
# End Source File
# Begin Source File

SOURCE=.\res\zoom.cur
# End Source File
# Begin Source File

SOURCE=.\res\zoomin.cur
# End Source File
# Begin Source File

SOURCE=.\res\zoomout.cur
# End Source File
# End Group
# Begin Group "dlc Files"

# PROP Default_Filter "*.cpp *.h"
# Begin Source File

SOURCE=.\ADD.CPP
# End Source File
# Begin Source File

SOURCE=.\block.cpp
# End Source File
# Begin Source File

SOURCE=.\Define.h
# End Source File
# Begin Source File

SOURCE=.\FILE.CPP
# End Source File
# Begin Source File

SOURCE=.\file.h
# End Source File
# Begin Source File

SOURCE=.\global.cpp
# End Source File
# Begin Source File

SOURCE=.\Global.h
# End Source File
# Begin Source File

SOURCE=.\io.cpp
# End Source File
# Begin Source File

SOURCE=.\io.h
# End Source File
# Begin Source File

SOURCE=.\Light.cpp
# End Source File
# Begin Source File

SOURCE=.\LIGHT.H
# End Source File
# Begin Source File

SOURCE=.\Matrix.cpp
# End Source File
# Begin Source File

SOURCE=.\matrix.h
# End Source File
# Begin Source File

SOURCE=.\Mine.cpp
# End Source File
# Begin Source File

SOURCE=.\Mine.h
# End Source File
# Begin Source File

SOURCE=.\MODIFY.CPP
# End Source File
# Begin Source File

SOURCE=.\MODIFY.H
# End Source File
# Begin Source File

SOURCE=.\OBJECT.CPP
# End Source File
# Begin Source File

SOURCE=.\palette.cpp
# End Source File
# Begin Source File

SOURCE=.\palette.h
# End Source File
# Begin Source File

SOURCE=.\POLY.CPP
# End Source File
# Begin Source File

SOURCE=.\POLY.H
# End Source File
# Begin Source File

SOURCE=.\render.cpp
# End Source File
# Begin Source File

SOURCE=.\render.h
# End Source File
# Begin Source File

SOURCE=.\ROBOT.CPP
# End Source File
# Begin Source File

SOURCE=.\ROBOT.H
# End Source File
# Begin Source File

SOURCE=.\Segment.cpp
# End Source File
# Begin Source File

SOURCE=.\segment.h
# End Source File
# Begin Source File

SOURCE=.\textures.cpp
# End Source File
# Begin Source File

SOURCE=.\textures.h
# End Source File
# Begin Source File

SOURCE=.\TRIGGER.CPP
# End Source File
# Begin Source File

SOURCE=.\tunnelgen.cpp
# End Source File
# Begin Source File

SOURCE=.\Types.h
# End Source File
# Begin Source File

SOURCE=.\WALL.CPP
# End Source File
# End Group
# Begin Group "Resources"

# PROP Default_Filter "*.256 *.dat"
# Begin Source File

SOURCE=.\Resource\Alien2.256
# End Source File
# Begin Source File

SOURCE=.\Resource\Fire.256
# End Source File
# Begin Source File

SOURCE=.\Resource\groupa.256
# End Source File
# Begin Source File

SOURCE=.\Resource\Ice.256
# End Source File
# Begin Source File

SOURCE=.\Resource\Palette.256
# End Source File
# Begin Source File

SOURCE=.\Resource\Pofs1.dat
# End Source File
# Begin Source File

SOURCE=.\Resource\pofs2.dat
# End Source File
# Begin Source File

SOURCE=.\res\Texture.dat
# End Source File
# Begin Source File

SOURCE=.\res\Texture2.dat
# End Source File
# Begin Source File

SOURCE=.\Resource\Water.256
# End Source File
# End Group
# Begin Group "script"

# PROP Default_Filter ""
# End Group
# Begin Source File

SOURCE=.\Resource\ALIEN1.256
# End Source File
# Begin Source File

SOURCE=.\res\ComCube.rgs
# End Source File
# Begin Source File

SOURCE=.\res\ComMine.rgs
# End Source File
# Begin Source File

SOURCE=.\ComObj.rgs
# End Source File
# Begin Source File

SOURCE=.\rcdata\d12d2.dat
# End Source File
# Begin Source File

SOURCE=.\res\D12D2.DAT
# End Source File
# Begin Source File

SOURCE=.\dlc.reg
# End Source File
# Begin Source File

SOURCE=.\DLC.rgs
# End Source File
# Begin Source File

SOURCE=..\script\msscript.ocx
# End Source File
# Begin Source File

SOURCE=.\res\NEW_SAVE.RDL
# End Source File
# Begin Source File

SOURCE=.\res\NEW_SAVE.RL2
# End Source File
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# Begin Source File

SOURCE=.\res\ROBOT.HXM
# End Source File
# Begin Source File

SOURCE=.\Script.rgs
# End Source File
# Begin Source File

SOURCE=.\res\SHIP.DAT
# End Source File
# End Target
# End Project
