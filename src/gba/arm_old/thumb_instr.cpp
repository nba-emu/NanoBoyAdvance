///////////////////////////////////////////////////////////////////////////////////
//
//  NanoboyAdvance is a modern Game Boy Advance emulator written in C++
//  with performance, platform independency and reasonable accuracy in mind.
//  Copyright (C) 2016 Frederic Meyer
//
//  This file is part of nanoboyadvance.
//
//  nanoboyadvance is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 2 of the License, or
//  (at your option) any later version.
//
//  nanoboyadvance is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with nanoboyadvance. If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////////


#include "arm.hpp"


// Artifact of old code. Remove when I got time.
#define reg(i) this->m_State.m_reg[i]

namespace GBA
{
    const arm::ThumbInstruction arm::THUMB_TABLE[1024] =
    {
        /* THUMB.1 Move shifted register */
        &arm::Thumb1<0,0>,  &arm::Thumb1<1,0>,  &arm::Thumb1<2,0>,  &arm::Thumb1<3,0>,
        &arm::Thumb1<4,0>,  &arm::Thumb1<5,0>,  &arm::Thumb1<6,0>,  &arm::Thumb1<7,0>,
        &arm::Thumb1<8,0>,  &arm::Thumb1<9,0>,  &arm::Thumb1<10,0>, &arm::Thumb1<11,0>,
        &arm::Thumb1<12,0>, &arm::Thumb1<13,0>, &arm::Thumb1<14,0>, &arm::Thumb1<15,0>,
        &arm::Thumb1<16,0>, &arm::Thumb1<17,0>, &arm::Thumb1<18,0>, &arm::Thumb1<19,0>,
        &arm::Thumb1<20,0>, &arm::Thumb1<21,0>, &arm::Thumb1<22,0>, &arm::Thumb1<23,0>,
        &arm::Thumb1<24,0>, &arm::Thumb1<25,0>, &arm::Thumb1<26,0>, &arm::Thumb1<27,0>,
        &arm::Thumb1<28,0>, &arm::Thumb1<29,0>, &arm::Thumb1<30,0>, &arm::Thumb1<31,0>,
        &arm::Thumb1<0,1>,  &arm::Thumb1<1,1>,  &arm::Thumb1<2,1>,  &arm::Thumb1<3,1>,
        &arm::Thumb1<4,1>,  &arm::Thumb1<5,1>,  &arm::Thumb1<6,1>,  &arm::Thumb1<7,1>,
        &arm::Thumb1<8,1>,  &arm::Thumb1<9,1>,  &arm::Thumb1<10,1>, &arm::Thumb1<11,1>,
        &arm::Thumb1<12,1>, &arm::Thumb1<13,1>, &arm::Thumb1<14,1>, &arm::Thumb1<15,1>,
        &arm::Thumb1<16,1>, &arm::Thumb1<17,1>, &arm::Thumb1<18,1>, &arm::Thumb1<19,1>,
        &arm::Thumb1<20,1>, &arm::Thumb1<21,1>, &arm::Thumb1<22,1>, &arm::Thumb1<23,1>,
        &arm::Thumb1<24,1>, &arm::Thumb1<25,1>, &arm::Thumb1<26,1>, &arm::Thumb1<27,1>,
        &arm::Thumb1<28,1>, &arm::Thumb1<29,1>, &arm::Thumb1<30,1>, &arm::Thumb1<31,1>,
        &arm::Thumb1<0,2>,  &arm::Thumb1<1,2>,  &arm::Thumb1<2,2>,  &arm::Thumb1<3,2>,
        &arm::Thumb1<4,2>,  &arm::Thumb1<5,2>,  &arm::Thumb1<6,2>,  &arm::Thumb1<7,2>,
        &arm::Thumb1<8,2>,  &arm::Thumb1<9,2>,  &arm::Thumb1<10,2>, &arm::Thumb1<11,2>,
        &arm::Thumb1<12,2>, &arm::Thumb1<13,2>, &arm::Thumb1<14,2>, &arm::Thumb1<15,2>,
        &arm::Thumb1<16,2>, &arm::Thumb1<17,2>, &arm::Thumb1<18,2>, &arm::Thumb1<19,2>,
        &arm::Thumb1<20,2>, &arm::Thumb1<21,2>, &arm::Thumb1<22,2>, &arm::Thumb1<23,2>,
        &arm::Thumb1<24,2>, &arm::Thumb1<25,2>, &arm::Thumb1<26,2>, &arm::Thumb1<27,2>,
        &arm::Thumb1<28,2>, &arm::Thumb1<29,2>, &arm::Thumb1<30,2>, &arm::Thumb1<31,2>,

        /* THUMB.2 Add / subtract */
        &arm::Thumb2<false,false,0>, &arm::Thumb2<false,false,1>, &arm::Thumb2<false,false,2>, &arm::Thumb2<false,false,3>,
        &arm::Thumb2<false,false,4>, &arm::Thumb2<false,false,5>, &arm::Thumb2<false,false,6>, &arm::Thumb2<false,false,7>,
        &arm::Thumb2<false,true,0>,  &arm::Thumb2<false,true,1>,  &arm::Thumb2<false,true,2>,  &arm::Thumb2<false,true,3>,
        &arm::Thumb2<false,true,4>,  &arm::Thumb2<false,true,5>,  &arm::Thumb2<false,true,6>,  &arm::Thumb2<false,true,7>,
        &arm::Thumb2<true,false,0>,  &arm::Thumb2<true,false,1>,  &arm::Thumb2<true,false,2>,  &arm::Thumb2<true,false,3>,
        &arm::Thumb2<true,false,4>,  &arm::Thumb2<true,false,5>,  &arm::Thumb2<true,false,6>,  &arm::Thumb2<true,false,7>,
        &arm::Thumb2<true,true,0>,   &arm::Thumb2<true,true,1>,   &arm::Thumb2<true,true,2>,   &arm::Thumb2<true,true,3>,
        &arm::Thumb2<true,true,4>,   &arm::Thumb2<true,true,5>,   &arm::Thumb2<true,true,6>,   &arm::Thumb2<true,true,7>,

        /* THUMB.3 Move/compare/add/subtract immediate */
        &arm::Thumb3<0,0>, &arm::Thumb3<0,0>, &arm::Thumb3<0,0>, &arm::Thumb3<0,0>,
        &arm::Thumb3<0,1>, &arm::Thumb3<0,1>, &arm::Thumb3<0,1>, &arm::Thumb3<0,1>,
        &arm::Thumb3<0,2>, &arm::Thumb3<0,2>, &arm::Thumb3<0,2>, &arm::Thumb3<0,2>,
        &arm::Thumb3<0,3>, &arm::Thumb3<0,3>, &arm::Thumb3<0,3>, &arm::Thumb3<0,3>,
        &arm::Thumb3<0,4>, &arm::Thumb3<0,4>, &arm::Thumb3<0,4>, &arm::Thumb3<0,4>,
        &arm::Thumb3<0,5>, &arm::Thumb3<0,5>, &arm::Thumb3<0,5>, &arm::Thumb3<0,5>,
        &arm::Thumb3<0,6>, &arm::Thumb3<0,6>, &arm::Thumb3<0,6>, &arm::Thumb3<0,6>,
        &arm::Thumb3<0,7>, &arm::Thumb3<0,7>, &arm::Thumb3<0,7>, &arm::Thumb3<0,7>,
        &arm::Thumb3<1,0>, &arm::Thumb3<1,0>, &arm::Thumb3<1,0>, &arm::Thumb3<1,0>,
        &arm::Thumb3<1,1>, &arm::Thumb3<1,1>, &arm::Thumb3<1,1>, &arm::Thumb3<1,1>,
        &arm::Thumb3<1,2>, &arm::Thumb3<1,2>, &arm::Thumb3<1,2>, &arm::Thumb3<1,2>,
        &arm::Thumb3<1,3>, &arm::Thumb3<1,3>, &arm::Thumb3<1,3>, &arm::Thumb3<1,3>,
        &arm::Thumb3<1,4>, &arm::Thumb3<1,4>, &arm::Thumb3<1,4>, &arm::Thumb3<1,4>,
        &arm::Thumb3<1,5>, &arm::Thumb3<1,5>, &arm::Thumb3<1,5>, &arm::Thumb3<1,5>,
        &arm::Thumb3<1,6>, &arm::Thumb3<1,6>, &arm::Thumb3<1,6>, &arm::Thumb3<1,6>,
        &arm::Thumb3<1,7>, &arm::Thumb3<1,7>, &arm::Thumb3<1,7>, &arm::Thumb3<1,7>,
        &arm::Thumb3<2,0>, &arm::Thumb3<2,0>, &arm::Thumb3<2,0>, &arm::Thumb3<2,0>,
        &arm::Thumb3<2,1>, &arm::Thumb3<2,1>, &arm::Thumb3<2,1>, &arm::Thumb3<2,1>,
        &arm::Thumb3<2,2>, &arm::Thumb3<2,2>, &arm::Thumb3<2,2>, &arm::Thumb3<2,2>,
        &arm::Thumb3<2,3>, &arm::Thumb3<2,3>, &arm::Thumb3<2,3>, &arm::Thumb3<2,3>,
        &arm::Thumb3<2,4>, &arm::Thumb3<2,4>, &arm::Thumb3<2,4>, &arm::Thumb3<2,4>,
        &arm::Thumb3<2,5>, &arm::Thumb3<2,5>, &arm::Thumb3<2,5>, &arm::Thumb3<2,5>,
        &arm::Thumb3<2,6>, &arm::Thumb3<2,6>, &arm::Thumb3<2,6>, &arm::Thumb3<2,6>,
        &arm::Thumb3<2,7>, &arm::Thumb3<2,7>, &arm::Thumb3<2,7>, &arm::Thumb3<2,7>,
        &arm::Thumb3<3,0>, &arm::Thumb3<3,0>, &arm::Thumb3<3,0>, &arm::Thumb3<3,0>,
        &arm::Thumb3<3,1>, &arm::Thumb3<3,1>, &arm::Thumb3<3,1>, &arm::Thumb3<3,1>,
        &arm::Thumb3<3,2>, &arm::Thumb3<3,2>, &arm::Thumb3<3,2>, &arm::Thumb3<3,2>,
        &arm::Thumb3<3,3>, &arm::Thumb3<3,3>, &arm::Thumb3<3,3>, &arm::Thumb3<3,3>,
        &arm::Thumb3<3,4>, &arm::Thumb3<3,4>, &arm::Thumb3<3,4>, &arm::Thumb3<3,4>,
        &arm::Thumb3<3,5>, &arm::Thumb3<3,5>, &arm::Thumb3<3,5>, &arm::Thumb3<3,5>,
        &arm::Thumb3<3,6>, &arm::Thumb3<3,6>, &arm::Thumb3<3,6>, &arm::Thumb3<3,6>,
        &arm::Thumb3<3,7>, &arm::Thumb3<3,7>, &arm::Thumb3<3,7>, &arm::Thumb3<3,7>,

        /* THUMB.4 ALU operations */
        &arm::Thumb4<0>,  &arm::Thumb4<1>,  &arm::Thumb4<2>,  &arm::Thumb4<3>,
        &arm::Thumb4<4>,  &arm::Thumb4<5>,  &arm::Thumb4<6>,  &arm::Thumb4<7>,
        &arm::Thumb4<8>,  &arm::Thumb4<9>,  &arm::Thumb4<10>, &arm::Thumb4<11>,
        &arm::Thumb4<12>, &arm::Thumb4<13>, &arm::Thumb4<14>, &arm::Thumb4<15>,

        /* THUMB.5 High register operations/branch exchange
         * TODO: Eventually move BX into it's own method. */
        &arm::Thumb5<0,false,false>, &arm::Thumb5<0,false,true>,
        &arm::Thumb5<0,true,false>,  &arm::Thumb5<0,true,true>,
        &arm::Thumb5<1,false,false>, &arm::Thumb5<1,false,true>,
        &arm::Thumb5<1,true,false>,  &arm::Thumb5<1,true,true>,
        &arm::Thumb5<2,false,false>, &arm::Thumb5<2,false,true>,
        &arm::Thumb5<2,true,false>,  &arm::Thumb5<2,true,true>,
        &arm::Thumb5<3,false,false>, &arm::Thumb5<3,false,true>,
        &arm::Thumb5<3,true,false>,  &arm::Thumb5<3,true,true>,

        /* THUMB.6 PC-relative load */
        &arm::Thumb6<0>, &arm::Thumb6<0>, &arm::Thumb6<0>, &arm::Thumb6<0>,
        &arm::Thumb6<1>, &arm::Thumb6<1>, &arm::Thumb6<1>, &arm::Thumb6<1>,
        &arm::Thumb6<2>, &arm::Thumb6<2>, &arm::Thumb6<2>, &arm::Thumb6<2>,
        &arm::Thumb6<3>, &arm::Thumb6<3>, &arm::Thumb6<3>, &arm::Thumb6<3>,
        &arm::Thumb6<4>, &arm::Thumb6<4>, &arm::Thumb6<4>, &arm::Thumb6<4>,
        &arm::Thumb6<5>, &arm::Thumb6<5>, &arm::Thumb6<5>, &arm::Thumb6<5>,
        &arm::Thumb6<6>, &arm::Thumb6<6>, &arm::Thumb6<6>, &arm::Thumb6<6>,
        &arm::Thumb6<7>, &arm::Thumb6<7>, &arm::Thumb6<7>, &arm::Thumb6<7>,

        /* THUMB.7 Load/store with register offset, 
           THUMB.8 Load/store sign-extended byte/halfword */
        &arm::Thumb7<0,0>, &arm::Thumb7<0,1>, &arm::Thumb7<0,2>, &arm::Thumb7<0,3>,
        &arm::Thumb7<0,4>, &arm::Thumb7<0,5>, &arm::Thumb7<0,6>, &arm::Thumb7<0,7>,
        &arm::Thumb8<0,0>, &arm::Thumb8<0,1>, &arm::Thumb8<0,2>, &arm::Thumb8<0,3>,
        &arm::Thumb8<0,4>, &arm::Thumb8<0,5>, &arm::Thumb8<0,6>, &arm::Thumb8<0,7>,
        &arm::Thumb7<1,0>, &arm::Thumb7<1,1>, &arm::Thumb7<1,2>, &arm::Thumb7<1,3>,
        &arm::Thumb7<1,4>, &arm::Thumb7<1,5>, &arm::Thumb7<1,6>, &arm::Thumb7<1,7>,
        &arm::Thumb8<1,0>, &arm::Thumb8<1,1>, &arm::Thumb8<1,2>, &arm::Thumb8<1,3>,
        &arm::Thumb8<1,4>, &arm::Thumb8<1,5>, &arm::Thumb8<1,6>, &arm::Thumb8<1,7>,
        &arm::Thumb7<2,0>, &arm::Thumb7<2,1>, &arm::Thumb7<2,2>, &arm::Thumb7<2,3>,
        &arm::Thumb7<2,4>, &arm::Thumb7<2,5>, &arm::Thumb7<2,6>, &arm::Thumb7<2,7>, 
        &arm::Thumb8<2,0>, &arm::Thumb8<2,1>, &arm::Thumb8<2,2>, &arm::Thumb8<2,3>,
        &arm::Thumb8<2,4>, &arm::Thumb8<2,5>, &arm::Thumb8<2,6>, &arm::Thumb8<2,7>, 
        &arm::Thumb7<3,0>, &arm::Thumb7<3,1>, &arm::Thumb7<3,2>, &arm::Thumb7<3,3>,
        &arm::Thumb7<3,4>, &arm::Thumb7<3,5>, &arm::Thumb7<3,6>, &arm::Thumb7<3,7>,   
        &arm::Thumb8<3,0>, &arm::Thumb8<3,1>, &arm::Thumb8<3,2>, &arm::Thumb8<3,3>,
        &arm::Thumb8<3,4>, &arm::Thumb8<3,5>, &arm::Thumb8<3,6>, &arm::Thumb8<3,7>, 

        /* THUMB.9 Load/store with immediate offset */
        &arm::Thumb9<0,0>,  &arm::Thumb9<0,1>,  &arm::Thumb9<0,2>,  &arm::Thumb9<0,3>, 
        &arm::Thumb9<0,4>,  &arm::Thumb9<0,5>,  &arm::Thumb9<0,6>,  &arm::Thumb9<0,7>, 
        &arm::Thumb9<0,8>,  &arm::Thumb9<0,9>,  &arm::Thumb9<0,10>, &arm::Thumb9<0,11>,
        &arm::Thumb9<0,12>, &arm::Thumb9<0,13>, &arm::Thumb9<0,14>, &arm::Thumb9<0,15>,
        &arm::Thumb9<0,16>, &arm::Thumb9<0,17>, &arm::Thumb9<0,18>, &arm::Thumb9<0,19>,
        &arm::Thumb9<0,20>, &arm::Thumb9<0,21>, &arm::Thumb9<0,22>, &arm::Thumb9<0,23>, 
        &arm::Thumb9<0,24>, &arm::Thumb9<0,25>, &arm::Thumb9<0,26>, &arm::Thumb9<0,27>,
        &arm::Thumb9<0,28>, &arm::Thumb9<0,29>, &arm::Thumb9<0,30>, &arm::Thumb9<0,31>,
        &arm::Thumb9<1,0>,  &arm::Thumb9<1,1>,  &arm::Thumb9<1,2>,  &arm::Thumb9<1,3>,
        &arm::Thumb9<1,4>,  &arm::Thumb9<1,5>,  &arm::Thumb9<1,6>,  &arm::Thumb9<1,7>, 
        &arm::Thumb9<1,8>,  &arm::Thumb9<1,9>,  &arm::Thumb9<1,10>, &arm::Thumb9<1,11>,
        &arm::Thumb9<1,12>, &arm::Thumb9<1,13>, &arm::Thumb9<1,14>, &arm::Thumb9<1,15>,
        &arm::Thumb9<1,16>, &arm::Thumb9<1,17>, &arm::Thumb9<1,18>, &arm::Thumb9<1,19>,
        &arm::Thumb9<1,20>, &arm::Thumb9<1,21>, &arm::Thumb9<1,22>, &arm::Thumb9<1,23>, 
        &arm::Thumb9<1,24>, &arm::Thumb9<1,25>, &arm::Thumb9<1,26>, &arm::Thumb9<1,27>,
        &arm::Thumb9<1,28>, &arm::Thumb9<1,29>, &arm::Thumb9<1,30>, &arm::Thumb9<1,31>,
        &arm::Thumb9<2,0>,  &arm::Thumb9<2,1>,  &arm::Thumb9<2,2>,  &arm::Thumb9<2,3>,
        &arm::Thumb9<2,4>,  &arm::Thumb9<2,5>,  &arm::Thumb9<2,6>,  &arm::Thumb9<2,7>, 
        &arm::Thumb9<2,8>,  &arm::Thumb9<2,9>,  &arm::Thumb9<2,10>, &arm::Thumb9<2,11>,
        &arm::Thumb9<2,12>, &arm::Thumb9<2,13>, &arm::Thumb9<2,14>, &arm::Thumb9<2,15>,
        &arm::Thumb9<2,16>, &arm::Thumb9<2,17>, &arm::Thumb9<2,18>, &arm::Thumb9<2,19>,
        &arm::Thumb9<2,20>, &arm::Thumb9<2,21>, &arm::Thumb9<2,22>, &arm::Thumb9<2,23>, 
        &arm::Thumb9<2,24>, &arm::Thumb9<2,25>, &arm::Thumb9<2,26>, &arm::Thumb9<2,27>,
        &arm::Thumb9<2,28>, &arm::Thumb9<2,29>, &arm::Thumb9<2,30>, &arm::Thumb9<2,31>,
        &arm::Thumb9<3,0>,  &arm::Thumb9<3,1>,  &arm::Thumb9<3,2>,  &arm::Thumb9<3,3>,
        &arm::Thumb9<3,4>,  &arm::Thumb9<3,5>,  &arm::Thumb9<3,6>,  &arm::Thumb9<3,7>, 
        &arm::Thumb9<3,8>,  &arm::Thumb9<3,9>,  &arm::Thumb9<3,10>, &arm::Thumb9<3,11>,
        &arm::Thumb9<3,12>, &arm::Thumb9<3,13>, &arm::Thumb9<3,14>, &arm::Thumb9<3,15>,
        &arm::Thumb9<3,16>, &arm::Thumb9<3,17>, &arm::Thumb9<3,18>, &arm::Thumb9<3,19>,
        &arm::Thumb9<3,20>, &arm::Thumb9<3,21>, &arm::Thumb9<3,22>, &arm::Thumb9<3,23>, 
        &arm::Thumb9<3,24>, &arm::Thumb9<3,25>, &arm::Thumb9<3,26>, &arm::Thumb9<3,27>,
        &arm::Thumb9<3,28>, &arm::Thumb9<3,29>, &arm::Thumb9<3,30>, &arm::Thumb9<3,31>,
        
        /* THUMB.10 Load/store halfword */
        &arm::Thumb10<false,0>,  &arm::Thumb10<false,1>,  &arm::Thumb10<false,2>,  &arm::Thumb10<false,3>,
        &arm::Thumb10<false,4>,  &arm::Thumb10<false,5>,  &arm::Thumb10<false,6>,  &arm::Thumb10<false,7>,
        &arm::Thumb10<false,8>,  &arm::Thumb10<false,9>,  &arm::Thumb10<false,10>, &arm::Thumb10<false,11>,
        &arm::Thumb10<false,12>, &arm::Thumb10<false,13>, &arm::Thumb10<false,14>, &arm::Thumb10<false,15>,
        &arm::Thumb10<false,16>, &arm::Thumb10<false,17>, &arm::Thumb10<false,18>, &arm::Thumb10<false,19>,
        &arm::Thumb10<false,20>, &arm::Thumb10<false,21>, &arm::Thumb10<false,22>, &arm::Thumb10<false,23>,
        &arm::Thumb10<false,24>, &arm::Thumb10<false,25>, &arm::Thumb10<false,26>, &arm::Thumb10<false,27>,
        &arm::Thumb10<false,28>, &arm::Thumb10<false,29>, &arm::Thumb10<false,30>, &arm::Thumb10<false,31>,
        &arm::Thumb10<true,0>,  &arm::Thumb10<true,1>,  &arm::Thumb10<true,2>,  &arm::Thumb10<true,3>,
        &arm::Thumb10<true,4>,  &arm::Thumb10<true,5>,  &arm::Thumb10<true,6>,  &arm::Thumb10<true,7>,
        &arm::Thumb10<true,8>,  &arm::Thumb10<true,9>,  &arm::Thumb10<true,10>, &arm::Thumb10<true,11>,
        &arm::Thumb10<true,12>, &arm::Thumb10<true,13>, &arm::Thumb10<true,14>, &arm::Thumb10<true,15>,
        &arm::Thumb10<true,16>, &arm::Thumb10<true,17>, &arm::Thumb10<true,18>, &arm::Thumb10<true,19>,
        &arm::Thumb10<true,20>, &arm::Thumb10<true,21>, &arm::Thumb10<true,22>, &arm::Thumb10<true,23>,
        &arm::Thumb10<true,24>, &arm::Thumb10<true,25>, &arm::Thumb10<true,26>, &arm::Thumb10<true,27>,
        &arm::Thumb10<true,28>, &arm::Thumb10<true,29>, &arm::Thumb10<true,30>, &arm::Thumb10<true,31>,
        
        /* THUMB.11 SP-relative load/store */
        &arm::Thumb11<false,0>, &arm::Thumb11<false,0>, &arm::Thumb11<false,0>, &arm::Thumb11<false,0>,
        &arm::Thumb11<false,1>, &arm::Thumb11<false,1>, &arm::Thumb11<false,1>, &arm::Thumb11<false,1>,
        &arm::Thumb11<false,2>, &arm::Thumb11<false,2>, &arm::Thumb11<false,2>, &arm::Thumb11<false,2>,
        &arm::Thumb11<false,3>, &arm::Thumb11<false,3>, &arm::Thumb11<false,3>, &arm::Thumb11<false,3>,
        &arm::Thumb11<false,4>, &arm::Thumb11<false,4>, &arm::Thumb11<false,4>, &arm::Thumb11<false,4>,
        &arm::Thumb11<false,5>, &arm::Thumb11<false,5>, &arm::Thumb11<false,5>, &arm::Thumb11<false,5>,
        &arm::Thumb11<false,6>, &arm::Thumb11<false,6>, &arm::Thumb11<false,6>, &arm::Thumb11<false,6>,
        &arm::Thumb11<false,7>, &arm::Thumb11<false,7>, &arm::Thumb11<false,7>, &arm::Thumb11<false,7>,
        &arm::Thumb11<true,0>, &arm::Thumb11<true,0>, &arm::Thumb11<true,0>, &arm::Thumb11<true,0>,
        &arm::Thumb11<true,1>, &arm::Thumb11<true,1>, &arm::Thumb11<true,1>, &arm::Thumb11<true,1>,
        &arm::Thumb11<true,2>, &arm::Thumb11<true,2>, &arm::Thumb11<true,2>, &arm::Thumb11<true,2>,
        &arm::Thumb11<true,3>, &arm::Thumb11<true,3>, &arm::Thumb11<true,3>, &arm::Thumb11<true,3>,
        &arm::Thumb11<true,4>, &arm::Thumb11<true,4>, &arm::Thumb11<true,4>, &arm::Thumb11<true,4>,
        &arm::Thumb11<true,5>, &arm::Thumb11<true,5>, &arm::Thumb11<true,5>, &arm::Thumb11<true,5>,
        &arm::Thumb11<true,6>, &arm::Thumb11<true,6>, &arm::Thumb11<true,6>, &arm::Thumb11<true,6>,
        &arm::Thumb11<true,7>, &arm::Thumb11<true,7>, &arm::Thumb11<true,7>, &arm::Thumb11<true,7>,

        /* THUMB.12 Load address */
        &arm::Thumb12<false,0>, &arm::Thumb12<false,0>, &arm::Thumb12<false,0>, &arm::Thumb12<false,0>,
        &arm::Thumb12<false,1>, &arm::Thumb12<false,1>, &arm::Thumb12<false,1>, &arm::Thumb12<false,1>,
        &arm::Thumb12<false,2>, &arm::Thumb12<false,2>, &arm::Thumb12<false,2>, &arm::Thumb12<false,2>,
        &arm::Thumb12<false,3>, &arm::Thumb12<false,3>, &arm::Thumb12<false,3>, &arm::Thumb12<false,3>,
        &arm::Thumb12<false,4>, &arm::Thumb12<false,4>, &arm::Thumb12<false,4>, &arm::Thumb12<false,4>,
        &arm::Thumb12<false,5>, &arm::Thumb12<false,5>, &arm::Thumb12<false,5>, &arm::Thumb12<false,5>,
        &arm::Thumb12<false,6>, &arm::Thumb12<false,6>, &arm::Thumb12<false,6>, &arm::Thumb12<false,6>,
        &arm::Thumb12<false,7>, &arm::Thumb12<false,7>, &arm::Thumb12<false,7>, &arm::Thumb12<false,7>,
        &arm::Thumb12<true,0>,  &arm::Thumb12<true,0>,  &arm::Thumb12<true,0>,  &arm::Thumb12<true,0>,
        &arm::Thumb12<true,1>,  &arm::Thumb12<true,1>,  &arm::Thumb12<true,1>,  &arm::Thumb12<true,1>,
        &arm::Thumb12<true,2>,  &arm::Thumb12<true,2>,  &arm::Thumb12<true,2>,  &arm::Thumb12<true,2>,
        &arm::Thumb12<true,3>,  &arm::Thumb12<true,3>,  &arm::Thumb12<true,3>,  &arm::Thumb12<true,3>,
        &arm::Thumb12<true,4>,  &arm::Thumb12<true,4>,  &arm::Thumb12<true,4>,  &arm::Thumb12<true,4>,
        &arm::Thumb12<true,5>,  &arm::Thumb12<true,5>,  &arm::Thumb12<true,5>,  &arm::Thumb12<true,5>,
        &arm::Thumb12<true,6>,  &arm::Thumb12<true,6>,  &arm::Thumb12<true,6>,  &arm::Thumb12<true,6>,
        &arm::Thumb12<true,7>,  &arm::Thumb12<true,7>,  &arm::Thumb12<true,7>,  &arm::Thumb12<true,7>,
        
        /* THUMB.13 Add offset to stack pointer,
           THUMB.14 Push/pop registers */
        &arm::Thumb13<false>,       &arm::Thumb13<false>,       &arm::Thumb13<true>,        &arm::Thumb13<true>,
        &arm::Thumb13<false>,       &arm::Thumb13<false>,       &arm::Thumb13<true>,        &arm::Thumb13<true>,
        &arm::Thumb13<false>,       &arm::Thumb13<false>,       &arm::Thumb13<true>,        &arm::Thumb13<true>,
        &arm::Thumb13<false>,       &arm::Thumb13<false>,       &arm::Thumb13<true>,        &arm::Thumb13<true>,
        &arm::Thumb14<false,false>, &arm::Thumb14<false,false>, &arm::Thumb14<false,false>, &arm::Thumb14<false,false>,
        &arm::Thumb14<false,true>,  &arm::Thumb14<false,true>,  &arm::Thumb14<false,true>,  &arm::Thumb14<false,true>,
        &arm::Thumb14<false,false>, &arm::Thumb14<false,false>, &arm::Thumb14<false,false>, &arm::Thumb14<false,false>,
        &arm::Thumb14<false,true>,  &arm::Thumb14<false,true>,  &arm::Thumb14<false,true>,  &arm::Thumb14<false,true>,
        &arm::Thumb13<false>,       &arm::Thumb13<false>,      &arm::Thumb13<true>,         &arm::Thumb13<true>,
        &arm::Thumb13<false>,       &arm::Thumb13<false>,       &arm::Thumb13<true>,        &arm::Thumb13<true>,
        &arm::Thumb13<false>,       &arm::Thumb13<false>,       &arm::Thumb13<true>,        &arm::Thumb13<true>,
        &arm::Thumb13<false>,       &arm::Thumb13<false>,       &arm::Thumb13<true>,        &arm::Thumb13<true>,
        &arm::Thumb14<true,false>,  &arm::Thumb14<true,false>,  &arm::Thumb14<true,false>,  &arm::Thumb14<true,false>,
        &arm::Thumb14<true,true>,   &arm::Thumb14<true,true>,   &arm::Thumb14<true,true>,   &arm::Thumb14<true,true>,
        &arm::Thumb14<true,false>,  &arm::Thumb14<true,false>,  &arm::Thumb14<true,false>,  &arm::Thumb14<true,false>,
        &arm::Thumb14<true,true>,   &arm::Thumb14<true,true>,   &arm::Thumb14<true,true>,   &arm::Thumb14<true,true>,
        
        /* THUMB.15 Multiple load/store */
        &arm::Thumb15<false,0>, &arm::Thumb15<false,0>, &arm::Thumb15<false,0>, &arm::Thumb15<false,0>,
        &arm::Thumb15<false,1>, &arm::Thumb15<false,1>, &arm::Thumb15<false,1>, &arm::Thumb15<false,1>,
        &arm::Thumb15<false,2>, &arm::Thumb15<false,2>, &arm::Thumb15<false,2>, &arm::Thumb15<false,2>,
        &arm::Thumb15<false,3>, &arm::Thumb15<false,3>, &arm::Thumb15<false,3>, &arm::Thumb15<false,3>,
        &arm::Thumb15<false,4>, &arm::Thumb15<false,4>, &arm::Thumb15<false,4>, &arm::Thumb15<false,4>,
        &arm::Thumb15<false,5>, &arm::Thumb15<false,5>, &arm::Thumb15<false,5>, &arm::Thumb15<false,5>,
        &arm::Thumb15<false,6>, &arm::Thumb15<false,6>, &arm::Thumb15<false,6>, &arm::Thumb15<false,6>,
        &arm::Thumb15<false,7>, &arm::Thumb15<false,7>, &arm::Thumb15<false,7>, &arm::Thumb15<false,7>,
        &arm::Thumb15<true,0>,  &arm::Thumb15<true,0>,  &arm::Thumb15<true,0>,  &arm::Thumb15<true,0>,
        &arm::Thumb15<true,1>,  &arm::Thumb15<true,1>,  &arm::Thumb15<true,1>,  &arm::Thumb15<true,1>,
        &arm::Thumb15<true,2>,  &arm::Thumb15<true,2>,  &arm::Thumb15<true,2>,  &arm::Thumb15<true,2>,
        &arm::Thumb15<true,3>,  &arm::Thumb15<true,3>,  &arm::Thumb15<true,3>,  &arm::Thumb15<true,3>,
        &arm::Thumb15<true,4>,  &arm::Thumb15<true,4>,  &arm::Thumb15<true,4>,  &arm::Thumb15<true,4>,
        &arm::Thumb15<true,5>,  &arm::Thumb15<true,5>,  &arm::Thumb15<true,5>,  &arm::Thumb15<true,5>,
        &arm::Thumb15<true,6>,  &arm::Thumb15<true,6>,  &arm::Thumb15<true,6>,  &arm::Thumb15<true,6>,
        &arm::Thumb15<true,7>,  &arm::Thumb15<true,7>,  &arm::Thumb15<true,7>,  &arm::Thumb15<true,7>,
        
        /* THUMB.16 Conditional branch */
        &arm::Thumb16<0>,  &arm::Thumb16<0>,  &arm::Thumb16<0>,  &arm::Thumb16<0>, 
        &arm::Thumb16<1>,  &arm::Thumb16<1>,  &arm::Thumb16<1>,  &arm::Thumb16<1>,
        &arm::Thumb16<2>,  &arm::Thumb16<2>,  &arm::Thumb16<2>,  &arm::Thumb16<2>, 
        &arm::Thumb16<3>,  &arm::Thumb16<3>,  &arm::Thumb16<3>,  &arm::Thumb16<3>, 
        &arm::Thumb16<4>,  &arm::Thumb16<4>,  &arm::Thumb16<4>,  &arm::Thumb16<4>,
        &arm::Thumb16<5>,  &arm::Thumb16<5>,  &arm::Thumb16<5>,  &arm::Thumb16<5>, 
        &arm::Thumb16<6>,  &arm::Thumb16<6>,  &arm::Thumb16<6>,  &arm::Thumb16<6>,
        &arm::Thumb16<7>,  &arm::Thumb16<7>,  &arm::Thumb16<7>,  &arm::Thumb16<7>, 
        &arm::Thumb16<8>,  &arm::Thumb16<8>,  &arm::Thumb16<8>,  &arm::Thumb16<8>, 
        &arm::Thumb16<9>,  &arm::Thumb16<9>,  &arm::Thumb16<9>,  &arm::Thumb16<9>, 
        &arm::Thumb16<10>, &arm::Thumb16<10>, &arm::Thumb16<10>, &arm::Thumb16<10>, 
        &arm::Thumb16<11>, &arm::Thumb16<11>, &arm::Thumb16<11>, &arm::Thumb16<11>,
        &arm::Thumb16<12>, &arm::Thumb16<12>, &arm::Thumb16<12>, &arm::Thumb16<12>, 
        &arm::Thumb16<13>, &arm::Thumb16<13>, &arm::Thumb16<13>, &arm::Thumb16<13>, 
        &arm::Thumb16<14>, &arm::Thumb16<14>, &arm::Thumb16<14>, &arm::Thumb16<14>, 
        
        /* THUMB.17 Software Interrupt */
        &arm::Thumb17, &arm::Thumb17, &arm::Thumb17, &arm::Thumb17, 
        
        /* THUMB.18 Unconditional branch */
        &arm::Thumb18, &arm::Thumb18, &arm::Thumb18, &arm::Thumb18,
        &arm::Thumb18, &arm::Thumb18, &arm::Thumb18, &arm::Thumb18,
        &arm::Thumb18, &arm::Thumb18, &arm::Thumb18, &arm::Thumb18,
        &arm::Thumb18, &arm::Thumb18, &arm::Thumb18, &arm::Thumb18,
        &arm::Thumb18, &arm::Thumb18, &arm::Thumb18, &arm::Thumb18,
        &arm::Thumb18, &arm::Thumb18, &arm::Thumb18, &arm::Thumb18,
        &arm::Thumb18, &arm::Thumb18, &arm::Thumb18, &arm::Thumb18,
        &arm::Thumb18, &arm::Thumb18, &arm::Thumb18, &arm::Thumb18,
        &arm::Thumb18, &arm::Thumb18, &arm::Thumb18, &arm::Thumb18,
        &arm::Thumb18, &arm::Thumb18, &arm::Thumb18, &arm::Thumb18,
        &arm::Thumb18, &arm::Thumb18, &arm::Thumb18, &arm::Thumb18,
        &arm::Thumb18, &arm::Thumb18, &arm::Thumb18, &arm::Thumb18,
        &arm::Thumb18, &arm::Thumb18, &arm::Thumb18, &arm::Thumb18,
        &arm::Thumb18, &arm::Thumb18, &arm::Thumb18, &arm::Thumb18,
        &arm::Thumb18, &arm::Thumb18, &arm::Thumb18, &arm::Thumb18,
        &arm::Thumb18, &arm::Thumb18, &arm::Thumb18, &arm::Thumb18,

        /* THUMB.19 Long branch with link */
        &arm::Thumb19<false>, &arm::Thumb19<false>, &arm::Thumb19<false>, &arm::Thumb19<false>,
        &arm::Thumb19<false>, &arm::Thumb19<false>, &arm::Thumb19<false>, &arm::Thumb19<false>,
        &arm::Thumb19<false>, &arm::Thumb19<false>, &arm::Thumb19<false>, &arm::Thumb19<false>,
        &arm::Thumb19<false>, &arm::Thumb19<false>, &arm::Thumb19<false>, &arm::Thumb19<false>,
        &arm::Thumb19<false>, &arm::Thumb19<false>, &arm::Thumb19<false>, &arm::Thumb19<false>,
        &arm::Thumb19<false>, &arm::Thumb19<false>, &arm::Thumb19<false>, &arm::Thumb19<false>,
        &arm::Thumb19<false>, &arm::Thumb19<false>, &arm::Thumb19<false>, &arm::Thumb19<false>,
        &arm::Thumb19<false>, &arm::Thumb19<false>, &arm::Thumb19<false>, &arm::Thumb19<false>,
        &arm::Thumb19<true>,  &arm::Thumb19<true>,  &arm::Thumb19<true>,  &arm::Thumb19<true>,
        &arm::Thumb19<true>,  &arm::Thumb19<true>,  &arm::Thumb19<true>,  &arm::Thumb19<true>,
        &arm::Thumb19<true>,  &arm::Thumb19<true>,  &arm::Thumb19<true>,  &arm::Thumb19<true>,
        &arm::Thumb19<true>,  &arm::Thumb19<true>,  &arm::Thumb19<true>,  &arm::Thumb19<true>,
        &arm::Thumb19<true>,  &arm::Thumb19<true>,  &arm::Thumb19<true>,  &arm::Thumb19<true>,
        &arm::Thumb19<true>,  &arm::Thumb19<true>,  &arm::Thumb19<true>,  &arm::Thumb19<true>,
        &arm::Thumb19<true>,  &arm::Thumb19<true>,  &arm::Thumb19<true>,  &arm::Thumb19<true>,
        &arm::Thumb19<true>,  &arm::Thumb19<true>,  &arm::Thumb19<true>,  &arm::Thumb19<true>,
    };

