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
#include "TuioDevice.h"

using namespace TUIO;

#define MAX_IGESTURE_FORCE 5.0
#define MAX_IGESTURE_ID 12
#define DEVICE_MULTIPLIER 1000

class Igesture : public TuioDevice { 
	
public:
	Igesture(TuioServer* server, int i, int m);
	~Igesture() {
	};
	
	void run();
	bool tc_init();
	int tc_check() { return 0; };
	void tc_pressed(float x, float y, int uid, int id, float force);
	void tc_released(float x, float y, int uid, int id, float force);
	void tc_dragged(float x, float y, int uid, int id, float force);

	int uid_for_id[MAX_IGESTURE_ID*12*DEVICE_MULTIPLIER];   // 5 devices, 12 fingers per device

private:

	static void _mycallback(int devnum, int fingnum, int event, float x, float y, float prox);
	static Igesture* _me;

	int verbose, running;
	int wasupdated;
	
	int width, height;
	int screen_width, screen_height;
	int window_width, window_height;
	TuioTime currentTime;
	int s_id;
};

#endif /* INCLUDED_Igesture_H */
