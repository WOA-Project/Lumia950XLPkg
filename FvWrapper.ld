ENTRY(_start);

SECTIONS
{
    _start = 0x00200000;
    . = 0x00200000;
    .data : {
        *(.data)
    }
}