    template <int imm, int type>
    void arm::Thumb1(u16 instruction)
    {
        // THUMB.1 Move shifted register
        int reg_dest = instruction & 7;
        int reg_source = (instruction >> 3) & 7;
        bool carry = m_State.m_cpsr & MASK_CFLAG;

        reg(reg_dest) = reg(reg_source);

        switch (type)
        {
        case 0: // LSL
            LSL(reg(reg_dest), imm, carry);
            set_carry(carry);
            break;
        case 1: // LSR
            LSR(reg(reg_dest), imm, carry, true);
            set_carry(carry);
            break;
        case 2: // ASR
        {
            ASR(reg(reg_dest), imm, carry, true);
            set_carry(carry);
            break;
        }
        }

        // Update sign and zero flag
        update_sign(reg(reg_dest));
        update_zero(reg(reg_dest));

        // Update cycle counter
        cycles += Memory::SequentialAccess(m_State.m_reg[15], ACCESS_HWORD);
    }

    template <bool immediate, bool subtract, int field3>
    void arm::Thumb2(u16 instruction)
    {
        // THUMB.2 Add/subtract
        int reg_dest = instruction & 7;
        int reg_source = (instruction >> 3) & 7;
        u32 operand;

        // Either a register or an immediate
        if (immediate)
            operand = field3;
        else
            operand = reg(field3);

        // Determine wether to subtract or add
        if (subtract)
        {
            u32 result = reg(reg_source) - operand;

            // Calculate flags
            set_carry(reg(reg_source) >= operand);
            update_overflow_sub(result, reg(reg_source), operand);
            update_sign(result);
            update_zero(result);

            reg(reg_dest) = result;
        }
        else
        {
            u32 result = reg(reg_source) + operand;
            u64 result_long = (u64)(reg(reg_source)) + (u64)operand;

            // Calculate flags
            set_carry(result_long & 0x100000000);
            update_overflow_add(result, reg(reg_source), operand);
            update_sign(result);
            update_zero(result);

            reg(reg_dest) = result;
        }

        // Update cycle counter
        cycles += Memory::SequentialAccess(m_State.m_reg[15], ACCESS_HWORD);
    }

