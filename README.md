ofxArtnetProtocol
=================

The port of Art-Net protocol parser [natcl/Artnet](https://github.com/natcl/Artnet) to openFrameworks and mbed.

## Usage

```
ofxArtnetProtocol artnet;
uint8_t universe[512];

void setup() {
	artnet.begin("10.0.1.2");
}
void loop() {

	// do something with your universe

	artnet.send(universe);
}
```


## API

```c++
void begin(const char* ip);
uint16_t read();
void setDmxData(uint8_t* data, uint16_t size);
void setDmxData(uint16_t index, uint8_t data);
void send(uint8_t* data, uint16_t universe = 0, uint16_t size = 512);
void printPacketHeader();
void printPacketContent();

uint8_t* getDmxFrame(void);
uint16_t getOpcode(void);
uint8_t getSequence(void);
uint16_t getUniverse(void);
uint16_t getLength(void);
```


## Dependencies

#### ofxArtnet

- ofxNetwork


#### Artnet for mbed

- EthernetInterface
- mbed-rtos



## License

MIT License



