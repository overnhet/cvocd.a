////////////////////////////////////////////////////////////
//
// MINI MIDI CV
//
// GLOBAL DEFINITIONS
//
////////////////////////////////////////////////////////////

//
// MACRO DEFS
//
#define P_LED1		lata.2
#define P_LED2		latc.2

#define P_SRDAT1	lata.0
#define P_SRDAT2	lata.1
#define P_SRCLK		lata.4
#define P_SRLAT		lata.5

#define TRIS_A		0b11001000
#define TRIS_C		0b11111011

#define CV_MAX		4
#define GATE_MAX	12

#define DEFAULT_GATE_NOTE 60
#define DEFAULT_GATE_CC 1
#define DEFAULT_GATE_CC_THRESHOLD 64
#define DEFAULT_GATE_DIV	6
#define DEFAULT_GATE_DURATION 10
#define DEFAULT_ACCENT_VELOCITY 127
#define DEFAULT_MIDI_CHANNEL 0
#define DEFAULT_CV_BPM_MAX_VOLTS 5
#define DEFAULT_CV_CC_MAX_VOLTS 5
#define DEFAULT_CV_PB_MAX_VOLTS 5
#define DEFAULT_CV_VEL_MAX_VOLTS 5
#define DEFAULT_CV_AFTERTOUCH_MAX_VOLTS 5
#define VEL_ACCENT 100

#define LED_PULSE_MIDI_IN 2
#define LED_PULSE_MIDI_TICK 10
#define LED_PULSE_MIDI_BEAT 100
#define LED_PULSE_PARAM 255

#define MIDI_SYNCH_TICK     	0xf8
#define MIDI_SYNCH_START    	0xfa
#define MIDI_SYNCH_CONTINUE 	0xfb
#define MIDI_SYNCH_STOP     	0xfc
#define MIDI_SYSEX_BEGIN     	0xf0
#define MIDI_SYSEX_END     		0xf7

#define MIDI_CC_NRPN_HI 		99
#define MIDI_CC_NRPN_LO 		98
#define MIDI_CC_DATA_HI 		6
#define MIDI_CC_DATA_LO 		38

#define SZ_NOTE_STACK 5	// max notes in a single stack
#define NUM_NOTE_STACKS 4	// number of stacks supported
#define NO_NOTE_OUT 0xFF 	// special "no note" value

#define I2C_TX_BUF_SZ 12

// Sysex ID
#define MY_SYSEX_ID0	0x00
#define MY_SYSEX_ID1	0x7f
#define MY_SYSEX_ID2	0x12

// Utility macros to flash an LED
#define LED_1_PULSE(ms) { P_LED1 = 1; g_led_1_timeout = ms; }
#define LED_2_PULSE(ms) { P_LED2 = 1; g_led_2_timeout = ms; }

// Check if MIDI channel mychan matches chan - taking into account GLOBAL and OMNI modes
#define IS_CHAN(mychan, chan) (((chan) == (mychan)) || (CHAN_OMNI == (mychan)) || \
 ((CHAN_GLOBAL == (mychan)) && (g_chan == (chan))))


// Check if a note matches a min-max range. If max==0 then it must exactly equal min
#define IS_NOTE_MATCH(mymin, mymax, note) (!(mymax)?((note)==(mymin)):((note)>=(mymin) && (note)<=(mymax)))

//
// TYPE DEFS
//
typedef unsigned char byte;

enum {
	GATE_DUR_INFINITE = 0x00,
	GATE_DUR_GLOBAL = 0x80
};


// special channels
enum {
	CHAN_OMNI = 0x80,
	CHAN_GLOBAL = 0x81,
	CHAN_DISABLE = 0xFF
};



// Events from note stack
enum {
	EV_NOTE_A = 1,
	EV_NOTE_B,
	EV_NOTE_C,
	EV_NOTE_D,
	EV_NO_NOTE_A,
	EV_NO_NOTE_B,
	EV_NO_NOTE_C,
	EV_NO_NOTE_D,
	EV_NOTES_OFF,
	EV_NOTE_ON,
	EV_BEND,
	EV_AFTERTOUCH
};

// note stack note priority orders
enum {
	PRIORITY_OFF			= 0,	// single note only
	PRIORITY_NEW			= 1,	// gives priority to newest note
	PRIORITY_LOW			= 2,	// gives priority to lowest note
	PRIORITY_LOW_SPREAD		= 3,	// lowest note priority with highest pitch note on output 4
	PRIORITY_HIGH			= 4,	// gives priority to highest note
	PRIORITY_HIGH_SPREAD	= 5,	// highest note priority with lowest pitch note on output 4
	PRIORITY_OLD			= 6,	// gives priority to oldest note
	PRIORITY_RANDOM			= 7,	// randomly prioritises notes	
	PRIORITY_PLAYORDER		= 8,	// work as queue 
	PRIORITY_MAX			= 9
};

