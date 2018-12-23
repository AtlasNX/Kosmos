/*
 * Copyright (c) 2018 Atmosphère-NX
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#pragma once
#include <switch.h>

static inline void RebootToRcm() {
    SecmonArgs args = {0};
    args.X[0] = 0xC3000401; /* smcSetConfig */
    args.X[1] = 65001; /* Exosphere reboot to rcm */
    args.X[3] = 1; /* Perform reboot. */
    svcCallSecureMonitor(&args);
}
