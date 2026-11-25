/* localbinrepo — flat in-memory archive for Cact .cctk modules.
 *
 * Layout (little-endian, contiguous bytes):
 *   localbinrepo_hdr_t          (32 bytes, file offset 0)
 *   localbinrepo_entry_t[count] (24 bytes each, immediately after header)
 *   name blob             (NUL-separated UTF-8 paths, 8-byte aligned)
 *   data blobs            (each entry's payload, 16-byte aligned)
 *
 * Reader walks entries[], indexes name_off into the name blob and
 * data_off as a byte offset from the *image base* (offset 0).
 */

#ifndef LOCALBINREPO_H
#define LOCALBINREPO_H

#include <stdint.h>

#define LOCALBINREPO_MAGIC    0x53464B43u   /* 'C','K','F','S' little-endian */
#define LOCALBINREPO_VERSION  1u

typedef struct {
    uint32_t magic;        /* LOCALBINREPO_MAGIC */
    uint32_t version;      /* LOCALBINREPO_VERSION */
    uint32_t total_size;   /* size of the entire image, including header */
    uint32_t count;        /* number of entries */
    uint32_t entries_off;  /* file offset to entries[] */
    uint32_t names_off;    /* file offset to name blob */
    uint32_t names_size;   /* bytes in name blob */
    uint32_t reserved;
} localbinrepo_hdr_t;

typedef struct {
    uint32_t name_off;     /* offset within names blob (0-based) */
    uint32_t name_len;     /* bytes (no trailing NUL counted) */
    uint32_t data_off;     /* file offset to payload (from image base) */
    uint32_t data_size;    /* payload size in bytes */
    uint32_t flags;        /* reserved, 0 */
    uint32_t reserved;
} localbinrepo_entry_t;

#endif