/*
idea
2 notes - 
lowest mapped to synth A
highest mapped to synth B
when highest note is released, synth B must not drop to low note
when lowest note is release, synth A must not rise to high note
need a split point

*/


/*
+---+---+---+---+	+---+---+---+---+
|CV0|CV1|GT0|GT1|	|GT4|GT5|GT6|GT7|
+---+---+---+---+	+---+---+---+---+
|CV2|CV3|GT2|GT3|	|GT8|GT9|G10|G11|
+---+---+---+---+	+---+---+---+---+
*/


/////////////////////////////////////////////////////////////////////////////
// NRPN VALUES FOR CONFIGURATION

// Parameter Number High Byte 
enum {
	// global settings
	NRPNH_GLOBAL 	= 1,	
	// note stacks
	NRPNH_STACK1 	= 11,
	NRPNH_STACK2 	= 12,
	NRPNH_STACK3 	= 13,
	NRPNH_STACK4 	= 14,
	// cv's
	NRPNH_CV1		= 21,
	NRPNH_CV2		= 22,
	NRPNH_CV3		= 23,
	NRPNH_CV4		= 24,
	// gates
	NRPNH_GATE1 	= 31,
	NRPNH_GATE2 	= 32,
	NRPNH_GATE3 	= 33,
	NRPNH_GATE4 	= 34,
	NRPNH_GATE5 	= 35,
	NRPNH_GATE6 	= 36,
	NRPNH_GATE7 	= 37,
	NRPNH_GATE8 	= 38,
	NRPNH_GATE9 	= 39,
	NRPNH_GATE10	= 40,
	NRPNH_GATE11	= 41,
	NRPNH_GATE12	= 42
};

// Parameter Number Low Byte 
enum {
	NRPNL_SRC			= 1,
	NRPNL_CHAN			= 2,
	NRPNL_NOTE_MIN  	= 3,
	NRPNL_NOTE			= NRPNL_NOTE_MIN,
	NRPNL_NOTE_MAX  	= 4,
	NRPNL_VEL_MIN  		= 5,
	//NRPNL_VEL_ACCENT	= 6,
	NRPNL_PB_RANGE		= 7,
	NRPNL_PRIORITY		= 8,	
	NRPNL_SPLIT  		= 9,
	NRPNL_TICK_OFS		= 11,
	NRPNL_GATE_DUR		= 12,
	NRPNL_THRESHOLD		= 13,
	NRPNL_TRANSPOSE		= 14,
	NRPNL_VOLTS			= 15,
	NRPNL_CV_OFFSET		= 126,
	NRPNL_CV_GAIN		= 127
};

// Parameter Value High Byte
enum {
	NRPVH_SRC_DISABLE		= 0,

	NRPVH_SRC_MIDINOTE		= 1,
	NRPVH_SRC_MIDICC		= 2,
	NRPVH_SRC_MIDIBEND		= 3,

	NRPVH_SRC_STACK1		= 11,
	NRPVH_SRC_STACK2		= 12,
	NRPVH_SRC_STACK3		= 13,
	NRPVH_SRC_STACK4		= 14,

	NRPVH_SRC_MIDITICK		= 20,
	NRPVH_SRC_MIDITICKRUN	= 21,
	NRPVH_SRC_MIDIRUN		= 22,
	NRPVH_SRC_MIDISTART		= 23,
	NRPVH_SRC_MIDICONT		= 24,
	NRPVH_SRC_MIDISTOP		= 25,

	NRPVH_SRC_TESTVOLTAGE0	= 119,
	NRPVH_SRC_TESTVOLTAGE1	= 120,
	NRPVH_SRC_TESTVOLTAGE2	= 121,
	NRPVH_SRC_TESTVOLTAGE3	= 122,
	NRPVH_SRC_TESTVOLTAGE4	= 123,
	NRPVH_SRC_TESTVOLTAGE5	= 124,
	NRPVH_SRC_TESTVOLTAGE6	= 125,
	NRPVH_SRC_TESTVOLTAGE7	= 126,
	NRPVH_SRC_TESTVOLTAGE8	= 127,
	
	NRPVH_CHAN_SPECIFIC		= 0,
	NRPVH_CHAN_OMNI			= 1,
	NRPVH_CHAN_GLOBAL		= 2,
	
	NRPVH_DUR_INF			= 0,
	NRPVH_DUR_MS			= 1,
	NRPVH_DUR_GLOBAL		= 2,
};

