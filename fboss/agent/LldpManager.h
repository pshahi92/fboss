/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */
// Copyright 2014-present Facebook. All Rights Reserved.
#pragma once
#include <folly/io/async/AsyncTimeout.h>
#include <unordered_map>
#include <memory>
#include "fboss/agent/state/Port.h"
#include "fboss/agent/state/PortMap.h"
#include "fboss/agent/Platform.h"
#include "fboss/agent/state/SwitchState.h"

namespace folly { namespace io {
class Cursor;
}}

namespace facebook { namespace fboss {
class LldpManager : private folly::AsyncTimeout {
  /*
   * LldpManager is the class that manages Lldp support.
   * Responsible for processing received LLDP frames and maintaining the
   * LLDP neighbors table.
   * Also, responsible for periodically sending LLDP frames on all the ports
   * to inform of this switch's presence to its neighbors. Hence inheriting
   * the TAsyncTimeout class for that purpose.
   */
 public:
  enum : uint16_t { ETHERTYPE_LLDP = 0x88CC,
                    LLDP_INTERVAL = 15*1000,
                    TLV_TYPE_BITS_LENGTH = 7,
                    TLV_LENGTH_BITS_LENGTH = 9,
                    TLV_TYPE_LEFT_SHIFT_OFFSET = 9,
                    TLV_LENGTH_LEFT_SHIFT_OFFSET = 1,
                    CHASSIS_TLV_TYPE = 0x01,
                    CHASSIS_TLV_LENGTH = 0x07,
                    CHASSIS_TLV_SUB_TYPE_MAC = 0x04,
                    PORT_TLV_TYPE = 0x02,
                    PORT_TLV_SUB_TYPE_INTERFACE = 0x5,
                    SYSTEM_NAME_TLV_TYPE = 0x5,
                    SYSTEM_DESCRIPTION_TLV_TYPE = 0x6,
                    SYSTEM_CAPABILITY_TLV_TYPE = 0x7,
                    SYSTEM_CAPABILITY_TLV_LENGTH = 0x4,
                    SYSTEM_CAPABILITY_SWITCH = 1 << 2, // 3rd bit for switch
                    SYSTEM_CAPABILITY_ROUTER = 1 << 4, // 5th bit for router
                    TTL_TLV_TYPE = 0x3,
                    TTL_TLV_LENGTH = 0x2,
                    TTL_TLV_VALUE = 120,
                    PDU_END_TLV_TYPE = 0,
                    PDU_END_TLV_LENGTH = 0};
  explicit LldpManager(SwSwitch* sw);
  ~LldpManager();
  static const folly::MacAddress LLDP_DEST_MAC;

  /*
   * Start the timer to send LLDP packets.
   *
   * This may be called from any thread.
   */
  void start();

  /*
   * Stop sending LLDP packets.
   *
   * This must be called before while the SwSwitch background thread is still
   * running.
   */
  void stop();

  // This function is internal.  It is only public for use in unit tests.
  void sendLldpOnAllPorts(bool checkPortStatusFlag);

 private:
  void timeoutExpired() noexcept;

  SwSwitch* sw_{nullptr};
  std::chrono::milliseconds interval_;
  void sendLldpInfo(SwSwitch* sw, const std::shared_ptr<SwitchState>& swState,
                    const std::shared_ptr<Port>& port);
};
}} // facebook::fboss
