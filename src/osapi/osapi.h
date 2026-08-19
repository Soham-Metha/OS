/*
 * osapi.h
 *  Copyright (C) 2026 Soham Metha
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef OSAPI_1
#define OSAPI_1

#include <common/result.h>
#include <common/types.h>

/* write 1 char */
public
void store(file_discriptor fd, uint8 c);

/* read 1 char */
public
Result8 load(file_discriptor fd);

/* restart current process later */
public
void p_yield();

/* exit current process */
public
void p_exit();

#endif
