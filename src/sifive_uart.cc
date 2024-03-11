#include "sifive_uart.h"

bool sifive_uart_t::load(reg_t addr, size_t len, uint8_t* bytes) {
  if (addr >= 0x1000 || len > 4) return false;
  uint32_t r = 0;
  switch (addr) {
  case UART_TXFIFO: r = 0x0          ; break;
  case UART_RXFIFO: r = read_rxfifo(); break;
  case UART_TXCTRL: r = txctrl       ; break;
  case UART_RXCTRL: r = rxctrl       ; break;
  case UART_IE:     r = ie           ; break;
  case UART_IP:     r = read_ip()    ; break;
  case UART_DIV:    r = div          ; break;
  default: printf("LOAD -- ADDR=0x%lx LEN=%lu\n", addr, len); abort();
  }
  memcpy(bytes, &r, len);
  return true;
}


bool sifive_uart_t::store(reg_t addr, size_t len, const uint8_t* bytes) {
  if (addr >= 0x1000 || len > 4) return false;
  switch (addr) {
  case UART_TXFIFO: canonical_terminal_t::write(*bytes); return true;
  case UART_TXCTRL: memcpy(&txctrl, bytes, len); return true;
  case UART_RXCTRL: memcpy(&rxctrl, bytes, len); return true;
  case UART_IE:     memcpy(&ie, bytes, len); update_interrupts(); return true;
  case UART_DIV:    memcpy(&div, bytes, len); return true;
  default: printf("STORE -- ADDR=0x%lx LEN=%lu\n", addr, len); abort();
  }
}

void sifive_uart_t::tick(reg_t UNUSED rtc_ticks) {
  if (rx_fifo.size() >= UART_RX_FIFO_SIZE) return;
/* int rc = canonical_terminal_t::read(); */
/* if (rc < 0) return; */
/* rx_fifo.push((uint8_t)rc); */
  update_interrupts();
}

int fdt_parse_sifive_uart(const void *fdt, reg_t *sifive_uart_addr,
			  const char *compatible) {
  int nodeoffset, len, rc;
  const fdt32_t *reg_p;

  nodeoffset = fdt_node_offset_by_compatible(fdt, -1, compatible);
  if (nodeoffset < 0)
    return nodeoffset;

  rc = fdt_get_node_addr_size(fdt, nodeoffset, sifive_uart_addr, NULL, "reg");
  if (rc < 0 || !sifive_uart_addr)
    return -ENODEV;

  return 0;
}

// This function parses an instance of this device from the FDT
// An FDT node for a device should encode, at minimum, the base address for the device
sifive_uart_t* sifive_uart_parse_from_fdt(const void* fdt, const sim_t* sim, reg_t* base, std::vector<std::string> sargs) {
  if (fdt_parse_sifive_uart(fdt, base, "sifive,uart0") == 0) {
    printf("Found uart at %lx\n", *base);
    return new sifive_uart_t(sim->get_intctrl(), UART_INTERRUPT_ID);
  } else {
    return nullptr;
  }
}

// This function is used to provide a default DTS node for spike's default DTS
// generator. This UART should not be generated by default, since it would
// conflict with the builtin ns16550 UART.
std::string sifive_uart_generate_dts(const sim_t* sim) {
  std::stringstream s;
  s << std::hex
    << "    SERIAL0: sifive-uart@" << UART_BASE << " {\n"
       "      compatible = \"sifive,uart0\";\n"
       "      interrupt-parent = <&PLIC>;\n"
       "      interrupts = <" << std::dec << UART_INTERRUPT_ID;
  reg_t uart_bs = UART_BASE;
  reg_t uart_sz = UART_SIZE;
  s << std::hex << ">;\n"
       "      reg = <0x" << (uart_bs >> 32) << " 0x" << (uart_bs & (uint32_t)-1) <<
                   " 0x" << (uart_sz >> 32) << " 0x" << (uart_sz & (uint32_t)-1) << ">;\n"
       "    };\n";
  return s.str();
  return std::string();
}

REGISTER_DEVICE(sifive_uart, sifive_uart_parse_from_fdt, sifive_uart_generate_dts);
