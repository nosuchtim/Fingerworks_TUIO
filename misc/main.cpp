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
#include "../igesture/Igesture.h"
// #include "igesturelib.h"
#include "TuioDevice.h"
#include "xgetopt.h"
#include "tchar.h"

using namespace TUIO;

TuioDevice *device;

extern "C" {

// void _snprintf() {
// }


FILE _iob[3];

// extern "C" { FILE _iob[3] = {__iob_func()[0], __iob_func()[1], __iob_func()[2]}; }

}


int main(int argc, const char* argv[])
{
	const char *host;
	int port;
	int verbose = 0;
	int c;
	int alive_update_interval = 1000; // milliseconds
	int initial_session_id = 11000;   // session id space
	int device_multiplier =   DEVICE_MULTIPLIER;   // in session id's.  I.e. the second device starts at 12000
	bool flipx = false;
	bool flipy = false;

	while ((c = getopt(argc, argv, "vxyV:a:i:m:")) != EOF) {
		switch (c) {
			case _T('v'):
				verbose = 1;
				break;
			case _T('x'):
				flipx = true;
				break;
			case _T('y'):
				flipy = true;
				break;
			case _T('V'):
				verbose = atoi(optarg);
				break;
			case _T('a'):
				alive_update_interval = atoi(optarg);
				break;
			case _T('i'):
				initial_session_id = atoi(optarg);
				break;
			case _T('m'):
				device_multiplier = atoi(optarg);
				break;
			case _T('?'):
				std::cout << "usage: igesture_tuio [-v] [-V #] [-a #] [host] [port]\n";
				std::cout << "  -V #    Verbosity level\n";
				std::cout << "  -a #    Number of milliseconds between alive messages\n";
				std::cout << "  -i #    initial session id\n";
				std::cout << "  -m #    device session id multiplier\n";
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

	TuioServer* tuioServer = new TuioServer(host, port, verbose);
	tuioServer->flipX(flipx);
	tuioServer->flipY(flipy);
	tuioServer->setAliveUpdateInterval(alive_update_interval);
	//tuioServer->enablePeriodicMessages();

	device = new Igesture(tuioServer,initial_session_id,device_multiplier);
	if (device->tc_init()) {
		device->run();
	}
	delete(device);

	return 0;
}




