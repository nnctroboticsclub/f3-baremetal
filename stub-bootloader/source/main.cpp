#include <array>
#include <cstddef>
#include "stm32f303x8.h"

#define BL_FUNCTION __attribute__((section(".bl_text")))
#define BL_FUNCTION2 __attribute__((section(".bl_text2")))

class CharLIFO {
  std::array<char, 64> buffer_ = {};
  size_t head_ = 0;
  size_t tail_ = 64 - 1;

 public:
  BL_FUNCTION bool Empty() const volatile {
    return (64 + tail_ - head_ + 1) % 64 == 0;
  }

  BL_FUNCTION bool Full() const { return head_ == tail_; }

  BL_FUNCTION void Clear() {
    head_ = 0;
    tail_ = 64 - 1;
  }

  BL_FUNCTION bool Push(char const& data) {
    if (Full()) {
      return false;
    }

    buffer_[head_] = data;
    head_ = (head_ + 1) % 64;

    return true;
  }

  BL_FUNCTION char Pop() {
    if (Empty()) {
      return {};
    }

    tail_ = (tail_ + 1) % 64;
    auto data = buffer_[tail_];

    return data;
  }
};
namespace bootloader {

// Skeleton

class USART_1 {
 public:
  static constexpr auto IRQn = USART1_IRQn;

  BL_FUNCTION static inline USART_TypeDef* Instance() { return USART1; }

  BL_FUNCTION static void EnableRxInterrupt() {
    Instance()->CR1 |= USART_CR1_RXNEIE;

    NVIC_EnableIRQ(IRQn);
  }

  BL_FUNCTION static void Write(uint8_t data) {
    while (!(Instance()->ISR & USART_ISR_TXE))
      ;
    Instance()->TDR = static_cast<uint8_t>(data);

    while (!(Instance()->ISR & USART_ISR_TC))
      ;
  }

  BL_FUNCTION static void Write(const char* data, size_t len) {
    while (len) {
      Write(*data++);
      len--;
    }
  }
};
class USART_2 {
  static constexpr uintptr_t usart = USART2_BASE;

 public:
  static constexpr auto IRQn = USART2_IRQn;

  BL_FUNCTION static inline USART_TypeDef* Instance() {
    return reinterpret_cast<USART_TypeDef*>(usart);
  }

  BL_FUNCTION static void EnableRxInterrupt() {
    Instance()->CR1 |= USART_CR1_RXNEIE;

    NVIC_EnableIRQ(IRQn);
  }

  BL_FUNCTION static void Write(uint8_t data) {
    while (!(Instance()->ISR & USART_ISR_TXE))
      ;
    Instance()->TDR = static_cast<uint8_t>(data);

    while (!(Instance()->ISR & USART_ISR_TC))
      ;
  }

  BL_FUNCTION static void Write(const char* data, size_t len) {
    while (len) {
      Write(*data++);
      len--;
    }
  }
};

struct Peripheral {
  using BL_Port = USART_1;
  using Console = USART_2;

  static constexpr uint32_t kCmdPortAddr = 0x20000200;

  static constexpr uint32_t kBufferAddr = 0x10000000;  // on CCMRAM
  static constexpr uint32_t kBufferLength = 0x1000;
};

// Skeleton
class Flash {
  BL_FUNCTION static void WaitFlash() {
    while (FLASH->SR & FLASH_SR_BSY)
      ;

    if (FLASH->SR & FLASH_SR_EOP) {
      FLASH->SR |= FLASH_SR_EOP;  // clear eop
    }
  }

 public:
  BL_FUNCTION static void Unlock() {
    FLASH->KEYR = 0x45670123;
    FLASH->KEYR = 0xcdef89ab;
  }

  BL_FUNCTION static void Write(uint16_t* addr, uint16_t value) {
    WaitFlash();

    FLASH->CR |= FLASH_CR_PG;
    *addr = value;
    WaitFlash();

    FLASH->SR |= FLASH_SR_PGERR;
    FLASH->CR &= ~FLASH_CR_PG;

    return;
  }

