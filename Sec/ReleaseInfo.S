//------------------------------------------------------------------------------ 
//
// Copyright (c) 2012-2014, Qualcomm Technologies Inc. All rights reserved.
//
//------------------------------------------------------------------------------

//=============================================================================
//                              EDIT HISTORY
//
//
// when       who     what, where, why
// --------   ---     ---------------------------------------------------------
// 05/26/14   vk      Add full string to info block
// 01/21/14   vk      Align InfoBlkPtr using align directive
// 01/17/14   vk      Align InfoBlkPtr
// 10/31/13   vk      AARCH64 version
// 12/10/12   yg      Add UefiInfoBlk and Format
// 11/02/12   vk      Initial revision
//
//============================================================================
  
#include <ReleaseInfo.h>
#define LINE_FEED  0x0A

#define REL_INF_STRING  REL_LABEL_FIELD REL_LABEL " "    \
                        REL_DATE_FIELD REL_DATE " "      \
                        BUILD_VER_FIELD BUILD_VER1 " "   \
                        BUILD_DATE_FIELD  BUILD_DATE " " \
                        BUILD_TIME_FIELD  BUILD_TIME " " 

.text
.align 3

.global _ReleaseInfo
.global _UefiInfoBlockPtr
.global _UefiRelInfoString
        
.extern _ModuleEntryPoint

_ReleaseInfo:
  b _ModuleEntryPoint

.align 3
_UefiInfoBlockPtr:
.quad 0

.align 3
_UefiRelInfoString:
 .ascii REL_INF_STRING

.align 3 
.quad 0
  