    template <int op, int reg_dest>
    void arm::Thumb3(u16 instruction)
    {
        // THUMB.3 Move/compare/add/subtract immediate
        u32 immediate_value = instruction & 0xFF;

        switch (op)
        {
        case 0b00: // MOV
            update_sign(0);
            update_zero(immediate_value);
            reg(reg_dest) = immediate_value;
            break;
        case 0b01: // CMP
        {
            u32 result = reg(reg_dest) - immediate_value;
            set_carry(reg(reg_dest) >= immediate_value);
            update_overflow_sub(result, reg(reg_dest), immediate_value);
            update_sign(result);
            update_zero(result);
            break;
        }
        case 0b10: // ADD
        {
            u32 result = reg(reg_dest) + immediate_value;
            u64 result_long = (u64)(reg(reg_dest)) + (u64)immediate_value;
            set_carry(result_long & 0x100000000);
            update_overflow_add(result, reg(reg_dest), immediate_value);
            update_sign(result);
            update_zero(result);
            reg(reg_dest) = result;
            break;
        }
        case 0b11: // SUB
        {
            u32 result = reg(reg_dest) - immediate_value;
            set_carry(reg(reg_dest) >= immediate_value);
            update_overflow_sub(result, reg(reg_dest), immediate_value);
            update_sign(result);
            update_zero(result);
            reg(reg_dest) = result;
            break;
        }
        }

        // Update cycle counter
        cycles += Memory::SequentialAccess(m_State.m_reg[15], ACCESS_HWORD);
    }

