#ifdef __cplusplus
extern "C" {
#endif

#define GESTURE_MAX_DEVICES 10

// This is the maximum number of fingers PER DEVICE
#define GESTURE_MAX_FINGERS 10

struct fingdevice_s;

typedef enum fingereventtype_e {
	FINGER_DOWN = 0,
	FINGER_UP = 1,
	FINGER_DRAG = 2
} fingereventtype;

typedef struct finger_s {
			int isdown;
			float xpos;
			float ypos;
			int hand_id;
			float xvel;
			float yvel;
			float proximity;
			float orientation;
			float eccentricity;
			int devnum;
			int fingnum;
			long up_timeout;
} finger_t;

typedef struct fingdevice_s {
	float width;
	float height;
	float xoff;
	float yoff;
	char *name;
	finger_t finger[GESTURE_MAX_FINGERS];
	char touched[GESTURE_MAX_FINGERS];
	int processed;
} fingdevice_t;

typedef void (*FingerCallback)(int devnum, int fingnum, int event, float x, float y, float prox);

extern FingerCallback fingercallback;

extern int Nfingdevices;
extern fingdevice_t Fingdevice[GESTURE_MAX_DEVICES];

bool gesture_init(HWND hwnd);
int gesture_setcallback(FingerCallback f);
int gesture_processframes();

#ifdef __cplusplus
}
#endif
