/*
	TUIO C++ Server Demo - part of the reacTIVision project
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

#include "TuioServer.h"
#include "TuioCursor.h"
#include "Morph.h"
#include "xgetopt.h"
#include "tchar.h"

using namespace TUIO;

void Morph::tc_pressed(float x, float y, int uid, int id, float force) {
	// printf("clicked %f %f   uid=%d id=%d\n",x,y,uid,id);

	uid = uid_for_id[id];
	if ( uid == 0 ) {
		uid = ++s_id;
		uid_for_id[id] = uid;
	}
	TuioCursor *c = tuioServer->addTuioCursorId(x,y,uid,id);
	c->setForce(force / MAX_IGESTURE_FORCE);

	std::list<TuioCursor*> cursorList = tuioServer->getTuioCursors();
	wasupdated++;
}

void Morph::tc_dragged(float x, float y, int uid, int id, float force) {
	// printf("dragged %f %f   uid=%d id=%d\n",x,y,uid,id);
	TuioCursor *match = NULL;
	uid = uid_for_id[id];
	if ( uid == 0 ) {
		printf("UNEXPECTED!  uid==0 in tc_dragged!?\n");
		uid = ++s_id;
		uid_for_id[id] = uid;
	}
	std::list<TuioCursor*> cursorList = tuioServer->getTuioCursors();
	for (std::list<TuioCursor*>::iterator tuioCursor = cursorList.begin(); tuioCursor!=cursorList.end(); tuioCursor++) {
		if (((*tuioCursor)->getCursorID()) == (id)) {
			// printf("tc_dragged found match of cursor id=%d !!\n",id);
			match = (*tuioCursor);
			break;
		}
	}
	if ( match == NULL ) {
		printf("Hey, didn't find existing cursor with id=%d !?\n",id);
	} else {
		// if (cursor->getTuioTime()==currentTime) return;
		tuioServer->updateTuioCursor(match,x,y);
		// printf("UPDATING TuioCursor with xy=%f,%f\n",x,y);
		match->setForce(force/ MAX_IGESTURE_FORCE);
		wasupdated++;
	}
}

void Morph::tc_released(float x, float y, int uid, int id, float force) {
	// printf("released  uid=%d id=%d\n",uid,id);
	uid_for_id[id] = 0;
	std::list<TuioCursor*> cursorList = tuioServer->getTuioCursors();
	TuioCursor *match = NULL;
	for (std::list<TuioCursor*>::iterator tuioCursor = cursorList.begin(); tuioCursor!=cursorList.end(); tuioCursor++) {
		if (((*tuioCursor)->getCursorID()) == (id)) {
			// printf("tc_released found cursor id=%d !!\n",id);
			match = (*tuioCursor);
			break;
		}
	}
	if (match!=NULL) {
		tuioServer->removeTuioCursor(match);
		wasupdated++;
	}
	if ((int)(tuioServer->getTuioCursors()).size() == 0) {
		// printf("Should be resetting sid?\n");
		s_id = 0;
	}
}

Morph::Morph(const char* host, int port, int v, int a, int i, int m, bool flipx, bool flipy) {

	verbose = v;
	s_id = 0;
	wasupdated = 0;
	initial_session_id = i;
	device_multiplier = m;
	_me = this;

	// std::cout << "IGESTURE init verbose=" << verbose << std::endl;
	memset(uid_for_id,0,sizeof(uid_for_id));

	tuioServer = new TuioServer(host, port, verbose);
	tuioServer->flipX(flipx);
	tuioServer->flipY(flipy);
	tuioServer->setAliveUpdateInterval(a);
	//tuioServer->enablePeriodicMessages();
}

Morph* Morph::_me = 0;

void Morph::_mycallback(int devnum, int fingnum, int event, float x, float y, float prox)
{
#ifdef WAIT_TILL_I_HAVE_A_MORPH
	// We want the first device (whatever its devnum is) to start with the initial_session_id,
	// so, we create a map of devnum to the initial_session_id for that devnum
	static int devnum_initial_session_id[GESTURE_MAX_DEVICES];
	static bool initialized = false;
	static int next_initial_session_id;

	if (!initialized) {
		for (int i = 0; i < GESTURE_MAX_DEVICES; i++) {
			devnum_initial_session_id[i] = -1;
		}
		next_initial_session_id = _me->initial_session_id;
		initialized = true;
	}

	if (devnum_initial_session_id[devnum] < 0) {
		devnum_initial_session_id[devnum] = next_initial_session_id;
		next_initial_session_id += _me->device_multiplier;
	}

	y = 1.0 - y;
	int id = devnum_initial_session_id[devnum] + fingnum;
	// printf("MYCALLBACK!! fing=%d xy=%f,%f\n",fingnum,x,y);
	switch(event) {
		case FINGER_DRAG:  // UPDATE
			_me->tc_dragged(x, y, id, id, prox);
			break;
		case FINGER_DOWN:  // START
			_me->tc_pressed(x, y, id, id, prox);
			break;
		case FINGER_UP:  // END
			_me->tc_released(x, y, id, id, prox);
			break;
	}	
	// printf("end MYCALLBACK\n");
#endif
}

bool
Morph::tc_init()
{
#ifdef WAIT_TILL_I_HAVE_A_MORPH
	static TCHAR szAppName[] = TEXT("Simple Window");
	HWND hwnd;
	MSG msg;
	WNDCLASS wndclass;
	HINSTANCE hInstance = GetModuleHandle(NULL);

	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = DefWindowProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;

	wndclass.lpszClassName = szAppName;

	if (!RegisterClass(&wndclass)) {
		MessageBox(NULL, TEXT("This program requires Windows NT!"), szAppName, MB_ICONERROR);
		return 0;
	}

	hwnd = CreateWindow(szAppName,
		TEXT("Simple Application Window"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		hInstance,
		NULL);

	int r = gesture_init(hwnd);
	gesture_setcallback(Morph::_mycallback);
	return r;
#endif
	return true;
}

void Morph::run() {
	running=true;
	while (running) {
		wasupdated = 0;
		// printf("run loop top\n");
		// morph_processframes();   // EVENTUALLY
		// if ( wasupdated > 0 ) {
		if ( 1 ) {
			// printf("run loop WASUPDATED\n");
			tuioServer->initFrame();
			// tc_check();
			tuioServer->commitFrame();
		}
		// printf("run loop pre sleep\n");
		Sleep(1);
		// printf("run loop post sleep\n");
	} 
}
