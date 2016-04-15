#ifndef TUIODEVICE_H
#define TUIODEVICE_H

#include <list>

namespace TUIO {

	class TuioDevice {

	public:
		TuioDevice() { };
		// TuioDevice(const char *host, int port, int v, int a, int i, int m, bool flipx, bool flipy);
		~TuioDevice() {
			delete tuioServer;
		};

		virtual void run() = 0;
		virtual bool tc_init() = 0;
		virtual void tc_pressed(float x, float y, int uid, int id, float force) = 0;
		virtual void tc_released(float x, float y, int uid, int id, float force) = 0;
		virtual void tc_dragged(float x, float y, int uid, int id, float force) = 0;

		int device_multiplier;
		int initial_session_id;
		TuioServer *tuioServer;
		std::list<TuioCursor*> stickyCursorList;

	private:

#if 0

		static void _mycallback(int devnum, int fingnum, int event, float x, float y, float prox);
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
#endif

	};
};

#endif
