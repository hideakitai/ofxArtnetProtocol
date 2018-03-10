/*The MIT License (MIT)

 Copyright (c) 2014 Nathanaël Lécaudé
 https://github.com/natcl/Artnet, http://forum.pjrc.com/threads/24688-Artnet-to-OctoWS2811

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 */

#include <ofxArtnetProtocol.h>

ofxArtnetProtocol::ofxArtnetProtocol() {}


void ofxArtnetProtocol::begin()
{
	udpConnection.Create();
	udpConnection.Bind(ART_NET_PORT);
	udpConnection.SetNonBlocking(true);
	bConnected = true;
}

void ofxArtnetProtocol::begin(const char* ip)
{
    bConnected = udpConnection.Create();

		udpConnection.SetEnableBroadcast(true);
		udpConnection.SetReuseAddress(true);

    if (!bConnected) {
        ofLogWarning("ofxArtnetProtocol") << "UDP socket creation failed!!";
        udpConnection.Close();
        return;
    }
    bConnected = udpConnection.Connect(ip, ART_NET_PORT);
    if (!bConnected) {
        ofLogWarning("ofxArtnetProtocol") << "UDP socket connection failed!!";
        udpConnection.Close();
        return;
    }
    udpConnection.SetNonBlocking(true);

    ofLogNotice("ofxArtnetProtocol") << "artnet is successfully connected to: "+ofToString(ip)+" / post: "+ofToString(ART_NET_PORT);

    for (size_t i=0; i<sizeof(ART_NET_ID); i++) {
        packet.id[i] = ART_NET_ID[i];
    }
    packet.opcode[0] = (char)((ART_DMX >> 0) & 0xFF);
    packet.opcode[1] = (char)((ART_DMX >> 8) & 0xFF);
    packet.sequence = 0;
    packet.universe[0] = packet.universe[1] = 0;
    packet.length[0] = ((512 >> 0) & 0xFF);
    packet.length[1] = ((512 >> 8) & 0xFF);
}

uint16_t ofxArtnetProtocol::read()
{
    if (!bConnected) return 0;

    int packetSize = udpConnection.Receive((char*)artnetPacket, MAX_BUFFER_ARTNET);

    if (packetSize <= MAX_BUFFER_ARTNET && packetSize > 0)
    {
        // Check that packetID is "Art-Net" else ignore
        for (size_t i = 0; i < 8; i++)
        {
            if (artnetPacket[i] != ART_NET_ID[i])
                return 0;
        }

        opcode = artnetPacket[8] | artnetPacket[9] << 8;

        if (opcode == ART_DMX)
        {
            sequence = artnetPacket[12];
            incomingUniverse = artnetPacket[14] | artnetPacket[15] << 8;
            dmxDataLength = artnetPacket[17] | artnetPacket[16] << 8;

            if (artDmxCallback) (*artDmxCallback)(incomingUniverse, dmxDataLength, sequence, artnetPacket + ART_DMX_START);
            return ART_DMX;
        }
        if (opcode == ART_POLL)
        {
            return ART_POLL;
        }
    }
    else
    {
        return 0;
    }
}

void ofxArtnetProtocol::setDmxData(uint8_t* data, uint16_t size)
{
    if (!bConnected) return;

    for (size_t i=0; i<size; i++) {
        setDmxData(i, data[i]);
    }
}

void ofxArtnetProtocol::setDmxData(uint16_t index, uint8_t data)
{
    if (!bConnected) return;

    packet.data[index] = (char)data;
}

void ofxArtnetProtocol::send(uint8_t* data, uint16_t universe, uint16_t size)
{
    if (!bConnected) return;

    static uint8_t seq = 0;
    packet.sequence = seq;
    packet.universe[0] = ((universe >> 0) & 0xFF);
    packet.universe[1] = ((universe >> 8) & 0xFF);
    packet.length[0] = ((size >> 0) & 0xFF);
    packet.length[1] = ((size >> 8) & 0xFF);
    setDmxData(data, size);


    for (size_t i=0; i<sizeof(packet.id); i++) {
        artnet_send_packet[i] = packet.id[i];
    }
    artnet_send_packet[8] = packet.opcode[0];
    artnet_send_packet[9] = packet.opcode[1];
    artnet_send_packet[10] = 0;
    artnet_send_packet[11] = 14;    // ProtVer (Art-Net Protocol Revision)
    artnet_send_packet[12] = seq;
    artnet_send_packet[13] = 0;                             // physical input port of DMX
    artnet_send_packet[14] = packet.universe[0];
    artnet_send_packet[15] = packet.universe[1];
    artnet_send_packet[16] = packet.length[1];
    artnet_send_packet[17] = packet.length[0];

    memcpy(artnet_send_packet+ART_DMX_START, packet.data, 512);

    udpConnection.Send((char*)artnet_send_packet, MAX_BUFFER_ARTNET);

    ++seq;
}

void ofxArtnetProtocol::printPacketHeader()
{
    if (!bConnected) return;

    printf("\npacket size = %d", packetSize);
    printf("\nopcode = %2x", opcode);
    printf("\nuniverse number = %d", incomingUniverse);
    printf("\ndata length = %d", dmxDataLength);
    printf("\nsequence n0. = %d", sequence);
}

void ofxArtnetProtocol::printPacketContent()
{
    if (!bConnected) return;

    for (uint16_t i = ART_DMX_START ; i < dmxDataLength ; i++){
        printf("%d ", artnetPacket[i]);
    }
    printf("\n\n");
}
