#include <windows.h>
#include "FWHID_MultiTouchDevice.h"
#include "igesturelib.h"

int Nfingdevices = 0;
fingdevice_t Fingdevice[GESTURE_MAX_DEVICES];
FWHID_ContactFrame ContactFrame[GESTURE_MAX_DEVICES];
FWMultiTouchDevicePtr gDevice[GESTURE_MAX_DEVICES];
int gInitialized = 0;
HANDLE gMutex;
HWND gKhwnd;

static int Tcount = 0;

static int
gesture_lockframedata(char *s)
{
	DWORD dwWaitResult;
	dwWaitResult = WaitForSingleObject(gMutex,5000);
	switch(dwWaitResult){
		case WAIT_OBJECT_0:
			return TRUE;
		case WAIT_TIMEOUT:
		default:
			return FALSE;
	}
}

static void
gesture_unlockframedata(void)
{
	ReleaseMutex(gMutex);
}

static FingerCallback Fcallback = NULL;

int
gesture_setcallback(FingerCallback f)
{
	Fcallback = f;
	return 0;
}

//This is called after every frame
static void
DeviceFrameCompleteCallback(FWMultiTouchDevicePtr fwdevice, void *targetData)
{
	//Find which MultiTouch device this is
	int i=0;
	int locked;

	while(fwdevice != gDevice[i] && i < Nfingdevices)
		i++;

	if(i == Nfingdevices)
		return; //Device not found in tracking list

	locked = gesture_lockframedata("framecallback");

	FWHID_getContactFrame(gDevice[i],&(ContactFrame[i]));
	Fingdevice[i].processed = 0;

	if ( locked )
		gesture_unlockframedata();

}

//The Device Creation Callback is called for every FW device found in the system
static void
DeviceCreationCallback(FWMultiTouchDevicePtr dev, void *targetData)
{
	FWSurfaceRectangle r;
	char *s;

	if(Nfingdevices < GESTURE_MAX_DEVICES){
		FWHID_disableStreams(dev,kFW_DefaultStreams);
		FWHID_enableStreams(dev,kFW_ContactPathStream);
		gDevice[Nfingdevices] = dev;

		r = FWHID_getSurfaceDimensions(dev);
		Fingdevice[Nfingdevices].width = (float)r.width_cm;
		Fingdevice[Nfingdevices].height = (float)r.height_cm;
		Fingdevice[Nfingdevices].xoff = 0.0f;
		Fingdevice[Nfingdevices].yoff = 0.0f;

		s = FWHID_getProductName(dev);
		if ( s == NULL )
			s = "NullReturnFromGetProductName";
		Fingdevice[Nfingdevices].name = s;

		FWHID_setContactFrameCallback(
			gDevice[Nfingdevices],DeviceFrameCompleteCallback);


		Nfingdevices++;
	}
}

//Called everytime a FW device is removed.
//Note: If a device is plugged in all devices are removed and then rediscovered
void
DeviceDisposalCallback(FWMultiTouchDevicePtr fwdevice, void *targetData)
{
	int i=0;

	while(fwdevice != gDevice[i] && i < Nfingdevices)
		i++;

	if(i == Nfingdevices)
		return; //device removed was not found in devices

	FWHID_disableStreams(fwdevice,kFW_ContactPathStream);
	FWHID_enableStreams(fwdevice,kFW_DefaultStreams);

	if(i+1 < GESTURE_MAX_DEVICES)
		memmove(&gDevice[i],&gDevice[i+1],
			sizeof(FWMultiTouchDevicePtr)*(Nfingdevices-i+1));

	Nfingdevices--;
}

int
gesture_init(HWND hwnd)
{
	// try this
	int dev;
	int fi;

	gMutex = CreateMutex(NULL, FALSE, NULL);
	if ( gMutex == NULL ) {
		return FALSE;
	}
	Nfingdevices = 0;
	memset(Fingdevice,0,sizeof(Fingdevice));

	for ( dev=0; dev<GESTURE_MAX_DEVICES; dev++ ) {
		for ( fi=0; fi<GESTURE_MAX_FINGERS; fi++) {
			Fingdevice[dev].finger[fi].fingnum = fi;
		}
		Fingdevice[dev].processed = 1;
	}
	FWHID_setDeviceCreationCallback(DeviceCreationCallback);
	FWHID_setDeviceDisposalCallback(DeviceDisposalCallback);
	FWHID_registerMultiTouchDevices((HWND)hwnd);
	gInitialized = 1;
	gKhwnd = (HWND)hwnd;
	return TRUE;
}

void
gesture_end(void)
{
	if ( gInitialized ) {
		gInitialized = 0;
		FWHID_releaseMultiTouchDevices();
	}
}

int
Fingdevices()
{
	return Nfingdevices;
}

