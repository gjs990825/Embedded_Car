SET PATH=C:\Keil_v5\ARM\ARMCC\Bin;C:\Program Files (x86)\NetSarang\Xshell 6\;D:\Program Files\nTurn\Goto;D:\DATA\Java\bin;D:\DATA\Java\jre\bin;C:\WINDOWS\system32;C:\WINDOWS;C:\WINDOWS\System32\Wbem;C:\Windows\SysWOW64;C:\Program Files\Git\cmd;D:\DATA\Android\Sdk\platform-tools;C:\WINDOWS\System32\WindowsPowerShell\v1.0\;C:\WINDOWS\System32\OpenSSH\;D:\DATA\GNU_For_Arm\7 2018-q2-update\bin;C:\Users\Gao\AppData\Local\Microsoft\WindowsApps;C:\Users\Gao\AppData\Local\Programs\Microsoft VS Code\bin;C:\Users\Gao\AppData\Local\GitHubDesktop\bin;C:\Users\Gao\AppData\Local\Microsoft\WindowsApps;D:\Program Files\GnuWin32\bin
SET CPU_TYPE=STM32F407IG
SET CPU_VENDOR=STMicroelectronics
SET UV2_TARGET=Embedded_Car
SET CPU_CLOCK=0x00B71B00
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via "..\output\main.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via "..\output\infrared.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via "..\output\syn7318.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmLink" --Via "..\OUTPUT\Embedded_Car.lnp"
"C:\Keil_v5\ARM\ARMCC\Bin\fromelf.exe" "..\OUTPUT\Embedded_Car.axf" --i32combined --output "..\OUTPUT\Embedded_Car.hex"
