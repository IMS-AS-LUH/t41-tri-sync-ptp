#ifndef T41_TRI_SYNC_PTP_H
#define T41_TRI_SYNC_PTP_H
/*
 *  Set the following defines BEFORE including this header if you want to change the defaults
 *  TRISYNC_MHZ: Define the CPU clock frequency in MHz [Default: 600]
 *  TRISYNC_FPS: Define the FPS for the signal generation [Default: 30]
 *  TRISYNC_OFFSETNS: Define a manual offset for the synchronization point in ns [Default: 0]
 *
 *  Inside ptp-base.cpp, there are a few parameters available:
 *  DRIFT_OUTLIER_THRESHOLD: If a drift with a deviation greater than the configured value based
 *                           of the low pass estimation of the drift is detected, it is disregarded.
 *                           In nanoseconds. [Default: 30000]
 *  STABLE_THRESHOLD: The threshold at which the ptp algorithms says the synchronization is stable
 *                    In nanoseconds. [Default: 100]
 *  MAX_CLOCK_DRIFT: A parameter which defines the maximum clock drift allowed. In Nanoseconds.
 *                   [Default: 5000000]
 *
 * Configure the ptp controller with the ptp.begin() function.
 * See the full example for more details. Example call with rate limit of 1000 per update and 4 Hz:
 *  ptp.begin(trisync::FRAMEDURATION, trisync::DEFAULTJUMPTHRESHOLD, 1000, 4,
              &trisync::block_incomming_interrupt, &trisync::update_interupt_counter);
 */

#include "t41-ptp.h"
#include "t41-tri-sync.h"

#endif // T41_TRI_SYNC_PTP_H