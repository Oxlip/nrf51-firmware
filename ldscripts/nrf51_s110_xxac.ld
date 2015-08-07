SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

/* Linker script to configure memory regions. */
MEMORY
{
  FLASH (rx) : ORIGIN = 96 * 1024, LENGTH = (256 - 96) * 1024 /* 256KB Total Flash and S100 SD takes 96KB. */
  RAM (rwx) : ORIGIN = 0x20000000 + (8 * 1024), LENGTH = (32 - 8) * 1024 /* 32KB Total RAM and SD takes 8 kB */
}
INCLUDE "nrf51_common.ld"
