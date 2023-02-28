#include <LoRa.h>
#include <SerPrint.h>
#include <SPI.h>
#include "config.h"
#include "crypto.h"
#include "gateway.h"
#include "loracomm.h"

#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26

#define BAND 8681E5

/**
 * Callback on recieved message
 */
IRAM_ATTR void callbackRecv(int packetSize)
{
    char buffer[packetSize];
    char plain[packetSize];

    for (int i = 0; i < packetSize; i++)
    {
        buffer[i] = LoRa.read();
    }

    if (strncmp(buffer, MAGIC_WORD, strlen(MAGIC_WORD)) == 0)
    {
        SerPrint("Received valid msg with rssi: ");
        SerPrintln(LoRa.packetRssi());

        int ret = decrypt_packet(plain, sizeof(plain) / sizeof(plain[0]), &buffer[strlen(MAGIC_WORD)], packetSize - strlen(MAGIC_WORD));
        SerPrint("Decryption returned: ");
        SerPrintln(ret);
        if (ret == 0)
        {
            Strring.pushstr(plain);
        }
    }
    else
    {
        SerPrintln("Wrong sync word, ignore msg");
    }
}

/**
 * Starts lora module and sets default configuration
 * returns 0 on success, otherwise -1
 */
int loraInit()
{
    SPI.begin(SCK, MISO, MOSI, SS);
    LoRa.setSPI(SPI);
    LoRa.setPins(SS, RST, DIO0);

    if (!LoRa.begin(BAND))
    {
        return -1;
    }

    LoRa.setSignalBandwidth(125E3);
    LoRa.enableCrc();
    LoRa.setSpreadingFactor(7);
    LoRa.setSyncWord(SYNC_WORD);

    return 0;
}

/**
 * Starts receiving of messages
 */
void loraReceive()
{
    LoRa.onReceive(callbackRecv);
    LoRa.receive();
}