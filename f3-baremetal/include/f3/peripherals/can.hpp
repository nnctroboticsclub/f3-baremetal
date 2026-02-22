#pragma once

#include <stm32f303x8.h>

#include <array>
#include <cstdint>
#include <cstdio>
#include <utility>

#include <f3/peripherals/rcc.hpp>
#include <f3/ram_vector.hpp>

namespace stm32f3::can {
struct CANTiming {
  uint32_t brp;
  uint32_t ts1;
  uint32_t ts2;

  template <uint32_t apb1_freq>
  static consteval CANTiming FindAppropriateTiming(uint32_t target) {
    struct {
      float error;
      CANTiming timing;
    } best = {.error = 1.0, .timing = {.brp = 0, .ts1 = 3, .ts2 = 1}};

    for (uint32_t scale = 1; scale < 1024; scale++) {
      // ts1 = 3, ts2 = 1 ==> (1 + ts1 + ts2) = 5
      if ((float)apb1_freq / (float)scale / 5 != target)
        continue;

      best.error = 0;
      best.timing.brp = scale;
      best.timing.ts1 = 3;
      best.timing.ts2 = 1;

      break;
    }

    if (best.error != 0) {
      printf("Cannot find exact timing\n");
    }
    if (apb1_freq / best.timing.brp / (1 + best.timing.ts1 + best.timing.ts2) !=
        target) {
      printf("Cannot find appropriate timing\n");
    }

    return best.timing;
  }
};

template <int kNumber>
struct CANFilter {
  static constexpr int kActivePos = kNumber * 1;       // FA1R
  static constexpr int kActiveMask = 1 << kActivePos;  // FA1R

  static constexpr int kScalePos = kNumber * 1;      // FS1R
  static constexpr int kScaleMask = 1 << kScalePos;  // FS1R
                                                     //
  static constexpr int kModePos = kNumber * 1;       // FS1R
  static constexpr int kModeMask = 1 << kModePos;    // FS1R

  static constexpr int kFifoAssignmentPos = kNumber * 1;               // FS1R
  static constexpr int kFifoAssignmentMask = 1 << kFifoAssignmentPos;  // FS1R

  static constexpr auto FilterRegister() {
    return &CAN->sFilterRegister[kNumber];
  }
};

struct MaskFilter {
  uint32_t id;
  uint32_t mask;

  template <int kFilterNumber>
  inline void Configure() const {
    using Filter = CANFilter<kFilterNumber>;

    CAN->FA1R &= ~Filter::kActiveMask;  // Inactive

    CAN->FS1R &= ~Filter::kScaleMask;     // 32-bit scale
    Filter::FilterRegister()->FR1 = 0x0;  // ID
    Filter::FilterRegister()->FR2 = 0x0;  // Mask
    CAN->FM1R &= ~Filter::kModeMask;      // Mask mode

    CAN->FFA1R &= ~Filter::kFifoAssignmentMask;  // FIFO 0
                                                 //
    CAN->FA1R |= Filter::kActiveMask;            // Inactive
  }
};

struct CANErrorStatistic {
  unsigned int rec;   // : 8
  unsigned int tec;   // : 8
  unsigned int lec;   // : 3
  unsigned int boff;  // : 1
  unsigned int epvf;  // : 1
  unsigned int ewgf;  // : 1

  void Update() {
    rec = (CAN->ESR & CAN_ESR_REC_Msk) >> CAN_ESR_REC_Pos;
    tec = (CAN->ESR & CAN_ESR_TEC_Msk) >> CAN_ESR_TEC_Pos;
    lec = (CAN->ESR & CAN_ESR_LEC_Msk) >> CAN_ESR_LEC_Pos;
    boff = (CAN->ESR & CAN_ESR_BOFF_Msk) >> CAN_ESR_BOFF_Pos;
    epvf = (CAN->ESR & CAN_ESR_EPVF_Msk) >> CAN_ESR_EPVF_Pos;
    ewgf = (CAN->ESR & CAN_ESR_EWGF_Msk) >> CAN_ESR_EWGF_Pos;
  }

  [[nodiscard]] const char* StatusToString() const {
    static char status[] = "...";
    if (boff)
      status[0] = 'B';
    if (epvf)
      status[1] = 'P';
    if (ewgf)
      status[2] = 'W';

    return status;
  }

  [[nodiscard]] const char* LastErrorCodeToString() const {
    static const char* lec_str[] = {"No Error",  "Stuff Error",   "Form Error",
                                    "Ack Error", "Bit1 Error",    "Bit0 Error",
                                    "CRC Error", "Software Error"};

    return lec_str[lec];
  }
};

struct CANMessage {
  uint32_t id = 0;
  uint32_t length;
  std::array<uint8_t, 8> data = {0};

  constexpr uint32_t EncodeHigh() const {
    return data[4] | (data[5] << 8) | (data[6] << 16) | (data[7] << 24);
  }

  constexpr uint32_t EncodeLow() const {
    return data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
  }
};

template <int kMailboxId>
class Mailbox {
  static inline CAN_TxMailBox_TypeDef& mailbox = CAN->sTxMailBox[kMailboxId];

