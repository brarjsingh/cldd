/**
 * Copyright (C) 2010 Control, Logging, and Data Acquisition Daemon
 * http://www.gitorious.org/cld/cldd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

public class Cldd.Daemon : GLib.Object {

    public signal void closed ();

    public Daemon () {}

    public int init () {
        return 0;
    }

    public int interrupt () {
        return 0;
    }

    public void launch () {
    }

    public void close () {
        closed ();
    }

    public void pid_file_remove () {
    }
}
