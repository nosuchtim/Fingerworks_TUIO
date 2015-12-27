/*
 TUIO Demo - part of the reacTIVision project
 http://reactivision.sourceforge.net/
 
 Copyright (c) 2005-2009 Martin Kaltenbrunner <mkalten@iua.upf.edu>
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef INCLUDED_Igesture_H
#define INCLUDED_Igesture_H

#include "TuioServer.h"
#include "TuioCursor.h"
#include <list>
#include <math.h>

using namespace TUIO;

#define MAX_IGESTURE_FORCE 5.0
#define MAX_IGESTURE_ID 12

class Igesture { 
	
public:
	Igesture(const char *host, int port, int v, int a);
	~Igesture() {
		delete tuioServer;
	};
	
	void run();
	int tc_init();
	int tc_check();
	int tc_close();
	TuioServer *tuioServer;
	TuioCursor *cursor;
	std::list<TuioCursor*> stickyCursorList;
	// std::list<TuioCursor*> tc_CursorList;

	int uid_for_id[MAX_IGESTURE_ID*12];   // 5 devices, 12 fingers per device

	void tc_pressed(float x, float y, int uid, int id, float force);
	void tc_released(float x, float y, int uid, int id, float force);
	void tc_dragged(float x, float y, int uid, int id, float force);

private:
	int verbose, running;
	int wasupdated;
	
	int width, height;
	int screen_width, screen_height;
	int window_width, window_height;
	TuioTime currentTime;
	
	void mousePressed(float x, float y);
	void mouseReleased(float x, float y);
	void mouseDragged(float x, float y);
	int s_id;
};

#endif /* INCLUDED_Igesture_H */