void
oldgesture_devinfo(int n, const char **surface, double *width, double *height)
{
	FWMultiTouchDevicePtr gd;
	FWSurfaceVersion v;
	FWSurfaceRectangle r;

	if ( n >= Nfingdevices )
		return;
	gd = gDevice[n];

	r = FWHID_getSurfaceDimensions(gd);
	*width = r.width_cm;
	*height = r.height_cm;

	v = FWHID_getSurfaceVersion(gd);
	switch(v){
	case kFW_iGesturePad_Surface:
		*surface = "iGesturePad";
		break;
	default:
		*surface = "unknown";
		break;
	}
}

#define ignore_contact(cp) (((cp)->finger_id==0)||((cp)->proximity<0.001))

// NOTE: we assume the lock is held when this function is called
static void
gesture_processframe(int dev)
{
	int ci;
	int fi;
	fingdevice_t *devp;
	FWHID_Contact *cp;
	finger_t *fp;
	DWORD tm = timeGetTime();
	FWHID_ContactFrame *cframep;
	int locked;

	locked = gesture_lockframedata("processframes");

	cframep = &ContactFrame[dev];

	devp = &(Fingdevice[dev]);
	if ( cframep == NULL || devp->processed ) {
		if ( locked )
			gesture_unlockframedata();
		goto dotimeouts;
	}
	devp->processed = 1;

	memset(devp->touched,0,GESTURE_MAX_FINGERS);

	for ( fi=0; fi<GESTURE_MAX_FINGERS; fi++) {
		int ncontacts = cframep->contact_count;
		for ( ci=0; ci<ncontacts; ci++ ) {

			cp = &(cframep->contacts[ci]);
			if ( ignore_contact(cp) )
				continue;
			if ( (cp->finger_id-1) == fi ) {
				fp = &(devp->finger[fi]);
				fp->fingnum = fi;
				cp->xpos += devp->xoff;
				cp->ypos += devp->yoff;
				if ( cp->xpos < 0.0 ) {
						devp->xoff += (-cp->xpos);
						cp->xpos = 0.0;
				} else if ( cp->xpos > devp->width ) {
						devp->xoff += (devp->width - cp->xpos);
						cp->xpos = devp->width;
				}
				if ( cp->ypos < 0.0 ) {
						devp->yoff += (-cp->ypos);
						cp->ypos = 0.0;
				} else if ( cp->ypos > devp->height ) {
						devp->yoff += (devp->height - cp->ypos);
						cp->ypos = devp->height;
				}
				fp->xpos = (float)(cp->xpos / devp->width);
				fp->ypos = (float)(cp->ypos / devp->height);
				// printf("fp num=%d xy=%f %f\n",fp->fingnum,fp->xpos,fp->ypos);
				fp->hand_id = cp->hand_id;
				fp->xvel = (float)cp->xvel;
				fp->yvel = (float)cp->yvel;
				fp->proximity = (float)cp->proximity;
				fp->orientation = (float)cp->orientation;
				fp->eccentricity = (float)cp->eccentricity;
				fp->devnum = dev;
				devp->touched[fi] = 1;
			}
		}
	}

	if ( locked )
		gesture_unlockframedata();

	// SHOULD LOCK FINGER DATA?  processframes is mutexed, though

	for ( fi=0; fi<GESTURE_MAX_FINGERS; fi++) {
		fp = &(devp->finger[fi]);
		if ( devp->touched[fi] == 0 ) {
			if ( fp->isdown ) {
				// SHOULDN'T DO THIS RIGHT AWAY!
				// WAIT FOR N MILLISECONDS BEFORE
				// CONCLUDING IT'S UP.
				if ( fp->up_timeout == 0 ) {
#define UP_TIMEOUT 100
					fp->up_timeout = tm+UP_TIMEOUT;
				}
			}
		} else {
			if ( fp->isdown ) {
				fp->up_timeout = 0;
				if ( Fcallback ) {
				    (Fcallback)(fp->devnum,fp->fingnum,FINGER_DRAG,fp->xpos,fp->ypos,fp->proximity);
				}
			} else {
				fp->isdown = 1;
				fp->up_timeout = 0;
				if ( Fcallback ) {
					(Fcallback)(fp->devnum,fp->fingnum,FINGER_DOWN,fp->xpos,fp->ypos,fp->proximity);
				}
			}
		}
	}
dotimeouts:
	// See if any of the timeout ones are due
	// NOTE: might not want to do this all the time?
	for ( fi=0; fi<GESTURE_MAX_FINGERS; fi++) {
		fp = &(devp->finger[fi]);
		if ( fp->isdown
			&& (fp->up_timeout > 0)
			&& (fp->up_timeout < tm )) {

			fp->isdown = 0;
			if ( Fcallback ) {
				(Fcallback)(fp->devnum,fp->fingnum,FINGER_UP,fp->xpos,fp->ypos,fp->proximity);
			}
		}
	}
}

int processframenested = 0;

int
gesture_processframes()
{
	int i;

	// SHOULD LOCK AROUND PROCESSFRAMENESTED ACCESS?

	if ( processframenested > 0 ) {
		// printf("PROCESSFRAMENESTED = %d!!!\n",processframenested);
		return 0;
	}
	processframenested++;

	for ( i=0; i < Nfingdevices; i++ ) {
		gesture_processframe(i);
	}

	processframenested--;

	return 0;
}
