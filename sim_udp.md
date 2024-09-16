
# Simulated User Datagram Protocol (SimUDP)

The SimUDP peripheral provides a mean to transmit and receive datagrams through a UDP socket connection. 

## SimUDP Registers

### UDP RXIP Register

Address offset: 0x00
Reset value: 0x0100_007F

`Bit 31:0` IP: The IP address to listen to, in network endianness (big-endian)

### UDP TXIP Register

Address offset: 0x04
Reset value: 0x0100_007F

`Bit 31:0` IP: The IP address to send to, in network endianness (big-endian)

### UDP RXPORT Register

Address offset: 0x08
Reset value: 0x0000_0000

`Bit 15:0` PORT: The receive port in network endianness (big-endian)

### UDP TXPORT Register

Address offset: 0x0C
Reset value: 0x0000_0000

`Bit 15:0` PORT: The transmit port in network endianness (big-endian)

### UDP CTRL Register

Address offset: 0x10
Reset value: 0x0000_0000

`Bit 0` ENABLE: Enable the device. 

`Bit 1` RXEN: Enable the reception functionality for one frame. 

`Bit 2` TXEN: Enable the transmission functionality for one frame. 

### UDP STATUS Register

Address offset: 0x14
Reset value: 0x0000_0000

`Bit 15:0` MAXRXSIZE: Maximum number of bytes to receive.

### UDP RXSIZE Register

Address offset: 0x18
Reset value: 0x0000_0000

`Bit 15:0` SIZE: Number of bytes received.

### UDP TXSIZE Register

Address offset: 0x1C
Reset value: 0x0000_0000

`Bit 15:0` SIZE: Number of bytes to transmit.

### UDP RXFIFO Register[0:255]

Address offset: 0x30
Reset value: 0x0000_0000

`Bit 31:0` DATA

### UDP TXSIZE Register[0:255]

Address offset: 0x130
Reset value: 0x0000_0000

`Bit 31:0` DATA


## Programming Model

```
void UDP_enable() {
    regwrite32(rxip, rxip);
    regwrite32(txip, txip);
    regwrite32(rxport, rxport);
    regwrite32(txport, txport);
    regwrite32(ctrl, 0x01); // enables the device
}
```
```
void UDP_config_recv_size(uint32_t size){
    regwrite32(rx_size, size);
}
```
```
void UDP_receive_frame(){
    regwrite32(ctrl, 0x01 << 1); // turns on the udp rx flag for a frame
    while (!rx_status); // poll untile done
}
```