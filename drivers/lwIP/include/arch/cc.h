//
// Created by sacha on 10/29/26.
//

#pragma once

#include <stdint.h>
#include <stddef.h>

#define BYTE_ORDER LITTLE_ENDIAN

#define PACK_STRUCT_FIELD(x) x
#define PACK_STRUCT_STRUCT __attribute__((packed))
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_END
#define U16_F "u"
#define U32_F "u"

#define LWIP_NOASSERT 1
#define LWIP_NO_CTYPE_H 1
#define SYS_LIGHTWEIGHT_PROT 0

typedef uint32_t sys_prot_t;

#define LWIP_PLATFORM_DIAG(x) do {} while (0)
#define LWIP_PLATFORM_ASSERT(x) do {} while (0)

#define LWIP_NO_UNISTD_H 1

typedef int64_t ssize_t;