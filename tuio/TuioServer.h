/*
 TUIO Server Component - part of the reacTIVision project
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

#ifndef INCLUDED_TuioServer_H
#define INCLUDED_TuioServer_H

#ifndef WIN32
#include <pthread.h>
#include <sys/time.h>
#else
#include <windows.h>
#endif

#include <iostream>
#include <list>
#include <algorithm>

#include "osc/OscOutboundPacketStream.h"
#include "ip/NetworkingUtils.h"
#include "ip/UdpSocket.h"

#include "TuioCursor.h"

#define IP_MTU_SIZE 1500
#define MAX_UDP_SIZE 65536
#define MIN_UDP_SIZE 576
#define OBJ_MESSAGE_SIZE 108	// setMessage + seqMessage size
#define CUR_MESSAGE_SIZE 88

namespace TUIO {
	/**
	 * <p>The TuioServer class is the central TUIO protocol encoder component.
	 * In order to encode and send TUIO messages an instance of TuioServer needs to be created. The TuioServer instance then generates TUIO messaged
	 * which are sent via OSC over UDP to the configured IP address and port.</p> 
	 * <p>During runtime the each frame is marked with the initFrame and commitFrame methods, 
	 * while the currently present TuioObjects are managed by the server with ADD, UPDATE and REMOVE methods in analogy to the TuioClient's TuioListener interface.</p> 
	 * <p><code>
	 * TuioClient *server = new TuioServer();<br/>
	 * ...<br/>
	 * server->initFrame(TuioTime::getSessionTime());<br/>
	 * TuioObject *tobj = server->addTuioObject(xpos,ypos, angle);<br/>
	 * TuioCursor *tcur = server->addTuioObject(xpos,ypos);<br/>
	 * server->commitFrame();<br/>
	 * ...<br/>
	 * server->initFrame(TuioTime::getSessionTime());<br/>
	 * server->updateTuioObject(tobj, xpos,ypos, angle);<br/>
	 * server->updateTuioCursor(tcur, xpos,ypos);<br/>
	 * server->commitFrame();<br/>
	 * ...<br/>
	 * server->initFrame(TuioTime::getSessionTime());<br/>
	 * server->removeTuioObject(tobj);<br/>
	 * server->removeTuioCursor(tcur);<br/>
	 * server->commitFrame();<br/>
	 * </code></p>
	 *
	 * @author Martin Kaltenbrunner
	 * @version 1.4
	 */ 

	class TuioServer { 
		
	public:

		TuioServer();

		/**
		 * The destructor is doing nothing in particular. 
		 */
		~TuioServer();
		
		/**
		 * Creates a new TuioCursor based on the given arguments.
		 * The new TuioCursor is added to the TuioServer's internal list of active TuioCursors 
		 * and a reference is returned to the caller.
		 *
		 * @param	xp	the X coordinate to assign
		 * @param	yp	the Y coordinate to assign
		 * @return	reference to the created TuioCursor
		 */
		TuioCursor* addTuioCursor(float xp, float yp);
		TuioCursor* TuioServer::addTuioCursorId(float x, float y, int uid, int id);

		/**
		 * Updates the referenced TuioCursor based on the given arguments.
		 *
		 * @param	tcur	the TuioCursor to update
		 * @param	xp	the X coordinate to assign
		 * @param	yp	the Y coordinate to assign
		 */
		void updateTuioCursor(TuioCursor *tcur, float xp, float yp);

		/**
		 * Removes the referenced TuioCursor from the TuioServer's internal list of TuioCursors
		 * and deletes the referenced TuioCursor afterwards
		 *
		 * @param	tcur	the TuioCursor to remove
		 */
		void removeTuioCursor(TuioCursor *tcur);

		void addExternalTuioCursor(TuioCursor *tcur);

#if 0
		/**
		 * Updates an externally managed TuioCursor 
		 *
		 * @param	tcur	the TuioCursor to update
		 */
		void updateExternalTuioCursor(TuioCursor *tcur);
#endif

		/**
		 * Removes an externally managed TuioCursor from the TuioServer's internal list of TuioCursor
		 * The referenced TuioCursor is not deleted
		 *
		 * @param	tcur	the TuioCursor to remove
		 */
		void removeExternalTuioCursor(TuioCursor *tcur);
		
		/**
		 * Returns the next available Session ID for external use.
		 * @return	the next available Session ID for external use
		 */
		long getSessionID();

		/**
		 * Returns a List of all currently active TuioCursors
		 *
		 * @return  a List of all currently active TuioCursors
		 */
		std::list<TuioCursor*> getTuioCursors();
		
		/**
		 * Returns the TuioCursor corresponding to the provided Session ID
		 * or NULL if the Session ID does not refer to an active TuioCursor
		 *
		 * @return  an active TuioCursor corresponding to the provided Session ID or NULL
		 */
		TuioCursor* getTuioCursor(long s_id);

		/**
		 * Returns the TuioCursor closest to the provided coordinates
		 * or NULL if there isn't any active TuioCursor
		 *
		 * @return  the closest TuioCursor corresponding to the provided coordinates or NULL
		 */
		TuioCursor* getClosestTuioCursor(float xp, float yp);
		
		/**
		 * Returns true if this TuioServer is currently connected.
		 * @return	true if this TuioServer is currently connected
		 */
		bool isConnected() { return connected; }

#if 0
		bool flipX() { return flipx; }
		bool flipY() { return flipy; }
		bool flipX(bool b) { flipx = b; return flipx; }
		bool flipY(bool b) { flipy = b; return flipy;}
#endif

		int verbose;
		int initial_session_id;
		int device_multiplier;
		bool flipX;
		bool flipY;
		
		virtual void update() { }

	protected:
		std::list<TuioCursor*> cursorList;
		long sessionID;
		int maxCursorID;
		bool updateCursor;
		long lastCursorUpdate;
		bool connected;

	private:
		std::list<TuioCursor*> freeCursorList;
		std::list<TuioCursor*> freeCursorBuffer;
		
	};

	class TuioUdpServer : public TuioServer {
	public:
		TuioUdpServer(const char *host, int port, int alive_interval);
		~TuioUdpServer();

		void setAliveUpdateInterval(int milli) {
			alive_update_interval = milli;
		}
		void initialize(const char *host, int port);
		void sendFullMessages();
		void enablePeriodicMessages(int interval = 1);
		void disablePeriodicMessages();
		bool periodicMessagesEnabled() {
			return periodic_update;
		}
		int getUpdateInterval() {
			return update_interval;
		}
		void sendEmptyCursorBundle();
		void startCursorBundle();
		void addCursorMessage(TuioCursor *tcur);
		void sendCursorBundle(long fseq);

		void update() {
			initFrame();
			// check?
			commitFrame();
		}

	private:

		void initFrame();
		void commitFrame();

		long getFrameID();
		TuioTime getFrameTime();

		int alive_update_interval;
		bool periodic_update;
		int update_interval;

		UdpTransmitSocket *socket;	
		osc::OutboundPacketStream  *oscPacket;
		char *oscBuffer; 
		osc::OutboundPacketStream  *fullPacket;
		char *fullBuffer; 
		long currentFrame;

#ifndef WIN32
		pthread_t thread;
#else
		HANDLE thread;
#endif	
		
	};

};

#endif /* INCLUDED_TuioServer_H */
