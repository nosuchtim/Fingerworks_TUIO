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

#include "Igesture.h"
// #include "TC.h"
#include "igesturelib.h"
#include "xgetopt.h"
#include "tchar.h"

Igesture *app;


extern "C" { FILE _iob[3] = {__iob_func()[0], __iob_func()[1], __iob_func()[2]}; }


void Igesture::tc_pressed(float x, float y, int uid, int id, float force) {
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

void Igesture::tc_dragged(float x, float y, int uid, int id, float force) {
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

void Igesture::tc_released(float x, float y, int uid, int id, float force) {
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

Igesture::Igesture(const char* host, int port, int v, int alive_update_interval = 10) {

	verbose = v;
	s_id = 0;
	wasupdated = 0;

	// std::cout << "IGESTURE init verbose=" << verbose << std::endl;
	memset(uid_for_id,0,sizeof(uid_for_id));

	cursor = NULL;
	tuioServer = new TuioServer(host, port, verbose);
	tuioServer->setAliveUpdateInterval(alive_update_interval);
	//tuioServer->enablePeriodicMessages();
}

void mycallback(int devnum, int fingnum, int event, float x, float y, float prox)
{
	y = 1.0 - y;
#define DEVICE_NUM_MULTIPLIER 100
	int id = devnum * DEVICE_NUM_MULTIPLIER + fingnum;
	// printf("MYCALLBACK!! fing=%d xy=%f,%f\n",fingnum,x,y);
	switch(event) {
		case FINGER_DRAG:  // UPDATE
			app->tc_dragged(x, y, id, id, prox);
			break;
		case FINGER_DOWN:  // START
			app->tc_pressed(x, y, id, id, prox);
			break;
		case FINGER_UP:  // END
			app->tc_released(x, y, id, id, prox);
			break;
	}	
	// printf("end MYCALLBACK\n");
}

int Igesture::tc_init()
{
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
	gesture_setcallback(mycallback);
	return r;
}

void Igesture::run() {
	running=true;
	while (running) {
		wasupdated = 0;
		// printf("run loop top\n");
		gesture_processframes();
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

int main(int argc, char* argv[])
{
	
	char *host;
	int port;
	int verbose = 0;
	int c;
	int alive_update_interval = 1000;   // milliseconds

	while ((c = getopt(argc, argv, _T("vV:a:"))) != EOF) {
		switch (c) {
			case _T('v'):
				verbose = 1;
				break;
			case _T('V'):
				verbose = atoi(optarg);
				break;
			case _T('a'):
				alive_update_interval = atoi(optarg);
				break;
			case _T('?'):
				std::cout << "usage: igesture_tuio [-v] [-V #] [-a #] [host] [port]\n";
				std::cout << "  -V #    Verbosity level\n";
				std::cout << "  -a #    Number of milliseconds between alive messages\n";
        		return 0;
		}
	}
	int nleft = argc - optind;
	// std::cout << "nleft = " << nleft << " verbose = " << verbose << "\n";
	if ( nleft == 0 ) {
		host = "192.168.1.102";
		host = "127.0.0.1";
		port = 3333;
		std::cout << "Assuming host=" << host << " port=" << port << "\n";
	} else if ( nleft == 2 ) {
		host = argv[optind++];
		port = atoi(argv[optind++]);
	} else {
		std::cout << "usage: igesture_tuio [-v] [-V #] [host] [port]\n";
		return 0;
	}

	app = new Igesture(host,port,verbose,alive_update_interval);
	app->tc_init();
	app->run();
	delete(app);

	return 0;
}




