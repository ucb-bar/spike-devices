#include "sim_udp.h"

bool sim_udp_t::load(reg_t addr, size_t len, uint8_t* bytes) {
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
  case ETH_ADDR_IP: r = reg_addr_ip  ; break;
  case ETH_ADDR_PORT: r = reg_addr_port; break;
  default: printf("LOAD -- ADDR=0x%lx LEN=%lu\n", addr, len); abort();
  }
  memcpy(bytes, &r, len);
  return true;
}


bool sim_udp_t::store(reg_t addr, size_t len, const uint8_t* bytes) {
  if (addr >= 0x1000 || len > 4) return false;
  switch (addr) {
  case UART_TXFIFO: canonical_terminal_t::write(*bytes); return true;
  case UART_TXCTRL: memcpy(&txctrl, bytes, len); return true;
  case UART_RXCTRL: memcpy(&rxctrl, bytes, len); return true;
  case UART_IE:     memcpy(&ie, bytes, len); update_interrupts(); return true;
  case UART_DIV:    memcpy(&div, bytes, len); return true;
  case ETH_ADDR_IP: memcpy(&reg_addr_ip, bytes, len); return true;
  case ETH_ADDR_PORT: memcpy(&reg_addr_port, bytes, len); return true;
  default: printf("STORE -- ADDR=0x%lx LEN=%lu\n", addr, len); abort();
  }
}

void sim_udp_t::tick(reg_t UNUSED rtc_ticks) {
  if (rx_fifo.size() >= UART_RX_FIFO_SIZE) return;
  int rc = canonical_terminal_t::read();
  if (rc < 0) return;
  rx_fifo.push((uint8_t)rc);
  update_interrupts();
}

int SimUDP_parseFDT(const void *fdt, reg_t *address,
			  const char *compatible) {
  int nodeoffset, len, rc;
  const fdt32_t *reg_p;

  nodeoffset = fdt_node_offset_by_compatible(fdt, -1, compatible);
  if (nodeoffset < 0)
    return nodeoffset;

  rc = fdt_get_node_addr_size(fdt, nodeoffset, address, NULL, "reg");
  if (rc < 0 || !address)
    return -ENODEV;

  return 0;
}


// This function parses an instance of this device from the FDT
// An FDT node for a device should encode, at minimum, the base address for the device
sim_udp_t* SimUDP_parseFromFDT(const void* fdt, const sim_t* sim, reg_t* base, std::vector<std::string> sargs) {
  if (SimUDP_parseFDT(fdt, base, "ucbbar,sim_udp") == 0) {
    printf("Found SimUDP at %lx\n", *base);
    return new sim_udp_t(sim->get_intctrl(), 1);
  } else {
    return nullptr;
  }
}

std::string SimUDP_generateDTS(const sim_t* sim) {
  std::stringstream s;
  s << std::hex
    << "    udp: udp@" << 0x10001000 << " {\n"
       "      compatible = \"ucbbar,sim_udp\";\n"
       "      interrupt-parent = <&PLIC>;\n"
       "      interrupts = <" << std::dec << 2;
  reg_t blkdevbs = 0x10001000;
  reg_t blkdevsz = 0x1000;
  s << std::hex << ">;\n"
       "      reg = <0x" << (blkdevbs >> 32) << " 0x" << (blkdevbs & (uint32_t)-1) <<
                   " 0x" << (blkdevsz >> 32) << " 0x" << (blkdevsz & (uint32_t)-1) << ">;\n"
       "    };\n";
  return s.str();
}

REGISTER_DEVICE(sim_udp, SimUDP_parseFromFDT, SimUDP_generateDTS);
