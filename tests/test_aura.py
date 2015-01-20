"""
Test code for Aura's BLE functionality.
"""
import time
from btle import UUID, Peripheral
from Nuton import Aura

def test_connect(mac_address):
    """ Test connecitivity to given Aura
    """
    aura = Aura(mac_address)

def test_triac(mac_address):
    aura = Aura(mac_address)
    return
    for i in [100, 75, 50, 25, 0]:
        aura.set_dimmer(i)
        time.sleep(2)
        assert aura.get_dimmer() == i