  BL_FUNCTION static void Erase(uint16_t page) {
    Peripheral::Console::Write("--- Flash Erase Page: ", 22);
    Peripheral::Console::Write("0123456789ABCDEF"[(int(page) >> 0x0C) & 0x0F]);
    Peripheral::Console::Write("0123456789ABCDEF"[(int(page) >> 0x08) & 0x0F]);
    Peripheral::Console::Write("0123456789ABCDEF"[(int(page) >> 0x04) & 0x0F]);
    Peripheral::Console::Write("0123456789ABCDEF"[(int(page) >> 0x00) & 0x0F]);
    Peripheral::Console::Write('\n');

    WaitFlash();

    FLASH->CR |= FLASH_CR_PER;
    FLASH->AR = 0x08000000 + 0x0800 * page;
    FLASH->CR |= FLASH_CR_STRT;

    asm volatile("nop");  // latency of CPU between to Flash peripheral
    asm volatile("nop");  // latency of CPU between to Flash peripheral

    WaitFlash();

    FLASH->CR = 0;

    return;
  }

  BL_FUNCTION static bool MassErase() {
    Peripheral::Console::Write("--- Flash Erase Mass\n", 22);

    while (FLASH->SR & FLASH_SR_BSY)
      ;

    FLASH->CR |= FLASH_CR_MER;
    FLASH->CR |= FLASH_CR_STRT;
    asm volatile("nop");  // latency of CPU between to Flash peripheral
    while (FLASH->SR & FLASH_SR_BSY)
      ;

    bool is_successed = FLASH->SR & FLASH_SR_EOP;
    if (is_successed) {  // clear EOP
      FLASH->SR |= FLASH_SR_EOP;
    }

    return is_successed;
  }
};

enum class Command : uint8_t {
  kACK = 0xC0,
  kRead = 0xC1,
  kChecksum = 0xC2,
  kWrite = 0xC3,
  kReset = 0xC4,
  kLoad = 0xC5,
  kErase = 0xC6,
  kInvalid = 0xFF
};

enum class ACK : uint8_t { kACK = 0x55, kNACK = 0x1F };

// Singleton
namespace port_buf {
BL_FUNCTION2 void Init() {
  auto& rx_buf = *reinterpret_cast<CharLIFO*>(Peripheral::kCmdPortAddr);

  rx_buf.Clear();

  while (!rx_buf.Full()) {
    rx_buf.Push(0);
  }
  while (!rx_buf.Empty()) {
    rx_buf.Pop();
  }
}
BL_FUNCTION2 uint8_t ReceiveChar() {
  auto rx_buf = reinterpret_cast<CharLIFO*>(Peripheral::kCmdPortAddr);

  while (rx_buf->Empty())
    ;
  auto ch = rx_buf->Pop();

  // char buf[2];
  // buf[0] = "0123456789ABCDEF"[int(ch) >> 4];
  // buf[1] = "0123456789ABCDEF"[int(ch) & 0xF];
  // Peripheral::Console::Write(buf, 2);
  // Peripheral::Console::Write('\n');

  return ch;
}

BL_FUNCTION2 void PushChar(uint8_t ch) {
  auto rx_buf = reinterpret_cast<CharLIFO*>(Peripheral::kCmdPortAddr);

  rx_buf->Push(ch);
}
}  // namespace port_buf

class CmdPort {
  using Port = Peripheral::BL_Port;

  BL_FUNCTION Command ReceiveCommand() {
    auto cmd = port_buf::ReceiveChar();
    auto c_cmd = port_buf::ReceiveChar();

    if ((cmd ^ c_cmd) == 0xFF) {
      Peripheral::Console::Write("- CMD ", 7);
      Peripheral::Console::Write("0123456789ABCDEF"[int(cmd) >> 4]);
      Peripheral::Console::Write("0123456789ABCDEF"[int(cmd) & 0xF]);
      Peripheral::Console::Write('\n');

      Port::Write(static_cast<uint8_t>(ACK::kACK));
      return static_cast<Command>(cmd);
    } else {
      Peripheral::Console::Write("- CMD N\n", 8);

      Port::Write(static_cast<uint8_t>(ACK::kNACK));
      return Command::kInvalid;
    }
  }