    template <int op>
    void arm::Thumb4(u16 instruction)
    {
        // THUMB.4 ALU operations
        int reg_dest = instruction & 7;
        int reg_source = (instruction >> 3) & 7;

        switch (op)
        {
        case 0b0000: // AND
            reg(reg_dest) &= reg(reg_source);
            update_sign(reg(reg_dest));
            update_zero(reg(reg_dest));
            break;
        case 0b0001: // EOR
            reg(reg_dest) ^= reg(reg_source);
            update_sign(reg(reg_dest));
            update_zero(reg(reg_dest));
            break;
        case 0b0010: // LSL
        {
            u32 amount = reg(reg_source);
            bool carry = m_State.m_cpsr & MASK_CFLAG;
            LSL(reg(reg_dest), amount, carry);
            set_carry(carry);
            update_sign(reg(reg_dest));
            update_zero(reg(reg_dest));
            cycles++;
            break;
        }
        case 0b0011: // LSR
        {
            u32 amount = reg(reg_source);
            bool carry = m_State.m_cpsr & MASK_CFLAG;
            LSR(reg(reg_dest), amount, carry, false);
            set_carry(carry);
            update_sign(reg(reg_dest));
            update_zero(reg(reg_dest));
            cycles++;
            break;
        }
        case 0b0100: // ASR
        {
            u32 amount = reg(reg_source);
            bool carry = m_State.m_cpsr & MASK_CFLAG;
            ASR(reg(reg_dest), amount, carry, false);
            set_carry(carry);
            update_sign(reg(reg_dest));
            update_zero(reg(reg_dest));
            cycles++;
            break;
        }
        case 0b0101: // ADC
        {
            int carry = (m_State.m_cpsr >> 29) & 1;
            u32 result = reg(reg_dest) + reg(reg_source) + carry;
            u64 result_long = (u64)(reg(reg_dest)) + (u64)(reg(reg_source)) + (u64)carry;
            set_carry(result_long & 0x100000000);
            update_overflow_add(result, reg(reg_dest), reg(reg_source));
            update_sign(result);
            update_zero(result);
            reg(reg_dest) = result;
            break;
        }
        case 0b0110: // SBC
        {
            int carry = (m_State.m_cpsr >> 29) & 1;
            u32 result = reg(reg_dest) - reg(reg_source) + carry - 1;
            set_carry(reg(reg_dest) >= (reg(reg_source) + carry - 1));
            update_overflow_sub(result, reg(reg_dest), reg(reg_source));
            update_sign(result);
            update_zero(result);
            reg(reg_dest) = result;
            break;
        }
        case 0b0111: // ROR
        {
            u32 amount = reg(reg_source);
            bool carry = m_State.m_cpsr & MASK_CFLAG;
            ROR(reg(reg_dest), amount, carry, false);
            set_carry(carry);
            update_sign(reg(reg_dest));
            update_zero(reg(reg_dest));
            cycles++;
            break;
        }
        case 0b1000: // TST
        {
            u32 result = reg(reg_dest) & reg(reg_source);
            update_sign(result);
            update_zero(result);
            break;
        }
        case 0b1001: // NEG
        {
            u32 result = 0 - reg(reg_source);
            set_carry(0 >= reg(reg_source));
            update_overflow_sub(result, 0, reg(reg_source));
            update_sign(result);
            update_zero(result);
            reg(reg_dest) = result;
            break;
        }
        case 0b1010: // CMP
        {
            u32 result = reg(reg_dest) - reg(reg_source);
            set_carry(reg(reg_dest) >= reg(reg_source));
            update_overflow_sub(result, reg(reg_dest), reg(reg_source));
            update_sign(result);
            update_zero(result);
            break;
        }
        case 0b1011: // CMN
        {
            u32 result = reg(reg_dest) + reg(reg_source);
            u64 result_long = (u64)(reg(reg_dest)) + (u64)(reg(reg_source));
            set_carry(result_long & 0x100000000);
            update_overflow_add(result, reg(reg_dest), reg(reg_source));
            update_sign(result);
            update_zero(result);
            break;
        }
        case 0b1100: // ORR
            reg(reg_dest) |= reg(reg_source);
            update_sign(reg(reg_dest));
            update_zero(reg(reg_dest));
            break;
        case 0b1101: // MUL
            // TODO: how to calc. the internal cycles?
            reg(reg_dest) *= reg(reg_source);
            update_sign(reg(reg_dest));
            update_zero(reg(reg_dest));
            set_carry(false);
            break;
        case 0b1110: // BIC
            reg(reg_dest) &= ~(reg(reg_source));
            update_sign(reg(reg_dest));
            update_zero(reg(reg_dest));
            break;
        case 0b1111: // MVN
            reg(reg_dest) = ~(reg(reg_source));
            update_sign(reg(reg_dest));
            update_zero(reg(reg_dest));
            break;
        }

        // Update cycle counter
        cycles += Memory::SequentialAccess(m_State.m_reg[15], ACCESS_HWORD);
    }

