/** @file
  This files implements shared routines for the BlockRamVariableDxe driver.

  Copyright (c) 2016, Brian McKenzie. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD
License which accompanies this distribution.  The full text of the license may
be found at http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _BLOCK_RAM_VARIABLE_LIB_H
#define _BLOCK_RAM_VARIABLE_LIB_H

VOID EFIAPI BrvSetBootContextActiveState(IN BOOLEAN State);

BOOLEAN
EFIAPI
BrvBootContextActive(VOID);

#endif /* !_BLOCK_RAM_VARIABLE_LIB_H */