  BL_FUNCTION2 uint32_t ReceiveU32() {
    auto oct0 = port_buf::ReceiveChar();
    auto oct1 = port_buf::ReceiveChar();
    auto oct2 = port_buf::ReceiveChar();
    auto oct3 = port_buf::ReceiveChar();
    auto checksum = port_buf::ReceiveChar();

    if ((oct0 ^ oct1 ^ oct2 ^ oct3) != checksum) {
      Peripheral::Console::Write("- U32 N\n", 8);

      Port::Write(static_cast<uint8_t>(ACK::kNACK));
      return 0x5555AAAA;
    } else {
      Peripheral::Console::Write("- U32 ", 7);
      Peripheral::Console::Write("0123456789ABCDEF"[int(oct0) >> 4]);
      Peripheral::Console::Write("0123456789ABCDEF"[int(oct0) & 0xF]);
      Peripheral::Console::Write("0123456789ABCDEF"[int(oct1) >> 4]);
      Peripheral::Console::Write("0123456789ABCDEF"[int(oct1) & 0xF]);
      Peripheral::Console::Write("0123456789ABCDEF"[int(oct2) >> 4]);
      Peripheral::Console::Write("0123456789ABCDEF"[int(oct2) & 0xF]);
      Peripheral::Console::Write("0123456789ABCDEF"[int(oct3) >> 4]);
      Peripheral::Console::Write("0123456789ABCDEF"[int(oct3) & 0xF]);
      Peripheral::Console::Write('\n');

      Port::Write(static_cast<uint8_t>(ACK::kACK));
      return (oct0 << 24) | (oct1 << 16) | (oct2 << 8) | oct3;
    }
  }

  BL_FUNCTION2 uint16_t ReceiveU16() {
    auto oct0 = port_buf::ReceiveChar();
    auto oct1 = port_buf::ReceiveChar();
    auto checksum = port_buf::ReceiveChar();

    if ((oct0 ^ oct1) != checksum) {
      Peripheral::Console::Write("- U16 N\n", 8);
      Port::Write(static_cast<uint8_t>(ACK::kNACK));
      return 0xAAAA;
    } else {
      Peripheral::Console::Write("- U16 ", 7);
      Peripheral::Console::Write("0123456789ABCDEF"[int(oct0) >> 4]);
      Peripheral::Console::Write("0123456789ABCDEF"[int(oct0) & 0xF]);
      Peripheral::Console::Write("0123456789ABCDEF"[int(oct1) >> 4]);
      Peripheral::Console::Write("0123456789ABCDEF"[int(oct1) & 0xF]);
      Peripheral::Console::Write('\n');

      Port::Write(static_cast<uint8_t>(ACK::kACK));
      return (oct0 << 8) | oct1;
    }
  }

  BL_FUNCTION bool ReceiveBuffer(uint8_t* buffer, uint16_t length) {
    uint8_t local_checksum = 0;
    for (size_t i = 0; i < length; i++) {
      auto ch = port_buf::ReceiveChar();
      buffer[i] = ch;

      local_checksum ^= ch;
    }

    auto remote_checksum = port_buf::ReceiveChar();
    if (remote_checksum == local_checksum) {
      Port::Write(static_cast<uint8_t>(ACK::kACK));
      return true;
    } else {
      Port::Write(static_cast<uint8_t>(ACK::kNACK));
      return false;
    }
  }

  BL_FUNCTION bool SendBuffer(uint8_t* buffer, uint32_t length) {
    {
      auto ptr = buffer;
      auto len = length;
      while (len) {
        Port::Write(*ptr++);
        len--;
      }
    }
    Port::Write(Checksum(buffer, length));

    return port_buf::ReceiveChar() == static_cast<uint8_t>(ACK::kACK);
  }
  BL_FUNCTION bool SendU32(uint32_t value) {
    uint8_t oct0 = (value >> 24) & 0xFF;
    uint8_t oct1 = (value >> 16) & 0xFF;
    uint8_t oct2 = (value >> 8) & 0xFF;
    uint8_t oct3 = value & 0xFF;

    uint8_t checksum = oct0 ^ oct1 ^ oct2 ^ oct3;

    Port::Write(oct0);
    Port::Write(oct1);
    Port::Write(oct2);
    Port::Write(oct3);
    Port::Write(checksum);

    return port_buf::ReceiveChar() == static_cast<uint8_t>(ACK::kACK);
  }