    template <int op, bool high1, bool high2>
    void arm::Thumb5(u16 instruction)
    {
        // THUMB.5 Hi register operations/branch exchange
        int reg_dest = instruction & 7;
        int reg_source = (instruction >> 3) & 7;
        bool compare = false;
        u32 operand;

        if (high1) reg_dest += 8;
        if (high2) reg_source += 8;

        operand = reg(reg_source);
        if (reg_source == 15) operand &= ~1;

        // Time next pipeline prefetch
        cycles += Memory::SequentialAccess(m_State.m_reg[15], ACCESS_HWORD);

        // Perform the actual operation
        switch (op)
        {
        case 0: // ADD
            reg(reg_dest) += operand;
            break;
        case 1: // CMP
        {
            u32 result = reg(reg_dest) - operand;
            set_carry(reg(reg_dest) >= operand);
            update_overflow_sub(result, reg(reg_dest), operand);
            update_sign(result);
            update_zero(result);
            compare = true;
            break;
        }
        case 2: // MOV
            reg(reg_dest) = operand;
            break;
        case 3: // BX
            // Bit0 being set in the address indicates
            // that the destination instruction is in THUMB mode.
            if (operand & 1)
            {
                m_State.m_reg[15] = operand & ~1;

                // Emulate pipeline refill cycles
                cycles += Memory::NonSequentialAccess(m_State.m_reg[15], ACCESS_HWORD) +
                          Memory::SequentialAccess(m_State.m_reg[15] + 2, ACCESS_HWORD);
            }
            else
            {
                m_State.m_cpsr &= ~MASK_THUMB;
                m_State.m_reg[15] = operand & ~3;

                // Emulate pipeline refill cycles
                cycles += Memory::NonSequentialAccess(m_State.m_reg[15], ACCESS_WORD) +
                          Memory::SequentialAccess(m_State.m_reg[15] + 4, ACCESS_WORD);
            }

            // Flush pipeline
            m_Pipe.m_Flush = true;
            break;
        }

        if (reg_dest == 15 && !compare && op != 0b11)
        {
            // Flush pipeline
            reg(reg_dest) &= ~1;
            m_Pipe.m_Flush = true;

            // Emulate pipeline refill cycles
            cycles += Memory::NonSequentialAccess(m_State.m_reg[15], ACCESS_HWORD) +
                      Memory::SequentialAccess(m_State.m_reg[15] + 2, ACCESS_HWORD);
        }
    }

