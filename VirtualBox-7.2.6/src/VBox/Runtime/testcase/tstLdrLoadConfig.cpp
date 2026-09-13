/* SPDX-License-Identifier: GPL-3.0-only OR CDDL-1.0 */
/** @file
 * PE load-config revision handling, without loading code into the kernel.
 */

#include <iprt/alloc.h>
#include <iprt/err.h>
#include <iprt/formats/mz.h>
#include <iprt/formats/pecoff.h>
#include <iprt/ldr.h>
#include <iprt/string.h>
#include <iprt/test.h>

template<typename TNt, typename TCfg>
static void testConfig(const char *pszName, bool f64Bit, uint32_t cbDir, uint32_t cbCfg,
                       bool fUnknownNonZero, bool fLockPrefix, int rcExpected)
{
    RTTestSub(NIL_RTTEST, pszName);
    uint8_t *pb = (uint8_t *)RTMemAllocZ(0x600);
    RTTESTI_CHECK_RETV(pb != NULL);
    PIMAGE_DOS_HEADER pDos = (PIMAGE_DOS_HEADER)pb;
    pDos->e_magic = IMAGE_DOS_SIGNATURE;
    pDos->e_lfanew = 0x80;
    TNt *pNt = (TNt *)(pb + 0x80);
    pNt->Signature = IMAGE_NT_SIGNATURE;
    pNt->FileHeader.Machine = f64Bit ? IMAGE_FILE_MACHINE_AMD64 : IMAGE_FILE_MACHINE_I386;
    pNt->FileHeader.NumberOfSections = 1;
    pNt->FileHeader.SizeOfOptionalHeader = sizeof(pNt->OptionalHeader);
    pNt->FileHeader.Characteristics = IMAGE_FILE_EXECUTABLE_IMAGE | IMAGE_FILE_DLL;
    pNt->OptionalHeader.Magic = f64Bit ? IMAGE_NT_OPTIONAL_HDR64_MAGIC : IMAGE_NT_OPTIONAL_HDR32_MAGIC;
    pNt->OptionalHeader.ImageBase = 0x400000;
    pNt->OptionalHeader.Subsystem = IMAGE_SUBSYSTEM_NATIVE;
    pNt->OptionalHeader.SectionAlignment = 0x1000;
    pNt->OptionalHeader.FileAlignment = 0x200;
    pNt->OptionalHeader.SizeOfHeaders = 0x200;
    pNt->OptionalHeader.SizeOfImage = 0x2000;
    pNt->OptionalHeader.NumberOfRvaAndSizes = IMAGE_NUMBEROF_DIRECTORY_ENTRIES;
    pNt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG].VirtualAddress = 0x1000;
    pNt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG].Size = cbDir;
    PIMAGE_SECTION_HEADER pSection = (PIMAGE_SECTION_HEADER)(pNt + 1);
    memcpy(pSection->Name, ".rdata", 6);
    pSection->VirtualAddress = 0x1000;
    pSection->Misc.VirtualSize = 0x400;
    pSection->PointerToRawData = 0x200;
    pSection->SizeOfRawData = 0x400;
    pSection->Characteristics = IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ;

    TCfg *pCfg = (TCfg *)(pb + 0x200);
    pCfg->Size = cbCfg;
    /* Nonzero known fields must remain acceptable even when followed by a
       newer revision.  The pointers are metadata, never executed by this test. */
    pCfg->SecurityCookie = 0x401200;
    if (cbCfg >= RT_UOFFSETOF(TCfg, GuardMemcpyFunctionPointer))
        pCfg->CastGuardOsDeterminedFailureMode = 0x401208;
    if (cbCfg >= RT_UOFFSETOF(TCfg, UmaFunctionPointers))
        pCfg->GuardMemcpyFunctionPointer = 0x401210;
    if (cbCfg >= sizeof(TCfg))
        pCfg->UmaFunctionPointers = 0x401218;
    if (fUnknownNonZero)
        pb[0x200 + sizeof(TCfg)] = 1;
    if (fLockPrefix)
        pCfg->LockPrefixTable = 0x401220;

    RTLDRMOD hMod = NIL_RTLDRMOD;
    RTERRINFOSTATIC ErrInfo;
    int rc = RTLdrOpenInMemory(pszName, 0, RTLDRARCH_WHATEVER, 0x600, NULL, NULL, pb,
                              &hMod, RTErrInfoInitStatic(&ErrInfo));
    if (rc != rcExpected)
        RTTestFailed(NIL_RTTEST, "expected %Rrc, got %Rrc: %s", rcExpected, rc, ErrInfo.Core.pszMsg);
    if (RT_SUCCESS(rc))
        RTTESTI_CHECK_RC(RTLdrClose(hMod), VINF_SUCCESS);
}

template<typename TNt, typename TCfg>
static void testArch(bool f64Bit)
{
    uint32_t const cbV12 = f64Bit ? 304 : 184;
    uint32_t const cbV14 = f64Bit ? 320 : 192;
    uint32_t const cbV15 = f64Bit ? 328 : 196;
    testConfig<TNt, TCfg>("V12", f64Bit, cbV12, cbV12, false, false, VINF_SUCCESS);
    testConfig<TNt, TCfg>("V14", f64Bit, cbV14, cbV14, false, false, VINF_SUCCESS);
    testConfig<TNt, TCfg>("V15 with UMA metadata", f64Bit, cbV15, cbV15, false, false, VINF_SUCCESS);
    testConfig<TNt, TCfg>("V15 with old directory size", f64Bit, cbV14, cbV15, false, false, VINF_SUCCESS);
    testConfig<TNt, TCfg>("future zero tail", f64Bit, cbV15 + 8, cbV15 + 8, false, false, VINF_SUCCESS);
    testConfig<TNt, TCfg>("future nonzero tail", f64Bit, cbV15 + 8, cbV15 + 8, true, false,
                         VERR_LDRPE_LOAD_CONFIG_SIZE);
    testConfig<TNt, TCfg>("future nonzero tail with old directory size", f64Bit, cbV14, cbV15 + 8, true, false,
                         VERR_LDRPE_LOAD_CONFIG_SIZE);
    testConfig<TNt, TCfg>("lock prefix rejected", f64Bit, cbV15, cbV15, false, true,
                         VERR_LDRPE_LOCK_PREFIX_TABLE);
    testConfig<TNt, TCfg>("lock prefix rejected after reread", f64Bit, cbV14, cbV15, false, true,
                         VERR_LDRPE_LOCK_PREFIX_TABLE);
}

int main(int argc, char **argv)
{
    RTTEST hTest;
    RTEXITCODE rcExit = RTTestInitAndCreate("tstLdrLoadConfig", &hTest);
    if (rcExit != RTEXITCODE_SUCCESS)
        return rcExit;
    RT_NOREF(argc, argv);
    RTTestBanner(hTest);
    RTTestPrintf(hTest, RTTESTLVL_ALWAYS, "PE32 load configuration\n");
    testArch<IMAGE_NT_HEADERS32, IMAGE_LOAD_CONFIG_DIRECTORY32>(false);
    RTTestPrintf(hTest, RTTESTLVL_ALWAYS, "PE32+ load configuration\n");
    testArch<IMAGE_NT_HEADERS64, IMAGE_LOAD_CONFIG_DIRECTORY64>(true);
    return RTTestSummaryAndDestroy(hTest);
}
