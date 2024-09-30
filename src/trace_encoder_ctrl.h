#ifndef _TRACE_ENCODER_CTRL_H
#define _TRACE_ENCODER_CTRL_H

#include <riscv/abstract_device.h>
#include <riscv/trace_encoder_n.h>
#include <riscv/dts.h>
#include <riscv/sim.h>
#include <fdt/libfdt.h>

#define TRACE_ENCODER_CTRL_BASE 0x10000000
#define TRACE_ENCODER_CTRL_SIZE 0x1000

#define TR_TE_CTRL 0x000
#define TR_TE_IMPL 0x004

class trace_encoder_ctrl_t : public abstract_device_t {
public:
    trace_encoder_ctrl_t(trace_encoder_n* encoder);
    bool load(reg_t addr, size_t len, uint8_t* bytes) override;
    bool store(reg_t addr, size_t len, const uint8_t* bytes) override;
private:
    trace_encoder_n* encoder;
};

#endif