  BL_FUNCTION uint8_t Checksum(uint8_t* buffer, uint32_t length) {
    uint8_t checksum = 0;
    for (size_t i = 0; i < length; i++) {
      checksum ^= buffer[i];
    }

    return checksum;
  }
  BL_FUNCTION uint32_t Checksum_Word(uint8_t* buffer, uint32_t length) {
    uint32_t checksum = 0;
    for (size_t i = 0; i < 4 * (length / 4); i++) {
      checksum ^= buffer[i];
    }

    return checksum;
  }

  BL_FUNCTION static void USART1_IRQ() {
    if (!(Port::Instance()->ISR & USART_ISR_RXNE)) {
      return;
    }

    port_buf::PushChar(Port::Instance()->RDR);
  }

  friend class VecT;

 public:
  BL_FUNCTION void Init() { Port::EnableRxInterrupt(); }

  BL_FUNCTION void Main() {
    Flash::Unlock();

    while (true) {
      Peripheral::Console::Write("## CMD WAIT\n", 13);
      auto cmd = ReceiveCommand();

      switch (cmd) {
        case Command::kACK: {
          Port::Write(static_cast<uint8_t>(ACK::kACK));
          break;
        }
        case Command::kRead: {
          auto addr = ReceiveU32();
          auto len = ReceiveU16();
          SendBuffer(reinterpret_cast<uint8_t*>(addr), len);
          break;
        }
        case Command::kChecksum: {
          auto addr = ReceiveU32();
          auto len = ReceiveU16();
          auto checksum = Checksum_Word(reinterpret_cast<uint8_t*>(addr), len);
          SendU32(checksum);
          break;
        }
        case Command::kWrite: {
          auto buffer = reinterpret_cast<uint16_t*>(Peripheral::kBufferAddr);

          auto addr = ReceiveU32();
          auto len = ReceiveU16();
          ReceiveBuffer((uint8_t*)buffer, len);

          if ((len & 1) == 1) {  // nack
            break;
          }

          auto flash_base = reinterpret_cast<uint16_t*>(addr);
          for (size_t i = 0; i < len / 2; i++) {
            Flash::Write(&flash_base[i], buffer[i]);
          }
          break;
        }
        case Command::kReset: {
          NVIC_SystemReset();
        }
        case Command::kLoad: {
          auto vect = reinterpret_cast<uint32_t*>(0x08000000);
          auto msp = vect[0];
          auto pc = reinterpret_cast<void (*)()>(vect[1]);

          __set_MSP(msp);
          pc();

          asm("nop");  // never reach here.
        }
        case Command::kErase: {
          auto pages = ReceiveU16();
          if (pages == 0xFFFF) {
            Flash::MassErase();
            break;
          }

          for (size_t i = 0; i < pages; i++) {
            auto page = ReceiveU16();
            Flash::Erase(page);
          }
          break;
        }

        case Command::kInvalid: {
          break;
        }
      }
    }
  }

  //* Singleton
 private:
  BL_FUNCTION CmdPort() = default;

 public:
  BL_FUNCTION static inline CmdPort& GetInstance() {
    return *reinterpret_cast<CmdPort*>(Peripheral::kCmdPortAddr);
  }
};

extern "C" void BL_Main();

extern "C" char _estack;
class VecT {
  using HandlerType = void (*)(void);
  static_assert(sizeof(HandlerType) == sizeof(void*));

  __attribute__((section(
      ".isr_vector"))) static inline HandlerType flash_vector[0x200 / 4] = {
      reinterpret_cast<VecT::HandlerType>(&_estack),  //
      BL_Main,                                        //
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,

      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,

      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,

      0,
      0,
      0,
      0,
      0,
      CmdPort::USART1_IRQ,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
  };

  static_assert(sizeof(flash_vector) == 0x200);

 public:
  static void Init() { SCB->VTOR = reinterpret_cast<uint32_t>(&flash_vector); }
};

extern "C" BL_FUNCTION2 void BL_Main() {
  __set_MSP(0x20003000);

  NVIC_DisableIRQ(TIM6_DAC_IRQn);
  NVIC_DisableIRQ(USART1_IRQn);
  NVIC_DisableIRQ(USART2_IRQn);

  SysTick->CTRL = 0;
  NVIC_DisableIRQ(SysTick_IRQn);

  VecT::Init();

  auto& cmd_port = CmdPort::GetInstance();
  cmd_port.Init();

  port_buf::Init();

  cmd_port.Main();
}

}  // namespace bootloader
