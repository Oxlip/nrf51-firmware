SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

/* Linker script to configure memory regions. */
MEMORY
{
  FLASH (rx) : ORIGIN = 112 * 1024, LENGTH = (256 - 112) * 1024 /* 256KB Total Flash and 112 kB is taken by S130 */
  RAM (rwx) : ORIGIN = 0x20000000 + (10 * 1024), LENGTH = (32 - 10) * 1024 /* 32KB Total RAM and 10 kB is taken by S130 */
}
INCLUDE "nrf51_common.ld"