    template <int reg_dest>
    void arm::Thumb6(u16 instruction)
    {
        // THUMB.6 PC-relative load
        u32 immediate_value = instruction & 0xFF;
        u32 address = (m_State.m_reg[15] & ~2) + (immediate_value << 2);

        reg(reg_dest) = ReadWord(address);

        cycles += 1 + Memory::SequentialAccess(m_State.m_reg[15], ACCESS_HWORD) +
                      Memory::NonSequentialAccess(address, ACCESS_WORD);
    }

    template <int op, int reg_offset>
    void arm::Thumb7(u16 instruction)
    {
        // THUMB.7 Load/store with register offset
        // TODO: check LDR(B) timings.
        int reg_dest = instruction & 7;
        int reg_base = (instruction >> 3) & 7;
        u32 address = reg(reg_base) + reg(reg_offset);

        switch (op)
        {
        case 0b00: // STR
            WriteWord(address, reg(reg_dest));
            cycles += Memory::NonSequentialAccess(m_State.m_reg[15], ACCESS_HWORD) +
                      Memory::NonSequentialAccess(address, ACCESS_WORD);
            break;
        case 0b01: // STRB
            WriteByte(address, reg(reg_dest) & 0xFF);
            cycles += Memory::NonSequentialAccess(m_State.m_reg[15], ACCESS_HWORD) +
                      Memory::NonSequentialAccess(address, ACCESS_BYTE);
            break;
        case 0b10: // LDR
            reg(reg_dest) = ReadWordRotated(address);
            cycles += 1 + Memory::SequentialAccess(m_State.m_reg[15], ACCESS_HWORD) +
                          Memory::NonSequentialAccess(address, ACCESS_WORD);
            break;
        case 0b11: // LDRB
            reg(reg_dest) = ReadByte(address);
            cycles += 1 + Memory::SequentialAccess(m_State.m_reg[15], ACCESS_HWORD) +
                          Memory::NonSequentialAccess(address, ACCESS_BYTE);
            break;
        }
    }

