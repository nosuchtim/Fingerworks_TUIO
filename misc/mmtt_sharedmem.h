#ifndef MMTT_SHARED_MEM_HEADER_H
#define MMTT_SHARED_MEM_HEADER_H

#define MMTT_SHM_MAGIC_NUMBER 	0xe95df674
// Version 2 - got rid of cursors in favor of outlines
// Version 3 - not sure what the change was
#define MMTT_SHM_VERSION_NUMBER 	3
#define MMTT_CURSORS_MAX 100
#define MMTT_OUTLINES_MAX 1000
#define MMTT_POINTS_MAX 100000

typedef struct PointMem {
	float x;
	float y;
	float z;
} PointMem;

typedef struct OutlineMem {
	int region;
	int sid;
	float x;
	float y;
	float z;
	float area;
	int npoints;
	int index_of_firstpoint;
} OutlineMem;

#define BUFF_UNSET (-1)
#define NUM_BUFFS 3
typedef int buff_index;

class MMTT_SharedMemHeader
{
public:
    // Magic number to make sure we are looking at the correct memory
    // must be set to MMTT_SHM_MAGIC_NUMBER
    int							magicNumber;  

    // version number of this header, must be set to MMTT_SHM_VERSION_NUMBER
    int							version;

    int							noutlines_max; 
    int							npoints_max; 

	// These are the values that, whenever they are looked at or changed,
	// need to be locked. //////////////////////////////////////////////////
	buff_index		buff_being_constructed; //  -1, 0, 1, 2
	buff_index		buff_displayed_last_time; //  -1, 0, 1, 2
	buff_index		buff_to_display_next; //  -1, 0, 1, 2
	buff_index		buff_to_display;
	bool			buff_inuse[3];
	////////////////////////////////////////////////////////////////////////

    int							numoutlines[3]; 
    int							numpoints[3]; 

    // This offset (in bytes) is the distance from the start of the data.
	// WARNING: do not re-order these fields, the calc.* methods depend on it.
    int							outlinesOffset[3]; 
    int							pointsOffset[3]; 

	int	seqnum;
	long lastUpdateTime;  // directly from timeGetTime()
	int active;

	// END OF DATA, the rest is method declarations

	char *Data() {
		return (char*)this + sizeof(MMTT_SharedMemHeader);
	}

    OutlineMem* outline(int buffnum, int outlinenum) {
		int offset = calcOutlineOffset(buffnum,outlinenum);
		OutlineMem* om = (OutlineMem*)( Data() + offset);
		return om;
	}
    PointMem* point(int buffnum, int pointnum) {
		return (PointMem*)( Data() + calcPointOffset(buffnum,pointnum));
	}

    int	calcOutlineOffset(int buffnum, int outlinenum = 0) {
		int v1 = 0;
		int v2 = buffnum*noutlines_max*sizeof(OutlineMem);
		int v3 = outlinenum*sizeof(OutlineMem);
		return v1 + v2 + v3;
    }
    int	calcPointOffset(int buffnum, int pointnum = 0) {
		return calcOutlineOffset(0) + NUM_BUFFS*noutlines_max*sizeof(OutlineMem)
			+ buffnum*npoints_max*sizeof(PointMem)
			+ pointnum*sizeof(PointMem);
    }

	int addPoint(buff_index b, float x, float y, float z);
	int addOutline(buff_index b, int region, int sid, float x, float y, float z, float area, int npoints);

	void xinit();
	void clear_lists(buff_index b);
	void check_sanity();
	buff_index grab_unused_buffer();
};

 void print_buff_info(char *prefix, MMTT_SharedMemHeader* h);

#endif 
