#define TRIPLET_ARCH "arm"
#define TRIPLET_OS "linux"
#define TRIPLET_ABI "gnueabihf"
#define TRIPLET TRIPLET_ARCH "-" TRIPLET_ABI
#define TCC_VERSION "0.9.26"

#define CONFIG_SYSROOT "/system"
#define CONFIG_TCCDIR "/sdcard/tcc"
#define CONFIG_TCC_SYSINCLUDEPATHS "/sdcard/tcc/include"
#define CONFIG_TCC_LIBPATHS "/system/lib"
#define CONFIG_TCC_CRTPREFIX ""
#define CONFIG_TCC_ELFINTERP "$tcc_elfinterp"
#define CONFIG_LDDIR "/system/lib"
#define CONFIG_MULTIARCHDIR TRIPLET
#define HOST_ARM 1
#define TCC_ARM_VERSION 7
#define WITHOUT_LIBTCC 1
#define TCC_TARGET_TCC_ARM_EABI 1
//#define CONFIG_TCC_BACKTRACE 1 /* enabled by default */
#undef CONFIG_TCC_BCHECK /* not supported for anything but x86 yet */


