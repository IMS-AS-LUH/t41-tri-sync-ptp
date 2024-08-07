/*
 * Example of generating a Tri-Level Sync Signal with the Teensy 4.1 based on a PTP synchronized
 * clock. The configuration is set up for 30 FPS and the Teensy should run at 600 MHz. Pins 10 and
 * 12 are used for the lower and higher output pins. The Teensy is configured with a static ip of
 * 192.168.0.210
 */

// define the configuration before including the library!
#define TRISYNC_MHZ 600
#define TRISYNC_FPS 30

#include <QNEthernet.h>
#include <t41-tri-sync-ptp.h>

// configure the output pins and network interface
const int pinbit0 = 10;
const int pinbit1 = 12;

byte mac[6];
IPAddress staticIP{192, 168, 0, 210};
IPAddress subnetMask{255, 255, 255, 0};
IPAddress gateway{192, 168, 0, 6};

bool p2p = false;
bool master = false;
bool slave = true;

l3PTP ptp(master, slave, p2p);
// l2PTP ptp(master,slave,p2p);

void setup()
{
    Serial.begin(2000000);
    pinMode(13, OUTPUT); // LED output

    // Setup networking
    qindesign::network::Ethernet.setHostname("t41ptpslave");
    qindesign::network::Ethernet.macAddress(mac);
    qindesign::network::Ethernet.begin(staticIP, subnetMask, gateway);
    qindesign::network::EthernetIEEE1588.begin();

    qindesign::network::Ethernet.onLinkState([](bool state) {
        Serial.printf("[Ethernet] Link %dMbps %s\n", qindesign::network::Ethernet.linkSpeed(),
                      state ? "ON" : "OFF");
        if (state)
        {
            // configure the ptp controller
            ptp.begin(trisync::FRAMEDURATION, trisync::DEFAULTJUMPTHRESHOLD, 800, 4,
                      &trisync::block_incomming_interrupt, &trisync::update_interrupt_counter,
                      false);
        }
    });

    // configure the trisync library
    trisync::begin(2, pinbit0, pinbit1);

    // configure the trisync pulse for the interrupt
    attachInterruptVector(IRQ_ENET_TIMER, trisync::pulse);
    NVIC_ENABLE_IRQ(IRQ_ENET_TIMER);
}

void loop()
{
    // run ptp updates
    ptp.update();
    // turn on LED if stable time is detected
    digitalWrite(13, ptp.getLockCount() > 5 ? HIGH : LOW);
}