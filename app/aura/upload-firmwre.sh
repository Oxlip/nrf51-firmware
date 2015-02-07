#!/bin/bash
expect -c "
spawn sftp firmware@nuton.in
expect \"password\"
send \"binaryRocks\r\"
expect \"sftp>\"
send \"put _build/aura_s110.bin upload\r\"
expect \"sftp>\"
send \"bye\r\""
