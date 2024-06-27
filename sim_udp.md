
# Simulated User Datagram Protocol (SimUDP)

The SimUDP peripheral provides a mean to transmit and receive datagrams through a UDP socket connection. 

## SimUDP Registers

### USART RXIP Register

Address offset: 0x00
Reset value: 0x0100_007F

`Bit 31:0` IP: The IP address to listen to, in network endianness (big-endian)

### USART TXIP Register

Address offset: 0x04
Reset value: 0x0100_007F

`Bit 31:0` IP: The IP address to send to, in network endianness (big-endian)

### USART RXPORT Register

Address offset: 0x08
Reset value: 0x0000_0000

`Bit 15:0` PORT: The receive port in network endianness (big-endian)

### USART TXPORT Register

Address offset: 0x0C
Reset value: 0x0000_0000

`Bit 15:0` PORT: The transmit port in network endianness (big-endian)

### USART CTRL Register

Address offset: 0x10
Reset value: 0x0000_0000

`Bit 0` INIT: Initialize the socket object. Writing 1 to this bit creates a new socket.

`Bit 1` RXEN: Enable the reception functionality. 

`Bit 2` TXEN: Enable the transmission functionality. 

`Bit 3` ST: Start tranmission of a Datagram. Writing 1 to this bit starts the transmitsion of a Datagram. After transmission, this bit will be cleared by hardware.

### USART STATUS Register

Address offset: 0x14
Reset value: 0x0000_0000

`Bit 15:0` MAXRXSIZE: Maximum number of bytes to receive.

### USART RXSIZE Register

Address offset: 0x18
Reset value: 0x0000_0000

`Bit 15:0` SIZE: Number of bytes received.

### USART TXSIZE Register

Address offset: 0x1C
Reset value: 0x0000_0000

`Bit 15:0` SIZE: Number of bytes to transmit.

### USART RXFIFO Register[0:255]

Address offset: 0x30
Reset value: 0x0000_0000

`Bit 31:0` DATA

### USART TXSIZE Register[0:255]

Address offset: 0x130
Reset value: 0x0000_0000

`Bit 31:0` DATA

