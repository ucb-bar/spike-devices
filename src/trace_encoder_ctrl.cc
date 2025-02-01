#include "trace_encoder_ctrl.h"

trace_encoder_ctrl_t::trace_encoder_ctrl_t(trace_encoder_l* encoder) {
  this->encoder = encoder;
}

bool trace_encoder_ctrl_t::load(reg_t addr, size_t len, uint8_t* bytes) {
  switch (addr) {
    case TR_TE_CTRL:
      bytes[0] = this->encoder->get_enable() << 1;
      return true;
    default:
      return false;
  }
}

bool trace_encoder_ctrl_t::store(reg_t addr, size_t len, const uint8_t* bytes) {
    printf("[TRACE_ENCODER_CTRL]: Storing %d bytes with value %lx to 0x%lx\n", len, bytes[0], addr);
    switch (addr) {
        case TR_TE_CTRL:
            printf("[TRACE_ENCODER_CTRL]: Setting enable to %d\n", (bytes[0] >> 1) & 0x1);
            this->encoder->set_enable((bytes[0] >> 1) & 0x1); // Set enable to the second bit
            return true;
        case TR_TE_TARGET:
            // printf("[TRACE_ENCODER_CTRL]: Setting enable to %d\n", (bytes[0] >> 1) & 0x1);
            // this->encoder->set_enable((bytes[0] >> 1) & 0x1);
            return true;
        default:
            return false;
    }
}

int trace_encoder_ctrl_parseFDT(const void *fdt, reg_t *address,
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
trace_encoder_ctrl_t* trace_encoder_ctrl_parseFromFDT(const void* fdt, const sim_t* sim, reg_t* base, std::vector<std::string> sargs) {
  if (trace_encoder_ctrl_parseFDT(fdt, base, "ucbbar,trace_encoder_ctrl") == 0) {
    printf("Found trace_encoder_ctrl at 0x%lx\n", *base);
    return new trace_encoder_ctrl_t(&(const_cast<sim_t*>(sim)->get_core(0)->trace_encoder));
  } else {
    return nullptr;
  }
}

std::string trace_encoder_ctrl_generateDTS(const sim_t* sim, const std::vector<std::string>& args) {
  std::stringstream s;
  s << std::hex
    << "    trace_encoder_ctrl: trace_encoder_ctrl@" << TRACE_ENCODER_CTRL_BASE << " {\n"
       "      compatible = \"ucbbar,trace_encoder_ctrl\";\n"
       "      interrupt-parent = <&PLIC>;\n"
       "      interrupts = <" << std::dec << 2;
  reg_t blkdevbs = TRACE_ENCODER_CTRL_BASE;
  reg_t blkdevsz = TRACE_ENCODER_CTRL_SIZE;
  s << std::hex << ">;\n"
       "      reg = <0x" << (blkdevbs >> 32) << " 0x" << (blkdevbs & (uint32_t)-1) <<
                   " 0x" << (blkdevsz >> 32) << " 0x" << (blkdevsz & (uint32_t)-1) << ">;\n"
       "    };\n";
  return s.str();
}

REGISTER_DEVICE(trace_encoder_ctrl, trace_encoder_ctrl_parseFromFDT, trace_encoder_ctrl_generateDTS);
