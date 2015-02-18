#!/bin/bash
expect -c "
spawn sftp firmware@nuton.in
expect \"password\"
send \"binaryRocks\r\"
expect \"sftp>\"
send \"rm upload/aura_s110.bin\r\"
expect \"sftp>\"
send \"put _build/aura_s110.bin upload\r\"
expect \"sftp>\"
send \"bye\r\""


curl "https://nuton.in/download/update_firmware?dt=aura&hw=v1&filename=aura_s110.bin&fw_date=`cat _build/build_time.txt`"
