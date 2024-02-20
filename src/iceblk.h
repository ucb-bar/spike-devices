#ifndef __BLKDEV_H__
#define __BLKDEV_H__

#include <queue>
#include <vector>
#include <optional>
#include <string>
#include <riscv/abstract_device.h>
#include <riscv/simif.h>
#include <riscv/abstract_interrupt_controller.h>
#include <riscv/mmu.h>
#include <riscv/processor.h>
#include <riscv/simif.h>
#include <riscv/sim.h>
#include <riscv/dts.h>
#include <fdt/libfdt.h>

#define BLKDEV_BASE         0x10015000
#define BLKDEV_INTERRUPT_ID 1
#define BLKDEV_SIZE        0x1000

#define INTERRUPT_LO 0
#define INTERRUPT_HI 1

class iceblk_t : public abstract_device_t {
public:
  iceblk_t(
      const simif_t* sim,
      abstract_interrupt_controller_t *intctrl,
      uint32_t interrupt_id,
      std::vector<std::string> sargs);
  ~iceblk_t();
  bool load(reg_t addr, size_t len, uint8_t* bytes) override;
  bool store(reg_t addr, size_t len, const uint8_t* bytes) override;
  void tick(reg_t rtc_ticks) override;
  void serialize_proto(void* msg, void* arena) override;
  void deserialize_proto(void* msg) override;

private:
  void raise_interrupt();
  void lower_interrupt();

  void post_request();
  void handle_request();
  void handle_read_request();
  void handle_write_request();

  void read_blockdevice_u64(uint64_t* data, reg_t sector_idx, reg_t byte_offset);
  void write_blockdevice_u64(uint64_t data, reg_t sector_idx, reg_t byte_offset);

private:
  const uint64_t blockdevice_latency = 500;
  uint64_t* blockdevice;
  uint64_t blockdevice_size;

  const simif_t* sim;
  abstract_interrupt_controller_t *intctrl;
  uint32_t interrupt_id;
  uint32_t interrupt_level;

  int trackers = 1;
  std::queue<unsigned int> idle_tags;
  std::queue<unsigned int> pending_tags;
  std::queue<unsigned int> cmpl_tags;

  // FIXME : When tracker > 1, need to change these to queues as well
  reg_t req_addr   = 0;
  reg_t req_offset = 0;
  reg_t req_len    = 0;
  reg_t req_write  = 0;

  // checkpointing stuff
private:
  bool ckpt_tracking = false;

  std::map<reg_t, uint64_t> ckpt_block_dev;
  uint32_t ckpt_interrupt_level;

  std::queue<unsigned int> ckpt_idle_tags;
  std::queue<unsigned int> ckpt_pending_tags;
  std::queue<unsigned int> ckpt_cmpl_tags;

  reg_t ckpt_req_addr;
  reg_t ckpt_req_offset;
  reg_t ckpt_req_len;
  reg_t ckpt_req_write;
};

#endif //__BLKDEV_H__
