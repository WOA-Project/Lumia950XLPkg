.text
.align 3

.global _ReleaseInfo
        
.extern _ModuleEntryPoint

_ReleaseInfo:
  b _ModuleEntryPoint
