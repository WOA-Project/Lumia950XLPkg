ENTRY(_start);

SECTIONS
{
    _start = 0x00080000;
    . = 0x00080000;
    .data : {
        *(.data)
    }
}