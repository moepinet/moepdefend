/*
 This file is part of moep80211
 (C) 2011 2012 Stephan M. Guenther (and other contributing authors)

 moep80211 is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published
 by the Free Software Foundation; either version 3, or (at your
 option) any later version.

 moep80211 is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with moep80211; see the file COPYING. If not, write to the
 Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 Boston, MA 02111-1307, USA.
 */


/**
 * @file common/daemonize.h
 * @brief Offers a function to run process as daemon
 * @author Stephan M. Guenther
 */


#ifndef _DAEMONIZE_H_
#define _DAEMONIZE_H_


/**
 * Forks a process into background and kills the parent
 * @param lockfile File system path to a lock file; this way we make sure that a
 * daemon can be started only once.
 */
void
daemonize();


#endif // _DAEMONIZE_H_