// Parameter Value Low Byte
enum {
	NRPVL_SRC_NO_NOTES			= 0,
	NRPVL_SRC_NOTE1				= 1,
	NRPVL_SRC_NOTE2				= 2,
	NRPVL_SRC_NOTE3				= 3,
	NRPVL_SRC_NOTE4				= 4,
	NRPVL_SRC_ANY_NOTES			= 5,

	NRPVL_SRC_VEL				= 20,
//	NRPVL_SRC_PB				= 21,		
	NRPVL_SRC_AFTERTOUCH		= 22
};




// Parameter values for P_DIV
/*
enum {
  P_DIV_1    = 96,
  P_DIV_2D   = 72,
  P_DIV_2    = 48,
  P_DIV_4D   = 36,
  P_DIV_2T   = 32,  
  P_DIV_4    = 24,
  P_DIV_8D   = 18,
  P_DIV_4T   = 16,
  P_DIV_8    = 12,
  P_DIV_16D  = 9,
  P_DIV_8T   = 8,
  P_DIV_16   = 6,
  P_DIV_16T  = 4,
  P_DIV_32   = 3	
};*/

//
// STRUCT DEFS
//

// note stack config
typedef struct {
	byte chan;			// midi channel
	byte note_min;		// lowest note
	byte note_max;		// highest note
	byte vel_min;		// minimum velocity threshold
	byte bend_range;	// pitch bend range (+/- semitones)
	byte priority;		// how notes are prioritised when assigned to outputs
} NOTE_STACK_CFG;

// note stack state
typedef struct {
	byte note[SZ_NOTE_STACK];	// the notes held in the stack
	char count;					// number of held notes
	byte out[4];				// the stack output notes
	long bend;					// pitch bend
	byte vel;					// note velocity	
} NOTE_STACK;

// GLOBAL DATA
extern char g_led_1_timeout;
extern char g_led_2_timeout;
extern NOTE_STACK g_stack[NUM_NOTE_STACKS];
extern NOTE_STACK_CFG g_stack_cfg[NUM_NOTE_STACKS];
extern byte g_chan;
extern byte g_gate_duration;
extern byte g_cv_dac_pending;
extern volatile byte g_i2c_tx_buf[I2C_TX_BUF_SZ];
extern volatile byte g_i2c_tx_buf_index;
extern volatile byte g_i2c_tx_buf_len;
extern volatile unsigned int g_sr_data;
extern volatile unsigned int g_sync_sr_data;
extern volatile unsigned int g_sync_sr_mask;
extern volatile byte g_sync_sr_data_pending;
extern volatile byte g_sr_data_pending;

// EXPORTED FUNCTIONS FROM MAIN FILE
void i2c_send(byte data);
void i2c_begin_write(byte address);
void i2c_end();
void nrpn(byte param_hi, byte param_lo, byte value_hi, byte value_lo);

// EXPORTED FUNCTIONS FROM GLOBAL MODULE
void global_init();
byte global_nrpn(byte param_lo, byte value_hi, byte value_lo);

// EXPORTED FUNCTIONS FROM NOTE STACK MODULE
void stack_midi_note(byte chan, byte note, byte vel);
void stack_midi_bend(byte chan, int bend);
void stack_midi_aftertouch(byte chan, byte value);
byte stack_nrpn(byte which_stack, byte param_lo, byte value_hi, byte value_lo);
void stack_init();
void stack_reset();

// PUBLIC FUNCTIONS FROM GATES MODULE
void gate_event(byte event, byte stack_id);
void gate_midi_note(byte chan, byte note, byte vel);
void gate_midi_cc(byte chan, byte cc, byte value);
void gate_midi_clock(byte msg);
void gate_run();
void gate_init();
void gate_reset();
void gate_trigger(byte which_gate, byte trigger_enabled);
byte gate_nrpn(byte which_gate, byte param_lo, byte value_hi, byte value_lo);
void gate_update();
void gate_write();

// PUBLIC FUNCTIONS FROM CV MODULE
void cv_event(byte event, byte stack_id);
void cv_midi_cc(byte chan, byte cc, byte value);
void cv_midi_touch(byte chan, byte value);
void cv_midi_bend(byte chan, int bend);
void cv_midi_bpm(long value);
void cv_init(); 
void cv_reset();
byte cv_nrpn(byte which_cv, byte param_lo, byte value_hi, byte value_lo);
//void cv_write_dac(byte which, int value);
//void cv_write_note(byte which, byte midi_note, int pitch_bend);
//void cv_write_vel(byte which, long value);
//void cv_write_cc(byte which, long value);
//void cv_write_bend(byte which, long value);
void cv_dac_prepare();

// PRESETS
void preset1();