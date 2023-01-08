#!/bin/bash

set -euo pipefail

platformio ci --board esp32dev --lib=. examples/send_receive_sms
platformio ci --board esp32dev --lib=. examples/send_sms
