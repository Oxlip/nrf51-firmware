"""
Test BLE Discoverability of Aura/Lyra/Mira...
"""

from btle import UUID, Peripheral

def _BASE_UUID(offset):
    return UUID("%08X-9324-4085-aba0-0902c0e8950a" % (0xc0f40000 + offset))

DIMMER_SVC_UUID = _BASE_UUID(0x1001)
CS_SVC_UUID = _BASE_UUID(0x1002)
TS_SVC_UUID = _BASE_UUID(0x1003)
HS_SVC_UUID = _BASE_UUID(0x1004)
LS_SVC_UUID = _BASE_UUID(0x1005)
MS_SVC_UUID = _BASE_UUID(0x1006)

DIMMER_CHAR_UUID = _BASE_UUID(0x2001)
CS_CHAR_UUID = _BASE_UUID(0x2002)
TS_CHAR_UUID = _BASE_UUID(0x2003)
HS_CHAR_UUID = _BASE_UUID(0x2004)
LS_CHAR_UUID = _BASE_UUID(0x2005)
MS_CHAR_UUID = _BASE_UUID(0x2006)


class Aura(Peripheral):
    def __init__(self, mac_address):
        self.peripheral = Peripheral(mac_address)

        #self.dimmer_svc = self.peripheral.getServiceByUUID(DIMMER_SVC_UUID)
        #self.dimmer_char = self.service.getCharacteristics(DIMMER_CHAR_UUID) [0]

    def __del__(self):
        self.peripheral.disconnect()

    def set_dimmer(self, value):
        self.dimmer_char.write(value)

    def get_dimmer(self):
        return self.dimmer_char.read()

    def get_cs_value(self):
        pass
        
