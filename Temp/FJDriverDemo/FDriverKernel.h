#ifndef FDRIVER_KERNEL_H
#define FDRIVER_KERNEL_H

//NtCurrentPeb()->KernelCallbackTable


/******************************************************************************************************************
* CSRSS.exe -- Console, TS, HardError ?
* WinLogon.exe
******************************************************************************************************************/

/******************************************************************************************************************
* Ntdll.dll --
*   KiFastSystemCall -- 
******************************************************************************************************************/

/******************************************************************************************************************
* Win32k.sys -- Windows��ϵͳ�ں��Ĳ��ʵ�֣���Ҫ���� User(�������)��GDI��DX(dxg.sys)����� -- Dxthunksto dxg.sys
*   ������������(table)?
*     1.W32pServiceTable(function & Return value)
*     2.W32pArgument
*     3.TableProvidedto NT kernel via KeAddSystemServiceTableon initialization
******************************************************************************************************************/

#endif //FDRIVER_KERNEL_H