    template <int op, int reg_offset>
    void arm::Thumb8(u16 instruction)
    {
        // THUMB.8 Load/store sign-extended byte/halfword
        int reg_dest = instruction & 7;
        int reg_base = (instruction >> 3) & 7;
        u32 address = reg(reg_base) + reg(reg_offset);

        switch (op)
        {
        case 0b00: // STRH
            WriteHWord(address, reg(reg_dest));
            cycles += Memory::NonSequentialAccess(m_State.m_reg[15], ACCESS_HWORD) +
                    Memory::NonSequentialAccess(address, ACCESS_HWORD);
            break;
        case 0b01: // LDSB
            reg(reg_dest) = ReadByte(address);

            if (reg(reg_dest) & 0x80)
                reg(reg_dest) |= 0xFFFFFF00;

            cycles += 1 + Memory::SequentialAccess(m_State.m_reg[15], ACCESS_HWORD) +
                        Memory::NonSequentialAccess(address, ACCESS_BYTE);
            break;
        case 0b10: // LDRH
            reg(reg_dest) = ReadHWord(address);
            cycles += 1 + Memory::SequentialAccess(m_State.m_reg[15], ACCESS_HWORD) +
                        Memory::NonSequentialAccess(address, ACCESS_HWORD);
            break;
        case 0b11: // LDSH
            reg(reg_dest) = ReadHWordSigned(address);

            // Uff... we should check wether ReadHWordSigned reads a
            // byte or a hword. However this should never really make difference.
            cycles += 1 + Memory::SequentialAccess(m_State.m_reg[15], ACCESS_HWORD) +
                        Memory::NonSequentialAccess(address, ACCESS_HWORD);
            break;
        }
    }

    template <int op, int imm>
    void arm::Thumb9(u16 instruction)
    {
        // THUMB.9 Load store with immediate offset
        int reg_dest = instruction & 7;
        int reg_base = (instruction >> 3) & 7;

        switch (op)
        {
        case 0b00: { // STR
            u32 address = reg(reg_base) + (imm << 2);
            WriteWord(address, reg(reg_dest));
            cycles += Memory::NonSequentialAccess(m_State.m_reg[15], ACCESS_HWORD) +
                      Memory::NonSequentialAccess(address, ACCESS_WORD);
            break;
        }
        case 0b01: { // LDR
            u32 address = reg(reg_base) + (imm << 2);
            reg(reg_dest) = ReadWordRotated(address);
            cycles += 1 + Memory::SequentialAccess(m_State.m_reg[15],  ACCESS_HWORD) +
                          Memory::NonSequentialAccess(address, ACCESS_WORD);
            break;
        }
        case 0b10: { // STRB
            u32 address = reg(reg_base) + imm;
            WriteByte(address, reg(reg_dest));
            cycles += Memory::NonSequentialAccess(m_State.m_reg[15], ACCESS_HWORD) +
                      Memory::NonSequentialAccess(address, ACCESS_BYTE);
            break;
        }
        case 0b11: { // LDRB
            u32 address = reg(reg_base) + imm;
            reg(reg_dest) = ReadByte(address);
            cycles += 1 + Memory::SequentialAccess(m_State.m_reg[15], ACCESS_HWORD) +
                          Memory::NonSequentialAccess(address, ACCESS_BYTE);
            break;
        }
        }
    }

    template <bool load, int imm> 
    void arm::Thumb10(u16 instruction)
    {
        // THUMB.10 Load/store halfword
        int reg_dest = instruction & 7;
        int reg_base = (instruction >> 3) & 7;
        u32 address = reg(reg_base) + (imm << 1);

        if (load)
        {
            reg(reg_dest) = ReadHWord(address); // TODO: alignment?
            cycles += 1 + Memory::SequentialAccess(m_State.m_reg[15], ACCESS_HWORD) +
                          Memory::NonSequentialAccess(address, ACCESS_WORD);
        }
        else
        {
            WriteHWord(address, reg(reg_dest));
            cycles += Memory::NonSequentialAccess(m_State.m_reg[15], ACCESS_HWORD) +
                      Memory::NonSequentialAccess(address, ACCESS_WORD);
        }
    }

    template <bool load, int reg_dest>
    void arm::Thumb11(u16 instruction)
    {
        // THUMB.11 SP-relative load/store
        u32 immediate_value = instruction & 0xFF;
        u32 address = reg(13) + (immediate_value << 2);

        // Is the load bit set? (ldr)
        if (load)
        {
            reg(reg_dest) = ReadWordRotated(address);
            cycles += 1 + Memory::SequentialAccess(m_State.m_reg[15], ACCESS_HWORD) +
                          Memory::NonSequentialAccess(address, ACCESS_WORD);
        }
        else
        {
            WriteWord(address, reg(reg_dest));
            cycles += Memory::NonSequentialAccess(m_State.m_reg[15], ACCESS_HWORD) +
                      Memory::NonSequentialAccess(address, ACCESS_WORD);
        }
    }

    template <bool stackptr, int reg_dest>
    void arm::Thumb12(u16 instruction)
    {
        // THUMB.12 Load address
        u32 immediate_value = instruction & 0xFF;

        // Use stack pointer as base?
        if (stackptr)
            reg(reg_dest) = reg(13) + (immediate_value << 2); // sp
        else
            reg(reg_dest) = (m_State.m_reg[15] & ~2) + (immediate_value << 2); // pc

        cycles += Memory::SequentialAccess(m_State.m_reg[15], ACCESS_HWORD);
    }

    template <bool sub>
    void arm::Thumb13(u16 instruction)
    {
        // THUMB.13 Add offset to stack pointer
        u32 immediate_value = (instruction & 0x7F) << 2;

        // Immediate-value is negative?
        if (sub)
            reg(13) -= immediate_value;
        else
            reg(13) += immediate_value;

        cycles += Memory::SequentialAccess(m_State.m_reg[15], ACCESS_HWORD);
    }

    template <bool pop, bool rbit>
    void arm::Thumb14(u16 instruction)
    {
        // THUMB.14 push/pop registers
        // TODO: how to handle an empty register list?
        bool first_access = true;

        // One non-sequential prefetch cycle
        cycles += Memory::NonSequentialAccess(m_State.m_reg[15], ACCESS_HWORD);

        // Is this a POP instruction?
        if (pop)
        {
            // Iterate through the entire register list
            for (int i = 0; i <= 7; i++)
            {
                // Pop into this register?
                if (instruction & (1 << i))
                {
                    u32 address = reg(13);

                    // Read word and update SP.
                    reg(i) = ReadWord(address);
                    reg(13) += 4;

                    // Time the access based on if it's a first access
                    if (first_access)
                    {
                        cycles += Memory::NonSequentialAccess(address, ACCESS_WORD);
                        first_access = false;
                    }
                    else
                    {
                        cycles += Memory::SequentialAccess(address, ACCESS_WORD);
                    }
                }
            }

            // Also pop r15/pc if neccessary
            if (rbit)
            {
                u32 address = reg(13);

                // Read word and update SP.
                m_State.m_reg[15] = ReadWord(reg(13)) & ~1;
                reg(13) += 4;

                // Time the access based on if it's a first access
                if (first_access)
                {
                    cycles += Memory::NonSequentialAccess(address, ACCESS_WORD);
                    first_access = false;
                }
                else
                {
                    cycles += Memory::SequentialAccess(address, ACCESS_WORD);
                }

                m_Pipe.m_Flush = true;
            }
        }
        else
        {
            // Push r14/lr if neccessary
            if (rbit)
            {
                u32 address;

                // Write word and update SP.
                reg(13) -= 4;
                address = reg(13);
                WriteWord(address, reg(14));

                // Time the access based on if it's a first access
                if (first_access)
                {
                    cycles += Memory::NonSequentialAccess(address, ACCESS_WORD);
                    first_access = false;
                }
                else
                {
                    cycles += Memory::SequentialAccess(address, ACCESS_WORD);
                }
            }

            // Iterate through the entire register list
            for (int i = 7; i >= 0; i--)
            {
                // Push this register?
                if (instruction & (1 << i))
                {
                    u32 address;

                    // Write word and update SP.
                    reg(13) -= 4;
                    address = reg(13);
                    WriteWord(address, reg(i));

                    // Time the access based on if it's a first access
                    if (first_access)
                    {
                        cycles += Memory::NonSequentialAccess(address, ACCESS_WORD);
                        first_access = false;
                    }
                    else
                    {
                        cycles += Memory::SequentialAccess(address, ACCESS_WORD);
                    }
                }
            }
        }
    }

