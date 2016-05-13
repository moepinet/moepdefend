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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <syslog.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "daemonize.h"
#include <moepcommon/util.h>


// Similar to Linux Daemon Writing HOWTO by David Watson, v1.0, May 2004
void
daemonize() {
	pid_t pid, sid;

	// Fork and exit the parent if successful
	if (0 > (pid = fork())) {
		fprintf(stderr,"failed to daemonize\n\n");
		exit(-1);
	}
	else if (pid > 0) {
		exit(0);
	}

	// Change file creation mode mask
	(void) umask(0);

	// Open syslog
	openlog("moep80211ncm",LOG_PID,LOG_USER);
	setlogmask(LOG_UPTO(LOG_DEBUG));
	
	// Create a new session
	if (0 > (sid = setsid())) {
		LOG(LOG_ERR,"setsid() failed");
		exit(-1);
	}

	// Change directory to something safe
	if (0 > chdir("/tmp/")) {
		LOG(LOG_ERR,"failed to change directory to /tmp/");
		exit(-1);
	}

	// Close file descriptors
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	
	LOG(LOG_INFO,"daemonized");
}

