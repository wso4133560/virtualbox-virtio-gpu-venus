<?xml version="1.0" encoding="utf-8"?>
<idl>
<if target="midl">
  <cpp line="enum {"/>
  <cpp line="    kTypeLibraryMajorVersion = 1,"/>
  <cpp line="    kTypeLibraryMinorVersion = 3"/>
  <cpp line="};"/>
</if>
<if target="xpidl">
  <cpp>
/* currently, nsISupportsImpl.h lacks the below-like macros */
#define NS_IMPL_THREADSAFE_QUERY_INTERFACE1_CI NS_IMPL_QUERY_INTERFACE1_CI
#define NS_IMPL_THREADSAFE_QUERY_INTERFACE2_CI NS_IMPL_QUERY_INTERFACE2_CI
#define NS_IMPL_THREADSAFE_QUERY_INTERFACE3_CI NS_IMPL_QUERY_INTERFACE3_CI
#define NS_IMPL_THREADSAFE_QUERY_INTERFACE4_CI NS_IMPL_QUERY_INTERFACE4_CI
#define NS_IMPL_THREADSAFE_QUERY_INTERFACE5_CI NS_IMPL_QUERY_INTERFACE5_CI
#define NS_IMPL_THREADSAFE_QUERY_INTERFACE6_CI NS_IMPL_QUERY_INTERFACE6_CI
#define NS_IMPL_THREADSAFE_QUERY_INTERFACE7_CI NS_IMPL_QUERY_INTERFACE7_CI
#define NS_IMPL_THREADSAFE_QUERY_INTERFACE8_CI NS_IMPL_QUERY_INTERFACE8_CI
#ifndef NS_IMPL_THREADSAFE_ISUPPORTS1_CI
# define NS_IMPL_THREADSAFE_ISUPPORTS1_CI(_class, _interface)                 \
  NS_IMPL_THREADSAFE_ADDREF(_class)                                           \
  NS_IMPL_THREADSAFE_RELEASE(_class)                                          \
  NS_IMPL_THREADSAFE_QUERY_INTERFACE1_CI(_class, _interface)                  \
  NS_IMPL_CI_INTERFACE_GETTER1(_class, _interface)
#endif
#ifndef NS_IMPL_THREADSAFE_ISUPPORTS2_CI
# define NS_IMPL_THREADSAFE_ISUPPORTS2_CI(_class, _i1, _i2)                   \
  NS_IMPL_THREADSAFE_ADDREF(_class)                                           \
  NS_IMPL_THREADSAFE_RELEASE(_class)                                          \
  NS_IMPL_THREADSAFE_QUERY_INTERFACE2_CI(_class, _i1, _i2)                    \
  NS_IMPL_CI_INTERFACE_GETTER2(_class, _i1, _i2)
#endif
#ifndef NS_IMPL_THREADSAFE_ISUPPORTS3_CI
# define NS_IMPL_THREADSAFE_ISUPPORTS3_CI(_class, _i1, _i2, _i3)              \
  NS_IMPL_THREADSAFE_ADDREF(_class)                                           \
  NS_IMPL_THREADSAFE_RELEASE(_class)                                          \
  NS_IMPL_THREADSAFE_QUERY_INTERFACE3_CI(_class, _i1, _i2, _i3)               \
  NS_IMPL_CI_INTERFACE_GETTER3(_class, _i1, _i2, _i3)
#endif
#ifndef NS_IMPL_THREADSAFE_ISUPPORTS4_CI
# define NS_IMPL_THREADSAFE_ISUPPORTS4_CI(_class, _i1, _i2, _i3, _i4)         \
  NS_IMPL_THREADSAFE_ADDREF(_class)                                           \
  NS_IMPL_THREADSAFE_RELEASE(_class)                                          \
  NS_IMPL_THREADSAFE_QUERY_INTERFACE4_CI(_class, _i1, _i2, _i3, _i4)          \
  NS_IMPL_CI_INTERFACE_GETTER4(_class, _i1, _i2, _i3, _i4)
#endif
#ifndef NS_IMPL_THREADSAFE_ISUPPORTS5_CI
# define NS_IMPL_THREADSAFE_ISUPPORTS5_CI(_class, _i1, _i2, _i3, _i4, _i5)    \
  NS_IMPL_THREADSAFE_ADDREF(_class)                                           \
  NS_IMPL_THREADSAFE_RELEASE(_class)                                          \
  NS_IMPL_THREADSAFE_QUERY_INTERFACE5_CI(_class, _i1, _i2, _i3, _i4, _i5)     \
  NS_IMPL_CI_INTERFACE_GETTER5(_class, _i1, _i2, _i3, _i4, _i5)
#endif
#ifndef NS_IMPL_THREADSAFE_ISUPPORTS6_CI
# define NS_IMPL_THREADSAFE_ISUPPORTS6_CI(_class, _i1, _i2, _i3, _i4, _i5, _i6) \
  NS_IMPL_THREADSAFE_ADDREF(_class) \
  NS_IMPL_THREADSAFE_RELEASE(_class) \
  NS_IMPL_THREADSAFE_QUERY_INTERFACE6_CI(_class, _i1, _i2, _i3, _i4, _i5, _i6) \
  NS_IMPL_CI_INTERFACE_GETTER6(_class, _i1, _i2, _i3, _i4, _i5, _i6)
#endif
#ifndef NS_IMPL_THREADSAFE_ISUPPORTS7_CI
# define NS_IMPL_THREADSAFE_ISUPPORTS7_CI(_class, _i1, _i2, _i3, _i4, _i5, _i6, _i7) \
  NS_IMPL_THREADSAFE_ADDREF(_class) \
  NS_IMPL_THREADSAFE_RELEASE(_class) \
  NS_IMPL_THREADSAFE_QUERY_INTERFACE7_CI(_class, _i1, _i2, _i3, _i4, _i5, _i6, _i7) \
  NS_IMPL_CI_INTERFACE_GETTER7(_class, _i1, _i2, _i3, _i4, _i5, _i6, _i7)
#endif
#ifndef NS_IMPL_THREADSAFE_ISUPPORTS8_CI
# define NS_IMPL_THREADSAFE_ISUPPORTS8_CI(_class, _i1, _i2, _i3, _i4, _i5, _i6, _i7, _i8) \
  NS_IMPL_THREADSAFE_ADDREF(_class) \
  NS_IMPL_THREADSAFE_RELEASE(_class) \
  NS_IMPL_THREADSAFE_QUERY_INTERFACE8_CI(_class, _i1, _i2, _i3, _i4, _i5, _i6, _i7, _i8) \
  NS_IMPL_CI_INTERFACE_GETTER8(_class, _i1, _i2, _i3, _i4, _i5, _i6, _i7, _i8)
#endif
#ifndef NS_IMPL_QUERY_INTERFACE1_AMBIGUOUS_CI
# define NS_IMPL_QUERY_INTERFACE1_AMBIGUOUS_CI(_class, _i1, _ic1)             \
  NS_INTERFACE_MAP_BEGIN(_class)                                              \
    NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(_i1, _ic1)                               \
    NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, _ic1)                       \
    NS_IMPL_QUERY_CLASSINFO(_class)                                           \
  NS_INTERFACE_MAP_END
#endif
#ifndef NS_IMPL_QUERY_INTERFACE2_AMBIGUOUS_CI
# define NS_IMPL_QUERY_INTERFACE2_AMBIGUOUS_CI(_class, _i1, _ic1,             \
                                               _i2, _ic2)                     \
  NS_INTERFACE_MAP_BEGIN(_class)                                              \
    NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(_i1, _ic1)                               \
    NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(_i2, _ic2)                               \
    NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, _ic1)                       \
    NS_IMPL_QUERY_CLASSINFO(_class)                                           \
  NS_INTERFACE_MAP_END
#endif
#ifndef NS_IMPL_QUERY_INTERFACE3_AMBIGUOUS_CI
# define NS_IMPL_QUERY_INTERFACE3_AMBIGUOUS_CI(_class, _i1, _ic1,             \
                                               _i2, _ic2, _i3, _ic3)          \
  NS_INTERFACE_MAP_BEGIN(_class)                                              \
    NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(_i1, _ic1)                               \
    NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(_i2, _ic2)                               \
    NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(_i3, _ic3)                               \
    NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, _ic1)                       \
    NS_IMPL_QUERY_CLASSINFO(_class)                                           \
  NS_INTERFACE_MAP_END
#endif
#define NS_IMPL_THREADSAFE_QUERY_INTERFACE1_AMBIGUOUS_CI NS_IMPL_QUERY_INTERFACE1_AMBIGUOUS_CI
#define NS_IMPL_THREADSAFE_QUERY_INTERFACE2_AMBIGUOUS_CI NS_IMPL_QUERY_INTERFACE2_AMBIGUOUS_CI
#define NS_IMPL_THREADSAFE_QUERY_INTERFACE3_AMBIGUOUS_CI NS_IMPL_QUERY_INTERFACE3_AMBIGUOUS_CI
#ifndef NS_IMPL_THREADSAFE_ISUPPORTS1_AMBIGUOUS_CI
# define NS_IMPL_THREADSAFE_ISUPPORTS1_AMBIGUOUS_CI(_class, _i1, _ic1)        \
  NS_IMPL_THREADSAFE_ADDREF(_class)                                           \
  NS_IMPL_THREADSAFE_RELEASE(_class)                                          \
  NS_IMPL_THREADSAFE_QUERY_INTERFACE1_AMBIGUOUS_CI(_class, _i1, _ic1)         \
  NS_IMPL_CI_INTERFACE_GETTER1(_class, _i1)
#endif
#ifndef NS_IMPL_THREADSAFE_ISUPPORTS2_AMBIGUOUS_CI
# define NS_IMPL_THREADSAFE_ISUPPORTS2_AMBIGUOUS_CI(_class, _i1, _ic1,        \
                                                     _i2, _ic2)               \
  NS_IMPL_THREADSAFE_ADDREF(_class)                                           \
  NS_IMPL_THREADSAFE_RELEASE(_class)                                          \
  NS_IMPL_THREADSAFE_QUERY_INTERFACE2_AMBIGUOUS_CI(_class, _i1, _ic1,         \
                                                   _i2, _ic2)                 \
  NS_IMPL_CI_INTERFACE_GETTER2(_class, _i1, _i2)
#endif
#ifndef NS_IMPL_THREADSAFE_ISUPPORTS3_AMBIGUOUS_CI
# define NS_IMPL_THREADSAFE_ISUPPORTS3_AMBIGUOUS_CI(_class, _i1, _ic1,        \
                                                     _i2, _ic2, _i3, _ic3)    \
  NS_IMPL_THREADSAFE_ADDREF(_class)                                           \
  NS_IMPL_THREADSAFE_RELEASE(_class)                                          \
  NS_IMPL_THREADSAFE_QUERY_INTERFACE3_AMBIGUOUS_CI(_class, _i1, _ic1,         \
                                                   _i2, _ic2, _i3, _ic3)      \
  NS_IMPL_CI_INTERFACE_GETTER3(_class, _i1, _i2, _i3)
#endif
  </cpp>
</if>
<cpp line="#define GUEST_OS_ID_STR_X86(a_szOSid)     a_szOSid"/>
<cpp line="#if 1 /* 2023-10-06: We stick with legacy variant, see bugref{10384#c19}. */"/>
<cpp line="# define GUEST_OS_ID_STR_X64(a_szOSid)    a_szOSid  &quot;_64&quot;"/>
<cpp line="# define GUEST_OS_ID_STYLE_LEGACY"/>
<cpp line="#else"/>
<cpp line="# define GUEST_OS_ID_STR_X64(a_szOSid)    a_szOSid  &quot;_x64&quot;"/>
<cpp line="# define GUEST_OS_ID_STYLE_PARTIAL_CLEANUP"/>
<cpp line="#endif"/>
<cpp line="#define GUEST_OS_ID_STR_A32(a_szOSid)     a_szOSid  &quot;_arm32&quot;"/>
<cpp line="#define GUEST_OS_ID_STR_A64(a_szOSid)     a_szOSid  &quot;_arm64&quot;"/>
<cpp line="#define GUEST_OS_ID_STR_AMD64(a_szOSid)   GUEST_OS_ID_STR_X64(a_szOSid)"/>
<cpp line="#define GUEST_OS_ID_STR_ARM32(a_szOSid)   GUEST_OS_ID_STR_A32(a_szOSid)"/>
<cpp line="#define GUEST_OS_ID_STR_ARM64(a_szOSid)   GUEST_OS_ID_STR_A64(a_szOSid)"/>
<cpp line="#define GUEST_OS_ID_STR_PARTIAL(a_szOSidPart) a_szOSidPart"/>
<library name="VirtualBox" uuid="d7569351-1750-46f0-936e-bd127d5bc264" version="1.3">
<application name="VirtualBox" uuid="61ff179a-5f10-4077-81f4-060beba09dec" supportsErrorInfo="yes">
  <descGroup id="VirtualBox_COM_result_codes" title="VirtualBox COM result codes">
  </descGroup>
  <result name="VBOX_E_OBJECT_NOT_FOUND" value="0x80BB0001">
  </result>
  <result name="VBOX_E_INVALID_VM_STATE" value="0x80BB0002">
  </result>
  <result name="VBOX_E_VM_ERROR" value="0x80BB0003">
  </result>
  <result name="VBOX_E_FILE_ERROR" value="0x80BB0004">
  </result>
  <result name="VBOX_E_IPRT_ERROR" value="0x80BB0005">
  </result>
  <result name="VBOX_E_PDM_ERROR" value="0x80BB0006">
  </result>
  <result name="VBOX_E_INVALID_OBJECT_STATE" value="0x80BB0007">
  </result>
  <result name="VBOX_E_HOST_ERROR" value="0x80BB0008">
  </result>
  <result name="VBOX_E_NOT_SUPPORTED" value="0x80BB0009">
  </result>
  <result name="VBOX_E_XML_ERROR" value="0x80BB000A">
  </result>
  <result name="VBOX_E_INVALID_SESSION_STATE" value="0x80BB000B">
  </result>
  <result name="VBOX_E_OBJECT_IN_USE" value="0x80BB000C">
  </result>
  <result name="VBOX_E_PASSWORD_INCORRECT" value="0x80BB000D">
  </result>
  <result name="VBOX_E_MAXIMUM_REACHED" value="0x80BB000E">
  </result>
  <result name="VBOX_E_GSTCTL_GUEST_ERROR" value="0x80BB000F">
  </result>
  <result name="VBOX_E_TIMEOUT" value="0x80BB0010">
  </result>
  <result name="VBOX_E_DND_ERROR" value="0x80BB0011">
  </result>
  <result name="VBOX_E_PLATFORM_ARCH_NOT_SUPPORTED" value="0x80BB0012">
  </result>
  <result name="VBOX_E_RECORDING_ERROR" value="0x80BB0013">
  </result>
  <descGroup/>
  <enum name="SettingsVersion" uuid="85d84a63-bda8-424e-95fe-3650b6cf5a95">
    <const name="Null" value="0" wsmap="managed">
    </const>
    <const name="v1_0" value="1" wsmap="managed">
    </const>
    <const name="v1_1" value="2" wsmap="managed">
    </const>
    <const name="v1_2" value="3" wsmap="managed">
    </const>
    <const name="v1_3pre" value="4" wsmap="managed">
    </const>
    <const name="v1_3" value="5" wsmap="managed">
    </const>
    <const name="v1_4" value="6" wsmap="managed">
    </const>
    <const name="v1_5" value="7" wsmap="managed">
    </const>
    <const name="v1_6" value="8" wsmap="managed">
    </const>
    <const name="v1_7" value="9" wsmap="managed">
    </const>
    <const name="v1_8" value="10" wsmap="managed">
    </const>
    <const name="v1_9" value="11" wsmap="managed">
    </const>
    <const name="v1_10" value="12" wsmap="managed">
    </const>
    <const name="v1_11" value="13" wsmap="managed">
    </const>
    <const name="v1_12" value="14" wsmap="managed">
    </const>
    <const name="v1_13" value="15" wsmap="managed">
    </const>
    <const name="v1_14" value="16" wsmap="managed">
    </const>
    <const name="v1_15" value="17" wsmap="managed">
    </const>
    <const name="v1_16" value="18" wsmap="managed">
    </const>
    <const name="v1_17" value="19" wsmap="managed">
    </const>
    <const name="v1_18" value="20" wsmap="managed">
    </const>
    <const name="v1_19" value="21" wsmap="managed">
    </const>
    <const name="v1_20" value="22" wsmap="managed">
    </const>
  <const name="v1_21" value="23" wsmap="managed">
  </const>
    <const name="Future" value="99999" wsmap="managed">
    </const>
  </enum>
  <enum name="AccessMode" uuid="1da0007c-ddf7-4be8-bcac-d84a1558785f">
    <const name="ReadOnly" value="1" wsmap="managed"/>
    <const name="ReadWrite" value="2" wsmap="managed"/>
  </enum>
  <enum name="MachineState" uuid="00bc01b5-00a4-48db-000a-9061008357aa">
    <const name="Null" value="0" wsmap="managed">
    </const>
    <const name="PoweredOff" value="1" wsmap="managed">
    </const>
    <const name="Saved" value="2" wsmap="managed">
    </const>
    <const name="Teleported" value="3" wsmap="managed">
    </const>
    <const name="Aborted" value="4" wsmap="managed">
    </const>
    <const name="AbortedSaved" value="5" wsmap="managed">
    </const>
    <const name="Running" value="6" wsmap="managed">
    </const>
    <const name="Paused" value="7" wsmap="managed">
    </const>
    <const name="Stuck" value="8" wsmap="managed">
    </const>
    <const name="Teleporting" value="9" wsmap="managed">
    </const>
    <const name="LiveSnapshotting" value="10" wsmap="managed">
    </const>
    <const name="Starting" value="11" wsmap="managed">
    </const>
    <const name="Stopping" value="12" wsmap="managed">
    </const>
    <const name="Saving" value="13" wsmap="managed">
    </const>
    <const name="Restoring" value="14" wsmap="managed">
    </const>
    <const name="TeleportingPausedVM" value="15" wsmap="managed">
    </const>
    <const name="TeleportingIn" value="16" wsmap="managed">
    </const>
    <const name="DeletingSnapshotOnline" value="17" wsmap="managed">
    </const>
    <const name="DeletingSnapshotPaused" value="18" wsmap="managed">
    </const>
    <const name="OnlineSnapshotting" value="19" wsmap="managed">
    </const>
    <const name="RestoringSnapshot" value="20" wsmap="managed">
    </const>
    <const name="DeletingSnapshot" value="21" wsmap="managed">
    </const>
    <const name="SettingUp" value="22" wsmap="managed">
    </const>
    <const name="Snapshotting" value="23" wsmap="managed">
    </const>
    <const name="FirstOnline" value="6" wsmap="suppress">
    </const>
    <const name="LastOnline" value="19" wsmap="suppress">
    </const>
    <const name="FirstTransient" value="9" wsmap="suppress">
    </const>
    <const name="LastTransient" value="23" wsmap="suppress">
    </const>
  </enum>
  <enum name="SessionState" uuid="cf2700c0-ea4b-47ae-9725-7810114b94d8">
    <const name="Null" value="0" wsmap="managed">
    </const>
    <const name="Unlocked" value="1" wsmap="managed">
    </const>
    <const name="Locked" value="2" wsmap="managed">
    </const>
    <const name="Spawning" value="3" wsmap="managed">
    </const>
    <const name="Unlocking" value="4" wsmap="managed">
    </const>
  </enum>
  <enum name="PlatformArchitecture" uuid="36e21926-1713-413d-8cf5-2f169b2cbcf3">
    <const name="None" value="0" wsmap="managed">
    </const>
    <const name="x86" value="1" wsmap="managed">
    </const>
    <const name="ARM" value="2" wsmap="managed">
    </const>
  </enum>
  <enum name="CPUArchitecture" uuid="12906c63-9967-49c8-ba5a-6521adaf0185">
    <const name="Any" value="0" wsmap="managed">
    </const>
    <const name="x86" value="1" wsmap="managed">
    </const>
    <const name="AMD64" value="2" wsmap="managed">
    </const>
    <const name="ARMv8_32" value="3" wsmap="managed">
    </const>
    <const name="ARMv8_64" value="4" wsmap="managed">
    </const>
  </enum>
  <enum name="CPUPropertyTypeX86" uuid="a754b765-3687-4a68-abb9-78dcf4c07d5f">
    <const name="Null" value="0" wsmap="managed">
    </const>
    <const name="PAE" value="1" wsmap="managed">
    </const>
    <const name="LongMode" value="2" wsmap="managed">
    </const>
    <const name="TripleFaultReset" value="3" wsmap="managed">
    </const>
    <const name="APIC" value="4" wsmap="managed">
    </const>
    <const name="X2APIC" value="5" wsmap="managed">
    </const>
    <const name="IBPBOnVMExit" value="6" wsmap="managed">
    </const>
    <const name="IBPBOnVMEntry" value="7" wsmap="managed">
    </const>
    <const name="HWVirt" value="8" wsmap="managed">
    </const>
    <const name="SpecCtrl" value="9" wsmap="managed">
    </const>
    <const name="SpecCtrlByHost" value="10" wsmap="managed">
    </const>
    <const name="L1DFlushOnEMTScheduling" value="11" wsmap="managed">
    </const>
    <const name="L1DFlushOnVMEntry" value="12" wsmap="managed">
    </const>
    <const name="MDSClearOnEMTScheduling" value="13" wsmap="managed">
    </const>
    <const name="MDSClearOnVMEntry" value="14" wsmap="managed">
    </const>
  </enum>
  <enum name="CPUPropertyTypeARM" uuid="f84eb90a-3e01-47cf-88c1-f8fc559dd37a">
    <const name="Null" value="0" wsmap="managed">
    </const>
    <const name="HWVirt" value="1" wsmap="managed">
    </const>
    <const name="GICITS" value="2" wsmap="managed">
    </const>
  </enum>
  <enum name="HWVirtExPropertyType" uuid="00069d9c-00b5-460c-00dd-64250024f7aa">
    <const name="Null" value="0" wsmap="managed">
    </const>
    <const name="Enabled" value="1" wsmap="managed">
    </const>
    <const name="VPID" value="2" wsmap="managed">
    </const>
    <const name="NestedPaging" value="3" wsmap="managed">
    </const>
    <const name="UnrestrictedExecution" value="4" wsmap="managed">
    </const>
    <const name="LargePages" value="5" wsmap="managed">
    </const>
    <const name="Force" value="6" wsmap="managed">
    </const>
    <const name="UseNativeApi" value="7" wsmap="managed">
    </const>
    <const name="VirtVmsaveVmload" value="8" wsmap="managed">
    </const>
  </enum>
  <enum name="ParavirtProvider" uuid="696453ec-3742-4a05-bead-658ccbf2c944">
    <const name="None" value="0" wsmap="managed">
    </const>
    <const name="Default" value="1" wsmap="managed">
    </const>
    <const name="Legacy" value="2" wsmap="managed">
    </const>
    <const name="Minimal" value="3" wsmap="managed">
    </const>
    <const name="HyperV" value="4" wsmap="managed">
    </const>
    <const name="KVM" value="5" wsmap="managed">
    </const>
  </enum>
  <enum name="LockType" uuid="678aaf14-2815-4c3e-b20a-e86ed0216498">
    <const name="Null" value="0" wsmap="managed">
    </const>
    <const name="Shared" value="1" wsmap="managed">
    </const>
    <const name="Write" value="2" wsmap="managed">
    </const>
    <const name="VM" value="3" wsmap="managed">
    </const>
  </enum>
  <enum name="SessionType" uuid="A13C02CB-0C2C-421E-8317-AC0E8AAA153A">
    <const name="Null" value="0" wsmap="managed">
    </const>
    <const name="WriteLock" value="1" wsmap="managed">
    </const>
    <const name="Remote" value="2" wsmap="managed">
    </const>
    <const name="Shared" value="3" wsmap="managed">
    </const>
  </enum>
  <enum name="DeviceType" uuid="cb977be1-d1fb-41f8-ad7e-951736c6cb3e">
    <const name="Null" value="0" wsmap="managed">
    </const>
    <const name="Floppy" value="1" wsmap="managed">
    </const>
    <const name="DVD" value="2" wsmap="managed">
    </const>
    <const name="HardDisk" value="3" wsmap="managed">
    </const>
    <const name="Network" value="4" wsmap="managed">
    </const>
    <const name="USB" value="5" wsmap="managed">
    </const>
    <const name="SharedFolder" value="6" wsmap="managed">
    </const>
    <const name="Graphics3D" value="7" wsmap="managed">
    </const>
    <const name="End" value="8" wsmap="managed">
    </const>
  </enum>
  <enum name="DeviceActivity" uuid="6FC8AEAA-130A-4eb5-8954-3F921422D707">
    <const name="Null" value="0" wsmap="managed"/>
    <const name="Idle" value="1" wsmap="managed"/>
    <const name="Reading" value="2" wsmap="managed"/>
    <const name="Writing" value="3" wsmap="managed"/>
  </enum>
  <enum name="ClipboardMode" uuid="33364716-4008-4701-8f14-be0fa3d62950">
    <const name="Disabled" value="0" wsmap="managed"/>
    <const name="HostToGuest" value="1" wsmap="managed"/>
    <const name="GuestToHost" value="2" wsmap="managed"/>
    <const name="Bidirectional" value="3" wsmap="managed"/>
  </enum>
  <enum name="DnDMode" uuid="07af8800-f936-4b33-9172-cd400e83c148">
    <const name="Disabled" value="0" wsmap="managed"/>
    <const name="HostToGuest" value="1" wsmap="managed"/>
    <const name="GuestToHost" value="2" wsmap="managed"/>
    <const name="Bidirectional" value="3" wsmap="managed"/>
  </enum>
  <enum name="Scope" uuid="7c91096e-499e-4eca-9f9b-9001438d7855">
    <const name="Global" value="0" wsmap="managed"/>
    <const name="Machine" value="1" wsmap="managed"/>
    <const name="Session" value="2" wsmap="managed"/>
  </enum>
  <enum name="FirmwareBootMenuMode" uuid="bf88ec90-7fbb-4c78-bd71-5f8751d71450">
    <const name="Disabled" value="0" wsmap="managed"/>
    <const name="MenuOnly" value="1" wsmap="managed"/>
    <const name="MessageAndMenu" value="2" wsmap="managed"/>
  </enum>
  <enum name="APICMode" uuid="c6884ba5-3cc4-4a92-a7f6-4410f9fd894e">
    <const name="Disabled" value="0" wsmap="managed"/>
    <const name="APIC" value="1" wsmap="managed"/>
    <const name="X2APIC" value="2" wsmap="managed"/>
  </enum>
  <enum name="ProcessorFeature" uuid="0064dece-000e-4963-00f8-eb9b00674c8a">
    <const name="HWVirtEx" value="0" wsmap="managed"/>
    <const name="PAE" value="1" wsmap="managed"/>
    <const name="LongMode" value="2" wsmap="managed"/>
    <const name="NestedPaging" value="3" wsmap="managed"/>
    <const name="UnrestrictedGuest" value="4" wsmap="managed"/>
    <const name="NestedHWVirt" value="5" wsmap="managed"/>
    <const name="VirtVmsaveVmload" value="6" wsmap="managed"/>
  </enum>
  <enum name="FirmwareType" uuid="b903f264-c230-483e-ac74-2b37ce60d371">
    <const name="BIOS" value="1" wsmap="managed">
    </const>
    <const name="EFI" value="2" wsmap="managed">
    </const>
    <const name="EFI32" value="3" wsmap="managed">
    </const>
    <const name="EFI64" value="4" wsmap="managed">
    </const>
    <const name="EFIDUAL" value="5" wsmap="managed">
    </const>
  </enum>
  <enum name="PointingHIDType" uuid="b3fd8215-6870-4e61-b6d9-2998fa625de0">
    <const name="None" value="1" wsmap="managed">
    </const>
    <const name="PS2Mouse" value="2" wsmap="managed">
    </const>
    <const name="USBMouse" value="3" wsmap="managed">
    </const>
    <const name="USBTablet" value="4" wsmap="managed">
    </const>
    <const name="ComboMouse" value="5" wsmap="managed">
    </const>
    <const name="USBMultiTouch" value="6" wsmap="managed">
    </const>
    <const name="USBMultiTouchScreenPlusPad" value="7" wsmap="managed">
    </const>
  </enum>
  <enum name="KeyboardHIDType" uuid="383e43d7-5c7c-4ec8-9cb8-eda1bccd6699">
    <const name="None" value="1" wsmap="managed">
    </const>
    <const name="PS2Keyboard" value="2" wsmap="managed">
    </const>
    <const name="USBKeyboard" value="3" wsmap="managed">
    </const>
    <const name="ComboKeyboard" value="4" wsmap="managed">
    </const>
  </enum>
  <enum name="BitmapFormat" uuid="afb2bf39-8b1e-4f9f-8948-d1b887f83eb0">
    <const name="Opaque" value="0" wsmap="managed">
    </const>
    <const name="BGR" value="0x20524742" wsmap="managed">
    </const>
    <const name="BGR0" value="0x30524742" wsmap="managed">
    </const>
    <const name="BGRA" value="0x41524742" wsmap="managed">
    </const>
    <const name="RGBA" value="0x41424752" wsmap="managed">
    </const>
    <const name="PNG" value="0x20474E50" wsmap="managed">
    </const>
    <const name="JPEG" value="0x4745504A" wsmap="managed">
    </const>
  </enum>
  <enum name="PartitioningType" uuid="64c4c806-8908-4c0b-9a51-2d7a0151321f">
    <const name="MBR" value="0" wsmap="managed"/>
    <const name="GPT" value="1" wsmap="managed"/>
  </enum>
  <enum name="PartitionType" uuid="84a6629c-8e9c-474c-adbb-21995671597f">
    <const name="Empty" value="0" wsmap="managed"/>
    <const name="FAT12" value="1" wsmap="managed">  </const>
    <const name="FAT16" value="4" wsmap="managed">  </const>
    <const name="FAT" value="6" wsmap="managed">  </const>
    <const name="IFS" value="7" wsmap="managed">  </const>
    <const name="FAT32CHS" value="11" wsmap="managed"> </const>
    <const name="FAT32LBA" value="12" wsmap="managed"> </const>
    <const name="FAT16B" value="14" wsmap="managed"> </const>
    <const name="Extended" value="15" wsmap="managed"> </const>
    <const name="WindowsRE" value="39" wsmap="managed"> </const>
    <const name="LinuxSwapOld" value="66" wsmap="managed"> </const>
    <const name="LinuxOld" value="67" wsmap="managed"> </const>
    <const name="DragonFlyBSDSlice" value="108" wsmap="managed"/>
    <const name="LinuxSwap" value="130" wsmap="managed"/>
    <const name="Linux" value="131" wsmap="managed"/>
    <const name="LinuxExtended" value="133" wsmap="managed"/>
    <const name="LinuxLVM" value="142" wsmap="managed"/>
    <const name="BSDSlice" value="165" wsmap="managed"/>
    <const name="AppleUFS" value="168" wsmap="managed"/>
    <const name="AppleHFS" value="175" wsmap="managed"/>
    <const name="Solaris" value="191" wsmap="managed"/>
    <const name="GPT" value="238" wsmap="managed"/>
    <const name="EFI" value="239" wsmap="managed"/>
    <const name="Unknown" value="256" wsmap="managed"/>
    <const name="MBR" value="257" wsmap="managed"/>
    <const name="iFFS" value="258" wsmap="managed"/>
    <const name="SonyBoot" value="259" wsmap="managed"/>
    <const name="LenovoBoot" value="260" wsmap="managed"/>
    <const name="WindowsMSR" value="270" wsmap="managed"/>
    <const name="WindowsBasicData" value="271" wsmap="managed"/>
    <const name="WindowsLDMMeta" value="272" wsmap="managed"/>
    <const name="WindowsLDMData" value="273" wsmap="managed"/>
    <const name="WindowsRecovery" value="274" wsmap="managed"/>
    <const name="WindowsStorageSpaces" value="276" wsmap="managed"/>
    <const name="WindowsStorageReplica" value="277" wsmap="managed"/>
    <const name="IBMGPFS" value="275" wsmap="managed"/>
    <const name="LinuxData" value="300" wsmap="managed"/>
    <const name="LinuxRAID" value="301" wsmap="managed"/>
    <const name="LinuxRootX86" value="302" wsmap="managed"/>
    <const name="LinuxRootAMD64" value="303" wsmap="managed"/>
    <const name="LinuxRootARM32" value="304" wsmap="managed"/>
    <const name="LinuxRootARM64" value="305" wsmap="managed"/>
    <const name="LinuxHome" value="306" wsmap="managed"/>
    <const name="LinuxSrv" value="307" wsmap="managed"/>
    <const name="LinuxPlainDmCrypt" value="308" wsmap="managed"/>
    <const name="LinuxLUKS" value="309" wsmap="managed"/>
    <const name="LinuxReserved" value="310" wsmap="managed"/>
    <const name="FreeBSDBoot" value="330" wsmap="managed"/>
    <const name="FreeBSDData" value="331" wsmap="managed"/>
    <const name="FreeBSDSwap" value="332" wsmap="managed"/>
    <const name="FreeBSDUFS" value="333" wsmap="managed"/>
    <const name="FreeBSDVinum" value="334" wsmap="managed"/>
    <const name="FreeBSDZFS" value="335" wsmap="managed"/>
    <const name="FreeBSDUnknown" value="359" wsmap="managed"/>
    <const name="AppleHFSPlus" value="360" wsmap="managed"/>
    <const name="AppleAPFS" value="361" wsmap="managed"/>
    <const name="AppleRAID" value="362" wsmap="managed"/>
    <const name="AppleRAIDOffline" value="363" wsmap="managed"/>
    <const name="AppleBoot" value="364" wsmap="managed"/>
    <const name="AppleLabel" value="365" wsmap="managed"/>
    <const name="AppleTvRecovery" value="366" wsmap="managed"/>
    <const name="AppleCoreStorage" value="367" wsmap="managed"/>
    <const name="SoftRAIDStatus" value="370" wsmap="managed"/>
    <const name="SoftRAIDScratch" value="371" wsmap="managed"/>
    <const name="SoftRAIDVolume" value="372" wsmap="managed"/>
    <const name="SoftRAIDCache" value="373" wsmap="managed"/>
    <const name="AppleUnknown" value="389" wsmap="managed"/>
    <const name="SolarisBoot" value="390" wsmap="managed"/>
    <const name="SolarisRoot" value="391" wsmap="managed"/>
    <const name="SolarisSwap" value="392" wsmap="managed"/>
    <const name="SolarisBackup" value="393" wsmap="managed"/>
    <const name="SolarisUsr" value="394" wsmap="managed"/>
    <const name="SolarisVar" value="395" wsmap="managed"/>
    <const name="SolarisHome" value="396" wsmap="managed"/>
    <const name="SolarisAltSector" value="397" wsmap="managed"/>
    <const name="SolarisReserved" value="398" wsmap="managed"/>
    <const name="SolarisUnknown" value="419" wsmap="managed"/>
    <const name="NetBSDSwap" value="420" wsmap="managed"/>
    <const name="NetBSDFFS" value="421" wsmap="managed"/>
    <const name="NetBSDLFS" value="422" wsmap="managed"/>
    <const name="NetBSDRAID" value="423" wsmap="managed"/>
    <const name="NetBSDConcatenated" value="424" wsmap="managed"/>
    <const name="NetBSDEncrypted" value="425" wsmap="managed"/>
    <const name="NetBSDUnknown" value="449" wsmap="managed"/>
    <const name="ChromeOSKernel" value="450" wsmap="managed"/>
    <const name="ChromeOSRootFS" value="451" wsmap="managed"/>
    <const name="ChromeOSFuture" value="452" wsmap="managed"/>
    <const name="ContLnxUsr" value="480" wsmap="managed"/>
    <const name="ContLnxRoot" value="481" wsmap="managed"/>
    <const name="ContLnxReserved" value="482" wsmap="managed"/>
    <const name="ContLnxRootRAID" value="483" wsmap="managed"/>
    <const name="HaikuBFS" value="510" wsmap="managed"/>
    <const name="MidntBSDBoot" value="540" wsmap="managed"/>
    <const name="MidntBSDData" value="541" wsmap="managed"/>
    <const name="MidntBSDSwap" value="542" wsmap="managed"/>
    <const name="MidntBSDUFS" value="543" wsmap="managed"/>
    <const name="MidntBSDVium" value="544" wsmap="managed"/>
    <const name="MidntBSDZFS" value="545" wsmap="managed"/>
    <const name="MidntBSDUnknown" value="569" wsmap="managed"/>
    <const name="OpenBSDData" value="570" wsmap="managed"/>
    <const name="QNXPowerSafeFS" value="600" wsmap="managed"/>
    <const name="Plan9" value="630" wsmap="managed"/>
    <const name="VMWareVMKCore" value="660" wsmap="managed"/>
    <const name="VMWareVMFS" value="661" wsmap="managed"/>
    <const name="VMWareReserved" value="662" wsmap="managed"/>
    <const name="VMWareUnknown" value="689" wsmap="managed"/>
    <const name="AndroidX86Bootloader" value="690" wsmap="managed"/>
    <const name="AndroidX86Bootloader2" value="691" wsmap="managed"/>
    <const name="AndroidX86Boot" value="692" wsmap="managed"/>
    <const name="AndroidX86Recovery" value="693" wsmap="managed"/>
    <const name="AndroidX86Misc" value="694" wsmap="managed"/>
    <const name="AndroidX86Metadata" value="695" wsmap="managed"/>
    <const name="AndroidX86System" value="696" wsmap="managed"/>
    <const name="AndroidX86Cache" value="697" wsmap="managed"/>
    <const name="AndroidX86Data" value="698" wsmap="managed"/>
    <const name="AndroidX86Persistent" value="699" wsmap="managed"/>
    <const name="AndroidX86Vendor" value="700" wsmap="managed"/>
    <const name="AndroidX86Config" value="701" wsmap="managed"/>
    <const name="AndroidX86Factory" value="702" wsmap="managed"/>
    <const name="AndroidX86FactoryAlt" value="703" wsmap="managed"/>
    <const name="AndroidX86Fastboot" value="704" wsmap="managed"/>
    <const name="AndroidX86OEM" value="705" wsmap="managed"/>
    <const name="AndroidARMMeta" value="720" wsmap="managed"/>
    <const name="AndroidARMExt" value="721" wsmap="managed"/>
    <const name="ONIEBoot" value="750" wsmap="managed"/>
    <const name="ONIEConfig" value="751" wsmap="managed"/>
    <const name="PowerPCPrep" value="780" wsmap="managed"/>
    <const name="XDGShrBootConfig" value="810" wsmap="managed"/>
    <const name="CephBlock" value="830" wsmap="managed"/>
    <const name="CephBlockDB" value="831" wsmap="managed"/>
    <const name="CephBlockDBDmc" value="832" wsmap="managed"/>
    <const name="CephBlockDBDmcLUKS" value="833" wsmap="managed"/>
    <const name="CephBlockDmc" value="834" wsmap="managed"/>
    <const name="CephBlockDmcLUKS" value="835" wsmap="managed"/>
    <const name="CephBlockWALog" value="836" wsmap="managed"/>
    <const name="CephBlockWALogDmc" value="837" wsmap="managed"/>
    <const name="CephBlockWALogDmcLUKS" value="838" wsmap="managed"/>
    <const name="CephDisk" value="839" wsmap="managed"/>
    <const name="CephDiskDmc" value="840" wsmap="managed"/>
    <const name="CephJournal" value="841" wsmap="managed"/>
    <const name="CephJournalDmc" value="842" wsmap="managed"/>
    <const name="CephJournalDmcLUKS" value="843" wsmap="managed"/>
    <const name="CephLockbox" value="844" wsmap="managed"/>
    <const name="CephMultipathBlock1" value="845" wsmap="managed"/>
    <const name="CephMultipathBlock2" value="846" wsmap="managed"/>
    <const name="CephMultipathBlockDB" value="847" wsmap="managed"/>
    <const name="CephMultipathBLockWALog" value="848" wsmap="managed"/>
    <const name="CephMultipathJournal" value="849" wsmap="managed"/>
    <const name="CephMultipathOSD" value="851" wsmap="managed"/>
    <const name="CephOSD" value="852" wsmap="managed"/>
    <const name="CephOSDDmc" value="853" wsmap="managed"/>
    <const name="CephOSDDmcLUKS" value="854" wsmap="managed"/>
  </enum>
  <enum name="TrackedObjectState" uuid="a5ee031d-aba8-4a7f-887a-9a1f12e7cc6c">
    <const name="None" value="0" wsmap="managed"/>
    <const name="Alive" value="1" wsmap="managed"/>
    <const name="Deleted" value="2" wsmap="managed"/>
    <const name="Invalid" value="3" wsmap="managed"/>
  </enum>
  <interface name="IVirtualBoxErrorInfo" extends="$errorinfo" uuid="c1bcc6d5-7966-481d-ab0b-d0ed73e28135" supportsErrorInfo="no" wsmap="managed" rest="managed" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="resultCode" type="long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="resultDetail" type="long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="interfaceID" type="uuid" mod="string" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="component" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="text" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="next" type="IVirtualBoxErrorInfo" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="INATNetwork" extends="$unknown" uuid="90b89e78-1a61-4e66-af61-b9d2532e5632" wsmap="managed" rest="managed" reservedMethods="4" reservedAttributes="8" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="networkName" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="enabled" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default"/>
    <attribute name="network" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="gateway" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="IPv6Enabled" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="IPv6Prefix" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="advertiseDefaultIPv6RouteEnabled" type="boolean" dtracename="advertiseDefaultIPv6Route" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default"/>
    <attribute name="needDhcpServer" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default"/>
    <attribute name="eventSource" type="IEventSource" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
    <attribute name="portForwardRules4" type="wstring" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="localMappings" type="wstring" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="localhostReachable" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="forwardBroadcast" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="natMTU" type="unsigned long" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="natMRU" type="unsigned long" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <method name="addLocalMapping" const="no" internal="no" wsmap="managed">
      <rest request="post" path="/natnetwork/{networkname}/config/"/>
      <param name="hostid" type="wstring" dir="in" safearray="no"/>
      <param name="offset" type="long" dir="in" safearray="no"/>
    </method>
    <attribute name="loopbackIp6" type="long" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="portForwardRules6" type="wstring" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <method name="addPortForwardRule" const="no" internal="no" wsmap="managed">
      <rest request="post" path="/natnetwork/{networkname}/config/"/>
      <param name="isIpv6" type="boolean" dir="in" safearray="no"/>
      <param name="ruleName" type="wstring" dir="in" safearray="no"/>
      <param name="proto" type="NATProtocol" dir="in" safearray="no">
      </param>
      <param name="hostIP" type="wstring" dir="in" safearray="no">
      </param>
      <param name="hostPort" type="unsigned short" dir="in" safearray="no">
      </param>
      <param name="guestIP" type="wstring" dir="in" safearray="no">
        </param>
        <param name="guestPort" type="unsigned short" dir="in" safearray="no">
        </param>
    </method>
    <method name="removePortForwardRule" const="no" internal="no" wsmap="managed">
      <rest request="post" path="/natnetwork/{networkname}/config/"/>
      <param name="isIpv6" type="boolean" dir="in" safearray="no"/>
      <param name="ruleName" type="wstring" dir="in" safearray="no"/>
    </method>
    <method name="start" const="no" internal="no" wsmap="managed">
      <rest request="post" path="/natnetwork/{networkname}/action/"/>
    </method>
    <method name="stop" const="no" internal="no" wsmap="managed">
      <rest request="post" path="/natnetwork/{networkname}/action/"/>
    </method>
  </interface>
  <interface name="ICloudNetwork" extends="$unknown" uuid="d8e3496e-735f-4fde-8a54-427d49409b5f" wsmap="managed" rest="managed" reservedMethods="4" reservedAttributes="12" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="networkName" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="enabled" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default"/>
    <attribute name="provider" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="profile" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="networkId" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IHostOnlyNetwork" extends="$unknown" uuid="6eb5de7d-9a67-4fde-83be-f768084d03b5" wsmap="managed" rest="managed" reservedMethods="4" reservedAttributes="8" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="networkName" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="enabled" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default"/>
    <attribute name="networkMask" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="hostIP" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="lowerIP" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="upperIP" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="id" type="uuid" mod="string" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <enum name="DHCPOption" uuid="00f5b10f-0021-4513-00f7-5bf4000982bf">
    <const name="SubnetMask" value="1" wsmap="managed"/>
    <const name="TimeOffset" value="2" wsmap="managed"/>
    <const name="Routers" value="3" wsmap="managed"/>
    <const name="TimeServers" value="4" wsmap="managed"/>
    <const name="NameServers" value="5" wsmap="managed"/>
    <const name="DomainNameServers" value="6" wsmap="managed"/>
    <const name="LogServers" value="7" wsmap="managed"/>
    <const name="CookieServers" value="8" wsmap="managed"/>
    <const name="LPRServers" value="9" wsmap="managed"/>
    <const name="ImpressServers" value="10" wsmap="managed"/>
    <const name="ResourseLocationServers" value="11" wsmap="managed"/>
    <const name="HostName" value="12" wsmap="managed"/>
    <const name="BootFileSize" value="13" wsmap="managed"/>
    <const name="MeritDumpFile" value="14" wsmap="managed"/>
    <const name="DomainName" value="15" wsmap="managed"/>
    <const name="SwapServer" value="16" wsmap="managed"/>
    <const name="RootPath" value="17" wsmap="managed"/>
    <const name="ExtensionPath" value="18" wsmap="managed"/>
    <const name="IPForwarding" value="19" wsmap="managed"/>
    <const name="OptNonLocalSourceRouting" value="20" wsmap="managed"/>
    <const name="PolicyFilter" value="21" wsmap="managed"/>
    <const name="MaxDgramReassemblySize" value="22" wsmap="managed"/>
    <const name="DefaultIPTTL" value="23" wsmap="managed"/>
    <const name="PathMTUAgingTimeout" value="24" wsmap="managed"/>
    <const name="PathMTUPlateauTable" value="25" wsmap="managed"/>
    <const name="InterfaceMTU" value="26" wsmap="managed"/>
    <const name="AllSubnetsAreLocal" value="27" wsmap="managed"/>
    <const name="BroadcastAddress" value="28" wsmap="managed"/>
    <const name="PerformMaskDiscovery" value="29" wsmap="managed"/>
    <const name="MaskSupplier" value="30" wsmap="managed"/>
    <const name="PerformRouterDiscovery" value="31" wsmap="managed"/>
    <const name="RouterSolicitationAddress" value="32" wsmap="managed"/>
    <const name="StaticRoute" value="33" wsmap="managed"/>
    <const name="TrailerEncapsulation" value="34" wsmap="managed"/>
    <const name="ARPCacheTimeout" value="35" wsmap="managed"/>
    <const name="EthernetEncapsulation" value="36" wsmap="managed"/>
    <const name="TCPDefaultTTL" value="37" wsmap="managed"/>
    <const name="TCPKeepaliveInterval" value="38" wsmap="managed"/>
    <const name="TCPKeepaliveGarbage" value="39" wsmap="managed"/>
    <const name="NISDomain" value="40" wsmap="managed"/>
    <const name="NISServers" value="41" wsmap="managed"/>
    <const name="NTPServers" value="42" wsmap="managed"/>
    <const name="VendorSpecificInfo" value="43" wsmap="managed"/>
    <const name="NetBIOSNameServers" value="44" wsmap="managed"/>
    <const name="NetBIOSDatagramServers" value="45" wsmap="managed"/>
    <const name="NetBIOSNodeType" value="46" wsmap="managed"/>
    <const name="NetBIOSScope" value="47" wsmap="managed"/>
    <const name="XWindowsFontServers" value="48" wsmap="managed"/>
    <const name="XWindowsDisplayManager" value="49" wsmap="managed"/>
    <const name="NetWareIPDomainName" value="62" wsmap="managed"/>
    <const name="NetWareIPInformation" value="63" wsmap="managed"/>
    <const name="NISPlusDomain" value="64" wsmap="managed"/>
    <const name="NISPlusServers" value="65" wsmap="managed"/>
    <const name="TFTPServerName" value="66" wsmap="managed"/>
    <const name="BootfileName" value="67" wsmap="managed"/>
    <const name="MobileIPHomeAgents" value="68" wsmap="managed"/>
    <const name="SMTPServers" value="69" wsmap="managed"/>
    <const name="POP3Servers" value="70" wsmap="managed"/>
    <const name="NNTPServers" value="71" wsmap="managed"/>
    <const name="WWWServers" value="72" wsmap="managed"/>
    <const name="FingerServers" value="73" wsmap="managed"/>
    <const name="IRCServers" value="74" wsmap="managed"/>
    <const name="StreetTalkServers" value="75" wsmap="managed"/>
    <const name="STDAServers" value="76" wsmap="managed"/>
    <const name="SLPDirectoryAgent" value="78" wsmap="managed"/>
    <const name="SLPServiceScope" value="79" wsmap="managed"/>
    <const name="DomainSearch" value="119" wsmap="managed"/>
  </enum>
  <enum name="DHCPOptionEncoding" uuid="84b6d460-2838-4682-c0d6-ef5b573ef28a">
    <const name="Normal" value="0" wsmap="managed">
    </const>
    <const name="Hex" value="1" wsmap="managed">
    </const>
  </enum>
  <enum name="DHCPConfigScope" uuid="469c42e4-b9ec-43f2-bdcb-9e9d1eb434ae">
    <const name="Global" value="0" wsmap="managed"/>
    <const name="Group" value="1" wsmap="managed"/>
    <const name="MachineNIC" value="2" wsmap="managed"/>
    <const name="MAC" value="3" wsmap="managed"/>
  </enum>
  <enum name="DHCPGroupConditionType" uuid="2cb9280f-ada2-4194-dee8-bfb8ad77119d">
    <const name="MAC" value="0" wsmap="managed"/>
    <const name="MACWildcard" value="1" wsmap="managed"/>
    <const name="vendorClassID" value="2" wsmap="managed"/>
    <const name="vendorClassIDWildcard" value="3" wsmap="managed"/>
    <const name="userClassID" value="4" wsmap="managed"/>
    <const name="userClassIDWildcard" value="5" wsmap="managed"/>
  </enum>
  <interface name="IDHCPServer" extends="$unknown" uuid="cadef0a2-a1a9-4ac2-8e80-c049af69dac8" wsmap="managed" rest="managed" reservedMethods="0" reservedAttributes="3" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="eventSource" type="IEventSource" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
    <attribute name="enabled" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="IPAddress" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="networkMask" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="networkName" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="lowerIP" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="upperIP" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="globalConfig" type="IDHCPGlobalConfig" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="groupConfigs" type="IDHCPGroupConfig" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="individualConfigs" type="IDHCPIndividualConfig" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <method name="setConfiguration" const="no" internal="no" wsmap="managed">
      <rest request="post" path="/dhcpserver/{networkname}/config/"/>
      <param name="IPAddress" type="wstring" dir="in" safearray="no">
      </param>
      <param name="networkMask" type="wstring" dir="in" safearray="no">
      </param>
      <param name="FromIPAddress" type="wstring" dir="in" safearray="no">
      </param>
      <param name="ToIPAddress" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <method name="start" const="no" internal="no" wsmap="managed">
      <rest request="post" path="/dhcpserver/{networkname}/action/"/>
      <param name="trunkName" type="wstring" dir="in" safearray="no">
      </param>
      <param name="trunkType" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <method name="stop" const="no" internal="no" wsmap="managed">
      <rest request="post" path="/dhcpserver/{networkname}/action/"/>
    </method>
    <method name="restart" const="no" internal="no" wsmap="managed">
      <rest request="post" path="/dhcpserver/{networkname}/action/"/>
    </method>
    <method name="findLeaseByMAC" const="no" internal="no" wsmap="managed">
      <param name="mac" type="wstring" dir="in" safearray="no">
      </param>
      <param name="type" type="long" dir="in" safearray="no">
      </param>
      <param name="address" type="wstring" dir="out" safearray="no">
      </param>
      <param name="state" type="wstring" dir="out" safearray="no">
      </param>
      <param name="issued" type="long long" dir="out" safearray="no">
      </param>
      <param name="expire" type="long long" dir="out" safearray="no">
      </param>
    </method>
    <method name="getConfig" const="no" internal="no" wsmap="managed">
      <rest request="get" path="/dhcpserver/{networkname}/configuration/"/>
      <param name="scope" dir="in" type="DHCPConfigScope" safearray="no">
      </param>
      <param name="name" dir="in" type="wstring" safearray="no">
      </param>
      <param name="slot" dir="in" type="unsigned long" safearray="no">
      </param>
      <param name="mayAdd" dir="in" type="boolean" safearray="no">
      </param>
      <param name="config" dir="return" type="IDHCPConfig" safearray="no">
      </param>
    </method>
  </interface>
  <interface name="IDHCPConfig" extends="$unknown" uuid="00f4a8dc-0002-4b81-0077-1dcb004571ba" wsmap="managed" rest="managed" reservedMethods="8" reservedAttributes="16" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="scope" type="DHCPConfigScope" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="minLeaseTime" type="unsigned long" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="defaultLeaseTime" type="unsigned long" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="maxLeaseTime" type="unsigned long" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="forcedOptions" type="DHCPOption" safearray="yes" readonly="no" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="suppressedOptions" type="DHCPOption" safearray="yes" readonly="no" internal="no" wsmap="managed" rest="default">
    </attribute>
    <method name="setOption" const="no" internal="no" wsmap="managed">
      <param name="option" dir="in" type="DHCPOption" safearray="no">
      </param>
      <param name="encoding" dir="in" type="DHCPOptionEncoding" safearray="no">
      </param>
      <param name="value" dir="in" type="wstring" safearray="no">
      </param>
    </method>
    <method name="removeOption" const="no" internal="no" wsmap="managed">
      <param name="option" dir="in" type="DHCPOption" safearray="no"/>
    </method>
    <method name="removeAllOptions" const="no" internal="no" wsmap="managed">
    </method>
    <method name="getOption" const="no" internal="no" wsmap="managed">
      <param name="option" dir="in" type="DHCPOption" safearray="no">
      </param>
      <param name="encoding" dir="out" type="DHCPOptionEncoding" safearray="no">
      </param>
      <param name="value" dir="return" type="wstring" safearray="no">
      </param>
    </method>
    <method name="getAllOptions" const="no" internal="no" wsmap="managed">
      <param name="options" dir="out" type="DHCPOption" safearray="yes">
      </param>
      <param name="encodings" dir="out" type="DHCPOptionEncoding" safearray="yes">
      </param>
      <param name="values" dir="return" type="wstring" safearray="yes">
      </param>
    </method>
    <method name="remove" const="no" internal="no" wsmap="managed">
    </method>
  </interface>
  <interface name="IDHCPGlobalConfig" extends="IDHCPConfig" uuid="46735de7-f4c4-4020-a185-0d2881bcfa8b" wsmap="managed" rest="managed" reservedMethods="4" reservedAttributes="4" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
  </interface>
  <interface name="IDHCPGroupCondition" extends="$unknown" uuid="5ca9e537-5a1d-43f1-6f27-6a0db298a9a8" wsmap="managed" rest="managed" reservedMethods="3" reservedAttributes="3" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="inclusive" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="type" type="DHCPGroupConditionType" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="value" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <method name="remove" const="no" internal="no" wsmap="managed">
    </method>
  </interface>
  <interface name="IDHCPGroupConfig" extends="IDHCPConfig" uuid="537707f7-ebf9-4d5c-7aea-877bfc4256ba" wsmap="managed" rest="managed" reservedMethods="8" reservedAttributes="8" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="name" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="conditions" type="IDHCPGroupCondition" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <method name="addCondition" const="no" internal="no" wsmap="managed">
      <param name="inclusive" dir="in" type="boolean" safearray="no"/>
      <param name="type" dir="in" type="DHCPGroupConditionType" safearray="no"/>
      <param name="value" dir="in" type="wstring" safearray="no"/>
      <param name="condition" dir="return" type="IDHCPGroupCondition" safearray="no"/>
    </method>
    <method name="removeAllConditions" const="no" internal="no" wsmap="managed">
    </method>
  </interface>
  <interface name="IDHCPIndividualConfig" extends="IDHCPConfig" uuid="c40c2b86-73a5-46cc-8227-93fe57d006a6" wsmap="managed" rest="managed" reservedMethods="8" reservedAttributes="8" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="MACAddress" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="machineId" type="uuid" mod="string" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="slot" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="fixedAddress" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IVirtualBox" extends="$unknown" uuid="2ce10519-3c09-45d8-a12d-e887786146b7" wsmap="managed" rest="managed" wrap-gen-hook="yes" reservedMethods="6" reservedAttributes="12" default="no" internal="no" wscpp="generate" waitable="no" notdual="no">
    <attribute name="version" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="versionNormalized" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="revision" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="packageType" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="APIVersion" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="APIRevision" type="long long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="homeFolder" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="settingsFilePath" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="host" type="IHost" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="systemProperties" type="ISystemProperties" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="machines" type="IMachine" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="machineGroups" type="wstring" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="hardDisks" type="IMedium" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="DVDImages" type="IMedium" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="floppyImages" type="IMedium" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="progressOperations" type="IProgress" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default"/>
    <attribute name="guestOSTypes" type="IGuestOSType" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="guestOSFamilies" type="wstring" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="sharedFolders" type="ISharedFolder" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="performanceCollector" type="IPerformanceCollector" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="DHCPServers" type="IDHCPServer" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="NATNetworks" type="INATNetwork" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default"/>
    <attribute name="eventSource" type="IEventSource" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="extensionPackManager" type="IExtPackManager" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="internalNetworks" type="wstring" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="hostOnlyNetworks" type="IHostOnlyNetwork" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="genericNetworkDrivers" type="wstring" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="cloudNetworks" type="ICloudNetwork" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="cloudProviderManager" type="ICloudProviderManager" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <method name="composeMachineFilename" const="no" internal="no" wsmap="managed">
      <rest request="get" path="/server/method/"/>
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="group" type="wstring" dir="in" safearray="no">
      </param>
      <param name="createFlags" type="wstring" dir="in" safearray="no">
      </param>
      <param name="baseFolder" type="wstring" dir="in" safearray="no">
      </param>
      <param name="file" type="wstring" dir="return" safearray="no">
      </param>
    </method>
    <method name="getPlatformProperties" const="no" internal="no" wsmap="managed">
      <rest name="find" request="get" path="/platform/property/"/>
      <param name="architecture" type="PlatformArchitecture" dir="in" safearray="no">
      </param>
      <param name="properties" type="IPlatformProperties" dir="return" safearray="no">
      </param>
    </method>
    <method name="createMachine" const="no" internal="no" wsmap="managed">
      <rest name="create" request="post" path="/vm/"/>
      <param name="settingsFile" type="wstring" dir="in" safearray="no">
      </param>
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="platform" type="PlatformArchitecture" dir="in" safearray="no">
      </param>
      <param name="groups" type="wstring" safearray="yes" dir="in">
      </param>
      <param name="osTypeId" type="wstring" dir="in" safearray="no">
      </param>
      <param name="flags" type="wstring" dir="in" safearray="no">
      </param>
      <param name="cipher" type="wstring" dir="in" safearray="no">
      </param>
      <param name="passwordId" type="wstring" dir="in" safearray="no">
      </param>
      <param name="password" type="wstring" dir="in" safearray="no">
      </param>
      <param name="machine" type="IMachine" dir="return" safearray="no">
      </param>
    </method>
    <method name="openMachine" const="no" internal="no" wsmap="managed">
      <param name="settingsFile" type="wstring" dir="in" safearray="no">
      </param>
      <param name="password" type="wstring" dir="in" safearray="no">
      </param>
      <param name="machine" type="IMachine" dir="return" safearray="no">
      </param>
    </method>
    <method name="registerMachine" const="no" internal="no" wsmap="managed">
      <param name="machine" type="IMachine" dir="in" safearray="no"/>
    </method>
    <method name="findMachine" const="yes" internal="no" wsmap="managed">
      <rest name="find" request="get" path="/vm/{vmid}/"/>
      <param name="nameOrId" type="wstring" dir="in" safearray="no">
      </param>
      <param name="machine" type="IMachine" dir="return" safearray="no">
      </param>
    </method>
    <method name="getMachinesByGroups" const="no" internal="no" wsmap="managed">
      <rest request="get" path="/server/method/"/>
      <param name="groups" type="wstring" dir="in" safearray="yes">
      </param>
      <param name="machines" type="IMachine" dir="return" safearray="yes">
      </param>
    </method>
    <method name="getMachineStates" const="no" internal="no" wsmap="managed">
      <rest request="get" path="/server/method/"/>
      <param name="machines" type="IMachine" dir="in" safearray="yes">
      </param>
      <param name="states" type="MachineState" dir="return" safearray="yes">
      </param>
    </method>
    <method name="createAppliance" const="no" internal="no" wsmap="managed">
     <rest name="create" request="post" path="/appliance/"/>
      <param name="appliance" type="IAppliance" dir="return" safearray="no">
      </param>
    </method>
    <method name="createUnattendedInstaller" const="no" internal="no" wsmap="managed">
      <rest request="post" path="/server/method/"/>
      <param name="unattended" type="IUnattended" dir="return" safearray="no">
      </param>
    </method>
    <method name="createMedium" const="no" internal="no" wsmap="managed">
      <rest name="create" request="post" path="/media/"/>
      <param name="format" type="wstring" dir="in" safearray="no">
      </param>
      <param name="location" type="wstring" dir="in" safearray="no">
      </param>
      <param name="accessMode" type="AccessMode" dir="in" safearray="no">
      </param>
      <param name="aDeviceTypeType" type="DeviceType" dir="in" safearray="no">
      </param>
      <param name="medium" type="IMedium" dir="return" safearray="no">
      </param>
    </method>
    <method name="openMedium" const="no" internal="no" wsmap="managed">
      <rest request="post" path="/server/method/"/>
      <param name="location" type="wstring" dir="in" safearray="no">
      </param>
      <param name="deviceType" type="DeviceType" dir="in" safearray="no">
      </param>
      <param name="accessMode" type="AccessMode" dir="in" safearray="no">
      </param>
      <param name="forceNewUuid" type="boolean" dir="in" safearray="no">
        </param>
      <param name="medium" type="IMedium" dir="return" safearray="no">
      </param>
    </method>
    <method name="getGuestOSType" const="no" internal="no" wsmap="managed">
      <rest name="find" request="get" path="/guest/os/type/"/>
      <param name="id" type="wstring" dir="in" safearray="no">
      </param>
      <param name="type" type="IGuestOSType" dir="return" safearray="no">
      </param>
    </method>
    <method name="getGuestOSSubtypesByFamilyId" const="no" internal="no" wsmap="managed">
      <rest name="list" request="get" path="/guest/os/subtype/"/>
      <param name="family" type="wstring" dir="in" safearray="no">
      </param>
      <param name="OSsubtypes" type="wstring" safearray="yes" dir="return">
      </param>
    </method>
    <method name="getGuestOSDescsBySubtype" const="no" internal="no" wsmap="managed">
      <rest name="list" request="get" path="/guest/os/subtype/desc/"/>
      <param name="OSSubtype" type="wstring" dir="in" safearray="no">
      </param>
      <param name="GuestOSDescs" type="wstring" safearray="yes" dir="return">
      </param>
    </method>
    <method name="createSharedFolder" const="no" internal="no" wsmap="managed">
      <rest name="create" request="post" path="/sharedfolder/"/>
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="hostPath" type="wstring" dir="in" safearray="no">
      </param>
      <param name="writable" type="boolean" dir="in" safearray="no">
      </param>
      <param name="automount" type="boolean" dir="in" safearray="no">
      </param>
      <param name="autoMountPoint" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <method name="removeSharedFolder" const="no" internal="no" wsmap="managed">
      <rest name="remove" request="delete" path="/sharedfolder/"/>
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <method name="getExtraDataKeys" const="no" internal="no" wsmap="managed">
      <rest request="get" path="/server/method/"/>
      <param name="keys" type="wstring" dir="return" safearray="yes">
      </param>
    </method>
    <method name="getExtraData" const="no" internal="no" wsmap="managed">
      <rest request="get" path="/server/method/"/>
      <param name="key" type="wstring" dir="in" safearray="no">
      </param>
      <param name="value" type="wstring" dir="return" safearray="no">
      </param>
    </method>
    <method name="setExtraData" const="no" internal="no" wsmap="managed">
      <rest request="put" path="/server/method/"/>
      <param name="key" type="wstring" dir="in" safearray="no">
      </param>
      <param name="value" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <method name="setSettingsSecret" const="no" internal="no" wsmap="managed">
      <param name="password" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <method name="createDHCPServer" const="no" internal="no" wsmap="managed">
      <rest name="create" request="post" path="/dhcpserver/"/>
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="server" type="IDHCPServer" dir="return" safearray="no">
      </param>
    </method>
    <method name="findDHCPServerByNetworkName" const="no" internal="no" wsmap="managed">
      <rest name="find" request="get" path="/dhcpserver/"/>
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="server" type="IDHCPServer" dir="return" safearray="no">
      </param>
    </method>
    <method name="removeDHCPServer" const="no" internal="no" wsmap="managed">
      <rest name="remove" request="delete" path="/dhcpserver/"/>
      <param name="server" type="IDHCPServer" dir="in" safearray="no">
      </param>
    </method>
    <method name="createNATNetwork" const="no" internal="no" wsmap="managed">
      <rest name="create" request="post" path="/natnetwork/"/>
      <param name="networkName" type="wstring" dir="in" safearray="no"/>
      <param name="network" type="INATNetwork" dir="return" safearray="no"/>
    </method>
    <method name="findNATNetworkByName" const="no" internal="no" wsmap="managed">
      <rest name="find" request="get" path="/natnetwork/"/>
      <param name="networkName" type="wstring" dir="in" safearray="no"/>
      <param name="network" type="INATNetwork" dir="return" safearray="no"/>
    </method>
    <method name="removeNATNetwork" const="no" internal="no" wsmap="managed">
      <rest name="remove" request="delete" path="/natnetwork/"/>
      <param name="network" type="INATNetwork" dir="in" safearray="no"/>
    </method>
    <method name="createHostOnlyNetwork" const="no" internal="no" wsmap="managed">
      <rest name="create" request="post" path="/hostonlynetwork/"/>
      <param name="networkName" type="wstring" dir="in" safearray="no"/>
      <param name="network" type="IHostOnlyNetwork" dir="return" safearray="no"/>
    </method>
    <method name="findHostOnlyNetworkByName" const="no" internal="no" wsmap="managed">
      <param name="networkName" type="wstring" dir="in" safearray="no"/>
      <param name="network" type="IHostOnlyNetwork" dir="return" safearray="no"/>
    </method>
    <method name="findHostOnlyNetworkById" const="no" internal="no" wsmap="managed">
      <rest name="find" request="get" path="/hostonlynetwork/"/>
      <param name="id" type="uuid" mod="string" dir="in" safearray="no">
      </param>
      <param name="network" type="IHostOnlyNetwork" dir="return" safearray="no">
      </param>
    </method>
    <method name="removeHostOnlyNetwork" const="no" internal="no" wsmap="managed">
      <rest name="remove" request="delete" path="/hostonlynetwork/"/>
      <param name="network" type="IHostOnlyNetwork" dir="in" safearray="no"/>
    </method>
    <method name="createCloudNetwork" const="no" internal="no" wsmap="managed">
      <rest name="create" request="post" path="/cloudnetwork/"/>
      <param name="networkName" type="wstring" dir="in" safearray="no"/>
      <param name="network" type="ICloudNetwork" dir="return" safearray="no"/>
    </method>
    <method name="findCloudNetworkByName" const="no" internal="no" wsmap="managed">
      <rest name="find" request="get" path="/cloudnetwork/"/>
      <param name="networkName" type="wstring" dir="in" safearray="no"/>
      <param name="network" type="ICloudNetwork" dir="return" safearray="no"/>
    </method>
    <method name="removeCloudNetwork" const="no" internal="no" wsmap="managed">
      <rest name="remove" request="delete" path="/cloudnetwork/"/>
      <param name="network" type="ICloudNetwork" dir="in" safearray="no"/>
    </method>
    <method name="checkFirmwarePresent" const="no" internal="no" wsmap="managed">
      <rest name="support" request="get" path="/server/firmware/"/>
      <param name="platformArchitecture" type="PlatformArchitecture" dir="in" safearray="no">
      </param>
      <param name="firmwareType" type="FirmwareType" dir="in" safearray="no">
      </param>
      <param name="version" type="wstring" dir="in" safearray="no">
      </param>
      <param name="url" type="wstring" dir="out" safearray="no">
      </param>
      <param name="file" type="wstring" dir="out" safearray="no">
      </param>
      <param name="result" type="boolean" dir="return" safearray="no">
      </param>
    </method>
    <method name="findProgressById" const="no" internal="no" wsmap="managed">
      <rest name="find" request="get" path="/progress/{progressid}/"/>
      <param name="id" type="uuid" mod="string" dir="in" safearray="no">
      </param>
      <param name="progressObject" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="getTrackedObject" const="no" internal="no" wsmap="managed">
      <rest request="get" path="/server/methods/"/>
      <param name="trObjId" type="wstring" dir="in" safearray="no">
      </param>
      <param name="pIface" type="$unknown" dir="out" safearray="no">
      </param>
      <param name="state" type="TrackedObjectState" dir="out" safearray="no">
      </param>
      <param name="creationTime" type="long long" dir="out" safearray="no">
      </param>
      <param name="deletionTime" type="long long" dir="out" safearray="no">
      </param>
    </method>
    <method name="getTrackedObjectIds" const="no" internal="no" wsmap="managed">
      <rest request="get" path="/server/methods/"/>
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="objIdsList" type="wstring" safearray="yes" dir="return">
      </param>
    </method>
  </interface>
  <enum name="VFSType" uuid="813999ba-b949-48a8-9230-aadc6285e2f2">
    <const name="File" value="1" wsmap="managed"/>
    <const name="Cloud" value="2" wsmap="managed"/>
    <const name="S3" value="3" wsmap="managed"/>
    <const name="WebDav" value="4" wsmap="managed"/>
  </enum>
  <interface name="IVFSExplorer" extends="$unknown" uuid="fb220201-2fd3-47e2-a5dc-2c2431d833cc" wsmap="managed" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="path" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="type" type="VFSType" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <method name="update" const="no" internal="no" wsmap="managed">
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="cd" const="no" internal="no" wsmap="managed">
      <param name="dir" type="wstring" dir="in" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="cdUp" const="no" internal="no" wsmap="managed">
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="entryList" const="no" internal="no" wsmap="managed">
      <param name="names" type="wstring" safearray="yes" dir="out">
      </param>
      <param name="types" type="unsigned long" safearray="yes" dir="out">
      </param>
      <param name="sizes" type="long long" safearray="yes" dir="out">
      </param>
      <param name="modes" type="unsigned long" safearray="yes" dir="out">
      </param>
    </method>
    <method name="exists" const="no" internal="no" wsmap="managed">
      <param name="names" type="wstring" safearray="yes" dir="in">
      </param>
      <param name="exists" type="wstring" safearray="yes" dir="return">
      </param>
    </method>
    <method name="remove" const="no" internal="no" wsmap="managed">
      <param name="names" type="wstring" safearray="yes" dir="in">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
  </interface>
  <enum name="ImportOptions" uuid="0a981523-3b20-4004-8ee3-dfd322202ace">
    <const name="KeepAllMACs" value="1" wsmap="managed">
    </const>
    <const name="KeepNATMACs" value="2" wsmap="managed">
    </const>
    <const name="ImportToVDI" value="3" wsmap="managed">
    </const>
  </enum>
  <enum name="ExportOptions" uuid="8f45eb08-fd34-41ee-af95-a880bdee5554">
    <const name="CreateManifest" value="1" wsmap="managed">
    </const>
    <const name="ExportDVDImages" value="2" wsmap="managed">
    </const>
    <const name="StripAllMACs" value="3" wsmap="managed">
    </const>
    <const name="StripAllNonNATMACs" value="4" wsmap="managed">
    </const>
  </enum>
  <enum name="CertificateVersion" uuid="9e232a99-51d0-4dbd-96a0-ffac4bc3e2a8">
    <const name="V1" value="1" wsmap="managed"/>
    <const name="V2" value="2" wsmap="managed"/>
    <const name="V3" value="3" wsmap="managed"/>
    <const name="Unknown" value="99" wsmap="managed"/>
  </enum>
  <interface name="ICertificate" extends="$unknown" uuid="392f1de4-80e1-4a8a-93a1-67c5f92a838a" wsmap="managed" rest="managed" reservedAttributes="12" reservedMethods="2" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="versionNumber" type="CertificateVersion" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="serialNumber" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="signatureAlgorithmOID" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="signatureAlgorithmName" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="issuerName" type="wstring" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="subjectName" type="wstring" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="friendlyName" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="validityPeriodNotBefore" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="validityPeriodNotAfter" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="publicKeyAlgorithmOID" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="publicKeyAlgorithm" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="subjectPublicKey" type="octet" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="issuerUniqueIdentifier" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="subjectUniqueIdentifier" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="certificateAuthority" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="keyUsage" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="extendedKeyUsage" type="wstring" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="rawCertData" type="octet" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="selfSigned" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="trusted" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="expired" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <method name="isCurrentlyExpired" const="no" internal="no" wsmap="managed">
      <param name="result" type="boolean" dir="return" safearray="no"/>
    </method>
    <method name="queryInfo" const="no" internal="no" wsmap="managed">
      <param name="what" type="long" dir="in" safearray="no"/>
      <param name="result" type="wstring" dir="return" safearray="no"/>
    </method>
  </interface>
  <interface name="IAppliance" extends="$unknown" uuid="86a98347-7619-41aa-aece-b21ac5c1a7e6" wsmap="managed" rest="managed" reservedMethods="7" reservedAttributes="8" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="path" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="disks" type="wstring" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="virtualSystemDescriptions" type="IVirtualSystemDescription" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="machines" type="wstring" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="certificate" type="ICertificate" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <method name="read" const="no" internal="no" wsmap="managed">
      <param name="file" type="wstring" dir="in" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="interpret" const="no" internal="no" wsmap="managed">
    </method>
    <method name="importMachines" const="no" internal="no" wsmap="managed">
      <param name="options" type="ImportOptions" dir="in" safearray="yes">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="createVFSExplorer" const="no" internal="no" wsmap="managed">
      <param name="URI" type="wstring" dir="in" safearray="no">
      </param>
      <param name="explorer" type="IVFSExplorer" dir="return" safearray="no">
      </param>
    </method>
    <method name="write" const="no" internal="no" wsmap="managed">
      <param name="format" type="wstring" dir="in" safearray="no">
      </param>
      <param name="options" type="ExportOptions" dir="in" safearray="yes">
      </param>
      <param name="path" type="wstring" dir="in" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="getWarnings" const="no" internal="no" wsmap="managed">
      <param name="warnings" type="wstring" dir="return" safearray="yes">
      </param>
    </method>
    <method name="getPasswordIds" const="no" internal="no" wsmap="managed">
      <param name="identifiers" type="wstring" dir="return" safearray="yes">
      </param>
    </method>
    <method name="getMediumIdsForPasswordId" const="no" internal="no" wsmap="managed">
      <param name="passwordId" type="wstring" dir="in" safearray="no">
      </param>
      <param name="identifiers" type="uuid" mod="string" dir="return" safearray="yes">
      </param>
    </method>
    <method name="addPasswords" const="no" internal="no" wsmap="managed">
      <param name="identifiers" type="wstring" dir="in" safearray="yes">
      </param>
      <param name="passwords" type="wstring" dir="in" safearray="yes">
      </param>
    </method>
    <method name="createVirtualSystemDescriptions" const="no" internal="no" wsmap="managed">
      <param name="requested" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="created" type="unsigned long" dir="return" safearray="no">
      </param>
    </method>
  </interface>
  <enum name="VirtualSystemDescriptionType" uuid="d171d08c-f7bf-4bee-932f-ffbf998f7ac4">
    <const name="Ignore" value="1" wsmap="managed"/>
    <const name="OS" value="2" wsmap="managed"/>
    <const name="Name" value="3" wsmap="managed"/>
    <const name="Product" value="4" wsmap="managed"/>
    <const name="Vendor" value="5" wsmap="managed"/>
    <const name="Version" value="6" wsmap="managed"/>
    <const name="ProductUrl" value="7" wsmap="managed"/>
    <const name="VendorUrl" value="8" wsmap="managed"/>
    <const name="Description" value="9" wsmap="managed"/>
    <const name="License" value="10" wsmap="managed"/>
    <const name="Miscellaneous" value="11" wsmap="managed"/>
    <const name="CPU" value="12" wsmap="managed"/>
    <const name="Memory" value="13" wsmap="managed"/>
    <const name="HardDiskControllerIDE" value="14" wsmap="managed"/>
    <const name="HardDiskControllerSATA" value="15" wsmap="managed"/>
    <const name="HardDiskControllerSCSI" value="16" wsmap="managed"/>
    <const name="HardDiskControllerSAS" value="17" wsmap="managed"/>
    <const name="HardDiskImage" value="18" wsmap="managed"/>
    <const name="Floppy" value="19" wsmap="managed"/>
    <const name="CDROM" value="20" wsmap="managed"/>
    <const name="NetworkAdapter" value="21" wsmap="managed"/>
    <const name="USBController" value="22" wsmap="managed"/>
    <const name="SoundCard" value="23" wsmap="managed"/>
    <const name="SettingsFile" value="24" wsmap="managed">
    </const>
    <const name="BaseFolder" value="25" wsmap="managed">
    </const>
    <const name="PrimaryGroup" value="26" wsmap="managed">
    </const>
    <const name="CloudInstanceShape" value="27" wsmap="managed"/>
    <const name="CloudDomain" value="28" wsmap="managed"/>
    <const name="CloudBootDiskSize" value="29" wsmap="managed"/>
    <const name="CloudBucket" value="30" wsmap="managed"/>
    <const name="CloudOCIVCN" value="31" wsmap="managed"/>
    <const name="CloudPublicIP" value="32" wsmap="managed"/>
    <const name="CloudProfileName" value="33" wsmap="managed"/>
    <const name="CloudOCISubnet" value="34" wsmap="managed"/>
    <const name="CloudKeepObject" value="35" wsmap="managed"/>
    <const name="CloudLaunchInstance" value="36" wsmap="managed"/>
    <const name="CloudInstanceId" value="37" wsmap="managed"/>
    <const name="CloudImageId" value="38" wsmap="managed"/>
    <const name="CloudInstanceState" value="39" wsmap="managed"/>
    <const name="CloudImageState" value="40" wsmap="managed"/>
    <const name="CloudInstanceDisplayName" value="41" wsmap="managed"/>
    <const name="CloudImageDisplayName" value="42" wsmap="managed"/>
    <const name="CloudOCILaunchMode" value="43" wsmap="managed"/>
    <const name="CloudPrivateIP" value="44" wsmap="managed"/>
    <const name="CloudBootVolumeId" value="45" wsmap="managed"/>
    <const name="CloudOCIVCNCompartment" value="46" wsmap="managed"/>
    <const name="CloudOCISubnetCompartment" value="47" wsmap="managed"/>
    <const name="CloudPublicSSHKey" value="48" wsmap="managed"/>
    <const name="BootingFirmware" value="49" wsmap="managed"/>
    <const name="CloudInitScriptPath" value="50" wsmap="managed"/>
    <const name="CloudCompartmentId" value="51" wsmap="managed"/>
    <const name="CloudShapeCpus" value="52" wsmap="managed"/>
    <const name="CloudShapeMemory" value="53" wsmap="managed"/>
    <const name="CloudInstanceMetadata" value="54" wsmap="managed"/>
    <const name="CloudInstanceFreeFormTags" value="55" wsmap="managed"/>
    <const name="CloudImageFreeFormTags" value="56" wsmap="managed"/>
    <const name="HardDiskControllerVirtioSCSI" value="60" wsmap="managed"/>
    <const name="HardDiskControllerNVMe" value="61" wsmap="managed"/>
    <const name="NVRAM" value="0x8000" wsmap="managed"/>
  </enum>
  <enum name="VirtualSystemDescriptionValueType" uuid="56d9403f-3425-4118-9919-36f2a9b8c77c">
    <const name="Reference" value="1" wsmap="managed"/>
    <const name="Original" value="2" wsmap="managed"/>
    <const name="Auto" value="3" wsmap="managed"/>
    <const name="ExtraConfig" value="4" wsmap="managed"/>
  </enum>
  <interface name="IVirtualSystemDescription" extends="$unknown" uuid="01510f40-c196-4d26-b8db-4c8c389f1f82" wsmap="managed" rest="managed" reservedMethods="4" reservedAttributes="4" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="count" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <method name="getDescription" const="no" internal="no" wsmap="managed">
      <param name="types" type="VirtualSystemDescriptionType" dir="out" safearray="yes">
      </param>
      <param name="refs" type="wstring" dir="out" safearray="yes">
      </param>
      <param name="OVFValues" type="wstring" dir="out" safearray="yes">
      </param>
      <param name="VBoxValues" type="wstring" dir="out" safearray="yes">
      </param>
      <param name="extraConfigValues" type="wstring" dir="out" safearray="yes">
      </param>
    </method>
    <method name="getDescriptionByType" const="no" internal="no" wsmap="managed">
      <param name="type" type="VirtualSystemDescriptionType" dir="in" safearray="no">
      </param>
      <param name="types" type="VirtualSystemDescriptionType" dir="out" safearray="yes">
      </param>
      <param name="refs" type="wstring" dir="out" safearray="yes">
      </param>
      <param name="OVFValues" type="wstring" dir="out" safearray="yes">
      </param>
      <param name="VBoxValues" type="wstring" dir="out" safearray="yes">
      </param>
      <param name="extraConfigValues" type="wstring" dir="out" safearray="yes">
      </param>
    </method>
    <method name="removeDescriptionByType" const="no" internal="no" wsmap="managed">
      <param name="type" type="VirtualSystemDescriptionType" dir="in" safearray="no">
      </param>
    </method>
    <method name="getValuesByType" const="no" internal="no" wsmap="managed">
      <param name="type" type="VirtualSystemDescriptionType" dir="in" safearray="no">
      </param>
      <param name="which" type="VirtualSystemDescriptionValueType" dir="in" safearray="no">
      </param>
      <param name="values" type="wstring" dir="return" safearray="yes">
      </param>
    </method>
    <method name="setFinalValues" const="no" internal="no" wsmap="managed">
      <param name="enabled" type="boolean" dir="in" safearray="yes">
      </param>
      <param name="VBoxValues" type="wstring" dir="in" safearray="yes">
      </param>
      <param name="extraConfigValues" type="wstring" dir="in" safearray="yes">
      </param>
    </method>
    <method name="addDescription" const="no" internal="no" wsmap="managed">
      <param name="type" type="VirtualSystemDescriptionType" dir="in" safearray="no">
      </param>
      <param name="VBoxValue" type="wstring" dir="in" safearray="no">
      </param>
      <param name="extraConfigValue" type="wstring" dir="in" safearray="no">
      </param>
    </method>
  </interface>
  <interface name="IUnattended" extends="$unknown" uuid="39d1f56c-c1c2-40df-84e9-468062d84768" wsmap="managed" rest="managed" reservedMethods="4" reservedAttributes="16" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="isoPath" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="machine" type="IMachine" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="user" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="userPassword" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="adminPassword" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="fullUserName" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="productKey" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="additionsIsoPath" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="installGuestAdditions" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="productKeyRequired" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="validationKitIsoPath" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="installTestExecService" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="userPayloadIsoPath" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="installUserPayload" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="timeZone" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="keyboardLayout" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="keyboardVariant" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="locale" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="language" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="country" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="proxy" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="packageSelectionAdjustments" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="hostname" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="auxiliaryBasePath" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="imageIndex" type="unsigned long" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="scriptTemplatePath" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="postInstallScriptTemplatePath" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="postInstallCommand" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="extraInstallKernelParameters" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="detectedOSTypeId" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="detectedOSVersion" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="detectedOSFlavor" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="detectedOSLanguages" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="detectedOSHints" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="detectedImageNames" type="wstring" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="detectedImageIndices" type="unsigned long" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="isUnattendedInstallSupported" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="avoidUpdatesOverNetwork" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <method name="detectIsoOS" const="no" internal="no" wsmap="managed">
    </method>
    <method name="prepare" const="no" internal="no" wsmap="managed">
    </method>
    <method name="constructMedia" const="no" internal="no" wsmap="managed">
    </method>
    <method name="reconfigureVM" const="no" internal="no" wsmap="managed">
    </method>
    <method name="done" const="no" internal="no" wsmap="managed">
    </method>
  </interface>
  <interface name="IInternalMachineControl" extends="$unknown" uuid="EA05E40C-CB31-423B-B3B7-A5B19300F40C" internal="yes" wsmap="suppress" default="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <method name="updateState" const="no" internal="no" wsmap="managed">
      <param name="state" type="MachineState" dir="in" safearray="no"/>
    </method>
    <method name="beginPowerUp" const="no" internal="no" wsmap="managed">
      <param name="progress" type="IProgress" dir="in" safearray="no"/>
    </method>
    <method name="endPowerUp" const="no" internal="no" wsmap="managed">
      <param name="result" type="long" dir="in" safearray="no"/>
    </method>
    <method name="beginPoweringDown" const="no" internal="no" wsmap="managed">
      <param name="progress" type="IProgress" dir="out" safearray="no">
      </param>
    </method>
    <method name="endPoweringDown" const="no" internal="no" wsmap="managed">
      <param name="result" type="long" dir="in" safearray="no">
      </param>
      <param name="errMsg" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <method name="runUSBDeviceFilters" const="no" internal="no" wsmap="managed">
      <param name="device" type="IUSBDevice" dir="in" safearray="no"/>
      <param name="matched" type="boolean" dir="out" safearray="no"/>
      <param name="maskedInterfaces" type="unsigned long" dir="out" safearray="no"/>
    </method>
    <method name="captureUSBDevice" const="no" internal="no" wsmap="managed">
      <param name="id" type="uuid" mod="string" dir="in" safearray="no"/>
      <param name="captureFilename" type="wstring" dir="in" safearray="no"/>
    </method>
    <method name="detachUSBDevice" const="no" internal="no" wsmap="managed">
      <param name="id" type="uuid" mod="string" dir="in" safearray="no"/>
      <param name="done" type="boolean" dir="in" safearray="no"/>
    </method>
    <method name="autoCaptureUSBDevices" const="no" internal="no" wsmap="managed">
    </method>
    <method name="detachAllUSBDevices" const="no" internal="no" wsmap="managed">
      <param name="done" type="boolean" dir="in" safearray="no"/>
    </method>
    <method name="onSessionEnd" const="no" internal="no" wsmap="managed">
      <param name="session" type="ISession" dir="in" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="finishOnlineMergeMedium" const="no" internal="no" wsmap="managed">
    </method>
    <method name="pullGuestProperties" const="no" internal="no" wsmap="managed">
      <param name="names" type="wstring" dir="out" safearray="yes">
      </param>
      <param name="values" type="wstring" dir="out" safearray="yes">
      </param>
      <param name="timestamps" type="long long" dir="out" safearray="yes">
      </param>
      <param name="flags" type="wstring" dir="out" safearray="yes">
      </param>
    </method>
    <method name="pushGuestProperty" const="no" internal="no" wsmap="managed">
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="value" type="wstring" dir="in" safearray="no">
      </param>
      <param name="timestamp" type="long long" dir="in" safearray="no">
      </param>
      <param name="flags" type="wstring" dir="in" safearray="no">
      </param>
      <param name="fWasDeleted" type="boolean" dir="in" safearray="no">
      </param>
    </method>
    <method name="lockMedia" const="no" internal="no" wsmap="managed">
    </method>
    <method name="unlockMedia" const="no" internal="no" wsmap="managed">
    </method>
    <method name="ejectMedium" const="no" internal="no" wsmap="managed">
      <param name="attachment" type="IMediumAttachment" dir="in" safearray="no">
      </param>
      <param name="newAttachment" type="IMediumAttachment" dir="return" safearray="no">
      </param>
    </method>
    <method name="reportVmStatistics" const="no" internal="no" wsmap="managed">
      <param name="validStats" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="cpuUser" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="cpuKernel" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="cpuIdle" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="memTotal" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="memFree" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="memBalloon" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="memShared" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="memCache" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="pagedTotal" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="memAllocTotal" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="memFreeTotal" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="memBalloonTotal" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="memSharedTotal" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="vmNetRx" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="vmNetTx" type="unsigned long" dir="in" safearray="no">
      </param>
    </method>
    <method name="authenticateExternal" const="no" internal="no" wsmap="managed">
      <param name="authParams" type="wstring" dir="in" safearray="yes">
      </param>
      <param name="result" type="wstring" dir="out" safearray="no">
      </param>
    </method>
  </interface>
  <interface name="IGraphicsAdapter" extends="$unknown" uuid="13ee6eb4-06fe-406b-ad69-f6e08b0587a3" wsmap="managed" rest="managed" reservedMethods="4" reservedAttributes="8" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="graphicsControllerType" type="GraphicsControllerType" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="VRAMSize" type="unsigned long" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="monitorCount" type="unsigned long" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <method name="setFeature" const="no" internal="no" wsmap="managed">
      <rest request="post" path="/graphic/adapter/conf/"/>
      <param name="feature" type="GraphicsFeature" dir="in" safearray="no">
      </param>
      <param name="enabled" type="boolean" dir="in" safearray="no">
      </param>
    </method>
    <method name="isFeatureEnabled" const="yes" internal="no" wsmap="managed">
      <rest request="post" path="/graphic/adapter/conf/"/>
      <param name="feature" type="GraphicsFeature" dir="in" safearray="no">
      </param>
      <param name="enabled" type="boolean" dir="return" safearray="no">
      </param>
    </method>
  </interface>
  <interface name="IFirmwareSettings" extends="$unknown" uuid="426EF1B8-DE91-49FB-ABC3-0E2BAE654FF2" wsmap="managed" rest="managed" reservedMethods="2" reservedAttributes="8" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="firmwareType" type="FirmwareType" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="logoFadeIn" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="logoFadeOut" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="logoDisplayTime" type="unsigned long" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="logoImagePath" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="bootMenuMode" type="FirmwareBootMenuMode" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="ACPIEnabled" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="IOAPICEnabled" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="APICMode" type="APICMode" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="timeOffset" type="long long" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="PXEDebugEnabled" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="SMBIOSUuidLittleEndian" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="AutoSerialNumGen" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <enum name="TpmType" uuid="c669b9f7-a547-42b6-8464-636aa53401eb">
    <const name="None" value="0" wsmap="managed">
    </const>
    <const name="v1_2" value="1" wsmap="managed">
    </const>
    <const name="v2_0" value="2" wsmap="managed">
    </const>
    <const name="Host" value="3" wsmap="managed">
    </const>
    <const name="Swtpm" value="4" wsmap="managed">
    </const>
  </enum>
  <interface name="ITrustedPlatformModule" extends="$unknown" uuid="cf11d345-0241-4ea9-ac4c-c69ed3d674e3" wsmap="managed" reservedMethods="2" reservedAttributes="8" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="type" type="TpmType" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="location" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <enum name="RecordingDestination" uuid="11E3F06B-DEC1-48B9-BDC4-1E618D72893C">
    <const name="None" value="0" wsmap="managed">
    </const>
    <const name="File" value="1" wsmap="managed">
    </const>
  </enum>
  <enum name="RecordingFeature" uuid="A7DDC6A5-DAA8-4485-B860-E9F2E98A7794">
    <const name="None" value="0x0" wsmap="managed">
    </const>
    <const name="Video" value="0x1" wsmap="managed">
    </const>
    <const name="Audio" value="0x2" wsmap="managed">
    </const>
  </enum>
  <enum name="RecordingAudioCodec" uuid="16c42be8-1713-4717-a8b9-c65a6549fbcd">
    <const name="None" value="0" wsmap="managed">
    </const>
    <const name="WavPCM" value="1" wsmap="managed">
    </const>
    <const name="MP3" value="2" wsmap="managed">
    </const>
    <const name="OggVorbis" value="3" wsmap="managed">
    </const>
    <const name="Opus" value="4" wsmap="managed">
    </const>
    <const name="Other" value="5" wsmap="managed">
    </const>
  </enum>
  <enum name="RecordingCodecDeadline" uuid="53627ae1-31cf-4303-9cd5-0c22fd2637cf">
    <const name="Default" value="0" wsmap="managed">
    </const>
    <const name="Realtime" value="1" wsmap="managed">
    </const>
    <const name="Good" value="2" wsmap="managed">
    </const>
    <const name="Best" value="3" wsmap="managed">
    </const>
  </enum>
  <enum name="RecordingVideoCodec" uuid="93791063-3e8c-4310-987c-ddb43ff03ffe">
    <const name="None" value="0" wsmap="managed">
    </const>
    <const name="MJPEG" value="1" wsmap="managed">
    </const>
    <const name="H262" value="2" wsmap="managed">
    </const>
    <const name="H264" value="3" wsmap="managed">
    </const>
    <const name="H265" value="4" wsmap="managed">
    </const>
    <const name="H266" value="5" wsmap="managed">
    </const>
    <const name="VP8" value="6" wsmap="managed">
    </const>
    <const name="VP9" value="7" wsmap="managed">
    </const>
    <const name="AV1" value="8" wsmap="managed">
    </const>
    <const name="Other" value="9" wsmap="managed">
    </const>
  </enum>
  <enum name="RecordingVideoScalingMode" uuid="0dd1127d-4f62-4b82-beee-91086a9f1d24">
    <const name="None" value="0" wsmap="managed">
    </const>
    <const name="NearestNeighbor" value="1" wsmap="managed">
    </const>
    <const name="Bilinear" value="2" wsmap="managed">
    </const>
    <const name="Bicubic" value="3" wsmap="managed">
    </const>
  </enum>
  <enum name="RecordingRateControlMode" uuid="d07a33ac-b4ea-4917-942d-c03be14fbe2f">
    <const name="ABR" value="0" wsmap="managed">
    </const>
    <const name="CBR" value="1" wsmap="managed">
    </const>
    <const name="VBR" value="2" wsmap="managed">
    </const>
  </enum>
  <enum name="RecordingState" uuid="c8a770a9-0412-467b-8dae-0e861279d0be">
    <const name="Unknown" value="0" wsmap="managed">
    </const>
    <const name="Initializing" value="1" wsmap="managed">
    </const>
    <const name="Started" value="2" wsmap="managed">
    </const>
    <const name="Paused" value="3" wsmap="managed">
    </const>
    <const name="Resumed" value="4" wsmap="managed">
    </const>
    <const name="Finalizing" value="5" wsmap="managed">
    </const>
    <const name="Stopped" value="6" wsmap="managed">
    </const>
    <const name="Canceled" value="7" wsmap="managed">
    </const>
    <const name="LimitReached" value="8" wsmap="managed">
    </const>
    <const name="Error" value="9" wsmap="managed">
    </const>
   </enum>
  <interface name="IRecordingScreenSettings" extends="$unknown" uuid="c1844087-ec6b-488d-afbb-c90f6452a04b" wsmap="managed" rest="managed" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <method name="isFeatureEnabled" const="no" internal="no" wsmap="managed">
      <param name="feature" type="RecordingFeature" dir="in" safearray="no">
      </param>
      <param name="enabled" type="boolean" dir="return" safearray="no">
      </param>
    </method>
    <attribute name="id" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="enabled" type="boolean" default="false" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="features" type="RecordingFeature" safearray="yes" readonly="no" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="destination" type="RecordingDestination" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="filename" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="maxTime" type="unsigned long" default="0" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="maxFileSize" type="unsigned long" default="0" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="options" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="audioCodec" type="RecordingAudioCodec" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="audioRateControlMode" type="RecordingRateControlMode" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="audioDeadline" type="RecordingCodecDeadline" default="Default" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="audioHz" type="unsigned long" default="22050" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="audioBits" type="unsigned long" default="16" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="audioChannels" type="unsigned long" default="2" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="videoCodec" type="RecordingVideoCodec" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="videoDeadline" type="RecordingCodecDeadline" default="Default" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="videoWidth" type="unsigned long" default="1024" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="videoHeight" type="unsigned long" default="768" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="videoRate" type="unsigned long" default="512" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="videoRateControlMode" type="RecordingRateControlMode" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="videoFPS" type="unsigned long" default="25" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="videoScalingMode" type="RecordingVideoScalingMode" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IRecordingSettings" extends="$unknown" uuid="83cfdc66-4f20-4569-ad39-8722f7d472b3" wsmap="managed" rest="managed" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <method name="getScreenSettings" const="no" internal="no" wsmap="managed">
      <param name="screenId" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="recordScreenSettings" type="IRecordingScreenSettings" dir="return" safearray="no">
      </param>
    </method>
    <method name="start" const="no" internal="no" wsmap="managed">
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
      <result name="E_FAIL">
        Unable to start recording due to recording already being started or not
        available. See error message for more information.
      </result>
    </method>
    <method name="resume" const="no" internal="no" wsmap="managed">
      <result name="E_FAIL">
        Unable to resume recording due to recording not being started
        or not available. See error message for more information.
      </result>
      <result name="VBOX_E_NOT_SUPPORTED">
        Resuming (and thus pausing also) is not supported.
      </result>
    </method>
    <attribute name="paused" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="enabled" type="boolean" default="false" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="screens" type="IRecordingScreenSettings" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="progress" type="IProgress" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IPCIAddress" extends="$unknown" uuid="c984d15f-e191-400b-840e-970f3dad7296" wsmap="managed" rest="managed" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="bus" type="short" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="device" type="short" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="devFunction" type="short" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <method name="asLong" const="no" internal="no" wsmap="managed">
      <param name="result" type="long" dir="return" safearray="no"/>
    </method>
    <method name="fromLong" const="no" internal="no" wsmap="managed">
      <param name="number" type="long" dir="in" safearray="no"/>
    </method>
  </interface>
  <interface name="IPCIDeviceAttachment" extends="$unknown" uuid="91f33d6f-e621-4f70-a77e-15f0e3c714d5" wsmap="struct" rest="managed" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="name" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="isPhysicalDevice" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="hostAddress" type="long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="guestAddress" type="long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
   <enum name="SignatureType" uuid="6f6e67ef-9a32-4084-af84-5702679f882a">
    <const name="X509" value="0" wsmap="managed">
    </const>
    <const name="Sha256" value="1" wsmap="managed">
    </const>
  </enum>
  <enum name="UefiVariableAttributes" uuid="cda505ec-b444-4aef-b55c-b687717bdac5">
    <const name="None" value="0" wsmap="managed">
    </const>
    <const name="NonVolatile" value="0x01" wsmap="managed">
    </const>
    <const name="BootServiceAccess" value="0x02" wsmap="managed">
    </const>
    <const name="RuntimeAccess" value="0x04" wsmap="managed">
    </const>
    <const name="HwErrorRecord" value="0x08" wsmap="managed">
    </const>
    <const name="AuthWriteAccess" value="0x100" wsmap="managed">
    </const>
    <const name="AuthTimeBasedWriteAccess" value="0x200" wsmap="managed">
    </const>
    <const name="AuthAppendWrite" value="0x400" wsmap="managed">
    </const>
  </enum>
  <interface name="IUefiVariableStore" extends="$unknown" uuid="d134c6b6-4479-430d-bb73-68a452ba3e67" wsmap="managed" reservedMethods="9" reservedAttributes="5" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="secureBootEnabled" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <method name="addVariable" const="no" internal="no" wsmap="managed">
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="owner" type="uuid" mod="string" dir="in" safearray="no">
      </param>
      <param name="attributes" type="UefiVariableAttributes" safearray="yes" dir="in">
      </param>
      <param name="data" type="octet" dir="in" safearray="yes">
      </param>
    </method>
    <method name="deleteVariable" const="no" internal="no" wsmap="managed">
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="owner" type="uuid" mod="string" dir="in" safearray="no">
      </param>
    </method>
    <method name="changeVariable" const="no" internal="no" wsmap="managed">
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="data" type="octet" dir="in" safearray="yes">
      </param>
    </method>
    <method name="queryVariableByName" const="no" internal="no" wsmap="managed">
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="owner" type="uuid" mod="string" dir="out" safearray="no">
      </param>
      <param name="attributes" type="UefiVariableAttributes" safearray="yes" dir="out">
      </param>
      <param name="data" type="octet" dir="out" safearray="yes">
      </param>
    </method>
    <method name="queryVariables" const="no" internal="no" wsmap="managed">
      <param name="names" type="wstring" dir="out" safearray="yes">
      </param>
      <param name="owners" type="uuid" mod="string" dir="out" safearray="yes">
      </param>
    </method>
    <method name="enrollOraclePlatformKey" const="no" internal="no" wsmap="managed">
    </method>
    <method name="enrollPlatformKey" const="no" internal="no" wsmap="managed">
      <param name="platformKey" type="octet" safearray="yes" dir="in">
      </param>
      <param name="owner" type="uuid" mod="string" dir="in" safearray="no">
      </param>
    </method>
    <method name="addKek" const="no" internal="no" wsmap="managed">
      <param name="keyEncryptionKey" type="octet" safearray="yes" dir="in">
      </param>
      <param name="owner" type="uuid" mod="string" dir="in" safearray="no">
      </param>
      <param name="signatureType" type="SignatureType" dir="in" safearray="no">
      </param>
    </method>
    <method name="addSignatureToDb" const="no" internal="no" wsmap="managed">
      <param name="signature" type="octet" safearray="yes" dir="in">
      </param>
      <param name="owner" type="uuid" mod="string" dir="in" safearray="no">
      </param>
      <param name="signatureType" type="SignatureType" dir="in" safearray="no">
      </param>
    </method>
    <method name="addSignatureToDbx" const="no" internal="no" wsmap="managed">
      <param name="signature" type="octet" safearray="yes" dir="in">
      </param>
      <param name="owner" type="uuid" mod="string" dir="in" safearray="no">
      </param>
      <param name="signatureType" type="SignatureType" dir="in" safearray="no">
      </param>
    </method>
    <method name="enrollDefaultMsSignatures" const="no" internal="no" wsmap="managed">
    </method>
    <method name="addSignatureToMok" const="no" internal="no" wsmap="managed">
      <param name="signature" type="octet" safearray="yes" dir="in">
      </param>
      <param name="owner" type="uuid" mod="string" dir="in" safearray="no">
      </param>
      <param name="signatureType" type="SignatureType" dir="in" safearray="no">
      </param>
    </method>
  </interface>
  <interface name="INvramStore" extends="$unknown" uuid="5bfd8965-b81b-469f-8649-f717ce97a5d5" wsmap="managed" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="nonVolatileStorageFile" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="uefiVariableStore" type="IUefiVariableStore" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
     <attribute name="keyId" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="keyStore" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <method name="initUefiVariableStore" const="no" internal="no" wsmap="managed">
      <param name="size" type="unsigned long" dir="in" safearray="no">
      </param>
    </method>
  </interface>
  <interface name="IResourceStore" extends="$unknown" uuid="d96a2540-8aab-4c71-8af9-95379f23b23d" wsmap="managed" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="midlDoesNotLikeEmptyInterfaces" readonly="yes" type="boolean" internal="no" safearray="no" wsmap="managed" rest="default"/>
  </interface>
  <enum name="GraphicsControllerType" uuid="e1379bad-ae01-493d-abf5-740a95c41676">
    <const name="Null" value="0" wsmap="managed">
    </const>
    <const name="VBoxVGA" value="1" wsmap="managed">
    </const>
    <const name="VMSVGA" value="2" wsmap="managed">
    </const>
    <const name="VBoxSVGA" value="3" wsmap="managed">
    </const>
    <const name="QemuRamFB" value="4" wsmap="managed">
    </const>
  </enum>
  <enum name="GraphicsFeature" uuid="1BC430F8-D061-45FB-8830-155058FF3F66">
    <const name="None" value="0" wsmap="managed">
    </const>
    <const name="Acceleration3D" value="2" wsmap="managed">
    </const>
  </enum>
  <enum name="CleanupMode" uuid="67897c50-7cca-47a9-83f6-ce8fd8eb5441">
    <const name="UnregisterOnly" value="1" wsmap="managed">
    </const>
    <const name="DetachAllReturnNone" value="2" wsmap="managed">
    </const>
    <const name="DetachAllReturnHardDisksOnly" value="3" wsmap="managed">
    </const>
    <const name="Full" value="4" wsmap="managed">
    </const>
    <const name="DetachAllReturnHardDisksAndVMRemovable" value="5" wsmap="managed">
    </const>
  </enum>
  <enum name="CloneMode" uuid="A7A159FE-5096-4B8D-8C3C-D033CB0B35A8">
    <const name="MachineState" value="1" wsmap="managed">
    </const>
    <const name="MachineAndChildStates" value="2" wsmap="managed">
    </const>
    <const name="AllStates" value="3" wsmap="managed">
    </const>
  </enum>
  <enum name="CloneOptions" uuid="22243f8e-96ab-497c-8cf0-f40a566c630b">
    <const name="Link" value="1" wsmap="managed">
    </const>
    <const name="KeepAllMACs" value="2" wsmap="managed">
    </const>
    <const name="KeepNATMACs" value="3" wsmap="managed">
    </const>
    <const name="KeepDiskNames" value="4" wsmap="managed">
    </const>
    <const name="KeepHwUUIDs" value="5" wsmap="managed">
    </const>
  </enum>
  <enum name="AutostopType" uuid="6bb96740-cf34-470d-aab2-2cd48ea2e10e">
    <const name="Disabled" value="1" wsmap="managed">
    </const>
    <const name="SaveState" value="2" wsmap="managed">
    </const>
    <const name="PowerOff" value="3" wsmap="managed">
    </const>
    <const name="AcpiShutdown" value="4" wsmap="managed">
    </const>
  </enum>
  <enum name="VMProcPriority" uuid="6fa72dd5-19b7-46ba-bc52-f223c98c7d80">
    <const name="Invalid" value="0" wsmap="managed">
    </const>
    <const name="Default" value="1" wsmap="managed">
    </const>
    <const name="Flat" value="2" wsmap="managed">
    </const>
    <const name="Low" value="3" wsmap="managed">
    </const>
    <const name="Normal" value="5" wsmap="managed">
    </const>
    <const name="High" value="6" wsmap="managed">
    </const>
  </enum>
  <enum name="IommuType" uuid="f47339cb-b94e-48fe-b507-2900103d7b9f">
    <const name="None" value="0" wsmap="managed">
    </const>
    <const name="Automatic" value="1" wsmap="managed">
    </const>
    <const name="AMD" value="2" wsmap="managed">
    </const>
    <const name="Intel" value="3" wsmap="managed">
    </const>
  </enum>
  <interface name="IPlatform" extends="$unknown" uuid="a670a023-e172-452c-b731-14ef855f4da6" wsmap="managed" rest="managed" reservedMethods="8" reservedAttributes="16" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="architecture" type="PlatformArchitecture" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="properties" type="IPlatformProperties" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="x86" type="IPlatformX86" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="ARM" type="IPlatformARM" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="chipsetType" type="ChipsetType" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="iommuType" type="IommuType" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="RTCUseUTC" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IPlatformX86" extends="$unknown" uuid="5ada589f-09c9-4604-b700-9ab3a5572e3a" wsmap="managed" rest="managed" reservedMethods="8" reservedAttributes="16" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="HPETEnabled" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <method name="getCPUProperty" const="yes" internal="no" wsmap="managed">
      <rest name="property" request="get" path="/vm/{vmid}/config/cpu/x86/"/>
      <param name="property" type="CPUPropertyTypeX86" dir="in" safearray="no">
      </param>
      <param name="value" type="boolean" dir="return" safearray="no">
      </param>
    </method>
    <method name="setCPUProperty" const="no" internal="no" wsmap="managed">
      <rest name="property" request="post" path="/vm/{vmid}/config/cpu/x86/"/>
      <param name="property" type="CPUPropertyTypeX86" dir="in" safearray="no">
      </param>
      <param name="value" type="boolean" dir="in" safearray="no">
      </param>
    </method>
    <method name="getCPUIDLeafByOrdinal" const="yes" internal="no" wsmap="managed">
      <param name="ordinal" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="idx" type="unsigned long" dir="out" safearray="no">
      </param>
      <param name="idxSub" type="unsigned long" dir="out" safearray="no">
      </param>
      <param name="valEax" type="unsigned long" dir="out" safearray="no">
      </param>
      <param name="valEbx" type="unsigned long" dir="out" safearray="no">
      </param>
      <param name="valEcx" type="unsigned long" dir="out" safearray="no">
      </param>
      <param name="valEdx" type="unsigned long" dir="out" safearray="no">
      </param>
    </method>
    <method name="getCPUIDLeaf" const="yes" internal="no" wsmap="managed">
      <param name="idx" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="idxSub" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="valEax" type="unsigned long" dir="out" safearray="no">
      </param>
      <param name="valEbx" type="unsigned long" dir="out" safearray="no">
      </param>
      <param name="valEcx" type="unsigned long" dir="out" safearray="no">
      </param>
      <param name="valEdx" type="unsigned long" dir="out" safearray="no">
      </param>
    </method>
    <method name="setCPUIDLeaf" const="no" internal="no" wsmap="managed">
      <param name="idx" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="idxSub" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="valEax" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="valEbx" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="valEcx" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="valEdx" type="unsigned long" dir="in" safearray="no">
      </param>
    </method>
    <method name="removeCPUIDLeaf" const="no" internal="no" wsmap="managed">
      <param name="idx" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="idxSub" type="unsigned long" dir="in" safearray="no">
      </param>
    </method>
    <method name="removeAllCPUIDLeaves" const="no" internal="no" wsmap="managed">
    </method>
    <method name="getHWVirtExProperty" const="yes" internal="no" wsmap="managed">
      <rest name="HWVirtExProperty" request="get" path="/vm/{vmid}/config/"/>
      <param name="property" type="HWVirtExPropertyType" dir="in" safearray="no">
      </param>
      <param name="value" type="boolean" dir="return" safearray="no">
      </param>
    </method>
    <method name="setHWVirtExProperty" const="no" internal="no" wsmap="managed">
      <rest name="HWVirtExProperty" request="post" path="/vm/{vmid}/config/"/>
      <param name="property" type="HWVirtExPropertyType" dir="in" safearray="no">
      </param>
      <param name="value" type="boolean" dir="in" safearray="no">
      </param>
    </method>
  </interface>
  <interface name="IPlatformARM" extends="$unknown" uuid="002c75fe-3316-4920-aece-7d21ce6f624b" wsmap="managed" rest="managed" reservedMethods="8" reservedAttributes="16" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <method name="getCPUProperty" const="yes" internal="no" wsmap="managed">
      <rest name="property" request="get" path="/vm/{vmid}/config/cpu/arm/"/>
      <param name="property" type="CPUPropertyTypeARM" dir="in" safearray="no">
      </param>
      <param name="value" type="boolean" dir="return" safearray="no">
      </param>
    </method>
    <method name="setCPUProperty" const="no" internal="no" wsmap="managed">
      <rest name="property" request="post" path="/vm/{vmid}/config/cpu/arm/"/>
      <param name="property" type="CPUPropertyTypeARM" dir="in" safearray="no">
      </param>
      <param name="value" type="boolean" dir="in" safearray="no">
      </param>
    </method>
  </interface>
  <enum name="VMExecutionEngine" uuid="69485f52-5cd3-40c9-946f-ae7d5c50798b">
    <const name="NotSet" value="0" wsmap="managed">
    </const>
    <const name="Default" value="1" wsmap="managed">
    </const>
    <const name="HwVirt" value="2" wsmap="managed">
    </const>
    <const name="NativeApi" value="3" wsmap="managed">
    </const>
    <const name="Interpreter" value="4" wsmap="managed">
    </const>
    <const name="Recompiler" value="5" wsmap="managed">
    </const>
  </enum>
  <interface name="IMachine" extends="$unknown" uuid="e36a5081-a82a-40bd-9e4e-42a44d6ce50f" wsmap="managed" rest="managed" wrap-hint-server-addinterfaces="IInternalMachineControl" wrap-hint-server="manualaddinterfaces" reservedMethods="8" reservedAttributes="16" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="parent" type="IVirtualBox" readonly="yes" wrap-hint-server="limitedcaller" rest="suppress" internal="no" safearray="no" wsmap="managed">
    </attribute>
    <attribute name="icon" type="octet" safearray="yes" readonly="no" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="accessible" type="boolean" readonly="yes" wrap-hint-server="limitedcaller" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="accessError" type="IVirtualBoxErrorInfo" readonly="yes" wrap-hint-server="limitedcaller" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="name" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="description" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="id" type="uuid" mod="string" readonly="yes" wrap-hint-server="limitedcaller" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="groups" type="wstring" safearray="yes" readonly="no" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="OSTypeId" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="hardwareVersion" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="hardwareUUID" type="uuid" mod="string" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="CPUCount" type="unsigned long" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="CPUHotPlugEnabled" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="CPUExecutionCap" type="unsigned long" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="CPUIDPortabilityLevel" type="unsigned long" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="memorySize" type="unsigned long" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="memoryBalloonSize" type="unsigned long" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="pageFusionEnabled" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="graphicsAdapter" type="IGraphicsAdapter" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="platform" type="IPlatform" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="firmwareSettings" type="IFirmwareSettings" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="trustedPlatformModule" type="ITrustedPlatformModule" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="nonVolatileStore" type="INvramStore" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="recordingSettings" type="IRecordingSettings" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="pointingHIDType" type="PointingHIDType" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="keyboardHIDType" type="KeyboardHIDType" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="snapshotFolder" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="VRDEServer" type="IVRDEServer" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="emulatedUSBCardReaderEnabled" type="boolean" default="false" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default"/>
    <attribute name="mediumAttachments" type="IMediumAttachment" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="USBControllers" type="IUSBController" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="USBDeviceFilters" type="IUSBDeviceFilters" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="audioSettings" type="IAudioSettings" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="storageControllers" type="IStorageController" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="settingsFilePath" type="wstring" readonly="yes" wrap-hint-server="limitedcaller" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="settingsAuxFilePath" type="wstring" readonly="yes" wrap-hint-server="limitedcaller" rest="suppress" internal="no" safearray="no" wsmap="managed">
    </attribute>
    <attribute name="settingsModified" type="boolean" readonly="yes" rest="suppress" internal="no" safearray="no" wsmap="managed">
    </attribute>
    <attribute name="sessionState" type="SessionState" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="sessionName" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="sessionPID" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="state" type="MachineState" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="lastStateChange" type="long long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="stateFilePath" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="logFolder" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="currentSnapshot" type="ISnapshot" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="snapshotCount" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="currentStateModified" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="sharedFolders" type="ISharedFolder" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="clipboardMode" type="ClipboardMode" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="clipboardFileTransfersEnabled" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="dnDMode" type="DnDMode" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="teleporterEnabled" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="teleporterPort" type="unsigned long" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="teleporterAddress" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="teleporterPassword" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="paravirtProvider" type="ParavirtProvider" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="IOCacheEnabled" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="IOCacheSize" type="unsigned long" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="PCIDeviceAssignments" type="IPCIDeviceAttachment" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="bandwidthControl" type="IBandwidthControl" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="tracingEnabled" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="tracingConfig" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="allowTracingToAccessVM" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="autostartEnabled" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="autostartDelay" type="unsigned long" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="autostopType" type="AutostopType" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="defaultFrontend" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="USBProxyAvailable" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="VMProcessPriority" type="VMProcPriority" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="VMExecutionEngine" type="VMExecutionEngine" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="paravirtDebug" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="CPUProfile" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="stateKeyId" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="stateKeyStore" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="logKeyId" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="logKeyStore" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="guestDebugControl" type="IGuestDebugControl" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <method name="lockMachine" const="no" internal="no" wsmap="managed">
      <rest name="lock" request="post" path="/vm/{vmid}/action/"/>
      <param name="session" type="ISession" dir="in" safearray="no">
      </param>
      <param name="lockType" type="LockType" dir="in" safearray="no">
      </param>
    </method>
    <method name="launchVMProcess" const="no" internal="no" wsmap="managed">
      <rest name="launch" request="post" path="/vm/{vmid}/action/"/>
      <param name="session" type="ISession" dir="in" safearray="no">
      </param>
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="environmentChanges" type="wstring" safearray="yes" dir="in">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="setBootOrder" const="no" internal="no" wsmap="managed">
      <rest name="bootorder" request="post" path="/vm/{vmid}/config/"/>
      <param name="position" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="device" type="DeviceType" dir="in" safearray="no">
      </param>
    </method>
    <method name="getBootOrder" const="yes" internal="no" wsmap="managed">
      <rest name="bootorder" request="get" path="/vm/{vmid}/config/"/>
      <param name="position" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="device" type="DeviceType" dir="return" safearray="no">
      </param>
    </method>
    <method name="attachDevice" const="no" internal="no" wsmap="managed">
      <rest name="attach" request="post" path="/vm/{vmid}/dev/action/"/>
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="controllerPort" type="long" dir="in" safearray="no">
      </param>
      <param name="device" type="long" dir="in" safearray="no">
      </param>
      <param name="type" type="DeviceType" dir="in" safearray="no">
      </param>
      <param name="medium" type="IMedium" dir="in" safearray="no">
      </param>
    </method>
    <method name="attachDeviceWithoutMedium" const="no" internal="no" wsmap="managed">
      <rest name="attachempty" request="post" path="/vm/{vmid}/dev/action/"/>
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="controllerPort" type="long" dir="in" safearray="no">
      </param>
      <param name="device" type="long" dir="in" safearray="no">
      </param>
      <param name="type" type="DeviceType" dir="in" safearray="no">
      </param>
    </method>
    <method name="detachDevice" const="no" internal="no" wsmap="managed">
      <rest name="detach" request="post" path="/vm/{vmid}/dev/action/"/>
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="controllerPort" type="long" dir="in" safearray="no">
      </param>
      <param name="device" type="long" dir="in" safearray="no">
      </param>
    </method>
    <method name="passthroughDevice" const="no" internal="no" wsmap="managed">
      <rest request="post" path="/vm/{vmid}/dev/config/"/>
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="controllerPort" type="long" dir="in" safearray="no">
      </param>
      <param name="device" type="long" dir="in" safearray="no">
      </param>
      <param name="passthrough" type="boolean" dir="in" safearray="no">
      </param>
    </method>
    <method name="temporaryEjectDevice" const="no" internal="no" wsmap="managed">
      <rest request="post" path="/vm/{vmid}/dev/config/"/>
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="controllerPort" type="long" dir="in" safearray="no">
      </param>
      <param name="device" type="long" dir="in" safearray="no">
      </param>
      <param name="temporaryEject" type="boolean" dir="in" safearray="no">
      </param>
    </method>
    <method name="nonRotationalDevice" const="no" internal="no" wsmap="managed">
      <rest request="post" path="/vm/{vmid}/dev/config/"/>
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="controllerPort" type="long" dir="in" safearray="no">
      </param>
      <param name="device" type="long" dir="in" safearray="no">
      </param>
      <param name="nonRotational" type="boolean" dir="in" safearray="no">
      </param>
    </method>
    <method name="setAutoDiscardForDevice" const="no" internal="no" wsmap="managed">
      <rest name="AutoDiscard" request="post" path="/vm/{vmid}/dev/config/"/>
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="controllerPort" type="long" dir="in" safearray="no">
      </param>
      <param name="device" type="long" dir="in" safearray="no">
      </param>
      <param name="discard" type="boolean" dir="in" safearray="no">
      </param>
    </method>
    <method name="setHotPluggableForDevice" const="no" internal="no" wsmap="managed">
      <rest name="HotPluggable" request="post" path="/vm/{vmid}/dev/config/"/>
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="controllerPort" type="long" dir="in" safearray="no">
      </param>
      <param name="device" type="long" dir="in" safearray="no">
      </param>
      <param name="hotPluggable" type="boolean" dir="in" safearray="no">
      </param>
    </method>
    <method name="setBandwidthGroupForDevice" const="no" internal="no" wsmap="managed">
      <rest name="BandwidthGroup" request="post" path="/vm/{vmid}/dev/config/"/>
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="controllerPort" type="long" dir="in" safearray="no">
      </param>
      <param name="device" type="long" dir="in" safearray="no">
      </param>
      <param name="bandwidthGroup" type="IBandwidthGroup" dir="in" safearray="no">
      </param>
    </method>
    <method name="setNoBandwidthGroupForDevice" const="no" internal="no" wsmap="managed">
      <rest name="NoBandwidthGroup" request="post" path="/vm/{vmid}/dev/config/"/>
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="controllerPort" type="long" dir="in" safearray="no">
      </param>
      <param name="device" type="long" dir="in" safearray="no">
      </param>
    </method>
    <method name="unmountMedium" const="no" internal="no" wsmap="managed">
      <rest name="unmount" request="post" path="/vm/{vmid}/media/action/"/>
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="controllerPort" type="long" dir="in" safearray="no">
      </param>
      <param name="device" type="long" dir="in" safearray="no">
      </param>
      <param name="force" type="boolean" dir="in" safearray="no">
      </param>
    </method>
    <method name="mountMedium" const="no" internal="no" wsmap="managed">
      <rest name="mount" request="post" path="/vm/{vmid}/media/action/"/>
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="controllerPort" type="long" dir="in" safearray="no">
      </param>
      <param name="device" type="long" dir="in" safearray="no">
      </param>
      <param name="medium" type="IMedium" dir="in" safearray="no">
      </param>
      <param name="force" type="boolean" dir="in" safearray="no">
      </param>
    </method>
    <method name="getMedium" const="yes" internal="no" wsmap="managed">
      <rest name="find" request="get" path="/vm/{vmid}/media/"/>
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="controllerPort" type="long" dir="in" safearray="no">
      </param>
      <param name="device" type="long" dir="in" safearray="no">
      </param>
      <param name="medium" type="IMedium" dir="return" safearray="no">
      </param>
    </method>
    <method name="getMediumAttachmentsOfController" const="yes" internal="no" wsmap="managed">
      <rest name="AttachmentsList" request="get" path="/vm/{vmid}/controller/"/>
      <param name="name" type="wstring" dir="in" safearray="no"/>
      <param name="mediumAttachments" type="IMediumAttachment" safearray="yes" dir="return"/>
    </method>
    <method name="getMediumAttachment" const="yes" internal="no" wsmap="managed">
      <rest name="Attachment" request="get" path="/vm/{vmid}/controller/"/>
      <param name="name" type="wstring" dir="in" safearray="no"/>
      <param name="controllerPort" type="long" dir="in" safearray="no"/>
      <param name="device" type="long" dir="in" safearray="no"/>
      <param name="attachment" type="IMediumAttachment" dir="return" safearray="no"/>
    </method>
    <method name="attachHostPCIDevice" const="no" internal="no" wsmap="managed">
      <param name="hostAddress" type="long" dir="in" safearray="no">
      </param>
      <param name="desiredGuestAddress" type="long" dir="in" safearray="no">
      </param>
      <param name="tryToUnbind" type="boolean" dir="in" safearray="no">
      </param>
    </method>
    <method name="detachHostPCIDevice" const="no" internal="no" wsmap="managed">
      <param name="hostAddress" type="long" dir="in" safearray="no">
      </param>
    </method>
    <method name="getNetworkAdapter" const="yes" internal="no" wsmap="managed">
      <rest name="find" request="get" path="/vm/{vmid}/networkadapter/"/>
      <param name="slot" type="unsigned long" dir="in" safearray="no"/>
      <param name="adapter" type="INetworkAdapter" dir="return" safearray="no"/>
    </method>
    <method name="addStorageController" const="no" internal="no" wsmap="managed">
      <rest name="add" request="post" path="/vm/{vmid}/storagecontroller/action/"/>
      <param name="name" type="wstring" dir="in" safearray="no"/>
      <param name="connectionType" type="StorageBus" dir="in" safearray="no"/>
      <param name="controller" type="IStorageController" dir="return" safearray="no"/>
    </method>
    <method name="getStorageControllerByName" const="yes" internal="no" wsmap="managed">
      <rest name="findbyname" request="get" path="/vm/{vmid}/storagecontroller/"/>
      <param name="name" type="wstring" dir="in" safearray="no"/>
      <param name="storageController" type="IStorageController" dir="return" safearray="no"/>
    </method>
    <method name="getStorageControllerByInstance" const="yes" internal="no" wsmap="managed">
      <rest name="findbyinstance" request="get" path="/vm/{vmid}/storagecontroller/"/>
      <param name="connectionType" type="StorageBus" dir="in" safearray="no"/>
      <param name="instance" type="unsigned long" dir="in" safearray="no"/>
      <param name="storageController" type="IStorageController" dir="return" safearray="no"/>
    </method>
    <method name="removeStorageController" const="no" internal="no" wsmap="managed">
      <rest name="remove" request="post" path="/vm/{vmid}/storagecontroller/action/"/>
      <param name="name" type="wstring" dir="in" safearray="no"/>
    </method>
    <method name="setStorageControllerBootable" const="no" internal="no" wsmap="managed">
      <rest name="bootable" request="post" path="/vm/{vmid}/storagecontroller/config/"/>
      <param name="name" type="wstring" dir="in" safearray="no"/>
      <param name="bootable" type="boolean" dir="in" safearray="no"/>
    </method>
    <method name="addUSBController" const="no" internal="no" wsmap="managed">
      <rest name="add" request="post" path="/vm/{vmid}/usbcontroller/action/"/>
      <param name="name" type="wstring" dir="in" safearray="no"/>
      <param name="type" type="USBControllerType" dir="in" safearray="no"/>
      <param name="controller" type="IUSBController" dir="return" safearray="no"/>
    </method>
    <method name="removeUSBController" const="no" internal="no" wsmap="managed">
      <rest name="remove" request="post" path="/vm/{vmid}/usbcontroller/action/"/>
      <param name="name" type="wstring" dir="in" safearray="no"/>
    </method>
    <method name="getUSBControllerByName" const="yes" internal="no" wsmap="managed">
      <rest name="find" request="get" path="/vm/{vmid}/usbcontroller/"/>
      <param name="name" type="wstring" dir="in" safearray="no"/>
      <param name="controller" type="IUSBController" dir="return" safearray="no"/>
    </method>
    <method name="getUSBControllerCountByType" const="yes" internal="no" wsmap="managed">
      <rest name="number" request="get" path="/vm/{vmid}/usbcontroller/config/"/>
      <param name="type" type="USBControllerType" dir="in" safearray="no"/>
      <param name="controllers" type="unsigned long" dir="return" safearray="no"/>
    </method>
    <method name="getSerialPort" const="yes" internal="no" wsmap="managed">
      <rest name="find" request="get" path="/vm/{vmid}/serialport/"/>
      <param name="slot" type="unsigned long" dir="in" safearray="no"/>
      <param name="port" type="ISerialPort" dir="return" safearray="no"/>
    </method>
    <method name="getParallelPort" const="yes" internal="no" wsmap="managed">
      <rest name="find" request="get" path="/vm/{vmid}/parallelport/"/>
      <param name="slot" type="unsigned long" dir="in" safearray="no"/>
      <param name="port" type="IParallelPort" dir="return" safearray="no"/>
    </method>
    <method name="getExtraDataKeys" const="no" internal="no" wsmap="managed">
      <rest name="ExtraDataKeyList" request="get" path="/vm/{vmid}/config/"/>
      <param name="keys" type="wstring" dir="return" safearray="yes">
      </param>
    </method>
    <method name="getExtraData" const="no" internal="no" wsmap="managed">
      <rest name="ExtraData" request="get" path="/vm/{vmid}/config/"/>
      <param name="key" type="wstring" dir="in" safearray="no">
      </param>
      <param name="value" type="wstring" dir="return" safearray="no">
      </param>
    </method>
    <method name="setExtraData" const="no" internal="no" wsmap="managed">
      <rest name="ExtraData" request="post" path="/vm/{vmid}/config/"/>
      <param name="key" type="wstring" dir="in" safearray="no">
      </param>
      <param name="value" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <method name="setSettingsFilePath" const="no" internal="no" wsmap="managed">
      <rest name="SettingsFilePath" request="post" path="/vm/{vmid}/config/"/>
      <param name="settingsFilePath" type="wstring" dir="in" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="saveSettings" const="no" internal="no" wsmap="managed">
      <rest name="save" request="post" path="/vm/{vmid}/config/"/>
    </method>
    <method name="discardSettings" const="no" internal="no" wsmap="managed">
      <rest name="discard" request="post" path="/vm/{vmid}/config/"/>
    </method>
    <method name="unregister" wrap-hint-server="limitedcaller,passcaller" const="no" internal="no" wsmap="managed">
      <rest request="post" path="/vm/{vmid}/action/"/>
      <param name="cleanupMode" type="CleanupMode" dir="in" safearray="no">
      </param>
      <param name="media" type="IMedium" safearray="yes" dir="return">
      </param>
    </method>
    <method name="deleteConfig" const="no" internal="no" wsmap="managed">
      <rest name="remove" request="delete" path="/vm/{vmid}/"/>
      <param name="media" type="IMedium" safearray="yes" dir="in">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="exportTo" const="no" internal="no" wsmap="managed">
      <param name="appliance" type="IAppliance" dir="in" safearray="no">
      </param>
      <param name="location" type="wstring" dir="in" safearray="no">
      </param>
      <param name="description" type="IVirtualSystemDescription" dir="return" safearray="no">
      </param>
    </method>
    <method name="findSnapshot" const="no" internal="no" wsmap="managed">
      <rest name="find" request="get" path="/vm/{vmid}/snapshot/"/>
      <param name="nameOrId" type="wstring" dir="in" safearray="no">
      </param>
      <param name="snapshot" type="ISnapshot" dir="return" safearray="no">
      </param>
    </method>
    <method name="createSharedFolder" const="no" internal="no" wsmap="managed">
      <rest name="create" request="post" path="/vm/{vmid}/sharedfolder/action/"/>
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="hostPath" type="wstring" dir="in" safearray="no">
      </param>
      <param name="writable" type="boolean" dir="in" safearray="no">
      </param>
      <param name="automount" type="boolean" dir="in" safearray="no">
      </param>
      <param name="autoMountPoint" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <method name="removeSharedFolder" const="no" internal="no" wsmap="managed">
      <rest name="remove" request="delete" path="/vm/{vmid}/sharedfolder/action/"/>
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <method name="canShowConsoleWindow" const="no" internal="no" wsmap="managed">
      <param name="canShow" type="boolean" dir="return" safearray="no">
      </param>
    </method>
    <method name="showConsoleWindow" const="no" internal="no" wsmap="managed">
      <param name="winId" type="long long" dir="return" safearray="no">
      </param>
    </method>
    <method name="getGuestProperty" const="yes" internal="no" wsmap="managed">
      <rest name="property" request="get" path="/vm/{vmid}/guest/config/"/>
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="value" type="wstring" dir="out" safearray="no">
      </param>
      <param name="timestamp" type="long long" dir="out" safearray="no">
      </param>
      <param name="flags" type="wstring" dir="out" safearray="no">
      </param>
    </method>
    <method name="getGuestPropertyValue" const="yes" internal="no" wsmap="managed">
      <rest name="propertyvalue" request="get" path="/vm/{vmid}/guest/config/"/>
      <param name="property" type="wstring" dir="in" safearray="no">
      </param>
      <param name="value" type="wstring" dir="return" safearray="no">
      </param>
    </method>
    <method name="getGuestPropertyTimestamp" const="yes" internal="no" wsmap="managed">
      <rest name="propertytimestamp" request="get" path="/vm/{vmid}/guest/config/"/>
      <param name="property" type="wstring" dir="in" safearray="no">
      </param>
      <param name="value" type="long long" dir="return" safearray="no">
      </param>
    </method>
    <method name="setGuestProperty" const="no" internal="no" wsmap="managed">
      <rest name="property" request="post" path="/vm/{vmid}/guest/config/"/>
      <param name="property" type="wstring" dir="in" safearray="no">
      </param>
      <param name="value" type="wstring" dir="in" safearray="no">
      </param>
      <param name="flags" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <method name="setGuestPropertyValue" const="no" internal="no" wsmap="managed">
      <rest name="propertyvalue" request="post" path="/vm/{vmid}/guest/config/"/>
      <param name="property" type="wstring" dir="in" safearray="no">
      </param>
      <param name="value" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <method name="deleteGuestProperty" const="yes" internal="no" wsmap="managed">
      <rest name="remove" request="delete" path="/vm/{vmid}/guest/property/"/>
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <method name="enumerateGuestProperties" const="yes" internal="no" wsmap="managed">
      <rest name="propertylist" request="get" path="/vm/{vmid}/guest/config/"/>
      <param name="patterns" type="wstring" dir="in" safearray="no">
      </param>
      <param name="names" type="wstring" dir="out" safearray="yes">
      </param>
      <param name="values" type="wstring" dir="out" safearray="yes">
      </param>
      <param name="timestamps" type="long long" dir="out" safearray="yes">
      </param>
      <param name="flags" type="wstring" dir="out" safearray="yes">
      </param>
    </method>
    <method name="querySavedGuestScreenInfo" const="yes" internal="no" wsmap="managed">
      <param name="screenId" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="originX" type="unsigned long" dir="out" safearray="no">
      </param>
      <param name="originY" type="unsigned long" dir="out" safearray="no">
      </param>
      <param name="width" type="unsigned long" dir="out" safearray="no">
      </param>
      <param name="height" type="unsigned long" dir="out" safearray="no">
      </param>
      <param name="enabled" type="boolean" dir="out" safearray="no">
      </param>
    </method>
    <method name="readSavedThumbnailToArray" const="no" internal="no" wsmap="managed">
      <param name="screenId" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="bitmapFormat" type="BitmapFormat" dir="in" safearray="no">
      </param>
      <param name="width" type="unsigned long" dir="out" safearray="no">
      </param>
      <param name="height" type="unsigned long" dir="out" safearray="no">
      </param>
      <param name="data" type="octet" safearray="yes" dir="return">
      </param>
    </method>
    <method name="querySavedScreenshotInfo" const="no" internal="no" wsmap="managed">
      <param name="screenId" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="width" type="unsigned long" dir="out" safearray="no">
      </param>
      <param name="height" type="unsigned long" dir="out" safearray="no">
      </param>
      <param name="bitmapFormats" type="BitmapFormat" safearray="yes" dir="return">
      </param>
    </method>
    <method name="readSavedScreenshotToArray" const="no" internal="no" wsmap="managed">
      <param name="screenId" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="bitmapFormat" type="BitmapFormat" dir="in" safearray="no">
      </param>
      <param name="width" type="unsigned long" dir="out" safearray="no">
      </param>
      <param name="height" type="unsigned long" dir="out" safearray="no">
      </param>
      <param name="data" type="octet" dir="return" safearray="yes">
      </param>
    </method>
    <method name="hotPlugCPU" const="no" internal="no" wsmap="managed">
      <rest name="plug" request="post" path="/vm/{vmid}/cpu/action/"/>
      <param name="cpu" type="unsigned long" dir="in" safearray="no">
      </param>
    </method>
    <method name="hotUnplugCPU" const="no" internal="no" wsmap="managed">
      <rest name="unplug" request="post" path="/vm/{vmid}/cpu/action/"/>
      <param name="cpu" type="unsigned long" dir="in" safearray="no">
      </param>
    </method>
    <method name="getCPUStatus" const="no" internal="no" wsmap="managed">
      <rest name="status" request="get" path="/vm/{vmid}/cpu/"/>
      <param name="cpu" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="attached" type="boolean" dir="return" safearray="no">
      </param>
    </method>
    <method name="getEffectiveParavirtProvider" const="yes" internal="no" wsmap="managed">
      <rest name="ParavirtProvider" request="get" path="/vm/{vmid}/config/"/>
      <param name="paravirtProvider" type="ParavirtProvider" dir="return" safearray="no">
      </param>
    </method>
    <method name="queryLogFilename" const="no" internal="no" wsmap="managed">
      <rest name="queryname" request="get" path="/vm/{vmid}/log/"/>
      <param name="idx" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="filename" type="wstring" dir="return" safearray="no">
      </param>
    </method>
    <method name="readLog" const="no" internal="no" wsmap="managed">
      <rest name="read" request="get" path="/vm/{vmid}/log/action/"/>
      <param name="idx" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="offset" type="long long" dir="in" safearray="no">
      </param>
      <param name="size" type="long long" dir="in" safearray="no">
      </param>
      <param name="data" type="octet" dir="return" safearray="yes">
      </param>
    </method>
    <method name="cloneTo" const="no" internal="no" wsmap="managed">
      <rest name="clone" request="post" path="/vm/{vmid}/action/"/>
      <param name="target" type="IMachine" dir="in" safearray="no">
      </param>
      <param name="mode" type="CloneMode" dir="in" safearray="no">
      </param>
      <param name="options" type="CloneOptions" dir="in" safearray="yes">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="moveTo" const="no" internal="no" wsmap="managed">
      <rest name="move" request="post" path="/vm/{vmid}/action/"/>
      <param name="folder" type="wstring" dir="in" safearray="no">
      </param>
      <param name="type" type="wstring" dir="in" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="saveState" const="no" internal="no" wsmap="managed">
      <rest name="save" request="post" path="/vm/{vmid}/action/"/>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="adoptSavedState" const="no" internal="no" wsmap="managed">
      <param name="savedStateFile" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <method name="discardSavedState" const="no" internal="no" wsmap="managed">
      <rest request="post" path="/vm/{vmid}/action/"/>
      <param name="fRemoveFile" type="boolean" dir="in" safearray="no">
      </param>
    </method>
    <method name="takeSnapshot" const="no" internal="no" wsmap="managed">
      <rest name="take" request="put" path="/vm/{vmid}/snapshot/action/"/>
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="description" type="wstring" dir="in" safearray="no">
      </param>
      <param name="pause" type="boolean" dir="in" safearray="no">
      </param>
      <param name="id" type="uuid" mod="string" dir="out" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="deleteSnapshot" const="no" internal="no" wsmap="managed">
      <rest name="remove" request="delete" path="/vm/{vmid}/snapshot/"/>
      <param name="id" type="uuid" mod="string" dir="in" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="deleteSnapshotAndAllChildren" const="no" internal="no" wsmap="managed">
      <rest name="deleteall" request="delete" path="/vm/{vmid}/snapshot/action/"/>
      <param name="id" type="uuid" mod="string" dir="in" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="deleteSnapshotRange" const="no" internal="no" wsmap="managed">
      <rest name="deleterange" request="delete" path="/vm/{vmid}/snapshot/action/"/>
      <param name="startId" type="uuid" mod="string" dir="in" safearray="no">
      </param>
      <param name="endId" type="uuid" mod="string" dir="in" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="restoreSnapshot" const="no" internal="no" wsmap="managed">
      <rest name="restore" request="post" path="/vm/{vmid}/snapshot/action/"/>
      <param name="snapshot" type="ISnapshot" dir="in" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="applyDefaults" const="no" internal="no" wsmap="managed">
      <rest request="post" path="/vm/{vmid}/guest/config/"/>
      <param name="flags" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <method name="changeEncryption" wrap-hint-server="limitedcaller" const="no" internal="no" wsmap="managed">
      <param name="currentPassword" type="wstring" dir="in" safearray="no">
      </param>
      <param name="cipher" type="wstring" dir="in" safearray="no">
      </param>
      <param name="newPassword" type="wstring" dir="in" safearray="no">
      </param>
      <param name="newPasswordId" type="wstring" dir="in" safearray="no">
      </param>
      <param name="force" type="boolean" dir="in" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="getEncryptionSettings" const="yes" wrap-hint-server="limitedcaller" internal="no" wsmap="managed">
      <param name="cipher" type="wstring" dir="out" safearray="no">
      </param>
      <param name="passwordId" type="wstring" dir="out" safearray="no">
      </param>
    </method>
    <method name="checkEncryptionPassword" const="yes" wrap-hint-server="limitedcaller" internal="no" wsmap="managed">
      <param name="password" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <method name="addEncryptionPassword" wrap-hint-server="limitedcaller" const="no" internal="no" wsmap="managed">
      <param name="id" type="wstring" dir="in" safearray="no">
      </param>
      <param name="password" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <method name="addEncryptionPasswords" wrap-hint-server="limitedcaller" const="no" internal="no" wsmap="managed">
      <param name="ids" type="wstring" dir="in" safearray="yes">
      </param>
      <param name="passwords" type="wstring" dir="in" safearray="yes">
      </param>
    </method>
    <method name="removeEncryptionPassword" wrap-hint-server="limitedcaller,passcaller" const="no" internal="no" wsmap="managed">
      <param name="id" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <method name="clearAllEncryptionPasswords" wrap-hint-server="limitedcaller,passcaller" const="no" internal="no" wsmap="managed">
      <result name="VBOX_E_INVALID_VM_STATE">
        The VM is not in proper state e.g powered on.
      </result>
    </method>
  </interface>
  <interface name="IEmulatedUSB" extends="$unknown" uuid="6e253ee8-477a-2497-6759-88b8292a5af0" wsmap="managed" reservedMethods="4" reservedAttributes="4" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <method name="webcamAttach" const="no" internal="no" wsmap="managed">
      <param name="path" type="wstring" dir="in" safearray="no">
      </param>
      <param name="settings" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <method name="webcamDetach" const="no" internal="no" wsmap="managed">
      <param name="path" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <attribute name="webcams" type="wstring" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IVRDEServerInfo" extends="$unknown" uuid="c39ef4d6-7532-45e8-96da-eb5986ae76e4" wsmap="struct" rest="managed" reservedAttributes="8" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="active" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="port" type="long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="numberOfClients" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="beginTime" type="long long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="endTime" type="long long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="bytesSent" type="long long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="bytesSentTotal" type="long long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="bytesReceived" type="long long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="bytesReceivedTotal" type="long long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="user" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="domain" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="clientName" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="clientIP" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="clientVersion" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="encryptionStyle" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IConsole" extends="$unknown" uuid="6ac83d89-6ee7-4e33-8ae6-b257b2e81be8" wsmap="managed" rest="managed" reservedMethods="8" reservedAttributes="8" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="machine" type="IMachine" readonly="yes" rest="uuid" internal="no" safearray="no" wsmap="managed">
    </attribute>
    <attribute name="state" type="MachineState" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="guest" type="IGuest" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="keyboard" type="IKeyboard" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="mouse" type="IMouse" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="display" type="IDisplay" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="debugger" type="IMachineDebugger" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="USBDevices" type="IUSBDevice" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="remoteUSBDevices" type="IHostUSBDevice" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="sharedFolders" type="ISharedFolder" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="VRDEServerInfo" type="IVRDEServerInfo" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="eventSource" type="IEventSource" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="attachedPCIDevices" type="IPCIDeviceAttachment" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="useHostClipboard" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="emulatedUSB" type="IEmulatedUSB" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <method name="powerUp" const="no" internal="no" wsmap="managed">
      <rest request="post" path="/vm/{vmid}/console/action/"/>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="powerUpPaused" const="no" internal="no" wsmap="managed">
      <rest request="post" path="/vm/{vmid}/console/action/"/>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="powerDown" const="no" internal="no" wsmap="managed">
      <rest request="post" path="/vm/{vmid}/console/action/"/>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="reset" const="no" internal="no" wsmap="managed">
      <rest request="post" path="/vm/{vmid}/console/action/"/>
    </method>
    <method name="pause" const="no" internal="no" wsmap="managed">
      <rest request="post" path="/vm/{vmid}/console/action/"/>
    </method>
    <method name="resume" const="no" internal="no" wsmap="managed">
      <rest request="post" path="/vm/{vmid}/console/action/"/>
    </method>
    <method name="powerButton" const="no" internal="no" wsmap="managed">
      <rest request="post" path="/vm/{vmid}/console/action/"/>
    </method>
    <method name="sleepButton" const="no" internal="no" wsmap="managed">
      <rest request="post" path="/vm/{vmid}/console/action/"/>
    </method>
    <method name="getPowerButtonHandled" const="no" internal="no" wsmap="managed">
      <param name="handled" type="boolean" dir="return" safearray="no"/>
    </method>
    <method name="getGuestEnteredACPIMode" const="no" internal="no" wsmap="managed">
      <param name="entered" type="boolean" dir="return" safearray="no"/>
    </method>
    <method name="getDeviceActivity" const="no" internal="no" wsmap="managed">
      <param name="type" type="DeviceType" safearray="yes" dir="in"/>
      <param name="activity" type="DeviceActivity" safearray="yes" dir="return"/>
    </method>
    <method name="attachUSBDevice" const="no" internal="no" wsmap="managed">
      <rest name="attach" request="post" path="/vm/{vmid}/usb/action/"/>
      <param name="id" type="uuid" mod="string" dir="in" safearray="no">
      </param>
      <param name="captureFilename" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <method name="detachUSBDevice" const="no" internal="no" wsmap="managed">
      <rest name="detach" request="post" path="/vm/{vmid}/usb/action/"/>
      <param name="id" type="uuid" mod="string" dir="in" safearray="no">
      </param>
      <param name="device" type="IUSBDevice" dir="return" safearray="no">
      </param>
    </method>
    <method name="findUSBDeviceByAddress" const="no" internal="no" wsmap="managed">
      <rest name="findByAddress" request="get" path="/vm/{vmid}/usb/"/>
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="device" type="IUSBDevice" dir="return" safearray="no">
      </param>
    </method>
    <method name="findUSBDeviceById" const="no" internal="no" wsmap="managed">
      <rest name="findById" request="get" path="/vm/{vmid}/usb/"/>
      <param name="id" type="uuid" mod="string" dir="in" safearray="no">
      </param>
      <param name="device" type="IUSBDevice" dir="return" safearray="no">
      </param>
    </method>
    <method name="createSharedFolder" const="no" internal="no" wsmap="managed">
      <rest request="post" path="/vm/{vmid}/console/action/"/>
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="hostPath" type="wstring" dir="in" safearray="no">
      </param>
      <param name="writable" type="boolean" dir="in" safearray="no">
      </param>
      <param name="automount" type="boolean" dir="in" safearray="no">
      </param>
      <param name="autoMountPoint" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <method name="removeSharedFolder" const="no" internal="no" wsmap="managed">
      <rest request="post" path="/vm/{vmid}/console/action/"/>
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <method name="teleport" const="no" internal="no" wsmap="managed">
      <param name="hostname" type="wstring" dir="in" safearray="no">
      </param>
      <param name="tcpport" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="password" type="wstring" dir="in" safearray="no">
      </param>
      <param name="maxDowntime" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="addEncryptionPassword" const="no" internal="no" wsmap="managed">
      <rest request="post" path="/vm/{vmid}/console/method/"/>
      <param name="id" type="wstring" dir="in" safearray="no">
      </param>
      <param name="password" type="wstring" dir="in" safearray="no">
      </param>
      <param name="clearOnSuspend" type="boolean" dir="in" safearray="no">
      </param>
    </method>
    <method name="addEncryptionPasswords" const="no" internal="no" wsmap="managed">
      <rest request="post" path="/vm/{vmid}/console/method/"/>
      <param name="ids" type="wstring" dir="in" safearray="yes">
      </param>
      <param name="passwords" type="wstring" dir="in" safearray="yes">
      </param>
      <param name="clearOnSuspend" type="boolean" dir="in" safearray="no">
      </param>
    </method>
    <method name="removeEncryptionPassword" const="no" internal="no" wsmap="managed">
      <rest request="post" path="/vm/{vmid}/console/method/"/>
      <param name="id" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <method name="clearAllEncryptionPasswords" const="no" internal="no" wsmap="managed">
      <rest request="post" path="/vm/{vmid}/console/method/"/>
    </method>
  </interface>
  <enum name="HostNetworkInterfaceMediumType" uuid="1aa54aaf-2497-45a2-bfb1-8eb225e93d5b">
    <const name="Unknown" value="0" wsmap="managed">
    </const>
    <const name="Ethernet" value="1" wsmap="managed">
    </const>
    <const name="PPP" value="2" wsmap="managed">
    </const>
    <const name="SLIP" value="3" wsmap="managed">
    </const>
  </enum>
  <enum name="HostNetworkInterfaceStatus" uuid="CC474A69-2710-434B-8D99-C38E5D5A6F41">
    <const name="Unknown" value="0" wsmap="managed">
    </const>
    <const name="Up" value="1" wsmap="managed">
    </const>
    <const name="Down" value="2" wsmap="managed">
    </const>
  </enum>
  <enum name="HostNetworkInterfaceType" uuid="67431b00-9946-48a2-bc02-b25c5919f4f3">
    <const name="Invalid" value="0" wsmap="managed"/>
    <const name="Bridged" value="1" wsmap="managed"/>
    <const name="HostOnly" value="2" wsmap="managed"/>
  </enum>
  <interface name="IHostNetworkInterface" extends="$unknown" uuid="455f8c45-44a0-a470-ba20-27890b96dba9" wsmap="managed" rest="managed" reservedMethods="2" reservedAttributes="4" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="name" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="shortName" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="id" type="uuid" mod="string" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="networkName" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="DHCPEnabled" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="IPAddress" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="networkMask" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="IPV6Supported" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="IPV6Address" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="IPV6NetworkMaskPrefixLength" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="hardwareAddress" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="mediumType" type="HostNetworkInterfaceMediumType" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="status" type="HostNetworkInterfaceStatus" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="interfaceType" type="HostNetworkInterfaceType" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="wireless" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <method name="enableStaticIPConfig" const="no" internal="no" wsmap="managed">
      <param name="IPAddress" type="wstring" dir="in" safearray="no">
      </param>
      <param name="networkMask" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <method name="enableStaticIPConfigV6" const="no" internal="no" wsmap="managed">
      <param name="IPV6Address" type="wstring" dir="in" safearray="no">
      </param>
      <param name="IPV6NetworkMaskPrefixLength" type="unsigned long" dir="in" safearray="no">
      </param>
    </method>
    <method name="enableDynamicIPConfig" const="no" internal="no" wsmap="managed">
    </method>
    <method name="DHCPRediscover" const="no" internal="no" wsmap="managed">
    </method>
  </interface>
  <interface name="IHostVideoInputDevice" extends="$unknown" uuid="e8c25d4d-ac97-4c16-b3e2-81bd8a57cc27" wsmap="managed" rest="managed" reservedAttributes="4" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="name" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="path" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="alias" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <enum name="UpdateChannel" uuid="9f1562a5-e47a-496b-a818-66776f06ce40">
    <const name="Invalid" value="0" wsmap="managed">
    </const>
    <const name="Stable" value="1" wsmap="managed">
    </const>
    <const name="All" value="2" wsmap="managed">
    </const>
    <const name="WithBetas" value="3" wsmap="managed">
    </const>
    <const name="WithTesting" value="4" wsmap="managed">
    </const>
  </enum>
  <enum name="UpdateSeverity" uuid="359a5fee-8a06-4306-8068-2f2dd5cde06f">
    <const name="Invalid" value="0" wsmap="managed">
    </const>
    <const name="Critical" value="1" wsmap="managed">
    </const>
    <const name="Major" value="2" wsmap="managed">
    </const>
    <const name="Minor" value="3" wsmap="managed">
    </const>
    <const name="Testing" value="4" wsmap="managed">
    </const>
  </enum>
  <enum name="UpdateState" uuid="6623e363-c892-45f8-80cb-4e8ffd9b4e60">
    <const name="Invalid" value="0" wsmap="managed">
    </const>
    <const name="Available" value="1" wsmap="managed">
    </const>
    <const name="NotAvailable" value="2" wsmap="managed">
    </const>
    <const name="Downloading" value="3" wsmap="managed">
    </const>
    <const name="Downloaded" value="4" wsmap="managed">
    </const>
    <const name="Installing" value="5" wsmap="managed">
    </const>
    <const name="Installed" value="6" wsmap="managed">
    </const>
    <const name="UserInteraction" value="7" wsmap="managed">
    </const>
    <const name="Canceled" value="8" wsmap="managed">
    </const>
    <const name="Maintenance" value="9" wsmap="managed">
    </const>
    <const name="Error" value="10" wsmap="managed">
    </const>
  </enum>
  <interface name="IUpdateAgent" extends="$unknown" uuid="c4b1b5f4-8cdf-4923-9ef6-b92476a84109" wsmap="managed" reservedMethods="2" reservedAttributes="4" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <method name="checkFor" const="no" internal="no" wsmap="managed">
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="download" const="no" internal="no" wsmap="managed">
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="install" const="no" internal="no" wsmap="managed">
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="rollback" const="no" internal="no" wsmap="managed">
      <result name="VBOX_E_NOT_SUPPORTED">
        Rolling back update not supported.
      </result>
    </method>
    <attribute name="name" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="eventSource" type="IEventSource" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="order" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="dependsOn" type="wstring" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="version" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="downloadUrl" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="webUrl" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="releaseNotes" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="enabled" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="hidden" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="state" type="UpdateState" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="checkFrequency" type="unsigned long" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="channel" type="UpdateChannel" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="repositoryURL" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="lastCheckDate" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="checkCount" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="isCheckNeeded" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="supportedChannels" type="UpdateChannel" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IHostUpdateAgent" extends="IUpdateAgent" uuid="d782dba7-cd4f-4ace-951a-58321c23e258" wsmap="managed" reservedMethods="2" reservedAttributes="4" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="midlDoesNotLikeEmptyInterfaces" readonly="yes" type="boolean" internal="no" safearray="no" wsmap="managed" rest="default"/>
  </interface>
  <interface name="IHostDrivePartition" extends="$unknown" uuid="4f529a14-ace3-407c-9c49-066e8e8027f0" wsmap="struct" rest="managed" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="number" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="size" type="long long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="start" type="long long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="type" type="PartitionType" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="active" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="typeMBR" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="startCylinder" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="startHead" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="startSector" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="endCylinder" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="endHead" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="endSector" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="typeUuid" type="uuid" mod="string" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="uuid" type="uuid" mod="string" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="name" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IHostDrive" extends="$unknown" uuid="70e2e0c3-332c-4d72-b822-2db16e2cb31b" wsmap="managed" rest="managed" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="drivePath" type="wstring" readonly="yes" wrap-hint-server="limitedcaller" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="partitioningType" type="PartitioningType" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="uuid" type="uuid" mod="string" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="sectorSize" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="size" type="long long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="model" type="wstring" readonly="yes" wrap-hint-server="limitedcaller" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="partitions" type="IHostDrivePartition" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IHostX86" extends="$unknown" uuid="50c97996-9cf8-417f-bd79-1e0471367cd3" wsmap="managed" rest="managed" reservedMethods="6" reservedAttributes="12" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <method name="getProcessorCPUIDLeaf" const="no" internal="no" wsmap="managed">
      <param name="cpuId" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="leaf" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="subLeaf" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="valEax" type="unsigned long" dir="out" safearray="no">
      </param>
      <param name="valEbx" type="unsigned long" dir="out" safearray="no">
      </param>
      <param name="valEcx" type="unsigned long" dir="out" safearray="no">
      </param>
      <param name="valEdx" type="unsigned long" dir="out" safearray="no">
      </param>
    </method>
  </interface>
  <interface name="IHost" extends="$unknown" uuid="d5dcece0-b202-4416-a138-03502784cc07" wsmap="managed" rest="managed" reservedMethods="6" reservedAttributes="12" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="architecture" type="PlatformArchitecture" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="x86" type="IHostX86" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="DVDDrives" type="IMedium" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="floppyDrives" type="IMedium" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="audioDevices" type="IHostAudioDevice" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="USBDevices" type="IHostUSBDevice" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="USBDeviceFilters" type="IHostUSBDeviceFilter" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="networkInterfaces" type="IHostNetworkInterface" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="nameServers" type="wstring" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="domainName" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="searchStrings" type="wstring" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="processorCount" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="processorOnlineCount" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="processorCoreCount" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="processorOnlineCoreCount" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="hostDrives" type="IHostDrive" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <method name="getProcessorSpeed" const="no" internal="no" wsmap="managed">
      <param name="cpuId" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="speed" type="unsigned long" dir="return" safearray="no">
      </param>
    </method>
    <method name="getProcessorFeature" const="no" internal="no" wsmap="managed">
      <param name="feature" type="ProcessorFeature" dir="in" safearray="no">
      </param>
      <param name="supported" type="boolean" dir="return" safearray="no">
      </param>
    </method>
    <method name="getProcessorDescription" const="no" internal="no" wsmap="managed">
      <param name="cpuId" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="description" type="wstring" dir="return" safearray="no">
      </param>
    </method>
    <attribute name="memorySize" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="memoryAvailable" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="operatingSystem" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="OSVersion" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="UTCTime" type="long long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <method name="createHostOnlyNetworkInterface" const="no" internal="no" wsmap="managed">
      <param name="hostInterface" type="IHostNetworkInterface" dir="out" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="removeHostOnlyNetworkInterface" const="no" internal="no" wsmap="managed">
      <param name="id" type="uuid" mod="string" dir="in" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="createUSBDeviceFilter" const="no" internal="no" wsmap="managed">
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="filter" type="IHostUSBDeviceFilter" dir="return" safearray="no">
      </param>
    </method>
    <method name="insertUSBDeviceFilter" const="no" internal="no" wsmap="managed">
      <param name="position" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="filter" type="IHostUSBDeviceFilter" dir="in" safearray="no">
      </param>
    </method>
    <method name="removeUSBDeviceFilter" const="no" internal="no" wsmap="managed">
      <param name="position" type="unsigned long" dir="in" safearray="no">
      </param>
    </method>
    <method name="findHostDVDDrive" const="no" internal="no" wsmap="managed">
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="drive" type="IMedium" dir="return" safearray="no">
      </param>
    </method>
    <method name="findHostFloppyDrive" const="no" internal="no" wsmap="managed">
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="drive" type="IMedium" dir="return" safearray="no">
      </param>
    </method>
    <method name="findHostNetworkInterfaceByName" const="no" internal="no" wsmap="managed">
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="networkInterface" type="IHostNetworkInterface" dir="return" safearray="no">
      </param>
    </method>
    <method name="findHostNetworkInterfaceById" const="no" internal="no" wsmap="managed">
      <param name="id" type="uuid" mod="string" dir="in" safearray="no">
      </param>
      <param name="networkInterface" type="IHostNetworkInterface" dir="return" safearray="no">
      </param>
    </method>
    <method name="findHostNetworkInterfacesOfType" const="no" internal="no" wsmap="managed">
      <param name="type" type="HostNetworkInterfaceType" dir="in" safearray="no">
      </param>
      <param name="networkInterfaces" type="IHostNetworkInterface" safearray="yes" dir="return">
      </param>
    </method>
    <method name="findUSBDeviceById" const="no" internal="no" wsmap="managed">
      <param name="id" type="uuid" mod="string" dir="in" safearray="no">
      </param>
      <param name="device" type="IHostUSBDevice" dir="return" safearray="no">
      </param>
    </method>
    <method name="findUSBDeviceByAddress" const="no" internal="no" wsmap="managed">
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="device" type="IHostUSBDevice" dir="return" safearray="no">
      </param>
    </method>
    <method name="generateMACAddress" const="no" internal="no" wsmap="managed">
      <param name="address" type="wstring" dir="return" safearray="no">
      </param>
    </method>
    <attribute name="videoInputDevices" type="IHostVideoInputDevice" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <method name="addUSBDeviceSource" const="no" internal="no" wsmap="managed">
      <param name="backend" type="wstring" dir="in" safearray="no">
      </param>
      <param name="id" type="wstring" dir="in" safearray="no">
      </param>
      <param name="address" type="wstring" dir="in" safearray="no">
      </param>
      <param name="propertyNames" type="wstring" safearray="yes" dir="in">
      </param>
      <param name="propertyValues" type="wstring" safearray="yes" dir="in">
      </param>
    </method>
    <method name="removeUSBDeviceSource" const="no" internal="no" wsmap="managed">
      <param name="id" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <attribute name="updateHost" type="IUpdateAgent" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="updateExtPack" type="IUpdateAgent" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="updateGuestAdditions" type="IUpdateAgent" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <method name="isExecutionEngineSupported" const="no" internal="no" wsmap="managed">
      <param name="cpuArchitecture" type="CPUArchitecture" dir="in" safearray="no">
      </param>
      <param name="executionEngine" type="VMExecutionEngine" dir="in" safearray="no">
      </param>
      <param name="isSupported" type="boolean" dir="return" safearray="no">
      </param>
    </method>
  </interface>
  <interface name="ICPUProfile" extends="$unknown" uuid="b7fda727-7a08-46ee-8dd8-f8d7308b519c" wsmap="managed" rest="managed" reservedMethods="4" reservedAttributes="8" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="name" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="fullName" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="architecture" type="CPUArchitecture" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IPlatformProperties" extends="$unknown" uuid="5bae19d0-ca40-4ca2-a485-c8065190bbe5" wsmap="managed" rest="managed" reservedMethods="3" reservedAttributes="16" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="rawModeSupported" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="exclusiveHwVirt" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="serialPortCount" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="parallelPortCount" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="maxBootPosition" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="supportedParavirtProviders" type="ParavirtProvider" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="supportedFirmwareTypes" type="FirmwareType" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="supportedGuestOSTypes" type="IGuestOSType" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="supportedGfxControllerTypes" type="GraphicsControllerType" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="supportedNetAdpPromiscModePols" type="NetworkAdapterPromiscModePolicy" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="supportedNetworkAdapterTypes" type="NetworkAdapterType" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="supportedUartTypes" type="UartType" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="supportedUSBControllerTypes" type="USBControllerType" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="supportedAudioControllerTypes" type="AudioControllerType" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="supportedBootDevices" type="DeviceType" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="supportedStorageBuses" type="StorageBus" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="supportedStorageControllerTypes" type="StorageControllerType" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="supportedChipsetTypes" type="ChipsetType" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="supportedIommuTypes" type="IommuType" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="supportedTpmTypes" type="TpmType" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <method name="getMaxNetworkAdapters" const="no" internal="no" wsmap="managed">
      <param name="chipset" type="ChipsetType" dir="in" safearray="no">
      </param>
      <param name="maxNetworkAdapters" type="unsigned long" dir="return" safearray="no">
      </param>
    </method>
    <method name="getMaxNetworkAdaptersOfType" const="no" internal="no" wsmap="managed">
      <param name="chipset" type="ChipsetType" dir="in" safearray="no">
      </param>
      <param name="type" type="NetworkAttachmentType" dir="in" safearray="no">
      </param>
      <param name="maxNetworkAdapters" type="unsigned long" dir="return" safearray="no">
      </param>
    </method>
    <method name="getMaxDevicesPerPortForStorageBus" const="no" internal="no" wsmap="managed">
      <param name="bus" type="StorageBus" dir="in" safearray="no">
      </param>
      <param name="maxDevicesPerPort" type="unsigned long" dir="return" safearray="no">
      </param>
    </method>
    <method name="getMinPortCountForStorageBus" const="no" internal="no" wsmap="managed">
      <param name="bus" type="StorageBus" dir="in" safearray="no">
      </param>
      <param name="minPortCount" type="unsigned long" dir="return" safearray="no">
      </param>
    </method>
    <method name="getMaxPortCountForStorageBus" const="no" internal="no" wsmap="managed">
      <param name="bus" type="StorageBus" dir="in" safearray="no">
      </param>
      <param name="maxPortCount" type="unsigned long" dir="return" safearray="no">
      </param>
    </method>
    <method name="getMaxInstancesOfStorageBus" const="no" internal="no" wsmap="managed">
      <param name="chipset" type="ChipsetType" dir="in" safearray="no">
      </param>
      <param name="bus" type="StorageBus" dir="in" safearray="no">
      </param>
      <param name="maxInstances" type="unsigned long" dir="return" safearray="no">
      </param>
    </method>
    <method name="getSupportedVRAMRange" const="no" internal="no" wsmap="managed">
      <param name="graphicsControllerType" type="GraphicsControllerType" dir="in" safearray="no">
      </param>
      <param name="accelerate3DEnabled" type="boolean" dir="in" safearray="no">
      </param>
      <param name="minMB" type="unsigned long" dir="out" safearray="no">
      </param>
      <param name="maxMB" type="unsigned long" dir="out" safearray="no">
      </param>
      <param name="strideSizeMB" type="unsigned long" dir="return" safearray="no">
      </param>
    </method>
    <method name="getSupportedGfxFeaturesForType" const="no" internal="no" wsmap="managed">
      <param name="graphicsControllerType" type="GraphicsControllerType" dir="in" safearray="no">
      </param>
      <param name="supportedControllerFeatures" type="GraphicsFeature" safearray="yes" dir="return">
      </param>
    </method>
    <method name="getDeviceTypesForStorageBus" const="no" internal="no" wsmap="managed">
      <param name="bus" type="StorageBus" dir="in" safearray="no">
      </param>
      <param name="deviceTypes" type="DeviceType" safearray="yes" dir="return">
      </param>
    </method>
    <method name="getStorageBusForControllerType" const="no" internal="no" wsmap="managed">
      <param name="storageControllerType" type="StorageControllerType" dir="in" safearray="no">
      </param>
      <param name="storageBus" type="StorageBus" dir="return" safearray="no">
      </param>
    </method>
    <method name="getStorageControllerTypesForBus" const="no" internal="no" wsmap="managed">
      <param name="storageBus" type="StorageBus" dir="in" safearray="no">
      </param>
      <param name="storageControllerType" type="StorageControllerType" safearray="yes" dir="return">
      </param>
    </method>
    <method name="getStorageControllerHotplugCapable" const="no" internal="no" wsmap="managed">
      <param name="controllerType" type="StorageControllerType" dir="in" safearray="no">
      </param>
      <param name="hotplugCapable" type="boolean" dir="return" safearray="no">
      </param>
    </method>
    <method name="getMaxInstancesOfUSBControllerType" const="no" internal="no" wsmap="managed">
      <param name="chipset" type="ChipsetType" dir="in" safearray="no">
      </param>
      <param name="type" type="USBControllerType" dir="in" safearray="no">
      </param>
      <param name="maxInstances" type="unsigned long" dir="return" safearray="no">
      </param>
    </method>
    <method name="getMinGuestRAM" const="no" internal="no" wsmap="managed">
      <param name="firmware" type="FirmwareType" dir="in" safearray="no">
      </param>
      <param name="minMegabytes" type="unsigned long" dir="return" safearray="no">
      </param>
    </method>
  </interface>
  <enum name="ProxyMode" uuid="885264b3-b517-40fc-ce46-36e3bae895a4">
    <const name="System" value="0" wsmap="managed">
    </const>
    <const name="NoProxy" value="1" wsmap="managed">
    </const>
    <const name="Manual" value="2" wsmap="managed">
    </const>
  </enum>
  <interface name="ISystemProperties" extends="$unknown" uuid="925084b0-625f-422f-a67d-0ede1880a56c" wsmap="managed" rest="managed" reservedMethods="4" reservedAttributes="16" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="platform" type="IPlatformProperties" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="minGuestRAM" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="maxGuestRAM" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="minGuestVRAM" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="maxGuestVRAM" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="minGuestCPUCount" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="maxGuestCPUCount" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="maxGuestMonitors" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="infoVDSize" type="long long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="defaultMachineFolder" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="loggingLevel" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="mediumFormats" type="IMediumFormat" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="defaultHardDiskFormat" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="freeDiskSpaceWarning" type="long long" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="freeDiskSpacePercentWarning" type="unsigned long" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="freeDiskSpaceError" type="long long" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="freeDiskSpacePercentError" type="unsigned long" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="VRDEAuthLibrary" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="webServiceAuthLibrary" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="defaultVRDEExtPack" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="defaultCryptoExtPack" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="logHistoryCount" type="unsigned long" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="defaultAudioDriver" type="AudioDriverType" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="autostartDatabasePath" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="defaultAdditionsISO" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="defaultFrontend" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="screenShotFormats" type="BitmapFormat" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="proxyMode" type="ProxyMode" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="proxyURL" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="supportedPlatformArchitectures" type="PlatformArchitecture" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="supportedClipboardModes" type="ClipboardMode" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="supportedDnDModes" type="DnDMode" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="supportedPointingHIDTypes" type="PointingHIDType" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="supportedKeyboardHIDTypes" type="KeyboardHIDType" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="supportedVFSTypes" type="VFSType" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="supportedImportOptions" type="ImportOptions" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="supportedExportOptions" type="ExportOptions" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="supportedGraphicsFeatures" type="GraphicsFeature" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="supportedRecordingFeatures" type="RecordingFeature" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="supportedRecordingAudioCodecs" type="RecordingAudioCodec" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="supportedRecordingVideoCodecs" type="RecordingVideoCodec" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="supportedRecordingVSModes" type="RecordingVideoScalingMode" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="supportedRecordingARCModes" type="RecordingRateControlMode" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="supportedRecordingVRCModes" type="RecordingRateControlMode" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="supportedCloneOptions" type="CloneOptions" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="supportedAutostopTypes" type="AutostopType" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="supportedVMProcPriorities" type="VMProcPriority" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="supportedNetworkAttachmentTypes" type="NetworkAttachmentType" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="supportedPortModes" type="PortMode" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="supportedAudioDriverTypes" type="AudioDriverType" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="languageId" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <method name="getDefaultIoCacheSettingForStorageController" dtracename="getDefaultStorageCtrlCacheSetting" const="no" internal="no" wsmap="managed">
      <param name="controllerType" type="StorageControllerType" dir="in" safearray="no">
      </param>
      <param name="enabled" type="boolean" dir="return" safearray="no">
      </param>
    </method>
    <method name="getCPUProfiles" const="no" internal="no" wsmap="managed">
      <param name="architecture" type="CPUArchitecture" dir="in" safearray="no">
      </param>
      <param name="namePattern" type="wstring" dir="in" safearray="no">
      </param>
      <param name="profiles" type="ICPUProfile" safearray="yes" dir="return">
      </param>
    </method>
    <method name="getExecutionEnginesForVmCpuArchitecture" const="no" internal="no" wsmap="managed">
      <param name="cpuArchitecture" type="CPUArchitecture" dir="in" safearray="no">
      </param>
      <param name="executionEngine" type="VMExecutionEngine" safearray="yes" dir="return">
      </param>
    </method>
  </interface>
  <interface name="IGuestOSType" extends="$unknown" uuid="c6e30023-751e-487d-9bd1-595c22b104ba" wsmap="struct" rest="managed" reservedAttributes="16" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="familyId" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="familyDescription" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="id" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="subtype" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="description" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="is64Bit" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="platformArchitecture" type="PlatformArchitecture" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="recommendedIOAPIC" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="recommendedVirtEx" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="recommendedRAM" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="recommendedGraphicsController" type="GraphicsControllerType" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="recommendedVRAM" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="recommended3DAcceleration" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="recommendedHDD" type="long long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="adapterType" type="NetworkAdapterType" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="recommendedPAE" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="recommendedDVDStorageController" type="StorageControllerType" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="recommendedDVDStorageBus" type="StorageBus" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="recommendedHDStorageController" type="StorageControllerType" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="recommendedHDStorageBus" type="StorageBus" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="recommendedFirmware" type="FirmwareType" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="recommendedUSBHID" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="recommendedHPET" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="recommendedUSBTablet" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="recommendedRTCUseUTC" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="recommendedChipset" type="ChipsetType" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="recommendedIommuType" type="IommuType" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="recommendedAudioController" type="AudioControllerType" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="recommendedAudioCodec" type="AudioCodecType" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="recommendedFloppy" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="recommendedUSB" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="recommendedUSB3" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="recommendedTFReset" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="recommendedX2APIC" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="recommendedCPUCount" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="recommendedTpmType" type="TpmType" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="recommendedSecureBoot" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="recommendedWDDMGraphics" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="guestAdditionsInstallPackageName" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <enum name="AdditionsFacilityType" uuid="c4b10d74-dd48-4ff4-9a40-785a2a389ade">
    <const name="None" value="0" wsmap="managed">
    </const>
    <const name="VBoxGuestDriver" value="20" wsmap="managed">
    </const>
    <const name="AutoLogon" value="90" wsmap="managed">
    </const>
    <const name="VBoxService" value="100" wsmap="managed">
    </const>
    <const name="VBoxTrayClient" value="101" wsmap="managed">
    </const>
    <const name="Seamless" value="1000" wsmap="managed">
    </const>
    <const name="Graphics" value="1100" wsmap="managed">
    </const>
    <const name="MonitorAttach" value="1101" wsmap="managed">
    </const>
    <const name="All" value="2147483646" wsmap="managed">
    </const>
  </enum>
  <enum name="AdditionsFacilityClass" uuid="446451b2-c88d-4e5d-84c9-91bc7f533f5f">
    <const name="None" value="0" wsmap="managed">
    </const>
    <const name="Driver" value="10" wsmap="managed">
    </const>
    <const name="Service" value="30" wsmap="managed">
    </const>
    <const name="Program" value="50" wsmap="managed">
    </const>
    <const name="Feature" value="100" wsmap="managed">
    </const>
    <const name="ThirdParty" value="999" wsmap="managed">
    </const>
    <const name="All" value="2147483646" wsmap="managed">
    </const>
  </enum>
  <enum name="AdditionsFacilityStatus" uuid="ce06f9e1-394e-4fe9-9368-5a88c567dbde">
    <const name="Inactive" value="0" wsmap="managed">
    </const>
    <const name="Paused" value="1" wsmap="managed">
    </const>
    <const name="PreInit" value="20" wsmap="managed">
    </const>
    <const name="Init" value="30" wsmap="managed">
    </const>
    <const name="Active" value="50" wsmap="managed">
    </const>
    <const name="Terminating" value="100" wsmap="managed">
    </const>
    <const name="Terminated" value="101" wsmap="managed">
    </const>
    <const name="Failed" value="800" wsmap="managed">
    </const>
    <const name="Unknown" value="999" wsmap="managed">
    </const>
  </enum>
  <interface name="IAdditionsFacility" extends="$unknown" uuid="f2f7fae4-4a06-81fc-a916-78b2da1fa0e5" wsmap="struct" rest="managed" reservedAttributes="2" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="classType" type="AdditionsFacilityClass" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="lastUpdated" type="long long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="name" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="status" type="AdditionsFacilityStatus" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="type" type="AdditionsFacilityType" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <enum name="AdditionsRunLevelType" uuid="a25417ee-a9dd-4f5b-b0dc-377860087754">
    <const name="None" value="0" wsmap="managed">
    </const>
    <const name="System" value="1" wsmap="managed">
    </const>
    <const name="Userland" value="2" wsmap="managed">
    </const>
    <const name="Desktop" value="3" wsmap="managed">
    </const>
  </enum>
  <enum name="AdditionsUpdateFlag" uuid="726a818d-18d6-4389-94e8-3e9e6826171a">
    <const name="None" value="0" wsmap="managed">
    </const>
    <const name="WaitForUpdateStartOnly" value="1" wsmap="managed">
    </const>
  </enum>
  <enum name="GuestShutdownFlag" uuid="28D19C9C-5862-4930-B29A-F117712B4864">
    <const name="None" value="0" wsmap="managed">
    </const>
    <const name="PowerOff" value="1" wsmap="managed">
    </const>
    <const name="Reboot" value="2" wsmap="managed">
    </const>
    <const name="Force" value="4" wsmap="managed">
    </const>
  </enum>
  <enum name="GuestSessionStatus" uuid="ac2669da-4624-44f2-85b5-0b0bfb8d8673">
    <const name="Undefined" value="0" wsmap="managed">
    </const>
    <const name="Starting" value="10" wsmap="managed">
    </const>
    <const name="Started" value="100" wsmap="managed">
    </const>
    <const name="Terminating" value="480" wsmap="managed">
    </const>
    <const name="Terminated" value="500" wsmap="managed">
    </const>
    <const name="TimedOutKilled" value="512" wsmap="managed">
    </const>
    <const name="TimedOutAbnormally" value="513" wsmap="managed">
    </const>
    <const name="Down" value="600" wsmap="managed">
    </const>
    <const name="Error" value="800" wsmap="managed">
    </const>
  </enum>
  <enum name="GuestSessionWaitForFlag" uuid="bb7a372a-f635-4e11-a81a-e707f3a52ef5">
    <const name="None" value="0" wsmap="managed">
    </const>
    <const name="Start" value="1" wsmap="managed">
    </const>
    <const name="Terminate" value="2" wsmap="managed">
    </const>
    <const name="Status" value="4" wsmap="managed">
    </const>
  </enum>
  <enum name="GuestSessionWaitResult" uuid="c0f6a8a5-fdb6-42bf-a582-56c6f82bcd2d">
    <const name="None" value="0" wsmap="managed">
    </const>
    <const name="Start" value="1" wsmap="managed">
    </const>
    <const name="Terminate" value="2" wsmap="managed">
    </const>
    <const name="Status" value="3" wsmap="managed">
    </const>
    <const name="Error" value="4" wsmap="managed">
    </const>
    <const name="Timeout" value="5" wsmap="managed">
    </const>
    <const name="WaitFlagNotSupported" value="6" wsmap="managed">
    </const>
  </enum>
  <enum name="GuestUserState" uuid="b2a82b02-fd3d-4fc2-ba84-6ba5ac8be198">
    <const name="Unknown" value="0" wsmap="managed">
    </const>
    <const name="LoggedIn" value="1" wsmap="managed">
    </const>
    <const name="LoggedOut" value="2" wsmap="managed">
    </const>
    <const name="Locked" value="3" wsmap="managed">
    </const>
    <const name="Unlocked" value="4" wsmap="managed">
    </const>
    <const name="Disabled" value="5" wsmap="managed">
    </const>
    <const name="Idle" value="6" wsmap="managed">
    </const>
    <const name="InUse" value="7" wsmap="managed">
    </const>
    <const name="Created" value="8" wsmap="managed">
    </const>
    <const name="Deleted" value="9" wsmap="managed">
    </const>
    <const name="SessionChanged" value="10" wsmap="managed">
    </const>
    <const name="CredentialsChanged" value="11" wsmap="managed">
    </const>
    <const name="RoleChanged" value="12" wsmap="managed">
    </const>
    <const name="GroupAdded" value="13" wsmap="managed">
    </const>
    <const name="GroupRemoved" value="14" wsmap="managed">
    </const>
    <const name="Elevated" value="15" wsmap="managed">
    </const>
  </enum>
  <enum name="FileSeekOrigin" uuid="ad32f789-4279-4530-979c-f16892e1c263">
    <const name="Begin" value="0" wsmap="managed">
    </const>
    <const name="Current" value="1" wsmap="managed">
    </const>
    <const name="End" value="2" wsmap="managed">
    </const>
  </enum>
  <enum name="ProcessInputFlag" uuid="5d38c1dd-2604-4ddf-92e5-0c0cdd3bdbd5">
    <const name="None" value="0" wsmap="managed">
    </const>
    <const name="EndOfFile" value="1" wsmap="managed">
    </const>
  </enum>
  <enum name="ProcessOutputFlag" uuid="9979e85a-52bb-40b7-870c-57115e27e0f1">
    <const name="None" value="0" wsmap="managed">
    </const>
    <const name="StdErr" value="1" wsmap="managed">
    </const>
  </enum>
  <enum name="ProcessWaitForFlag" uuid="23b550c7-78e1-437e-98f0-65fd9757bcd2">
    <const name="None" value="0" wsmap="managed">
    </const>
    <const name="Start" value="1" wsmap="managed">
    </const>
    <const name="Terminate" value="2" wsmap="managed">
    </const>
    <const name="StdIn" value="4" wsmap="managed">
    </const>
    <const name="StdOut" value="8" wsmap="managed">
    </const>
    <const name="StdErr" value="16" wsmap="managed">
    </const>
  </enum>
  <enum name="ProcessWaitResult" uuid="40719cbe-f192-4fe9-a231-6697b3c8e2b4">
    <const name="None" value="0" wsmap="managed">
    </const>
    <const name="Start" value="1" wsmap="managed">
    </const>
    <const name="Terminate" value="2" wsmap="managed">
    </const>
    <const name="Status" value="3" wsmap="managed">
    </const>
    <const name="Error" value="4" wsmap="managed">
    </const>
    <const name="Timeout" value="5" wsmap="managed">
    </const>
    <const name="StdIn" value="6" wsmap="managed">
    </const>
    <const name="StdOut" value="7" wsmap="managed">
    </const>
    <const name="StdErr" value="8" wsmap="managed">
    </const>
    <const name="WaitFlagNotSupported" value="9" wsmap="managed">
    </const>
  </enum>
  <enum name="FileCopyFlag" uuid="791909d7-4c64-2fa4-4303-adb10658d347">
    <const name="None" value="0" wsmap="managed">
    </const>
    <const name="NoReplace" value="1" wsmap="managed">
    </const>
    <const name="FollowLinks" value="2" wsmap="managed">
    </const>
    <const name="Update" value="4" wsmap="managed">
    </const>
  </enum>
  <enum name="FsObjMoveFlag" uuid="2450a05d-80c6-4c96-9a17-94d73293ff86">
    <const name="None" value="0" wsmap="managed">
    </const>
    <const name="Replace" value="1" wsmap="managed">
    </const>
    <const name="FollowLinks" value="2" wsmap="managed">
    </const>
    <const name="AllowDirectoryMoves" value="4" wsmap="managed">
    </const>
  </enum>
  <enum name="DirectoryCreateFlag" uuid="bd721b0e-ced5-4f79-b368-249897c32a36">
    <const name="None" value="0" wsmap="managed">
    </const>
    <const name="Parents" value="1" wsmap="managed">
    </const>
  </enum>
  <enum name="DirectoryCopyFlag" uuid="20108C67-B1EB-4EF6-869B-25539A47A18E">
    <const name="None" value="0" wsmap="managed">
    </const>
    <const name="CopyIntoExisting" value="1" wsmap="managed">
    </const>
    <const name="Recursive" value="2" wsmap="managed">
    </const>
    <const name="FollowLinks" value="4" wsmap="managed">
    </const>
  </enum>
  <enum name="DirectoryRemoveRecFlag" uuid="455aabf0-7692-48f6-9061-f21579b65769">
    <const name="None" value="0" wsmap="managed">
    </const>
    <const name="ContentAndDir" value="1" wsmap="managed">
    </const>
    <const name="ContentOnly" value="2" wsmap="managed">
    </const>
  </enum>
  <enum name="FsObjRenameFlag" uuid="59bbf3a1-4e23-d7cf-05d5-ccae32080ed2">
    <const name="NoReplace" value="0" wsmap="managed">
    </const>
    <const name="Replace" value="1" wsmap="managed">
    </const>
  </enum>
  <enum name="ProcessCreateFlag" uuid="C544CD2B-F02D-4886-9901-71C523DB8DC5">
    <const name="None" value="0" wsmap="managed">
    </const>
    <const name="WaitForProcessStartOnly" value="1" wsmap="managed">
    </const>
    <const name="IgnoreOrphanedProcesses" value="2" wsmap="managed">
    </const>
    <const name="Hidden" value="4" wsmap="managed">
    </const>
    <const name="Profile" value="8" wsmap="managed">
    </const>
    <const name="WaitForStdOut" value="16" wsmap="managed">
    </const>
    <const name="WaitForStdErr" value="32" wsmap="managed">
    </const>
    <const name="ExpandArguments" value="64" wsmap="managed">
    </const>
    <const name="UnquotedArguments" value="128" wsmap="managed">
    </const>
  </enum>
  <enum name="ProcessPriority" uuid="ee8cac50-e232-49fe-806b-d1214d9c2e49">
    <const name="Invalid" value="0" wsmap="managed">
    </const>
    <const name="Default" value="1" wsmap="managed">
    </const>
  </enum>
  <enum name="SymlinkType" uuid="37794668-f8f1-4714-98a5-6f8fa2ed0118">
    <const name="Unknown" value="0" wsmap="managed">
    </const>
    <const name="Directory" value="1" wsmap="managed">
    </const>
    <const name="File" value="2" wsmap="managed">
    </const>
  </enum>
  <enum name="SymlinkReadFlag" uuid="b7fe2b9d-790e-4b25-8adf-1ca33026931f">
    <const name="None" value="0" wsmap="managed">
    </const>
    <const name="NoSymlinks" value="1" wsmap="managed">
    </const>
  </enum>
  <enum name="ProcessStatus" uuid="4d52368f-5b48-4bfe-b486-acf89139b52f">
    <const name="Undefined" value="0" wsmap="managed">
    </const>
    <const name="Starting" value="10" wsmap="managed">
    </const>
    <const name="Started" value="100" wsmap="managed">
    </const>
    <const name="Paused" value="110" wsmap="managed">
    </const>
    <const name="Terminating" value="480" wsmap="managed">
    </const>
    <const name="TerminatedNormally" value="500" wsmap="managed">
    </const>
    <const name="TerminatedSignal" value="510" wsmap="managed">
    </const>
    <const name="TerminatedAbnormally" value="511" wsmap="managed">
    </const>
    <const name="TimedOutKilled" value="512" wsmap="managed">
    </const>
    <const name="TimedOutAbnormally" value="513" wsmap="managed">
    </const>
    <const name="Down" value="600" wsmap="managed">
    </const>
    <const name="Error" value="800" wsmap="managed">
    </const>
  </enum>
  <enum name="ProcessInputStatus" uuid="a4a0ef9c-29cc-4805-9803-c8215ae9da6c">
    <const name="Undefined" value="0" wsmap="managed">
    </const>
    <const name="Broken" value="1" wsmap="managed">
    </const>
    <const name="Available" value="10" wsmap="managed">
    </const>
    <const name="Written" value="50" wsmap="managed">
    </const>
    <const name="Overflow" value="100" wsmap="managed">
    </const>
  </enum>
  <enum name="PathStyle" uuid="97303a5b-42e8-0a55-d16f-d2a92c295261">
    <const name="DOS" value="1" wsmap="managed">
    </const>
    <const name="UNIX" value="2" wsmap="managed">
    </const>
    <const name="Unknown" value="8" wsmap="managed">
    </const>
  </enum>
  <enum name="DirectoryStatus" uuid="a50ca1fc-85a9-4a7a-b755-68c3db01caf1">
    <const name="Undefined" value="0" wsmap="managed">
    </const>
    <const name="Open" value="1" wsmap="managed">
    </const>
    <const name="Close" value="2" wsmap="managed">
    </const>
    <const name="Rewind" value="3" wsmap="managed">
    </const>
    <const name="Down" value="4" wsmap="managed">
    </const>
    <const name="Error" value="5" wsmap="managed">
    </const>
  </enum>
  <enum name="FileAccessMode" uuid="231a578f-47fb-ea30-3b3e-8489558227f0">
    <const name="ReadOnly" value="1" wsmap="managed">
    </const>
    <const name="WriteOnly" value="2" wsmap="managed">
    </const>
    <const name="ReadWrite" value="3" wsmap="managed">
    </const>
    <const name="AppendOnly" value="4" wsmap="managed">
    </const>
    <const name="AppendRead" value="5" wsmap="managed">
    </const>
  </enum>
  <enum name="FileOpenAction" uuid="12bc97e2-4fc6-a8b4-4f84-0cbf4ab970d2">
    <const name="OpenExisting" value="1" wsmap="managed">
    </const>
    <const name="OpenOrCreate" value="2" wsmap="managed">
    </const>
    <const name="CreateNew" value="3" wsmap="managed">
    </const>
    <const name="CreateOrReplace" value="4" wsmap="managed">
    </const>
    <const name="OpenExistingTruncated" value="5" wsmap="managed">
    </const>
    <const name="AppendOrCreate" value="99" wsmap="managed">
    </const>
  </enum>
  <enum name="FileSharingMode" uuid="f87dfe58-425b-c5ba-7d6d-22adeea25de1">
    <const name="Read" value="1" wsmap="managed">
    </const>
    <const name="Write" value="2" wsmap="managed">
    </const>
    <const name="ReadWrite" value="3" wsmap="managed">
    </const>
    <const name="Delete" value="4" wsmap="managed">
    </const>
    <const name="ReadDelete" value="5" wsmap="managed">
    </const>
    <const name="WriteDelete" value="6" wsmap="managed">
    </const>
    <const name="All" value="7" wsmap="managed">
    </const>
  </enum>
  <enum name="FileOpenExFlag" uuid="4671abd4-f70c-42aa-8542-6c169cb87a5c">
    <const name="None" value="0" wsmap="managed">
    </const>
  </enum>
  <enum name="FileStatus" uuid="8c86468b-b97b-4080-8914-e29f5b0abd2c">
    <const name="Undefined" value="0" wsmap="managed">
    </const>
    <const name="Opening" value="10" wsmap="managed">
    </const>
    <const name="Open" value="100" wsmap="managed">
    </const>
    <const name="Closing" value="150" wsmap="managed">
    </const>
    <const name="Closed" value="200" wsmap="managed">
    </const>
    <const name="Down" value="600" wsmap="managed">
    </const>
    <const name="Error" value="800" wsmap="managed">
    </const>
  </enum>
  <enum name="FsObjType" uuid="34a0d1aa-491e-e209-e150-84964d6cee5f">
    <const name="Unknown" value="1" wsmap="managed">
    </const>
    <const name="Fifo" value="2" wsmap="managed">
    </const>
    <const name="DevChar" value="3" wsmap="managed">
    </const>
    <const name="Directory" value="4" wsmap="managed">
    </const>
    <const name="DevBlock" value="5" wsmap="managed">
    </const>
    <const name="File" value="6" wsmap="managed">
    </const>
    <const name="Symlink" value="7" wsmap="managed">
    </const>
    <const name="Socket" value="8" wsmap="managed">
    </const>
    <const name="WhiteOut" value="9" wsmap="managed">
    </const>
  </enum>
  <enum name="DnDAction" uuid="17609e74-778e-4d0e-8827-35f5230f287b">
    <const name="Ignore" value="0" wsmap="managed">
    </const>
    <const name="Copy" value="1" wsmap="managed">
    </const>
    <const name="Move" value="2" wsmap="managed">
    </const>
    <const name="Link" value="3" wsmap="managed">
    </const>
  </enum>
  <enum name="DirectoryOpenFlag" uuid="92f838f6-cbf9-4760-bb63-92c567773366">
    <const name="None" value="0" wsmap="managed">
    </const>
    <const name="NoSymlinks" value="1" wsmap="managed">
    </const>
    <const name="DenyAscent" value="2" wsmap="managed">
    </const>
    <const name="NoFollowSymlinks" value="4" wsmap="managed">
    </const>
  </enum>
  <interface name="IDnDBase" extends="$unknown" uuid="00727A73-000A-4C4A-006D-E7D300351186" wsmap="managed" reservedMethods="1" reservedAttributes="2" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="formats" type="wstring" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <method name="isFormatSupported" const="no" internal="no" wsmap="managed">
      <param name="format" type="wstring" dir="in" safearray="no">
      </param>
      <param name="supported" type="boolean" dir="return" safearray="no">
      </param>
    </method>
    <method name="addFormats" const="no" internal="no" wsmap="managed">
      <param name="formats" type="wstring" safearray="yes" dir="in">
      </param>
    </method>
    <method name="removeFormats" const="no" internal="no" wsmap="managed">
      <param name="formats" type="wstring" safearray="yes" dir="in">
      </param>
    </method>
  </interface>
  <interface name="IDnDSource" extends="IDnDBase" uuid="d23a9ca3-42da-c94b-8aec-21968e08355d" wsmap="managed" reservedMethods="1" reservedAttributes="2" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <method name="dragIsPending" const="no" internal="no" wsmap="managed">
      <param name="screenId" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="formats" type="wstring" dir="out" safearray="yes">
      </param>
      <param name="allowedActions" type="DnDAction" dir="out" safearray="yes">
      </param>
      <param name="defaultAction" type="DnDAction" dir="return" safearray="no">
      </param>
    </method>
    <method name="drop" const="no" internal="no" wsmap="managed">
      <param name="format" type="wstring" dir="in" safearray="no">
      </param>
      <param name="action" type="DnDAction" dir="in" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="receiveData" const="no" internal="no" wsmap="managed">
      <param name="data" type="octet" safearray="yes" dir="return">
      </param>
    </method>
  </interface>
  <interface name="IGuestDnDSource" extends="IDnDSource" uuid="dedfb5d9-4c1b-edf7-fdf3-c1be6827dc28" wsmap="managed" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="midlDoesNotLikeEmptyInterfaces" readonly="yes" type="boolean" internal="no" safearray="no" wsmap="managed" rest="default"/>
  </interface>
  <interface name="IDnDTarget" extends="IDnDBase" uuid="ff5befc3-4ba3-7903-2aa4-43988ba11554" wsmap="managed" reservedMethods="1" reservedAttributes="2" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <method name="enter" const="no" internal="no" wsmap="managed">
      <param name="screenId" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="y" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="x" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="defaultAction" type="DnDAction" dir="in" safearray="no">
      </param>
      <param name="allowedActions" type="DnDAction" dir="in" safearray="yes">
      </param>
      <param name="formats" type="wstring" dir="in" safearray="yes">
      </param>
      <param name="resultAction" type="DnDAction" dir="return" safearray="no">
      </param>
    </method>
    <method name="move" const="no" internal="no" wsmap="managed">
      <param name="screenId" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="x" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="y" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="defaultAction" type="DnDAction" dir="in" safearray="no">
      </param>
      <param name="allowedActions" type="DnDAction" dir="in" safearray="yes">
      </param>
      <param name="formats" type="wstring" dir="in" safearray="yes">
      </param>
      <param name="resultAction" type="DnDAction" dir="return" safearray="no">
      </param>
    </method>
    <method name="leave" const="no" internal="no" wsmap="managed">
      <param name="screenId" type="unsigned long" dir="in" safearray="no">
      </param>
    </method>
    <method name="drop" const="no" internal="no" wsmap="managed">
      <param name="screenId" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="x" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="y" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="defaultAction" type="DnDAction" dir="in" safearray="no">
      </param>
      <param name="allowedActions" type="DnDAction" dir="in" safearray="yes">
      </param>
      <param name="formats" type="wstring" dir="in" safearray="yes">
      </param>
      <param name="format" type="wstring" dir="out" safearray="no">
      </param>
      <param name="resultAction" type="DnDAction" dir="return" safearray="no">
      </param>
    </method>
    <method name="sendData" const="no" internal="no" wsmap="managed">
      <param name="screenId" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="format" type="wstring" dir="in" safearray="no">
      </param>
      <param name="data" type="octet" dir="in" safearray="yes">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="cancel" const="no" internal="no" wsmap="managed">
      <param name="veto" type="boolean" dir="return" safearray="no">
      </param>
    </method>
  </interface>
  <interface name="IGuestDnDTarget" extends="IDnDTarget" uuid="50ce4b51-0ff7-46b7-a138-3c6e5ac946b4" wsmap="managed" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="midlDoesNotLikeEmptyInterfaces" readonly="yes" type="boolean" internal="no" safearray="no" wsmap="managed" rest="default"/>
  </interface>
  <interface name="IGuestSession" extends="$unknown" uuid="8b2b6773-8b5a-4cd2-95f8-38faf73913e1" wsmap="managed" reservedMethods="8" reservedAttributes="12" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="user" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="domain" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="name" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="id" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="timeout" type="unsigned long" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="protocolVersion" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="status" type="GuestSessionStatus" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="environmentChanges" type="wstring" safearray="yes" readonly="no" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="environmentBase" type="wstring" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="processes" type="IGuestProcess" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="pathStyle" type="PathStyle" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="currentDirectory" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="userHome" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="userDocuments" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="mountPoints" type="wstring" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="directories" type="IGuestDirectory" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="files" type="IGuestFile" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="eventSource" type="IEventSource" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <method name="close" const="no" internal="no" wsmap="managed">
    </method>
    <method name="copyFromGuest" const="no" internal="no" wsmap="managed">
      <param name="sources" type="wstring" dir="in" safearray="yes">
      </param>
      <param name="filters" type="wstring" dir="in" safearray="yes">
      </param>
      <param name="flags" type="wstring" dir="in" safearray="yes">
      </param>
      <param name="destination" type="wstring" dir="in" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="copyToGuest" const="no" internal="no" wsmap="managed">
      <param name="sources" type="wstring" dir="in" safearray="yes">
      </param>
      <param name="filters" type="wstring" dir="in" safearray="yes">
      </param>
      <param name="flags" type="wstring" dir="in" safearray="yes">
      </param>
      <param name="destination" type="wstring" dir="in" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="directoryCopy" const="no" internal="no" wsmap="managed">
      <param name="source" type="wstring" dir="in" safearray="no">
      </param>
      <param name="destination" type="wstring" dir="in" safearray="no">
      </param>
      <param name="flags" type="DirectoryCopyFlag" dir="in" safearray="yes">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="directoryCopyFromGuest" const="no" internal="no" wsmap="managed">
      <param name="source" type="wstring" dir="in" safearray="no">
      </param>
      <param name="destination" type="wstring" dir="in" safearray="no">
      </param>
      <param name="flags" type="DirectoryCopyFlag" dir="in" safearray="yes">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="directoryCopyToGuest" const="no" internal="no" wsmap="managed">
      <param name="source" type="wstring" dir="in" safearray="no">
      </param>
      <param name="destination" type="wstring" dir="in" safearray="no">
      </param>
      <param name="flags" type="DirectoryCopyFlag" dir="in" safearray="yes">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="directoryCreate" const="no" internal="no" wsmap="managed">
      <param name="path" type="wstring" dir="in" safearray="no">
      </param>
      <param name="mode" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="flags" type="DirectoryCreateFlag" dir="in" safearray="yes">
      </param>
    </method>
    <method name="directoryCreateTemp" const="no" internal="no" wsmap="managed">
      <param name="templateName" type="wstring" dir="in" safearray="no">
      </param>
      <param name="mode" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="path" type="wstring" dir="in" safearray="no">
      </param>
      <param name="secure" type="boolean" dir="in" safearray="no">
      </param>
      <param name="directory" type="wstring" dir="return" safearray="no">
      </param>
    </method>
    <method name="directoryExists" const="no" internal="no" wsmap="managed">
      <param name="path" type="wstring" dir="in" safearray="no">
      </param>
      <param name="followSymlinks" type="boolean" dir="in" safearray="no">
      </param>
      <param name="exists" type="boolean" dir="return" safearray="no">
      </param>
    </method>
    <method name="directoryOpen" const="no" internal="no" wsmap="managed">
      <param name="path" type="wstring" dir="in" safearray="no">
      </param>
      <param name="filter" type="wstring" dir="in" safearray="no">
      </param>
      <param name="flags" type="DirectoryOpenFlag" dir="in" safearray="yes">
      </param>
      <param name="directory" type="IGuestDirectory" dir="return" safearray="no">
      </param>
    </method>
    <method name="directoryRemove" const="no" internal="no" wsmap="managed">
      <param name="path" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <method name="directoryRemoveRecursive" const="no" internal="no" wsmap="managed">
      <param name="path" type="wstring" dir="in" safearray="no">
      </param>
      <param name="flags" type="DirectoryRemoveRecFlag" dir="in" safearray="yes">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="environmentScheduleSet" const="no" internal="no" wsmap="managed">
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="value" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <method name="environmentScheduleUnset" const="no" internal="no" wsmap="managed">
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <method name="environmentGetBaseVariable" const="no" internal="no" wsmap="managed">
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="value" type="wstring" dir="return" safearray="no">
      </param>
    </method>
    <method name="environmentDoesBaseVariableExist" const="no" internal="no" wsmap="managed">
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="exists" type="boolean" dir="return" safearray="no">
      </param>
    </method>
    <method name="fileCopy" const="no" internal="no" wsmap="managed">
      <param name="source" type="wstring" dir="in" safearray="no">
      </param>
      <param name="destination" type="wstring" dir="in" safearray="no">
      </param>
      <param name="flags" type="FileCopyFlag" dir="in" safearray="yes">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="fileCopyFromGuest" const="no" internal="no" wsmap="managed">
      <param name="source" type="wstring" dir="in" safearray="no">
      </param>
      <param name="destination" type="wstring" dir="in" safearray="no">
      </param>
      <param name="flags" type="FileCopyFlag" dir="in" safearray="yes">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="fileCopyToGuest" const="no" internal="no" wsmap="managed">
      <param name="source" type="wstring" dir="in" safearray="no">
      </param>
      <param name="destination" type="wstring" dir="in" safearray="no">
      </param>
      <param name="flags" type="FileCopyFlag" dir="in" safearray="yes">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="fileCreateTemp" const="no" internal="no" wsmap="managed">
      <param name="templateName" type="wstring" dir="in" safearray="no">
      </param>
      <param name="mode" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="path" type="wstring" dir="in" safearray="no">
      </param>
      <param name="secure" type="boolean" dir="in" safearray="no">
      </param>
      <param name="file" type="IGuestFile" dir="return" safearray="no">
      </param>
    </method>
    <method name="fileExists" const="no" internal="no" wsmap="managed">
      <param name="path" type="wstring" dir="in" safearray="no">
      </param>
      <param name="followSymlinks" type="boolean" dir="in" safearray="no">
      </param>
      <param name="exists" type="boolean" dir="return" safearray="no">
      </param>
    </method>
    <method name="fileOpen" const="no" internal="no" wsmap="managed">
      <param name="path" type="wstring" dir="in" safearray="no">
      </param>
      <param name="accessMode" type="FileAccessMode" dir="in" safearray="no">
      </param>
      <param name="openAction" type="FileOpenAction" dir="in" safearray="no">
      </param>
      <param name="creationMode" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="file" type="IGuestFile" dir="return" safearray="no">
      </param>
    </method>
    <method name="fileOpenEx" const="no" internal="no" wsmap="managed">
      <param name="path" type="wstring" dir="in" safearray="no">
      </param>
      <param name="accessMode" type="FileAccessMode" dir="in" safearray="no">
      </param>
      <param name="openAction" type="FileOpenAction" dir="in" safearray="no">
      </param>
      <param name="sharingMode" type="FileSharingMode" dir="in" safearray="no">
      </param>
      <param name="creationMode" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="flags" type="FileOpenExFlag" dir="in" safearray="yes">
      </param>
      <param name="file" type="IGuestFile" dir="return" safearray="no">
      </param>
    </method>
    <method name="fileQuerySize" const="no" internal="no" wsmap="managed">
      <param name="path" type="wstring" dir="in" safearray="no">
      </param>
      <param name="followSymlinks" type="boolean" dir="in" safearray="no">
      </param>
      <param name="size" type="long long" dir="return" safearray="no">
      </param>
    </method>
    <method name="fsObjExists" const="no" internal="no" wsmap="managed">
      <param name="path" type="wstring" dir="in" safearray="no">
      </param>
      <param name="followSymlinks" type="boolean" dir="in" safearray="no">
      </param>
      <param name="exists" type="boolean" dir="return" safearray="no">
      </param>
    </method>
    <method name="fsObjQueryInfo" const="no" internal="no" wsmap="managed">
      <param name="path" type="wstring" dir="in" safearray="no">
      </param>
      <param name="followSymlinks" type="boolean" dir="in" safearray="no">
      </param>
      <param name="info" type="IGuestFsObjInfo" dir="return" safearray="no">
      </param>
    </method>
    <method name="fsObjRemove" const="no" internal="no" wsmap="managed">
      <param name="path" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <method name="fsObjRemoveArray" const="no" internal="no" wsmap="managed">
      <param name="path" type="wstring" dir="in" safearray="yes">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="fsObjRename" const="no" internal="no" wsmap="managed">
      <param name="oldPath" type="wstring" dir="in" safearray="no">
      </param>
      <param name="newPath" type="wstring" dir="in" safearray="no">
      </param>
      <param name="flags" type="FsObjRenameFlag" dir="in" safearray="yes">
      </param>
    </method>
    <method name="fsObjMove" const="no" internal="no" wsmap="managed">
      <param name="source" type="wstring" dir="in" safearray="no">
      </param>
      <param name="destination" type="wstring" dir="in" safearray="no">
      </param>
      <param name="flags" type="FsObjMoveFlag" dir="in" safearray="yes">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="fsObjMoveArray" const="no" internal="no" wsmap="managed">
      <param name="source" type="wstring" dir="in" safearray="yes">
      </param>
      <param name="destination" type="wstring" dir="in" safearray="no">
      </param>
      <param name="flags" type="FsObjMoveFlag" dir="in" safearray="yes">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="fsObjCopyArray" const="no" internal="no" wsmap="managed">
      <param name="source" type="wstring" dir="in" safearray="yes">
      </param>
      <param name="destination" type="wstring" dir="in" safearray="no">
      </param>
      <param name="flags" type="FileCopyFlag" dir="in" safearray="yes">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="fsObjSetACL" const="no" internal="no" wsmap="managed">
      <param name="path" type="wstring" dir="in" safearray="no">
      </param>
      <param name="followSymlinks" type="boolean" dir="in" safearray="no">
      </param>
      <param name="acl" type="wstring" dir="in" safearray="no">
      </param>
      <param name="mode" type="unsigned long" dir="in" safearray="no">
      </param>
    </method>
    <method name="fsQueryFreeSpace" const="no" internal="no" wsmap="managed">
      <param name="path" type="wstring" dir="in" safearray="no">
      </param>
      <param name="freeSpace" type="long long" dir="return" safearray="no">
      </param>
    </method>
    <method name="fsQueryInfo" const="no" internal="no" wsmap="managed">
      <param name="path" type="wstring" dir="in" safearray="no">
      </param>
      <param name="info" type="IGuestFsInfo" dir="return" safearray="no">
      </param>
    </method>
    <method name="processCreate" const="no" internal="no" wsmap="managed">
      <param name="executable" type="wstring" dir="in" safearray="no">
      </param>
      <param name="arguments" type="wstring" dir="in" safearray="yes">
      </param>
      <param name="cwd" type="wstring" dir="in" safearray="no">
      </param>
      <param name="environmentChanges" type="wstring" dir="in" safearray="yes">
      </param>
      <param name="flags" type="ProcessCreateFlag" dir="in" safearray="yes">
      </param>
      <param name="timeoutMS" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="guestProcess" type="IGuestProcess" dir="return" safearray="no">
      </param>
    </method>
    <method name="processCreateEx" const="no" internal="no" wsmap="managed">
      <param name="executable" type="wstring" dir="in" safearray="no">
      </param>
      <param name="arguments" type="wstring" dir="in" safearray="yes">
      </param>
      <param name="cwd" type="wstring" dir="in" safearray="no">
      </param>
      <param name="environmentChanges" type="wstring" dir="in" safearray="yes">
      </param>
      <param name="flags" type="ProcessCreateFlag" dir="in" safearray="yes">
      </param>
      <param name="timeoutMS" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="priority" type="ProcessPriority" dir="in" safearray="no">
      </param>
      <param name="affinity" type="long" dir="in" safearray="yes">
      </param>
      <param name="guestProcess" type="IGuestProcess" dir="return" safearray="no">
      </param>
    </method>
    <method name="processGet" const="no" internal="no" wsmap="managed">
      <param name="pid" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="guestProcess" type="IGuestProcess" dir="return" safearray="no">
      </param>
    </method>
    <method name="symlinkCreate" const="no" internal="no" wsmap="managed">
      <param name="symlink" type="wstring" dir="in" safearray="no">
      </param>
      <param name="target" type="wstring" dir="in" safearray="no">
      </param>
      <param name="type" type="SymlinkType" dir="in" safearray="no">
      </param>
    </method>
    <method name="symlinkExists" const="no" internal="no" wsmap="managed">
      <param name="symlink" type="wstring" dir="in" safearray="no">
      </param>
      <param name="exists" type="boolean" dir="return" safearray="no">
      </param>
    </method>
    <method name="symlinkRead" const="no" internal="no" wsmap="managed">
      <param name="symlink" type="wstring" dir="in" safearray="no">
      </param>
      <param name="flags" type="SymlinkReadFlag" dir="in" safearray="yes">
      </param>
      <param name="target" type="wstring" dir="return" safearray="no">
      </param>
    </method>
    <method name="waitFor" const="no" internal="no" wsmap="managed">
      <param name="waitFor" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="timeoutMS" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="reason" type="GuestSessionWaitResult" dir="return" safearray="no">
      </param>
    </method>
    <method name="waitForArray" const="no" internal="no" wsmap="managed">
      <param name="waitFor" type="GuestSessionWaitForFlag" dir="in" safearray="yes">
      </param>
      <param name="timeoutMS" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="reason" type="GuestSessionWaitResult" dir="return" safearray="no">
      </param>
    </method>
  </interface>
  <interface name="IProcess" extends="$unknown" uuid="bc68370c-8a02-45f3-a07d-a67aa72756aa" wsmap="managed" reservedMethods="4" reservedAttributes="8" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="arguments" type="wstring" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="environment" type="wstring" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="eventSource" type="IEventSource" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="executablePath" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="exitCode" type="long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="name" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="PID" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="status" type="ProcessStatus" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <method name="waitFor" const="no" internal="no" wsmap="managed">
      <param name="waitFor" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="timeoutMS" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="reason" type="ProcessWaitResult" dir="return" safearray="no">
      </param>
    </method>
    <method name="waitForArray" const="no" internal="no" wsmap="managed">
      <param name="waitFor" type="ProcessWaitForFlag" dir="in" safearray="yes">
      </param>
      <param name="timeoutMS" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="reason" type="ProcessWaitResult" dir="return" safearray="no">
      </param>
    </method>
    <method name="read" const="no" internal="no" wsmap="managed">
      <param name="handle" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="toRead" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="timeoutMS" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="data" type="octet" dir="return" safearray="yes">
      </param>
    </method>
    <method name="write" const="no" internal="no" wsmap="managed">
      <param name="handle" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="flags" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="data" type="octet" dir="in" safearray="yes">
      </param>
      <param name="timeoutMS" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="written" type="unsigned long" dir="return" safearray="no">
      </param>
    </method>
    <method name="writeArray" const="no" internal="no" wsmap="managed">
      <param name="handle" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="flags" type="ProcessInputFlag" dir="in" safearray="yes">
      </param>
      <param name="data" type="octet" dir="in" safearray="yes">
      </param>
      <param name="timeoutMS" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="written" type="unsigned long" dir="return" safearray="no">
      </param>
    </method>
    <method name="terminate" const="no" internal="no" wsmap="managed">
    </method>
  </interface>
  <interface name="IGuestProcess" extends="IProcess" uuid="35cf4b3f-4453-4f3e-c9b8-5686939c80b6" wsmap="managed" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="midlDoesNotLikeEmptyInterfaces" readonly="yes" type="boolean" internal="no" safearray="no" wsmap="managed" rest="default"/>
  </interface>
  <interface name="IDirectory" extends="$unknown" uuid="a508e094-bf24-4eca-80c6-467766a1e4c0" wsmap="managed" reservedMethods="4" reservedAttributes="8" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="directoryName" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="eventSource" type="IEventSource" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="filter" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="id" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="status" type="DirectoryStatus" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <method name="close" const="no" internal="no" wsmap="managed">
    </method>
    <method name="list" const="no" internal="no" wsmap="managed">
      <param name="maxEntries" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="objInfo" type="IFsObjInfo" safearray="yes" dir="return">
      </param>
    </method>
    <method name="read" const="no" internal="no" wsmap="managed">
      <param name="objInfo" type="IFsObjInfo" dir="return" safearray="no">
      </param>
    </method>
    <method name="rewind" const="no" internal="no" wsmap="managed">
    </method>
  </interface>
  <interface name="IGuestDirectory" extends="IDirectory" uuid="cc830458-4974-a19c-4dc6-cc98c2269626" wsmap="managed" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="midlDoesNotLikeEmptyInterfaces" readonly="yes" type="boolean" internal="no" safearray="no" wsmap="managed" rest="default"/>
  </interface>
  <interface name="IFile" extends="$unknown" uuid="59a235ac-2f1a-4d6c-81fc-e3fa843f49ae" wsmap="managed" reservedMethods="4" reservedAttributes="8" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="eventSource" type="IEventSource" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="id" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="initialSize" type="long long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="offset" type="long long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="status" type="FileStatus" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="filename" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="creationMode" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="openAction" type="FileOpenAction" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="accessMode" type="FileAccessMode" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <method name="close" const="no" internal="no" wsmap="managed">
    </method>
    <method name="queryInfo" const="no" internal="no" wsmap="managed">
      <param name="objInfo" type="IFsObjInfo" dir="return" safearray="no">
      </param>
    </method>
    <method name="querySize" const="no" internal="no" wsmap="managed">
      <param name="size" type="long long" dir="return" safearray="no">
      </param>
    </method>
    <method name="read" const="no" internal="no" wsmap="managed">
      <param name="toRead" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="timeoutMS" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="data" type="octet" dir="return" safearray="yes">
      </param>
    </method>
    <method name="readAt" const="no" internal="no" wsmap="managed">
      <param name="offset" type="long long" dir="in" safearray="no">
      </param>
      <param name="toRead" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="timeoutMS" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="data" type="octet" dir="return" safearray="yes">
      </param>
    </method>
    <method name="seek" const="no" internal="no" wsmap="managed">
      <param name="offset" type="long long" dir="in" safearray="no">
      </param>
      <param name="whence" type="FileSeekOrigin" dir="in" safearray="no">
      </param>
      <param name="newOffset" type="long long" dir="return" safearray="no">
      </param>
    </method>
    <method name="setACL" const="no" internal="no" wsmap="managed">
      <param name="acl" type="wstring" dir="in" safearray="no">
      </param>
      <param name="mode" type="unsigned long" dir="in" safearray="no">
      </param>
    </method>
    <method name="setSize" const="no" internal="no" wsmap="managed">
      <param name="size" type="long long" dir="in" safearray="no">
      </param>
    </method>
    <method name="write" const="no" internal="no" wsmap="managed">
      <param name="data" type="octet" dir="in" safearray="yes">
      </param>
      <param name="timeoutMS" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="written" type="unsigned long" dir="return" safearray="no">
      </param>
    </method>
    <method name="writeAt" const="no" internal="no" wsmap="managed">
      <param name="offset" type="long long" dir="in" safearray="no">
      </param>
      <param name="data" type="octet" dir="in" safearray="yes">
      </param>
      <param name="timeoutMS" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="written" type="unsigned long" dir="return" safearray="no">
      </param>
    </method>
  </interface>
  <interface name="IGuestFile" extends="IFile" uuid="92f21dc0-44de-1653-b717-2ebf0ca9b664" wsmap="managed" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="midlDoesNotLikeEmptyInterfaces" readonly="yes" type="boolean" internal="no" safearray="no" wsmap="managed" rest="default"/>
  </interface>
  <interface name="IFsInfo" extends="$unknown" uuid="3db2ab1a-6cf7-42f1-8bf5-e1c0553e0b30" wsmap="managed" reservedAttributes="8" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="freeSize" type="long long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="totalSize" type="long long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="blockSize" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="sectorSize" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="serialNumber" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="isRemote" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="isCaseSensitive" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="isReadOnly" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="isCompressed" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="supportsFileCompression" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="maxComponent" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="type" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="label" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="mountPoint" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IFsObjInfo" extends="$unknown" uuid="081fc833-c6fa-430e-6020-6a505d086387" wsmap="managed" reservedAttributes="8" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="name" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="type" type="FsObjType" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="fileAttributes" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="objectSize" type="long long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="allocatedSize" type="long long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="accessTime" type="long long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="birthTime" type="long long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="changeTime" type="long long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="modificationTime" type="long long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="UID" type="long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="userName" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="GID" type="long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="groupName" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="nodeId" type="long long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="nodeIdDevice" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="hardLinks" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="deviceNumber" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="generationId" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="userFlags" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IGuestFsInfo" extends="IFsInfo" uuid="726eaca9-091e-41b4-bca6-355efe864107" wsmap="managed" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="midlDoesNotLikeEmptyInterfaces" readonly="yes" type="boolean" internal="no" safearray="no" wsmap="managed" rest="default"/>
  </interface>
  <interface name="IGuestFsObjInfo" extends="IFsObjInfo" uuid="6620db85-44e0-ca69-e9e0-d4907ceccbe5" wsmap="managed" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="midlDoesNotLikeEmptyInterfaces" readonly="yes" type="boolean" internal="no" safearray="no" wsmap="managed" rest="default"/>
  </interface>
  <interface name="IGuest" extends="$unknown" uuid="00892186-A4AF-4627-B21F-FC561CE4473C" wsmap="managed" reservedMethods="8" reservedAttributes="16" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="OSTypeId" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="additionsRunLevel" type="AdditionsRunLevelType" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="additionsVersion" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="additionsRevision" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="dnDSource" type="IGuestDnDSource" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="dnDTarget" type="IGuestDnDTarget" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="eventSource" type="IEventSource" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="facilities" type="IAdditionsFacility" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="sessions" type="IGuestSession" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="memoryBalloonSize" type="unsigned long" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="statisticsUpdateInterval" type="unsigned long" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <method name="internalGetStatistics" const="no" internal="no" wsmap="managed">
      <param name="cpuUser" type="unsigned long" dir="out" safearray="no">
      </param>
      <param name="cpuKernel" type="unsigned long" dir="out" safearray="no">
      </param>
      <param name="cpuIdle" type="unsigned long" dir="out" safearray="no">
      </param>
      <param name="memTotal" type="unsigned long" dir="out" safearray="no">
      </param>
      <param name="memFree" type="unsigned long" dir="out" safearray="no">
      </param>
      <param name="memBalloon" type="unsigned long" dir="out" safearray="no">
      </param>
      <param name="memShared" type="unsigned long" dir="out" safearray="no">
      </param>
      <param name="memCache" type="unsigned long" dir="out" safearray="no">
      </param>
      <param name="pagedTotal" type="unsigned long" dir="out" safearray="no">
      </param>
      <param name="memAllocTotal" type="unsigned long" dir="out" safearray="no">
      </param>
      <param name="memFreeTotal" type="unsigned long" dir="out" safearray="no">
      </param>
      <param name="memBalloonTotal" type="unsigned long" dir="out" safearray="no">
      </param>
      <param name="memSharedTotal" type="unsigned long" dir="out" safearray="no">
      </param>
    </method>
    <method name="getFacilityStatus" const="no" internal="no" wsmap="managed">
      <param name="facility" type="AdditionsFacilityType" dir="in" safearray="no">
      </param>
      <param name="timestamp" type="long long" dir="out" safearray="no">
      </param>
      <param name="status" type="AdditionsFacilityStatus" dir="return" safearray="no">
      </param>
    </method>
    <method name="getAdditionsStatus" const="no" internal="no" wsmap="managed">
      <param name="level" type="AdditionsRunLevelType" dir="in" safearray="no">
      </param>
      <param name="active" type="boolean" dir="return" safearray="no">
      </param>
    </method>
    <method name="setCredentials" const="no" internal="no" wsmap="managed">
      <param name="userName" type="wstring" dir="in" safearray="no">
      </param>
      <param name="password" type="wstring" dir="in" safearray="no">
      </param>
      <param name="domain" type="wstring" dir="in" safearray="no">
      </param>
      <param name="allowInteractiveLogon" type="boolean" dir="in" safearray="no">
      </param>
    </method>
    <method name="createSession" const="no" internal="no" wsmap="managed">
      <param name="user" type="wstring" dir="in" safearray="no">
      </param>
      <param name="password" type="wstring" dir="in" safearray="no">
      </param>
      <param name="domain" type="wstring" dir="in" safearray="no">
      </param>
      <param name="sessionName" type="wstring" dir="in" safearray="no">
      </param>
      <param name="guestSession" type="IGuestSession" dir="return" safearray="no">
      </param>
    </method>
    <method name="findSession" const="no" internal="no" wsmap="managed">
      <param name="sessionName" type="wstring" dir="in" safearray="no">
      </param>
      <param name="sessions" type="IGuestSession" safearray="yes" dir="return">
      </param>
    </method>
    <method name="shutdown" const="no" internal="no" wsmap="managed">
      <param name="flags" type="GuestShutdownFlag" dir="in" safearray="yes">
      </param>
    </method>
    <method name="updateGuestAdditions" const="no" internal="no" wsmap="managed">
      <param name="source" type="wstring" dir="in" safearray="no">
      </param>
      <param name="arguments" type="wstring" dir="in" safearray="yes">
      </param>
      <param name="flags" type="AdditionsUpdateFlag" dir="in" safearray="yes">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
  </interface>
  <interface name="IProgress" extends="$unknown" uuid="d7b98d2b-30e8-447e-99cb-e31becae6ae4" wsmap="managed" rest="managed" wrap-hint-server-addinterfaces="IInternalProgressControl" reservedMethods="8" reservedAttributes="12" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="id" type="uuid" mod="string" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="description" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="initiator" type="$unknown" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="cancelable" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="percent" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="timeRemaining" type="long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="completed" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="canceled" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="resultCode" type="long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="errorInfo" type="IVirtualBoxErrorInfo" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="operationCount" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="operation" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="operationDescription" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="operationPercent" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="operationWeight" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="timeout" type="unsigned long" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="eventSource" type="IEventSource" readonly="yes" rest="suppress" internal="no" safearray="no" wsmap="managed"/>
    <method name="waitForCompletion" const="no" internal="no" wsmap="managed">
      <param name="timeout" type="long" dir="in" safearray="no">
      </param>
    </method>
    <method name="waitForOperationCompletion" const="no" internal="no" wsmap="managed">
      <param name="operation" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="timeout" type="long" dir="in" safearray="no">
      </param>
    </method>
    <method name="cancel" const="no" internal="no" wsmap="managed">
      <rest request="post" path="/progress/{progressid}/action/"/>
    </method>
  </interface>
  <interface name="IInternalProgressControl" extends="$unknown" uuid="41a033b8-cc87-4f6e-a0e9-47bb7f2d4be5" internal="yes" wsmap="suppress" reservedMethods="8" reservedAttributes="8" default="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <method name="setCurrentOperationProgress" const="no" internal="no" wsmap="managed">
      <param name="percent" type="unsigned long" dir="in" safearray="no"/>
    </method>
    <method name="waitForOtherProgressCompletion" const="no" internal="no" wsmap="managed">
      <param name="progressOther" type="IProgress" dir="in" safearray="no">
      </param>
      <param name="timeoutMS" type="unsigned long" dir="in" safearray="no">
      </param>
    </method>
    <method name="setNextOperation" const="no" internal="no" wsmap="managed">
      <param name="nextOperationDescription" type="wstring" dir="in" safearray="no"/>
      <param name="nextOperationsWeight" type="unsigned long" dir="in" safearray="no"/>
    </method>
    <method name="notifyPointOfNoReturn" const="no" internal="no" wsmap="managed">
    </method>
    <method name="notifyComplete" const="no" internal="no" wsmap="managed">
      <param name="resultCode" type="long" dir="in" safearray="no"/>
      <param name="errorInfo" type="IVirtualBoxErrorInfo" dir="in" safearray="no"/>
    </method>
  </interface>
  <interface name="ISnapshot" extends="$unknown" uuid="6cc49055-dad4-4496-85cf-3f76bcb3b5fa" wsmap="managed" rest="managed" reservedMethods="4" reservedAttributes="8" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="id" type="uuid" mod="string" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="name" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="description" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="timeStamp" type="long long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="online" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="machine" type="IMachine" readonly="yes" rest="uuid" internal="no" safearray="no" wsmap="managed">
    </attribute>
    <attribute name="parent" type="ISnapshot" readonly="yes" rest="uuid" internal="no" safearray="no" wsmap="managed">
    </attribute>
    <attribute name="children" type="ISnapshot" readonly="yes" safearray="yes" rest="uuid" internal="no" wsmap="managed">
    </attribute>
    <attribute name="childrenCount" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <enum name="MediumState" uuid="ef41e980-e012-43cd-9dea-479d4ef14d13">
    <const name="NotCreated" value="0" wsmap="managed">
    </const>
    <const name="Created" value="1" wsmap="managed">
    </const>
    <const name="LockedRead" value="2" wsmap="managed">
    </const>
    <const name="LockedWrite" value="3" wsmap="managed">
    </const>
    <const name="Inaccessible" value="4" wsmap="managed">
    </const>
    <const name="Creating" value="5" wsmap="managed">
    </const>
    <const name="Deleting" value="6" wsmap="managed">
    </const>
  </enum>
  <enum name="MediumType" uuid="fe663fb5-c244-4e1b-9d81-c628b417dd04">
    <const name="Normal" value="0" wsmap="managed">
    </const>
    <const name="Immutable" value="1" wsmap="managed">
    </const>
    <const name="Writethrough" value="2" wsmap="managed">
    </const>
    <const name="Shareable" value="3" wsmap="managed">
    </const>
    <const name="Readonly" value="4" wsmap="managed">
    </const>
    <const name="MultiAttach" value="5" wsmap="managed">
    </const>
  </enum>
  <enum name="MediumVariant" uuid="0282e97f-4ef3-4411-a8e0-47c384803cb6">
    <const name="Standard" value="0" wsmap="managed">
    </const>
    <const name="VmdkSplit2G" value="0x01" wsmap="managed">
    </const>
    <const name="VmdkRawDisk" value="0x02" wsmap="managed">
    </const>
    <const name="VmdkStreamOptimized" value="0x04" wsmap="managed">
    </const>
    <const name="VmdkESX" value="0x08" wsmap="managed">
    </const>
    <const name="VdiZeroExpand" value="0x100" wsmap="managed">
    </const>
    <const name="Fixed" value="0x10000" wsmap="managed">
    </const>
    <const name="Diff" value="0x20000" wsmap="managed">
    </const>
    <const name="Formatted" value="0x20000000" wsmap="managed">
    </const>
    <const name="NoCreateDir" value="0x40000000" wsmap="managed">
    </const>
  </enum>
  <interface name="IMediumAttachment" extends="$unknown" uuid="8d095cb0-0126-43e0-b05d-326e74abb356" wsmap="struct" rest="managed" reservedAttributes="8" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="machine" type="IMachine" readonly="yes" rest="uuid" internal="no" safearray="no" wsmap="managed">
    </attribute>
    <attribute name="medium" type="IMedium" readonly="yes" rest="uuid" internal="no" safearray="no" wsmap="managed">
    </attribute>
    <attribute name="controller" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="port" type="long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="device" type="long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="type" type="DeviceType" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="passthrough" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="temporaryEject" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="isEjected" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="nonRotational" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="discard" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="hotPluggable" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="bandwidthGroup" type="IBandwidthGroup" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IMedium" extends="$unknown" uuid="7d510820-a678-4730-a862-818dcd3fbed0" wsmap="managed" rest="managed" reservedMethods="7" reservedAttributes="12" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="id" type="uuid" mod="string" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="description" type="wstring" wrap-hint-server="passcaller" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="state" type="MediumState" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="variant" type="MediumVariant" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="location" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="name" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="deviceType" type="DeviceType" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="hostDrive" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="size" type="long long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="format" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="mediumFormat" type="IMediumFormat" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="type" type="MediumType" wrap-hint-server="passcaller" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="allowedTypes" type="MediumType" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="parent" type="IMedium" readonly="yes" wrap-hint-server="passcaller" rest="uuid" internal="no" safearray="no" wsmap="managed">
    </attribute>
    <attribute name="children" type="IMedium" safearray="yes" readonly="yes" wrap-hint-server="passcaller" rest="uuid" internal="no" wsmap="managed">
    </attribute>
    <attribute name="base" type="IMedium" readonly="yes" wrap-hint-server="passcaller" rest="uuid" internal="no" safearray="no" wsmap="managed">
    </attribute>
    <attribute name="readOnly" type="boolean" readonly="yes" wrap-hint-server="passcaller" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="logicalSize" type="long long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="autoReset" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="lastAccessError" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="machineIds" type="uuid" mod="string" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <method name="setIds" wrap-hint-server="passcaller" const="no" internal="no" wsmap="managed">
      <rest request="post" path="/media/{mediumid}/config/"/>
      <param name="setImageId" type="boolean" dir="in" safearray="no">
      </param>
      <param name="imageId" type="uuid" mod="string" dir="in" safearray="no">
      </param>
      <param name="setParentId" type="boolean" dir="in" safearray="no">
      </param>
      <param name="parentId" type="uuid" mod="string" dir="in" safearray="no">
      </param>
        <result name="E_INVALIDARG">
          Invalid parameter combination.
        </result>
      <result name="VBOX_E_NOT_SUPPORTED">
        Medium is not a hard disk medium.
      </result>
    </method>
    <method name="refreshState" wrap-hint-server="passcaller" const="no" internal="no" wsmap="managed">
      <rest request="post" path="/media/{mediumid}/config/"/>
      <param name="state" type="MediumState" dir="return" safearray="no">
      </param>
    </method>
    <method name="getSnapshotIds" const="no" internal="no" wsmap="managed">
      <rest name="SnapshotIdList" request="get" path="/media/{mediumid}/"/>
      <param name="machineId" type="uuid" mod="string" dir="in" safearray="no">
      </param>
      <param name="snapshotIds" type="uuid" mod="string" safearray="yes" dir="return">
      </param>
    </method>
    <method name="lockRead" const="no" internal="no" wsmap="managed">
      <rest request="post" path="/media/{mediumid}/config/"/>
      <param name="token" type="IToken" dir="return" safearray="no">
      </param>
    </method>
    <method name="lockWrite" const="no" internal="no" wsmap="managed">
      <rest request="post" path="/media/{mediumid}/config/"/>
      <param name="token" type="IToken" dir="return" safearray="no">
      </param>
    </method>
    <method name="close" wrap-hint-server="passcaller" const="no" internal="no" wsmap="managed">
      <rest request="post" path="/media/{mediumid}/action/"/>
    </method>
    <method name="getProperty" const="yes" internal="no" wsmap="managed">
      <rest name="property" request="get" path="/media/{mediumid}/config/"/>
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="value" type="wstring" dir="return" safearray="no">
      </param>
    </method>
    <method name="setProperty" const="no" internal="no" wsmap="managed">
      <rest name="property" request="post" path="/media/{mediumid}/config/"/>
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="value" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <method name="getProperties" const="yes" internal="no" wsmap="managed">
      <rest name="propertylist" request="get" path="/media/{mediumid}/config/"/>
      <param name="names" type="wstring" dir="in" safearray="no">
      </param>
      <param name="returnNames" type="wstring" safearray="yes" dir="out">
      </param>
      <param name="returnValues" type="wstring" safearray="yes" dir="return">
      </param>
    </method>
    <method name="setProperties" const="no" internal="no" wsmap="managed">
      <rest name="propertylist" request="post" path="/media/{mediumid}/config/"/>
      <param name="names" type="wstring" safearray="yes" dir="in">
      </param>
      <param name="values" type="wstring" safearray="yes" dir="in">
      </param>
    </method>
    <method name="createBaseStorage" const="no" internal="no" wsmap="managed">
      <rest request="post" path="/media/{mediumid}/action/"/>
      <param name="logicalSize" type="long long" dir="in" safearray="no">
      </param>
      <param name="variant" type="MediumVariant" safearray="yes" dir="in">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="deleteStorage" const="no" internal="no" wsmap="managed">
      <rest request="post" path="/media/{mediumid}/action/"/>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="createDiffStorage" wrap-hint-server="passcaller" const="no" internal="no" wsmap="managed">
      <rest request="post" path="/media/{mediumid}/action/"/>
      <param name="target" type="IMedium" dir="in" safearray="no">
      </param>
      <param name="variant" type="MediumVariant" safearray="yes" dir="in">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="mergeTo" const="no" internal="no" wsmap="managed">
      <rest name="merge" request="post" path="/media/{mediumid}/action/"/>
      <param name="target" type="IMedium" dir="in" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="cloneTo" const="no" internal="no" wsmap="managed">
      <rest name="clone" request="post" path="/media/{mediumid}/action/"/>
      <param name="target" type="IMedium" dir="in" safearray="no">
      </param>
      <param name="variant" type="MediumVariant" safearray="yes" dir="in">
      </param>
      <param name="parent" type="IMedium" dir="in" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="cloneToBase" const="no" internal="no" wsmap="managed">
      <rest request="post" path="/media/{mediumid}/action/"/>
    <param name="target" type="IMedium" dir="in" safearray="no">
    </param>
    <param name="variant" type="MediumVariant" safearray="yes" dir="in">
    </param>
    <param name="progress" type="IProgress" dir="return" safearray="no">
    </param>
    </method>
    <method name="moveTo" wrap-hint-server="passcaller" const="no" internal="no" wsmap="managed">
      <rest name="move" request="post" path="/media/{mediumid}/action/"/>
      <param name="location" type="wstring" dir="in" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="compact" const="no" internal="no" wsmap="managed">
      <rest request="post" path="/media/{mediumid}/action/"/>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="resize" const="no" internal="no" wsmap="managed">
      <rest request="post" path="/media/{mediumid}/action/"/>
      <param name="logicalSize" type="long long" dir="in" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="reset" wrap-hint-server="passcaller" const="no" internal="no" wsmap="managed">
      <rest request="post" path="/media/{mediumid}/action/"/>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="changeEncryption" const="no" internal="no" wsmap="managed">
      <rest name="encript" request="post" path="/media/{mediumid}/action/"/>
      <param name="currentPassword" type="wstring" dir="in" safearray="no">
      </param>
      <param name="cipher" type="wstring" dir="in" safearray="no">
      </param>
      <param name="newPassword" type="wstring" dir="in" safearray="no">
      </param>
      <param name="newPasswordId" type="wstring" dir="in" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="getEncryptionSettings" const="yes" wrap-hint-server="passcaller" internal="no" wsmap="managed">
      <rest name="EncryptionSettingList" request="get" path="/media/{mediumid}/config/"/>
      <param name="cipher" type="wstring" dir="out" safearray="no">
      </param>
      <param name="passwordId" type="wstring" dir="return" safearray="no">
      </param>
    </method>
    <method name="checkEncryptionPassword" const="yes" internal="no" wsmap="managed">
      <rest request="get" path="/media/{mediumid}/config/"/>
      <param name="password" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <method name="openForIO" const="no" internal="no" wsmap="managed">
      <param name="writable" type="boolean" dir="in" safearray="no">
      </param>
      <param name="password" type="wstring" dir="in" safearray="no">
      </param>
      <param name="mediumIO" type="IMediumIO" dir="return" safearray="no">
      </param>
    </method>
    <method name="resizeAndCloneTo" const="no" internal="no" wsmap="managed">
      <rest name="resizeAndClone" request="post" path="/media/{mediumid}/action/"/>
      <param name="target" type="IMedium" dir="in" safearray="no">
      </param>
      <param name="logicalSize" type="long long" dir="in" safearray="no">
      </param>
      <param name="variant" type="MediumVariant" safearray="yes" dir="in">
      </param>
      <param name="parent" type="IMedium" dir="in" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
  </interface>
  <enum name="DataType" uuid="d90ea51e-a3f1-4a01-beb1-c1723c0d3ba7">
    <const name="Int32" value="0" wsmap="managed"/>
    <const name="Int8" value="1" wsmap="managed"/>
    <const name="String" value="2" wsmap="managed"/>
  </enum>
  <enum name="DataFlags" uuid="86884dcf-1d6b-4f1b-b4bf-f5aa44959d60">
    <const name="None" value="0x00" wsmap="managed"/>
    <const name="Mandatory" value="0x01" wsmap="managed"/>
    <const name="Expert" value="0x02" wsmap="managed"/>
    <const name="Array" value="0x04" wsmap="managed"/>
    <const name="FlagMask" value="0x07" wsmap="managed"/>
  </enum>
  <enum name="MediumFormatCapabilities" uuid="7342ba79-7ce0-4d94-8f86-5ed5a185d9bd">
    <const name="Uuid" value="0x01" wsmap="managed">
    </const>
    <const name="CreateFixed" value="0x02" wsmap="managed">
    </const>
    <const name="CreateDynamic" value="0x04" wsmap="managed">
    </const>
    <const name="CreateSplit2G" value="0x08" wsmap="managed">
    </const>
    <const name="Differencing" value="0x10" wsmap="managed">
    </const>
    <const name="Asynchronous" value="0x20" wsmap="managed">
    </const>
    <const name="File" value="0x40" wsmap="managed">
    </const>
    <const name="Properties" value="0x80" wsmap="managed">
    </const>
    <const name="TcpNetworking" value="0x100" wsmap="managed">
    </const>
    <const name="VFS" value="0x200" wsmap="managed">
    </const>
    <const name="Discard" value="0x400" wsmap="managed">
    </const>
    <const name="Preferred" value="0x800" wsmap="managed">
    </const>
    <const name="CapabilityMask" value="0xFFF" wsmap="managed"/>
  </enum>
  <interface name="IMediumFormat" extends="$unknown" uuid="11be93c7-a862-4dc9-8c89-bf4ba74a886a" wsmap="managed" rest="managed" reservedMethods="2" reservedAttributes="4" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="id" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="name" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="capabilities" type="MediumFormatCapabilities" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <method name="describeFileExtensions" const="yes" internal="no" wsmap="managed">
      <rest name="description" request="get" path="/mediumformat/{formatid}/"/>
      <param name="extensions" type="wstring" safearray="yes" dir="out">
      </param>
      <param name="types" type="DeviceType" safearray="yes" dir="out">
      </param>
    </method>
    <method name="describeProperties" const="yes" internal="no" wsmap="managed">
      <rest name="description" request="get" path="/mediumformat/{formatid}/property/"/>
      <param name="names" type="wstring" safearray="yes" dir="out">
      </param>
      <param name="descriptions" type="wstring" safearray="yes" dir="out">
      </param>
      <param name="types" type="DataType" safearray="yes" dir="out">
      </param>
      <param name="flags" type="unsigned long" safearray="yes" dir="out">
      </param>
      <param name="defaults" type="wstring" safearray="yes" dir="out">
      </param>
    </method>
  </interface>
  <interface name="IDataStream" extends="$unknown" uuid="a338ed20-58d9-43ae-8b03-c1fd7088ef15" wsmap="managed" reservedMethods="4" reservedAttributes="8" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="readSize" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <method name="read" const="no" internal="no" wsmap="managed">
      <param name="size" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="timeoutMS" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="data" type="octet" dir="return" safearray="yes">
      </param>
    </method>
  </interface>
  <enum name="PartitionTableType" uuid="360066eb-d19e-4fa1-57ef-fed434fbe2a9">
    <const name="MBR" value="1" wsmap="managed"/>
    <const name="GPT" value="2" wsmap="managed"/>
  </enum>
  <interface name="IMediumIO" extends="$unknown" uuid="e4b301a9-5f86-4d65-ad1b-87ca284fb1c8" wsmap="managed" reservedMethods="4" reservedAttributes="8" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="medium" type="IMedium" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="writable" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="explorer" type="IVFSExplorer" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <method name="read" const="no" internal="no" wsmap="managed">
      <param name="offset" type="long long" dir="in" safearray="no">
      </param>
      <param name="size" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="data" type="octet" dir="return" safearray="yes">
      </param>
    </method>
    <method name="write" const="no" internal="no" wsmap="managed">
      <param name="offset" type="long long" dir="in" safearray="no">
      </param>
      <param name="data" type="octet" dir="in" safearray="yes">
      </param>
      <param name="written" type="unsigned long" dir="return" safearray="no">
      </param>
    </method>
    <method name="formatFAT" const="no" internal="no" wsmap="managed">
      <param name="quick" type="boolean" dir="in" safearray="no">
      </param>
    </method>
    <method name="initializePartitionTable" const="no" internal="no" wsmap="managed">
      <param name="format" type="PartitionTableType" dir="in" safearray="no">
      </param>
      <param name="wholeDiskInOneEntry" type="boolean" dir="in" safearray="no">
      </param>
    </method>
    <method name="convertToStream" const="no" internal="no" wsmap="managed">
      <param name="format" type="wstring" dir="in" safearray="no">
      </param>
      <param name="variant" type="MediumVariant" safearray="yes" dir="in">
      </param>
      <param name="bufferSize" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="stream" type="IDataStream" dir="out" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="close" const="no" internal="no" wsmap="managed">
    </method>
  </interface>
  <interface name="IToken" extends="$unknown" uuid="20479eaf-d8ed-44cf-85ac-c83a26c95a4d" wsmap="managed" rest="managed" reservedMethods="1" reservedAttributes="2" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <method name="abandon" wrap-hint-server="passcaller" const="no" internal="no" wsmap="managed">
    </method>
    <method name="dummy" const="no" internal="no" wsmap="managed">
    </method>
  </interface>
  <enum name="KeyboardLED" uuid="ef29ea38-409b-49c7-a817-c858d426dfba">
    <const name="NumLock" value="0x01" wsmap="managed"/>
    <const name="CapsLock" value="0x02" wsmap="managed"/>
    <const name="ScrollLock" value="0x04" wsmap="managed"/>
  </enum>
  <interface name="IKeyboard" extends="$unknown" uuid="755e6bdf-1640-41f9-bd74-3ef5fd653250" wsmap="managed" rest="managed" reservedMethods="4" reservedAttributes="4" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="keyboardLEDs" type="KeyboardLED" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <method name="putScancode" const="no" internal="no" wsmap="managed">
      <param name="scancode" type="long" dir="in" safearray="no"/>
    </method>
    <method name="putScancodes" const="no" internal="no" wsmap="managed">
      <param name="scancodes" type="long" dir="in" safearray="yes"/>
      <param name="codesStored" type="unsigned long" dir="return" safearray="no"/>
    </method>
    <method name="putCAD" const="no" internal="no" wsmap="managed">
    </method>
    <method name="releaseKeys" const="no" internal="no" wsmap="managed">
    </method>
    <method name="putUsageCode" const="no" internal="no" wsmap="managed">
      <param name="usageCode" type="long" dir="in" safearray="no"/>
      <param name="usagePage" type="long" dir="in" safearray="no"/>
      <param name="keyRelease" type="boolean" dir="in" safearray="no"/>
    </method>
    <attribute name="eventSource" type="IEventSource" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <enum name="MouseButtonState" uuid="9ee094b8-b28a-4d56-a166-973cb588d7f8">
    <const name="LeftButton" value="0x01" wsmap="managed"/>
    <const name="RightButton" value="0x02" wsmap="managed"/>
    <const name="MiddleButton" value="0x04" wsmap="managed"/>
    <const name="WheelUp" value="0x08" wsmap="managed"/>
    <const name="WheelDown" value="0x10" wsmap="managed"/>
    <const name="XButton1" value="0x20" wsmap="managed"/>
    <const name="XButton2" value="0x40" wsmap="managed"/>
    <const name="MouseStateMask" value="0x7F" wsmap="managed"/>
  </enum>
  <enum name="TouchContactState" uuid="3f942686-2506-421c-927c-90d4b45f4a38">
    <const name="None" value="0x00" wsmap="managed">
    </const>
    <const name="InContact" value="0x01" wsmap="managed">
    </const>
    <const name="InRange" value="0x02" wsmap="managed">
    </const>
    <const name="ContactStateMask" value="0x03" wsmap="managed"/>
  </enum>
  <interface name="IMousePointerShape" extends="$unknown" uuid="1e775ea3-9070-4f9c-b0d5-53054496dbe0" wsmap="managed" rest="managed" reservedAttributes="4" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="visible" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="alpha" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="hotX" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="hotY" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="width" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="height" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="shape" type="octet" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IMouse" extends="$unknown" uuid="25360a74-55e5-4f14-ac2a-f5cf8e62e4af" wsmap="managed" rest="managed" reservedMethods="4" reservedAttributes="4" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="absoluteSupported" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="relativeSupported" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="touchScreenSupported" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="touchPadSupported" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="needsHostCursor" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="pointerShape" type="IMousePointerShape" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <method name="putMouseEvent" const="no" internal="no" wsmap="managed">
      <param name="dx" type="long" dir="in" safearray="no">
      </param>
      <param name="dy" type="long" dir="in" safearray="no">
      </param>
      <param name="dz" type="long" dir="in" safearray="no">
      </param>
      <param name="dw" type="long" dir="in" safearray="no">
      </param>
      <param name="buttonState" type="long" dir="in" safearray="no">
      </param>
    </method>
    <method name="putMouseEventAbsolute" const="no" internal="no" wsmap="managed">
      <param name="x" type="long" dir="in" safearray="no">
      </param>
      <param name="y" type="long" dir="in" safearray="no">
      </param>
      <param name="dz" type="long" dir="in" safearray="no">
      </param>
      <param name="dw" type="long" dir="in" safearray="no">
      </param>
      <param name="buttonState" type="long" dir="in" safearray="no">
      </param>
    </method>
    <method name="putEventMultiTouch" const="no" internal="no" wsmap="managed">
      <param name="count" type="long" dir="in" safearray="no">
      </param>
      <param name="contacts" type="long long" dir="in" safearray="yes">
      </param>
      <param name="isTouchScreen" type="boolean" dir="in" safearray="no">
      </param>
      <param name="scanTime" type="unsigned long" dir="in" safearray="no">
      </param>
    </method>
    <method name="putEventMultiTouchString" const="no" internal="no" wsmap="managed">
      <param name="count" type="long" dir="in" safearray="no">
      </param>
      <param name="contacts" type="wstring" dir="in" safearray="no">
      </param>
      <param name="isTouchScreen" type="boolean" dir="in" safearray="no">
      </param>
      <param name="scanTime" type="unsigned long" dir="in" safearray="no">
      </param>
    </method>
    <attribute name="eventSource" type="IEventSource" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IDisplaySourceBitmap" extends="$unknown" uuid="5094f67a-8084-11e9-b185-dbe296e54799" wsmap="suppress" rest="managed" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="screenId" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
    <method name="queryBitmapInfo" const="no" internal="no" wsmap="managed">
      <param name="address" type="octet" mod="ptr" dir="out" safearray="no"/>
      <param name="width" type="unsigned long" dir="out" safearray="no"/>
      <param name="height" type="unsigned long" dir="out" safearray="no"/>
      <param name="bitsPerPixel" type="unsigned long" dir="out" safearray="no"/>
      <param name="bytesPerLine" type="unsigned long" dir="out" safearray="no"/>
      <param name="bitmapFormat" type="BitmapFormat" dir="out" safearray="no"/>
    </method>
  </interface>
  <enum name="FramebufferCapabilities" uuid="cc395839-30fa-4ca5-ae65-e6360e3edd7a">
    <const name="UpdateImage" value="0x01" wsmap="managed">
    </const>
    <const name="VHWA" value="0x02" wsmap="managed">
    </const>
    <const name="VisibleRegion" value="0x04" wsmap="managed">
    </const>
    <const name="RenderCursor" value="0x08" wsmap="managed">
    </const>
    <const name="MoveCursor" value="0x10" wsmap="managed">
    </const>
  </enum>
  <interface name="IFramebuffer" extends="$unknown" uuid="1e8d3f27-b45c-48ae-8b36-d35e83d207aa" wsmap="managed" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="width" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="height" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="bitsPerPixel" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="bytesPerLine" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="pixelFormat" type="BitmapFormat" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="heightReduction" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="overlay" type="IFramebufferOverlay" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="winId" type="long long" readonly="yes" wsmap="suppress" internal="no" safearray="no" rest="default">
    </attribute>
    <attribute name="capabilities" type="FramebufferCapabilities" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <method name="notifyUpdate" const="no" internal="no" wsmap="managed">
      <param name="x" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="y" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="width" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="height" type="unsigned long" dir="in" safearray="no">
      </param>
    </method>
    <method name="notifyUpdateImage" const="no" internal="no" wsmap="managed">
      <param name="x" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="y" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="width" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="height" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="image" type="octet" dir="in" safearray="yes">
      </param>
    </method>
    <method name="notifyChange" const="no" internal="no" wsmap="managed">
      <param name="screenId" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="xOrigin" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="yOrigin" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="width" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="height" type="unsigned long" dir="in" safearray="no">
      </param>
    </method>
    <method name="videoModeSupported" const="no" internal="no" wsmap="managed">
      <param name="width" type="unsigned long" dir="in" safearray="no"/>
      <param name="height" type="unsigned long" dir="in" safearray="no"/>
      <param name="bpp" type="unsigned long" dir="in" safearray="no"/>
      <param name="supported" type="boolean" dir="return" safearray="no"/>
    </method>
    <method name="getVisibleRegion" wsmap="suppress" const="no" internal="no">
      <param name="rectangles" type="octet" mod="ptr" dir="in" safearray="no">
      </param>
      <param name="count" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="countCopied" type="unsigned long" dir="return" safearray="no">
      </param>
    </method>
    <method name="setVisibleRegion" wsmap="suppress" const="no" internal="no">
      <param name="rectangles" type="octet" mod="ptr" dir="in" safearray="no">
      </param>
      <param name="count" type="unsigned long" dir="in" safearray="no">
      </param>
    </method>
    <method name="processVHWACommand" wsmap="suppress" const="no" internal="no">
      <param name="command" type="octet" mod="ptr" dir="in" safearray="no">
      </param>
      <param name="enmCmd" type="long" dir="in" safearray="no">
      </param>
      <param name="fromGuest" type="boolean" dir="in" safearray="no">
      </param>
    </method>
    <method name="notify3DEvent" const="no" internal="no" wsmap="managed">
      <param name="type" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="data" type="octet" dir="in" safearray="yes">
      </param>
    </method>
  </interface>
  <interface name="IFramebufferOverlay" extends="IFramebuffer" uuid="af398a9a-6b76-4805-8fab-00a9dcf4732b" wsmap="managed" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="x" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="y" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="visible" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="alpha" type="unsigned long" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <method name="move" const="no" internal="no" wsmap="managed">
      <param name="x" type="unsigned long" dir="in" safearray="no"/>
      <param name="y" type="unsigned long" dir="in" safearray="no"/>
    </method>
  </interface>
  <enum name="GuestMonitorStatus" uuid="6b8d3f71-39cb-459e-a916-48917ed43e19">
    <const name="Disabled" value="0" wsmap="managed">
    </const>
    <const name="Enabled" value="1" wsmap="managed">
    </const>
    <const name="Blank" value="2" wsmap="managed">
    </const>
  </enum>
  <enum name="ScreenLayoutMode" uuid="8fa1964c-8774-11e9-ae5d-1f419105e68d">
    <const name="Apply" value="0" wsmap="managed">
    </const>
    <const name="Reset" value="1" wsmap="managed">
    </const>
    <const name="Attach" value="2" wsmap="managed">
    </const>
    <const name="Silent" value="3" wsmap="managed">
    </const>
  </enum>
  <interface name="IGuestScreenInfo" extends="$unknown" uuid="6b2f98f8-9641-4397-854a-040439d0114b" wsmap="managed" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="screenId" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
    <attribute name="guestMonitorStatus" type="GuestMonitorStatus" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
    <attribute name="primary" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
    <attribute name="origin" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
    <attribute name="originX" type="long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
    <attribute name="originY" type="long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
    <attribute name="width" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
    <attribute name="height" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
    <attribute name="bitsPerPixel" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
    <attribute name="extendedInfo" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
  </interface>
  <interface name="IDisplay" extends="$unknown" uuid="14fd6676-ee6b-441a-988b-c83025ab693a" wsmap="managed" wrap-hint-server-addinterfaces="IEventListener" reservedMethods="8" reservedAttributes="16" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="guestScreenLayout" type="IGuestScreenInfo" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <method name="getScreenResolution" const="no" internal="no" wsmap="managed">
      <param name="screenId" type="unsigned long" dir="in" safearray="no"/>
      <param name="width" type="unsigned long" dir="out" safearray="no"/>
      <param name="height" type="unsigned long" dir="out" safearray="no"/>
      <param name="bitsPerPixel" type="unsigned long" dir="out" safearray="no"/>
      <param name="xOrigin" type="long" dir="out" safearray="no"/>
      <param name="yOrigin" type="long" dir="out" safearray="no"/>
      <param name="guestMonitorStatus" type="GuestMonitorStatus" dir="out" safearray="no"/>
    </method>
    <method name="attachFramebuffer" const="no" internal="no" wsmap="managed">
      <param name="screenId" type="unsigned long" dir="in" safearray="no"/>
      <param name="framebuffer" type="IFramebuffer" dir="in" safearray="no"/>
      <param name="id" type="uuid" mod="string" dir="return" safearray="no"/>
    </method>
    <method name="detachFramebuffer" const="no" internal="no" wsmap="managed">
      <param name="screenId" type="unsigned long" dir="in" safearray="no"/>
      <param name="id" type="uuid" mod="string" dir="in" safearray="no"/>
    </method>
    <method name="queryFramebuffer" const="no" internal="no" wsmap="managed">
      <param name="screenId" type="unsigned long" dir="in" safearray="no"/>
      <param name="framebuffer" type="IFramebuffer" dir="return" safearray="no"/>
    </method>
    <method name="setVideoModeHint" const="no" internal="no" wsmap="managed">
      <param name="display" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="enabled" type="boolean" dir="in" safearray="no">
      </param>
      <param name="changeOrigin" type="boolean" dir="in" safearray="no">
      </param>
      <param name="originX" type="long" dir="in" safearray="no">
      </param>
      <param name="originY" type="long" dir="in" safearray="no">
      </param>
      <param name="width" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="height" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="bitsPerPixel" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="notify" type="boolean" dir="in" safearray="no">
      </param>
    </method>
    <method name="getVideoModeHint" const="no" internal="no" wsmap="managed">
      <param name="display" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="enabled" type="boolean" dir="out" safearray="no">
      </param>
      <param name="changeOrigin" type="boolean" dir="out" safearray="no">
      </param>
      <param name="originX" type="long" dir="out" safearray="no">
      </param>
      <param name="originY" type="long" dir="out" safearray="no">
      </param>
      <param name="width" type="unsigned long" dir="out" safearray="no">
      </param>
      <param name="height" type="unsigned long" dir="out" safearray="no">
      </param>
      <param name="bitsPerPixel" type="unsigned long" dir="out" safearray="no">
      </param>
    </method>
    <method name="setSeamlessMode" const="no" internal="no" wsmap="managed">
      <param name="enabled" type="boolean" dir="in" safearray="no"/>
    </method>
    <method name="takeScreenShot" const="no" internal="no" wsmap="managed">
      <param name="screenId" type="unsigned long" dir="in" safearray="no"/>
      <param name="address" type="octet" mod="ptr" dir="in" safearray="no"/>
      <param name="width" type="unsigned long" dir="in" safearray="no"/>
      <param name="height" type="unsigned long" dir="in" safearray="no"/>
      <param name="bitmapFormat" type="BitmapFormat" dir="in" safearray="no"/>
    </method>
    <method name="takeScreenShotToArray" const="no" internal="no" wsmap="managed">
      <param name="screenId" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="width" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="height" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="bitmapFormat" type="BitmapFormat" dir="in" safearray="no">
      </param>
      <param name="screenData" type="octet" dir="return" safearray="yes">
      </param>
    </method>
    <method name="drawToScreen" const="no" internal="no" wsmap="managed">
      <param name="screenId" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="address" type="octet" mod="ptr" dir="in" safearray="no">
      </param>
      <param name="x" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="y" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="width" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="height" type="unsigned long" dir="in" safearray="no">
      </param>
    </method>
    <method name="invalidateAndUpdate" const="no" internal="no" wsmap="managed">
    </method>
    <method name="invalidateAndUpdateScreen" const="no" internal="no" wsmap="managed">
      <param name="screenId" type="unsigned long" dir="in" safearray="no">
      </param>
    </method>
    <method name="viewportChanged" const="no" internal="no" wsmap="managed">
      <param name="screenId" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="x" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="y" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="width" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="height" type="unsigned long" dir="in" safearray="no">
      </param>
    </method>
    <method name="querySourceBitmap" wsmap="suppress" const="no" internal="no">
      <param name="screenId" type="unsigned long" dir="in" safearray="no"/>
      <param name="displaySourceBitmap" type="IDisplaySourceBitmap" dir="out" safearray="no"/>
    </method>
    <method name="notifyScaleFactorChange" const="no" internal="no" wsmap="managed">
      <param name="screenId" type="unsigned long" dir="in" safearray="no"/>
      <param name="u32ScaleFactorWMultiplied" type="unsigned long" dir="in" safearray="no"/>
      <param name="u32ScaleFactorHMultiplied" type="unsigned long" dir="in" safearray="no"/>
    </method>
    <method name="notifyHiDPIOutputPolicyChange" const="no" internal="no" wsmap="managed">
      <param name="fUnscaledHiDPI" type="boolean" dir="in" safearray="no"/>
    </method>
    <method name="setScreenLayout" const="no" internal="no" wsmap="managed">
      <param name="screenLayoutMode" type="ScreenLayoutMode" dir="in" safearray="no"/>
      <param name="guestScreenInfo" type="IGuestScreenInfo" safearray="yes" dir="in"/>
    </method>
    <method name="detachScreens" const="no" internal="no" wsmap="managed">
      <param name="screenIds" type="long" safearray="yes" dir="in"/>
    </method>
    <method name="createGuestScreenInfo" const="no" internal="no" wsmap="managed">
      <param name="display" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="status" type="GuestMonitorStatus" dir="in" safearray="no">
      </param>
      <param name="primary" type="boolean" dir="in" safearray="no">
      </param>
      <param name="changeOrigin" type="boolean" dir="in" safearray="no">
      </param>
      <param name="originX" type="long" dir="in" safearray="no">
      </param>
      <param name="originY" type="long" dir="in" safearray="no">
      </param>
      <param name="width" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="height" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="bitsPerPixel" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="guestScreenInfo" type="IGuestScreenInfo" dir="return" safearray="no">
      </param>
    </method>
  </interface>
  <enum name="NetworkAttachmentType" uuid="cbfd17fa-fdc3-4d1c-97ec-4674da8d2a5d">
    <const name="Null" value="0" wsmap="managed">
    </const>
    <const name="NAT" value="1" wsmap="managed"/>
    <const name="Bridged" value="2" wsmap="managed"/>
    <const name="Internal" value="3" wsmap="managed"/>
    <const name="HostOnly" value="4" wsmap="managed"/>
    <const name="Generic" value="5" wsmap="managed"/>
    <const name="NATNetwork" value="6" wsmap="managed"/>
    <const name="Cloud" value="7" wsmap="managed"/>
    <const name="HostOnlyNetwork" value="8" wsmap="managed"/>
  </enum>
  <enum name="NetworkAdapterType" uuid="3c2281e4-d952-4e87-8c7d-24379cb6a81c">
    <const name="Null" value="0" wsmap="managed">
    </const>
    <const name="Am79C970A" value="1" wsmap="managed">
    </const>
    <const name="Am79C973" value="2" wsmap="managed">
    </const>
    <const name="I82540EM" value="3" wsmap="managed">
    </const>
    <const name="I82543GC" value="4" wsmap="managed">
    </const>
    <const name="I82545EM" value="5" wsmap="managed">
    </const>
    <const name="Virtio" value="6" wsmap="managed">
    </const>
    <const name="Am79C960" value="7" wsmap="managed">
    </const>
    <const name="NE2000" value="8" wsmap="managed">
    </const>
    <const name="NE1000" value="9" wsmap="managed">
    </const>
    <const name="WD8013" value="10" wsmap="managed">
    </const>
    <const name="WD8003" value="11" wsmap="managed">
    </const>
    <const name="ELNK2" value="12" wsmap="managed">
    </const>
    <const name="ELNK1" value="13" wsmap="managed">
    </const>
    <const name="UsbNet" value="14" wsmap="managed">
    </const>
    <const name="I82583V" value="15" wsmap="managed">
    </const>
  </enum>
  <enum name="NetworkAdapterPromiscModePolicy" uuid="c963768a-376f-4c85-8d84-d8ced4b7269e">
    <const name="Deny" value="1" wsmap="managed">
    </const>
    <const name="AllowNetwork" value="2" wsmap="managed">
    </const>
    <const name="AllowAll" value="3" wsmap="managed">
    </const>
  </enum>
  <interface name="INetworkAdapter" extends="$unknown" uuid="dcf47a1d-ed70-4db8-9a4b-2646bd166905" wsmap="managed" rest="managed" reservedMethods="4" reservedAttributes="8" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="adapterType" type="NetworkAdapterType" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="slot" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="enabled" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="MACAddress" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="attachmentType" type="NetworkAttachmentType" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="bridgedInterface" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="hostOnlyInterface" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="hostOnlyNetwork" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="internalNetwork" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="NATNetwork" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="genericDriver" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="cloudNetwork" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="cableConnected" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="lineSpeed" type="unsigned long" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="promiscModePolicy" type="NetworkAdapterPromiscModePolicy" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="traceEnabled" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="traceFile" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="NATEngine" type="INATEngine" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="bootPriority" type="unsigned long" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="bandwidthGroup" type="IBandwidthGroup" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <method name="getProperty" const="yes" internal="no" wsmap="managed">
      <param name="key" type="wstring" dir="in" safearray="no">
      </param>
      <param name="value" type="wstring" dir="return" safearray="no">
      </param>
    </method>
    <method name="setProperty" const="no" internal="no" wsmap="managed">
      <param name="key" type="wstring" dir="in" safearray="no">
      </param>
      <param name="value" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <method name="getProperties" const="yes" internal="no" wsmap="managed">
      <param name="names" type="wstring" dir="in" safearray="no">
      </param>
      <param name="returnNames" type="wstring" safearray="yes" dir="out">
      </param>
      <param name="returnValues" type="wstring" safearray="yes" dir="return">
      </param>
    </method>
  </interface>
  <enum name="PortMode" uuid="7485fcfd-d603-470a-87af-26d33beb7de9">
    <const name="Disconnected" value="0" wsmap="managed">
    </const>
    <const name="HostPipe" value="1" wsmap="managed">
    </const>
    <const name="HostDevice" value="2" wsmap="managed">
    </const>
    <const name="RawFile" value="3" wsmap="managed">
    </const>
    <const name="TCP" value="4" wsmap="managed">
    </const>
  </enum>
  <enum name="UartType" uuid="c8899d39-0b90-4265-9d02-1e38bd4d1b39">
    <const name="U16450" value="0" wsmap="managed">
    </const>
    <const name="U16550A" value="1" wsmap="managed">
    </const>
    <const name="U16750" value="2" wsmap="managed">
    </const>
  </enum>
  <interface name="ISerialPort" extends="$unknown" uuid="5587d0f6-a227-4f23-8278-2f675eea1bb2" wsmap="managed" rest="managed" reservedAttributes="4" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="slot" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="enabled" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="IOAddress" type="unsigned long" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="IRQ" type="unsigned long" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="hostMode" type="PortMode" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="server" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="path" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="uartType" type="UartType" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IParallelPort" extends="$unknown" uuid="788b87df-7708-444b-9eef-c116ce423d39" wsmap="managed" rest="managed" reservedAttributes="4" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="slot" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="enabled" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="IOBase" type="unsigned long" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="IRQ" type="unsigned long" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="path" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IMachineDebugger" extends="$unknown" uuid="fa43579a-2272-47c4-a443-9713f19a902f" wsmap="managed" rest="managed" reservedMethods="16" reservedAttributes="14" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <method name="dumpGuestCore" const="no" internal="no" wsmap="managed">
      <param name="filename" type="wstring" dir="in" safearray="no">
      </param>
      <param name="compression" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <method name="dumpHostProcessCore" const="no" internal="no" wsmap="managed">
      <param name="filename" type="wstring" dir="in" safearray="no">
      </param>
      <param name="compression" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <method name="info" const="no" internal="no" wsmap="managed">
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="args" type="wstring" dir="in" safearray="no">
      </param>
      <param name="info" type="wstring" dir="return" safearray="no">
      </param>
    </method>
    <method name="injectNMI" const="no" internal="no" wsmap="managed">
    </method>
    <method name="modifyLogGroups" const="no" internal="no" wsmap="managed">
      <param name="settings" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <method name="modifyLogFlags" const="no" internal="no" wsmap="managed">
      <param name="settings" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <method name="modifyLogDestinations" const="no" internal="no" wsmap="managed">
      <param name="settings" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <method name="readPhysicalMemory" const="no" internal="no" wsmap="managed">
      <param name="address" type="long long" dir="in" safearray="no">
      </param>
      <param name="size" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="bytes" type="octet" safearray="yes" dir="return">
      </param>
    </method>
    <method name="writePhysicalMemory" const="no" internal="no" wsmap="managed">
      <param name="address" type="long long" dir="in" safearray="no">
      </param>
      <param name="size" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="bytes" type="octet" safearray="yes" dir="in">
      </param>
    </method>
    <method name="readVirtualMemory" const="no" internal="no" wsmap="managed">
      <param name="cpuId" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="address" type="long long" dir="in" safearray="no">
      </param>
      <param name="size" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="bytes" type="octet" safearray="yes" dir="return">
      </param>
    </method>
    <method name="writeVirtualMemory" const="no" internal="no" wsmap="managed">
      <param name="cpuId" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="address" type="long long" dir="in" safearray="no">
      </param>
      <param name="size" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="bytes" type="octet" safearray="yes" dir="in">
      </param>
    </method>
    <method name="loadPlugIn" const="no" internal="no" wsmap="managed">
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="plugInName" type="wstring" dir="return" safearray="no">
      </param>
    </method>
    <method name="unloadPlugIn" const="no" internal="no" wsmap="managed">
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <method name="detectOS" const="no" internal="no" wsmap="managed">
      <param name="os" type="wstring" dir="return" safearray="no">
      </param>
    </method>
    <method name="queryOSKernelLog" const="no" internal="no" wsmap="managed">
      <param name="maxMessages" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="dmesg" type="wstring" dir="return" safearray="no">
      </param>
    </method>
    <method name="getRegister" const="no" internal="no" wsmap="managed">
      <param name="cpuId" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="value" type="wstring" dir="return" safearray="no">
      </param>
    </method>
    <method name="getRegisters" const="no" internal="no" wsmap="managed">
      <param name="cpuId" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="names" type="wstring" dir="out" safearray="yes">
      </param>
      <param name="values" type="wstring" dir="out" safearray="yes">
      </param>
    </method>
    <method name="setRegister" const="no" internal="no" wsmap="managed">
      <param name="cpuId" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="value" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <method name="setRegisters" const="no" internal="no" wsmap="managed">
      <param name="cpuId" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="names" type="wstring" dir="in" safearray="yes">
      </param>
      <param name="values" type="wstring" dir="in" safearray="yes">
      </param>
    </method>
    <method name="dumpGuestStack" const="no" internal="no" wsmap="managed">
      <param name="cpuId" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="stack" type="wstring" dir="return" safearray="no">
      </param>
    </method>
    <method name="resetStats" const="no" internal="no" wsmap="managed">
      <param name="pattern" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <method name="dumpStats" const="no" internal="no" wsmap="managed">
      <param name="pattern" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <method name="getStats" const="no" internal="no" wsmap="managed">
      <param name="pattern" type="wstring" dir="in" safearray="no">
      </param>
      <param name="withDescriptions" type="boolean" dir="in" safearray="no">
      </param>
      <param name="stats" type="wstring" dir="return" safearray="no">
      </param>
    </method>
    <method name="getCPULoad" const="no" internal="no" wsmap="managed">
      <param name="cpuId" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="pctExecuting" type="unsigned long" dir="out" safearray="no">
      </param>
      <param name="pctHalted" type="unsigned long" dir="out" safearray="no">
      </param>
      <param name="pctOther" type="unsigned long" dir="out" safearray="no">
      </param>
      <param name="msInterval" type="long long" dir="return" safearray="no">
      </param>
    </method>
    <method name="takeGuestSample" const="no" internal="no" wsmap="managed">
      <param name="filename" type="wstring" dir="in" safearray="no">
      </param>
      <param name="usInterval" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="usSampleTime" type="long long" dir="in" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="getUVMAndVMMFunctionTable" wsmap="suppress" const="no" internal="no">
      <param name="magicVersion" type="long long" dir="in" safearray="no">
      </param>
      <param name="VMMFunctionTable" type="long long" dir="out" safearray="no">
      </param>
      <param name="UVM" type="long long" dir="return" safearray="no">
      </param>
    </method>
    <attribute name="singleStep" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="executeAllInIEM" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="logEnabled" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="logDbgFlags" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="logDbgGroups" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="logDbgDestinations" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="logRelFlags" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="logRelGroups" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="logRelDestinations" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="executionEngine" type="VMExecutionEngine" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="HWVirtExNestedPagingEnabled" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="HWVirtExVPIDEnabled" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="HWVirtExUXEnabled" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="OSName" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="OSVersion" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="PAEEnabled" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="virtualTimeRate" type="unsigned long" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="uptime" type="long long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="recompiledIEMExecution" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IUSBDeviceFilters" extends="$unknown" uuid="9709db9b-3346-49d6-8f1c-41b0c4784ff2" wsmap="managed" rest="managed" reservedMethods="2" reservedAttributes="2" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="deviceFilters" type="IUSBDeviceFilter" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <method name="createDeviceFilter" const="no" internal="no" wsmap="managed">
      <rest name="create" request="post" path="/vm/{vmid}/usbdevicefilter/action/"/>
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="filter" type="IUSBDeviceFilter" dir="return" safearray="no">
      </param>
    </method>
    <method name="insertDeviceFilter" const="no" internal="no" wsmap="managed">
      <rest name="insert" request="patch" path="/vm/{vmid}/usbdevicefilter/{filterid}/action/"/>
      <param name="position" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="filter" type="IUSBDeviceFilter" dir="in" safearray="no">
      </param>
    </method>
    <method name="removeDeviceFilter" const="no" internal="no" wsmap="managed">
      <rest name="remove" request="delete" path="/vm/{vmid}/usbdevicefilter/{filterid}/action/"/>
      <param name="position" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="filter" type="IUSBDeviceFilter" dir="return" safearray="no">
      </param>
    </method>
  </interface>
  <enum name="USBControllerType" uuid="8fdd1c6a-5412-41da-ab07-7baed7d6e18e">
    <const name="Null" value="0" wsmap="managed">
    </const>
    <const name="OHCI" value="1" wsmap="managed"/>
    <const name="EHCI" value="2" wsmap="managed"/>
    <const name="XHCI" value="3" wsmap="managed"/>
    <const name="Last" value="4" wsmap="managed">
    </const>
  </enum>
  <interface name="IUSBController" extends="$unknown" uuid="ee206a6e-7ff8-4a84-bd34-0c651e118bb5" wsmap="managed" rest="managed" reservedAttributes="4" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="name" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="type" type="USBControllerType" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="USBStandard" type="unsigned short" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <enum name="USBConnectionSpeed" uuid="d2915840-ea26-4fb4-b72a-21eaf6b888ff">
    <const name="Null" value="0" wsmap="managed">
    </const>
    <const name="Low" value="1" wsmap="managed">
    </const>
    <const name="Full" value="2" wsmap="managed">
    </const>
    <const name="High" value="3" wsmap="managed">
    </const>
    <const name="Super" value="4" wsmap="managed">
    </const>
    <const name="SuperPlus" value="5" wsmap="managed">
    </const>
  </enum>
  <interface name="IUSBDevice" extends="$unknown" uuid="6dc83c2c-81a9-4005-9d52-fc45a78bf3f5" wsmap="managed" rest="managed" reservedAttributes="4" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="id" type="uuid" mod="string" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="vendorId" type="unsigned short" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="productId" type="unsigned short" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="revision" type="unsigned short" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="manufacturer" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="product" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="serialNumber" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="address" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="port" type="unsigned short" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="portPath" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="version" type="unsigned short" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="speed" type="USBConnectionSpeed" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="remote" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="deviceInfo" type="wstring" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="backend" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IUSBDeviceFilter" extends="$unknown" uuid="45587218-4289-ef4e-8e6a-e5b07816b631" wsmap="managed" rest="managed" reservedAttributes="8" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="name" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="active" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="vendorId" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="productId" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="revision" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="manufacturer" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="product" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="serialNumber" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="port" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="remote" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="maskedInterfaces" type="unsigned long" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <enum name="USBDeviceState" uuid="b99a2e65-67fb-4882-82fd-f3e5e8193ab4">
    <const name="NotSupported" value="0" wsmap="managed">
    </const>
    <const name="Unavailable" value="1" wsmap="managed">
    </const>
    <const name="Busy" value="2" wsmap="managed">
    </const>
    <const name="Available" value="3" wsmap="managed">
    </const>
    <const name="Held" value="4" wsmap="managed">
    </const>
    <const name="Captured" value="5" wsmap="managed">
    </const>
  </enum>
  <interface name="IHostUSBDevice" extends="IUSBDevice" uuid="c19073dd-cc7b-431b-98b2-951fda8eab89" wsmap="managed" rest="managed" reservedAttributes="4" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="state" type="USBDeviceState" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <enum name="USBDeviceFilterAction" uuid="cbc30a49-2f4e-43b5-9da6-121320475933">
    <const name="Null" value="0" wsmap="managed">
    </const>
    <const name="Ignore" value="1" wsmap="managed">
    </const>
    <const name="Hold" value="2" wsmap="managed">
    </const>
  </enum>
  <interface name="IHostUSBDeviceFilter" extends="IUSBDeviceFilter" uuid="01adb2d6-aedf-461c-be2c-99e91bdad8a1" wsmap="managed" rest="managed" reservedAttributes="8" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="action" type="USBDeviceFilterAction" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IUSBProxyBackend" extends="$unknown" uuid="dfe56449-6989-4002-80cf-3607f377d40c" wsmap="managed" rest="managed" reservedMethods="4" reservedAttributes="8" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="name" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="type" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <enum name="AudioDriverType" uuid="32b4acfd-79ab-4b7e-9a1c-92e99f4e000b">
    <const name="Default" value="0" wsmap="managed">
    </const>
    <const name="Null" value="1" wsmap="managed">
    </const>
    <const name="OSS" value="2" wsmap="managed">
    </const>
    <const name="ALSA" value="3" wsmap="managed">
    </const>
    <const name="Pulse" value="4" wsmap="managed">
    </const>
    <const name="WinMM" value="5" wsmap="managed">
    </const>
    <const name="DirectSound" value="6" wsmap="managed">
    </const>
    <const name="WAS" value="7" wsmap="managed">
    </const>
    <const name="CoreAudio" value="8" wsmap="managed">
    </const>
    <const name="MMPM" value="9" wsmap="managed">
    </const>
    <const name="SolAudio" value="10" wsmap="managed">
    </const>
  </enum>
  <enum name="AudioControllerType" uuid="6389dcc9-4e60-4983-9418-299e602efafc">
    <const name="AC97" value="0" wsmap="managed"/>
    <const name="SB16" value="1" wsmap="managed"/>
    <const name="HDA" value="2" wsmap="managed"/>
    <const name="VirtioSound" value="3" wsmap="managed"/>
  </enum>
  <enum name="AudioCodecType" uuid="7b406301-f520-420c-9805-8ce11c086370">
    <const name="Null" value="0" wsmap="managed">
    </const>
    <const name="SB16" value="1" wsmap="managed">
    </const>
    <const name="STAC9700" value="2" wsmap="managed">
    </const>
    <const name="AD1980" value="3" wsmap="managed">
    </const>
    <const name="STAC9221" value="4" wsmap="managed">
    </const>
  </enum>
  <interface name="IAudioAdapter" extends="$unknown" uuid="5155bfd3-7ba7-45a8-b26d-c91ae3754e37" wsmap="managed" rest="managed" reservedMethods="4" reservedAttributes="8" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="enabled" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="enabledIn" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="enabledOut" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="audioController" type="AudioControllerType" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="audioCodec" type="AudioCodecType" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="audioDriver" type="AudioDriverType" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="propertiesList" type="wstring" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <method name="setProperty" const="no" internal="no" wsmap="managed">
      <param name="key" type="wstring" dir="in" safearray="no">
      </param>
      <param name="value" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <method name="getProperty" const="yes" internal="no" wsmap="managed">
      <param name="key" type="wstring" dir="in" safearray="no">
      </param>
      <param name="value" type="wstring" dir="return" safearray="no">
      </param>
    </method>
  </interface>
  <enum name="AudioDirection" uuid="f27d209b-040c-4ae9-beae-85f9693ca07a">
    <const name="Unknown" value="0" wsmap="managed">
    </const>
    <const name="In" value="1" wsmap="managed">
    </const>
    <const name="Out" value="2" wsmap="managed">
    </const>
    <const name="Duplex" value="3" wsmap="managed">
    </const>
  </enum>
  <enum name="AudioDeviceType" uuid="95457071-ef88-45a9-8416-fc05b08731d2">
    <const name="Unknown" value="0" wsmap="managed">
    </const>
    <const name="BuiltLin" value="1" wsmap="managed">
    </const>
    <const name="ExternalUSB" value="2" wsmap="managed">
    </const>
    <const name="ExternalOther" value="3" wsmap="managed">
    </const>
  </enum>
  <enum name="AudioDeviceState" uuid="45c1b969-556a-4277-8570-b438d2ef5ebe">
    <const name="Unknown" value="0" wsmap="managed">
    </const>
    <const name="Active" value="1" wsmap="managed">
    </const>
    <const name="Disabled" value="2" wsmap="managed">
    </const>
    <const name="NotPresent" value="3" wsmap="managed">
    </const>
    <const name="Unplugged" value="4" wsmap="managed">
    </const>
  </enum>
  <interface name="IHostAudioDevice" extends="$unknown" uuid="cfde1265-3140-4048-a81f-a1e280dfbd75" wsmap="managed" rest="managed" reservedMethods="4" reservedAttributes="8" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="id" type="uuid" mod="string" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="name" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="type" type="AudioDeviceType" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="usage" type="AudioDirection" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="defaultIn" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="defaultOut" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="isHotPlug" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="state" type="AudioDeviceState" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <method name="getProperty" const="yes" internal="no" wsmap="managed">
      <param name="key" type="wstring" dir="in" safearray="no">
      </param>
      <param name="value" type="wstring" dir="return" safearray="no">
      </param>
    </method>
  </interface>
  <interface name="IAudioSettings" extends="$unknown" uuid="52f40b16-520e-473f-9428-3e69b0d915c3" wsmap="managed" rest="managed" reservedMethods="4" reservedAttributes="8" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <method name="getHostAudioDevice" const="no" internal="no" wsmap="managed">
      <param name="usage" type="AudioDirection" dir="in" safearray="no">
      </param>
      <param name="device" type="IHostAudioDevice" dir="return" safearray="no">
      </param>
    </method>
    <method name="setHostAudioDevice" const="no" internal="no" wsmap="managed">
      <param name="device" type="IHostAudioDevice" dir="in" safearray="no">
      </param>
      <param name="usage" type="AudioDirection" dir="in" safearray="no">
      </param>
    </method>
    <attribute name="adapter" type="IAudioAdapter" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <enum name="AuthType" uuid="7eef6ef6-98c2-4dc2-ab35-10d2b292028d">
    <const name="Null" value="0" wsmap="managed">
    </const>
    <const name="External" value="1" wsmap="managed"/>
    <const name="Guest" value="2" wsmap="managed"/>
  </enum>
  <interface name="IVRDEServer" extends="$unknown" uuid="08e25756-08a2-41af-a05f-d7c661abaebe" wsmap="managed" rest="managed" reservedMethods="2" reservedAttributes="4" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="enabled" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="authType" type="AuthType" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="authTimeout" type="unsigned long" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="allowMultiConnection" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="reuseSingleConnection" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="VRDEExtPack" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="authLibrary" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="VRDEProperties" type="wstring" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <method name="setVRDEProperty" const="no" internal="no" wsmap="managed">
      <param name="key" type="wstring" dir="in" safearray="no">
      </param>
      <param name="value" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <method name="getVRDEProperty" const="yes" internal="no" wsmap="managed">
      <param name="key" type="wstring" dir="in" safearray="no">
      </param>
      <param name="value" type="wstring" dir="return" safearray="no">
      </param>
    </method>
  </interface>
  <interface name="ISharedFolder" extends="$unknown" uuid="0b108b8c-62e0-4e06-9dfa-2f1a2ad70774" wsmap="managed" rest="managed" reservedAttributes="8" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="name" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="hostPath" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="accessible" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="writable" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="autoMount" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="autoMountPoint" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="lastAccessError" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="symlinkPolicy" type="SymlinkPolicy" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <enum name="Reason" uuid="e7e8e097-299d-4e98-8bbc-c31c2d47d0cc">
    <const name="Unspecified" value="0" wsmap="managed">
    </const>
    <const name="HostSuspend" value="1" wsmap="managed">
    </const>
    <const name="HostResume" value="2" wsmap="managed">
    </const>
    <const name="HostBatteryLow" value="3" wsmap="managed">
    </const>
    <const name="Snapshot" value="4" wsmap="managed">
    </const>
  </enum>
  <interface name="IInternalSessionControl" extends="$unknown" uuid="84a4e585-4f4a-45bb-99e8-fc9b226376ad" internal="yes" wsmap="suppress" default="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="PID" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="remoteConsole" type="IConsole" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="nominalState" type="MachineState" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
<if target="midl">
    <method name="assignMachine" const="no" internal="no" wsmap="managed">
      <param name="machine" type="IMachine" dir="in" safearray="no"/>
      <param name="lockType" type="LockType" dir="in" safearray="no"/>
      <param name="tokenId" type="wstring" dir="in" safearray="no"/>
    </method>
</if>
<if target="xpidl">
    <method name="assignMachine" const="no" internal="no" wsmap="managed">
      <param name="machine" type="IMachine" dir="in" safearray="no"/>
      <param name="lockType" type="LockType" dir="in" safearray="no"/>
      <param name="token" type="IToken" dir="in" safearray="no"/>
    </method>
</if>
    <method name="assignRemoteMachine" const="no" internal="no" wsmap="managed">
      <param name="machine" type="IMachine" dir="in" safearray="no"/>
      <param name="console" type="IConsole" dir="in" safearray="no"/>
    </method>
    <method name="updateMachineState" const="no" internal="no" wsmap="managed">
      <param name="machineState" type="MachineState" dir="in" safearray="no"/>
    </method>
    <method name="uninitialize" const="no" internal="no" wsmap="managed">
    </method>
    <method name="onNetworkAdapterChange" const="no" internal="no" wsmap="managed">
      <param name="networkAdapter" type="INetworkAdapter" dir="in" safearray="no"/>
      <param name="changeAdapter" type="boolean" dir="in" safearray="no"/>
    </method>
    <method name="onAudioAdapterChange" const="no" internal="no" wsmap="managed">
      <param name="audioAdapter" type="IAudioAdapter" dir="in" safearray="no"/>
    </method>
    <method name="onHostAudioDeviceChange" const="no" internal="no" wsmap="managed">
      <param name="device" type="IHostAudioDevice" dir="in" safearray="no">
      </param>
      <param name="isNew" type="boolean" dir="in" safearray="no">
      </param>
      <param name="state" type="AudioDeviceState" dir="in" safearray="no">
      </param>
      <param name="errorInfo" type="IVirtualBoxErrorInfo" dir="in" safearray="no">
      </param>
    </method>
    <method name="onSerialPortChange" const="no" internal="no" wsmap="managed">
      <param name="serialPort" type="ISerialPort" dir="in" safearray="no"/>
    </method>
    <method name="onParallelPortChange" const="no" internal="no" wsmap="managed">
      <param name="parallelPort" type="IParallelPort" dir="in" safearray="no"/>
    </method>
    <method name="onStorageControllerChange" const="no" internal="no" wsmap="managed">
      <param name="machineId" type="uuid" mod="string" dir="in" safearray="no">
      </param>
      <param name="controllerName" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <method name="onMediumChange" const="no" internal="no" wsmap="managed">
      <param name="mediumAttachment" type="IMediumAttachment" dir="in" safearray="no">
      </param>
      <param name="force" type="boolean" dir="in" safearray="no">
      </param>
    </method>
    <method name="onStorageDeviceChange" const="no" internal="no" wsmap="managed">
      <param name="mediumAttachment" type="IMediumAttachment" dir="in" safearray="no">
      </param>
      <param name="remove" type="boolean" dir="in" safearray="no">
      </param>
      <param name="silent" type="boolean" dir="in" safearray="no">
      </param>
    </method>
    <method name="onVMProcessPriorityChange" const="no" internal="no" wsmap="managed">
      <param name="priority" type="VMProcPriority" dir="in" safearray="no">
      </param>
    </method>
    <method name="onClipboardModeChange" const="no" internal="no" wsmap="managed">
      <param name="clipboardMode" type="ClipboardMode" dir="in" safearray="no">
      </param>
    </method>
    <method name="onClipboardFileTransferModeChange" const="no" internal="no" wsmap="managed">
      <param name="enabled" type="boolean" dir="in" safearray="no">
      </param>
    </method>
    <method name="onDnDModeChange" const="no" internal="no" wsmap="managed">
      <param name="dndMode" type="DnDMode" dir="in" safearray="no">
      </param>
    </method>
    <method name="onCPUChange" const="no" internal="no" wsmap="managed">
      <param name="cpu" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="add" type="boolean" dir="in" safearray="no">
      </param>
    </method>
    <method name="onCPUExecutionCapChange" const="no" internal="no" wsmap="managed">
      <param name="executionCap" type="unsigned long" dir="in" safearray="no">
      </param>
    </method>
    <method name="onVRDEServerChange" const="no" internal="no" wsmap="managed">
      <param name="restart" type="boolean" dir="in" safearray="no">
      </param>
    </method>
    <method name="onRecordingStateChange" const="no" internal="no" wsmap="managed">
      <param name="state" type="RecordingState" dir="in" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="out" safearray="no">
      </param>
    </method>
    <method name="onRecordingScreenStateChange" const="no" internal="no" wsmap="managed">
      <param name="state" type="RecordingState" dir="in" safearray="no">
      </param>
      <param name="screen" type="unsigned long" dir="in" safearray="no">
      </param>
    </method>
    <method name="onUSBControllerChange" const="no" internal="no" wsmap="managed">
    </method>
    <method name="onSharedFolderChange" const="no" internal="no" wsmap="managed">
      <param name="global" type="boolean" dir="in" safearray="no"/>
    </method>
    <method name="onGuestDebugControlChange" const="no" internal="no" wsmap="managed">
      <param name="guestDebugControl" type="IGuestDebugControl" dir="in" safearray="no"/>
    </method>
    <method name="onUSBDeviceAttach" const="no" internal="no" wsmap="managed">
      <param name="device" type="IUSBDevice" dir="in" safearray="no"/>
      <param name="error" type="IVirtualBoxErrorInfo" dir="in" safearray="no"/>
      <param name="maskedInterfaces" type="unsigned long" dir="in" safearray="no"/>
      <param name="captureFilename" type="wstring" dir="in" safearray="no"/>
    </method>
    <method name="onUSBDeviceDetach" const="no" internal="no" wsmap="managed">
      <param name="id" type="uuid" mod="string" dir="in" safearray="no"/>
      <param name="error" type="IVirtualBoxErrorInfo" dir="in" safearray="no"/>
    </method>
    <method name="onShowWindow" const="no" internal="no" wsmap="managed">
      <param name="check" type="boolean" dir="in" safearray="no"/>
      <param name="canShow" type="boolean" dir="out" safearray="no"/>
      <param name="winId" type="long long" dir="out" safearray="no"/>
    </method>
    <method name="onBandwidthGroupChange" const="no" internal="no" wsmap="managed">
      <param name="bandwidthGroup" type="IBandwidthGroup" dir="in" safearray="no">
      </param>
    </method>
    <method name="accessGuestProperty" const="no" internal="no" wsmap="managed">
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="value" type="wstring" dir="in" safearray="no">
      </param>
      <param name="flags" type="wstring" dir="in" safearray="no">
      </param>
      <param name="accessMode" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="retValue" type="wstring" dir="out" safearray="no">
      </param>
      <param name="retTimestamp" type="long long" dir="out" safearray="no">
      </param>
      <param name="retFlags" type="wstring" dir="out" safearray="no">
      </param>
    </method>
    <method name="enumerateGuestProperties" const="yes" internal="no" wsmap="managed">
      <param name="patterns" type="wstring" dir="in" safearray="no">
      </param>
      <param name="keys" type="wstring" dir="out" safearray="yes">
      </param>
      <param name="values" type="wstring" dir="out" safearray="yes">
      </param>
      <param name="timestamps" type="long long" dir="out" safearray="yes">
      </param>
      <param name="flags" type="wstring" dir="out" safearray="yes">
      </param>
    </method>
    <method name="onlineMergeMedium" const="no" internal="no" wsmap="managed">
      <param name="mediumAttachment" type="IMediumAttachment" dir="in" safearray="no">
      </param>
      <param name="sourceIdx" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="targetIdx" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="in" safearray="no">
      </param>
    </method>
    <method name="reconfigureMediumAttachments" const="no" internal="no" wsmap="managed">
      <param name="attachments" type="IMediumAttachment" dir="in" safearray="yes">
      </param>
    </method>
    <method name="enableVMMStatistics" const="no" internal="no" wsmap="managed">
      <param name="enable" type="boolean" dir="in" safearray="no">
      </param>
    </method>
    <method name="pauseWithReason" const="no" internal="no" wsmap="managed">
      <param name="reason" type="Reason" dir="in" safearray="no">
      </param>
    </method>
    <method name="resumeWithReason" const="no" internal="no" wsmap="managed">
      <param name="reason" type="Reason" dir="in" safearray="no">
      </param>
    </method>
    <method name="saveStateWithReason" const="no" internal="no" wsmap="managed">
      <param name="reason" type="Reason" dir="in" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="in" safearray="no">
      </param>
      <param name="snapshot" type="ISnapshot" dir="in" safearray="no">
      </param>
      <param name="stateFilePath" type="wstring" dir="in" safearray="no">
      </param>
      <param name="pauseVM" type="boolean" dir="in" safearray="no">
      </param>
      <param name="leftPaused" type="boolean" dir="return" safearray="no">
      </param>
    </method>
    <method name="cancelSaveStateWithReason" const="no" internal="no" wsmap="managed">
    </method>
  </interface>
  <interface name="ISession" extends="$unknown" uuid="c0447716-ff5a-4795-b57a-ecd5fffa18a4" wsmap="managed" rest="managed" wrap-hint-server-addinterfaces="IInternalSessionControl" reservedMethods="4" reservedAttributes="8" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="state" type="SessionState" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="type" type="SessionType" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="name" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="machine" type="IMachine" readonly="yes" rest="uuid" internal="no" safearray="no" wsmap="managed">
    </attribute>
    <attribute name="console" type="IConsole" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <method name="unlockMachine" const="no" internal="no" wsmap="managed">
    </method>
  </interface>
  <enum name="StorageBus" uuid="f9510869-7d07-46ba-96a6-6728fbf4adee">
    <const name="Null" value="0" wsmap="managed">
    </const>
    <const name="IDE" value="1" wsmap="managed"/>
    <const name="SATA" value="2" wsmap="managed"/>
    <const name="SCSI" value="3" wsmap="managed"/>
    <const name="Floppy" value="4" wsmap="managed"/>
    <const name="SAS" value="5" wsmap="managed"/>
    <const name="USB" value="6" wsmap="managed"/>
    <const name="PCIe" value="7" wsmap="managed"/>
    <const name="VirtioSCSI" value="8" wsmap="managed"/>
  </enum>
  <enum name="StorageControllerType" uuid="a77d457d-66a3-4368-b24c-293d0f562a9f">
    <const name="Null" value="0" wsmap="managed">
    </const>
    <const name="LsiLogic" value="1" wsmap="managed">
    </const>
    <const name="BusLogic" value="2" wsmap="managed">
    </const>
    <const name="IntelAhci" value="3" wsmap="managed">
    </const>
    <const name="PIIX3" value="4" wsmap="managed">
    </const>
    <const name="PIIX4" value="5" wsmap="managed">
    </const>
    <const name="ICH6" value="6" wsmap="managed">
    </const>
    <const name="I82078" value="7" wsmap="managed">
    </const>
    <const name="LsiLogicSas" value="8" wsmap="managed">
    </const>
    <const name="USB" value="9" wsmap="managed">
    </const>
    <const name="NVMe" value="10" wsmap="managed">
    </const>
    <const name="VirtioSCSI" value="11" wsmap="managed">
    </const>
  </enum>
  <enum name="ChipsetType" uuid="733d5f16-1f47-4e25-b272-d7c138bc2e3d">
    <const name="Null" value="0" wsmap="managed">
    </const>
    <const name="PIIX3" value="1" wsmap="managed">
    </const>
    <const name="ICH9" value="2" wsmap="managed">
    </const>
    <const name="ARMv8Virtual" value="3" wsmap="managed">
    </const>
  </enum>
  <interface name="IStorageController" extends="$unknown" uuid="ddca7247-bf98-47fb-ab2f-b5177533f493" wsmap="managed" reservedMethods="4" reservedAttributes="8" rest="managed" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="name" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="maxDevicesPerPortCount" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="minPortCount" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="maxPortCount" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="instance" type="unsigned long" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="portCount" type="unsigned long" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="bus" type="StorageBus" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="controllerType" type="StorageControllerType" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="useHostIOCache" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="bootable" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
<if target="wsdl">
  <interface name="IManagedObjectRef" extends="$unknown" uuid="9474d09d-2313-46de-b568-a42b8718e8ed" internal="yes" wsmap="managed" wscpp="hardcoded" default="no" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <method name="getInterfaceName" const="no" internal="no" wsmap="managed">
      <param name="return" type="wstring" dir="return" safearray="no"/>
    </method>
    <method name="release" const="no" internal="no" wsmap="managed">
    </method>
  </interface>
  <interface name="IWebsessionManager" extends="$unknown" uuid="930c8fea-453a-4a65-aca9-19ed9a872f88" internal="yes" wsmap="global" wscpp="hardcoded" default="no" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <method name="logon" const="no" internal="no" wsmap="managed">
      <param name="username" type="wstring" dir="in" safearray="no"/>
      <param name="password" type="wstring" dir="in" safearray="no"/>
      <param name="return" type="IVirtualBox" dir="return" safearray="no"/>
    </method>
    <method name="getSessionObject" const="no" internal="no" wsmap="managed">
      <param name="refIVirtualBox" type="IVirtualBox" dir="in" safearray="no"/>
      <param name="return" type="ISession" dir="return" safearray="no"/>
    </method>
    <method name="logoff" const="no" internal="no" wsmap="managed">
      <param name="refIVirtualBox" type="IVirtualBox" dir="in" safearray="no"/>
    </method>
  </interface>
</if>
  <interface name="IPerformanceMetric" extends="$unknown" uuid="81314d14-fd1c-411a-95c5-e9bb1414e632" wsmap="managed" rest="managed" reservedAttributes="8" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="metricName" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="object" type="$unknown" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="description" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="period" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="count" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="unit" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="minimumValue" type="long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="maximumValue" type="long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IPerformanceCollector" extends="$unknown" uuid="b14290ad-cd54-400c-b858-797bcb82570e" wsmap="managed" reservedMethods="4" reservedAttributes="8" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="metricNames" type="wstring" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <method name="getMetrics" const="no" internal="no" wsmap="managed">
      <param name="metricNames" type="wstring" dir="in" safearray="yes">
      </param>
      <param name="objects" type="$unknown" dir="in" safearray="yes">
      </param>
      <param name="metrics" type="IPerformanceMetric" dir="return" safearray="yes">
      </param>
    </method>
    <method name="setupMetrics" const="no" internal="no" wsmap="managed">
      <param name="metricNames" type="wstring" dir="in" safearray="yes">
      </param>
      <param name="objects" type="$unknown" dir="in" safearray="yes">
      </param>
      <param name="period" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="count" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="affectedMetrics" type="IPerformanceMetric" dir="return" safearray="yes">
      </param>
    </method>
    <method name="enableMetrics" const="no" internal="no" wsmap="managed">
      <param name="metricNames" type="wstring" dir="in" safearray="yes">
      </param>
      <param name="objects" type="$unknown" dir="in" safearray="yes">
      </param>
      <param name="affectedMetrics" type="IPerformanceMetric" dir="return" safearray="yes">
      </param>
    </method>
    <method name="disableMetrics" const="no" internal="no" wsmap="managed">
      <param name="metricNames" type="wstring" dir="in" safearray="yes">
      </param>
      <param name="objects" type="$unknown" dir="in" safearray="yes">
      </param>
      <param name="affectedMetrics" type="IPerformanceMetric" dir="return" safearray="yes">
      </param>
    </method>
    <method name="queryMetricsData" const="no" internal="no" wsmap="managed">
      <param name="metricNames" type="wstring" dir="in" safearray="yes">
      </param>
      <param name="objects" type="$unknown" dir="in" safearray="yes">
      </param>
      <param name="returnMetricNames" type="wstring" dir="out" safearray="yes">
      </param>
      <param name="returnObjects" type="$unknown" dir="out" safearray="yes">
      </param>
      <param name="returnUnits" type="wstring" dir="out" safearray="yes">
      </param>
      <param name="returnScales" type="unsigned long" dir="out" safearray="yes">
      </param>
      <param name="returnSequenceNumbers" type="unsigned long" dir="out" safearray="yes">
      </param>
      <param name="returnDataIndices" type="unsigned long" dir="out" safearray="yes">
      </param>
      <param name="returnDataLengths" type="unsigned long" dir="out" safearray="yes">
      </param>
      <param name="returnData" type="long" dir="return" safearray="yes">
      </param>
    </method>
  </interface>
  <enum name="NATAliasMode" uuid="67772168-50d9-11df-9669-7fb714ee4fa1">
    <const name="AliasLog" value="0x1" wsmap="managed">
    </const>
    <const name="AliasProxyOnly" value="0x02" wsmap="managed">
    </const>
    <const name="AliasUseSamePorts" value="0x04" wsmap="managed">
    </const>
  </enum>
  <enum name="NATProtocol" uuid="e90164be-eb03-11de-94af-fff9b1c1b19f">
    <const name="UDP" value="0" wsmap="managed">
    </const>
    <const name="TCP" value="1" wsmap="managed">
    </const>
  </enum>
  <interface name="INATEngine" extends="$unknown" uuid="55f6cedb-141d-41bb-9995-ee2fc7f9101a" wsmap="managed" rest="managed" reservedMethods="4" reservedAttributes="6" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="network" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="hostIP" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="TFTPPrefix" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="TFTPBootFile" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="TFTPNextServer" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="aliasMode" type="unsigned long" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="DNSPassDomain" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="DNSProxy" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="DNSUseHostResolver" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="redirects" type="wstring" readonly="yes" safearray="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="localhostReachable" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="forwardBroadcast" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="enableTFTP" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="natMRU" type="unsigned long" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="IPv6Enabled" type="boolean" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="IPv6Prefix" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <method name="setNetworkSettings" const="no" internal="no" wsmap="managed">
      <param name="mtu" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="sockSnd" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="sockRcv" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="TcpWndSnd" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="TcpWndRcv" type="unsigned long" dir="in" safearray="no">
      </param>
    </method>
    <method name="getNetworkSettings" const="no" internal="no" wsmap="managed">
      <param name="mtu" type="unsigned long" dir="out" safearray="no"/>
      <param name="sockSnd" type="unsigned long" dir="out" safearray="no"/>
      <param name="sockRcv" type="unsigned long" dir="out" safearray="no"/>
      <param name="TcpWndSnd" type="unsigned long" dir="out" safearray="no"/>
      <param name="TcpWndRcv" type="unsigned long" dir="out" safearray="no"/>
    </method>
    <method name="addRedirect" const="no" internal="no" wsmap="managed">
        <param name="name" type="wstring" dir="in" safearray="no">
        </param>
        <param name="proto" type="NATProtocol" dir="in" safearray="no">
        </param>
        <param name="hostIP" type="wstring" dir="in" safearray="no">
        </param>
        <param name="hostPort" type="unsigned short" dir="in" safearray="no">
        </param>
        <param name="guestIP" type="wstring" dir="in" safearray="no">
        </param>
        <param name="guestPort" type="unsigned short" dir="in" safearray="no">
        </param>
    </method>
    <method name="removeRedirect" const="no" internal="no" wsmap="managed">
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
    </method>
  </interface>
  <interface name="IExtPackPlugIn" extends="$unknown" uuid="78861431-d545-44aa-8013-181b8c288554" wsmap="suppress" reservedAttributes="4" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="name" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="description" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="frontend" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="modulePath" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IExtPackBase" extends="$unknown" uuid="aa204a12-5b29-45a5-b5d6-c2bafcdb9b0b" wsmap="suppress" reservedMethods="4" reservedAttributes="8" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="name" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="description" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="version" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="revision" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="edition" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="VRDEModule" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="CryptoModule" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="plugIns" type="IExtPackPlugIn" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="usable" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="whyUnusable" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="showLicense" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="license" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <method name="queryLicense" const="no" internal="no" wsmap="managed">
      <param name="preferredLocale" type="wstring" dir="in" safearray="no">
      </param>
      <param name="preferredLanguage" type="wstring" dir="in" safearray="no">
      </param>
      <param name="format" type="wstring" dir="in" safearray="no">
      </param>
      <param name="licenseText" type="wstring" dir="return" safearray="no">
      </param>
    </method>
  </interface>
  <interface name="IExtPack" extends="IExtPackBase" uuid="431685da-3618-4ebc-b038-833ba829b4b2" wsmap="suppress" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <method name="queryObject" const="no" internal="no" wsmap="managed">
      <param name="objUuid" type="wstring" dir="in" safearray="no">
      </param>
      <param name="returnInterface" type="$unknown" dir="return" safearray="no">
      </param>
    </method>
  </interface>
  <interface name="IExtPackFile" extends="IExtPackBase" uuid="41304f1b-7e72-4f34-b8f6-682785620c57" wsmap="suppress" reservedMethods="2" reservedAttributes="4" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="filePath" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <method name="install" const="no" internal="no" wsmap="managed">
      <param name="replace" type="boolean" dir="in" safearray="no">
      </param>
      <param name="displayInfo" type="wstring" dir="in" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
  </interface>
  <interface name="IExtPackManager" extends="$unknown" uuid="70401eef-c8e9-466b-9660-45cb3e9979e4" wsmap="suppress" reservedMethods="4" reservedAttributes="8" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="installedExtPacks" type="IExtPack" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <method name="find" const="no" internal="no" wsmap="managed">
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="returnData" type="IExtPack" dir="return" safearray="no">
      </param>
    </method>
    <method name="openExtPackFile" const="no" internal="no" wsmap="managed">
      <param name="path" type="wstring" dir="in" safearray="no">
      </param>
      <param name="file" type="IExtPackFile" dir="return" safearray="no">
      </param>
    </method>
    <method name="uninstall" const="no" internal="no" wsmap="managed">
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="forcedRemoval" type="boolean" dir="in" safearray="no">
      </param>
      <param name="displayInfo" type="wstring" dir="in" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="cleanup" const="no" internal="no" wsmap="managed">
    </method>
    <method name="queryAllPlugInsForFrontend" const="no" internal="no" wsmap="managed">
      <param name="frontendName" type="wstring" dir="in" safearray="no">
      </param>
      <param name="plugInModules" type="wstring" dir="return" safearray="yes">
      </param>
    </method>
    <method name="isExtPackUsable" const="no" internal="no" wsmap="managed">
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="usable" type="boolean" dir="return" safearray="no">
      </param>
    </method>
  </interface>
  <enum name="BandwidthGroupType" uuid="1d92b67d-dc69-4be9-ad4c-93a01e1e0c8e">
    <const name="Null" value="0" wsmap="managed">
    </const>
    <const name="Disk" value="1" wsmap="managed">
    </const>
    <const name="Network" value="2" wsmap="managed">
    </const>
  </enum>
  <interface name="IBandwidthGroup" extends="$unknown" uuid="31587f93-2d12-4d7c-ba6d-ce51d0d5b265" wsmap="managed" rest="managed" reservedAttributes="4" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="name" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="type" type="BandwidthGroupType" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="reference" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="maxBytesPerSec" type="long long" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IBandwidthControl" extends="$unknown" uuid="48c7f4c0-c9d6-4742-957c-a6fd52e8c4ae" wsmap="managed" rest="managed" reservedMethods="2" reservedAttributes="2" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="numGroups" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <method name="createBandwidthGroup" const="no" internal="no" wsmap="managed">
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="type" type="BandwidthGroupType" dir="in" safearray="no">
      </param>
      <param name="maxBytesPerSec" type="long long" dir="in" safearray="no">
      </param>
    </method>
    <method name="deleteBandwidthGroup" const="no" internal="no" wsmap="managed">
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <method name="getBandwidthGroup" const="yes" internal="no" wsmap="managed">
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="bandwidthGroup" type="IBandwidthGroup" dir="return" safearray="no">
      </param>
    </method>
    <method name="getAllBandwidthGroups" const="yes" internal="no" wsmap="managed">
      <param name="bandwidthGroups" type="IBandwidthGroup" dir="return" safearray="yes">
      </param>
    </method>
  </interface>
  <enum name="GuestDebugProvider" uuid="acdaddc5-aa0f-4f2e-be1f-a9be2828d24a">
    <const name="None" value="0" wsmap="managed">
    </const>
    <const name="Native" value="1" wsmap="managed">
    </const>
    <const name="GDB" value="2" wsmap="managed">
    </const>
    <const name="KD" value="3" wsmap="managed">
    </const>
  </enum>
  <enum name="GuestDebugIoProvider" uuid="0cf00b1b-2ff7-414c-81c6-6cf410eaec4a">
    <const name="None" value="0" wsmap="managed">
    </const>
    <const name="TCP" value="1" wsmap="managed">
    </const>
    <const name="UDP" value="2" wsmap="managed">
    </const>
    <const name="IPC" value="3" wsmap="managed">
    </const>
  </enum>
  <interface name="IGuestDebugControl" extends="$unknown" uuid="1474bb3a-f096-4cd7-a857-8d8e3cea7331" wsmap="managed" rest="managed" reservedMethods="2" reservedAttributes="2" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="debugProvider" type="GuestDebugProvider" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="debugIoProvider" type="GuestDebugIoProvider" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="debugAddress" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="debugPort" type="unsigned long" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IVirtualBoxClient" extends="$unknown" uuid="d2937a8e-cb8d-4382-90ba-b7da78a74573" wsmap="suppress" reservedMethods="4" reservedAttributes="4" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="virtualBox" type="IVirtualBox" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="session" type="ISession" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="eventSource" type="IEventSource" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <method name="checkMachineError" const="no" internal="no" wsmap="managed">
      <param name="machine" type="IMachine" dir="in" safearray="no">
      </param>
    </method>
  </interface>
  <enum name="VBoxEventType" uuid="0c8f8bd7-c5d8-4085-83c2-558a4025b215">
    <const name="Invalid" value="0" wsmap="managed">
    </const>
    <const name="Any" value="1" wsmap="managed">
    </const>
    <const name="Vetoable" value="2" wsmap="managed">
    </const>
    <const name="MachineEvent" value="3" wsmap="managed">
    </const>
    <const name="SnapshotEvent" value="4" wsmap="managed">
    </const>
    <const name="InputEvent" value="5" wsmap="managed">
    </const>
    <const name="LastWildcard" value="31" wsmap="managed">
    </const>
    <const name="OnMachineStateChanged" value="32" wsmap="managed">
    </const>
    <const name="OnMachineDataChanged" value="33" wsmap="managed">
    </const>
    <const name="OnExtraDataChanged" value="34" wsmap="managed">
    </const>
    <const name="OnExtraDataCanChange" value="35" wsmap="managed">
    </const>
    <const name="OnMediumRegistered" value="36" wsmap="managed">
    </const>
    <const name="OnMachineRegistered" value="37" wsmap="managed">
    </const>
    <const name="OnSessionStateChanged" value="38" wsmap="managed">
    </const>
    <const name="OnSnapshotTaken" value="39" wsmap="managed">
    </const>
    <const name="OnSnapshotDeleted" value="40" wsmap="managed">
    </const>
    <const name="OnSnapshotChanged" value="41" wsmap="managed">
    </const>
    <const name="OnGuestPropertyChanged" value="42" wsmap="managed">
    </const>
    <const name="OnMousePointerShapeChanged" value="43" wsmap="managed">
    </const>
    <const name="OnMouseCapabilityChanged" value="44" wsmap="managed">
    </const>
    <const name="OnKeyboardLedsChanged" value="45" wsmap="managed">
    </const>
    <const name="OnStateChanged" value="46" wsmap="managed">
    </const>
    <const name="OnAdditionsStateChanged" value="47" wsmap="managed">
    </const>
    <const name="OnNetworkAdapterChanged" value="48" wsmap="managed">
    </const>
    <const name="OnSerialPortChanged" value="49" wsmap="managed">
    </const>
    <const name="OnParallelPortChanged" value="50" wsmap="managed">
    </const>
    <const name="OnStorageControllerChanged" value="51" wsmap="managed">
    </const>
    <const name="OnMediumChanged" value="52" wsmap="managed">
    </const>
    <const name="OnVRDEServerChanged" value="53" wsmap="managed">
    </const>
    <const name="OnUSBControllerChanged" value="54" wsmap="managed">
    </const>
    <const name="OnUSBDeviceStateChanged" value="55" wsmap="managed">
    </const>
    <const name="OnSharedFolderChanged" value="56" wsmap="managed">
    </const>
    <const name="OnRuntimeError" value="57" wsmap="managed">
    </const>
    <const name="OnCanShowWindow" value="58" wsmap="managed">
    </const>
    <const name="OnShowWindow" value="59" wsmap="managed">
    </const>
    <const name="OnCPUChanged" value="60" wsmap="managed">
    </const>
    <const name="OnVRDEServerInfoChanged" value="61" wsmap="managed">
    </const>
    <const name="OnEventSourceChanged" value="62" wsmap="managed">
    </const>
    <const name="OnCPUExecutionCapChanged" value="63" wsmap="managed">
    </const>
    <const name="OnGuestKeyboard" value="64" wsmap="managed">
    </const>
    <const name="OnGuestMouse" value="65" wsmap="managed">
    </const>
    <const name="OnNATRedirect" value="66" wsmap="managed">
    </const>
    <const name="OnHostPCIDevicePlug" value="67" wsmap="managed">
    </const>
    <const name="OnVBoxSVCAvailabilityChanged" value="68" wsmap="managed">
    </const>
    <const name="OnBandwidthGroupChanged" value="69" wsmap="managed">
    </const>
    <const name="OnGuestMonitorChanged" value="70" wsmap="managed">
    </const>
    <const name="OnStorageDeviceChanged" value="71" wsmap="managed">
    </const>
    <const name="OnClipboardModeChanged" value="72" wsmap="managed">
    </const>
    <const name="OnDnDModeChanged" value="73" wsmap="managed">
    </const>
    <const name="OnNATNetworkChanged" value="74" wsmap="managed">
    </const>
    <const name="OnNATNetworkStartStop" value="75" wsmap="managed">
    </const>
    <const name="OnNATNetworkAlter" value="76" wsmap="managed">
    </const>
    <const name="OnNATNetworkCreationDeletion" value="77" wsmap="managed">
    </const>
    <const name="OnNATNetworkSetting" value="78" wsmap="managed">
    </const>
    <const name="OnNATNetworkPortForward" value="79" wsmap="managed">
    </const>
    <const name="OnGuestSessionStateChanged" value="80" wsmap="managed">
    </const>
    <const name="OnGuestSessionRegistered" value="81" wsmap="managed">
    </const>
    <const name="OnGuestProcessRegistered" value="82" wsmap="managed">
    </const>
    <const name="OnGuestProcessStateChanged" value="83" wsmap="managed">
    </const>
    <const name="OnGuestProcessInputNotify" value="84" wsmap="managed">
    </const>
    <const name="OnGuestProcessOutput" value="85" wsmap="managed">
    </const>
    <const name="OnGuestFileRegistered" value="86" wsmap="managed">
    </const>
    <const name="OnGuestFileStateChanged" value="87" wsmap="managed">
    </const>
    <const name="OnGuestFileOffsetChanged" value="88" wsmap="managed">
    </const>
    <const name="OnGuestFileRead" value="89" wsmap="managed">
    </const>
    <const name="OnGuestFileWrite" value="90" wsmap="managed">
    </const>
    <const name="OnRecordingStateChanged" value="91" wsmap="managed">
    </const>
    <const name="OnGuestUserStateChanged" value="92" wsmap="managed">
    </const>
    <const name="OnGuestMultiTouch" value="93" wsmap="managed">
    </const>
    <const name="OnHostNameResolutionConfigurationChange" value="94" wsmap="managed">
    </const>
    <const name="OnSnapshotRestored" value="95" wsmap="managed">
    </const>
    <const name="OnMediumConfigChanged" value="96" wsmap="managed">
    </const>
    <const name="OnAudioAdapterChanged" value="97" wsmap="managed">
    </const>
    <const name="OnProgressPercentageChanged" value="98" wsmap="managed">
    </const>
    <const name="OnProgressTaskCompleted" value="99" wsmap="managed">
    </const>
    <const name="OnCursorPositionChanged" value="100" wsmap="managed">
    </const>
    <const name="OnGuestAdditionsStatusChanged" value="101" wsmap="managed">
    </const>
    <const name="OnGuestMonitorInfoChanged" value="102" wsmap="managed">
    </const>
    <const name="OnGuestFileSizeChanged" value="103" wsmap="managed">
    </const>
    <const name="OnClipboardFileTransferModeChanged" value="104" wsmap="managed">
    </const>
    <const name="OnCloudProviderListChanged" value="105" wsmap="managed">
    </const>
    <const name="OnCloudProviderRegistered" value="106" wsmap="managed">
    </const>
    <const name="OnCloudProviderUninstall" value="107" wsmap="managed">
    </const>
    <const name="OnCloudProfileRegistered" value="108" wsmap="managed">
    </const>
    <const name="OnCloudProfileChanged" value="109" wsmap="managed">
    </const>
    <const name="OnProgressCreated" value="110" wsmap="managed">
    </const>
    <const name="OnLanguageChanged" value="111" wsmap="managed">
    </const>
    <const name="OnUpdateAgentAvailable" value="112" wsmap="managed">
    </const>
    <const name="OnUpdateAgentError" value="113" wsmap="managed">
    </const>
    <const name="OnUpdateAgentSettingsChanged" value="114" wsmap="managed">
    </const>
    <const name="OnUpdateAgentStateChanged" value="115" wsmap="managed">
    </const>
    <const name="OnHostAudioDeviceChanged" value="116" wsmap="managed">
    </const>
    <const name="OnGuestDebugControlChanged" value="117" wsmap="managed">
    </const>
    <const name="OnMachineGroupsChanged" value="118" wsmap="managed">
    </const>
    <const name="OnGuestDirectoryRegistered" value="119" wsmap="managed">
    </const>
    <const name="OnGuestDirectoryStateChanged" value="120" wsmap="managed">
    </const>
    <const name="OnGuestDirectoryRead" value="121" wsmap="managed">
    </const>
    <const name="OnClipboardError" value="122" wsmap="managed">
    </const>
    <const name="OnExtPackInstalled" value="123" wsmap="managed">
    </const>
    <const name="OnExtPackUninstalled" value="124" wsmap="managed">
    </const>
    <const name="OnRecordingScreenStateChanged" value="125" wsmap="managed">
    </const>
    <const name="End" value="126" wsmap="managed">
    </const>
  </enum>
  <interface name="IEventSource" extends="$unknown" uuid="9b6e1aee-35f3-4f4d-b5bb-ed0ecefd8538" wsmap="managed" rest="managed" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <method name="createListener" const="no" internal="no" wsmap="managed">
      <param name="listener" type="IEventListener" dir="return" safearray="no"/>
    </method>
    <method name="createAggregator" const="no" internal="no" wsmap="managed">
      <param name="subordinates" type="IEventSource" dir="in" safearray="yes">
      </param>
      <param name="result" type="IEventSource" dir="return" safearray="no">
      </param>
    </method>
    <method name="registerListener" const="no" internal="no" wsmap="managed">
      <param name="listener" type="IEventListener" dir="in" safearray="no">
      </param>
      <param name="interesting" type="VBoxEventType" dir="in" safearray="yes">
      </param>
      <param name="active" type="boolean" dir="in" safearray="no">
      </param>
    </method>
    <method name="unregisterListener" const="no" internal="no" wsmap="managed">
      <param name="listener" type="IEventListener" dir="in" safearray="no">
      </param>
    </method>
    <method name="fireEvent" const="no" internal="no" wsmap="managed">
      <param name="event" type="IEvent" dir="in" safearray="no">
      </param>
      <param name="timeout" type="long" dir="in" safearray="no">
      </param>
      <param name="result" type="boolean" dir="return" safearray="no">
      </param>
    </method>
    <method name="getEvent" const="no" internal="no" wsmap="managed">
      <param name="listener" type="IEventListener" dir="in" safearray="no">
      </param>
      <param name="timeout" type="long" dir="in" safearray="no">
      </param>
      <param name="event" type="IEvent" dir="return" safearray="no">
      </param>
    </method>
    <method name="eventProcessed" const="no" internal="no" wsmap="managed">
      <param name="listener" type="IEventListener" dir="in" safearray="no">
      </param>
      <param name="event" type="IEvent" dir="in" safearray="no">
      </param>
    </method>
  </interface>
  <interface name="IEventListener" extends="$unknown" uuid="67099191-32e7-4f6c-85ee-422304c71b90" wsmap="managed" rest="managed" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <method name="handleEvent" const="no" internal="no" wsmap="managed">
      <param name="event" type="IEvent" dir="in" safearray="no">
      </param>
    </method>
  </interface>
  <interface name="IEvent" extends="$unknown" uuid="0ca2adba-8f30-401b-a8cd-fe31dbe839c0" wsmap="managed" rest="managed" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="type" readonly="yes" type="VBoxEventType" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="source" readonly="yes" type="IEventSource" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="waitable" readonly="yes" type="boolean" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <method name="setProcessed" const="no" internal="no" wsmap="managed">
    </method>
    <method name="waitProcessed" const="no" internal="no" wsmap="managed">
      <param name="timeout" type="long" dir="in" safearray="no">
      </param>
      <param name="result" type="boolean" dir="return" safearray="no">
      </param>
    </method>
  </interface>
  <interface name="IReusableEvent" extends="IEvent" uuid="69bfb134-80f6-4266-8e20-16371f68fa25" wsmap="managed" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="generation" readonly="yes" type="unsigned long" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <method name="reuse" const="no" internal="no" wsmap="managed">
    </method>
  </interface>
  <interface name="IMachineEvent" extends="IEvent" uuid="92ed7b1a-0d96-40ed-ae46-a564d484325e" wsmap="managed" id="MachineEvent" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="machineId" readonly="yes" type="uuid" mod="string" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IMachineStateChangedEvent" extends="IMachineEvent" uuid="5748F794-48DF-438D-85EB-98FFD70D18C9" wsmap="managed" autogen="VBoxEvent" id="OnMachineStateChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="state" readonly="yes" type="MachineState" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IExtPackInstalledEvent" extends="IEvent" uuid="29059fea-2c99-11ee-be56-0242ac120002" wsmap="managed" autogen="VBoxEvent" id="OnExtPackInstalled" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="name" readonly="yes" type="wstring" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IExtPackUninstalledEvent" extends="IEvent" uuid="31aab263-95ef-48a4-9ce7-eaf0d3ae150f" wsmap="managed" autogen="VBoxEvent" id="OnExtPackUninstalled" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="name" readonly="yes" type="wstring" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IMachineDataChangedEvent" extends="IMachineEvent" uuid="abe94809-2e88-4436-83d7-50f3e64d0503" wsmap="managed" autogen="VBoxEvent" id="OnMachineDataChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="temporary" readonly="yes" type="boolean" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IMediumRegisteredEvent" extends="IEvent" uuid="53fac49a-b7f1-4a5a-a4ef-a11dd9c2a458" wsmap="managed" autogen="VBoxEvent" id="OnMediumRegistered" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="mediumId" readonly="yes" type="uuid" mod="string" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="mediumType" readonly="yes" type="DeviceType" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="registered" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IMediumConfigChangedEvent" extends="IEvent" uuid="dd3e2654-a161-41f1-b583-4892f4a9d5d5" wsmap="managed" autogen="VBoxEvent" id="OnMediumConfigChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="medium" type="IMedium" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IMachineRegisteredEvent" extends="IMachineEvent" uuid="c354a762-3ff2-4f2e-8f09-07382ee25088" wsmap="managed" autogen="VBoxEvent" id="OnMachineRegistered" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="registered" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="ISessionStateChangedEvent" extends="IMachineEvent" uuid="714a3eef-799a-4489-86cd-fe8e45b2ff8e" wsmap="managed" autogen="VBoxEvent" id="OnSessionStateChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="state" type="SessionState" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IGuestPropertyChangedEvent" extends="IMachineEvent" uuid="2d0f4c6f-a77e-45c5-96d2-7ca7daae63a9" wsmap="managed" autogen="VBoxEvent" id="OnGuestPropertyChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="name" readonly="yes" type="wstring" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="value" readonly="yes" type="wstring" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="flags" readonly="yes" type="wstring" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="fWasDeleted" readonly="yes" type="boolean" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="ISnapshotEvent" extends="IMachineEvent" uuid="21637b0e-34b8-42d3-acfb-7e96daf77c22" wsmap="managed" id="SnapshotEvent" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="snapshotId" readonly="yes" type="uuid" mod="string" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="ISnapshotTakenEvent" extends="ISnapshotEvent" uuid="d27c0b3d-6038-422c-b45e-6d4a0503d9f1" wsmap="managed" autogen="VBoxEvent" id="OnSnapshotTaken" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="midlDoesNotLikeEmptyInterfaces" readonly="yes" type="boolean" internal="no" safearray="no" wsmap="managed" rest="default"/>
  </interface>
  <interface name="ISnapshotDeletedEvent" extends="ISnapshotEvent" uuid="c48f3401-4a9e-43f4-b7a7-54bd285e22f4" wsmap="managed" autogen="VBoxEvent" id="OnSnapshotDeleted" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="midlDoesNotLikeEmptyInterfaces" readonly="yes" type="boolean" internal="no" safearray="no" wsmap="managed" rest="default"/>
  </interface>
  <interface name="ISnapshotRestoredEvent" extends="ISnapshotEvent" uuid="f4d803b4-9b2d-4377-bfe6-9702e881516b" wsmap="managed" autogen="VBoxEvent" id="OnSnapshotRestored" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="midlDoesNotLikeEmptyInterfaces" readonly="yes" type="boolean" internal="no" safearray="no" wsmap="managed" rest="default"/>
  </interface>
  <interface name="ISnapshotChangedEvent" extends="ISnapshotEvent" uuid="07541941-8079-447a-a33e-47a69c7980db" wsmap="managed" autogen="VBoxEvent" id="OnSnapshotChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="midlDoesNotLikeEmptyInterfaces" readonly="yes" type="boolean" internal="no" safearray="no" wsmap="managed" rest="default"/>
  </interface>
  <interface name="IMousePointerShapeChangedEvent" extends="IEvent" uuid="a6dcf6e8-416b-4181-8c4a-45ec95177aef" wsmap="managed" autogen="VBoxEvent" id="OnMousePointerShapeChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="visible" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="alpha" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="xhot" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="yhot" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="width" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="height" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="shape" type="octet" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IMouseCapabilityChangedEvent" extends="IEvent" uuid="4a773393-7a8c-4d57-b228-9ade4049a81f" wsmap="managed" autogen="VBoxEvent" id="OnMouseCapabilityChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="supportsAbsolute" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="supportsRelative" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="supportsTouchScreen" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="supportsTouchPad" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="needsHostCursor" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IKeyboardLedsChangedEvent" extends="IEvent" uuid="6DDEF35E-4737-457B-99FC-BC52C851A44F" wsmap="managed" autogen="VBoxEvent" id="OnKeyboardLedsChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="numLock" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="capsLock" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="scrollLock" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IStateChangedEvent" extends="IEvent" uuid="4376693C-CF37-453B-9289-3B0F521CAF27" wsmap="managed" autogen="VBoxEvent" id="OnStateChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="state" type="MachineState" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IAdditionsStateChangedEvent" extends="IEvent" uuid="D70F7915-DA7C-44C8-A7AC-9F173490446A" wsmap="managed" autogen="VBoxEvent" id="OnAdditionsStateChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="midlDoesNotLikeEmptyInterfaces" readonly="yes" type="boolean" internal="no" safearray="no" wsmap="managed" rest="default"/>
  </interface>
  <interface name="INetworkAdapterChangedEvent" extends="IEvent" uuid="08889892-1EC6-4883-801D-77F56CFD0103" wsmap="managed" autogen="VBoxEvent" id="OnNetworkAdapterChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="networkAdapter" type="INetworkAdapter" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IAudioAdapterChangedEvent" extends="IEvent" uuid="D5ABC823-04D0-4DB6-8D66-DC2F033120E1" wsmap="managed" autogen="VBoxEvent" id="OnAudioAdapterChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="audioAdapter" type="IAudioAdapter" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="ISerialPortChangedEvent" extends="IEvent" uuid="3BA329DC-659C-488B-835C-4ECA7AE71C6C" wsmap="managed" autogen="VBoxEvent" id="OnSerialPortChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="serialPort" type="ISerialPort" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IParallelPortChangedEvent" extends="IEvent" uuid="813C99FC-9849-4F47-813E-24A75DC85615" wsmap="managed" autogen="VBoxEvent" id="OnParallelPortChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="parallelPort" type="IParallelPort" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IStorageControllerChangedEvent" extends="IEvent" uuid="6BB335CC-1C58-440C-BB7B-3A1397284C7B" wsmap="managed" autogen="VBoxEvent" id="OnStorageControllerChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="machinId" type="uuid" mod="string" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="controllerName" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IMediumChangedEvent" extends="IEvent" uuid="0FE2DA40-5637-472A-9736-72019EABD7DE" wsmap="managed" autogen="VBoxEvent" id="OnMediumChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="mediumAttachment" type="IMediumAttachment" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IClipboardModeChangedEvent" extends="IEvent" uuid="cac21692-7997-4595-a731-3a509db604e5" wsmap="managed" autogen="VBoxEvent" id="OnClipboardModeChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="clipboardMode" type="ClipboardMode" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IClipboardFileTransferModeChangedEvent" extends="IEvent" uuid="00391758-00B1-4E9D-0000-11FA00F9D583" wsmap="managed" autogen="VBoxEvent" id="OnClipboardFileTransferModeChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="enabled" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IClipboardEvent" extends="IEvent" uuid="f22dd3b4-e4d0-437a-bfdf-0372896ba162" wsmap="managed" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="id" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IClipboardErrorEvent" extends="IClipboardEvent" uuid="9e5f6f25-beda-46ad-8ddb-23c0268ac345" wsmap="managed" autogen="VBoxEvent" id="OnClipboardError" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="msg" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="rcError" type="long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IDnDModeChangedEvent" extends="IEvent" uuid="b55cf856-1f8b-4692-abb4-462429fae5e9" wsmap="managed" autogen="VBoxEvent" id="OnDnDModeChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="dndMode" type="DnDMode" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="ICPUChangedEvent" extends="IEvent" uuid="4da2dec7-71b2-4817-9a64-4ed12c17388e" wsmap="managed" autogen="VBoxEvent" id="OnCPUChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="CPU" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="add" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="ICPUExecutionCapChangedEvent" extends="IEvent" uuid="dfa7e4f5-b4a4-44ce-85a8-127ac5eb59dc" wsmap="managed" autogen="VBoxEvent" id="OnCPUExecutionCapChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="executionCap" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IGuestKeyboardEvent" extends="IEvent" uuid="88394258-7006-40d4-b339-472ee3801844" wsmap="managed" autogen="VBoxEvent" id="OnGuestKeyboard" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="scancodes" type="long" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <enum name="GuestMouseEventMode" uuid="4b500146-ebba-4b7c-bc29-69c2d57a5caf">
    <const name="Relative" value="0" wsmap="managed">
    </const>
    <const name="Absolute" value="1" wsmap="managed">
    </const>
  </enum>
  <interface name="IGuestMouseEvent" extends="IReusableEvent" uuid="179f8647-319c-4e7e-8150-c5837bd265f6" wsmap="managed" autogen="VBoxEvent" id="OnGuestMouse" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="mode" type="GuestMouseEventMode" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="x" type="long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="y" type="long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="z" type="long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="w" type="long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="buttons" type="long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IGuestMultiTouchEvent" extends="IEvent" uuid="1f99d9dc-c144-4c28-9f88-e6f488db5441" wsmap="managed" autogen="VBoxEvent" id="OnGuestMultiTouch" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="contactCount" type="long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="xPositions" type="short" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="yPositions" type="short" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="contactIds" type="unsigned short" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="contactFlags" type="unsigned short" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="isTouchScreen" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="scanTime" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IGuestSessionEvent" extends="IEvent" uuid="b9acd33f-647d-45ac-8fe9-f49b3183ba37" wsmap="managed" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="session" type="IGuestSession" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IGuestSessionStateChangedEvent" extends="IGuestSessionEvent" uuid="327e3c00-ee61-462f-aed3-0dff6cbf9904" wsmap="managed" autogen="VBoxEvent" id="OnGuestSessionStateChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="id" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="status" type="GuestSessionStatus" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="error" type="IVirtualBoxErrorInfo" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IGuestSessionRegisteredEvent" extends="IGuestSessionEvent" uuid="b79de686-eabd-4fa6-960a-f1756c99ea1c" wsmap="managed" autogen="VBoxEvent" id="OnGuestSessionRegistered" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="registered" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IGuestProcessEvent" extends="IGuestSessionEvent" uuid="2405f0e5-6588-40a3-9b0a-68c05ba52c4b" wsmap="managed" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="process" type="IGuestProcess" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="pid" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IGuestProcessRegisteredEvent" extends="IGuestProcessEvent" uuid="1d89e2b3-c6ea-45b6-9d43-dc6f70cc9f02" wsmap="managed" autogen="VBoxEvent" id="OnGuestProcessRegistered" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="registered" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IGuestProcessStateChangedEvent" extends="IGuestProcessEvent" uuid="c365fb7b-4430-499f-92c8-8bed814a567a" wsmap="managed" autogen="VBoxEvent" id="OnGuestProcessStateChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="status" type="ProcessStatus" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="error" type="IVirtualBoxErrorInfo" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IGuestProcessIOEvent" extends="IGuestProcessEvent" uuid="9ea9227c-e9bb-49b3-bfc7-c5171e93ef38" wsmap="managed" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="handle" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="processed" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IGuestProcessInputNotifyEvent" extends="IGuestProcessIOEvent" uuid="0de887f2-b7db-4616-aac6-cfb94d89ba78" wsmap="managed" autogen="VBoxEvent" id="OnGuestProcessInputNotify" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="status" type="ProcessInputStatus" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IGuestProcessOutputEvent" extends="IGuestProcessIOEvent" uuid="d3d5f1ee-bcb2-4905-a7ab-cc85448a742b" wsmap="managed" autogen="VBoxEvent" id="OnGuestProcessOutput" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="data" type="octet" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IGuestDirectoryEvent" extends="IGuestSessionEvent" uuid="02b69798-7cc2-4005-ac57-1ad7ff7a0997" wsmap="managed" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="directory" type="IGuestDirectory" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IGuestDirectoryRegisteredEvent" extends="IGuestDirectoryEvent" uuid="926baa39-cfc9-462e-a1a1-c439e28f7f89" wsmap="managed" autogen="VBoxEvent" id="OnGuestDirectoryRegistered" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="registered" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IGuestDirectoryStateChangedEvent" extends="IGuestDirectoryEvent" uuid="c32bce60-d69d-4eb7-a02a-411ecbab6a18" wsmap="managed" autogen="VBoxEvent" id="OnGuestDirectoryStateChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="status" type="DirectoryStatus" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="error" type="IVirtualBoxErrorInfo" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IGuestDirectoryReadEvent" extends="IGuestDirectoryEvent" uuid="4e774bbe-5285-4517-8d34-21260633c513" wsmap="managed" autogen="VBoxEvent" id="OnGuestDirectoryRead" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="path" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="objInfo" type="IFsObjInfo" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="owner" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="groups" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IGuestFileEvent" extends="IGuestSessionEvent" uuid="c8adb7b0-057d-4391-b928-f14b06b710c5" wsmap="managed" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="file" type="IGuestFile" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IGuestFileRegisteredEvent" extends="IGuestFileEvent" uuid="d0d93830-70a2-487e-895e-d3fc9679f7b3" wsmap="managed" autogen="VBoxEvent" id="OnGuestFileRegistered" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="registered" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IGuestFileStateChangedEvent" extends="IGuestFileEvent" uuid="d37fe88f-0979-486c-baa1-3abb144dc82d" wsmap="managed" autogen="VBoxEvent" id="OnGuestFileStateChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="status" type="FileStatus" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="error" type="IVirtualBoxErrorInfo" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IGuestFileIOEvent" extends="IGuestFileEvent" uuid="b5191a7c-9536-4ef8-820e-3b0e17e5bbc8" wsmap="managed" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="offset" type="long long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="processed" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IGuestFileOffsetChangedEvent" extends="IGuestFileIOEvent" uuid="e8f79a21-1207-4179-94cf-ca250036308f" wsmap="managed" autogen="VBoxEvent" id="OnGuestFileOffsetChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="midlDoesNotLikeEmptyInterfaces" readonly="yes" type="boolean" internal="no" safearray="no" wsmap="managed" rest="default"/>
  </interface>
  <interface name="IGuestFileSizeChangedEvent" extends="IGuestFileEvent" uuid="d78374e9-486e-472f-481b-969746af2480" wsmap="managed" autogen="VBoxEvent" id="OnGuestFileSizeChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="newSize" readonly="yes" type="long long" internal="no" safearray="no" wsmap="managed" rest="default"/>
  </interface>
  <interface name="IGuestFileReadEvent" extends="IGuestFileIOEvent" uuid="4ee3cbcb-486f-40db-9150-deee3fd24189" wsmap="managed" autogen="VBoxEvent" id="OnGuestFileRead" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="data" type="octet" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IGuestFileWriteEvent" extends="IGuestFileIOEvent" uuid="e062a915-3cf5-4c0a-bc90-9b8d4cc94d89" wsmap="managed" autogen="VBoxEvent" id="OnGuestFileWrite" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="midlDoesNotLikeEmptyInterfaces" readonly="yes" type="boolean" internal="no" safearray="no" wsmap="managed" rest="default"/>
  </interface>
  <interface name="IVRDEServerChangedEvent" extends="IEvent" uuid="a06fd66a-3188-4c8c-8756-1395e8cb691c" wsmap="managed" autogen="VBoxEvent" id="OnVRDEServerChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="midlDoesNotLikeEmptyInterfaces" readonly="yes" type="boolean" internal="no" safearray="no" wsmap="managed" rest="default"/>
  </interface>
  <interface name="IVRDEServerInfoChangedEvent" extends="IEvent" uuid="dd6a1080-e1b7-4339-a549-f0878115596e" wsmap="managed" autogen="VBoxEvent" id="OnVRDEServerInfoChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="midlDoesNotLikeEmptyInterfaces" readonly="yes" type="boolean" internal="no" safearray="no" wsmap="managed" rest="default"/>
  </interface>
  <interface name="IRecordingStateChangedEvent" extends="IEvent" uuid="210498ae-c922-4e86-9c45-8cfb51395fc3" wsmap="managed" autogen="VBoxEvent" id="OnRecordingStateChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="state" type="RecordingState" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="error" type="IVirtualBoxErrorInfo" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IRecordingScreenStateChangedEvent" extends="IRecordingStateChangedEvent" uuid="5900472f-cc58-48ac-a088-b571b77f839b" wsmap="managed" autogen="VBoxEvent" id="OnRecordingScreenStateChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="screen" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IUSBControllerChangedEvent" extends="IEvent" uuid="93BADC0C-61D9-4940-A084-E6BB29AF3D83" wsmap="managed" autogen="VBoxEvent" id="OnUSBControllerChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="midlDoesNotLikeEmptyInterfaces" readonly="yes" type="boolean" internal="no" safearray="no" wsmap="managed" rest="default"/>
  </interface>
  <interface name="IUSBDeviceStateChangedEvent" extends="IEvent" uuid="806da61b-6679-422a-b629-51b06b0c6d93" wsmap="managed" autogen="VBoxEvent" id="OnUSBDeviceStateChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="device" type="IUSBDevice" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="attached" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="error" type="IVirtualBoxErrorInfo" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="ISharedFolderChangedEvent" extends="IEvent" uuid="B66349B5-3534-4239-B2DE-8E1535D94C0B" wsmap="managed" autogen="VBoxEvent" id="OnSharedFolderChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="scope" type="Scope" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IRuntimeErrorEvent" extends="IEvent" uuid="883DD18B-0721-4CDE-867C-1A82ABAF914C" wsmap="managed" autogen="VBoxEvent" autogenflags="BSTR" id="OnRuntimeError" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="fatal" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="id" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="message" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IEventSourceChangedEvent" extends="IEvent" uuid="e7932cb8-f6d4-4ab6-9cbf-558eb8959a6a" waitable="yes" wsmap="managed" autogen="VBoxEvent" id="OnEventSourceChanged" default="no" internal="no" wscpp="generate" rest="suppress" wrap-gen-hook="no" notdual="no">
    <attribute name="listener" type="IEventListener" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="add" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IExtraDataChangedEvent" extends="IEvent" uuid="024F00CE-6E0B-492A-A8D0-968472A94DC7" wsmap="managed" autogen="VBoxEvent" autogenflags="BSTR" id="OnExtraDataChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="machineId" type="uuid" mod="string" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="key" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="value" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IVetoEvent" extends="IEvent" uuid="7c5e945f-2354-4267-883f-2f417d216519" wsmap="managed" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <method name="addVeto" const="no" internal="no" wsmap="managed">
      <param name="reason" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <method name="isVetoed" const="no" internal="no" wsmap="managed">
      <param name="result" type="boolean" dir="return" safearray="no">
      </param>
    </method>
    <method name="getVetos" const="no" internal="no" wsmap="managed">
      <param name="result" type="wstring" dir="return" safearray="yes">
      </param>
    </method>
    <method name="addApproval" const="no" internal="no" wsmap="managed">
      <param name="reason" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <method name="isApproved" const="no" internal="no" wsmap="managed">
      <param name="result" type="boolean" dir="return" safearray="no"/>
    </method>
    <method name="getApprovals" const="no" internal="no" wsmap="managed">
      <param name="result" type="wstring" dir="return" safearray="yes">
      </param>
    </method>
  </interface>
  <interface name="IExtraDataCanChangeEvent" extends="IVetoEvent" uuid="245d88bd-800a-40f8-87a6-170d02249a55" wsmap="managed" autogen="VBoxEvent" id="OnExtraDataCanChange" waitable="yes" default="no" internal="no" wscpp="generate" rest="suppress" wrap-gen-hook="no" notdual="no">
    <attribute name="machineId" type="uuid" mod="string" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="key" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="value" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="ICanShowWindowEvent" extends="IVetoEvent" uuid="adf292b0-92c9-4a77-9d35-e058b39fe0b9" wsmap="managed" autogen="VBoxEvent" id="OnCanShowWindow" waitable="yes" default="no" internal="no" wscpp="generate" rest="suppress" wrap-gen-hook="no" notdual="no">
    <attribute name="midlDoesNotLikeEmptyInterfaces" readonly="yes" type="boolean" internal="no" safearray="no" wsmap="managed" rest="default"/>
  </interface>
  <interface name="IShowWindowEvent" extends="IEvent" uuid="B0A0904D-2F05-4D28-855F-488F96BAD2B2" wsmap="managed" autogen="VBoxEvent" id="OnShowWindow" waitable="yes" default="no" internal="no" wscpp="generate" rest="suppress" wrap-gen-hook="no" notdual="no">
    <attribute name="winId" type="long long" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="INATRedirectEvent" extends="IMachineEvent" uuid="24eef068-c380-4510-bc7c-19314a7352f1" wsmap="managed" autogen="VBoxEvent" id="OnNATRedirect" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="slot" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="remove" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="name" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="proto" type="NATProtocol" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="hostIP" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="hostPort" type="long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="guestIP" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="guestPort" type="long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IHostAudioDeviceChangedEvent" extends="IEvent" waitable="yes" uuid="8dcc633f-7b03-4f0a-9f40-7a784dd0835a" wsmap="managed" autogen="VBoxEvent" id="OnHostAudioDeviceChangedEvent" default="no" internal="no" wscpp="generate" rest="suppress" wrap-gen-hook="no" notdual="no">
    <attribute name="device" type="IHostAudioDevice" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="new" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="state" type="AudioDeviceState" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="error" type="IVirtualBoxErrorInfo" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IHostPCIDevicePlugEvent" extends="IMachineEvent" waitable="yes" uuid="a0bad6df-d612-47d3-89d4-db3992533948" wsmap="managed" autogen="VBoxEvent" autogenflags="BSTR" id="OnHostPCIDevicePlug" default="no" internal="no" wscpp="generate" rest="suppress" wrap-gen-hook="no" notdual="no">
    <attribute name="plugged" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="success" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="attachment" type="IPCIDeviceAttachment" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="message" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IVBoxSVCAvailabilityChangedEvent" extends="IEvent" uuid="97c78fcd-d4fc-485f-8613-5af88bfcfcdc" wsmap="managed" autogen="VBoxEvent" id="OnVBoxSVCAvailabilityChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="available" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IBandwidthGroupChangedEvent" extends="IEvent" uuid="334df94a-7556-4cbc-8c04-043096b02d82" wsmap="managed" autogen="VBoxEvent" id="OnBandwidthGroupChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="bandwidthGroup" type="IBandwidthGroup" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <enum name="GuestMonitorChangedEventType" uuid="ef172985-7e36-4297-95be-e46396968d66">
    <const name="Enabled" value="0" wsmap="managed">
    </const>
    <const name="Disabled" value="1" wsmap="managed">
    </const>
    <const name="NewOrigin" value="2" wsmap="managed">
    </const>
  </enum>
  <interface name="IGuestMonitorChangedEvent" extends="IEvent" uuid="0f7b8a22-c71f-4a36-8e5f-a77d01d76090" wsmap="managed" autogen="VBoxEvent" id="OnGuestMonitorChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="changeType" type="GuestMonitorChangedEventType" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="screenId" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="originX" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="originY" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="width" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="height" type="unsigned long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IGuestUserStateChangedEvent" extends="IEvent" uuid="39b4e759-1ec0-4c0f-857f-fbe2a737a256" wsmap="managed" autogen="VBoxEvent" id="OnGuestUserStateChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="name" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="domain" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="state" type="GuestUserState" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="stateDetails" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IStorageDeviceChangedEvent" extends="IEvent" uuid="232e9151-ae84-4b8e-b0f3-5c20c35caac9" wsmap="managed" autogen="VBoxEvent" id="OnStorageDeviceChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="storageDevice" type="IMediumAttachment" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="removed" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="silent" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="INATNetworkChangedEvent" extends="IEvent" uuid="101ae042-1a29-4a19-92cf-02285773f3b5" wsmap="managed" autogen="VBoxEvent" id="OnNATNetworkChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="networkName" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
  </interface>
  <interface name="INATNetworkStartStopEvent" extends="INATNetworkChangedEvent" uuid="269d8f6b-fa1e-4cee-91c7-6d8496bea3c1" wsmap="managed" autogen="VBoxEvent" id="OnNATNetworkStartStop" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="startEvent" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="INATNetworkAlterEvent" extends="INATNetworkChangedEvent" uuid="d947adf5-4022-dc80-5535-6fb116815604" wsmap="managed" autogen="VBoxEvent" id="OnNATNetworkAlter" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="midlDoesNotLikeEmptyInterfaces" readonly="yes" type="boolean" internal="no" safearray="no" wsmap="managed" rest="default"/>
  </interface>
  <interface name="INATNetworkCreationDeletionEvent" extends="INATNetworkAlterEvent" uuid="8d984a7e-b855-40b8-ab0c-44d3515b4528" wsmap="managed" autogen="VBoxEvent" autogenflags="BSTR" id="OnNATNetworkCreationDeletion" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="creationEvent" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
  </interface>
  <interface name="INATNetworkSettingEvent" extends="INATNetworkAlterEvent" uuid="9db3a9e6-7f29-4aae-a627-5a282c83092c" wsmap="managed" autogen="VBoxEvent" id="OnNATNetworkSetting" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="enabled" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
    <attribute name="network" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
    <attribute name="gateway" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
    <attribute name="advertiseDefaultIPv6RouteEnabled" type="boolean" readonly="yes" dtracename="advertiseDefIPv6Route" internal="no" safearray="no" wsmap="managed" rest="default"/>
    <attribute name="needDhcpServer" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
  </interface>
  <interface name="INATNetworkPortForwardEvent" extends="INATNetworkAlterEvent" uuid="2514881b-23d0-430a-a7ff-7ed7f05534bc" wsmap="managed" autogen="VBoxEvent" id="OnNATNetworkPortForward" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="create" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
    <attribute name="ipv6" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
    <attribute name="name" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
    <attribute name="proto" type="NATProtocol" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
    <attribute name="hostIp" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
    <attribute name="hostPort" type="long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
    <attribute name="guestIp" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
    <attribute name="guestPort" type="long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
  </interface>
  <interface name="IHostNameResolutionConfigurationChangeEvent" extends="IEvent" uuid="f9b9e1cf-cb63-47a1-84fb-02c4894b89a9" wsmap="managed" autogen="VBoxEvent" id="OnHostNameResolutionConfigurationChange" dtracename="HostNameResCfgChangeEvent" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="midlDoesNotLikeEmptyInterfaces" readonly="yes" type="boolean" internal="no" safearray="no" wsmap="managed" rest="default"/>
  </interface>
  <interface name="IProgressEvent" extends="IEvent" uuid="daaf9016-1f04-4191-aa2f-1fac9646ae4c" wsmap="managed" id="ProgressEvent" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="progressId" readonly="yes" type="uuid" mod="string" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IProgressCreatedEvent" extends="IProgressEvent" uuid="a85bba40-1b93-47bb-b125-dec708c30fc0" wsmap="managed" autogen="VBoxEvent" id="OnProgressCreated" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="create" readonly="yes" type="boolean" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IProgressPercentageChangedEvent" extends="IProgressEvent" uuid="f05d7e60-1bcf-4218-9807-04e036cc70f1" wsmap="managed" autogen="VBoxEvent" id="OnProgressPercentageChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="percent" readonly="yes" type="long" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IProgressTaskCompletedEvent" extends="IProgressEvent" uuid="a5bbdb7d-8ce7-469f-a4c2-6476f581ff72" wsmap="managed" autogen="VBoxEvent" id="OnProgressTaskCompleted" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="midlDoesNotLikeEmptyInterfaces" readonly="yes" type="boolean" internal="no" safearray="no" wsmap="managed" rest="default"/>
  </interface>
  <interface name="ICursorPositionChangedEvent" extends="IEvent" uuid="6f302674-c927-11e7-b788-33c248e71fc7" wsmap="managed" autogen="VBoxEvent" id="OnCursorPositionChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="hasData" readonly="yes" type="boolean" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="x" readonly="yes" type="unsigned long" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="y" readonly="yes" type="unsigned long" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IGuestAdditionsStatusChangedEvent" extends="IEvent" uuid="a443da5b-aa82-4720-bc84-bd097b2b13b8" wsmap="managed" autogen="VBoxEvent" id="OnGuestAdditionsStatusChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="facility" type="AdditionsFacilityType" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="status" type="AdditionsFacilityStatus" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="runLevel" type="AdditionsRunLevelType" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="timestamp" type="long long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IGuestMonitorInfoChangedEvent" extends="IEvent" uuid="0b3cdeb2-808e-11e9-b773-133d9330f849" wsmap="managed" autogen="VBoxEvent" id="OnGuestMonitorInfoChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="output" readonly="yes" type="unsigned long" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IUpdateAgentEvent" extends="IEvent" uuid="ff58a51d-54a1-411c-93e9-3047eb4dcd21" wsmap="managed" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="agent" readonly="yes" type="IUpdateAgent" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IUpdateAgentSettingsChangedEvent" extends="IUpdateAgentEvent" uuid="abef51ae-1493-49f4-aa03-efaf106bf086" wsmap="managed" autogen="VBoxEvent" id="OnUpdateAgentSettingsChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="attributeHint" readonly="yes" type="wstring" internal="no" safearray="no" wsmap="managed" rest="default"/>
  </interface>
  <interface name="IUpdateAgentErrorEvent" extends="IUpdateAgentEvent" uuid="2a88033d-82db-4ac2-97b5-e786c839420e" wsmap="managed" autogen="VBoxEvent" id="OnUpdateAgentError" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="msg" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="rcError" type="long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IUpdateAgentAvailableEvent" extends="IUpdateAgentEvent" uuid="243829cb-15b7-42a4-8664-7aa4e34993da" wsmap="managed" autogen="VBoxEvent" id="OnUpdateAgentAvailable" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="version" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="channel" type="UpdateChannel" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="severity" type="UpdateSeverity" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="downloadURL" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="webURL" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="releaseNotes" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IUpdateAgentStateChangedEvent" extends="IUpdateAgentEvent" uuid="eb000a0e-2079-4f47-bbcc-c6b28a4e50df" wsmap="managed" autogen="VBoxEvent" id="OnUpdateAgentStateChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="state" type="UpdateState" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IGuestDebugControlChangedEvent" extends="IEvent" uuid="a3d2799e-d3ad-4f73-91ef-7d839689f6d6" wsmap="managed" autogen="VBoxEvent" id="OnGuestDebugControlChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="guestDebugControl" type="IGuestDebugControl" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IMachineGroupsChangedEvent" extends="IMachineEvent" uuid="ee37afb5-7002-4786-a5c4-a9c29e1cce75" wsmap="managed" autogen="VBoxEvent" id="OnMachineGroupsChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="dummy" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IStringArray" extends="$unknown" uuid="3890b2c8-604d-11e9-92d3-53cb473db9fb" wsmap="managed" reservedMethods="4" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="values" type="wstring" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default"/>
  </interface>
  <enum name="FormValueType" uuid="5f10e2f2-911f-4f55-8afb-39c790f7dc42">
    <const name="Boolean" value="0" wsmap="managed"/>
    <const name="String" value="1" wsmap="managed"/>
    <const name="Choice" value="2" wsmap="managed"/>
    <const name="RangedInteger" value="3" wsmap="managed"/>
    <const name="RangedInteger64" value="4" wsmap="managed"/>
  </enum>
  <interface name="IFormValue" extends="$unknown" uuid="67c50afe-3e78-11e9-b25e-7768f80c0e07" wsmap="managed" reservedMethods="4" reservedAttributes="8" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="type" type="FormValueType" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
    <attribute name="generation" type="long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
    <attribute name="enabled" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
    <attribute name="visible" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
    <attribute name="label" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
    <attribute name="description" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
    <attribute name="help" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
  </interface>
  <interface name="IBooleanFormValue" extends="IFormValue" uuid="4f4adcf6-3e87-11e9-8af2-576e84223953" wsmap="managed" reservedMethods="4" reservedAttributes="4" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <method name="getSelected" const="no" internal="no" wsmap="managed">
      <param name="selected" type="boolean" dir="return" safearray="no"/>
    </method>
    <method name="setSelected" const="no" internal="no" wsmap="managed">
      <param name="selected" type="boolean" dir="in" safearray="no"/>
      <param name="progress" type="IProgress" dir="return" safearray="no"/>
    </method>
  </interface>
  <interface name="IRangedIntegerFormValue" extends="IFormValue" uuid="b31c4052-7bdc-11e9-8bc2-8ffdb8b19219" wsmap="managed" reservedMethods="4" reservedAttributes="4" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="suffix" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="minimum" type="long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
    <attribute name="maximum" type="long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
    <method name="getInteger" const="no" internal="no" wsmap="managed">
      <param name="value" type="long" dir="return" safearray="no"/>
    </method>
    <method name="setInteger" const="no" internal="no" wsmap="managed">
      <param name="value" type="long" dir="in" safearray="no"/>
      <param name="progress" type="IProgress" dir="return" safearray="no"/>
    </method>
  </interface>
  <interface name="IRangedInteger64FormValue" extends="IFormValue" uuid="f01e8b48-f44d-42cc-8a83-512f6a8552f1" wsmap="managed" reservedMethods="4" reservedAttributes="4" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="suffix" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="minimum" type="long long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
    <attribute name="maximum" type="long long" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
    <method name="getInteger" const="no" internal="no" wsmap="managed">
      <param name="value" type="long long" dir="return" safearray="no"/>
    </method>
    <method name="setInteger" const="no" internal="no" wsmap="managed">
      <param name="value" type="long long" dir="in" safearray="no"/>
      <param name="progress" type="IProgress" dir="return" safearray="no"/>
    </method>
  </interface>
  <interface name="IStringFormValue" extends="IFormValue" uuid="cb6f0f2c-8384-11e9-921d-8b984e28a686" wsmap="managed" reservedMethods="4" reservedAttributes="4" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="multiline" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
    <method name="getString" const="no" internal="no" wsmap="managed">
      <param name="text" type="wstring" dir="return" safearray="no"/>
    </method>
    <method name="setString" const="no" internal="no" wsmap="managed">
      <param name="text" type="wstring" dir="in" safearray="no"/>
      <param name="progress" type="IProgress" dir="return" safearray="no"/>
    </method>
    <attribute name="clipboardString" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
  </interface>
  <interface name="IChoiceFormValue" extends="IFormValue" uuid="7191cf38-3e8a-11e9-825c-ab7b2cabce23" wsmap="managed" reservedMethods="4" reservedAttributes="4" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="values" type="wstring" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default"/>
    <method name="getSelectedIndex" const="no" internal="no" wsmap="managed">
      <param name="index" type="long" dir="return" safearray="no"/>
    </method>
    <method name="setSelectedIndex" const="no" internal="no" wsmap="managed">
      <param name="index" type="long" dir="in" safearray="no"/>
      <param name="progress" type="IProgress" dir="return" safearray="no"/>
    </method>
  </interface>
  <interface name="IForm" extends="$unknown" uuid="d05c91e2-3e8a-11e9-8082-db8ae479ef87" wsmap="managed" reservedMethods="4" reservedAttributes="4" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="values" type="IFormValue" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default"/>
    <method name="getFieldGroup" const="no" internal="no" wsmap="managed">
      <param name="field" type="wstring" dir="in" safearray="no"/>
      <param name="group" type="wstring" safearray="yes" dir="return"/>
    </method>
    <method name="apply" const="no" internal="no" wsmap="managed">
      <param name="progress" type="IProgress" dir="return" safearray="no"/>
    </method>
  </interface>
  <interface name="IVirtualSystemDescriptionForm" extends="IForm" uuid="14c2db8a-3ee4-11e9-b872-cb9447aad965" wsmap="managed" reservedMethods="4" reservedAttributes="4" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <method name="getVirtualSystemDescription" const="no" internal="no" wsmap="managed">
      <param name="description" type="IVirtualSystemDescription" dir="return" safearray="no"/>
    </method>
  </interface>
 <enum name="CloudMachineState" uuid="67b6d054-0154-4f5d-b71b-6ac406e1ff78">
    <const name="Invalid" value="0" wsmap="managed">
    </const>
    <const name="Provisioning" value="1" wsmap="managed">
    </const>
    <const name="Running" value="2" wsmap="managed">
    </const>
    <const name="Starting" value="3" wsmap="managed">
    </const>
    <const name="Stopping" value="4" wsmap="managed">
    </const>
    <const name="Stopped" value="5" wsmap="managed">
    </const>
    <const name="CreatingImage" value="6" wsmap="managed">
    </const>
    <const name="Terminating" value="7" wsmap="managed">
    </const>
    <const name="Terminated" value="8" wsmap="managed">
    </const>
</enum>
  <enum name="CloudImageState" uuid="6e5d6762-eea2-4f2c-b104-2952d0aa8a0a">
    <const name="Invalid" value="0" wsmap="managed">
    </const>
    <const name="Provisioning" value="1" wsmap="managed">
    </const>
    <const name="Importing" value="2" wsmap="managed">
    </const>
    <const name="Available" value="3" wsmap="managed">
    </const>
    <const name="Exporting" value="4" wsmap="managed">
    </const>
    <const name="Disabled" value="5" wsmap="managed">
    </const>
    <const name="Deleted" value="6" wsmap="managed">
    </const>
</enum>
  <enum name="MetricType" uuid="adf26edc-7e32-11ee-b962-0242ac120002">
    <const name="Invalid" value="0" wsmap="managed">
    </const>
    <const name="CpuUtilization" value="1" wsmap="managed">
    </const>
    <const name="MemoryUtilization" value="2" wsmap="managed">
    </const>
    <const name="DiskBytesRead" value="3" wsmap="managed">
    </const>
    <const name="DiskBytesWritten" value="4" wsmap="managed">
    </const>
    <const name="NetworksBytesIn" value="5" wsmap="managed">
    </const>
    <const name="NetworksBytesOut" value="6" wsmap="managed">
    </const>
</enum>
  <enum name="SymlinkPolicy" uuid="a818472e-215d-4279-8af8-eac4c0517bcc">
    <const name="None" value="0" wsmap="managed">
    </const>
    <const name="Forbidden" value="1" wsmap="managed">
    </const>
    <const name="AllowedInShareSubtree" value="2" wsmap="managed">
    </const>
    <const name="AllowedToRelativeTargets" value="3" wsmap="managed">
    </const>
    <const name="AllowedToAnyTarget" value="4" wsmap="managed">
    </const>
</enum>
  <interface name="ICloudNetworkGatewayInfo" extends="$unknown" uuid="89a63ace-0c65-11ea-ad23-0ff257c71a7f" wsmap="managed" reservedAttributes="5" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="publicIP" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
    <attribute name="secondaryPublicIP" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
    <attribute name="macAddress" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
    <attribute name="instanceId" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
  </interface>
  <interface name="ICloudNetworkEnvironmentInfo" extends="$unknown" uuid="181dfb55-394d-44d3-9edb-af2c4472c40a" wsmap="managed" reservedAttributes="7" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="tunnelNetworkId" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
  </interface>
  <interface name="ICloudMachine" extends="$unknown" uuid="147816c8-17e0-11eb-81fa-87cea6263e1a" wsmap="managed" rest="managed" reservedMethods="13" reservedAttributes="7" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="id" type="uuid" mod="string" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="cloudId" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="accessible" type="boolean" readonly="yes" wrap-hint-server="limitedcaller" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="accessError" type="IVirtualBoxErrorInfo" readonly="yes" wrap-hint-server="limitedcaller" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="name" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="OSTypeId" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="state" type="CloudMachineState" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <method name="refresh" const="no" internal="no" wsmap="managed">
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="getDetailsForm" const="yes" internal="no" wsmap="managed">
      <param name="form" type="IForm" dir="return" safearray="no">
      </param>
    </method>
    <method name="getSettingsForm" const="no" internal="no" wsmap="managed">
      <param name="form" type="IForm" dir="out" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="powerUp" const="no" internal="no" wsmap="managed">
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="reboot" const="no" internal="no" wsmap="managed">
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="reset" const="no" internal="no" wsmap="managed">
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="shutdown" const="no" internal="no" wsmap="managed">
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="powerDown" const="no" internal="no" wsmap="managed">
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="terminate" const="no" internal="no" wsmap="managed">
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="unregister" wrap-hint-server="limitedcaller,passcaller" const="no" internal="no" wsmap="managed">
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="remove" wrap-hint-server="limitedcaller,passcaller" const="no" internal="no" wsmap="managed">
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="getConsoleHistory" const="no" internal="no" wsmap="managed">
      <param name="stream" type="IDataStream" dir="out" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="createConsoleConnection" const="no" internal="no" wsmap="managed">
      <param name="sshPublicKey" type="wstring" dir="in" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="deleteConsoleConnection" const="no" internal="no" wsmap="managed">
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <attribute name="consoleConnectionFingerprint" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="serialConsoleCommand" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="serialConsoleCommandWindows" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="VNCConsoleCommand" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="VNCConsoleCommandWindows" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <method name="listMetricNames" const="no" internal="no" wsmap="managed">
      <param name="metricNames" type="IStringArray" dir="out" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="enumerateMetricData" const="no" internal="no" wsmap="managed">
      <param name="metricType" type="MetricType" dir="in" safearray="no">
      </param>
      <param name="pointsNumber" type="unsigned long" dir="in" safearray="no">
      </param>
      <param name="values" type="IStringArray" dir="out" safearray="no">
      </param>
      <param name="timestamps" type="IStringArray" dir="out" safearray="no">
      </param>
      <param name="unit" type="IStringArray" dir="out" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
  </interface>
  <interface name="ICloudClient" extends="$unknown" uuid="c2db178a-7485-11ec-aec4-2fbf90681a84" wsmap="managed" reservedMethods="13" reservedAttributes="8" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <method name="getExportDescriptionForm" const="no" internal="no" wsmap="managed">
      <param name="description" type="IVirtualSystemDescription" dir="in" safearray="no">
      </param>
      <param name="form" type="IVirtualSystemDescriptionForm" dir="out" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="exportVM" const="no" internal="no" wsmap="managed">
      <param name="description" type="IVirtualSystemDescription" dir="in" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="in" safearray="no">
      </param>
    </method>
    <method name="getLaunchDescriptionForm" const="no" internal="no" wsmap="managed">
      <param name="description" type="IVirtualSystemDescription" dir="in" safearray="no">
      </param>
      <param name="form" type="IVirtualSystemDescriptionForm" dir="out" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="launchVM" const="no" internal="no" wsmap="managed">
      <param name="description" type="IVirtualSystemDescription" dir="in" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="getImportDescriptionForm" const="no" internal="no" wsmap="managed">
      <param name="description" type="IVirtualSystemDescription" dir="in" safearray="no">
      </param>
      <param name="form" type="IVirtualSystemDescriptionForm" dir="out" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="importInstance" const="no" internal="no" wsmap="managed">
      <param name="description" type="IVirtualSystemDescription" dir="in" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="in" safearray="no">
      </param>
    </method>
    <method name="getCloudMachine" const="no" internal="no" wsmap="managed">
      <param name="id" type="uuid" mod="string" dir="in" safearray="no">
      </param>
      <param name="machine" type="ICloudMachine" dir="return" safearray="no">
      </param>
    </method>
    <method name="readCloudMachineList" const="no" internal="no" wsmap="managed">
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <attribute name="cloudMachineList" type="ICloudMachine" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <method name="readCloudMachineStubList" const="no" internal="no" wsmap="managed">
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <attribute name="cloudMachineStubList" type="ICloudMachine" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <method name="addCloudMachine" const="no" internal="no" wsmap="managed">
      <param name="instanceId" type="wstring" dir="in" safearray="no">
      </param>
      <param name="machine" type="ICloudMachine" dir="out" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="createCloudMachine" const="no" internal="no" wsmap="managed">
      <param name="description" type="IVirtualSystemDescription" dir="in" safearray="no">
      </param>
      <param name="machine" type="ICloudMachine" dir="out" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="listInstances" const="yes" internal="no" wsmap="managed">
      <param name="machineState" type="CloudMachineState" dir="in" safearray="yes">
      </param>
      <param name="returnNames" type="IStringArray" dir="out" safearray="no">
      </param>
      <param name="returnIds" type="IStringArray" dir="out" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="listSourceInstances" const="yes" internal="no" wsmap="managed">
      <param name="returnNames" type="IStringArray" dir="out" safearray="no">
      </param>
      <param name="returnIds" type="IStringArray" dir="out" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="listImages" const="yes" internal="no" wsmap="managed">
      <param name="imageState" type="CloudImageState" dir="in" safearray="yes">
      </param>
      <param name="returnNames" type="IStringArray" dir="out" safearray="no">
      </param>
      <param name="returnIds" type="IStringArray" dir="out" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="listBootVolumes" const="yes" internal="no" wsmap="managed">
      <param name="returnNames" type="IStringArray" dir="out" safearray="no">
      </param>
      <param name="returnIds" type="IStringArray" dir="out" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="listSourceBootVolumes" const="yes" internal="no" wsmap="managed">
      <param name="returnNames" type="IStringArray" dir="out" safearray="no">
      </param>
      <param name="returnIds" type="IStringArray" dir="out" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="listVnicAttachments" const="yes" internal="no" wsmap="managed">
      <param name="parameters" type="wstring" dir="in" safearray="yes">
      </param>
      <param name="returnVnicAttachmentIds" type="IStringArray" dir="out" safearray="no">
      </param>
      <param name="returnVnicIds" type="IStringArray" dir="out" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="getInstanceInfo" const="yes" internal="no" wsmap="managed">
      <param name="uid" type="wstring" dir="in" safearray="no">
      </param>
      <param name="description" type="IVirtualSystemDescription" dir="in" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="startInstance" const="no" internal="no" wsmap="managed">
      <param name="uid" type="wstring" dir="in" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="pauseInstance" const="no" internal="no" wsmap="managed">
      <param name="uid" type="wstring" dir="in" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="terminateInstance" const="no" internal="no" wsmap="managed">
      <param name="uid" type="wstring" dir="in" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="resetInstance" const="no" internal="no" wsmap="managed">
      <param name="uid" type="wstring" dir="in" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="cloneInstance" const="no" internal="no" wsmap="managed">
      <param name="uid" type="wstring" dir="in" safearray="no">
      </param>
      <param name="newName" type="wstring" dir="in" safearray="no">
      </param>
      <param name="clone" type="ICloudMachine" dir="out" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="createImage" const="no" internal="no" wsmap="managed">
      <param name="parameters" type="wstring" dir="in" safearray="yes">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="exportImage" const="no" internal="no" wsmap="managed">
      <param name="image" type="IMedium" dir="in" safearray="no">
      </param>
      <param name="parameters" type="wstring" dir="in" safearray="yes">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="importImage" const="no" internal="no" wsmap="managed">
      <param name="uid" type="wstring" dir="in" safearray="no">
      </param>
      <param name="parameters" type="wstring" dir="in" safearray="yes">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="deleteImage" const="no" internal="no" wsmap="managed">
      <param name="uid" type="wstring" dir="in" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="getImageInfo" const="yes" internal="no" wsmap="managed">
      <param name="uid" type="wstring" dir="in" safearray="no">
      </param>
      <param name="infoArray" type="IStringArray" dir="out" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="startCloudNetworkGateway" const="no" internal="no" wsmap="managed">
      <param name="network" type="ICloudNetwork" dir="in" safearray="no">
      </param>
      <param name="sshPublicKey" type="wstring" dir="in" safearray="no">
      </param>
      <param name="gatewayInfo" type="ICloudNetworkGatewayInfo" dir="out" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="setupCloudNetworkEnvironment" const="no" internal="no" wsmap="managed">
      <param name="tunnelNetworkName" type="wstring" dir="in" safearray="no">
      </param>
      <param name="tunnelNetworkRange" type="wstring" dir="in" safearray="no">
      </param>
      <param name="gatewayOsName" type="wstring" dir="in" safearray="no">
      </param>
      <param name="gatewayOsVersion" type="wstring" dir="in" safearray="no">
      </param>
      <param name="gatewayShape" type="wstring" dir="in" safearray="no">
      </param>
      <param name="networkEnvironmentInfo" type="ICloudNetworkEnvironmentInfo" dir="out" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="getVnicInfo" const="yes" internal="no" wsmap="managed">
      <param name="uid" type="wstring" dir="in" safearray="no">
      </param>
      <param name="infoArray" type="IStringArray" dir="out" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="getSubnetSelectionForm" const="no" internal="no" wsmap="managed">
      <param name="description" type="IVirtualSystemDescription" dir="in" safearray="no">
      </param>
      <param name="form" type="IVirtualSystemDescriptionForm" dir="out" safearray="no">
      </param>
      <param name="progress" type="IProgress" dir="return" safearray="no">
      </param>
    </method>
    <method name="getMetricTypeByName" const="yes" internal="no" wsmap="managed">
      <param name="metricName" type="wstring" dir="in" safearray="no">
      </param>
      <param name="metricType" type="MetricType" dir="return" safearray="no">
      </param>
    </method>
  </interface>
  <interface name="ICloudProfile" extends="$unknown" uuid="b1d978b8-f7b7-4b05-900e-2a9253c00f51" wsmap="managed" rest="managed" reservedMethods="4" reservedAttributes="8" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="name" type="wstring" readonly="no" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="providerId" type="uuid" mod="string" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <method name="getProperty" const="yes" internal="no" wsmap="managed">
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="value" type="wstring" dir="return" safearray="no">
      </param>
    </method>
    <method name="setProperty" const="yes" internal="no" wsmap="managed">
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="value" type="wstring" dir="in" safearray="no">
      </param>
    </method>
    <method name="getProperties" const="yes" internal="no" wsmap="managed">
      <param name="names" type="wstring" dir="in" safearray="no">
      </param>
      <param name="returnNames" type="wstring" safearray="yes" dir="out">
      </param>
      <param name="returnValues" type="wstring" safearray="yes" dir="return">
      </param>
    </method>
    <method name="setProperties" const="no" internal="no" wsmap="managed">
      <param name="names" type="wstring" safearray="yes" dir="in">
      </param>
      <param name="values" type="wstring" safearray="yes" dir="in">
      </param>
    </method>
    <method name="remove" const="no" internal="no" wsmap="managed">
    </method>
    <method name="createCloudClient" const="no" internal="no" wsmap="managed">
      <param name="cloudClient" type="ICloudClient" dir="return" safearray="no">
      </param>
    </method>
  </interface>
  <interface name="ICloudProvider" extends="$unknown" uuid="22363cfc-07da-41ec-ac4a-3dd99db35594" wsmap="managed" rest="managed" reservedMethods="8" reservedAttributes="16" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="name" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="shortName" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="id" type="uuid" mod="string" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="profiles" type="ICloudProfile" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="profileNames" type="wstring" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <attribute name="supportedPropertyNames" type="wstring" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <method name="getPropertyDescription" const="yes" internal="no" wsmap="managed">
      <param name="name" type="wstring" dir="in" safearray="no">
      </param>
      <param name="description" type="wstring" dir="return" safearray="no">
      </param>
    </method>
    <method name="createProfile" const="no" internal="no" wsmap="managed">
      <param name="profileName" type="wstring" dir="in" safearray="no">
      </param>
      <param name="names" type="wstring" safearray="yes" dir="in">
      </param>
      <param name="values" type="wstring" safearray="yes" dir="in">
      </param>
    </method>
    <method name="importProfiles" const="no" internal="no" wsmap="managed">
    </method>
    <method name="restoreProfiles" const="no" internal="no" wsmap="managed">
    </method>
    <method name="saveProfiles" const="no" internal="no" wsmap="managed">
    </method>
    <method name="getProfileByName" const="no" internal="no" wsmap="managed">
      <param name="profileName" type="wstring" dir="in" safearray="no"/>
      <param name="profile" type="ICloudProfile" dir="return" safearray="no"/>
    </method>
    <method name="prepareUninstall" wrap-hint-server="passcaller" const="no" internal="no" wsmap="managed">
    </method>
  </interface>
  <interface name="ICloudProviderManager" extends="$unknown" uuid="9128800f-762e-4120-871c-a2014234a607" wsmap="managed" rest="managed" reservedMethods="4" reservedAttributes="8" default="no" internal="no" wscpp="generate" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="providers" type="ICloudProvider" safearray="yes" readonly="yes" internal="no" wsmap="managed" rest="default">
    </attribute>
    <method name="getProviderById" const="no" internal="no" wsmap="managed">
      <param name="providerId" type="uuid" mod="string" dir="in" safearray="no"/>
      <param name="provider" type="ICloudProvider" dir="return" safearray="no"/>
    </method>
    <method name="getProviderByShortName" const="no" internal="no" wsmap="managed">
      <param name="providerName" type="wstring" dir="in" safearray="no"/>
      <param name="provider" type="ICloudProvider" dir="return" safearray="no"/>
    </method>
    <method name="getProviderByName" const="no" internal="no" wsmap="managed">
      <param name="providerName" type="wstring" dir="in" safearray="no"/>
      <param name="provider" type="ICloudProvider" dir="return" safearray="no"/>
    </method>
  </interface>
  <interface name="ICloudProviderListChangedEvent" extends="IEvent" uuid="a54d9cca-f23f-11ea-9755-efd0f1f792d9" wsmap="managed" autogen="VBoxEvent" id="OnCloudProviderListChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="registered" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
  </interface>
  <interface name="ICloudProviderRegisteredEvent" extends="IEvent" uuid="e28e227a-f231-11ea-9641-9b500c6d5365" wsmap="managed" autogen="VBoxEvent" id="OnCloudProviderRegistered" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="id" type="uuid" mod="string" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
    <attribute name="registered" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
  </interface>
  <interface name="ICloudProviderUninstallEvent" extends="IEvent" uuid="f01f1066-f231-11ea-8eee-33bb2afb0b6e" wsmap="managed" autogen="VBoxEvent" id="OnCloudProviderCanUninstall" waitable="yes" default="no" internal="no" wscpp="generate" rest="suppress" wrap-gen-hook="no" notdual="no">
    <attribute name="id" type="uuid" mod="string" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
  </interface>
  <interface name="ICloudProfileRegisteredEvent" extends="IEvent" uuid="6a5e65ba-eeb9-11ea-ae38-73242bc0f172" wsmap="managed" id="OnCloudProfileRegistered" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="providerId" type="uuid" mod="string" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
    <attribute name="name" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
    <attribute name="registered" type="boolean" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
  </interface>
  <interface name="ICloudProfileChangedEvent" extends="IEvent" uuid="83795a4c-fce1-11ea-8a17-636028ae0be2" wsmap="managed" id="OnCloudProfileChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="providerId" type="uuid" mod="string" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
    <attribute name="name" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
  </interface>
  <interface name="ILanguageChangedEvent" extends="IEvent" uuid="28935887-782b-4c94-8410-ce557b9cfe44" wsmap="managed" autogen="VBoxEvent" id="OnLanguageChanged" default="no" internal="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no">
    <attribute name="languageId" type="wstring" readonly="yes" internal="no" safearray="no" wsmap="managed" rest="default"/>
  </interface>
  <module name="VBoxSVC" context="LocalServer">
    <class name="VirtualBox" uuid="B1A7A4F2-47B9-4A1E-82B2-07CCD5323C3F" namespace="virtualbox.org">
      <interface name="IVirtualBox" default="yes" internal="no" wsmap="fail" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no"/>
    </class>
  </module>
  <module name="VBoxC" context="InprocServer" threadingModel="Free">
    <class name="VirtualBoxClient" uuid="dd3fc71d-26c0-4fe1-bf6f-67f633265bba" namespace="virtualbox.org">
      <interface name="IVirtualBoxClient" default="yes" internal="no" wsmap="fail" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no"/>
    </class>
    <class name="Session" uuid="3C02F46D-C9D2-4F11-A384-53F0CF917214" namespace="virtualbox.org">
      <interface name="ISession" default="yes" internal="no" wsmap="fail" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no"/>
    </class>
  </module>
</application>
<if target="midl">
<application name="VirtualBox System Service" uuid="ec0e78e8-fa43-43e8-ac0a-02c784c4a4fa" supportsErrorInfo="yes">
  <interface name="IVBoxSVCRegistration" extends="$unknown" notdual="yes" uuid="9e106366-4521-44cc-df95-186e4d057c83" wsmap="suppress" internal="yes" reservedMethods="0" reservedAttributes="0" default="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no">
      <method name="getVirtualBox" const="no" internal="no" wsmap="managed">
        <param name="result" type="$unknown" dir="return" safearray="no">
        </param>
      </method>
  </interface>
  <interface name="IVirtualBoxSDS" extends="$unknown" notdual="yes" uuid="890ed3dc-cc19-43fa-8ebf-baecb6b9ec87" wsmap="suppress" internal="yes" reservedMethods="0" reservedAttributes="0" default="no" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no">
      <method name="registerVBoxSVC" const="no" internal="no" wsmap="managed">
        <param name="vboxSVC" type="IVBoxSVCRegistration" dir="in" safearray="no">
        </param>
        <param name="pid" type="long" dir="in" safearray="no">
        </param>
        <param name="existingVirtualBox" type="$unknown" dir="return" safearray="no">
        </param>
      </method>
      <method name="deregisterVBoxSVC" const="no" internal="no" wsmap="managed">
        <param name="vboxSVC" type="IVBoxSVCRegistration" dir="in" safearray="no">
        </param>
        <param name="pid" type="long" dir="in" safearray="no">
        </param>
      </method>
      <method name="launchVMProcess" const="no" internal="no" wsmap="managed">
        <param name="machine" type="wstring" dir="in" safearray="no">
        </param>
        <param name="comment" type="wstring" dir="in" safearray="no">
        </param>
        <param name="frontend" type="wstring" dir="in" safearray="no">
        </param>
        <param name="environmentChanges" type="wstring" safearray="yes" dir="in">
        </param>
        <param name="cmdOptions" type="wstring" dir="in" safearray="no">
        </param>
        <param name="sessionId" type="unsigned long" dir="in" safearray="no">
        </param>
        <param name="pid" type="unsigned long" dir="return" safearray="no">
        </param>
      </method>
  </interface>
  <module name="VBoxSDS" context="LocalService">
      <class name="VirtualBoxSDS" uuid="74ab5ffe-8726-4435-aa7e-876d705bcba5" namespace="virtualbox.org">
          <interface name="IVirtualBoxSDS" default="yes" internal="no" wsmap="fail" wscpp="generate" rest="suppress" waitable="no" wrap-gen-hook="no" notdual="no"/>
      </class>
  </module>
</application>
</if>
</library>
</idl>