  bool txrq = false;
  bool rqcp = false;
  bool txok = false;
  bool alst = false;
  bool terr = false;

 public:
  CAN_TxMailBox_TypeDef& GetMailbox() { return mailbox; }

  void RequestAbort() {
    auto abrq_pos = 7 + 8 * kMailboxId;
    CAN->TSR |= (1 << abrq_pos);

    while ((CAN->TSR & (1 << abrq_pos)) != 0)
      ;
  }

  void Update() {
    txrq = (mailbox.TIR & CAN_TI0R_TXRQ) >> CAN_TI0R_TXRQ_Pos;

    auto rqcp_pos = 0 + 8 * kMailboxId;
    auto txok_pos = 1 + 8 * kMailboxId;
    auto alst_pos = 2 + 8 * kMailboxId;
    auto terr_pos = 3 + 8 * kMailboxId;

    rqcp = CAN->TSR & (1 << rqcp_pos);  // Request Complete
    txok = CAN->TSR & (1 << txok_pos);  // Transmission OK
    alst = CAN->TSR & (1 << alst_pos);  // Arbitration Lost
    terr = CAN->TSR & (1 << terr_pos);  // Transmission Error
  }

  [[nodiscard]] const char* StatusToString() const {
    static char status[] = "...";
    if (txrq)
      status[0] = 'R';
    if (rqcp)
      status[1] = 'Q';
    if (txok)
      status[2] = 'O';
    if (alst)
      status[3] = 'A';

    return status;
  }

  void Send(CANMessage const& message) {
    mailbox.TIR = (message.id << CAN_TI0R_EXID_Pos);
    mailbox.TIR |= CAN_TI0R_IDE;
    mailbox.TDTR = message.length;
    mailbox.TDLR = message.EncodeLow();
    if (message.data.size() > 4) {
      mailbox.TDHR = message.EncodeHigh();
    }

    mailbox.TIR |= CAN_TI0R_TXRQ;

    Update();
  }

  [[nodiscard]] uint8_t GetDLC() const {
    return (mailbox.TDTR & CAN_TDT0R_DLC_Msk) >> CAN_TDT0R_DLC_Pos;
  }

  std::array<uint8_t, 8> GetData() const {
    std::array<uint8_t, 8> data = {0};

    data[0] = (mailbox.TDLR & 0x000000FF) >> 0;
    data[1] = (mailbox.TDLR & 0x0000FF00) >> 8;
    data[2] = (mailbox.TDLR & 0x00FF0000) >> 16;
    data[3] = (mailbox.TDLR & 0xFF000000) >> 24;

    data[4] = (mailbox.TDHR & 0x000000FF) >> 0;
    data[5] = (mailbox.TDHR & 0x0000FF00) >> 8;
    data[6] = (mailbox.TDHR & 0x00FF0000) >> 16;
    data[7] = (mailbox.TDHR & 0xFF000000) >> 24;

    return data;
  }
};

template <typename T>
concept CANHandler = requires {
  {T::HandleRx(0, std::declval<CANMessage>())}->std::same_as<void>;
  {T::HandleRx(1, std::declval<CANMessage>())}->std::same_as<void>;
  {T::HandleError()}->std::same_as<void>;
};

template <CANHandler Handler>
class BaremetalCAN {
  static inline void Reset() {
    CAN->MCR |= CAN_MCR_RESET;
    while (CAN->MCR & CAN_MCR_RESET)
      ;
  }

  static inline void ExitSleepMode() {
    CAN->MCR &= ~CAN_MCR_SLEEP;
    while ((CAN->MSR & CAN_MSR_SLAK) == 1)
      ;
  }

  static inline void RequestInitializationMode() {
    CAN->MCR |= CAN_MCR_INRQ;
    while ((CAN->MSR & CAN_MSR_INAK) == 0)
      ;
  }

  static inline void LeaveInitializationMode() {
    CAN->MCR &= ~CAN_MCR_INRQ;
    while ((CAN->MSR & CAN_MSR_INAK) != 0)
      ;
  }

  template <rcc::RCCConfigLike kRcc, int kBaudrate>
  static inline void InitCAN_Master() {
    Reset();
    ExitSleepMode();
    RequestInitializationMode();

    // Config
    CAN->MCR &= ~CAN_MCR_TTCM;  // Disable Time Trigger Communication Mode
    CAN->MCR &= ~CAN_MCR_ABOM;  // Disable Automatic Bus-Off Management
    CAN->MCR &= ~CAN_MCR_AWUM;  // Disable Automatic Wakeup Mode
    CAN->MCR &= ~CAN_MCR_NART;  // ENable Automatic Retransmission
    CAN->MCR &= ~CAN_MCR_RFLM;  // Disable Receive FIFO Locked Mode
    CAN->MCR &= ~CAN_MCR_TXFP;  // Disable Transmit FIFO Priority

    // Timing
    constexpr auto timing =
        CANTiming::FindAppropriateTiming<kRcc::GetAPB1Clock()>(kBaudrate);
    uint32_t btr = 0;
    btr |= ((timing.brp - 1) << CAN_BTR_BRP_Pos);
    btr |= ((timing.ts1 - 1) << CAN_BTR_TS1_Pos);
    btr |= ((timing.ts2 - 1) << CAN_BTR_TS2_Pos);
    btr |= (1 << CAN_BTR_SJW_Pos);
    CAN->BTR = btr;
  }

