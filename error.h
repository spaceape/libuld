#ifndef uld_error_h
#define uld_error_h
/** 
    Copyright (c) 2022, wicked systems
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following
    conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following
      disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
      disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of wicked systems nor the names of its contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
    INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
    EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**/
#include <uld.h>

namespace uld {

/* e_memory
   Memory allocation error
*/
constexpr int e_memory = 2;

/* e_access
   Access error - could not fetch expected data.
*/
constexpr int e_access = 3;

/* e_nosym
   Symbol does not exist or could not be found.
*/
constexpr int e_nosym = 12;

/* e_norel
   Relocation can not be applied, it's either undefined or not implemented.
*/
constexpr int e_norel = 13;

/* e_noreach
   Trying to set an address field to a memory location outside its boundaries.
*/
constexpr int e_noreach = 14;

/* e_nodef
   General fault while trying to define symbol.
*/
constexpr int e_nodef = 15;

/* e_redef
   Symbol redefinition.
*/
constexpr int e_redef = 16;

/* e_invalid_host
*/
constexpr int e_invalid_host = 127;

/* e_invalid_target
*/
constexpr int e_invalid_target = 128;

/* e_fault
   Generic internal fault
*/
constexpr int e_fault = 255;

/*namespace uld*/ }
#endif
