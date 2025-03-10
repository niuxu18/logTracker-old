/*
 * Test harness for unsafe_symlink().  Not linked into rsync itself.
 *
 * Copyright (C) 2002 Martin Pool
 * Copyright (C) 2003 Wayne Davison
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.
 */

/* Prints either "safe" or "unsafe" depending on the two arguments.
 * Always returns 0 unless something extraordinary happens. */

#include "rsync.h"

int dry_run, read_only, list_only, verbose;
int preserve_perms = 0;

int
main(int argc, char **argv)
{
	if (argc != 3) {
		fprintf(stderr, "usage: t_unsafe LINKDEST SRCDIR\n");
