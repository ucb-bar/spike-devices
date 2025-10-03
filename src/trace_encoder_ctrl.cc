#include "trace_encoder_ctrl.h"

trace_encoder_ctrl_t::trace_encoder_ctrl_t(abstract_trace_encoder_t* encoder) {
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
        case TR_TE_SINK:
            printf("[TRACE_ENCODER_CTRL]: IGNORING sink setting to %d\n", (bytes[0] >> 1) & 0x1);
            return true;
        case TR_TE_BR_MODE:
            printf("[TRACE_ENCODER_CTRL]: Setting br_mode to %d\n", bytes[0]);
            this->encoder->set_br_mode(static_cast<br_mode_t>(bytes[0]));
            return true;
        // case TR_TE_CTX_MODE:
        //     printf("[TRACE_ENCODER_CTRL]: Setting ctx_mode to %d\n", bytes[0]);
        //     this->encoder->set_ctx_mode(static_cast<ctx_mode_t>(bytes[0]));
        //     return true;
        // case TR_TE_CTX_ASID:
        //     uint32_t ctx_id = bytes[3] << 24 | bytes[2] << 16 | bytes[1] << 8 | bytes[0];
        //     printf("[TRACE_ENCODER_CTRL]: Setting ctx_id to %d\n", ctx_id);
        //     this->encoder->set_ctx_id(ctx_id);
        //     return true;
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
  if (trace_encoder_ctrl_parseFDT(fdt, base, "ucbbar,trace") == 0) {
    printf("Found trace_encoder_ctrl at 0x%lx\n", *base);
    return new trace_encoder_ctrl_t((const_cast<sim_t*>(sim)->get_core(0)->trace_encoder));
  } else {
    return nullptr;
  }
}

std::string trace_encoder_ctrl_generateDTS(const sim_t* sim, const std::vector<std::string>& args) {
  std::stringstream s;
  reg_t base = TRACE_ENCODER_CTRL_BASE;
  reg_t size = TRACE_ENCODER_CTRL_SIZE;
  
  s << "    trace_encoder_ctrl: trace_encoder_ctrl@" << std::hex << base << " {\n"
    << "      compatible = \"ucbbar,trace\";\n"
    << "      reg = <0x" << (base >> 32) << " 0x" << (base & 0xffffffff)
    << " 0x" << (size >> 32) << " 0x" << (size & 0xffffffff) << ">;\n"
    << "    };\n";
  
  return s.str();
}

REGISTER_DEVICE(trace_encoder_ctrl, trace_encoder_ctrl_parseFromFDT, trace_encoder_ctrl_generateDTS);
