#ifndef T41_TRI_SYNC_H
#define T41_TRI_SYNC_H

#include "Arduino.h"
#include <QNEthernet.h>
#include <math.h>

// OUTSIDE (Pre-include) defines defaults
// teensy clock speed in MHz
#ifndef TRISYNC_MHZ
#define TRISYNC_MHZ 600
#endif

// frame rate of the signal
#ifndef TRISYNC_FPS
#define TRISYNC_FPS 30
#endif

// manual offset in ns
#ifndef TRISYNC_OFFSETNS
#define TRISYNC_OFFSETNS 0
#endif

namespace trisync
{

// FIXED defines
constexpr uint32_t MS_PER_S = 1000;
constexpr uint32_t NS_PER_S = 1000 * 1000 * 1000;
constexpr uint32_t LINES_PER_FRAME = 1125;                     // st0274-2008 table 1
constexpr uint32_t SAMPLES_PER_EDGE = 44;                      // st0274-2008 10.2
constexpr uint32_t VSYNC_START_DELAY = 132 - SAMPLES_PER_EDGE; // st0274-2008 figure 5
constexpr uint32_t VSYNC_END_RESET = 44; // st0274-2008 figure 5 (2200 samples per line only)

// DEPENDEND defines
// check for supported frame rates and set defines
#if TRISYNC_FPS == 60
constexpr uint32_t SAMPLES_PER_MS = 148500; // st0274-2008 table 1
#elif TRISYNC_FPS == 30
constexpr uint32_t SAMPLES_PER_MS = 74250; // st0274-2008 table 1
#else
#error "Given FPS is not supported"
#endif

#if TRISYNC_OFFSETNS >= 1000000000
#error "Manual offset must be smaller than +1 second"
#elif TRISYNC_OFFSETNS <= -1000000000
#error "Manual offset must be greater than -1 second"
#endif

constexpr double FRAMEDURATION = NS_PER_S / TRISYNC_FPS;
constexpr int64_t DEFAULTJUMPTHRESHOLD = NS_PER_S / (1.6 * TRISYNC_FPS);
constexpr double PULSE_PERIOD_NS = NS_PER_S / static_cast<double>(LINES_PER_FRAME * TRISYNC_FPS);

constexpr int NS_TO_SYNC = SAMPLES_PER_EDGE * MS_PER_S * MS_PER_S / SAMPLES_PER_MS +
                           VSYNC_END_RESET * MS_PER_S * MS_PER_S / SAMPLES_PER_MS;
const int PREOFFSET = NS_TO_SYNC + 200 + 64; // 200 for initial setup | 100 based on osci
                                             // 64 vs 100 jumps more than 36?!Interrupt collision?
const int MANUAL_OFFSET = TRISYNC_OFFSETNS - NS_TO_SYNC - 200;

// convert to cylces
constexpr double CYCLES_PER_NS = TRISYNC_MHZ / 1000;
constexpr uint32_t CYCLES_PER_EDGE = SAMPLES_PER_EDGE * 1000 * TRISYNC_MHZ / SAMPLES_PER_MS;
constexpr uint32_t CYCLES_VSYNC_START_DELAY =
    VSYNC_START_DELAY * 1000 * TRISYNC_MHZ / SAMPLES_PER_MS;
constexpr uint32_t CYCLES_VSYNC_END_RESET = VSYNC_END_RESET * 1000 * TRISYNC_MHZ / SAMPLES_PER_MS;

// internal variables
volatile uint32_t linenumber = 0;
int interrupt_channel = 0;
int pinbit0 = 0;
int pinbit1 = 1;

// delay routine based on CPU Cycles
static inline void delayCycles(uint32_t) __attribute__((always_inline, unused));
static inline void delayCycles(uint32_t cycles)
{ // MIN return in 7 cycles NEAR 20 cycles it gives wait +/- 2 cycles - with sketch timing overhead
    uint32_t begin = ARM_DWT_CYCCNT - 12; // Overhead Factor for execution
    while (ARM_DWT_CYCCNT - begin < cycles)
        ; // wait
}

// blocks if an interrupt is imminent
inline void block_incomming_interrupt()
{
    timespec ts;
    qindesign::network::EthernetIEEE1588.readTimer(ts);
    uint32_t current;
    qindesign::network::EthernetIEEE1588.getChannelCompareValue(interrupt_channel, current);

    int diff = current - ts.tv_nsec;
    if (diff < PULSE_PERIOD_NS * 0.15)
    {
        delayNanoseconds(PULSE_PERIOD_NS * 0.2);
    }
}

// when an offset correction is applied, update the interrupt handling
// linenumber is not modified, as jumps should keep the phase of the signal
inline void update_interrupt_counter()
{
    timespec ts;
    qindesign::network::EthernetIEEE1588.readTimer(ts);

    int64_t current = round(floor(ts.tv_nsec / PULSE_PERIOD_NS + 1) * PULSE_PERIOD_NS);
    if (current == NS_PER_S)
    {
        // make sure it does not drift for whatever reason
        linenumber = 0;
    }
    current += MANUAL_OFFSET;
    current = current >= NS_PER_S ? current - NS_PER_S : current;
    current = current < 0 ? current + NS_PER_S : current;

    qindesign::network::EthernetIEEE1588.setChannelCompareValue(interrupt_channel,
                                                                static_cast<uint32_t>(current));
    qindesign::network::EthernetIEEE1588.setChannelMode(
        interrupt_channel,
        qindesign::network::EthernetIEEE1588
            .TimerChannelModes::kSoftwareCompare); // apply double buffer of compare value
}

// initialization functionl.
// Define the channel number of the timer for the interrupt
// Define GPIO pins for lower and upper bit for the signal generation
void begin(int interruptchannel, int pin_bit0, int pin_bit1)
{
    interrupt_channel = interruptchannel;
    pinbit0 = pin_bit0;
    pinbit1 = pin_bit1;

    pinMode(pinbit0, OUTPUT);
    pinMode(pinbit1, OUTPUT);

    update_interrupt_counter();
    qindesign::network::EthernetIEEE1588.setChannelInterruptEnable(interrupt_channel, true);
}

// interrupt routine generating the tri-sync pulse
// first part generates the pulse, second part generates the next interrupt value
void pulse()
{
    // NOTE: critical time section
    // initial wait for a clean following pulse form
    delayCycles(200 * CYCLES_PER_NS);
    // reset vsync broad pulse (if it was set. Otherwise nothing will happen)
    digitalWriteFast(pinbit0, HIGH);
    delayCycles(CYCLES_VSYNC_END_RESET);

    // Tri-level sync pulse (0mV -> -300mV -> + 300mV -> 0mV)
    digitalWriteFast(pinbit0, LOW); // -300mV
    delayCycles(CYCLES_PER_EDGE);
    digitalWriteFast(pinbit1, HIGH); // +300mV
    delayCycles(CYCLES_PER_EDGE);
    digitalWriteFast(pinbit1, LOW);
    digitalWriteFast(pinbit0, HIGH); // 0mV

    // vsync. Broad pulse in lines 1 to 5 (inclusive) st0274-20078 10.7
    if (linenumber < 5)
    {
        // start broad vsync pulse
        delayCycles(CYCLES_VSYNC_START_DELAY);
        digitalWriteFast(pinbit0, LOW);
    }

    // NOTE: non-critical time section
    // check for next frame
    linenumber++;
    if (linenumber >= LINES_PER_FRAME)
    {
        linenumber = 0;
    }

    // interrupt handling of the network chip
    qindesign::network::EthernetIEEE1588.getAndClearChannelStatus(interrupt_channel);
    update_interrupt_counter();

    asm("dsb"); // dont fire interrupt twice. Added if this function is called directly
}

} // namespace trisync

#endif // T41_TRI_SYNC_H