    template <bool load, int reg_base>
    void arm::Thumb15(u16 instruction)
    {
        // THUMB.15 Multiple load/store
        // TODO: Handle empty register list
        bool write_back = true;
        u32 address = reg(reg_base);

        // Is the load bit set? (ldmia or stmia)
        if (load)
        {
            cycles += 1 + Memory::NonSequentialAccess(m_State.m_reg[15], ACCESS_HWORD) +
                          Memory::SequentialAccess(m_State.m_reg[15] + 2, ACCESS_HWORD);

            // Iterate through the entire register list
            for (int i = 0; i <= 7; i++)
            {
                // Load to this register?
                if (instruction & (1 << i))
                {
                    reg(i) = ReadWord(address);
                    cycles += Memory::SequentialAccess(address, ACCESS_WORD);
                    address += 4;
                }
            }

            // Write back address into the base register if specified
            // and the base register is not in the register list
            if (write_back && !(instruction & (1 << reg_base)))
                reg(reg_base) = address;
        }
        else
        {
            int first_register = 0;
            bool first_access = true;

            cycles += Memory::NonSequentialAccess(m_State.m_reg[15], ACCESS_HWORD);

            // Find the first register
            for (int i = 0; i < 8; i++)
            {
                if (instruction & (1 << i))
                {
                    first_register = i;
                    break;
                }
            }

            // Iterate through the entire register list
            for (int i = 0; i <= 7; i++)
            {
                // Store this register?
                if (instruction & (1 << i))
                {
                    // Write register to the base address. If the current register is the
                    // base register and also the first register instead the original base is written.
                    if (i == reg_base && i == first_register)
                        WriteWord(reg(reg_base), address);
                    else
                        WriteWord(reg(reg_base), reg(i));

                    // Time the access based on if it's a first access
                    if (first_access)
                    {
                        cycles += Memory::NonSequentialAccess(reg(reg_base), ACCESS_WORD);
                        first_access = false;
                    }
                    else
                    {
                        cycles += Memory::SequentialAccess(reg(reg_base), ACCESS_WORD);
                    }

                    // Update base address
                    reg(reg_base) += 4;
                }
            }
        }
    }

    template <int cond>
    void arm::Thumb16(u16 instruction)
    {
        // THUMB.16 Conditional branch
        u32 signed_immediate = instruction & 0xFF;

        cycles += Memory::SequentialAccess(m_State.m_reg[15], ACCESS_HWORD);

        // Check if the instruction will be executed
        switch (cond)
        {
        case 0x0: if (!(m_State.m_cpsr & MASK_ZFLAG))     return; break;
        case 0x1: if (  m_State.m_cpsr & MASK_ZFLAG)      return; break;
        case 0x2: if (!(m_State.m_cpsr & MASK_CFLAG))    return; break;
        case 0x3: if (  m_State.m_cpsr & MASK_CFLAG)     return; break;
        case 0x4: if (!(m_State.m_cpsr & MASK_NFLAG))     return; break;
        case 0x5: if (  m_State.m_cpsr & MASK_NFLAG)      return; break;
        case 0x6: if (!(m_State.m_cpsr & MASK_VFLAG)) return; break;
        case 0x7: if (  m_State.m_cpsr & MASK_VFLAG)  return; break;
        case 0x8: if (!(m_State.m_cpsr & MASK_CFLAG) ||  (m_State.m_cpsr & MASK_ZFLAG)) return; break;
        case 0x9: if ( (m_State.m_cpsr & MASK_CFLAG) && !(m_State.m_cpsr & MASK_ZFLAG)) return; break;
        case 0xA: if ((m_State.m_cpsr & MASK_NFLAG) != (m_State.m_cpsr & MASK_VFLAG)) return; break;
        case 0xB: if ((m_State.m_cpsr & MASK_NFLAG) == (m_State.m_cpsr & MASK_VFLAG)) return; break;
        case 0xC: if ((m_State.m_cpsr & MASK_ZFLAG) || ((m_State.m_cpsr & MASK_NFLAG) != (m_State.m_cpsr & MASK_VFLAG))) return; break;
        case 0xD: if (!(m_State.m_cpsr & MASK_ZFLAG) && ((m_State.m_cpsr & MASK_NFLAG) == (m_State.m_cpsr & MASK_VFLAG))) return; break;
        }

        // Sign-extend the immediate value if neccessary
        if (signed_immediate & 0x80)
            signed_immediate |= 0xFFFFFF00;

        // Update r15/pc and flush pipe
        m_State.m_reg[15] += (signed_immediate << 1);
        m_Pipe.m_Flush = true;

        // Emulate pipeline refill timings
        cycles += Memory::NonSequentialAccess(m_State.m_reg[15], ACCESS_HWORD) +
                  Memory::SequentialAccess(m_State.m_reg[15] + 2, ACCESS_HWORD);
    }

    void arm::Thumb17(u16 instruction)
    {
        // THUMB.17 Software Interrupt
        u8 bios_call = ReadByte(m_State.m_reg[15] - 4);

        // Log SWI to the console
        #ifdef DEBUG
        LOG(LOG_INFO, "swi 0x%x r0=0x%x, r1=0x%x, r2=0x%x, r3=0x%x, lr=0x%x, pc=0x%x (thumb)",
            bios_call, m_State.m_reg[0], m_State.m_reg[1], m_State.m_reg[2], m_State.m_reg[3], reg(14), m_State.m_reg[15]);
        #endif

        // "Useless" prefetch from r15 and pipeline refill timing.
        cycles += Memory::SequentialAccess(m_State.m_reg[15], ACCESS_HWORD) +
                Memory::NonSequentialAccess(8, ACCESS_WORD) +
                Memory::SequentialAccess(12, ACCESS_WORD);

        // Dispatch SWI, either HLE or BIOS.
        if (hle)
        {
            SWI(bios_call);
        }
        else
        {
            // Store return address in r14<svc>
            m_State.m_svc.m_r14 = m_State.m_reg[15] - 2;

            // Save program status and switch mode
            SaveRegisters();
            m_State.m_spsr[SPSR_SVC] = m_State.m_cpsr;
            m_State.m_cpsr = (m_State.m_cpsr & ~(MASK_MODE | MASK_THUMB)) | MODE_SVC | MASK_IRQD;
            LoadRegisters();

            // Jump to exception vector
            m_State.m_reg[15] = EXCPT_SWI;
            m_Pipe.m_Flush = true;
        }
    }

    void arm::Thumb18(u16 instruction)
    {
        // THUMB.18 Unconditional branch
        u32 immediate_value = (instruction & 0x3FF) << 1;

        cycles += Memory::SequentialAccess(m_State.m_reg[15], ACCESS_HWORD);

        // Sign-extend r15/pc displacement
        if (instruction & 0x400)
            immediate_value |= 0xFFFFF800;

        // Update r15/pc and flush pipe
        m_State.m_reg[15] += immediate_value;
        m_Pipe.m_Flush = true;

        //Emulate pipeline refill timings
        cycles += Memory::NonSequentialAccess(m_State.m_reg[15], ACCESS_HWORD) +
                  Memory::SequentialAccess(m_State.m_reg[15] + 2, ACCESS_HWORD);
    }

    template <bool h>
    void arm::Thumb19(u16 instruction)
    {
        // THUMB.19 Long branch with link.
        u32 immediate_value = instruction & 0x7FF;

        // Branch with link consists of two instructions.
        if (h)
        {
            u32 temp_pc = m_State.m_reg[15] - 2;
            u32 value = reg(14) + (immediate_value << 1);

            cycles += Memory::SequentialAccess(m_State.m_reg[15], ACCESS_HWORD);

            // Update r15/pc
            value &= 0x7FFFFF;
            m_State.m_reg[15] &= ~0x7FFFFF;
            m_State.m_reg[15] |= value & ~1;

            // Store return address and flush pipe.
            reg(14) = temp_pc | 1;
            m_Pipe.m_Flush = true;

            //Emulate pipeline refill timings
            cycles += Memory::NonSequentialAccess(m_State.m_reg[15], ACCESS_HWORD) +
                      Memory::SequentialAccess(m_State.m_reg[15] + 2, ACCESS_HWORD);
        }
        else
        {
            reg(14) = m_State.m_reg[15] + (immediate_value << 12);
            cycles += Memory::SequentialAccess(m_State.m_reg[15], ACCESS_HWORD);
        }
    }
}