  static inline void InitCAN_Filter() {
    CAN->FMR |= CAN_FMR_FINIT;  // Enter Filter Initialization Mode

    MaskFilter{.id = 0x0, .mask = 0x0}.Configure<0>();

    CAN->FMR &= ~CAN_FMR_FINIT;  // Leave Filter Initialization Mode
  }

  static auto GetFreeMailbox() {
    auto tme0 = (CAN->TSR & CAN_TSR_TME0) >> CAN_TSR_TME0_Pos;
    auto tme1 = (CAN->TSR & CAN_TSR_TME1) >> CAN_TSR_TME1_Pos;
    auto tme2 = (CAN->TSR & CAN_TSR_TME2) >> CAN_TSR_TME2_Pos;

    if (tme0)
      return 0;

    if (tme1)
      return 1;

    if (tme2)
      return 2;

    return -1;
  }

 public:
  static inline void Start() { LeaveInitializationMode(); }

  template <rcc::RCCConfigLike kRcc, int kBaudrate>
  static inline void Init() {
    RCC->APB1ENR |= RCC_APB1ENR_CANEN;

    InitCAN_Master<kRcc, kBaudrate>();
    InitCAN_Filter();
    Start();

    // Interrupt
    CAN->IER |= CAN_IER_FMPIE0;  // FIFO 0 Message Pending Interrupt Enable
    CAN->IER |= CAN_IER_FMPIE1;  // FIFO 0 Message Pending Interrupt Enable

    NVIC_SetPriority(CAN_RX0_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
    NVIC_EnableIRQ(CAN_RX0_IRQn);
    stm32f3::ram_vector::ram_vector[16 + CAN_RX0_IRQn] = []() {
      ISR_ProcessRxFIFO(0);
    };

    NVIC_SetPriority(CAN_RX1_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
    NVIC_EnableIRQ(CAN_RX1_IRQn);
    stm32f3::ram_vector::ram_vector[16 + CAN_RX1_IRQn] = []() {
      ISR_ProcessRxFIFO(1);
    };
  }

  static inline auto GetErrorStatistic() {
    error_statistic_.Update();
    return error_statistic_;
  }

  template <int kMailboxNumber>
  static inline auto GetTxMailbox() {
    if constexpr (kMailboxNumber == 0) {
      return mailbox0_;
    } else if constexpr (kMailboxNumber == 1) {
      return mailbox1_;
    } else if constexpr (kMailboxNumber == 2) {
      return mailbox2_;
    }
  }

  static inline auto Send(const CANMessage& message) {
    switch (GetFreeMailbox()) {
      case 0:
        mailbox0_.Send(message);
        return true;
      case 1:
        mailbox1_.Send(message);
        return true;
      case 2:
        mailbox2_.Send(message);
        return true;
      default:
        return false;
    }
  }

  static inline void ISR_ProcessRxFIFO(int i) {
    const auto fr_from = i == 0 ? CAN_RF0R_RFOM0 : CAN_RF1R_RFOM1;
    auto& frr = i == 0 ? CAN->RF0R : CAN->RF1R;

    auto& fifo = CAN->sFIFOMailBox[i];

    auto ide = (fifo.RIR & CAN_RI0R_IDE) >> CAN_RI0R_IDE_Pos;

    auto id =
        ide ? (fifo.RIR >> CAN_RI0R_EXID_Pos) : (fifo.RIR >> CAN_RI0R_STID_Pos);

    auto dlc = (fifo.RDTR & CAN_RDT0R_DLC_Msk) >> CAN_RDT0R_DLC_Pos;

    CANMessage msg = {.id = id, .length = dlc};

    msg.data[0] = (fifo.RDLR & 0x000000FF) >> 0;
    msg.data[1] = (fifo.RDLR & 0x0000FF00) >> 8;
    msg.data[2] = (fifo.RDLR & 0x00FF0000) >> 16;
    msg.data[3] = (fifo.RDLR & 0xFF000000) >> 24;

    if (dlc > 4) {
      msg.data[4] = (fifo.RDHR & 0x000000FF) >> 0;
      msg.data[5] = (fifo.RDHR & 0x0000FF00) >> 8;
      msg.data[6] = (fifo.RDHR & 0x00FF0000) >> 16;
      msg.data[7] = (fifo.RDHR & 0xFF000000) >> 24;
    }

    Handler::HandleRx(i, msg);

    frr |= fr_from;  // Release FIFO 0
  }

 private:
  static inline CANErrorStatistic error_statistic_;
  static inline Mailbox<0> mailbox0_;
  static inline Mailbox<1> mailbox1_;
  static inline Mailbox<2> mailbox2_;
};

}  // namespace stm32f3::can
