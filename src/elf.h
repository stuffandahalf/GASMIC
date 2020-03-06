#ifndef GASMIC_ELF_H
#define GASMIC_ELF_H

//#if __STDC_VERSION__ >= 199901L || __cplusplus >= 201103L || _MSVC_LANG >= 201103L
#ifdef __cplusplus
#define __STDC_FORMAT_MACROS
#endif
/*#include <inttypes.h>
#else
#include <fixedint.h>
#endif*/

/* 32-bit data types */
typedef uint32_t	Elf32_Addr;		/* Unsigned program address */
typedef uint32_t	Elf32_Off;		/* Unsigned file offset */
typedef uint16_t	Elf32_Half;		/* Unsigned medium integer */
typedef uint32_t	Elf32_Word;		/* Unsigned integer */
typedef int32_t		Elf32_Sword;	/* Signed integer */

/* 64-bit data types */
typedef uint64_t	Elf64_Addr;		/* Unsigned program address */
typedef uint64_t	Elf64_Off;		/* Unsigned file offset */
typedef uint16_t	Elf64_Half;		/* Unsigned medium integer */
typedef uint32_t	Elf64_Word;		/* Unsigned integer */
typedef int32_t		Elf64_Sword;	/* Signed integer */
typedef uint64_t	Elf64_Xword;	/* Unsigned long integer */
typedef int64_t		Elf64_Xsword;	/* Signed long integer */

/*#if defined(__GNUC__)
#define PACK(declaration) declaration __attribute__((__packed__))
#elif defined(_MSC_VER)
#define PACK(declaration) __pragma(pack(push, 1)) declaration __pragma(pack(pop))
#endif*/

/* ELF Identification fields */
#define EI_MAG0			0	/* EI_MAG0 - EI_MAG3 are the ELF magic numbers */
#define EI_MAG1			1
#define EI_MAG2			2
#define EI_MAG3			3
#define EI_CLASS		4	/* File class */
#define EI_DATA			5	/* Data encoding */
#define EI_VERSION		6	/* File version */
#define EI_OSABI		7	/* Operating system / ABI identification */
#define EI_ABIVERSION	8
#define EI_NIDENT		16	/* Number of identification fields */

/* ELF Magic numbers */
#define ELFMAG0			0x7f
#define ELFMAG1			'E'
#define ELFMAG2			'L'
#define ELFMAG3			'F'

/* ELF File class / capacity */
#define ELFCLASSNONE	0	/* Invalid class */
#define ELFCLASS32		1	/* 32-bit objects */
#define ELFCLASS64		2	/* 64-bit objects */

/* ELF data layout */
#define ELFDATANONE		0	/* Invalid data encoding */
#define ELFDATA2LSB		1	/* 2's compliment, little endian */
#define ELFDATA2MSB		2	/* 2's compliment, big endian */

/* OS ABI ELF extensions */
#define ELFOSABI_NONE		0	/* No extensions or unspecified */
#define ELFOSABI_HPUX		1	/* Hewlett-Packard HP-UX */
#define ELFOSABI_NETBSD		2	/* NetBSD */
#define ELFOSABI_GNU		3	/* GNU */
#define ELFOSABI_LINUX		3	/* Linux, alias for ELFOSABI_GNU */
#define ELFOSABI_SOLARIS	6	/* Sun Solaris */
#define ELFOSABI_AIX		7	/* AIX */
#define ELFOSABI_IRIX		8	/* IRIX */
#define ELFOSABI_FREEBSD	9	/* FreeBSD */
#define ELFOSABI_TRU64		10	/* Compaq TRU64 UNIX */
#define ELFOSABI_MODESTO	11	/* Novell Modesto */
#define ELFOSABI_OPENBSD	12	/* OpenBSD */
#define ELFOSABI_OPENVMS	13	/* Open VMS */
#define ELFOSABI_NSK		14	/* Hewlett-Packard Non-Stop Kernel */
#define ELFOSABI_AROS		15	/* Amiga Research OS */
#define ELFOSABI_FENIXOS	16	/* The FenixOS highly scalable multi-core OS */
#define ELFOSABI_CLOUDABI	17	/* Nuxi CloudABI */
#define ELFOSABI_OPENVOS	18	/* Stratus Technologies OpenVOS */

/* ELF file version */
#define EV_NONE			0	/* Invalid version */
#define EV_CURRENT		1	/* Current version */

typedef struct {
	unsigned char   e_ident[EI_NIDENT];
	Elf32_Half	  e_type;
	Elf32_Half	  e_machine;
	Elf32_Word	  e_version;
	Elf32_Addr	  e_entry;
	Elf32_Off	   e_phoff;
	Elf32_Off	   e_shoff;
	Elf32_Word	  e_flags;
	Elf32_Half	  e_ehsize;
	Elf32_Half	  e_phentsize;
	Elf32_Half	  e_phnum;
	Elf32_Half	  e_shentsize;
	Elf32_Half	  e_shnum;
	Elf32_Half	  e_shstrndx;
} Elf32_Ehdr;

typedef struct {
	unsigned char	e_ident[EI_NIDENT];
	Elf64_Half	  e_type;
	Elf64_Half	  e_machine;
	Elf64_Word	  e_version;
	Elf64_Addr	  e_entry;
	Elf64_Off	   e_phoff;
	Elf64_Off	   e_shoff;
	Elf64_Word	  e_flags;
	Elf64_Half	  e_ehsize;
	Elf64_Half	  e_phentsize;
	Elf64_Half	  e_phnum;
	Elf64_Half	  e_shentsize;
	Elf64_Half	  e_shnum;
	Elf64_Half	  e_shstrndx;
} Elf64_Ehdr;



#endif